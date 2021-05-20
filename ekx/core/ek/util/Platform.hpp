#pragma once

#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__)

#define EK_ANDROID 1

#elif TARGET_OS_IPHONE

#define EK_IOS 1

#elif TARGET_OS_MAC

#define EK_MACOS 1

#elif defined(__EMSCRIPTEN__)

#define EK_WEB 1

#elif defined(__linux__)

#define EK_LINUX 1

#elif defined(_WIN32) || defined(_WIN64)

#define EK_WINDOWS 1

#include <winapifamily.h>

#if defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define EK_UWP 1
#endif

#endif