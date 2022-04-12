#include <function2/function2.hpp>

import CR.Engine.Platform;

import<chrono>;
import<cstdlib>;
import<thread>;

namespace cep = CR::Engine::Platform;

using namespace std::literals;

int main(int, char*) {
	bool done = false;
	cep::Window window("Conjure", 800, 600, [&done]() { done = true; });

	while(!done) { std::this_thread::sleep_for(16ms); }

	return EXIT_SUCCESS;
}