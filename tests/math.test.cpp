#include <dc/dtest.hpp>
#include <dc/math.hpp>

using namespace dc;

///////////////////////////////////////////////////////////////////////////////
// setBits
//

DTEST(setBitsSetInRange) {
  int value = 0;
  value = setBits(value, 3, 0, 7);

  ASSERT_EQ(value, 7);
}

DTEST(setBitsSetOutOfRange) {
  int value = 0;
  value = setBits(value, 3, 0, 9);

  ASSERT_EQ(value, 1);
}

DTEST(setBitsSetInRangeWithValueKeepingItsValue) {
  int value = 8 + 16;
  value = setBits(value, 3, 0, 7);

  ASSERT_EQ(value, 8 + 16 + 7);
}

DTEST(setBitsSetOutOfRangeWithValueKeepingItsValue) {
  int value = 16;
  value = setBits(value, 3, 0, 9);

  ASSERT_EQ(value, 16 + 1);
}

DTEST(setBitsSetInRangeWithValueKeepingItsValueWithOffset) {
  int value = 3;
  value = setBits(value, 3, 2, 7);

  ASSERT_EQ(value, 3 + (4 + 8 + 16));
}

DTEST(setBitsSetOutOfRangeWithValueKeepingItsValueWithOffset) {
  int value = 3;
  value = setBits(value, 3, 2, 9);

  ASSERT_EQ(value, (3) + (4));
}
