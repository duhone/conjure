#include <doctest/doctest.h>

import CR.Engine.Core.Locked;

import CR.Engine.Core.Algorithm;

import std;

using namespace std;
using namespace CR::Engine::Core;

TEST_CASE("Locked") {
	Locked<vector<int>> data;

	auto task1 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			data([i](vector<int>& a_data) { a_data.push_back(i); });
		}
	});
	auto task2 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			data([i](vector<int>& a_data) { a_data.push_back(i); });
		}
	});
	// should crash or undefined behavior if Locked doesn't work
	task1.wait();
	task2.wait();

	const auto& cdata = data;

	auto task3  = async(std::launch::async, [&]() {
        return cdata([](const vector<int>& a_data) { return accumulate(begin(a_data), end(a_data), 0); });
    });
	int result1 = task3.get();

	REQUIRE(result1 == 99990000);
}

TEST_CASE("Locked Multiple") {
	Locked<vector<int>, list<float>> data;

	auto task1 = async(std::launch::async, [&]() {
		for(int i = 0; i < 5; ++i) {
			data([i](vector<int>& a_intVec, list<float>& a_floatList) {
				a_intVec.push_back(i);
				a_floatList.push_back(2.0f * i);
			});
		}
	});
	auto task2 = async(std::launch::async, [&]() {
		for(int i = 5; i < 10; ++i) {
			data([i](vector<int>& a_intVec, list<float>& a_floatList) {
				a_intVec.push_back(i);
				a_floatList.push_back(2.0f * i);
			});
		}
	});
	task1.wait();
	task2.wait();

	const auto& cdata = data;

	auto task3   = async(std::launch::async, [&]() {
        return cdata([](const vector<int>& a_intVec, const list<float>& a_floatList) {
            return make_tuple(accumulate(begin(a_intVec), end(a_intVec), 0),
			                    accumulate(begin(a_floatList), end(a_floatList), 0.0f));
        });
    });
	auto result1 = task3.get();

	REQUIRE(get<0>(result1) == 45);
	REQUIRE(get<1>(result1) == 90);
}

class SemiregularType {
  public:
	SemiregularType()                                  = default;
	~SemiregularType()                                 = default;
	SemiregularType(const SemiregularType&)            = default;
	SemiregularType& operator=(const SemiregularType&) = default;

  private:
	int m_data{0};
};

TEST_CASE("Locked Container") {
	// This code should not compile. if Locked is holding a semi regular type, then locked should also
	// be semi regular and not work with a set.
	// set<Locked<SemiregularType>> setOfLocksSemiregular;
	// setOfLocksSemiregular.insert(Locked<SemiregularType>{});

	// test locked can work with a TotalyOrdered type
	set<Locked<int>> setOfLocks;
	setOfLocks.insert(Locked<int>{2});
	setOfLocks.insert(Locked<int>{10});
	setOfLocks.insert(Locked<int>{15});

	int sum = 0;
	for(const auto& item : setOfLocks) {
		item([&](const auto& arg) { sum += arg; });
	}
	REQUIRE(sum == 27);
}

TEST_CASE("lock multiple Lockeds") {
	Locked<int> data1;
	Locked<vector<int>> data2;

	auto multiLock = MakeMultiLock(data1, data2);

	multiLock([](int& d1, vector<int>& d2) {
		d1 = 1;
		d2.push_back(1);
	});
}

TEST_CASE("Locked try") {
	Locked<vector<int>> data;

	auto task1 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			data.Try([i](vector<int>& a_data) { a_data.push_back(i); });
		}
	});
	auto task2 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			data.Try([i](vector<int>& a_data) { a_data.push_back(i); });
		}
	});
	task1.wait();
	task2.wait();

	const auto& cdata = data;

	auto task3  = async(std::launch::async, [&]() {
        return cdata([](const vector<int>& a_data) { return accumulate(begin(a_data), end(a_data), 0); });
    });
	int result1 = task3.get();

	// Surely some of the try locks should have failed in this case, skipping some of the adds.
	REQUIRE(result1 != 99990000);
}

TEST_CASE("Locked try wait") {
	Locked<vector<int>> data;

	auto task1 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			while(!data.Try([i](vector<int>& a_data) { a_data.push_back(i); }));
		}
	});
	auto task2 = async(std::launch::async, [&]() {
		for(int i = 0; i < 10000; ++i) {
			while(!data.Try([i](vector<int>& a_data) { a_data.push_back(i); }));
		}
	});
	task1.wait();
	task2.wait();

	const auto& cdata = data;

	auto task3  = async(std::launch::async, [&]() {
        return cdata([](const vector<int>& a_data) { return accumulate(begin(a_data), end(a_data), 0); });
    });
	int result1 = task3.get();

	REQUIRE(result1 == 99990000);
}
