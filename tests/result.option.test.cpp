#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedString = dtest::TrackLifetime<std::string>;

///////////////////////////////////////////////////////////////////////////////
// LIFETIME
//

DTEST(construction) {
  dc::Option<int> simple;
  ASSERT_FALSE(simple);

  dc::Some<int> largeValue(1200300);
  dc::Option<int> someConstruction = std::move(largeValue);
  ASSERT_TRUE(someConstruction);
  ASSERT_EQ(someConstruction.value(), 1200300);

  dc::Option<int> noneConstruction = dc::None;
  ASSERT_FALSE(noneConstruction);
}

DTEST(construction_option) {
  auto optionNone = dc::makeNone<int>();
  dc::Option<int> optionNoneConstructed = std::move(optionNone);
  ASSERT_FALSE(optionNoneConstructed);

  TrackedString catName("Emma");
  auto maybeCatName = dc::makeSome(std::move(catName));
  const int catNameMoves = catName.getMoves();
  auto optionSomeConstructed = std::move(maybeCatName);
  ASSERT_TRUE(optionSomeConstructed);
  ASSERT_EQ(catName.getMoves(), catNameMoves + 1);
  ASSERT_EQ(optionSomeConstructed.value().getObject(), std::string("Emma"));
}

DTEST(assignment_option) {
  {
    auto a = dc::makeSome('a');
    auto b = dc::makeSome('b');
    a = std::move(b);
    ASSERT_TRUE(a);
    ASSERT_EQ(a.value(), 'b');
    ASSERT_TRUE(b);
    ASSERT_EQ(b.value(), 'a');
  }

  {
    TrackedInt original = 77;
    auto optionSome = dc::makeSome<TrackedInt>(std::move(original));
    auto optionNone = dc::makeNone<TrackedInt>();
    const int someMoves = original.getMoves();
    optionNone = std::move(optionSome);
    ASSERT_TRUE(optionNone);
    ASSERT_EQ(original.getMoves(), someMoves + 1);
    ASSERT_FALSE(optionSome);
  }

  {
    TrackedInt original = -2;
    auto optionSome = dc::makeSome<TrackedInt>(std::move(original));
    auto optionNone = dc::makeNone<TrackedInt>();
    const int destructs = original.getDestructs();
    optionSome = std::move(optionNone);
    ASSERT_EQ(original.getDestructs(), destructs + 1);
    ASSERT_FALSE(optionSome);
  }

  {
    auto optionNone = dc::makeNone<int>();
    auto optionNone2 = dc::makeNone<int>();
    optionNone2 = std::move(optionNone);
    ASSERT_FALSE(optionNone2);
  }
}

DTEST(destruction) {
  TrackedString str("wood");
  int destructs;
  {
    const auto option = dc::makeSome(std::move(str));
    ASSERT_TRUE(option);
    destructs = str.getDestructs();
  }
  ASSERT_EQ(str.getDestructs(), destructs + 1);
}

DTEST(clone) {
  TrackedInt original(3);
  dc::Option<TrackedInt> option = dc::Some(std::move(original));
  auto clone = option.clone();
  ASSERT_EQ(original.getCopies(), 1);
  ASSERT_EQ(clone.value(), option.value());
}

DTEST(as_mut_const_ref) {
  TrackedString original(std::string("awesome"));
  auto option = dc::makeSome(std::move(original));
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
  auto optionSome = dc::makeSome(std::move(i));
  const int moves = i.getMoves();
  int resSome = std::move(optionSome)
                    .match([](TrackedInt v) { return v == 7 ? 1 : -1; },
                           []() { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(i.getMoves(), moves + 1);
  ASSERT_EQ(i.getCopies(), 0);

  auto optionNone = dc::makeNone<int>();
  int resNone =
      std::move(optionNone).match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(match_lvalue) {
  TrackedInt i(7);
  auto optionSome = dc::makeSome(std::move(i));
  const int moves = i.getMoves();
  int resSome = optionSome.match([](TrackedInt& v) { return v == 7 ? 1 : -1; },
                                 []() { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(i.getMoves(), moves);
  ASSERT_EQ(i.getCopies(), 0);

  auto optionNone = dc::makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(match_const_lvalue) {
  dtest::NoCopy<int> i(7);
  const auto optionSome = dc::makeSome(std::move(i));
  int resSome = optionSome.match(
      [](const dtest::NoCopy<int>& v) { return v == 7 ? 1 : -1; },
      []() { return -100; });
  ASSERT_EQ(resSome, 1);

  const auto optionNone = dc::makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(value) {
  TrackedInt original(77);
  auto option = dc::makeSome(std::move(original));
  TrackedInt& value = option.value();
  value.getObject() = -11;
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(option.value().getObject(), -11);
}

DTEST(const_value) {
  TrackedInt original(77);
  const auto option = dc::makeSome(std::move(original));
  const TrackedInt& value = option.value();
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(value.getObject(), 77);
}

DTEST(unwrap) {
  TrackedInt original(101);
  auto option = dc::makeSome(std::move(original));
  const int moves = original.getMoves();
  TrackedInt value = std::move(option).unwrap();
  ASSERT_EQ(moves + 1, original.getMoves());
  ASSERT_EQ(value.getObject(), 101);
  ASSERT_EQ(original.getCopies(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// STATE
//

DTEST(is_some_none_bool) {
  const auto some = dc::makeSome(7);
  const auto none = dc::makeNone<int>();
  ASSERT_TRUE(some.isSome());
  ASSERT_FALSE(some.isNone());
  ASSERT_TRUE(none.isNone());
  ASSERT_FALSE(none.isSome());
  ASSERT_TRUE(some);
  ASSERT_FALSE(none);
}

DTEST(contains) {
  const auto some = dc::makeSome<char>('c');
  const auto none = dc::makeNone<char>();
  ASSERT_TRUE(some.contains('c'));
  ASSERT_FALSE(some.contains('w'));
  ASSERT_FALSE(none.contains('c'));
}

DTEST(compare) {
  const auto now = dc::makeSome<int>(2021);
  const auto then = dc::makeSome<int>(1969);
  const auto nowClone = now.clone();
  ASSERT_FALSE(now == then);
  ASSERT_TRUE(now != then);
  ASSERT_TRUE(now == nowClone);
  ASSERT_FALSE(now != nowClone);
}
