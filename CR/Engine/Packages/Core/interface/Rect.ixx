module;

#include <glm/glm.hpp>

export module CR.Engine.Core.Rect;

import std;

export namespace CR::Engine::Core {
	template<typename T>
	struct Rect2D {
		glm::vec<2, T> Position;
		glm::vec<2, T> Size;

		bool Contains(const glm::vec<2, T>& a_point) const noexcept {
			if(a_point.x < Position.x) return false;
			if(a_point.x >= Position.x + Size.x) return false;
			if(a_point.y < Position.y) return false;
			if(a_point.y >= Position.y + Size.y) return false;
			return true;
		}
	};

	using Rect2Di32 = Rect2D<std::int32_t>;
	using Rect2Df32 = Rect2D<float>;

}    // namespace CR::Engine::Core
