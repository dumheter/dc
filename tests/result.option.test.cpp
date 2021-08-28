#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <dc/traits.hpp>
#include <string>

using namespace dc;
using namespace dtest;

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

DTEST(constructionOptionNone) {
  Option<int> optionNone = None;
  Option<int> optionNoneConstructed = move(optionNone);
  ASSERT_FALSE(optionNoneConstructed);
}

DTEST(constructionOptionSome) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<String> catName("Emma");
  Option<LifetimeTracker<String>> maybeCatName = Some(dc::move(catName));
  const int catNameMoves = stats.moves;
  Option<LifetimeTracker<String>> optionSomeConstructed =
      dc::move(maybeCatName);

  ASSERT_TRUE(optionSomeConstructed);
  ASSERT_EQ(stats.moves, catNameMoves + 1);
  ASSERT_EQ(optionSomeConstructed.value().object, dc::String("Emma"));
}

DTEST(assignmentOption) {
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
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<int> original = 77;
    Option<LifetimeTracker<int>> optionSome = Some(dc::move(original));
    Option<LifetimeTracker<int>> optionNone = None;
    const int someMoves = stats.moves;
    optionNone = dc::move(optionSome);

    ASSERT_TRUE(optionNone);
    ASSERT_EQ(stats.moves, someMoves + 1);
    ASSERT_FALSE(optionSome);
  }

  {
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<int> original = -2;
    Option<LifetimeTracker<int>> optionSome = Some(move(original));
    Option<LifetimeTracker<int>> optionNone = None;
    const int destructs = stats.destructs;
    optionSome = move(optionNone);

    ASSERT_EQ(stats.destructs, destructs + 1);
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
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<std::string> str("wood");
  int destructs;
  {
    const Option<LifetimeTracker<std::string>> option = Some(dc::move(str));
    ASSERT_TRUE(option);
    destructs = stats.destructs;
  }
  ASSERT_EQ(stats.destructs, destructs + 1);
}

DTEST(clone) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original(3);
  Option<LifetimeTracker<int>> option = Some(move(original));
  auto clone = option.clone();

  ASSERT_EQ(stats.copies, 1);
  ASSERT_EQ(clone.value(), option.value());
}

DTEST(asMutConstRef) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<String> original(String("awesome"));
  auto option = makeSome(dc::move(original));
  auto constRef = option.asConstRef();
  auto mutRef = option.asMutRef();
  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(constRef.value().get().object, String("awesome"));

  mutRef.value().get().object = "you are";
  ASSERT_EQ(option.value().object, String("you are"));
  ASSERT_EQ(constRef.value().get().object, dc::String("you are"));
  ASSERT_EQ(mutRef.value().get().object, dc::String("you are"));
  ASSERT_EQ(stats.copies, 0);
}

///////////////////////////////////////////////////////////////////////////////
// ACCESSORS / MODIFIERS
//

DTEST(matchRValue) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> i(7);
  Option<LifetimeTracker<int>> optionSome = Some(move(i));
  const int moves = stats.moves;
  int resSome =
      dc::move(optionSome)
          .match([](LifetimeTracker<int> v) -> int { return v == 7 ? 1 : -1; },
                 []() -> int { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(stats.moves, moves + 1);
  ASSERT_EQ(stats.copies, 0);

  auto optionNone = makeNone<int>();
  int resNone =
      dc::move(optionNone).match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(matchLValue) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> i(7);
  auto optionSome = makeSome(dc::move(i));
  const int moves = stats.moves;
  int resSome =
      optionSome.match([](LifetimeTracker<int>& v) { return v == 7 ? 1 : -1; },
                       []() { return -100; });
  ASSERT_EQ(resSome, 1);
  ASSERT_EQ(stats.moves, moves);
  ASSERT_EQ(stats.copies, 0);

  auto optionNone = makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  ASSERT_EQ(resNone, 11);
}

DTEST(matchConstLValue) {
  NoCopy<int> i(7);
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
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original(77);
  auto option = makeSome(dc::move(original));
  LifetimeTracker<int>& value = option.value();
  value.object = -11;

  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(option.value().object, -11);
}

DTEST(constValue) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original(77);
  const auto option = makeSome(dc::move(original));
  const LifetimeTracker<int>& value = option.value();

  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(value.object, 77);
}

DTEST(unwrap) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original(101);
  auto option = makeSome(dc::move(original));
  const int moves = stats.moves;
  LifetimeTracker<int> value = dc::move(option).unwrap();

  ASSERT_EQ(moves + 1, stats.moves);
  ASSERT_EQ(value.object, 101);
  ASSERT_EQ(stats.copies, 0);
}

///////////////////////////////////////////////////////////////////////////////
// STATE
//

DTEST(isSomeNoneBool) {
  const Option<int> some = Some(7);
  const Option<int> none = None;
  ASSERT_TRUE(some.isSome());
  ASSERT_FALSE(some.isNone());
  ASSERT_TRUE(none.isNone());
  ASSERT_FALSE(none.isSome());
  ASSERT_TRUE(some);
  ASSERT_FALSE(none);
}

DTEST(contains) {
  const Option<char> some = Some('c');
  const Option<char> none = None;
  ASSERT_TRUE(some.contains('c'));
  ASSERT_FALSE(some.contains('w'));
  ASSERT_FALSE(none.contains('c'));
}

DTEST(compare) {
  const Option<int> now = Some(2021);
  const Option<int> then = Some(1969);
  const auto nowClone = now.clone();
  ASSERT_FALSE(now == then);
  ASSERT_TRUE(now != then);
  ASSERT_TRUE(now == nowClone);
  ASSERT_FALSE(now != nowClone);
}
