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

// TODO do we want to be able to catch SEH exceptions? see gtest.cc:2588
// https://github.com/google/googletest/blob/f16d43cd38e9f2e41357dba8445f9d3a32d4e83d/googletest/src/gtest.cc
// DWORD callstackFromException(_EXCEPTION_POINTERS* ep)
// {
// 	HANDLE thread = GetCurrentThread();
// 	CONTEXT* context = ep->ContextRecord;

// 	return callstack(thread, context);
// }

static Result<Callstack, CallstackErr> buildCallstackAux(HANDLE process,
                                                         HANDLE thread,
                                                         CONTEXT* context);

Result<Callstack, CallstackErr> buildCallstack() {
  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();
  CONTEXT context;
  RtlCaptureContext(&context);

  // TODO cgustafsson: make thread safe, symInitialize is not thread safe
  Result<Callstack, CallstackErr> out =
      SymInitialize(process, NULL, false)
          ? buildCallstackAux(process, thread, &context)
          : Err(CallstackErr{static_cast<u64>(GetLastError()),
                             CallstackErr::ErrType::Sys, __LINE__});

  SymCleanup(process);

  return out;
}

/// Note: Using an aux function to ensure we can return on error, but still do
/// the cleanup before fully returning to the caller.
static Result<Callstack, CallstackErr> buildCallstackAux(HANDLE process,
                                                         HANDLE thread,
                                                         CONTEXT* context) {
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
  if (!ok)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});

  if (bytesNeeded > 0) {
    moduleHandles.resize(bytesNeeded / sizeof(HMODULE));
    ok = EnumProcessModules(
        process, moduleHandles.begin(),
        static_cast<DWORD>(moduleHandles.getSize() * sizeof(HMODULE)),
        &bytesNeeded);
    if (!ok)
      return Err(CallstackErr{static_cast<u64>(GetLastError()),
                              CallstackErr::ErrType::Sys, __LINE__});
  }

  // TODO cgustafsson: why load all modules when only using 1?
  List<ModuleInfo> modules;
  modules.reserve(40);
  for (HMODULE module : moduleHandles) {
    auto res = ModuleInfo::create(process, module);
    if (res.isOk()) modules.add(dc::move(res).unwrap());
    // else
    //   return dc::move(res).map([](ModuleInfo&&) { return Callstack(); });
  }

  void* base = modules[0].baseAddr;
  IMAGE_NT_HEADERS* imageHeader = ImageNtHeader(base);
  if (!imageHeader)
    return Err(CallstackErr{static_cast<u64>(GetLastError()),
                            CallstackErr::ErrType::Sys, __LINE__});
  const DWORD machineType = imageHeader->FileHeader.Machine;

#if defined(_M_X64)
  STACKFRAME64 frame;
  frame.AddrPC.Offset = context->Rip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Rsp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Rbp;
  frame.AddrFrame.Mode = AddrModeFlat;
#elif defined(_M_IX86)
  STACKFRAME64 frame;
  frame.AddrPC.Offset = context->Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
#else
#error "platform not supported"
#endif

  String str;

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(line);
  [[maybe_unused]] DWORD offsetFromSymbol = 0;

  do {
    if (frame.AddrPC.Offset != 0) {
      Result<Symbol, CallstackErr> symbol =
          Symbol::create(process, frame.AddrPC.Offset);

      Result<String, CallstackErr> fnName = symbol.match(
          [](const Symbol& symbol) { return symbol.undecoratedName(); },
          [frame](const CallstackErr&) -> Result<String, CallstackErr> {
            return dc::move(dc::format("{#x}", frame.AddrPC.Offset))
                .mapErr([](const FormatErr& err) {
                  return CallstackErr(static_cast<u64>(err.kind),
                                      CallstackErr::ErrType::Fmt, __LINE__);
                });
          });

      if (fnName.isOk() && fnName.value() != "dc::buildCallstack") {
        auto res = formatTo(
            str, "{}",
            fnName.match(
                [](const String& s) -> String { return s.clone(); },
                [](const CallstackErr&) -> String { return String("?fn?"); }));
        if (res.isErr())
          return Err(CallstackErr(static_cast<u64>(-1),
                                  CallstackErr::ErrType::Fmt, __LINE__));

        if (fnName.isOk() && fnName.value() != "dc::buildCallstack") {
          auto formatRes2 = formatTo(
              str, "{}",
              fnName.match([](const String& s) -> String { return s.clone(); },
                           [](const CallstackErr&) -> String {
                             return String("?fn?");
                           }));
          if (res.isErr())
            return Err(CallstackErr(static_cast<u64>(-1),
                                    CallstackErr::ErrType::Fmt, __LINE__));
        }

        // TODO cgustafsson: how to handle RaiseException
        if (fnName.isOk() && fnName.value() == "RaiseException") break;

        if (fnName.isOk() && fnName.value() == "main") break;
      } else {
        auto res = formatTo(str, "(No symbols: AddrPC.Offset == 0)");
        if (res.isErr())
          return Err(CallstackErr(static_cast<u64>(-1),
                                  CallstackErr::ErrType::Fmt, __LINE__));
      }

      if (!StackWalk64(machineType, process, thread, &frame, context, NULL,
                       SymFunctionTableAccess64, SymGetModuleBase64, NULL))
        break;
    }
  } while (frame.AddrReturn.Offset != 0);
  for (const ModuleInfo& module : modules)
    SymUnloadModule64(process, (DWORD64)module.baseAddr);

  return Ok(Callstack{dc::move(str)});
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

Result<Callstack, CallstackErr> buildCallstack() {
  Result<Callstack, CallstackErr> r = Err(CallstackErr(0, __LINE__));

  constexpr usize fnRetAddrSize = 64;
  void* fnRetAddr[fnRetAddrSize];
  int size = backtrace(reinterpret_cast<void**>(&fnRetAddr),
                       sizeof(fnRetAddr) * fnRetAddrSize);

  std::vector<char> out;
  out.reserve(2048);

  constexpr usize fnBufferSize = 512;
  char* fnBuffer = new char[fnBufferSize];

  // TODO cgustafsson: find file name and line number.
  for (int i = 0; i < size; ++i) {
    Dl_info symInfo;
    int res = dladdr(static_cast<const void*>(fnRetAddr[i]), &symInfo);
    if (res != 0) {
      int status;
      usize fnBufferLen = fnBufferSize;
      fnBuffer[0] = 0;
      char* fnName = abi::__cxa_demangle(symInfo.dli_sname, fnBuffer,
                                         &fnBufferLen, &status);

#if 0
			fmt::format_to(std::back_inserter(out), "fn: {}\nstatus: {}\nfunction return address: {}\nfname: {}\nfbase: {}\nsaddr: {}\noffset: {#x}\n",
						   fnName ? fnName : fnBuffer,
						   status,
						   fnRetAddr[i],
						   symInfo.dli_fname ? symInfo.dli_fname : "null",
						   symInfo.dli_fbase,
						   symInfo.dli_saddr,
						   reinterpret_cast<intptr_t>(fnRetAddr[i]) - reinterpret_cast<intptr_t>(symInfo.dli_saddr));
#else
      if (status == 0 || status == -2)
        fmt::format_to(std::back_inserter(out), "{} in [{}]\n",
                       fnName ? fnName : symInfo.dli_sname,
                       symInfo.dli_fname ? symInfo.dli_fname : "?");
      else
        fmt::format_to(std::back_inserter(out), "{} in [?:?]\n", fnRetAddr[i]);

#endif

    } else {
      LOG_WARNING("failed to dladdr");
      symInfo.dli_fname = nullptr;
      symInfo.dli_sname = nullptr;
      fmt::format_to(std::back_inserter(out), "{#x}", fnRetAddr[i]);
    }

    // fmt::format_to(std::back_inserter(out), "\n");
  }

  delete[] fnBuffer;

  if (!out.empty()) out.resize(out.size() - 1);  //< remove last newline

  return Ok(Callstack(StringView(buffer.data(), buffer.size())));
}

String CallstackErr::toString() const {
  // TODO cgustafsson:
  return "todo";
}

#endif

}  // namespace dc
