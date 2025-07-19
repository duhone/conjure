module;

export module CR.Engine.Platform.SharedLibrary;

import CR.Engine.Core;

import std;

export namespace CR::Engine::Platform {
	class SharedLibrary final {
	  public:
		SharedLibrary() = default;
		SharedLibrary(std::string_view a_libraryName);
		~SharedLibrary();
		SharedLibrary(const SharedLibrary&)            = delete;
		SharedLibrary& operator=(const SharedLibrary&) = delete;
		SharedLibrary(SharedLibrary&& a_other) noexcept;
		SharedLibrary& operator=(SharedLibrary&& a_other) noexcept;

		void* GetFunction(std::string_view a_functionName) const;

		template<typename FunctionPtr_t>
		auto GetUniqueFunction(std::string_view a_functionName) const {
			return std::move_only_function<FunctionPtr_t>{
			    static_cast<Core::GetFunctionPtrType_t<FunctionPtr_t>>(GetFunction(a_functionName))};
		}

	  private:
		std::unique_ptr<struct SharedLibraryData> m_data;
	};
}    // namespace CR::Engine::Platform
