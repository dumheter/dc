#include <dc/dtest.hpp>
#include <dc/list.hpp>

using namespace dc;

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
  ASSERT_EQ(v.getCapacity(), 1);

  v.add(20);
  ASSERT_EQ(v.getSize(), 2);
  ASSERT_TRUE(v.getCapacity() > 1);
}

DTEST(remove) {
  List<int> v;

  v.add(10);
  v.add(20);
  v.add(30);

  v.remove(1);
  ASSERT_EQ(v.getSize(), 2);
  ASSERT_EQ(v[1], 30);
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
