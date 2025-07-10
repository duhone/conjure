module;

module CR.Engine.Core.Timer;

using namespace std;
using namespace CR::Engine::Core;

Timer::Timer() {
	Reset();
}

/*
    Reset the total time back to 0. Also resets the last frame time back to 0.
    Generally only used when total time is needed.
*/
void Timer::Reset() {
	starttime = chrono::high_resolution_clock::now();
}

/*
    Updates the last frame time, and the total time.
*/
void Timer::Update() {
	currenttime   = chrono::high_resolution_clock::now();
	timeLastFrame = chrono::duration_cast<chrono::microseconds>(currenttime - starttime).count() / 1000000.0;
	starttime     = currenttime;
	totalTime += timeLastFrame;
}

void Timer::StartFrame() {
	starttime = chrono::high_resolution_clock::now();
}

ScopedTimer::ScopedTimer(const char* text) : m_text(text) {}

ScopedTimer::~ScopedTimer() {
	m_timer.Update();
	std::println("{} {:.2f}ms", m_text, (m_timer.GetTotalTime() * 1000));
}
