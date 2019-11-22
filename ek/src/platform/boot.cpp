#include "boot.hpp"

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif

namespace ek {

program_arguments_t program_arguments_{};

const program_arguments_t& get_program_arguments() {
    return program_arguments_;
}

std::vector<std::string> program_arguments_t::to_vector() const {
    std::vector<std::string> result;
    for (int i = 0; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }
    return result;
}

}

/** Declare MAIN entry point if EK_NO_MAIN is not declared.
 * Disable MAIN if you want to use third-party or test framework or your own custom MAIN entry
 */
#ifndef EK_NO_MAIN

#if defined(_WIN32) || defined(_WIN64)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // TODO: argument parsing
    ::ek::main();
    return 0;
}

#else

int EK_MAIN_ENTRY_POINT(int argc, char* argv[]) {
    ::ek::program_arguments_.assign(argc, argv);
    ::ek::main();
    return 0;
}

#endif

#endif