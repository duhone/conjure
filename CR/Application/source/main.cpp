#include <function2/function2.hpp>

import CR.Engine.Audio;
import CR.Engine.Core;
import CR.Engine.Platform;

import<chrono>;
import<cstdlib>;
import<thread>;

namespace cea = CR::Engine::Audio;
namespace cec = CR::Engine::Core;
namespace cep = CR::Engine::Platform;

using namespace std::literals;

int main(int, char*) {
	cec::LogSystem logSystem;

	bool done = false;
	cep::Window window("Conjure", 800, 600, [&done]() { done = true; });

	cea::EngineStart();

	while(!done) { std::this_thread::sleep_for(16ms); }

	cea::EngineStop();

	return EXIT_SUCCESS;
}