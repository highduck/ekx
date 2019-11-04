#include "common.hpp"

#include <vector>
#include <fstream>

void warm_up() {
    static std::vector<uint32_t> buf_;
    const auto sz = 1'000'000ul;
    for (uint64_t i = 0; i < sz; ++i) {
        buf_.emplace_back(uint32_t(i));
    }
    buf_.clear();
    buf_.shrink_to_fit();
}

int main() {
    warm_up();

    std::ofstream out{"benchmarks.html"};

    run_context_t ctx;

    for (int i = 0; i < 5; ++i) {
        ctx.curr = "ecxx";
        run_ecxx(ctx);
        ctx.curr = "entt";
        run_entt(ctx);
    }

    out << R"PREFIX(<!DOCTYPE HTML><html><head><meta charset="utf-8"><title>Benchmarks</title></head>)PREFIX";
    out << R"PREFIX(<body><table border="1"><caption>Benchmarks</caption>)PREFIX";
    out << R"PREFIX(<tr><th>Test</th><th>ecxx</th><th>entt</th></tr>)PREFIX";

    auto td1 = "<td bgcolor=#00ff00>";
    auto td0 = "<td>";

    for (auto p : ctx.tests) {
        auto& test = p.second;
        auto test_sz = test.info["ecxx"].raw_times.size();
        process_test_info(test.info["ecxx"]);
        process_test_info(test.info["entt"]);

        auto time1 = test.info["ecxx"].avg;
        auto time2 = test.info["entt"].avg;

        out << "<tr>";
        out << "<td>" << p.first << "</td>";
        out << (time1 < time2 ? td1 : td0) << time1 << "</td>";
        out << (time1 > time2 ? td1 : td0) << time2 << "</td>";
        out << "</tr>";
    }

    out << "</table>";


    for (auto p : ctx.tests) {
        auto& test = p.second;
        out << R"PREFIX(<table border="1"><caption>)PREFIX" << p.first
            << R"PREFIX(</caption><tr><th>Test</th><th>#</th><th>ecxx</th><th>entt</th></tr>)PREFIX";
        auto test_sz = test.info["ecxx"].raw_times.size();
        for (int i = 0; i < test_sz; ++i) {
            auto time1 = test.info["ecxx"].raw_times[i];
            auto time2 = test.info["entt"].raw_times[i];
            out << "<tr>";
            out << "<td>" << p.first << "</td>";
            out << "<td>" << i << "</td>";
            out << (time1 < time2 ? td1 : td0) << time1 << "</td>";
            out << (time1 > time2 ? td1 : td0) << time2 << "</td>";
            out << "</tr>";
        }
    }
    out << "</table>";

    out << "</body></html>";
    out.flush();
    out.close();

    return 0;
}