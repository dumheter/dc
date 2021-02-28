#include "dtest.hpp"

DTEST(firstTest) {
  int x = 5;
  DTEST_ASSERT(x == 5);

  DTEST_ASSERT(x != 10);
}

DTEST(badTest) {
  int y = 2;
  DTEST_ASSERT(y == 3);
}

int main(int, char**) {
  DTEST_RUN();

  return 0;
}
