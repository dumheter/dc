#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <dc/string.hpp>

using namespace dc;
using namespace dtest;

///////////////////////////////////////////////////////////////////////////////
// LIFETIME
//

DTEST(ResultOk) {
  dc::Result<int, const char*> result = dc::Ok(1337);
  ASSERT_TRUE(result.isOk());
  ASSERT_TRUE(!result.isErr());
}

///////////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//

DTEST(accessViaStarOperator) {
  const dc::Result<int, NoneType> value = Ok(1337);
  ASSERT_TRUE(*value == 1337);
}

DTEST(SetViaStarOperator) {
  dc::Result<int, NoneType> value = Ok(1337);
  *value = 2021;
  ASSERT_TRUE(*value == 2021);
}

DTEST(accessViaArrowOperator) {
  struct Foo {
    s32 foo() const { return 1337; }
  };
  const dc::Result<Foo, NoneType> value = Ok(Foo{});
  ASSERT_TRUE(value->foo() == 1337);
}

DTEST(modificationViaArrowOperator) {
  struct Foo {
    void setFoo(s32 foo) { m_foo = foo; }
    s32 getFoo() { return m_foo; }
    s32 m_foo;
  };
  dc::Result<Foo, NoneType> value = Ok(Foo{});
  value->setFoo(-81);
  ASSERT_TRUE(value->getFoo() == -81);
}

DTEST(ok) {
  auto result = dc::makeOk<int, dc::String>(27);
  auto maybeInt = dc::move(result).ok();
  auto maybeString = dc::move(result).err();
  ASSERT_TRUE(maybeInt);
  ASSERT_EQ(maybeInt.value(), 27);
  ASSERT_FALSE(maybeString);
}

DTEST(err) {
  auto result = dc::makeErr<int, dc::String>("carrot");
  auto maybeInt = dc::move(result).ok();
  auto maybeString = dc::move(result).err();
  ASSERT_FALSE(maybeInt);
  ASSERT_TRUE(maybeString);
  ASSERT_EQ(maybeString.value(), dc::String("carrot", TEST_ALLOCATOR));
}

DTEST(value) {
  {
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<int> original(27);
    auto result = dc::makeOk<LifetimeTracker<int>, float>(dc::move(original));
    LifetimeTracker<int>& value = result.value();
    ASSERT_EQ(value.object, 27);
    ASSERT_TRUE(stats.copies == 0);

    int& object = value.object;
    object = 13;
    ASSERT_EQ(value.object, 13);
    ASSERT_TRUE(stats.copies == 0);
  }

  {
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<int> original(27);
    const auto result =
        dc::makeOk<LifetimeTracker<int>, float>(dc::move(original));
    const LifetimeTracker<int>& value = result.value();
    ASSERT_EQ(value.object, 27);
    ASSERT_TRUE(stats.copies == 0);
  }
}

DTEST(errValue) {
  {
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<float> original(27.f);
    auto result = dc::makeErr<int, LifetimeTracker<float>>(dc::move(original));
    LifetimeTracker<float>& value = result.errValue();
    ASSERT_TRUE(value.object > 26.f && value.object < 28.f);
    ASSERT_TRUE(stats.copies == 0);

    float& object = value.object;
    object = 13.f;
    ASSERT_TRUE(value.object > 12.f && value.object < 14.f);
    ASSERT_TRUE(stats.copies == 0);
  }

  {
    LifetimeStats::resetInstance();
    LifetimeStats& stats = LifetimeStats::getInstance();

    LifetimeTracker<float> original(27.f);
    const auto result =
        dc::makeErr<int, LifetimeTracker<float>>(dc::move(original));
    const LifetimeTracker<float>& value = result.errValue();

    ASSERT_TRUE(value.object > 26.f && value.object < 28.f);
    ASSERT_TRUE(stats.copies == 0);
  }
}

DTEST(unwrap) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original = 66;
  auto result = dc::makeOk<LifetimeTracker<int>, int>(dc::move(original));
  ASSERT_EQ(stats.copies, 0);
  const int moves = stats.moves;

  LifetimeTracker<int> moved = dc::move(result).unwrap();
  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(stats.moves, moves + 1);
  ASSERT_EQ(moved.object, 66);
}

DTEST(unwrapErr) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original = 5050;
  auto result = dc::makeErr<int, LifetimeTracker<int>>(dc::move(original));
  ASSERT_EQ(stats.copies, 0);
  const int moves = stats.moves;

  LifetimeTracker<int> moved = dc::move(result).unwrapErr();
  ASSERT_EQ(stats.copies, 0);
  ASSERT_EQ(stats.moves, moves + 1);
  ASSERT_EQ(moved.object, 5050);
}

DTEST(asConstRef) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<String> original(String("hey", TEST_ALLOCATOR));
  const auto result = makeOk<LifetimeTracker<String>, int>(dc::move(original));
  const auto constRef = result.asConstRef();

  ASSERT_EQ(result.value(), constRef.value().get());
  ASSERT_EQ(stats.copies, 0);
}

DTEST(asMutRef) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<String> original(String("hey", TEST_ALLOCATOR));
  auto result = makeOk<LifetimeTracker<String>, int>(dc::move(original));
  auto mutRef = result.asMutRef();
  ASSERT_EQ(result.value(), mutRef.value().get());
  ASSERT_EQ(stats.copies, 0);

  mutRef.value().get().object += " there";
  ASSERT_EQ(result.value(), mutRef.value().get());
  ASSERT_EQ(stats.copies, 0);
}

///////////////////////////////////////////////////////////////////////////////
// EQUALITY
//

DTEST(contains) {
  const auto resultOk = makeOk<int, String>(-133);
  const auto resultErr = makeErr<int, String>(String("wow", TEST_ALLOCATOR));
  ASSERT_TRUE(resultOk.contains(-133));
  ASSERT_TRUE(resultErr.containsErr(String("wow", TEST_ALLOCATOR)));
  ASSERT_FALSE(resultOk.containsErr(String("wow", TEST_ALLOCATOR)));
  ASSERT_FALSE(resultErr.contains(-133));
}

DTEST(eqOkErr) {
  Result<int, float> result = Ok<int>(15);
  auto ok = Ok<int>(15);
  auto err = Err<float>(15.f);
  ASSERT_TRUE(result == ok);
  ASSERT_FALSE(result == err);
}

DTEST(neqOkErr) {
  Result<int, float> result = Ok<int>(15);
  auto ok = Ok<int>(15);
  auto err = Err<float>(15.f);
  ASSERT_FALSE(result != ok);
  ASSERT_TRUE(result != err);
}

DTEST(eqResult) {
  const auto AOk = makeOk<int, char>(42);
  const auto AErr = makeErr<int, char>('X');
  const auto BOk = makeOk<int, char>(42);
  const auto BErr = makeErr<int, char>('X');
  ASSERT_TRUE(AOk == BOk);
  ASSERT_FALSE(AOk == BErr);
  ASSERT_FALSE(AErr == BOk);
  ASSERT_TRUE(AErr == BErr);
}

DTEST(neqResult) {
  const auto AOk = makeOk<int, char>(42);
  const auto AErr = makeErr<int, char>('X');
  const auto BOk = makeOk<int, char>(42);
  const auto BErr = makeErr<int, char>('X');
  ASSERT_FALSE(AOk != BOk);
  ASSERT_TRUE(AOk != BErr);
  ASSERT_TRUE(AErr != BOk);
  ASSERT_FALSE(AErr != BErr);
}

///////////////////////////////////////////////////////////////////////////////
// MATCH
//

DTEST(matchRValueOk) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  Result<LifetimeTracker<int>, String> okResult = Ok(dc::move(parent));
  float value = dc::move(okResult).match(
      [](LifetimeTracker<int>) -> float { return 1.f; },
      [](String&&) -> float { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(matchRValueErr) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  Result<float, LifetimeTracker<int>> errResult = Err(dc::move(parent));
  float value = dc::move(errResult).match(
      [](float) { return 1.f; }, [](LifetimeTracker<int>) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(matchLValueOk) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  Result<LifetimeTracker<int>, String> okResult = Ok(dc::move(parent));
  float value = okResult.match([](LifetimeTracker<int>&) { return 1.f; },
                               [](String&) { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(matchLValueErr) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  Result<float, LifetimeTracker<int>> errResult = Err(dc::move(parent));
  float value = errResult.match([](float&) { return 1.f; },
                                [](LifetimeTracker<int>&) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(matchConstLValueOk) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  const Result<LifetimeTracker<int>, String> okResult = Ok(dc::move(parent));
  float value = okResult.match([](const LifetimeTracker<int>&) { return 1.f; },
                               [](const String&) { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(stats.copies, 0);
}

DTEST(matchConstLValueErr) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> parent = 13;

  const Result<float, LifetimeTracker<int>> errResult = Err(dc::move(parent));
  float value =
      errResult.match([](const float&) { return 1.f; },
                      [](const LifetimeTracker<int>&) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(stats.copies, 0);
}

///////////////////////////////////////////////////////////////////////////////
// CLONE
//

DTEST(clone) {
  LifetimeStats::resetInstance();
  LifetimeStats& stats = LifetimeStats::getInstance();

  LifetimeTracker<int> original = 77;

  Result<LifetimeTracker<int>, float> okResult = Ok(dc::move(original));
  auto clone = okResult.clone();
  auto cloneOfClone = clone.clone();

  ASSERT_EQ(stats.copies, 2);
}

///////////////////////////////////////////////////////////////////////////////
// HELPERS
//

DTEST(makeOk) {
  auto result = makeOk<float, int>(13.f);
  ASSERT_TRUE(result.isOk());
}

DTEST(makeErr) {
  auto result = makeErr<int, String>(String("hey", TEST_ALLOCATOR));
  ASSERT_TRUE(result.isErr());
}

///////////////////////////////////////////////////////////////////////////////
// MAP
//

DTEST(map) {
  dc::Result<int, int> res0 = Ok(10);
  dc::Result<char8, int> res1 = dc::move(res0).map([](int v) {
    if (v == 10)
      return 'y';
    else
      return 'n';
  });
  ASSERT_EQ(*res1, 'y');
}

DTEST(mapErr) {
  dc::Result<int, int> res0 = Err(-103);
  dc::Result<int, char8> res1 = dc::move(res0).mapErr([](int v) {
    if (v == 10)
      return 'y';
    else
      return 'n';
  });
  ASSERT_EQ(res1.errValue(), 'n');
}
