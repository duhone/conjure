#include <doctest/doctest.h>

import CR.Engine.Core.Handle;

import std;

using namespace std;
using namespace CR::Engine::Core;

using TestHandle = CR::Engine::Core::Handle<class TestHandleTag>;

TEST_CASE("Handle Pool") {
	HandlePool<TestHandle, 512> pool;

	TestHandle handle1 = pool.aquire();
	CHECK(handle1.isValid());
	TestHandle handle1Dup = handle1;
	CHECK(handle1Dup.isValid());

	TestHandle handle2 = pool.aquire();
	CHECK(handle2.isValid());

	CHECK(handle1 != handle2);

	CHECK(pool.isValid(handle1));
	CHECK(pool.isValid(handle1Dup));
	CHECK(pool.isValid(handle2));

	pool.release(handle1);
	CHECK(!handle1.isValid());
	// a dup so handle itself will still be valid. but will be invalid when checked with the pool
	CHECK(handle1Dup.isValid());
	CHECK(!pool.isValid(handle1));
	CHECK(!pool.isValid(handle1Dup));
}
