#include "dtest.hpp"

#include <cstdio>

namespace dtest
{

void run()
{
	details::TestRegister& testRegister = details::getTestRegister();

	for (const details::TestInfo testInfo : testRegister.tests)
	{
		printf("\n=== Running test %s...\n", testInfo.name);
		testInfo.testFn();
		printf("=== Test %s\n", testInfo.name);
	}
}

}

namespace dtest::details
{

TestRegister& getTestRegister()
{
	static TestRegister testRegister{};
	return testRegister;
}

void registerTest(void (*testFn)(), const char *name)
{
	TestRegister& testRegister = getTestRegister();
	testRegister.tests.emplace_back(TestInfo{testFn, name});
}

}
