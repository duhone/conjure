module;

#include "core/Log.h"

export module CR.Engine.Graphics.Utils;

import <cstring>;

namespace CR::Engine::Graphics {
	export template<typename T>
	void ClearStruct(T& value) {
		memset(&value, 0, sizeof(T));
	}
}    // namespace CR::Engine::Graphics