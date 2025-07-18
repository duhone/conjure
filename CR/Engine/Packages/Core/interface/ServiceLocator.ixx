// Not used at the moment. Services not initialized, so ServiceLocator won't work.
module;

#include <core/Log.h>

export module CR.Engine.Core.ServiceLocator;

import CR.Engine.Core.EightCC;

import std;

export namespace CR::Engine::Core {
	class ServiceLocator final {
	  public:
		ServiceLocator() = default;
		~ServiceLocator();
		ServiceLocator(const ServiceLocator&) = delete;
		ServiceLocator(ServiceLocator&&)      = delete;

		ServiceLocator& operator=(const ServiceLocator&) = delete;
		ServiceLocator& operator=(ServiceLocator&&)      = delete;

		template<typename T, typename... ArgsT>
		T& Add(ArgsT&&... args) {
			CR_ASSERT(!m_services.contains(T::s_typeIndex), "Service {} already added",
			          EightCC(T::s_typeIndex));
			std::unique_ptr<Service> service = std::make_unique<ServiceImpl<T>>(std::forward<ArgsT>(args)...);
			auto result                      = std::launder(reinterpret_cast<T*>(service->GetService()));
			m_services[T::s_typeIndex]       = std::move(service);
			return *result;
		}

		template<typename T>
		T& Get() {
			auto serviceIter = m_services.find(T::s_typeIndex);
			CR_ASSERT(serviceIter != m_services.end(), "Could not find service {}", EightCC(T::s_typeIndex));
			CR_ASSERT_AUDIT(serviceIter->second.get(), "Service {} not constructed", EightCC(T::s_typeIndex));
			return *std::launder(reinterpret_cast<T*>(serviceIter->second->GetService()));
		}

	  private:
		struct Service {
			Service()          = default;
			virtual ~Service() = default;

			virtual std::byte* GetService() = 0;
			virtual void Stop()             = 0;
		};

		template<typename T>
		struct ServiceImpl : public Service {
			template<typename... ArgsT>
			ServiceImpl(ArgsT&&... args) {
				new(buffer) T(std::forward<ArgsT>(args)...);
			}
			virtual ~ServiceImpl() {
				T* t = std::launder(reinterpret_cast<T*>(buffer));
				t->~T();
			}

			std::byte* GetService() override { return buffer; }

			template<typename, typename = void>
			constexpr bool static HasStop = false;
			template<typename T>
			constexpr bool static HasStop<T, std::void_t<decltype(std::declval<T>().Stop())>> = true;

			void Stop() override {
				if constexpr(HasStop<T>) {
					T* t = std::launder(reinterpret_cast<T*>(buffer));
					t->Stop();
				}
			}

			alignas(T) std::byte buffer[sizeof(T)];
		};

		std::unordered_map<std::uint64_t, std::unique_ptr<Service>> m_services;
	};
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;

cecore::ServiceLocator ::~ServiceLocator() {
	for(auto& service : m_services) { service.second->Stop(); }
}