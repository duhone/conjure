import CR.Engine.Core.BitSet;

#include <doctest/doctest.h>

import <vector>;

using namespace std;
using namespace CR::Engine::Core;

TEST_CASE("BitSet Basic") {
	BitSet<512> bitset;
	CHECK(bitset.empty());
	CHECK(bitset.contains(6) == false);
	bitset.insert(6);
	bitset.insert(42);
	bitset.insert(128);
	CHECK(!bitset.empty());
	CHECK(bitset.size() == 3);
	CHECK(bitset.contains(0) == false);
	CHECK(bitset.contains(6) == true);
	CHECK(bitset.contains(42) == true);
	CHECK(bitset.contains(43) == false);
	CHECK(bitset.contains(128) == true);
	bitset.erase(6);
	CHECK(!bitset.empty());
	CHECK(bitset.size() == 2);
	CHECK(bitset.contains(0) == false);
	CHECK(bitset.contains(6) == false);
	CHECK(bitset.contains(42) == true);
	CHECK(bitset.contains(43) == false);
	CHECK(bitset.contains(128) == true);
	bitset.clear();
	CHECK(bitset.empty());
	CHECK(bitset.size() == 0);
	CHECK(bitset.contains(0) == false);
	CHECK(bitset.contains(6) == false);
	CHECK(bitset.contains(42) == false);
	CHECK(bitset.contains(43) == false);
	CHECK(bitset.contains(128) == false);
}

TEST_CASE("BitSet Find integer not in set") {
	BitSet<512> bitset;
	CHECK(bitset.FindNotInSet() == 0);
	bitset.insert(6);
	CHECK(bitset.FindNotInSet() == 0);
	bitset.insert(0);
	CHECK(bitset.FindNotInSet() == 1);
	for(std::uint16_t i = 0; i < 128; ++i) { bitset.insert(i); }
	CHECK(bitset.FindNotInSet() == 128);
	bitset.insert(128);
	CHECK(bitset.FindNotInSet() == 129);
}

TEST_CASE("BitSet iterate") {
	BitSet<512> bitset;
	// check can iterate an empty bitset, loop should exit immediatly
	CHECK(bitset.empty() == true);
	for([[maybe_unused]] const auto val : bitset) { CHECK(false); }

	// check can iterate an a bitset with one value
	bitset.insert(6);
	CHECK(bitset.size() == 1);
	int counter = 0;
	std::vector<std::uint16_t> expected;
	expected.push_back(6);
	for(const auto val : bitset) {
		CHECK(counter < expected.size());
		CHECK(val == expected[counter]);
		++counter;
	}

	// check can iterate an a bitset with several values
	bitset.insert(0);
	bitset.insert(128);
	bitset.insert(312);
	expected.clear();
	expected.push_back(0);
	expected.push_back(6);
	expected.push_back(128);
	expected.push_back(312);
	counter = 0;
	for(const auto val : bitset) {
		CHECK(counter < expected.size());
		CHECK(val == expected[counter]);
		++counter;
	}
}