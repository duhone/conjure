#include <doctest/doctest.h>

import CR.Engine.Core.Span;

import std;

using namespace CR::Engine::Core;

namespace {
	int Sum(SpanArg<const int> a_values) {
		return std::accumulate(a_values.begin(), a_values.end(), 0);
	}

	void Double(SpanArg<int> a_values) {
		for(auto& value : a_values) { value *= 2; }
	}

	std::size_t Count(std::span<const int> a_values) {
		return a_values.size();
	}
}    // namespace

TEST_CASE("SpanArg") {
	SUBCASE("single element") {
		int value = 5;
		CHECK(Sum(value) == 5);
		CHECK(Sum(7) == 7);
		Double(value);
		CHECK(value == 10);
	}

	SUBCASE("bulk") {
		std::vector<int> vec{1, 2, 3};
		std::array<int, 2> arr{4, 5};
		CHECK(Sum(vec) == 6);
		CHECK(Sum(arr) == 9);
		CHECK(Sum(std::span<int>(vec)) == 6);
		CHECK(Sum(std::span<const int>(vec)) == 6);
		CHECK(Sum({vec.begin() + 1, vec.end()}) == 5);
		Double(vec);
		CHECK(vec == std::vector<int>{2, 4, 6});
	}

	SUBCASE("passes on to std::span") {
		int value = 5;
		SpanArg<int> arg(value);
		CHECK(Count(arg) == 1);
		std::span<int> asSpan = arg;
		CHECK(asSpan.data() == &value);
	}
}
