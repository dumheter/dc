#include "main.test.hpp"
#include "dtest.hpp"
#include <cstdio>


/// @param fn Just type the function name of the test.
/// The fn should have signature: bool (*fn)()
// #define DUTIL_TEST(fn) \
// 	do {\
// printf("\n=== Running test %s...\n", #fn);\
// const bool res = fn();\
// printf("=== Test %s %s\n", #fn, res ? "passed" : "failed");\
// } while (0)

DTEST(intTest)
{
	int x = 5;
}

int main(int, char**)
{
	dtest::run();
	//DUTIL_TEST(resultTest);

	printf("%s %s %d %s\n", __FILE__, __FUNCTION__, __LINE__, __func__);

	return 0;
}
