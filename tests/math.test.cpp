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

///////////////////////////////////////////////////////////////////////////////
// roundUpToPowerOf2
//

DTEST(roundUpToPowerOf2Zero) {
  const u32 result = roundUpToPowerOf2(0);
  ASSERT_EQ(result, 0u);
}

DTEST(roundUpToPowerOf2One) {
  const u32 result = roundUpToPowerOf2(1);
  ASSERT_EQ(result, 1u);
}

DTEST(roundUpToPowerOf2Two) {
  const u32 result = roundUpToPowerOf2(2);
  ASSERT_EQ(result, 2u);
}

DTEST(roundUpToPowerOf2Three) {
  const u32 result = roundUpToPowerOf2(3);
  ASSERT_EQ(result, 4u);
}

DTEST(roundUpToPowerOf2Five) {
  const u32 result = roundUpToPowerOf2(5);
  ASSERT_EQ(result, 8u);
}

DTEST(roundUpToPowerOf2AlreadyPowerOf2) {
  const u32 result = roundUpToPowerOf2(16);
  ASSERT_EQ(result, 16u);
}

DTEST(roundUpToPowerOf2Seventeen) {
  const u32 result = roundUpToPowerOf2(17);
  ASSERT_EQ(result, 32u);
}

DTEST(roundUpToPowerOf2LargeValue) {
  const u32 result = roundUpToPowerOf2(1000000);
  ASSERT_EQ(result, 1048576u);  // 2^20
}

DTEST(roundUpToPowerOf2MaxValidValue) {
  const u32 result = roundUpToPowerOf2(0x80000000);  // 2^31
  ASSERT_EQ(result, 0x80000000u);
}
