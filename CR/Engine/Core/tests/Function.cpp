import CR.Engine.Core.Function;

#include <function2/function2.hpp>
#include <doctest/doctest.h>

TEST_CASE("function") {
	CR::Engine::Core::MultiFunction<void(int, int)> multiFunc;
	int test1 = 0;
	int test2 = 0;
	multiFunc += [&](int arg1, int arg2) { test1 = arg1 + arg2; };
	multiFunc += [&](int arg1, int arg2) { test2 = arg1 * arg2; };
	bool checkMulti = static_cast<bool>(multiFunc);
	REQUIRE(checkMulti == true);
	//CHECK_EQ(static_cast<bool>(multiFunc), true);
	multiFunc(2, 3);
	CHECK(test1 == 5);
	CHECK(test2 == 6);

	test1 = 0;
	test2 = 0;
	CR::Engine::Core::SelectableFunction<int(int, int)> selFunc;
	selFunc += [&](int arg1, int arg2) { return arg1 + arg2; };
	selFunc += [&](int arg1, int arg2) { return arg1 * arg2; };
	selFunc.SetOperation(0);
	CHECK(static_cast<bool>(selFunc) == true);
	test1 = selFunc(5, 6);
	CHECK(test1 == 11);

	selFunc.SetOperation(1);
	CHECK(static_cast<bool>(selFunc) == true);
	test1 = selFunc(5, 6);
	CHECK(test1 == 30);
}
