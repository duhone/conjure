export module CR.Engine.Core.Random;

import std;
import std.compat;

export namespace CR::Engine::Core {
	void SetSeed(uint64_t seed);
	// generate random number between first and last, closed interval
	int32_t Random(int32_t a_first, int32_t a_last);
	int64_t Random(int64_t a_first, int64_t a_last);
	float Random(float a_first, float a_last);
	double Random(double a_first, double a_last);
}    // namespace CR::Engine::Core