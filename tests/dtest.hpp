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

#include <cstring>
#include <vector>
#include "dtest.hpp"

/// # D T E S T
///
/// ## HOW TO USE DTEST
///
/// Call `dtest::run()` in your main function.
///
/// Then, to write your tests. Use the DTEST macro to declare tests, like so:
///   DTEST(yourTestName)
///   {
///     int x = 5;
///     DTEST_ASSERT_EQ(x, 5);
///   }

namespace dtest
{

/// Put this in your main function.
void run();

}

#define DTEST(testName)													\
	void dtest_##testName();											\
	struct DTestRegistrar_##testName									\
	{																	\
		DTestRegistrar_##testName()										\
		{																\
			dtest::details::registerTest(dtest_##testName, #testName);			\
		}																\
	};																	\
	DTestRegistrar_##testName dtestRegistrar_##testName{};				\
	void dtest_##testName()

// ========================================================================== //
// IMPLEMENTATION DETAILS BELOW
// ========================================================================== //

#if defined(_WIN32)
#define DTEST_FILENAME													\
	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define DTEST_FILENAME													\
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define DTEST_RUN_TESTS()

namespace dtest::details
{

struct TestInfo
{
	void (*testFn)();
	const char* name;
};

struct TestRegister
{
	std::vector<TestInfo> tests;
};



TestRegister& getTestRegister();

void registerTest(void (*testFn)(), const char* name);

}
