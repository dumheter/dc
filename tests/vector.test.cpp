#include <dc/dtest.hpp>
#include <dc/vector.hpp>

DTEST_VIP;

DTEST(canAppend) {
  dc::Vector<int> v(1);
  v.append(1337);
  ASSERT_EQ(v.size(), 1);
  ASSERT_EQ(v.capacity(), 1);
  ASSERT_EQ(v[0], 1337);
}

DTEST(emptyDefaultVectorIsEmpty) {
  dc::Vector<int> v;
  ASSERT_EQ(v.size(), 0);
}

DTEST(growWhenOOM) {
  dc::Vector<float> v(1);

  v.append(10);
  ASSERT_EQ(v.size(), 1);
  ASSERT_EQ(v.capacity(), 1);

  v.append(20);
  ASSERT_EQ(v.size(), 2);
  ASSERT_TRUE(v.capacity() > 1);
}

DTEST(remove) {
  dc::Vector<int> v;

  v.append(10);
  v.append(20);
  v.append(30);

  v.removeAt(1);
  ASSERT_EQ(v.size(), 2);
  ASSERT_EQ(v[1], 30);
}

DTEST(clone) {
  dc::Vector<int> v;

  v.append(10);
  v.append(20);
  v.append(30);

  dc::Vector<int> clone = v.clone();
  ASSERT_EQ(clone[0], 10);
  ASSERT_EQ(clone[1], 20);
  ASSERT_EQ(clone[2], 30);
  ASSERT_EQ(clone.size(), v.size());
}
