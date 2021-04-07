#pragma once

#ifdef _WIN32

// https://raw.githubusercontent.com/tronkko/dirent/master/include/dirent.h
#include "dirent_win32.h"
#include <windows.h>
#include <shlwapi.h>
//#pragma comment (lib, "Shlwapi.lib")

#else

#include <fnmatch.h>
#include <dirent.h>

#endif

inline const char* x_fnmatch(const char* p, const char* wildcard) {
    // https://stackoverflow.com/questions/35877738/windows-fnmatch-substitute
#ifdef _WIN32
    wchar_t wname[1024];
    wchar_t wmask[1024];

    size_t outsize;
    mbstowcs_s(&outsize, wname, 1024, p, strlen(p) + 1);
    mbstowcs_s(&outsize, wmask, 1024, wildcard, strlen(wildcard) + 1);

    if (PathMatchSpecW(wname, wmask)) {
        return p;
    }
#else
    if (!fnmatch(wildcard, p, 0)) {
        return p;
    }
#endif
    return nullptr;
}