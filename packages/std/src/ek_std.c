#include "std/assert.c.h"
#include "std/base64.c.h"
#include "std/bitset.c.h"
#include "std/buf.c.h"
#include "std/log.c.h"
#include "std/murmur.c.h"
#include "std/rnd.c.h"
#include "std/print.c.h"
#include "std/string.c.h"
#include "std/time.c.h"
#include "std/utf8.c.h"

#ifdef __EMSCRIPTEN__
#include "std/sparse_array.c.h"
#else
#include "std/sparse_array_vm.c.h"
#endif
