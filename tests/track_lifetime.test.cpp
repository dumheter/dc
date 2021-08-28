#include <dc/dtest.hpp>

using namespace dtest;

DTEST(copies) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;
  LifetimeTracker<int> child1 = parent;
  LifetimeTracker<int> child2 = child1;
  child1 = child2;
  parent = child1;
  parent = child2;

  ASSERT_EQ(stats.copies, 5);
  ASSERT_EQ(stats.moves, 0);
  ASSERT_EQ(parent.object, 13);
}

DTEST(moves) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 7;
  LifetimeTracker<int> child1 = dc::move(parent);
  LifetimeTracker<int> child2 = dc::move(child1);

  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(stats.moves, 2);
  ASSERT_EQ(child2.object, 7);
}
