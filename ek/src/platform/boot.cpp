#include "boot.h"

namespace ek {

program_arguments_type program_arguments_;

void load_program_arguments(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        program_arguments_.emplace_back(argv[i]);
    }
}

program_arguments_type& get_program_arguments() {
    return program_arguments_;
}

program_arguments_c_type get_program_c_arguments() {
    program_arguments_c_type c_args;
    for(const auto& arg : program_arguments_) {
        c_args.emplace_back(arg.c_str());
    }
    return c_args;
}

}

/** Declare MAIN entry point if EK_NO_MAIN is not declared.
 * Disable MAIN if you want to use third-party or test framework or your own custom MAIN entry
 */
#ifndef EK_NO_MAIN

int EK_MAIN_ENTRY_POINT(int argc, char* argv[]) {
    ek::load_program_arguments(argc, argv);
    ek::main();
    return 0;
}

#endif