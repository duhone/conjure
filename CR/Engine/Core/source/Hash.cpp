module CR.Engine.Core.Hash;

using namespace std;

namespace {
	uint32_t Get16(byte* a_data) { return ((uint32_t)a_data[0]) | (((uint32_t)a_data[1]) << 8); }
}    // namespace

uint32_t CR::Engine::Core::HashFast(std::span<std::byte> a_data) {
	byte* data = a_data.data();
	auto hash  = (uint32_t)a_data.size();
	auto left  = (uint32_t)a_data.size();

	if(left == 0) { return 0; }

	for(; left > 3; left -= 4) {
		uint32_t value;
		hash += Get16(data);
		value = (Get16(data + 2) << 11);
		hash  = (hash << 16) ^ hash ^ value;
		data += 4;
		hash += hash >> 11;
	}

	switch(left) {
		case 3:
			hash += Get16(data);
			hash ^= hash << 16;
			hash ^= ((uint32_t)data[2]) << 18;
			hash += hash >> 11;
			break;
		case 2:
			hash += Get16(data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1:
			hash += (uint32_t)data[0];
			hash ^= hash << 10;
			hash += hash >> 1;
			break;
		case 0:
			break;
	}

	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}
