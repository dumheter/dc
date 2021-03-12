#include "dtest.hpp"

DTEST(copies) {
  dtest::TrackLifetime<int> parent = 13;
  dtest::TrackLifetime<int> child1 = parent;
  dtest::TrackLifetime<int> child2 = child1;
  child1 = child2;
  parent = child1;
  parent = child2;
  DTEST_ASSERT_EQ(parent.getCopies(), 5);
  DTEST_ASSERT_EQ(parent.getMoves(), 0);
  DTEST_ASSERT_EQ(parent.getObject(), 13);
}

DTEST(moves) {
  dtest::TrackLifetime<int> parent = 7;
  dtest::TrackLifetime<int> child1 = std::move(parent);
  dtest::TrackLifetime<int> child2 = std::move(child1);
  child1 = std::move(child2);
  parent = std::move(child1);
  parent = std::move(child2);
  DTEST_ASSERT_EQ(parent.getCopies(), 0);
  DTEST_ASSERT_EQ(parent.getMoves(), 5);
  DTEST_ASSERT_EQ(parent.getObject(), 7);
}
