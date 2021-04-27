#include <dc/callstack.hpp>
#include <dc/dtest.hpp>
#include <dc/log.hpp>

DTEST(callstackFromFunctionCall) {
  dc::Result<dc::Callstack, dc::CallstackErr> callstack = dc::buildCallstack();
  if (callstack.isErr())
    LOG_INFO("Error in callstack.cpp:{} with msg: {}",
             callstack.errValue().getLine(),
             callstack.errValue().getErrMessage());
  DASSERT_TRUE(callstack.isOk());
  // LOG_RAW("{}", callstack.value().toString());
}

// TODO cgustafsson: be able to trap exceptions
// DTEST(callstackFromException)
// {
// 	__try {
// 		int a = 0;
// 		a = 1 / a;
// 	} __except (dc::callstackFromException(GetExceptionInformatio(n)));
// 	  {
// 		  LOG_INFO("exception happened");
// 	  }

// 	DASSERT_TRUE(true);
// }
