
#if defined(__APPLE__)
#include "LocalResource_Apple.hpp"
#elif defined(__ANDROID__)
#include "LocalResource_Android.hpp"
#elif defined(__EMSCRIPTEN__)
#include "LocalResource_Web.hpp"
#else
#include "LocalResource_Generic.hpp"
#endif