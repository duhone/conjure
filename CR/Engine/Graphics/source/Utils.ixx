module;

#include "core/Log.h"

#include "Core.h"
#include <vulkan/utility/vk_struct_helper.hpp>

#include "core/Reflection.h"

export module CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <cstring>;
import <type_traits>;

ReflectMember(sType);

namespace CR::Engine::Graphics {
	export template<typename T>
	inline void ClearStruct(T& value) {
		memset(&value, 0, sizeof(T));
		if constexpr(HasMembersType_v<T>) { value.sType = vku::GetSType<T>(); }
	}

	export template<typename T, int SIZE>
	inline void ClearStruct(T a_values[SIZE]) {
		for(const T& val : a_values) { ClearStruct(val); }
	}

}    // namespace CR::Engine::Graphics