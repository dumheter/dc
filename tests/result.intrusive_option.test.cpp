#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

DTEST(optionIsSome) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::Some(1337);
  ASSERT_TRUE(opt.isSome());
  ASSERT_TRUE(opt.value() == 1337);
}

DTEST(optionIsNone) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::None;
  ASSERT_TRUE(opt.isNone());
}

DTEST(optionIsNoneBySomeAssignment) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::Some(-1);
  ASSERT_TRUE(opt.isNone());
}
