#include <dc/dtest.hpp>
#include <dc/list.hpp>

DTEST_VIP;

DTEST(canAdd) {
  dc::List<int> v(1);
  v.add(1337);
  ASSERT_EQ(v.size(), 1);
  ASSERT_EQ(v.capacity(), 1);
  ASSERT_EQ(v[0], 1337);
}

DTEST(emptyDefaultListIsEmpty) {
  dc::List<int> v;
  ASSERT_EQ(v.size(), 0);
}

DTEST(growWhenOOM) {
  dc::List<float> v(1);

  v.add(10);
  ASSERT_EQ(v.size(), 1);
  ASSERT_EQ(v.capacity(), 1);

  v.add(20);
  ASSERT_EQ(v.size(), 2);
  ASSERT_TRUE(v.capacity() > 1);
}

DTEST(remove) {
  dc::List<int> v;

  v.add(10);
  v.add(20);
  v.add(30);

  v.remove(1);
  ASSERT_EQ(v.size(), 2);
  ASSERT_EQ(v[1], 30);
}

DTEST(find) {
  dc::List<int> v;

  v.add(10);
  v.add(11);
  v.add(12);
  v.add(13);
  v.add(14);

  dc::Option<usize> pos = v.find(12);
  ASSERT_TRUE(pos.isSome());
  ASSERT_TRUE(pos.contains(2u));

  v.remove(pos.value());

  dc::Option<usize> none = v.find(12);
  ASSERT_TRUE(none.isNone());
}

DTEST(clone) {
  dc::List<int> v;

  v.add(10);
  v.add(20);
  v.add(30);

  dc::List<int> clone = v.clone();
  ASSERT_EQ(clone[0], 10);
  ASSERT_EQ(clone[1], 20);
  ASSERT_EQ(clone[2], 30);
  ASSERT_EQ(clone.size(), v.size());
}

DTEST(constIterator) {
  dc::List<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 7);
}

DTEST(mutableIterator) {
  dc::List<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  for (int& i : v) i += 1;

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 10);
}
