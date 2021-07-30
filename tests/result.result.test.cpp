#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedFloat = dtest::TrackLifetime<float>;
using TrackedString = dtest::TrackLifetime<std::string>;

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

DTEST(ok) {
  auto result = dc::makeOk<int, std::string>(27);
  auto maybeInt = dc::move(result).ok();
  auto maybeString = dc::move(result).err();
  ASSERT_TRUE(maybeInt);
  ASSERT_EQ(maybeInt.value(), 27);
  ASSERT_FALSE(maybeString);
}

DTEST(err) {
  auto result = dc::makeErr<int, std::string>("carrot");
  auto maybeInt = dc::move(result).ok();
  auto maybeString = dc::move(result).err();
  ASSERT_FALSE(maybeInt);
  ASSERT_TRUE(maybeString);
  ASSERT_EQ(maybeString.value(), std::string("carrot"));
}

DTEST(value) {
  {
    TrackedInt original(27);
    auto result = dc::makeOk<TrackedInt, float>(dc::move(original));
    TrackedInt& value = result.value();
    ASSERT_EQ(value.getObject(), 27);
    ASSERT_TRUE(value.getCopies() == 0);

    int& object = value.getObject();
    object = 13;
    ASSERT_EQ(value.getObject(), 13);
    ASSERT_TRUE(value.getCopies() == 0);
  }

  {
    TrackedInt original(27);
    const auto result = dc::makeOk<TrackedInt, float>(dc::move(original));
    const TrackedInt& value = result.value();
    ASSERT_EQ(value.getObject(), 27);
    ASSERT_TRUE(value.getCopies() == 0);
  }
}

DTEST(errValue) {
  {
    TrackedFloat original(27.f);
    auto result = dc::makeErr<int, TrackedFloat>(dc::move(original));
    TrackedFloat& value = result.errValue();
    ASSERT_TRUE(value.getObject() > 26.f && value.getObject() < 28.f);
    ASSERT_TRUE(value.getCopies() == 0);

    float& object = value.getObject();
    object = 13.f;
    ASSERT_TRUE(value.getObject() > 12.f && value.getObject() < 14.f);
    ASSERT_TRUE(value.getCopies() == 0);
  }

  {
    TrackedFloat original(27.f);
    const auto result = dc::makeErr<int, TrackedFloat>(dc::move(original));
    const TrackedFloat& value = result.errValue();
    ASSERT_TRUE(value.getObject() > 26.f && value.getObject() < 28.f);
    ASSERT_TRUE(value.getCopies() == 0);
  }
}

DTEST(unwrap) {
  TrackedInt original = 66;
  auto result = dc::makeOk<TrackedInt, int>(dc::move(original));
  ASSERT_EQ(original.getCopies(), 0);
  const int moves = original.getMoves();

  TrackedInt moved = dc::move(result).unwrap();
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(original.getMoves(), moves + 1);
  ASSERT_EQ(moved.getObject(), 66);
}

DTEST(unwrapErr) {
  TrackedInt original = 5050;
  auto result = dc::makeErr<int, TrackedInt>(dc::move(original));
  ASSERT_EQ(original.getCopies(), 0);
  const int moves = original.getMoves();

  TrackedInt moved = dc::move(result).unwrapErr();
  ASSERT_EQ(original.getCopies(), 0);
  ASSERT_EQ(original.getMoves(), moves + 1);
  ASSERT_EQ(moved.getObject(), 5050);
}

DTEST(as_const_ref) {
  TrackedString original(std::string("hey"));
  const auto result = dc::makeOk<TrackedString, int>(dc::move(original));
  const auto constRef = result.asConstRef();
  ASSERT_EQ(result.value(), constRef.value().get());
  ASSERT_EQ(original.getCopies(), 0);
}

DTEST(as_mut_ref) {
  TrackedString original(std::string("hey"));
  auto result = dc::makeOk<TrackedString, int>(dc::move(original));
  auto mutRef = result.asMutRef();
  ASSERT_EQ(result.value(), mutRef.value().get());
  ASSERT_EQ(original.getCopies(), 0);

  mutRef.value().get().getObject() += " there";
  ASSERT_EQ(result.value(), mutRef.value().get());
  ASSERT_EQ(original.getCopies(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// EQUALITY
//

DTEST(contains) {
  const auto resultOk = dc::makeOk<int, std::string>(-133);
  const auto resultErr = dc::makeErr<int, std::string>(std::string("wow"));
  ASSERT_TRUE(resultOk.contains(-133));
  ASSERT_TRUE(resultErr.containsErr(std::string("wow")));
  ASSERT_FALSE(resultOk.containsErr(std::string("wow")));
  ASSERT_FALSE(resultErr.contains(-133));
}

DTEST(eq_ok_err) {
  dc::Result<int, float> result = dc::Ok<int>(15);
  auto ok = dc::Ok<int>(15);
  auto err = dc::Err<float>(15.f);
  ASSERT_TRUE(result == ok);
  ASSERT_FALSE(result == err);
}

DTEST(neq_ok_err) {
  dc::Result<int, float> result = dc::Ok<int>(15);
  auto ok = dc::Ok<int>(15);
  auto err = dc::Err<float>(15.f);
  ASSERT_FALSE(result != ok);
  ASSERT_TRUE(result != err);
}

DTEST(eq_result) {
  const auto AOk = dc::makeOk<int, char>(42);
  const auto AErr = dc::makeErr<int, char>('X');
  const auto BOk = dc::makeOk<int, char>(42);
  const auto BErr = dc::makeErr<int, char>('X');
  ASSERT_TRUE(AOk == BOk);
  ASSERT_FALSE(AOk == BErr);
  ASSERT_FALSE(AErr == BOk);
  ASSERT_TRUE(AErr == BErr);
}

DTEST(neq_result) {
  const auto AOk = dc::makeOk<int, char>(42);
  const auto AErr = dc::makeErr<int, char>('X');
  const auto BOk = dc::makeOk<int, char>(42);
  const auto BErr = dc::makeErr<int, char>('X');
  ASSERT_FALSE(AOk != BOk);
  ASSERT_TRUE(AOk != BErr);
  ASSERT_TRUE(AErr != BOk);
  ASSERT_FALSE(AErr != BErr);
}

///////////////////////////////////////////////////////////////////////////////
// MATCH
//

DTEST(match_rvalue_ok) {
  TrackedInt parent = 13;

  dc::Result<TrackedInt, std::string> okResult = dc::Ok(dc::move(parent));
  float value =
      std::move(okResult).match([](TrackedInt) -> float { return 1.f; },
                                [](std::string&&) -> float { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_rvalue_err) {
  TrackedInt parent = 13;

  dc::Result<float, TrackedInt> errResult = dc::Err(dc::move(parent));
  float value = dc::move(errResult).match([](float) { return 1.f; },
                                          [](TrackedInt) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_ok) {
  TrackedInt parent = 13;

  dc::Result<TrackedInt, std::string> okResult = dc::Ok(dc::move(parent));
  float value = okResult.match([](TrackedInt&) { return 1.f; },
                               [](std::string&) { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_err) {
  TrackedInt parent = 13;

  dc::Result<float, TrackedInt> errResult = dc::Err(dc::move(parent));
  float value = errResult.match([](float&) { return 1.f; },
                                [](TrackedInt&) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_ok) {
  TrackedInt parent = 13;

  const dc::Result<TrackedInt, std::string> okResult = dc::Ok(dc::move(parent));
  float value = okResult.match([](const TrackedInt&) { return 1.f; },
                               [](const std::string&) { return -1.f; });
  ASSERT_TRUE(value > 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_err) {
  TrackedInt parent = 13;

  const dc::Result<float, TrackedInt> errResult = dc::Err(dc::move(parent));
  float value = errResult.match([](const float&) { return 1.f; },
                                [](const TrackedInt&) { return -1.f; });
  ASSERT_TRUE(value < 0.f);
  ASSERT_EQ(parent.getCopies(), 0);
}

///////////////////////////////////////////////////////////////////////////////
// CLONE
//

DTEST(clone) {
  TrackedInt original = 77;

  dc::Result<TrackedInt, float> okResult = dc::Ok(dc::move(original));
  auto clone = okResult.clone();
  auto cloneOfClone = clone.clone();

  ASSERT_EQ(original.getCopies(), 2);
}

///////////////////////////////////////////////////////////////////////////////
// HELPERS
//

DTEST(make_ok) {
  auto result = dc::makeOk<float, int>(13.f);
  ASSERT_TRUE(result.isOk());
}

DTEST(make_err) {
  auto result = dc::makeErr<int, std::string>("hey");
  ASSERT_TRUE(result.isErr());
}
