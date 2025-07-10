module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.VertexLayout;

import CR.Engine.Graphics.Formats;

import <span>;
import <vector>;

export namespace CR::Engine::Graphics {
	// Source vertex struct must be tightly packed.
	class VertexLayout {
	  public:
		VertexLayout()                                  = default;
		~VertexLayout()                                 = default;
		VertexLayout(VertexLayout&)                     = delete;
		VertexLayout(VertexLayout&& a_other)            = delete;
		VertexLayout& operator=(VertexLayout&)          = delete;
		VertexLayout& operator=(VertexLayout&& a_other) = delete;

		struct Entry {
			VkFormat format{VK_FORMAT_UNDEFINED};
			uint16_t Offset{0};
			uint8_t Location{0};
		};

		// Someday we will have reflection. For now create a dummy struct of your vertex layout(must be the
		// proper types that match your compressed layout, UNorm2, SNorm4, ect), and then pass each member
		// variable, in order to this function(error prone, but aimed at switching to reflection some day). In
		// your shader, the input locations must be in same order as the struct.
		template<typename T>
		void AddVariable(const T& a_var) noexcept {
			Entry& entry   = m_layout.emplace_back();
			entry.Location = m_nextLocation++;
			entry.Offset   = m_nextOffset;
			m_nextOffset += sizeof(a_var);
			entry.format = Formats::GetVKFormat(a_var);
		}

		// Of a single vert
		[[nodiscard]] uint32_t GetSizeBytes() const noexcept { return m_nextOffset; }

		std::span<const Entry> GetLayout() const { return m_layout; }

	  private:
		std::vector<Entry> m_layout;
		uint8_t m_nextLocation{0};
		uint16_t m_nextOffset{0};
	};
}    // namespace CR::Engine::Graphics
