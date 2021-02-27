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


#include "dtest.hpp"

#include <cstdio>

namespace dtest::details
{

void Register::addTest(TestFunction fn, const char* testName, const char* fileName, u64 filePathHash)
{
	TestCategory& category = m_testCategories[filePathHash];
	category.name = fileName;
	category.tests.push_back({{testName, 0, 0}, std::move(fn)});
}

Register& getRegister()
{
	static Register r{};
	return r;
}

void runTests()
{
	Register& r = getRegister();

	printf("There are %d test categories to run.\n", static_cast<int>(r.getTestCategories().size()));

	int failedCategories = 0;
	
	for (auto& [_, category] : r.getTestCategories())
	{
		printf("### %s, Running %d tests.\n", category.name, static_cast<int>(category.tests.size()));

		for (TestCase& test : category.tests)
		{
			printf("\t=== %s, \n", test.state.name);
			test.fn(test.state);
			printf("\t=== %s, %s\n", test.state.name, !test.state.fail ? "PASSED" : "FAILED");
		}

		failedCategories += category.fail;
		
		printf("### %s, %s\n", category.name, !category.fail ? "PASSED" : "FAILED");
	}

	printf("\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\nSUMMARY:\n");

	if (failedCategories)
	{
		for (const auto& [_, category] : r.getTestCategories())
		{
			printf("FAILED: %s with %d/%d failed tests.\n", category.name, category.fail, category.fail + category.pass);
					}
	}
	else
	{
		printf("ALL PASSED\n");
	}
}

}
