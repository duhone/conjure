import CR.Engine.Core.Log;

#include <doctest/doctest.h>

import<thread>;

using namespace CR::Engine::Core;
using namespace std;

TEST_CASE("Log") {
	Log::Debug("Testing debug");

	Log::Info("Testing info");

	if constexpr(CR_DEBUG || CR_RELEASE) {
		CHECK_THROWS(Log::Warn("Testing warn"));
	} else {
		Log::Warn("Testing warn");
	}

	CHECK_THROWS(Log::Error("Testing Error"));

	if constexpr(CR_DEBUG || CR_RELEASE) { CHECK_THROWS(Log::Assert(false, "Testing assert")); }

	CHECK_THROWS(Log::Require(false, "Testing Require"));

	// catch logging interferes with spdlog, so give a little bit of time for spdlog to finish
	this_thread::sleep_for(250ms);
}
