#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <dc/traits.hpp>
#include <string>

using namespace dc;

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedString = dtest::TrackLifetime<std::string>;

///////////////////////////////////////////////////////////////////////////////
// LIFETIME
//

DTEST(construction) {
  Option<int> simple;
  ASSERT_FALSE(simple);

  Some<int> largeValue(1200300);
  Option<int> someConstruction = move(largeValue);
  ASSERT_TRUE(someConstruction);
  ASSERT_EQ(someConstruction.value(), 1200300);

  Option<int> noneConstruction = None;
  ASSERT_FALSE(noneConstruction);
}

DTEST(construction_option) {
  Option<int> optionNone = None;
  Option<int> optionNoneConstructed = move(optionNone);
  ASSERT_FALSE(optionNoneConstructed);

  TrackedString catName("Emma");
  Option<TrackedString> maybeCatName = Some(dc::move(catName));
  const int catNameMoves = catName.getMoves();
  Option<TrackedString> optionSomeConstructed = dc::move(maybeCatName);
  ASSERT_TRUE(optionSomeConstructed);
  ASSERT_EQ(catName.getMoves(), catNameMoves + 1);
  ASSERT_EQ(optionSomeConstructed.value().getObject(), std::string("Emma"));
}

DTEST(assignment_option) {
  {
    Option<char> a = Some('a');
    Option<char> b = Some('b');
    a = move(b);
    ASSERT_TRUE(a);
    ASSERT_EQ(a.value(), 'b');
    ASSERT_TRUE(b);
    ASSERT_EQ(b.value(), 'a');
  }

  {
    TrackedInt original = 77;
    Option<TrackedInt> optionSome = Some(dc::move(original));
    Option<TrackedInt> optionNone = None;
    const int someMoves = original.getMoves();
    optionNone = move(optionSome);
    ASSERT_TRUE(optionNone);
    ASSERT_EQ(original.getMoves(), someMoves + 1);
    ASSERT_FALSE(optionSome);
  }

  {
    TrackedInt original = -2;
    Option<TrackedInt> optionSome = Some(move(original));
    Option<TrackedInt> optionNone = None;
    const int destructs = original.getDestructs();
    optionSome = move(optionNone);
    ASSERT_EQ(original.getDestructs(), destructs + 1);
    ASSERT_FALSE(optionSome);
  }

  {
    Option<int> optionNone = None;
    Option<int> optionNone2 = None;
    optionNone2 = move(optionNone);
    ASSERT_FALSE(optionNone2);
  }
}

DTEST(destruction) {
  TrackedString str("wood");
  int destructs;
  {
    const Option<TrackedString> option = Some(dc::move(str));
    ASSERT_TRUE(option);
    destructs = str.getDestructs();
  }
  ASSERT_EQ(str.getDestructs(), destructs + 1);
}

DTEST(clone) {
  TrackedInt original(3);
  Option<TrackedInt> option = Some(move(original));
  auto clone = option.clone();
  ASSERT_EQ(original.getCopies(), 1);
  ASSERT_EQ(clone.value(), option.value());
}

DTEST(as_mut_const_ref) {
  TrackedString original(std::string("awesome"));
  auto option = makeSome(dc::move(original));
  auto constRef = option.asConstRef();
  auto mutRef = option.asMutRef();
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(constRef.value().get().getObject(), std::string("awesome"));

  mutRef.value().get().getObject() = "you are";
  ASSERT_EQ(option.value().getObject(), std::string("you are"));
  ASSERT_EQ(constRef.value().get().getObject(), std::string("you are"));
  ASSERT_EQ(mutRef.value().get().getObject(), std::string("you are"));
  ASSERT_EQ(original.getCopies(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// ACCESSORS / MODIFIERS
//

DTEST(match_rvalue) {
  TrackedInt i(7);
  Option<TrackedInt> optionSome = Some(move(i));
  const int moves = i.getMoves();
  int resSome = dc::move(optionSome)
                    .match([](TrackedInt v) -> int { return v == 7 ? 1 : -1; },
                           []() -> int { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(i.getMoves(), moves + 1);
  ASSERT_EQ(i.getCopies(), 0);

  auto optionNone = makeNone<int>();
  int resNone =
      dc::move(optionNone).match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(match_lvalue) {
  TrackedInt i(7);
  auto optionSome = makeSome(dc::move(i));
  const int moves = i.getMoves();
  int resSome = optionSome.match([](TrackedInt& v) { return v == 7 ? 1 : -1; },
                                 []() { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(i.getMoves(), moves);
  ASSERT_EQ(i.getCopies(), 0);

  auto optionNone = makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(match_const_lvalue) {
  dtest::NoCopy<int> i(7);
  const auto optionSome = makeSome(dc::move(i));
  int resSome = optionSome.match(
      [](const dtest::NoCopy<int>& v) { return v == 7 ? 1 : -1; },
      []() { return -100; });
  ASSERT_EQ(resSome, 1);

  const auto optionNone = makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(value) {
  TrackedInt original(77);
  auto option = makeSome(dc::move(original));
  TrackedInt& value = option.value();
  value.getObject() = -11;
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(option.value().getObject(), -11);
}

DTEST(const_value) {
  TrackedInt original(77);
  const auto option = makeSome(dc::move(original));
  const TrackedInt& value = option.value();
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(value.getObject(), 77);
}

DTEST(unwrap) {
  TrackedInt original(101);
  auto option = makeSome(dc::move(original));
  const int moves = original.getMoves();
  TrackedInt value = dc::move(option).unwrap();
  ASSERT_EQ(moves + 1, original.getMoves());
  ASSERT_EQ(value.getObject(), 101);
  ASSERT_EQ(original.getCopies(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// STATE
//

DTEST(is_some_none_bool) {
  const auto some = makeSome(7);
  const auto none = makeNone<int>();
  ASSERT_TRUE(some.isSome());
  ASSERT_FALSE(some.isNone());
  ASSERT_TRUE(none.isNone());
  ASSERT_FALSE(none.isSome());
  ASSERT_TRUE(some);
  ASSERT_FALSE(none);
}

DTEST(contains) {
  const auto some = makeSome<char>('c');
  const auto none = makeNone<char>();
  ASSERT_TRUE(some.contains('c'));
  ASSERT_FALSE(some.contains('w'));
  ASSERT_FALSE(none.contains('c'));
}

DTEST(compare) {
  const auto now = makeSome<int>(2021);
  const auto then = makeSome<int>(1969);
  const auto nowClone = now.clone();
  ASSERT_FALSE(now == then);
  ASSERT_TRUE(now != then);
  ASSERT_TRUE(now == nowClone);
  ASSERT_FALSE(now != nowClone);
}
