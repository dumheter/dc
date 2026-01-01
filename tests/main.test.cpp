#include <dc/dtest.hpp>
#include <dc/fmt.hpp>
#include "dc/assert.hpp"
#include "dc/callstack.hpp"

void abc()
{
  auto result = dc::buildCallstack();
  //DC_ASSERT(result.isErr(), "my assert message");
  DC_FATAL_ASSERT(result.isOk(), "my fatal assert message");
  LOG_INFO("{}", result->callstack);
}

// Temporary have disabled the actual test and instead focusing on fixing issue with callstack.
#if 0
int main(int argc, char** argv) { return DTEST_RUN(); }
#else
int main(int, char**)
{
  dc::log::init();
  LOG_INFO("<Callstack debugging>");
  abc();
  LOG_INFO("</Callstack debugging>");
  dc::log::deinit();
  return 0;
}
#endif
