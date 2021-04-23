#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedString = dtest::TrackLifetime<std::string>;

///////////////////////////////////////////////////////////////////////////////
// intrusive
//

dc::IntrusiveOption<int, -1> doStuffOpt(int seed)
{
	if (seed == 1337)
		return dc::Some(1);
	else
		return dc::None;
}

int doStuffInt(int seed)
{
	if (seed == 1337)
		return 1;
	else if (seed == 1)
		return 2;
	else if (seed == 2)
		return 3;
	else if (seed == 3)
		return 4;
	else if (seed == 4)
		return 5;
	else if (seed == 5)
		return 6;
	else if (seed == 6)
		return 7;
	else if (true)
	{
		do {
			for (;;)
				if (true)
					return -1;
		} while (false);
	}
}

DTEST(intrusive) {
	auto res = doStuffOpt(1337);
	DASSERT_TRUE(res.isSome());
	DASSERT_TRUE(res.value() == 123456789);

	auto res2 = doStuffOpt(123);
	DASSERT_TRUE(res2.isNone());

	dc::IntrusiveOption<int, -1> res3 = dc::Some(-1);
	DASSERT_FALSE(res2.isSome());
}

// ========================================================================== //
// LIFETIME
// ========================================================================== //

DTEST(construction) {
  dc::Option<int> simple;
  DASSERT_FALSE(simple);

  dc::Some<int> largeValue(1200300);
  dc::Option<int> someConstruction = std::move(largeValue);
  DASSERT_TRUE(someConstruction);
  DASSERT_EQ(someConstruction.value(), 1200300);

  dc::Option<int> noneConstruction = dc::None;
  DASSERT_FALSE(noneConstruction);
}

DTEST(construction_option) {
  auto optionNone = dc::makeNone<int>();
  dc::Option<int> optionNoneConstructed = std::move(optionNone);
  DASSERT_FALSE(optionNoneConstructed);

  TrackedString catName("Emma");
  auto maybeCatName = dc::makeSome(std::move(catName));
  const int catNameMoves = catName.getMoves();
  auto optionSomeConstructed = std::move(maybeCatName);
  DASSERT_TRUE(optionSomeConstructed);
  DASSERT_EQ(catName.getMoves(), catNameMoves + 1);
  DASSERT_EQ(optionSomeConstructed.value().getObject(), std::string("Emma"));
}

DTEST(assignment_option) {
  {
    auto a = dc::makeSome('a');
    auto b = dc::makeSome('b');
    a = std::move(b);
    DASSERT_TRUE(a);
    DASSERT_EQ(a.value(), 'b');
    DASSERT_TRUE(b);
    DASSERT_EQ(b.value(), 'a');
  }

  {
    TrackedInt original = 77;
    auto optionSome = dc::makeSome<TrackedInt>(std::move(original));
    auto optionNone = dc::makeNone<TrackedInt>();
    const int someMoves = original.getMoves();
    optionNone = std::move(optionSome);
    DASSERT_TRUE(optionNone);
    DASSERT_EQ(original.getMoves(), someMoves + 1);
    DASSERT_FALSE(optionSome);
  }

  {
    TrackedInt original = -2;
    auto optionSome = dc::makeSome<TrackedInt>(std::move(original));
    auto optionNone = dc::makeNone<TrackedInt>();
    const int destructs = original.getDestructs();
    optionSome = std::move(optionNone);
    DASSERT_EQ(original.getDestructs(), destructs + 1);
    DASSERT_FALSE(optionSome);
  }

  {
    auto optionNone = dc::makeNone<int>();
    auto optionNone2 = dc::makeNone<int>();
    optionNone2 = std::move(optionNone);
    DASSERT_FALSE(optionNone2);
  }
}

DTEST(destruction) {
  TrackedString str("wood");
  int destructs;
  {
    const auto option = dc::makeSome(std::move(str));
    DASSERT_TRUE(option);
    destructs = str.getDestructs();
  }
  DASSERT_EQ(str.getDestructs(), destructs + 1);
}

DTEST(clone) {
  TrackedInt original(3);
  dc::Option<TrackedInt> option = dc::Some(std::move(original));
  auto clone = option.clone();
  DASSERT_EQ(original.getCopies(), 1);
  DASSERT_EQ(clone.value(), option.value());
}

DTEST(as_mut_const_ref) {
  TrackedString original(std::string("awesome"));
  auto option = dc::makeSome(std::move(original));
  auto constRef = option.asConstRef();
  auto mutRef = option.asMutRef();
  DASSERT_EQ(original.getCopies(), 0);
  DASSERT_EQ(constRef.value().get().getObject(), std::string("awesome"));

  mutRef.value().get().getObject() = "you are";
  DASSERT_EQ(option.value().getObject(), std::string("you are"));
  DASSERT_EQ(constRef.value().get().getObject(), std::string("you are"));
  DASSERT_EQ(mutRef.value().get().getObject(), std::string("you are"));
  DASSERT_EQ(original.getCopies(), 0);
}

// ========================================================================== //
// ACCESSORS / MODIFIERS
// ========================================================================== //

DTEST(match_rvalue) {
  TrackedInt i(7);
  auto optionSome = dc::makeSome(std::move(i));
  const int moves = i.getMoves();
  int resSome = std::move(optionSome)
                    .match([](TrackedInt v) { return v == 7 ? 1 : -1; },
                           []() { return -100; });
  DASSERT_EQ(resSome, 1);
  DASSERT_EQ(i.getMoves(), moves + 1);
  DASSERT_EQ(i.getCopies(), 0);

  auto optionNone = dc::makeNone<int>();
  int resNone =
      std::move(optionNone).match([](int) { return 10; }, []() { return 11; });
  DASSERT_EQ(resNone, 11);
}

DTEST(match_lvalue) {
  TrackedInt i(7);
  auto optionSome = dc::makeSome(std::move(i));
  const int moves = i.getMoves();
  int resSome = optionSome.match([](TrackedInt& v) { return v == 7 ? 1 : -1; },
                                 []() { return -100; });
  DASSERT_EQ(resSome, 1);
  DASSERT_EQ(i.getMoves(), moves);
  DASSERT_EQ(i.getCopies(), 0);

  auto optionNone = dc::makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  DASSERT_EQ(resNone, 11);
}

DTEST(match_const_lvalue) {
  dtest::NoCopy<int> i(7);
  const auto optionSome = dc::makeSome(std::move(i));
  int resSome = optionSome.match(
      [](const dtest::NoCopy<int>& v) { return v == 7 ? 1 : -1; },
      []() { return -100; });
  DASSERT_EQ(resSome, 1);

  const auto optionNone = dc::makeNone<int>();
  int resNone = optionNone.match([](int) { return 10; }, []() { return 11; });
  DASSERT_EQ(resNone, 11);
}

DTEST(value) {
  TrackedInt original(77);
  auto option = dc::makeSome(std::move(original));
  TrackedInt& value = option.value();
  value.getObject() = -11;
  DASSERT_EQ(original.getCopies(), 0);
  DASSERT_EQ(option.value().getObject(), -11);
}

DTEST(const_value) {
  TrackedInt original(77);
  const auto option = dc::makeSome(std::move(original));
  const TrackedInt& value = option.value();
  DASSERT_EQ(original.getCopies(), 0);
  DASSERT_EQ(value.getObject(), 77);
}

DTEST(unwrap) {
  TrackedInt original(101);
  auto option = dc::makeSome(std::move(original));
  const int moves = original.getMoves();
  TrackedInt value = std::move(option).unwrap();
  DASSERT_EQ(moves + 1, original.getMoves());
  DASSERT_EQ(value.getObject(), 101);
  DASSERT_EQ(original.getCopies(), 0);
}

// ========================================================================== //
// STATE
// ========================================================================== //

DTEST(is_some_none_bool) {
  const auto some = dc::makeSome(7);
  const auto none = dc::makeNone<int>();
  DASSERT_TRUE(some.isSome());
  DASSERT_FALSE(some.isNone());
  DASSERT_TRUE(none.isNone());
  DASSERT_FALSE(none.isSome());
  DASSERT_TRUE(some);
  DASSERT_FALSE(none);
}

DTEST(contains) {
  const auto some = dc::makeSome<char>('c');
  const auto none = dc::makeNone<char>();
  DASSERT_TRUE(some.contains('c'));
  DASSERT_FALSE(some.contains('w'));
  DASSERT_FALSE(none.contains('c'));
}

DTEST(compare) {
  const auto now = dc::makeSome<int>(2021);
  const auto then = dc::makeSome<int>(1969);
  const auto nowClone = now.clone();
  DASSERT_FALSE(now == then);
  DASSERT_TRUE(now != then);
  DASSERT_TRUE(now == nowClone);
  DASSERT_FALSE(now != nowClone);
}
