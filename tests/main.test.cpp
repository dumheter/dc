#include <elfutils/libdw.h>

#include <dc/dtest.hpp>
#include <dc/fmt.hpp>

#include "dc/assert.hpp"
#include "dc/callstack.hpp"

void abc() {
  auto result = dc::buildCallstack();
  // DC_ASSERT(result.isErr(), "my assert message");
  DC_FATAL_ASSERT(result.isOk(), "my fatal assert message");
  LOG_INFO("{}", result->callstack);
}

// int main(int argc, char** argv) { return DTEST_RUN(); }
int main(int, char**) {
  dc::log::init();

  abc();

  dc::log::deinit();
  return 0;
}
