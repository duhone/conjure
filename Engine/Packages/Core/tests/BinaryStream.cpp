import CR.Engine.Core.BinaryStream;

#include <doctest/doctest.h>

import std;

using namespace std;
using namespace CR::Engine::Core;

TEST_CASE("BinaryStream") {
	struct Foo {
		uint32_t a;
		float b;
	};
	vector<byte> stream;

	Write(stream, 4);
	Write(stream, 3.14f);
	Write(stream, 2.02);
	Foo foo;
	foo.a = 537;
	foo.b = 49.024f;
	Write(stream, foo);

	vector<uint32_t> list{8, 54, 38, 52, 87, 3};
	Write(stream, list);

	BinaryReader reader{stream};
	int one = 0;
	Read(reader, one);
	CHECK(one == 4);

	float two;
	Read(reader, two);
	CHECK(two == doctest::Approx{3.14f});

	double three;
	Read(reader, three);
	CHECK(three == doctest::Approx{2.02});

	Foo four;
	Read(reader, four);
	CHECK(four.a == 537);
	CHECK(four.b == doctest::Approx{49.024f});

	vector<uint32_t> five;
	Read(reader, five);
	CHECK(five.size() == 6);
	CHECK(five[0] == 8);
	CHECK(five[1] == 54);
	CHECK(five[2] == 38);
	CHECK(five[3] == 52);
	CHECK(five[4] == 87);
	CHECK(five[5] == 3);
}
