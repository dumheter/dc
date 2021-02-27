/**
 * MIT License
 *
 * Copyright (c) 2021 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "dtest.hpp"

#include <dutil/types.hpp>
#include <dutil/misc.hpp>

#include <functional>

// ========================================================================== //
// DTEST
// ========================================================================== //

/// Add a test.
///
/// Example
///   DTEST(ExampleTest)
///   {
///      ...
///   }
#define DTEST(testName) DTEST_REGISTER(testName, DUTIL_FILENAME, __FILE__)

/// Run all registered tests.
#define DTEST_RUN() dtest::details::runTests()

#define DTEST_ASSERT(expr)								\
	do													\
	{													\
		if (!!(expr))									\
		{												\
			++dtest_details_testBodyState.pass;			\
			printf("\t\t+ Assert " #expr " passed\n");	\
		}												\
		else											\
		{												\
			++dtest_details_testBodyState.fail;			\
			printf("\t\t- Assert " #expr " failed\n");	\
		}												\
	} while(0)

// ========================================================================== //
// INTERNAL DETAILS
// ========================================================================== //

namespace dtest::details
{

struct TestBodyState
{
	const char* name = nullptr;
	int pass = 0;
	int fail = 0;
};

using TestFunction = std::function<void(TestBodyState&)>;

struct TestCase
{
	TestBodyState state;
	TestFunction fn;
};

struct TestCategory
{
	const char* name = nullptr;
	std::vector<TestCase> tests;
	int pass = 0;
	int fail = 0;
};

class Register
{
  public:
	Register() = default;
	DUTIL_DELETE_COPY(Register);

	void addTest(TestFunction fn, const char* testName, const char* fileName, u64 filePathHash);

	const std::unordered_map<u64, TestCategory>& getTestCategories() const { return m_testCategories; }
	std::unordered_map<u64, TestCategory>& getTestCategories() { return m_testCategories; }
	
  private:
	std::unordered_map<u64, TestCategory> m_testCategories;
};

/// Returnes a static instantitation of Register.
Register& getRegister();

void runTests();

template <typename Fn>
void dtestAdd(Fn&& fn, const char* testName, const char* fileName, u64 filePathHash)
{
	getRegister().addTest(std::forward<Fn>(fn), testName, fileName, filePathHash);
}

}

#define DTEST_REGISTER(testName, fileName, filePath)					\
	struct DTest_Reg_##testName											\
	{																	\
		void testBody(dtest::details::TestBodyState& dtest_details_testBodyState); \
		DTest_Reg_##testName() { dtest::details::dtestAdd([this](dtest::details::TestBodyState& dtest_details_testBodyState){ testBody(dtest_details_testBodyState); }, #testName, fileName, dutil::hash64fnv1a(filePath)); } \
	};																	\
	DTest_Reg_##testName dtest_Reg_##testName{};						\
	void DTest_Reg_##testName##::testBody(dtest::details::TestBodyState& dtest_details_testBodyState)

