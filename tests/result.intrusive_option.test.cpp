#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

using namespace dc;

DTEST(optionIsSome) {
  experimental::IntrusiveOption<int, -1> opt = Some<int>(1337);
  ASSERT_TRUE(opt.isSome());
  ASSERT_TRUE(opt.value() == 1337);
}

DTEST(optionIsNone) {
  experimental::IntrusiveOption<int, -1> opt = None;
  ASSERT_TRUE(opt.isNone());
}

DTEST(optionIsNoneBySomeAssignment) {
  experimental::IntrusiveOption<int, -1> opt = Some<int>(-1);
  ASSERT_TRUE(opt.isNone());
}
