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

#include <dc/assert.hpp>
#include <dc/callstack.hpp>
#include <dc/core.hpp>
#include <dc/log.hpp>
#include <dc/platform.hpp>
#include <iterator>
#include <string>
#include <vector>

////////////////////////
// Windows
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
////////////////////////
// Linux
#else
// TODO
#endif

namespace dc {

///////////////////////////////////////////////////////////////////////////////
// Windows
//
#if defined(DC_PLATFORM_WINDOWS)

struct ModuleInfo {
  std::string imageName;
  std::string moduleName;
  void* baseAddr;
  DWORD loadSize;

	static Result<ModuleInfo, CallstackErr> create(HANDLE process, HMODULE module);
};

// TODO cgustafsson: This function can only be called from one thread, should
// have a mutex or something to protect.
Result<ModuleInfo, CallstackErr> ModuleInfo::create(HANDLE process, HMODULE module) {
  ModuleInfo out;

  MODULEINFO moduleInfo;
  bool ok =
      GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo));
  if (!ok)
	  return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
  DC_ASSERT(ok, "failed to get module information");
  out.baseAddr = moduleInfo.lpBaseOfDll;
  out.loadSize = moduleInfo.SizeOfImage;

  constexpr int bufSize = 1024;
  std::vector<char> img(bufSize);
  DWORD len = GetModuleFileNameEx(process, module, img.data(),
                                  static_cast<DWORD>(img.size()));
  if (len == 0)
	  return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
  img.resize(len);
  out.imageName = img.data();

  std::vector<char> mod(bufSize);
  len = GetModuleBaseName(process, module, mod.data(),
                          static_cast<DWORD>(mod.size()));
  if (len == 0)
	  return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
  mod.resize(len);
  out.moduleName = mod.data();

  HANDLE file = NULL;               //< no file
  PMODLOAD_DATA headerData = NULL;  //< extra header info, not needed
  DWORD flags = 0;                  //< no extra flags
  DWORD64 okLoad = SymLoadModuleEx(process, file, img.data(), mod.data(),
                  reinterpret_cast<DWORD64>(out.baseAddr), out.loadSize,
                  headerData, flags);
  if (okLoad == 0)
	  return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});

  return Ok(std::move(out));
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
      return Ok(std::move(symbol));
    else
		return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
  }

  Symbol(Symbol&& other) : m_sym(other.m_sym) { other.m_sym = nullptr; }
  Symbol& operator=(Symbol&& other) {
    if (this != &other) std::swap(m_sym, other.m_sym);

    return *this;
  }

  ~Symbol() { delete m_sym; }

  std::string name() const { return std::string(m_sym->Name); }

  Result<std::string, CallstackErr> undecoratedName() const {
    if (*m_sym->Name == 0) return Ok<std::string>("<name not available>");

    std::string name;
    name.resize(kMaxNameLen);
    const DWORD bytesWritten =
        UnDecorateSymbolName(m_sym->Name, &name[0],
                             static_cast<DWORD>(name.size()), UNDNAME_COMPLETE);
    if (bytesWritten == 0)
      return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});

    name.resize(bytesWritten);
    return Ok<std::string>(std::move(name));
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

// DWORD callstack(HANDLE thread, CONTEXT* context);

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

  Result<Callstack, CallstackErr> out =
      SymInitialize(process, NULL, false)
          ? buildCallstackAux(process, thread, &context)
          : Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});

  SymCleanup(process);

  return out;
}

/// Note: Using an aux function to ensure we can return on error, but still do
/// the cleanup before fully returning to the caller.
static Result<Callstack, CallstackErr> buildCallstackAux(HANDLE process,
                                                         HANDLE thread,
                                                         CONTEXT* context) {
  Result<Callstack, CallstackErr> out = Ok(Callstack());

  if (!SymInitialize(process, NULL, false))
    return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});

  DWORD symOptions = SymGetOptions();
  symOptions |= SYMOPT_LOAD_LINES | SYMOPT_UNDNAME;
  SymSetOptions(symOptions);

  std::vector<HMODULE> moduleHandles(1);
  DWORD bytesNeeded;
  bool ok = EnumProcessModules(
      process, moduleHandles.data(),
      static_cast<DWORD>(moduleHandles.size() * sizeof(HMODULE)), &bytesNeeded);
  if (!ok) return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});

  if (bytesNeeded > 0) {
    moduleHandles.resize(bytesNeeded / sizeof(HMODULE));
    ok = EnumProcessModules(
        process, moduleHandles.data(),
        static_cast<DWORD>(moduleHandles.size() * sizeof(HMODULE)),
        &bytesNeeded);
    if (!ok) return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
  }

  std::vector<ModuleInfo> modules;
  for (HMODULE module : moduleHandles)
  {
	  auto res = ModuleInfo::create(process, module);
	  if (res.isOk())
		  modules.push_back(std::move(res).unwrap());
	  else
		  return std::move(res).map([](ModuleInfo&&){ return Callstack();});
  }

  void* base = modules[0].baseAddr;
  IMAGE_NT_HEADERS* imageHeader = ImageNtHeader(base);
  if (!imageHeader) return Err(CallstackErr{static_cast<u64>(GetLastError()), __LINE__});
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

  int n = 0;
  std::vector<char> buffer;
  buffer.reserve(2048);

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(line);
  DWORD offsetFromSymbol = 0;

  do {
	  if (frame.AddrPC.Offset != 0) {
		  Result<Symbol, CallstackErr> symbol =
				  Symbol::create(process, frame.AddrPC.Offset);

		  Result<std::string, CallstackErr> fnName =
				  symbol.match([](const Symbol& symbol) { return symbol.undecoratedName(); },
							   [frame](const CallstackErr&) -> Result<std::string, CallstackErr> { return Ok<std::string>(fmt::format("{:#08x}", frame.AddrPC.Offset)); });

		  fmt::format_to(std::back_inserter(buffer), "{}",
						 fnName.match([](const std::string& s){return s;},
									  [](const CallstackErr&){return std::string("?fn?");})
						 );

		  if (SymGetLineFromAddr64(process, frame.AddrPC.Offset,
								   &offsetFromSymbol, &line))
			  fmt::format_to(std::back_inserter(buffer), " [{}:{}]\n",
							 line.FileName, line.LineNumber);
		  else
			  fmt::format_to(std::back_inserter(buffer), " [-:-]\n");

		  // TODO cgustafsson: how to handle RaiseException
		  if (fnName.isOk() && fnName.value() == "RaiseException") break;

		  if (fnName.isOk() && fnName.value() == "main") break;
	  } else
		  fmt::format_to(std::back_inserter(buffer),
						 "(No symbols: AddrPC.Offset == 0)");

	  if (!StackWalk64(machineType, process, thread, &frame, context, NULL,
					   SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		  break;

	  if (++n > 10)  // TODO cgustafsson: remove this limit?
		  break;

  } while (frame.AddrReturn.Offset != 0);

  Callstack cs;
  cs.setCallstack(buffer.begin(), buffer.end());
  out = Ok(std::move(cs));

  return out;
}

std::string CallstackErr::getErrMessage() const {
  LPVOID lpMsgBuf;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, static_cast<DWORD>(m_err),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0,
                NULL);

  std::string out(static_cast<char*>(lpMsgBuf));
  LocalFree(lpMsgBuf);

  return out;
}

///////////////////////////////////////////////////////////////////////////////
// Linux
//

#else
// TODO
#endif

}  // namespace dc
