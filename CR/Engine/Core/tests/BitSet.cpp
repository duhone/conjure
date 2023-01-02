import CR.Engine.Core.BitSet;

#include <doctest/doctest.h>

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