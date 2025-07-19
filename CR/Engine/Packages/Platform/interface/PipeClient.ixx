module;

export module CR.Engine.Platform.PipeClient;

import std;

namespace CR::Engine::Platform {
	export class PipeClient final {
	  public:
		using MsgHandlerT = std::move_only_function<void(void*, size_t)>;

		PipeClient() = default;
		PipeClient(const char* a_name, PipeClient::MsgHandlerT a_msgHandler);
		~PipeClient();
		PipeClient(const PipeClient&)            = delete;
		PipeClient& operator=(const PipeClient&) = delete;
		PipeClient(PipeClient&&) noexcept;
		PipeClient& operator=(PipeClient&&) noexcept;

		// For now sends are blocking/synchronous. Design is currently around small msgs. For bulk data
		// use shared memory. recieving msgs is async though.
		void SendPipeMessage(const void* a_msg, size_t a_msgSize);
		template<typename MsgT>
		void SendPipeMessage(const MsgT& a_msg) {
			static_assert(std::is_standard_layout<MsgT>::value, "Messages should be pod types");
			SendPipeMessage(&a_msg, sizeof(a_msg));
		}

	  private:
		void RunMsgHandler();

		std::unique_ptr<struct PipeClientData> m_data;
	};
}    // namespace CR::Engine::Platform
