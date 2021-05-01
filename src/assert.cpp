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
#include <dc/core.hpp>
#include <dc/log.hpp>
#include <dc/platform.hpp>
#include <string>

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

static void dcDoAssert(const char* msg, const char* file, const char* func,
                       int line) {
  // TODO cgustafsson: we are allocating in this function, maybe reseve static
  // memory, such that asserts can always do something, even when low on memory.

  auto callstackResult = buildCallstack();
  std::string callstack =
      std::move(callstackResult)
          .match([](Callstack&& cs) { return cs.toString(); },
                 [](CallstackErr&& err) { return err.toString(); });

  if (callstack[callstack.size() - 1] == '\n')  //< remove last newline
    callstack[callstack.size() - 1] = '\0';

  LOG_ERROR(
      "assertion failed with msg: [{}] @ [{}:{}] in fuction [{}], "
      "callstack:\n{}",
      msg, file, line, func, callstack.c_str());

#if defined(DC_ASSERT_DIALOG)
  constexpr size_t strlen = 2048;
  char str[strlen];
  snprintf(str, strlen,
           "assertion failed with msg: [%s] @ [%s:%d] in fuction [%s], callstack:\n%s\n", msg,
           file, line, func, callstack.c_str();
#if defined(DC_PLATFORM_WINDOWS)
  wchar_t wstr[strlen];
  memset(wstr, 0, strlen * 2);
  char* p = reinterpret_cast<char*>(wstr);
  for (int i = 0; i < strlen; i++) {
    *(p + i * 2) = str[i];
  }
  MessageBoxW(nullptr, (LPCWSTR)wstr, (LPCWSTR)L"Assertion Failed",
              MB_OK | MB_APPLMODAL | MB_ICONERROR);
#elif defined(DC_PLATFORM_LINUX)
  if (!gtk_init_check(0, nullptr)) {
    return;
  }

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget* dialog =
      gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL,
                             GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL, "%s", str);
  gtk_window_set_title(GTK_WINDOW(dialog), "Assertion Failed");
  gtk_dialog_run(GTK_DIALOG(dialog));

  gtk_widget_destroy(GTK_WIDGET(window));
  gtk_widget_destroy(GTK_WIDGET(dialog));

  while (g_main_context_iteration(nullptr, false)) { //< Run event loop
  }
#endif
#endif
}

[[noreturn]] DC_NOINLINE static void dcDoFatalAssert(const char* msg,
                                                     const char* file,
                                                     const char* func,
                                                     int line) {
  dcDoAssert(msg, file, func, line);

#if defined(DC_PLATFORM_WINDOWS)
  DebugBreak();
#elif defined(DC_PLATFORM_LINUX)
  asm("int3");  //< software interrupt to set code breakpoint
#else
  *static_cast<volatile int*>(nullptr) = 1;  //< crash
#endif
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

}  // namespace dc::details
