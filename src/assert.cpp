/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
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

#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <dc/callstack.hpp>
#include <dc/log.hpp>
#include <dc/platform.hpp>
#include <dc/string.hpp>
#include <dc/traits.hpp>

#if defined(DC_ASSERT_DIALOG)

#if defined(DC_PLATFORM_LINUX)
#include <gtk/gtk.h>
#endif

#endif

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
#include <Windows.h>

#endif

namespace dc::details {

DC_NOINLINE void dcDoAssert(const char* msg, const char* file, const char* func,
                            int line) {
  // TODO cgustafsson: we are allocating in this function which is terrible,
  // reseve static memory, such that asserts can always do something, even when
  // low on memory.

  auto callstackResult = buildCallstack();
  dc::String callstack =
      dc::move(callstackResult)
          .match([](Callstack&& cs) { return move(cs.callstack); },
                 [](CallstackErr&& err) { return err.toString(); });

  if (callstack.getDataAt(callstack.getSize() - 1) ==
      '\n')  //< remove last newline
    callstack.setDataAt(callstack.getSize() - 1, '\0');

  LOG_ERROR(
      "Assertion failed: [{}] in [{}:{} @ {}]. "
      "Callstack:\n{}",
      msg, file, line, func, callstack);

#if defined(DC_ASSERT_DIALOG)
#if defined(DC_PLATFORM_WINDOWS)
  constexpr size_t strlen = 2048;
  wchar_t wstr[strlen];
  memset(wstr, 0, strlen * 2);
  char* p = reinterpret_cast<char*>(wstr);
  for (int i = 0; i < strlen; i++) {
    *(p + i * 2) = fmt[i];
  }
  MessageBoxW(nullptr, (LPCWSTR)wstr, (LPCWSTR)L"Assertion Failed",
              MB_OK | MB_APPLMODAL | MB_ICONERROR);
#elif defined(DC_PLATFORM_LINUX)
  if (!gtk_init_check(0, nullptr)) {
    return;
  }

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget* dialog = gtk_message_dialog_new(
      GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
      GTK_BUTTONS_CANCEL, "%s", fmt.c_str());
  gtk_window_set_title(GTK_WINDOW(dialog), "Assertion Failed");
  gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_destroy(GTK_WIDGET(window));
  gtk_widget_destroy(GTK_WIDGET(dialog));

  while (g_main_context_iteration(nullptr, false)) {  //< Run event loop
  }
#endif
#endif
}

[[noreturn]] DC_NOINLINE void dcDoFatalAssert(const char* msg, const char* file,
                                              const char* func, int line) {
  dcDoAssert(msg, file, func, line);

  debugBreak();
  log::deinit();  //< ensure the log queue + the assert message gets logged.
  exit(1);        //< program ends here
}

void dcAssert(bool condition, const char* msg, const char* file,
              const char* func, int line) {
  if (condition) return;

  dcDoAssert(msg, file, func, line);
}

void dcFatalAssert(bool condition, const char* msg, const char* file,
                   const char* func, int line) {
  if (condition) return;

  dcDoFatalAssert(msg, file, func, line);
}

void debugBreak() {
#if defined(DC_PLATFORM_WINDOWS)
  __try {
    DebugBreak();
  } __except (GetExceptionCode() == EXCEPTION_BREAKPOINT
                  ? EXCEPTION_EXECUTE_HANDLER
                  : EXCEPTION_CONTINUE_SEARCH) {
    // do nothing, just catch the exception
  }
#elif defined(DC_PLATFORM_LINUX)
  asm("int3");  //< software interrupt to set code breakpoint
#else
  *static_cast<volatile int*>(nullptr) = 1;  //< crash
#endif
}

}  // namespace dc::details
