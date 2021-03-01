#pragma once

#include <vector>
#include <functional>

namespace ek {

// inspired by https://github.com/klmr/multifunction

template<typename ...Args>
class signal_t {
public:

    using token = unsigned;

    template<typename Fn>
    token add(Fn listener) {
//        assert(!locked_);
        invocations_.push_back(listener);
        auto tk = static_cast<token>(tokens_.size());
        tokens_.push_back(invocations_.size());
        return tk;
    }

    template<typename Fn>
    token add_once(Fn listener) {
//        assert(!locked_);
        invocations_.push_back(listener);
        auto tk = static_cast<token>(tokens_.size());
        tokens_.push_back(invocations_.size());
        once_map_.push_back(tk);
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

    void emit(Args... args) {
        auto prevOnceCount = once_map_.size();
        auto prevInvocationsCount = invocations_.size();
        for (size_t i = 0; i < prevInvocationsCount; ++i) {
            std::function<void(Args...)> cp{invocations_[i]};
            cp(args...);
        }
        for (size_t i = 0; i < prevOnceCount; ++i) {
            auto onceToken = once_map_[i];
            remove(onceToken);
        }
        once_map_.erase(once_map_.begin(), once_map_.begin() + prevOnceCount);
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

    inline void operator()(Args...args) {
        emit(args...);
    }

    signal_t() = default;

    signal_t(signal_t&& mf) noexcept = default;

    signal_t(const signal_t& mf) noexcept = default;

    signal_t& operator=(signal_t&& mf) noexcept = default;

    signal_t& operator=(const signal_t& mf) noexcept = default;

private:
    std::vector<std::function<void(Args...)>> invocations_;
    std::vector<token> once_map_;
    std::vector<size_t> tokens_;
};

}


