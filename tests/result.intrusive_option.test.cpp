#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

DTEST(optionIsSome) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::Some(1337);
  DASSERT_TRUE(opt.isSome());
  DASSERT_TRUE(opt.value() == 1337);
}

DTEST(optionIsNone) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::None;
  DASSERT_TRUE(opt.isNone());
}

DTEST(optionIsNoneBySomeAssignment) {
  dc::experimental::IntrusiveOption<int, -1> opt = dc::Some(-1);
  DASSERT_TRUE(opt.isNone());
}
