#pragma once

#include <type_traits>
#include <cassert>
#include <utility>

namespace ek {

template<class Service>
class service_locator_instance {
public:
    service_locator_instance() = delete;

    ~service_locator_instance() = delete;

    inline static Service& get() {
        assert(value_);
        return *value_;
    }

    inline static Service* try_get() {
        return value_;
    }

    template<typename ServiceImpl = Service, typename... Args>
    inline static Service& init(Args&& ...args) {
        assert(value_ == nullptr);
        value_ = new ServiceImpl(std::forward<Args>(args)...);
        return *value_;
    }

private:
    static Service* value_;
};

template<class Service>
Service* service_locator_instance<Service>::value_ = nullptr;

template<class Service>
inline Service& resolve() {
    return service_locator_instance<Service>::get();
}

template<class Service>
inline Service* try_resolve() {
    return service_locator_instance<Service>::try_get();
}

}
