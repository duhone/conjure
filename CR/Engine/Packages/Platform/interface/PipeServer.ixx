module;

export module CR.Engine.Platform.PipeServer;

import std;

export namespace CR::Engine::Platform {
	class PipeServer final {
	  public:
		using WriteFinished_t = std::move_only_function<void(void*, size_t)>;
		using ReadFinished_t  = std::move_only_function<void(void*, size_t)>;

		PipeServer(const char* a_name, PipeServer::WriteFinished_t a_writeFinished,
		           PipeServer::ReadFinished_t a_readFinished);
		~PipeServer() noexcept;
		PipeServer(const PipeServer&)            = delete;
		PipeServer& operator=(const PipeServer&) = delete;
		PipeServer(PipeServer&& a_other) noexcept;
		PipeServer& operator=(PipeServer&& a_other) noexcept;

		void WriteMsg(void* a_msg, size_t a_msgSize);
		template<typename T>
		void WriteMsg(T* a_msg) {
			WriteMsg(a_msg, sizeof(T));
		}

	  private:
		std::unique_ptr<struct PipeServerData> m_data;
	};

}    // namespace CR::Engine::Platform
