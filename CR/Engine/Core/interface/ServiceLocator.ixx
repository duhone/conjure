module;

#include <core/Log.h>

export module CR.Engine.Core.ServiceLocator;

import<any>;
import<memory>;
import<typeinfo>;
import<typeindex>;
import<unordered_map>;

namespace CR::Engine::Core {
	export class ServiceLocator final {
	  public:
		ServiceLocator()                      = default;
		~ServiceLocator()                     = default;
		ServiceLocator(const ServiceLocator&) = delete;
		ServiceLocator(ServiceLocator&&)      = delete;

		ServiceLocator& operator=(const ServiceLocator&) = delete;
		ServiceLocator& operator=(ServiceLocator&&) = delete;

		template<typename T, typename... ArgsT>
		void Add(ArgsT&&... args) {
			CR_ASSERT(!m_services.contains(T::s_typeIndex), "Service {} already added",
			          T::s_typeIndex.name());
			std::unique_ptr<Service> service = std::make_unique<ServiceImpl<T>>(std::forward<ArgsT>(args)...);
			m_services[T::s_typeIndex]       = std::move(service);
		}

		template<typename T>
		T& Get() {
			auto serviceIter = m_services.find(T::s_typeIndex);
			CR_ASSERT(serviceIter != m_services.end(), "Could not find service {}", T::s_typeIndex.name());
			CR_ASSERT_AUDIT(serviceIter->second.get(), "Service {} not constructed", T::s_typeIndex.name());
			return *std::launder(reinterpret_cast<T*>(serviceIter->second->GetService()));
		}

	  private:
		struct Service {
			Service()          = default;
			virtual ~Service() = default;

			virtual std::byte* GetService() = 0;
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

			alignas(T) std::byte buffer[sizeof(T)];
		};

		std::unordered_map<std::type_index, std::unique_ptr<Service>> m_services;
	};
}    // namespace CR::Engine::Core
