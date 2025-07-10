export module CR.Engine.Platform.Process;

import<chrono>;
import<memory>;
import<optional>;

namespace CR::Engine::Platform {
	export class Process final {
	  public:
		Process() = default;
		Process(const char* a_executablePath, const char* a_commandLine);
		~Process();
		Process(const Process&) = delete;
		Process& operator=(const Process&) = delete;
		Process(Process&& a_other) noexcept;
		Process& operator=(Process&& a_other) noexcept;

		// Returns false if timed out waiting for process to close, or if some other error occured
		bool WaitForClose(const std::chrono::milliseconds& a_maxWait);

		[[nodiscard]] std::optional<int32_t> GetExitCode() const;

	  private:
		std::unique_ptr<struct ProcessData> m_data;
	};
}    // namespace CR::Engine::Platform
