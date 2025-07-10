module CR.Engine.Core.Random;

import std;

namespace {
	struct RandomInt32 {
		RandomInt32() {
			std::random_device device;
			Generator.seed(device());
		}
		std::mt19937 Generator;
	};
	struct RandomInt64 {
		RandomInt64() {
			std::random_device device;
			Generator.seed(device());
		}
		std::mt19937_64 Generator;
	};

	RandomInt32& GetInt32Engine() {
		static RandomInt32 engine;
		return engine;
	}

	RandomInt64& GetInt64Engine() {
		static RandomInt64 engine;
		return engine;
	}
}    // namespace

int32_t CR::Engine::Core::Random(int32_t a_first, int32_t a_last) {
	std::uniform_int_distribution<int32_t> dis(a_first, a_last);
	return dis(GetInt32Engine().Generator);
}

int64_t CR::Engine::Core::Random(int64_t a_first, int64_t a_last) {
	std::uniform_int_distribution<int64_t> dis(a_first, a_last);
	return dis(GetInt64Engine().Generator);
}

float CR::Engine::Core::Random(float a_first, float a_last) {
	std::uniform_real_distribution<float> dis(a_first, a_last);
	return dis(GetInt32Engine().Generator);
}

double CR::Engine::Core::Random(double a_first, double a_last) {
	std::uniform_real_distribution<double> dis(a_first, a_last);
	return dis(GetInt64Engine().Generator);
}

void CR::Engine::Core::SetSeed(uint64_t a_seed) {
	GetInt32Engine().Generator.seed((uint32_t)a_seed);
	GetInt64Engine().Generator.seed(a_seed);
}
