module;

#include <core/Log.h>

export module CR.Engine.Core.BinaryStream;

import CR.Engine.Core.FileHandle;
import CR.Engine.Core.Log;

import std;
import std.compat;

export namespace CR::Engine::Core {
	// Returns offset in vector where argument was written
	template<std::semiregular T>
	size_t Write(std::vector<std::byte>& a_stream, const T& a_arg) {
		if constexpr(std::is_trivially_copyable_v<T>) {
			auto offset = a_stream.size();
			a_stream.resize(a_stream.size() + sizeof(T));
			memcpy(a_stream.data() + offset, &a_arg, sizeof(T));
			return offset;
		} else {
			Write(a_stream, (std::uint32_t)a_arg.size());
			auto offset = a_stream.size();
			a_stream.resize(a_stream.size() + a_arg.size() * sizeof(T::value_type));
			memcpy(a_stream.data() + offset, a_arg.data(), a_arg.size() * sizeof(T::value_type));
			return offset;
		}
	}

	template<std::semiregular T>
	void Write(FileHandle& a_file, const T& a_arg) {
		if constexpr(std::is_trivially_copyable_v<T>) {
			fwrite(&a_arg, sizeof(T), 1, a_file.asFile());
		} else {
			Write(a_file, (uint32_t)a_arg.size());
			fwrite(a_arg.data(), sizeof(T::value_type), a_arg.size(), a_file);
		}
	}

	// Reading should always come from a memory mapped file, so less help is needed.
	struct BinaryReader final {
		BinaryReader() = default;
		// input stream must outlive the reader
		BinaryReader(const std::span<std::byte>& a_stream) :
		    Data(a_stream.data()), Size((uint32_t)a_stream.size()) {}

		const std::byte* Data{nullptr};
		uint32_t Offset{0};
		uint32_t Size{0};
	};

	// returns true if was able to read T, false other wise, which is not an error if the previous read ended
	// at exactly the end of the stream.
	template<std::semiregular T>
	bool Read(BinaryReader& a_stream, T& a_out) {
		if(a_stream.Offset == a_stream.Size) { return false; }
		if constexpr(std::is_trivially_copyable_v<T>) {
			CR_ASSERT(a_stream.Offset + sizeof(T) <= a_stream.Size,
			          "Tried to read past the end of the buffer");
			memcpy(&a_out, a_stream.Data + a_stream.Offset, sizeof(T));
			a_stream.Offset += sizeof(T);
			return true;
		} else {
			uint32_t outSize = 0;
			Read(a_stream, outSize);

			CR_ASSERT(a_stream.Offset + outSize * sizeof(typename T::value_type) <= a_stream.Size,
			          "Tried to read past the end of the buffer");

			a_out.resize(outSize);
			memcpy(a_out.data(), a_stream.Data + a_stream.Offset, outSize * sizeof(T::value_type));
			a_stream.Offset += outSize * sizeof(T::value_type);
			return true;
		}
	}
}    // namespace CR::Engine::Core
