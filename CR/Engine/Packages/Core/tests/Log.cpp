#include <doctest/doctest.h>

#include <core/Log.h>

import std;

using namespace CR::Engine::Core;
using namespace std;
using namespace std::chrono_literals;

TEST_CASE("Log") {
	LogSystem logSystem;

	CR_LOG("Testing log");

	/* need to skip if debugger attached, warns and up always break in debugger
	if constexpr(CR_DEBUG || CR_RELEASE) {
	    CHECK_THROWS([] { CR_WARN("Testing warn"); }());
	} else {
	    CR_WARN("Testing warn");
	}

	CHECK_THROWS([] { CR_ERROR("Testing Error"); }());

	if constexpr(CR_DEBUG || CR_RELEASE) {
	    CHECK_THROWS([] { CR_ASSERT_AUDIT(false, "Testing audit assert"); }());
	    CHECK_THROWS([] { CR_REQUIRES_AUDIT(false, "Testing audit requries"); }());
	    CHECK_THROWS([] { CR_ENSURES_AUDIT(false, "Testing audit ensures"); }());
	}

	CHECK_THROWS([] { CR_ASSERT(false, "Testing assert"); }());
	CHECK_THROWS([] { CR_REQUIRES(false, "Testing requries"); }());
	CHECK_THROWS([] { CR_ENSURES(false, "Testing ensures"); }());
	*/
	// catch logging interferes with spdlog, so give a little bit of time for spdlog to finish
	this_thread::sleep_for(250ms);
}
