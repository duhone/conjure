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

TEST_CASE("BitSet Insert Multiple") {
	BitSet<512> bitset;
	CHECK(bitset.size() == 0);
	bitset.insert(6);
	CHECK(bitset.FindNotInSet() == 0);
	bitset.insert(0);
	CHECK(bitset.FindNotInSet() == 1);
	for(std::uint16_t i = 0; i < 128; ++i) { bitset.insert(i); }
	CHECK(bitset.FindNotInSet() == 128);
	bitset.insert(128);
	CHECK(bitset.FindNotInSet() == 129);
}

TEST_CASE("BitSet bitwise") {
	BitSet<512> bitset1;
	bitset1.insert(2);
	bitset1.insert(6);
	BitSet<512> bitset2;
	bitset2.insert(2);
	bitset2.insert(12);

	BitSet<512> bitsetUnion = bitset1 | bitset2;
	CHECK(bitsetUnion.contains(2));
	CHECK(bitsetUnion.contains(6));
	CHECK(bitsetUnion.contains(12));

	BitSet<512> bitsetIntersection = bitset1 & bitset2;
	CHECK(bitsetIntersection.contains(2));
	CHECK(!bitsetIntersection.contains(6));
	CHECK(!bitsetIntersection.contains(12));

	BitSet<512> bitsetDisjunctiveUnion = bitset1 ^ bitset2;
	CHECK(!bitsetDisjunctiveUnion.contains(2));
	CHECK(bitsetDisjunctiveUnion.contains(6));
	CHECK(bitsetDisjunctiveUnion.contains(12));

	BitSet<512> bitsetComplement = ~bitset1;
	CHECK(bitsetComplement.contains(0));
	CHECK(bitsetComplement.contains(1));
	CHECK(!bitsetComplement.contains(2));
	CHECK(bitsetComplement.contains(3));
	CHECK(bitsetComplement.contains(4));
	CHECK(bitsetComplement.contains(5));
	CHECK(!bitsetComplement.contains(6));
	CHECK(bitsetComplement.contains(7));
}

TEST_CASE("BitSet Insert Range") {
	BitSet<512> bitset;
	CHECK(bitset.size() == 0);
	bitset.insertRange(0, 10);
	CHECK(bitset.FindNotInSet() == 10);
	for(std::uint16_t i = 0; i < 10; ++i) { CHECK(bitset.contains(i)); }
	for(std::uint16_t i = 10; i < 512; ++i) { CHECK(!bitset.contains(i)); }

	bitset.clear();
	bitset.insertRange(10, 12);
	for(std::uint16_t i = 0; i < 10; ++i) { CHECK(!bitset.contains(i)); }
	for(std::uint16_t i = 10; i < 22; ++i) { CHECK(bitset.contains(i)); }
	for(std::uint16_t i = 22; i < 512; ++i) { CHECK(!bitset.contains(i)); }

	bitset.clear();
	bitset.insertRange(10, 140);
	for(std::uint16_t i = 0; i < 10; ++i) { CHECK(!bitset.contains(i)); }
	for(std::uint16_t i = 10; i < 150; ++i) { CHECK(bitset.contains(i)); }
	for(std::uint16_t i = 150; i < 512; ++i) { CHECK(!bitset.contains(i)); }
}