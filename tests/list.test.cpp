#include <dc/dtest.hpp>
#include <dc/list.hpp>

using namespace dc;
using namespace dtest;

DTEST(canAdd) {
  List<int> v;

  v.add(1337);

  ASSERT_EQ(v.getSize(), 1);
  ASSERT_TRUE(v.getCapacity() > 0);
  ASSERT_EQ(v[0], 1337);
}

DTEST(emptyDefaultListIsEmpty) {
  List<int> v;
  ASSERT_EQ(v.getSize(), 0);
}

DTEST(growWhenOOM) {
  List<float> v(1);

  v.add(10);
  ASSERT_EQ(v.getSize(), 1);

  v.add(20);
  ASSERT_EQ(v.getSize(), 2);
}

DTEST(removeByPosition) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int>> list;

  list.add(10);
  list.add(20);
  list.add(30);

  const int destructsBefore = stats.destructs;
  list.removeAt(1);

  ASSERT_EQ(list.getSize(), 2);
  ASSERT_EQ(list[1], 30);
  ASSERT_EQ(stats.destructs, destructsBefore + 1);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(removeByIterator) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int*>> list;

  int abc[3];
  abc[0] = 10;
  abc[1] = 20;
  abc[2] = 30;

  list.add(&abc[0]);
  list.add(&abc[1]);
  list.add(&abc[2]);

  const int destructsBefore = stats.destructs;
  list.remove(&list[1]);
  list.remove(&list[1]);
  list.remove(&list[0]);

  ASSERT_EQ(list.getSize(), 0);
  ASSERT_EQ(stats.copies, 0);
  // ASSERT_EQ(stats.moves, 0);
  ASSERT_EQ(stats.destructs, destructsBefore + 3);
}

DTEST(removeByReference) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();
  List<LifetimeTracker<int*>> list;

  int abc[3];
  abc[0] = 10;
  abc[1] = 20;
  abc[2] = 30;

  list.add(&abc[0]);
  list.add(&abc[1]);
  list.add(&abc[2]);

  const int destructsBefore = stats.destructs;
  list.remove(&abc[1]);
  list.remove(&abc[0]);
  list.remove(&abc[2]);

  // double remove is noop
  const LifetimeTracker<int*> lookup(&abc[1]);
  list.remove(lookup);

  ASSERT_EQ(list.getSize(), 0);
  ASSERT_EQ(stats.copies, 0);
  // ASSERT_EQ(stats.moves, 0);
  ASSERT_EQ(stats.destructs, destructsBefore + 3);
}

DTEST(find) {
  List<int> v;

  v.add(10);
  v.add(11);
  v.add(12);
  v.add(13);
  v.add(14);

  int* iter = v.find(12);
  ASSERT_TRUE(iter != v.end());
  ASSERT_TRUE(*iter == 12);

  v.remove(iter);

  iter = v.find(12);
  ASSERT_TRUE(iter == v.end());
}

DTEST(clone) {
  List<int> v;

  v.add(10);
  v.add(20);
  v.add(30);

  List<int> clone = v.clone();
  ASSERT_EQ(clone[0], 10);
  ASSERT_EQ(clone[1], 20);
  ASSERT_EQ(clone[2], 30);
  ASSERT_EQ(clone.getSize(), v.getSize());
}

DTEST(constIterator) {
  List<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 7);
}

DTEST(mutableIterator) {
  List<int> v;

  v.add(1);
  v.add(2);
  v.add(4);

  for (int& i : v) i += 1;

  int sum = 0;
  for (const int& i : v) sum += i;

  ASSERT_EQ(sum, 10);
}
