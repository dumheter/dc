#include <dutil/result.hpp>
#include <string>

#include "dtest.hpp"

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedString = dtest::TrackLifetime<std::string>;

// ========================================================================== //
// LIFETIME
// ========================================================================== //

DTEST(construction)
{
	dutil::Option<int> simple;
	DASSERT_FALSE(simple);

	dutil::Some<int> largeValue(1200300);
	dutil::Option<int> someConstruction = std::move(largeValue);
	DASSERT_TRUE(someConstruction);
	DASSERT_EQ(someConstruction.value(), 1200300);

	dutil::Option<int> noneConstruction = dutil::None;
	DASSERT_FALSE(noneConstruction);
}

DTEST(construction_option)
{
	auto optionNone = dutil::makeNone<int>();
	dutil::Option<int> optionNoneConstructed = std::move(optionNone);
	DASSERT_FALSE(optionNoneConstructed);

	TrackedString catName("Emma");
	auto maybeCatName = dutil::makeSome(std::move(catName));
	const int catNameMoves = catName.getMoves();
	auto optionSomeConstructed = std::move(maybeCatName);
	DASSERT_TRUE(optionSomeConstructed);
	DASSERT_EQ(catName.getMoves(), catNameMoves + 1);
	DASSERT_EQ(optionSomeConstructed.value().getObject(), std::string("Emma"));
}

DTEST(assignment_option)
{
	{
		auto a = dutil::makeSome('a');
		auto b = dutil::makeSome('b');
		a = std::move(b);
		DASSERT_TRUE(a);
		DASSERT_EQ(a.value(), 'b');
		DASSERT_TRUE(b);
		DASSERT_EQ(b.value(), 'a');
	}
	
	{
		TrackedInt original = 77;
		auto optionSome = dutil::makeSome<TrackedInt>(std::move(original));
		auto optionNone = dutil::makeNone<TrackedInt>();
		const int someMoves = original.getMoves();
		optionNone = std::move(optionSome);
		DASSERT_TRUE(optionNone);
		DASSERT_EQ(original.getMoves(), someMoves + 1);
		DASSERT_FALSE(optionSome);
	}

	{
		TrackedInt original = -2;
		auto optionSome = dutil::makeSome<TrackedInt>(std::move(original));
		auto optionNone = dutil::makeNone<TrackedInt>();
		const int destructs = original.getDestructs();
		optionSome = std::move(optionNone);
		DASSERT_EQ(original.getDestructs(), destructs + 1);
		DASSERT_FALSE(optionSome);
	}

	{
		auto optionNone = dutil::makeNone<int>();
		auto optionNone2 = dutil::makeNone<int>();
		optionNone2 = std::move(optionNone);
		DASSERT_FALSE(optionNone2);
	}
}

DTEST(destruction)
{
	TrackedString str("wood");
	int destructs;
	{
		const auto option = dutil::makeSome(std::move(str));
		DASSERT_TRUE(option);
		destructs = str.getDestructs();
	}
	DASSERT_EQ(str.getDestructs(), destructs + 1);
}

DTEST(clone)
{
	TrackedInt original(3);
	dutil::Option<TrackedInt> option = dutil::Some(std::move(original));
	auto clone = option.clone();
	DASSERT_EQ(original.getCopies(), 1);
	DASSERT_EQ(clone.value(), option.value());
}

DTEST(as_mut_const_ref)
{
	TrackedString original(std::string("awesome"));
	auto option = dutil::makeSome(std::move(original));
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
// ACCESSORS
// ========================================================================== //

DTEST(value)
{
	TrackedInt original(77);
	auto option = dutil::makeSome(std::move(original));
	TrackedInt& value = option.value();
	value.getObject() = -11;
	DASSERT_EQ(original.getCopies(), 0);
	DASSERT_EQ(option.value().getObject(), -11);
}

DTEST(const_value)
{
	TrackedInt original(77);
	const auto option = dutil::makeSome(std::move(original));
	const TrackedInt& value = option.value();
	DASSERT_EQ(original.getCopies(), 0);
	DASSERT_EQ(value.getObject(), 77);
}

DTEST(unwrap)
{
	TrackedInt original(101);
	auto option = dutil::makeSome(std::move(original));
	const int moves = original.getMoves();
	TrackedInt value = std::move(option).unwrap();
	DASSERT_NE(moves, original.getMoves());
	DASSERT_EQ(value.getObject(), 101);
	DASSERT_EQ(original.getCopies(), 0);
}

// ========================================================================== //
// STATE
// ========================================================================== //

DTEST(is_some_none_bool)
{
	const auto some = dutil::makeSome(7);
	const auto none = dutil::makeNone<int>();
	DASSERT_TRUE(some.isSome());
	DASSERT_FALSE(some.isNone());
	DASSERT_TRUE(none.isNone());
	DASSERT_FALSE(none.isSome());
	DASSERT_TRUE(some);
	DASSERT_FALSE(none);
}

DTEST(contains)
{
	const auto some = dutil::makeSome<char>('c');
	const auto none = dutil::makeNone<char>();
	DASSERT_TRUE(some.contains('c'));
	DASSERT_FALSE(some.contains('w'));
	DASSERT_FALSE(none.contains('c'));
}

DTEST(compare)
{
	const auto now = dutil::makeSome<int>(2021);
	const auto then = dutil::makeSome<int>(1969);
	const auto nowClone = now.clone();
	DASSERT_FALSE(now == then);
	DASSERT_TRUE(now != then);
	DASSERT_TRUE(now == nowClone);
	DASSERT_FALSE(now != nowClone);
}
