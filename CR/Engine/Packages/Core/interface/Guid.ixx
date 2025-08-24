module;

#include <core/Log.h>

export module CR.Engine.Core.Guid;

import CR.Engine.Core.Algorithm;
import CR.Engine.Core.Log;

import std;
import std.compat;

export namespace CR::Engine::Core {
	class Guid final {
	  public:
		Guid(uint32_t a_data1, uint32_t a_data2, uint32_t a_data3, uint32_t a_data4) {
			m_data[0] = a_data1;
			m_data[1] = a_data2;
			m_data[2] = a_data3;
			m_data[3] = a_data4;
		}
		~Guid() = default;

		Guid(const Guid& a_other)     = default;
		Guid(Guid&& a_other) noexcept = default;

		template<typename T>
		Guid(const T& a_data) {
			Set(a_data);
		}
		Guid& operator=(const Guid& a_other)     = default;
		Guid& operator=(Guid&& a_other) noexcept = default;

		Guid() : Guid(0, 0, 0, 0) {}

		auto operator<=>(const Guid& a_other) const = default;

		static const Guid& Null() {
			static Guid null = Guid();
			return null;
		}

		[[nodiscard]] bool IsNull() const { return *this == Null(); }

		//! works with strings, wstrings, char arrays, wchar_t arrays
		template<typename T>
		void Set(const T& a_data) {
			*this         = Null();
			int count     = 0;
			auto iterator = std::begin(a_data);
			while(iterator != std::end(a_data)) {
				int index = (count & 0x0ff8) >> 3;
				if((*iterator) >= 48 && (*iterator) <= 57) {
					m_data[index] = (m_data[index] << 4) + ((*iterator) - 48);
					count++;
				}
				if((*iterator) >= 65 && (*iterator) <= 71) {
					m_data[index] = (m_data[index] << 4) + ((*iterator) - 55);
					count++;
				}
				if((*iterator) >= 97 && (*iterator) <= 102) {
					m_data[index] = (m_data[index] << 4) + ((*iterator) - 87);
					count++;
				}
				iterator++;
			}
			CR_ASSERT(
			    count == 32,
			    "guids must have 32 chars that are 0-9, a-f, or A-F, other chars are allowed but ignored");
		}

		[[nodiscard]] uint32_t Data1() const { return m_data[0]; }
		[[nodiscard]] uint32_t Data2() const { return m_data[1]; }
		[[nodiscard]] uint32_t Data3() const { return m_data[2]; }
		[[nodiscard]] uint32_t Data4() const { return m_data[3]; }

		unsigned int operator[](int a_index) const {
			CR_ASSERT(a_index >= 0 && a_index < 4, "Index out of range");
			return m_data[a_index];
		}

		operator std::string() const {
			static const char translation[] = {'0', '1', '2', '3', '4', '5', '6', '7',
			                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
			return Convert<std::string::value_type, std::string::traits_type>(translation, '{', '-', '}');
		}

		operator std::wstring() const {
			static const wchar_t translation[] = {L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
			                                      L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};
			return Convert<std::wstring::value_type, std::wstring::traits_type>(translation, L'{', L'-',
			                                                                    L'}');
		}

		[[nodiscard]] std::string ToStringClean() const {
			static const char translation[] = {'0', '1', '2', '3', '4', '5', '6', '7',
			                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
			return Convert<std::string::value_type, std::string::traits_type>(translation, '\0', '\0', '\0');
		}

	  private:
		template<typename CharT, typename Traits>
		std::basic_string<CharT, Traits> Convert(const CharT* a_translation, CharT a_firstChar,
		                                         CharT a_seperatorChar, CharT a_lastChar) const {
			std::basic_string<CharT, Traits> result;
			auto conversion = [&](unsigned short a_data) {
				result += a_translation[(a_data >> 12) & 0xf];
				result += a_translation[(a_data >> 8) & 0xf];
				result += a_translation[(a_data >> 4) & 0xf];
				result += a_translation[a_data & 0xf];
			};

			auto cdata = reinterpret_cast<const uint16_t*>(m_data);
			// little endian
			if(a_firstChar) { result += a_firstChar; }
			conversion(cdata[1]);
			conversion(cdata[0]);
			if(a_seperatorChar) { result += a_seperatorChar; }
			conversion(cdata[3]);
			if(a_seperatorChar) { result += a_seperatorChar; }
			conversion(cdata[2]);
			if(a_seperatorChar) { result += a_seperatorChar; }
			conversion(cdata[5]);
			if(a_seperatorChar) { result += a_seperatorChar; }
			conversion(cdata[4]);
			conversion(cdata[7]);
			conversion(cdata[6]);
			if(a_lastChar) { result += a_lastChar; }
			return result;
		}

		uint32_t m_data[4];
	};
}    // namespace CR::Engine::Core

export {
	template<typename CharT, typename Traits>
	inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& a_strm,
	                                                     const CR::Engine::Core::Guid& a_guid) {
		a_strm << static_cast<std::basic_string<CharT, Traits>>(a_guid);
		return a_strm;
	}

	template<>
	struct std::hash<CR::Engine::Core::Guid> {
		std::size_t operator()(const CR::Engine::Core::Guid& a_guid) const {
			return std::hash<unsigned int>()(a_guid.Data1()) ^ std::hash<unsigned int>()(a_guid.Data2()) ^
			       std::hash<unsigned int>()(a_guid.Data3()) ^ std::hash<unsigned int>()(a_guid.Data4());
		}
	};
}
