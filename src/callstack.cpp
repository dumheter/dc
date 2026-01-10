/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <dc/callstack.hpp>
#include <dc/fmt.hpp>
#include <dc/log.hpp>
#include <dc/platform.hpp>
#include <dc/traits.hpp>

#if defined(DC_PLATFORM_WINDOWS)
#if !defined(VC_EXTRALEAN)
#define VC_EXTRALEAN
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
// clang-format off
#include <Windows.h> //< windows first or pain
#include <Psapi.h>
// clang-format on
#pragma pack(push, before_imagehlp, 8)
#include <imagehlp.h>
#pragma pack(pop, before_imagehlp)
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

#else
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <libdwfl.h>
#include <limits.h>
#include <unistd.h>
#endif

namespace dc {

///////////////////////////////////////////////////////////////////////////////
// Windows
//
#if defined(DC_PLATFORM_WINDOWS)

struct ModuleInfo {
  String imageName;
  String moduleName;
  void* baseAddr;
  DWORD loadSize;

  static Result<ModuleInfo, CallstackErr> create(HANDLE process,
                                                 HMODULE module);
};

// TODO cgustafsson: This function can only be called from one thread, should
// have a mutex or something to protect.
Result<ModuleInfo, CallstackErr> ModuleInfo::create(HANDLE process,
                                                    HMODULE module) {
  ModuleInfo out;

  MODULEINFO moduleInfo;
  bool ok =
      GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo));
  if (!ok)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});
  out.baseAddr = moduleInfo.lpBaseOfDll;
  out.loadSize = moduleInfo.SizeOfImage;

  out.imageName.resize(1024);
  DWORD len = GetModuleFileNameEx(process, module, out.imageName.getData(),
                                  static_cast<DWORD>(out.imageName.getSize()));
  if (len == 0)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});
  out.imageName.resize(len);

  out.moduleName.resize(1024);
  len = GetModuleBaseName(process, module, out.moduleName.getData(),
                          static_cast<DWORD>(out.moduleName.getSize()));
  if (len == 0)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});
  out.moduleName.resize(len);

  HANDLE file = NULL;               //< no file
  PMODLOAD_DATA headerData = NULL;  //< extra header info, not needed
  DWORD flags = 0;                  //< no extra flags
  DWORD64 okLoad = SymLoadModuleEx(
      process, file, out.imageName.getData(), out.moduleName.getData(),
      reinterpret_cast<DWORD64>(out.baseAddr), out.loadSize, headerData, flags);
  if (okLoad == 0)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});

  return Ok(dc::move(out));
};

class Symbol {
 public:
  static constexpr int kMaxNameLen = 1024;

 public:
  static Result<Symbol, CallstackErr> create(HANDLE process, DWORD64 addr) {
    Symbol symbol;

    DWORD64 displacement;
    const bool ok = SymFromAddr(process, addr, &displacement, symbol.m_sym);
    if (ok)
      return Ok(dc::move(symbol));
    else
      return Err(CallstackErr{static_cast<u64>(GetLastError()),
                              CallstackErr::ErrType::Sys, __LINE__});
  }

  Symbol(Symbol&& other) : m_sym(other.m_sym) { other.m_sym = nullptr; }
  Symbol& operator=(Symbol&& other) {
    if (this != &other) dc::swap(m_sym, other.m_sym);

    return *this;
  }

  ~Symbol() { delete m_sym; }

  String name() const { return String(m_sym->Name); }

  Result<String, CallstackErr> undecoratedName() const {
    if (*m_sym->Name == 0) return Ok<String>("<name not available>");

    String name;
    name.resize(kMaxNameLen);
    const DWORD bytesWritten = UnDecorateSymbolName(
        m_sym->Name, reinterpret_cast<char*>(name.getData()),
        static_cast<DWORD>(name.getSize()), UNDNAME_COMPLETE);
    if (bytesWritten == 0)
      return Err(CallstackErr{static_cast<u64>(GetLastError()),
                              CallstackErr::ErrType::Sys, __LINE__});

    name.resize(bytesWritten);
    return Ok<String>(dc::move(name));
  }

 private:
  Symbol()
      : m_sym((decltype(m_sym))::operator new(sizeof(*m_sym) + kMaxNameLen)) {
    m_sym->SizeOfStruct = sizeof(*m_sym);
    m_sym->MaxNameLen = kMaxNameLen;
  }

 private:
  SYMBOL_INFO* m_sym;
};

Result<CallstackAddresses, CallstackErr> captureCallstack() {
  CallstackAddresses addresses;
  addresses.addresses.resize(64);

  const USHORT framesCaptured = RtlCaptureStackBackTrace(
      1 /*frames to skip*/, 64 /* max frames to capture */,
      addresses.addresses.begin(), NULL /* optional hash */);
  addresses.addresses.resize(framesCaptured);

  return Ok(dc::move(addresses));
}

Result<Callstack, CallstackErr> resolveCallstack(
    const CallstackAddresses& addresses) {
  HANDLE process = GetCurrentProcess();

  if (!SymInitialize(process, NULL, false))
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});

  DWORD symOptions = SymGetOptions();
  symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
  SymSetOptions(symOptions);

  List<HMODULE> moduleHandles;
  moduleHandles.resize(10);
  DWORD bytesNeeded;
  bool ok = EnumProcessModules(
      process, moduleHandles.begin(),
      static_cast<DWORD>(moduleHandles.getSize() * sizeof(HMODULE)),
      &bytesNeeded);
  if (!ok) {
    SymCleanup(process);
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});
  }

  if (bytesNeeded > 0) {
    moduleHandles.resize(bytesNeeded / sizeof(HMODULE));
    ok = EnumProcessModules(
        process, moduleHandles.begin(),
        static_cast<DWORD>(moduleHandles.getSize() * sizeof(HMODULE)),
        &bytesNeeded);
    if (!ok) {
      SymCleanup(process);
      return Err(CallstackErr{static_cast<u64>(GetLastError()),
                              CallstackErr::ErrType::Sys, __LINE__});
    }
  }

  List<ModuleInfo> modules;
  modules.reserve(40);
  for (HMODULE module : moduleHandles) {
    auto res = ModuleInfo::create(process, module);
    if (res.isOk()) modules.add(dc::move(res).unwrap());
  }

  String str;

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(line);
  DWORD offsetFromSymbol = 0;

  for (void* addr : addresses.addresses) {
    DWORD64 addrPC = reinterpret_cast<DWORD64>(addr);

    Result<Symbol, CallstackErr> symbol = Symbol::create(process, addrPC);

    Result<String, CallstackErr> fnName = symbol.match(
        [](const Symbol& symbol) { return symbol.undecoratedName(); },
        [addrPC](const CallstackErr&) -> Result<String, CallstackErr> {
          return dc::move(dc::formatStrict("{#x}", addrPC))
              .mapErr([](const FormatErr& err) {
                return CallstackErr(static_cast<u64>(err.kind),
                                    CallstackErr::ErrType::Fmt, __LINE__);
              });
        });

    if (fnName.isOk() && fnName.value() != "dc::buildCallstack") {
      const BOOL hasLine =
          SymGetLineFromAddr64(process, addrPC, &offsetFromSymbol, &line);

      String fileLine;
      if (hasLine) {
        fileLine = dc::formatStrict("{}:{}", line.FileName, line.LineNumber)
                       .unwrapOr("?:?");
      } else {
        fileLine = "?:?";
      }

      auto res = formatTo(
          str, "  {} ({})\n",
          fnName.match(
              [](const String& s) -> String { return s.clone(); },
              [](const CallstackErr&) -> String { return String("?fn?"); }),
          fileLine);
      if (res.isErr()) {
        for (const ModuleInfo& module : modules)
          SymUnloadModule64(process, (DWORD64)module.baseAddr);
        SymCleanup(process);
        return Err(CallstackErr(static_cast<u64>(-1),
                                CallstackErr::ErrType::Fmt, __LINE__));
      }

      if (fnName.isOk() && fnName.value() == "RaiseException") break;
      if (fnName.isOk() && fnName.value() == "main") break;
    }
  }

  for (const ModuleInfo& module : modules)
    SymUnloadModule64(process, (DWORD64)module.baseAddr);

  SymCleanup(process);

  return Ok(Callstack{dc::move(str)});
}

Result<Callstack, CallstackErr> buildCallstack() {
  Result<CallstackAddresses, CallstackErr> addresses = captureCallstack();
  if (addresses.isErr()) return Err(dc::move(addresses).errValue());

  return resolveCallstack(addresses.value());
}

String CallstackErr::toString() const {
  switch (errType) {
    case ErrType::Sys: {
      LPVOID lpMsgBuf;

      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, static_cast<DWORD>(errCode),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf, 0, NULL);

      String out(static_cast<char*>(lpMsgBuf));
      LocalFree(lpMsgBuf);

      return out;
    }

    case ErrType::Fmt:
    default:
      return String(dc::toString(static_cast<FormatErr::Kind>(errCode)));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Linux
//

#elif defined(DC_PLATFORM_LINUX)

static String getLineInfo(Dwfl* dwfl, void* addr) {
  String out = "?:?";
  Dwarf_Addr pc = reinterpret_cast<Dwarf_Addr>(addr);
  Dwfl_Line* line = dwfl_getsrc(dwfl, pc);
  if (!line) {
    return out;
  }

  int lineno = 0;
  const char* filename =
      dwfl_lineinfo(line, nullptr, nullptr, &lineno, nullptr, nullptr);

  formatTo(out, "{}:{}", filename, lineno);
  return out;
}

static Dwfl* initializeDwfl() {
  static Dwfl* s_dwfl = nullptr;

  if (s_dwfl) {
    return s_dwfl;
  }

  static const Dwfl_Callbacks proc_callbacks = {
      .find_elf = dwfl_linux_proc_find_elf,
      .find_debuginfo = dwfl_standard_find_debuginfo,
      .section_address = dwfl_offline_section_address,
      .debuginfo_path = nullptr,
  };

  s_dwfl = dwfl_begin(&proc_callbacks);
  if (!s_dwfl) {
    return nullptr;
  }

  int pid = getpid();
  if (dwfl_linux_proc_report(s_dwfl, pid) != 0) {
    dwfl_end(s_dwfl);
    s_dwfl = nullptr;
    return nullptr;
  }

  if (dwfl_report_end(s_dwfl, nullptr, nullptr) != 0) {
    dwfl_end(s_dwfl);
    s_dwfl = nullptr;
    return nullptr;
  }

  return s_dwfl;
}

Result<CallstackAddresses, CallstackErr> captureCallstack() {
  constexpr usize fnRetAddrSize = 128;
  void* fnRetAddr[fnRetAddrSize];
  int size = backtrace(fnRetAddr, fnRetAddrSize);

  CallstackAddresses addresses;
  addresses.addresses.reserve(static_cast<usize>(size));

  bool skipSelf = true;
  for (int i = 0; i < size; ++i) {
    if (skipSelf) {
      skipSelf = false;
      continue;
    }
    addresses.addresses.add(fnRetAddr[i]);
  }

  return Ok(dc::move(addresses));
}

Result<Callstack, CallstackErr> resolveCallstack(
    const CallstackAddresses& addresses) {
  Dwfl* dwfl = initializeDwfl();
  if (!dwfl) {
    return Err(CallstackErr(0, CallstackErr::ErrType::Sys, __LINE__));
  }

  String str;
  str.getInternalList().reserve(2048);

  for (void* addr : addresses.addresses) {
    Dl_info symInfo;
    int res = dladdr(static_cast<const void*>(addr), &symInfo);
    if (res != 0) {
      int status;
      // __cxa_demangle allocates with malloc, so pass nullptr and let it
      // allocate. We must free the result with free(), not delete[].
      char* fnName =
          abi::__cxa_demangle(symInfo.dli_sname, nullptr, nullptr, &status);

      String fileLine = getLineInfo(dwfl, addr);

      bool shouldBreak = false;
      if (status == 0 || status == -2) {
        const char* name = fnName ? fnName : symInfo.dli_sname;
        if (name && strcmp(name, "dc::buildCallstack") != 0) {
          auto fmtRes = formatTo(str, "  {} ({})\n", name, fileLine);
          if (fmtRes.isErr()) {
            free(fnName);
            return Err(CallstackErr(static_cast<u64>(fmtRes.errValue().kind),
                                    CallstackErr::ErrType::Fmt, __LINE__));
          }
          if (name && strcmp(name, "main") == 0) shouldBreak = true;
        }
      } else {
        auto fmtRes = formatTo(str, "{}\n", static_cast<const void*>(addr));
        if (fmtRes.isErr()) {
          free(fnName);
          return Err(CallstackErr(static_cast<u64>(fmtRes.errValue().kind),
                                  CallstackErr::ErrType::Fmt, __LINE__));
        }
      }
      free(fnName);
      if (shouldBreak) break;

    } else {
      LOG_WARNING("failed to dladdr");
      auto fmtRes = formatTo(str, "{}\n", static_cast<const void*>(addr));
      if (fmtRes.isErr())
        return Err(CallstackErr(static_cast<u64>(fmtRes.errValue().kind),
                                CallstackErr::ErrType::Fmt, __LINE__));
    }
  }

  if (!str.isEmpty()) str.resize(str.getSize() - 1);

  return Ok(Callstack(str.toView()));
}

Result<Callstack, CallstackErr> buildCallstack() {
  Result<CallstackAddresses, CallstackErr> addresses = captureCallstack();
  if (addresses.isErr()) return Err(dc::move(addresses).errValue());

  return resolveCallstack(addresses.value());
}

String CallstackErr::toString() const {
  return "<error building the callstack>";
}

#endif

}  // namespace dc
