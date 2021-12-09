#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

struct timer final {
    using clock_type = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock_type>;
    using duration_t = std::chrono::duration<double>;

    timer()
            : start_{clock_type::now()} {
    }

    void reset() {
        start_ = clock_type::now();
    }

    double elapsed() {
        auto now = clock_type::now();
        return 1'000 * duration_t(now - start_).count();
    }

private:
    time_point start_;
};

struct test_info_t {
    std::vector<double> raw_times;
    double avg;
    double high;
    double low;
};

inline void process_test_info(test_info_t& info) {
    info.avg = 0.0;
    info.low = 1e16;
    info.high = 0.0;
    for (auto t : info.raw_times) {
        info.avg += t;
        if (t < info.low) info.low = t;
        if (t > info.high) info.high = t;
    }
    info.avg /= static_cast<double>(info.raw_times.size());
}

struct test_results_t {
    std::map<std::string, test_info_t> info;
};

struct run_context_t {

    std::string curr;
    std::string curr_test;
    timer t;
    std::map<std::string, test_results_t> tests;

    double elapsed = 0.0;

    template<typename Fn>
    void run(const char* name, Fn&& fn) {
        curr_test = name;
        elapsed = 0.0;
        fn(*this);
        tests[curr_test].info[curr].raw_times.emplace_back(elapsed);
    }

    template<typename Fn>
    void measure(Fn&& fn) {
        start();
        fn();
        stop();
    }

    void start() {
        t.reset();
    }

    void stop() {
        elapsed += t.elapsed();
        t.reset();
    }
};

void run_ecxx(run_context_t&);

void run_entt(run_context_t&);