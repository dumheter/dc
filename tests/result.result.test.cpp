#include <dc/dtest.hpp>
#include <dc/result.hpp>
#include <string>

using TrackedInt = dtest::TrackLifetime<int>;
using TrackedFloat = dtest::TrackLifetime<float>;
using TrackedString = dtest::TrackLifetime<std::string>;

// ========================================================================== //
// LIFETIME
// ========================================================================== //

DTEST(ResultOk) {
  dc::Result<int, const char*> result = dc::Ok(1337);
  DASSERT_TRUE(result.isOk());
  DASSERT_TRUE(!result.isErr());
}

// ========================================================================== //
// DATA ACCESS
// ========================================================================== //

DTEST(ok) {
  auto result = dc::makeOk<int, std::string>(27);
  auto maybeInt = std::move(result).ok();
  auto maybeString = std::move(result).err();
  DASSERT_TRUE(maybeInt);
  DASSERT_EQ(maybeInt.value(), 27);
  DASSERT_FALSE(maybeString);
}

DTEST(err) {
  auto result = dc::makeErr<int, std::string>("carrot");
  auto maybeInt = std::move(result).ok();
  auto maybeString = std::move(result).err();
  DASSERT_FALSE(maybeInt);
  DASSERT_TRUE(maybeString);
  DASSERT_EQ(maybeString.value(), std::string("carrot"));
}

DTEST(value) {
  {
    TrackedInt original(27);
    auto result = dc::makeOk<TrackedInt, float>(std::move(original));
    TrackedInt& value = result.value();
    DASSERT_EQ(value.getObject(), 27);
    DASSERT_TRUE(value.getCopies() == 0);

    int& object = value.getObject();
    object = 13;
    DASSERT_EQ(value.getObject(), 13);
    DASSERT_TRUE(value.getCopies() == 0);
  }

  {
    TrackedInt original(27);
    const auto result = dc::makeOk<TrackedInt, float>(std::move(original));
    const TrackedInt& value = result.value();
    DASSERT_EQ(value.getObject(), 27);
    DASSERT_TRUE(value.getCopies() == 0);
  }
}

DTEST(errValue) {
  {
    TrackedFloat original(27.f);
    auto result = dc::makeErr<int, TrackedFloat>(std::move(original));
    TrackedFloat& value = result.errValue();
    DASSERT_TRUE(value.getObject() > 26.f && value.getObject() < 28.f);
    DASSERT_TRUE(value.getCopies() == 0);

    float& object = value.getObject();
    object = 13.f;
    DASSERT_TRUE(value.getObject() > 12.f && value.getObject() < 14.f);
    DASSERT_TRUE(value.getCopies() == 0);
  }

  {
    TrackedFloat original(27.f);
    const auto result = dc::makeErr<int, TrackedFloat>(std::move(original));
    const TrackedFloat& value = result.errValue();
    DASSERT_TRUE(value.getObject() > 26.f && value.getObject() < 28.f);
    DASSERT_TRUE(value.getCopies() == 0);
  }
}

DTEST(unwrap) {
  TrackedInt original = 66;
  auto result = dc::makeOk<TrackedInt, int>(std::move(original));
  DASSERT_EQ(original.getCopies(), 0);
  const int moves = original.getMoves();

  TrackedInt moved = std::move(result).unwrap();
  DASSERT_EQ(original.getCopies(), 0);
  DASSERT_EQ(original.getMoves(), moves + 1);
  DASSERT_EQ(moved.getObject(), 66);
}

DTEST(unwrapErr) {
  using TrackedInt = dtest::TrackLifetime<int>;

  TrackedInt original = 5050;
  auto result = dc::makeErr<int, TrackedInt>(std::move(original));
  DASSERT_EQ(original.getCopies(), 0);
  const int moves = original.getMoves();

  TrackedInt moved = std::move(result).unwrapErr();
  DASSERT_EQ(original.getCopies(), 0);
  DASSERT_EQ(original.getMoves(), moves + 1);
  DASSERT_EQ(moved.getObject(), 5050);
}

DTEST(as_const_ref) {
  using TrackedString = dtest::TrackLifetime<std::string>;

  TrackedString original(std::string("hey"));
  const auto result = dc::makeOk<TrackedString, int>(std::move(original));
  const auto constRef = result.asConstRef();
  DASSERT_EQ(result.value(), constRef.value().get());
  DASSERT_EQ(original.getCopies(), 0);
}

DTEST(as_mut_ref) {
  using TrackedString = dtest::TrackLifetime<std::string>;

  TrackedString original(std::string("hey"));
  auto result = dc::makeOk<TrackedString, int>(std::move(original));
  auto mutRef = result.asMutRef();
  DASSERT_EQ(result.value(), mutRef.value().get());
  DASSERT_EQ(original.getCopies(), 0);

  mutRef.value().get().getObject() += " there";
  DASSERT_EQ(result.value(), mutRef.value().get());
  DASSERT_EQ(original.getCopies(), 0);
}

// ========================================================================== //
// EQUALITY
// ========================================================================== //

DTEST(contains) {
  const auto resultOk = dc::makeOk<int, std::string>(-133);
  const auto resultErr = dc::makeErr<int, std::string>(std::string("wow"));
  DASSERT_TRUE(resultOk.contains(-133));
  DASSERT_TRUE(resultErr.containsErr(std::string("wow")));
  DASSERT_FALSE(resultOk.containsErr(std::string("wow")));
  DASSERT_FALSE(resultErr.contains(-133));
}

DTEST(eq_ok_err) {
  dc::Result<int, float> result = dc::Ok<int>(15);
  auto ok = dc::Ok<int>(15);
  auto err = dc::Err<float>(15.f);
  DASSERT_TRUE(result == ok);
  DASSERT_FALSE(result == err);
}

DTEST(neq_ok_err) {
  dc::Result<int, float> result = dc::Ok<int>(15);
  auto ok = dc::Ok<int>(15);
  auto err = dc::Err<float>(15.f);
  DASSERT_FALSE(result != ok);
  DASSERT_TRUE(result != err);
}

DTEST(eq_result) {
  const auto AOk = dc::makeOk<int, char>(42);
  const auto AErr = dc::makeErr<int, char>('X');
  const auto BOk = dc::makeOk<int, char>(42);
  const auto BErr = dc::makeErr<int, char>('X');
  DASSERT_TRUE(AOk == BOk);
  DASSERT_FALSE(AOk == BErr);
  DASSERT_FALSE(AErr == BOk);
  DASSERT_TRUE(AErr == BErr);
}

DTEST(neq_result) {
  const auto AOk = dc::makeOk<int, char>(42);
  const auto AErr = dc::makeErr<int, char>('X');
  const auto BOk = dc::makeOk<int, char>(42);
  const auto BErr = dc::makeErr<int, char>('X');
  DASSERT_FALSE(AOk != BOk);
  DASSERT_TRUE(AOk != BErr);
  DASSERT_TRUE(AErr != BOk);
  DASSERT_FALSE(AErr != BErr);
}

// ========================================================================== //
// MATCH
// ========================================================================== //

DTEST(match_rvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dc::Result<TrackedInt, std::string> okResult = dc::Ok(std::move(parent));
  float value = std::move(okResult).match([](TrackedInt value) { return 1.f; },
                                          [](std::string err) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_rvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dc::Result<float, TrackedInt> errResult = dc::Err(std::move(parent));
  float value = std::move(errResult).match([](float) { return 1.f; },
                                           [](TrackedInt) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dc::Result<TrackedInt, std::string> okResult = dc::Ok(std::move(parent));
  float value = okResult.match([](TrackedInt&) { return 1.f; },
                               [](std::string&) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dc::Result<float, TrackedInt> errResult = dc::Err(std::move(parent));
  float value = errResult.match([](float&) { return 1.f; },
                                [](TrackedInt&) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  const dc::Result<TrackedInt, std::string> okResult =
      dc::Ok(std::move(parent));
  float value = okResult.match([](const TrackedInt&) { return 1.f; },
                               [](const std::string&) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  const dc::Result<float, TrackedInt> errResult = dc::Err(std::move(parent));
  float value = errResult.match([](const float&) { return 1.f; },
                                [](const TrackedInt&) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

// ========================================================================== //
// CLONE
// ========================================================================== //

DTEST(clone) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt original = 77;

  dc::Result<TrackedInt, float> okResult = dc::Ok(std::move(original));
  auto clone = okResult.clone();
  auto cloneOfClone = clone.clone();

  DASSERT_EQ(original.getCopies(), 2);
}

// ========================================================================== //
// HELPERS
// ========================================================================== //

DTEST(make_ok) {
  auto result = dc::makeOk<float, int>(13.f);
  DASSERT_TRUE(result.isOk());
}

DTEST(make_err) {
  auto result = dc::makeErr<int, std::string>("hey");
  DASSERT_TRUE(result.isErr());
}
