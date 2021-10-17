#include <dc/dtest.hpp>
#include <dc/fmt.hpp>

int main(int, char**) {
  dc::format("{}", 13.07f);
  dc::format("{}", 13.37f);
  dc::format("{}", 0.37f);
  return DTEST_RUN();
}
