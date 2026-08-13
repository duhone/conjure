#include <doctest/doctest.h>

import CR.Engine.Core.Guid;

import std;

using namespace CR::Engine::Core;

TEST_CASE("guid") {
	// doctest problems with c++23 modules. can't see the guid ostream << overload
	/*auto nullGuid = Guid::Null();
	REQUIRE(nullGuid == Guid(0, 0, 0, 0));

	auto charGuid = Guid("{70C74876-2ABC-4B9A-A2D4-A8794F055A35}");
	REQUIRE(charGuid.Data1() == 0x70c74876);
	REQUIRE(charGuid.Data2() == 0x2abc4b9a);
	REQUIRE(charGuid.Data3() == 0xa2d4a879);
	REQUIRE(charGuid.Data4() == 0x4f055a35);
	REQUIRE(charGuid == Guid(0x70c74876, 0x2abc4b9a, 0xa2d4a879, 0x4f055a35));
	REQUIRE(charGuid != Guid(0x70c74875, 0x2abc4b9a, 0xa2d4a879, 0x4f055a35));

	auto wcharGuid = Guid(L"{70C74876-2ABC-4B9A-A2D4-A8794F055A35}");
	REQUIRE(wcharGuid == charGuid);

	auto stringGuid = Guid(std::string("{70C74876-2ABC-4B9A-A2D4-A8794F055A35}"));
	REQUIRE(stringGuid == charGuid);

	std::set<Guid> guidsSet = {nullGuid, charGuid};
	auto foundSet           = guidsSet.find(charGuid) != end(guidsSet);
	REQUIRE(foundSet == true);

	// visual studio bug, doesnt compile.
	// std::unordered_set<Guid> guidsUnorderedSet = {nullGuid, charGuid};
	// auto foundUSet                             = guidsUnorderedSet.find(charGuid) !=
	// end(guidsUnorderedSet); REQUIRE(foundUSet == true);

	std::stringstream stream;
	stream << charGuid;
	REQUIRE(stream.str() == std::string("{70C74876-2ABC-4B9A-A2D4-A8794F055A35}"));

	std::wstringstream wstream;
	wstream << charGuid;
	REQUIRE(wstream.str() == std::wstring(L"{70C74876-2ABC-4B9A-A2D4-A8794F055A35}"));
	*/
}
