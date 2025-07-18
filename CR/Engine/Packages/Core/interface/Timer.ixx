export module CR.Engine.Core.Timer;

import std;

export namespace CR::Engine::Core {
	//! A medium resolution timer.
	/*!
	    Uses high resolution performance counter to implement a timer. Medium precession, could switch to
	   rdtsc on PC, need to figure out the equivalent on android/mac. Use tracey for profiling, not this. All
	   returned values are in seconds. You should only call Update once per frame. The LastFrameTime is always
	   the amount of time between the last 2 calls to update.

	    Generally use GetTotalTime and Reset to keep track of the amount of time of a single event.
	    Use Update only when creating a frame rate type counter.

	    Oldest code in conjure, decades.
	    */
	class Timer final {
	  public:
		Timer();
		//! Total time since timer was created or Reset was called
		/*!
		    This will be set back to 0 every time Reset is called.
		    */
		[[nodiscard]] double GetTotalTime() const { return totalTime; }
		//! Time between last 2 calls to Update
		/*!
		    Don't use Reset if using this call and Update. Reset will set this back
		    to 0, and it won't change from that until after the next Update call.
		    */
		[[nodiscard]] double GetLastFrameTime() const { return timeLastFrame; }
		//! Reset the total time back to 0.
		void Reset();
		void StartFrame();
		//! Next Frame. Update the LastFrameTime
		void Update();

	  private:
		std::chrono::high_resolution_clock::time_point starttime;
		std::chrono::high_resolution_clock::time_point currenttime;

		double timerFreqInv{0.0};
		double totalTime{0.0};
		double timeLastFrame{0.0};
	};

	class ScopedTimer final {
	  public:
		ScopedTimer(const char* text);
		~ScopedTimer();

		ScopedTimer(const ScopedTimer&)            = delete;
		ScopedTimer(ScopedTimer&&)                 = delete;
		ScopedTimer& operator=(const ScopedTimer&) = delete;
		ScopedTimer& operator=(ScopedTimer&&)      = delete;

	  private:
		Timer m_timer;
		std::string m_text;
	};
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;

cecore::Timer::Timer() {
	Reset();
}

/*
    Reset the total time back to 0. Also resets the last frame time back to 0.
    Generally only used when total time is needed.
*/
void cecore::Timer::Reset() {
	starttime = std::chrono::high_resolution_clock::now();
}

/*
    Updates the last frame time, and the total time.
*/
void cecore::Timer::Update() {
	currenttime = std::chrono::high_resolution_clock::now();
	timeLastFrame =
	    std::chrono::duration_cast<std::chrono::microseconds>(currenttime - starttime).count() / 1000000.0;
	starttime = currenttime;
	totalTime += timeLastFrame;
}

void cecore::Timer::StartFrame() {
	starttime = std::chrono::high_resolution_clock::now();
}

cecore::ScopedTimer::ScopedTimer(const char* text) : m_text(text) {}

cecore::ScopedTimer::~ScopedTimer() {
	m_timer.Update();
	std::println("{} {:.2f}ms", m_text, (m_timer.GetTotalTime() * 1000));
}
