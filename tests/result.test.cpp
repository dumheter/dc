#include <dutil/result.hpp>
#include <string>

#include "dtest.hpp"

DTEST(ResultOk) {
  dutil::Result<int, const char*> result = dutil::Ok(1337);
  DASSERT_TRUE(result.isOk());
  DASSERT_TRUE(!result.isErr());
}

// ========================================================================== //
// EQUALITY
// ========================================================================== //

DTEST(eq_ok)
{
	dutil::Result<int, float> result = dutil::Ok<int>(15);
	auto ok = dutil::Ok<int>(15);
	auto err = dutil::Err<float>(15.f);
	DASSERT_TRUE(result == ok);
	DASSERT_FALSE(result == err);
}

DTEST(neq_ok)
{
	dutil::Result<int, float> result = dutil::Ok<int>(15);
	auto ok = dutil::Ok<int>(15);
	auto err = dutil::Err<float>(15.f);
	DASSERT_TRUE(result == ok);
	DASSERT_FALSE(result == err);
}

DTEST(eq_err)
{
	
}

DTEST(neq_err)
{
	
}

DTEST(eq_result)
{
	
}

DTEST(neq_result)
{
	
}

// ========================================================================== //
// MATCH
// ========================================================================== //

DTEST(match_rvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dutil::Result<TrackedInt, std::string> okResult =
      dutil::Ok(std::move(parent));
  float value = std::move(okResult).match([](TrackedInt value) { return 1.f; },
                                          [](std::string err) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_rvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dutil::Result<float, TrackedInt> errResult = dutil::Err(std::move(parent));
  float value = std::move(errResult).match([](float ok) { return 1.f; },
                                           [](TrackedInt err) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dutil::Result<TrackedInt, std::string> okResult =
      dutil::Ok(std::move(parent));
  float value = okResult.match([](TrackedInt& value) { return 1.f; },
                               [](std::string& err) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_lvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  dutil::Result<float, TrackedInt> errResult = dutil::Err(std::move(parent));
  float value = errResult.match([](float& ok) { return 1.f; },
                                [](TrackedInt& err) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_ok) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  const dutil::Result<TrackedInt, std::string> okResult =
      dutil::Ok(std::move(parent));
  float value = okResult.match([](const TrackedInt& value) { return 1.f; },
                               [](const std::string& err) { return -1.f; });
  DASSERT_TRUE(value > 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

DTEST(match_const_lvalue_err) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt parent = 13;

  const dutil::Result<float, TrackedInt> errResult =
      dutil::Err(std::move(parent));
  float value = errResult.match([](const float& ok) { return 1.f; },
                                [](const TrackedInt& err) { return -1.f; });
  DASSERT_TRUE(value < 0.f);
  DASSERT_EQ(parent.getCopies(), 0);
}

// ========================================================================== //
// CLONE
// ========================================================================== //

DTEST(clone) {
  using TrackedInt = dtest::TrackLifetime<int>;
  TrackedInt original = 77;

  dutil::Result<TrackedInt, float> okResult = dutil::Ok(std::move(original));
  auto clone = okResult.clone();
  auto cloneOfClone = clone.clone();

  DASSERT_EQ(original.getCopies(), 2);
}

// ========================================================================== //
// HELPERS
// ========================================================================== //

DTEST(make_ok) {
  auto result = dutil::make_ok<float, int>(13.f);
  DASSERT_TRUE(result.isOk());
}

DTEST(make_err) {
  auto result = dutil::make_err<int, std::string>("hey");
  DASSERT_TRUE(result.isErr());
}
