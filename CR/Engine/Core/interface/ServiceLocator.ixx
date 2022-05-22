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
			CR_ASSERT(!m_services.contains(std::type_index(typeid(T))), "Service {} already added",
			          typeid(T).name());
			std::any service(std::in_place_type_t<std::shared_ptr<T>>{},
			                 std::make_shared<T>(std::forward<ArgsT>(args)...));
			m_services[std::type_index(typeid(T))] = std::move(service);
		}

		template<typename T>
		T& Get() {
			auto serviceIter = m_services.find(std::type_index(typeid(T)));
			CR_ASSERT(serviceIter != m_services.end(), "Could not find service {}", typeid(T).name());
			CR_ASSERT_AUDIT(serviceIter->second.has_value(), "Service {} not constructed", typeid(T).name());
			CR_ASSERT_AUDIT(serviceIter->second.type() == typeid(std::shared_ptr<T>),
			                "Service {} has unexpected type", typeid(T).name());
			return **std::any_cast<std::shared_ptr<T>>(&serviceIter->second);
		}

	  private:
		std::unordered_map<std::type_index, std::any> m_services;
	};
}    // namespace CR::Engine::Core
