#pragma once

#include <vector>
#include <functional>

namespace ek {

// inspired by https://github.com/klmr/multifunction
namespace multifunction_details {
template<typename R, typename...Args>
struct multicall {
    static R call(
            const std::vector<std::function<R(Args...)>>& invokations,
            Args... args
    ) {
        R ret;
        for (auto listener : invokations) {
            ret = listener(args...);
        }
        return ret;
    }
};

template<typename...Args>
struct multicall<void, Args...> {
    static void call(
            const std::vector<std::function<void(Args...)>>& invokations,
            Args... args
    ) {
        for (auto listener : invokations) {
            listener(args...);
        }
    }
};
}

template<typename R, typename ...Args>
class multifunction {
public:

    using token = unsigned;

    template<typename Fn>
    token add(Fn listener) {
//        assert(!locked_);
        invocations_.emplace_back(listener);
        auto tk = static_cast<token>(tokens_.size());
        tokens_.emplace_back(invocations_.size());
        return tk;
    }

    template<typename Fn>
    token add_once(Fn listener) {
//        assert(!locked_);
        invocations_.emplace_back(listener);
        auto tk = static_cast<token>(tokens_.size());
        tokens_.emplace_back(invocations_.size());
        once_map_.emplace_back(tk);
        return tk;
    }

    bool remove(token tk) {
        size_t i = tokens_[tk];
        if (i) {
            for (auto& tk_ref : tokens_) {
                if (tk_ref && tk_ref > i) {
                    --tk_ref;
                }
            }
            invocations_.erase(invocations_.begin() + i - 1);
            tokens_[tk] = 0;
        }
        return i != 0;
    }

    R emit(Args... args) {
        std::vector<std::function<R(Args...)>> cp = invocations_;
        for (auto once_token : once_map_) {
            remove(once_token);
        }
        return multifunction_details::multicall<R, Args...>::call(cp, args...);
    }

    void clear() {
        invocations_.clear();
        once_map_.clear();
    }

    template<typename Fn>
    inline auto& operator+=(Fn invocation) {
        add(invocation);
        return *this;
    }

    template<typename Fn>
    inline auto& operator<<(Fn invocation) {
        add_once(invocation);
        return *this;
    }

    inline auto& operator-=(token tk) {
        remove(tk);
        return *this;
    }

    inline R operator()(Args...args) {
        return emit(args...);
    }

private:
    std::vector<std::function<R(Args...)>> invocations_;
    std::vector<token> once_map_;
    std::vector<size_t> tokens_;
};

template<typename ...Args>
using signal_t = multifunction<void, Args...>;

}


