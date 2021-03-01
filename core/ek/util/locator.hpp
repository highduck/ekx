#pragma once

#include "../assert.hpp"

namespace ek {

template<class Service>
class service_locator_instance {
public:
    service_locator_instance() = delete;

    ~service_locator_instance() = delete;

    inline static Service& get() {
        EK_ASSERT(value_);
        return *value_;
    }

    inline static Service* try_get() {
        return value_;
    }

    template<typename ServiceImpl = Service, typename... Args>
    inline static Service& init(Args&& ...args) {
        EK_ASSERT(value_ == nullptr);
        value_ = new ServiceImpl(args...);
        return *value_;
    }

    inline static void shutdown() {
        EK_ASSERT(value_ != nullptr);
        delete value_;
        value_ = nullptr;
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
