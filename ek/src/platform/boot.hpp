#pragma once

#include <string>
#include <vector>

#if defined(__ANDROID__)

#define EK_MAIN_ENTRY_POINT ek_android_main__

#elif defined(_WIN32) || defined(_WIN64)

#define EK_MAIN_ENTRY_POINT WinMain

#else

#define EK_MAIN_ENTRY_POINT main

#endif

#ifndef EK_NO_MAIN

#if !(defined(_WIN32) || defined(_WIN64))
int EK_MAIN_ENTRY_POINT(int argc, char* argv[]);
#endif

#endif

namespace ek {

/*
    WEB

    Module['arguments'].push('first_param');
    Module['arguments'].push('second_param');

    int main(int argc, char *argv[]) {
        assert(argc == 3);
        assert(strcmp(argv[1], "first_param") == 0);
        assert(strcmp(argv[2], "second_param") == 0);
    }
*/

struct program_arguments_t {
    int argc = 0;
    char** argv = nullptr;

    program_arguments_t() = default;

    void assign(int argc_, char** argv_) {
        argc = argc_;
        argv = argv_;
    }

    [[nodiscard]] std::vector<std::string> to_vector() const;
};

const program_arguments_t& get_program_arguments();

#ifndef EK_NO_MAIN

void main();

#endif

}
