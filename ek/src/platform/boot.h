#pragma once

#include <ek/config/detect_platform.hpp>

#include <string>
#include <vector>

#if (EK_MAC || EK_IOS || EK_WEB || EK_LINUX || EK_NULL)
#define EK_MAIN_ENTRY_POINT main
#elif (EK_ANDROID)
#define EK_MAIN_ENTRY_POINT ek_android_main__
#endif

#ifndef EK_NO_MAIN
int EK_MAIN_ENTRY_POINT(int argc, char* argv[]);
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

using program_arguments_type = std::vector<std::string>;
using program_arguments_c_type = std::vector<const char*>;

program_arguments_type& get_program_arguments();

program_arguments_c_type get_program_c_arguments();

void main();

}
