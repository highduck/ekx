/**
 * unit.h - v0.0.6 - Simple header-only testing library for C - https://github.com/eliasku/unit
 *
 * Minimal example. Compile executable with `-D UNIT_TESTING` to enable tests.
 *
 * ```c
 * #define UNIT_MAIN
 * #include "unit.h"
 *
 * suite( you_should_define_the_suite_name_here ) {
 *   describe( optionally_add_the_subject ) {
 *     it( "describe the test behaviour" ) {
 *       check("use warn / check / require functions");
 *     }
 *   }
 * }
 * ```
 *
 * ## Enable Tests
 *
 * By default all test code is stripped away, to enable test-code you should pass `-D UNIT_TESTING` and build executable
 *
 * ## Default configuration
 *
 * In any translation unit you need to `#define UNIT_MAIN` before `#include <unit.h>` to implement the library and
 * generate default main entry-point to run all tests.
 *
 * ## Custom `main()`
 *
 * If you need just to implement library, you `#define UNIT_IMPLEMENT` before `include <unit.h>` in any single translation unit
 *
 **/

/**
 * Declare default main arguments
 */
// #define UNIT_DEFAULT_ARGS "--no-colors", "--trace"
#ifndef UNIT_H
#define UNIT_H

#ifndef UNIT_TESTING

#define UNIT__NOOP (void)(0)
#define UNIT__CONCAT_(a, b) a ## b
#define UNIT__CONCAT(a, b) UNIT__CONCAT_(a, b)
#define UNIT_SUITE(Name, ...) __attribute__((unused)) static void UNIT__CONCAT(unit__, __COUNTER__)(void)
#define UNIT_DESCRIBE(Name, ...) while(0)
#define UNIT_TEST(Description, ...) while(0)

#define UNIT_ECHO(...) UNIT__NOOP

#define UNIT_WARN(x, ...) UNIT__NOOP
#define UNIT_WARN_FALSE(x, ...) UNIT__NOOP
#define UNIT_WARN_EQ(a, b, ...) UNIT__NOOP
#define UNIT_WARN_NE(a, b, ...) UNIT__NOOP
#define UNIT_WARN_GT(a, b, ...) UNIT__NOOP
#define UNIT_WARN_GE(a, b, ...) UNIT__NOOP
#define UNIT_WARN_LT(a, b, ...) UNIT__NOOP
#define UNIT_WARN_LE(a, b, ...) UNIT__NOOP

#define UNIT_CHECK(x, ...) UNIT__NOOP
#define UNIT_CHECK_FALSE(x, ...) UNIT__NOOP
#define UNIT_CHECK_EQ(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_NE(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_GT(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_GE(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_LT(a, b, ...) UNIT__NOOP
#define UNIT_CHECK_LE(a, b, ...) UNIT__NOOP

#define UNIT_REQUIRE(x, ...) UNIT__NOOP
#define UNIT_REQUIRE_FALSE(x, ...) UNIT__NOOP
#define UNIT_REQUIRE_EQ(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_NE(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_GT(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_GE(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_LT(a, b, ...) UNIT__NOOP
#define UNIT_REQUIRE_LE(a, b, ...) UNIT__NOOP

#define UNIT_SKIP(...) UNIT__NOOP

#define unit_main(...) (0)


#else

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UNIT_STATUS_RUN = 0,
    UNIT_STATUS_SUCCESS = 1,
    UNIT_STATUS_SKIPPED = 2,
    UNIT_STATUS_FAILED = 3
};

enum {
    UNIT__OP_TRUE = 0,
    UNIT__OP_FALSE = 1,
    UNIT__OP_EQ = 2,
    UNIT__OP_NE = 3,
    UNIT__OP_LT = 4,
    UNIT__OP_LE = 5,
    UNIT__OP_GT = 6,
    UNIT__OP_GE = 7,

    // не помечает тест неудачным, просто предупреждает
    UNIT__LEVEL_WARN = 0,
    // проваливает тест, но продолжает выполнять другие проверки
    UNIT__LEVEL_CHECK = 1,
    // проваливает тест и пропускает следующие проверки в текущем тесте
    UNIT__LEVEL_REQUIRE = 2,

    // types
    UNIT__TYPE_CASE = 0,
    UNIT__TYPE_TEST = 1,

    // printer commands
    UNIT__PRINTER_SETUP = 0,
    UNIT__PRINTER_SHUTDOWN = 1,
    UNIT__PRINTER_BEGIN = 2,
    UNIT__PRINTER_END = 3,
    UNIT__PRINTER_ECHO = 4,
    UNIT__PRINTER_FAIL = 5,
    UNIT__PRINTER_ASSERTION = 6,
};

struct unit_test {
    const char* name;
    const char* file;
    int line;

    void (* fn)(void);

    int type;
    // options {
    bool failing;
    bool skip;
    // }

    double t0;
    double elapsed;

    struct unit_test* next;
    struct unit_test* children;
    struct unit_test* parent;

    int total;
    int passed;

    // test
    // status of current assertion
    int status;
    // state for this test scope
    // позволять ли дальше работать другим проверкам в рамках этого теста
    int state;
    const char* assert_comment;
    const char* assert_desc;
    const char* assert_file;
    int assert_line;
    int assert_level;
    int assert_status;
};

extern struct unit_test* unit_tests;
extern struct unit_test* unit_cur;

struct unit_run_options {
    int version;
    int help;
    int list;
    int ascii;
    int trace;
    int quiet;
    int animate;
    int doctest_xml;
    int short_filenames;
    unsigned seed;
    const char* program;
};

extern struct unit_run_options unit__opts;

struct unit_printer {
    void (* callback)(int cmd, struct unit_test* unit, const char* msg);

    struct unit_printer* next;
};

int unit__begin(struct unit_test* unit);

void unit__end(struct unit_test* unit);

void unit__echo(const char* msg);

int unit_main(struct unit_run_options options);

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define UNIT__STR(x) #x
#define UNIT__X_STR(x) UNIT__STR(x)

#define UNIT__CONCAT(a, b) a ## b
#define UNIT__X_CONCAT(a, b) UNIT__CONCAT(a, b)

#define UNIT__SCOPE_BODY(begin, end, Var) for (int Var = (begin, 0); !Var; ++Var, end)
#define UNIT_SCOPE(begin, end) UNIT__SCOPE_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__TRY_BODY(begin, end, Var) for (int Var = (begin) ? 0 : (end, 1); !Var; ++Var, end)
#define UNIT_TRY_SCOPE(begin, end) UNIT__TRY_BODY(begin, end, UNIT__X_CONCAT(s__, __COUNTER__))

#define UNIT__SUITE(Var, Name, ...) \
    static void Var(void); \
    __attribute__((constructor)) static void UNIT__CONCAT(Var, _ctor)(void) { \
        static struct unit_test u = (struct unit_test){ .name=Name, .file=__FILE__, .line=__LINE__, .fn=Var, .type=UNIT__TYPE_CASE, __VA_ARGS__ }; \
        u.next = unit_tests; unit_tests = &u; \
    } \
    static void Var(void)

#define UNIT_SUITE_(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), Name, __VA_ARGS__)
#define UNIT_SUITE(Name, ...) UNIT__SUITE(UNIT__X_CONCAT(unit__, __COUNTER__), #Name, __VA_ARGS__)

#define UNIT__DECL(Type, Var, Name, ...) \
    static struct unit_test Var = (struct unit_test){ .name=Name, .file=__FILE__, .line=__LINE__, .fn=NULL, .type=Type, __VA_ARGS__ }; \
    UNIT_TRY_SCOPE(unit__begin(&Var), unit__end(&Var))

#define UNIT_DESCRIBE(Name, ...) UNIT__DECL(UNIT__TYPE_CASE, UNIT__X_CONCAT(u__, __COUNTER__), #Name, __VA_ARGS__)
#define UNIT_TEST(Name, ...) UNIT__DECL(UNIT__TYPE_TEST, UNIT__X_CONCAT(u__, __COUNTER__), "" Name, __VA_ARGS__)

bool unit__prepare_assert(int level, const char* file, int line, const char* comment, const char* desc);

// устанавливает читаемое описание проверки, в случае нормального состояния выполняет проверку,
// если установлено состояние пропускать тесты - НЕ ВЫЧИСЛЯЕТ аргументы для проверки
#define UNIT__ASSERT_LAZY(Assertion, Level, Comment, Description) \
if(unit__prepare_assert(Level, __FILE__, __LINE__, Comment, Description)) Assertion

#define UNIT__IS_TRUE(_, x) (!!(x))
#define UNIT__IS_NOT_EMPTY_STR(_, x) ((x) && (x)[0])
#define UNIT__CMP(a, b) ((a) == (b) ? 0 : ((a) > (b) ? 1 : -1))
#define UNIT__STRCMP(a, b) ((a) == (b) ? 0 : strcmp((a) ? (a) : "", (b) ? (b) : ""))

#define UNIT__FOR_ASSERTS(macro) \
macro(int, intmax_t, %jd, UNIT__CMP, UNIT__IS_TRUE) \
macro(uint, uintmax_t, %ju, UNIT__CMP, UNIT__IS_TRUE) \
macro(dbl, long double, %Lg, UNIT__CMP, UNIT__IS_TRUE) \
macro(ptr, const void*, %p, UNIT__CMP, UNIT__IS_TRUE) \
macro(str, const char*, %s, UNIT__STRCMP, UNIT__IS_NOT_EMPTY_STR)

#define UNIT__DEFINE_ASSERT(Tag, Type, ...) \
void unit__assert_ ## Tag(Type a, Type b, int op, const char* expr, const char* sa, const char* sb);

UNIT__FOR_ASSERTS(UNIT__DEFINE_ASSERT)

#define UNIT__SELECT_ASSERT(x) \
    _Generic((x), \
        void*: unit__assert_ptr, \
        const void*: unit__assert_ptr, \
        char*: unit__assert_str, \
        const char*: unit__assert_str, \
        bool: unit__assert_int, \
        int: unit__assert_int, \
        long long: unit__assert_int, \
        unsigned int: unit__assert_uint, \
        unsigned long: unit__assert_uint, \
        unsigned long long: unit__assert_uint, \
        float: unit__assert_dbl, \
        double: unit__assert_dbl, \
        long double: unit__assert_dbl)

#define UNIT__ASSERT(Level, Op, a, b, Desc, ...)  UNIT__ASSERT_LAZY(UNIT__SELECT_ASSERT(b)(a, b, Op, Desc, #a, #b), Level, "" #__VA_ARGS__, Desc)

#define UNIT_WARN(x, ...)       UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_TRUE,  0, x, "warn " #x, __VA_ARGS__)
#define UNIT_WARN_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_FALSE, 0, x, "warn " #x " is not true", __VA_ARGS__)
#define UNIT_WARN_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_EQ, a, b, "warn " #a " == " #b, __VA_ARGS__)
#define UNIT_WARN_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_NE, a, b, "warn " #a " != " #b, __VA_ARGS__)
#define UNIT_WARN_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_GT, a, b, "warn " #a " > " #b, __VA_ARGS__)
#define UNIT_WARN_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_GE, a, b, "warn " #a " >= " #b, __VA_ARGS__)
#define UNIT_WARN_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_LT, a, b, "warn " #a " < " #b, __VA_ARGS__)
#define UNIT_WARN_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_WARN, UNIT__OP_LE, a, b, "warn " #a " <= " #b, __VA_ARGS__)

#define UNIT_CHECK(x, ...)       UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_TRUE,  0, x, "check " #x, __VA_ARGS__)
#define UNIT_CHECK_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_FALSE, 0, x, "check " #x " is not true", __VA_ARGS__)
#define UNIT_CHECK_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_EQ, a, b, "check " #a " == " #b, __VA_ARGS__)
#define UNIT_CHECK_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_NE, a, b, "check " #a " != " #b, __VA_ARGS__)
#define UNIT_CHECK_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GT, a, b, "check " #a " > " #b, __VA_ARGS__)
#define UNIT_CHECK_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_GE, a, b, "check " #a " >= " #b, __VA_ARGS__)
#define UNIT_CHECK_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LT, a, b, "check " #a " < " #b, __VA_ARGS__)
#define UNIT_CHECK_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_CHECK, UNIT__OP_LE, a, b, "check " #a " <= " #b, __VA_ARGS__)

#define UNIT_REQUIRE(x, ...)       UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_TRUE,  0, x, "require " #x, __VA_ARGS__)
#define UNIT_REQUIRE_FALSE(x, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_FALSE, 0, x, "require " #x " is not true", __VA_ARGS__)
#define UNIT_REQUIRE_EQ(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_EQ, a, b, "require " #a " == " #b, __VA_ARGS__)
#define UNIT_REQUIRE_NE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_NE, a, b, "require " #a " != " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GT, a, b, "require " #a " > " #b, __VA_ARGS__)
#define UNIT_REQUIRE_GE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_GE, a, b, "require " #a " >= " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LT(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LT, a, b, "require " #a " < " #b, __VA_ARGS__)
#define UNIT_REQUIRE_LE(a, b, ...) UNIT__ASSERT(UNIT__LEVEL_REQUIRE, UNIT__OP_LE, a, b, "require " #a " <= " #b, __VA_ARGS__)

#define UNIT_SKIP() unit_cur->state |= UNIT__LEVEL_REQUIRE
#define UNIT_ECHO(msg) unit__echo(msg)

#ifdef __cplusplus
}
#endif

#endif // UNIT_TESTING

#define SUITE(...) UNIT_SUITE(__VA_ARGS__)
#define DESCRIBE(...) UNIT_DESCRIBE(__VA_ARGS__)
#define IT(...) UNIT_TEST(__VA_ARGS__)
#define TEST(...) UNIT_TEST(__VA_ARGS__)
#define ECHO(...) UNIT_ECHO(__VA_ARGS__)

#define WARN(...)       UNIT_WARN(__VA_ARGS__)
#define WARN_FALSE(...) UNIT_WARN_FALSE(__VA_ARGS__)
#define WARN_EQ(...)    UNIT_WARN_EQ(__VA_ARGS__)
#define WARN_NE(...)    UNIT_WARN_NE(__VA_ARGS__)
#define WARN_GT(...)    UNIT_WARN_GT(__VA_ARGS__)
#define WARN_GE(...)    UNIT_WARN_GE(__VA_ARGS__)
#define WARN_LT(...)    UNIT_WARN_LT(__VA_ARGS__)
#define WARN_LE(...)    UNIT_WARN_LE(__VA_ARGS__)

#define CHECK(...)       UNIT_CHECK(__VA_ARGS__)
#define CHECK_FALSE(...) UNIT_CHECK_FALSE(__VA_ARGS__)
#define CHECK_EQ(...)    UNIT_CHECK_EQ(__VA_ARGS__)
#define CHECK_NE(...)    UNIT_CHECK_NE(__VA_ARGS__)
#define CHECK_GT(...)    UNIT_CHECK_GT(__VA_ARGS__)
#define CHECK_GE(...)    UNIT_CHECK_GE(__VA_ARGS__)
#define CHECK_LT(...)    UNIT_CHECK_LT(__VA_ARGS__)
#define CHECK_LE(...)    UNIT_CHECK_LE(__VA_ARGS__)

#define REQUIRE(...)       UNIT_REQUIRE(__VA_ARGS__)
#define REQUIRE_FALSE(...) UNIT_REQUIRE_FALSE(__VA_ARGS__)
#define REQUIRE_EQ(...)    UNIT_REQUIRE_EQ(__VA_ARGS__)
#define REQUIRE_NE(...)    UNIT_REQUIRE_NE(__VA_ARGS__)
#define REQUIRE_GT(...)    UNIT_REQUIRE_GT(__VA_ARGS__)
#define REQUIRE_GE(...)    UNIT_REQUIRE_GE(__VA_ARGS__)
#define REQUIRE_LT(...)    UNIT_REQUIRE_LT(__VA_ARGS__)
#define REQUIRE_LE(...)    UNIT_REQUIRE_LE(__VA_ARGS__)

#define SKIP(...) UNIT_SKIP(__VA_ARGS__)


#endif // UNIT_H


#ifdef UNIT_TESTING

#define UNIT_VERSION "0.0.6"

/**
 * @deprecated use UNIT_IMPLEMENT or UNIT_MAIN options
 */
#ifdef UNIT_IMPL
#define UNIT_MAIN
#endif // UNIT_IMPL

#ifdef UNIT_MAIN
#define UNIT_IMPLEMENT
#endif // UNIT_MAIN

#ifdef UNIT_IMPLEMENT
#ifndef UNIT__IMPLEMENTED
#define UNIT__IMPLEMENTED

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#ifdef _WIN32

/* https://github.com/Arryboom/fmemopen_windows  */
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

FILE* fmemopen(void* buf, size_t len, const char* type) {
    int fd;
    FILE* fp;
    char tp[MAX_PATH - 13];
    char fn[MAX_PATH + 1];
    int* pfd = &fd;
    int retner = -1;
    char tfname[] = "MemTF_";
    if (!GetTempPathA(sizeof(tp), tp))
        return NULL;
    if (!GetTempFileNameA(tp, tfname, 0, fn))
        return NULL;
    retner = _sopen_s(pfd, fn, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT,
                      _SH_DENYRW, _S_IREAD | _S_IWRITE);
    if (retner != 0)
        return NULL;
    if (fd == -1)
        return NULL;
    fp = _fdopen(fd, "wb+");
    if (!fp) {
        _close(fd);
        return NULL;
    }
    /*File descriptors passed into _fdopen are owned by the returned FILE * stream.If _fdopen is successful, do not call _close on the file descriptor.Calling fclose on the returned FILE * also closes the file descriptor.*/
    fwrite(buf, len, 1, fp);
    rewind(fp);
    return fp;
}

#endif


// region Цвета, текстовые сообщения и логи

#define UNIT_COLOR_RESET "\033[m"
#define UNIT_COLOR_BOLD "\033[1m"
#define UNIT_COLOR_DIM "\033[2m"
#define UNIT_COLOR_UNDERLINE "\033[4m"
#define UNIT_COLOR_MAYBE "\033[35m"
#define UNIT_COLOR_COMMENT "\033[36m"
#define UNIT_COLOR_SUCCESS "\033[32m"
#define UNIT_COLOR_FAIL "\033[31m"
#define UNIT_COLOR_DESC "\033[33m"
#define UNIT_COLOR_INVERT_PASS "\033[30;42m"
#define UNIT_COLOR_LABEL_PASS UNIT_COLOR_BOLD UNIT_COLOR_INVERT_PASS
#define UNIT_COLOR_LABEL_FAIL UNIT_COLOR_BOLD "\033[30;41m"
#define UNIT_COLOR_LABEL_SKIP UNIT_COLOR_BOLD "\033[30;47m"
#define UNIT_COLOR_LABEL_RUNS UNIT_COLOR_BOLD "\033[30;46m"

/**
 * Alternative icons:
 * message icons: "💬 ", "# ", "ℹ ", " ⃫ "
 * skip icons: "Ⅱ "
 */

enum {
    ICON_RUN = 0,
    ICON_OK = 1,
    ICON_SKIP = 2,
    ICON_FAIL = 3,
    ICON_ASSERT = 4,
    ICON_MSG = 5
};

static const char* icon(int type) {
    static const char* fancy[] = {
            UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "▶ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_SUCCESS "✓ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_DIM "∅ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_FAIL "✕ " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_FAIL "● " UNIT_COLOR_RESET,
            UNIT_COLOR_BOLD UNIT_COLOR_COMMENT "» " UNIT_COLOR_RESET,
    };
    static const char* ascii[] = {
            "> ",
            "+ ",
            ". ",
            "x ",
            "! ",
            "# ",
    };
    return unit__opts.ascii ? ascii[type] : fancy[type];
}

// region reporting

static void unit__sleep(double seconds) {
#ifndef _WIN32
    const long secs = (long) seconds;
    const long nanos = (long) ((seconds - (double) secs) * 1000000000.0);
    struct timespec ts = {secs, nanos};
    nanosleep(&ts, NULL);
#endif
}

static void print_wait(FILE* f) {
    fflush(f);
    if (unit__opts.animate) {
        unit__sleep(0.1);
    }
}

void begin_style(FILE* file, const char* style) {
    if (style && !unit__opts.ascii) {
        fputs(style, file);
    }
}

void end_style(FILE* file) {
    if (!unit__opts.ascii) {
        fputs(UNIT_COLOR_RESET, file);
    }
}

void print_text(FILE* file, const char* text, const char* style) {
    begin_style(file, style);
    fputs(text, file);
    end_style(file);
}

static char unit__fails_mem[4096];
static FILE* unit__fails = 0;

static const char* unit_spaces[8] = {
        "",
        "  ",
        "    ",
        "      ",
        "        ",
        "          ",
        "            ",
        "              "
};

static const char* get_spaces(int n) {
    const int len = sizeof unit_spaces / sizeof unit_spaces[0];
    if(n < 0) n = 0;
    if(n >= len) n = len - 1;
    return unit_spaces[n];
}

void print_elapsed_time(FILE* f, double elapsed_time) {
    if (elapsed_time >= 0.00001) {
        begin_style(f, UNIT_COLOR_DIM);
        fprintf(f, " (%0.2f ms)", 1000.0 * elapsed_time);
        end_style(f);
    }
}

static const char* beautify_name(const char* name) {
    return (name && name[0]) ? name : "(anonymous)";
}

static const char* short_filename(const char* file) {
    if (file) {
        const char* p = strrchr(file, '/');
        if (p) {
            return p + 1;
        }
    }
    return file;
}

static const char* beautify_filename(const char* file) {
    return unit__opts.short_filenames ? short_filename(file) : file;
}

static void print_label(FILE* f, struct unit_test* node) {
    static const char* fancy[] = {
            UNIT_COLOR_LABEL_RUNS " RUNS " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_PASS " PASS " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_SKIP " SKIP " UNIT_COLOR_RESET,
            UNIT_COLOR_LABEL_FAIL " FAIL " UNIT_COLOR_RESET,
    };
    static const char* ascii[] = {
            "[ RUNS ]",
            "[ PASS ]",
            "[ SKIP ]",
            "[ FAIL ]",
    };
    const int type = node->status;
    const char* lbl = (unit__opts.ascii ? ascii : fancy)[type];

    fputs(lbl, f);
    fputc(' ', f);
    begin_style(f, UNIT_COLOR_BOLD);
    fputs(beautify_name(node->name), f);
    end_style(f);
    fputc(' ', f);
    switch (node->status) {
        case UNIT_STATUS_RUN:
            break;
        case UNIT_STATUS_SUCCESS:
        case UNIT_STATUS_FAILED:
            fprintf(f, ": passed %d/%d tests", node->passed, node->total);
            print_elapsed_time(f, node->elapsed);
            break;
        default:
            break;
    }
    print_wait(f);
}

void printer_def_begin(struct unit_test* unit) {
    FILE* f = stdout;
    if (!unit->parent) {
        print_label(f, unit);
    }
}

int def_depth = 0;

const char* unit__spaces(int delta) {
    return get_spaces(def_depth + delta);
}

static void print_node(struct unit_test* node) {
    FILE* f = stdout;
    ++def_depth;
    const char* name = beautify_name(node->name);
    fputs(unit__spaces(0), f);
    if (node->type == UNIT__TYPE_CASE) {
        if (node->status == UNIT_STATUS_SKIPPED) {
            print_text(f, name, UNIT_COLOR_DIM);
        } else {
            print_text(f, name, NULL);
        }
    } else {
        fputs(icon(node->status), f);
        print_text(f, name, UNIT_COLOR_DIM);
    }
    print_elapsed_time(f, node->elapsed);
    fputc('\n', f);
    print_wait(f);
    for (struct unit_test* child = node->children; child; child = child->next) {
        print_node(child);
    }
    --def_depth;
}

void printer_def_end(struct unit_test* unit) {
    FILE* f = stdout;
    if (unit->parent) {
        if (unit->type == UNIT__TYPE_TEST) {
            fputs(icon(unit->status), f);
            print_wait(f);
        }
        return;
    }
    // go back to the beginning of line
    // fputc('\n', f);
    // begin_style(f, "\033[1A\033[999D");
    fputc('\r', f);
    print_label(f, unit);
    fputc('\n', f);

    if (unit__fails) {
        for (struct unit_test* child = unit->children; child; child = child->next) {
            print_node(child);
        }
        fputc('\n', f);

        const long pos = ftell(unit__fails);
        unit__fails_mem[pos] = 0;
        fclose(unit__fails);
        unit__fails = 0;
        fputs(unit__fails_mem, f);

        fputc('\n', f);
    }
}

static void unit__breadcrumbs(FILE* f, struct unit_test* test) {
    if (test->parent) {
        unit__breadcrumbs(f, test->parent);
        print_text(f, unit__opts.ascii ? " > " : " → ", UNIT_COLOR_BOLD UNIT_COLOR_DIM);
    }
    print_text(f, beautify_name(test->name), UNIT_COLOR_BOLD UNIT_COLOR_FAIL);
}

void printer_def_fail(struct unit_test* unit, const char* msg) {
    if (unit__fails == 0) {
        // TODO: change to `tmpfile` ?
        unit__fails = fmemopen(unit__fails_mem, sizeof unit__fails_mem, "w");
    }
    FILE* f = unit__fails;
    fputs(unit_spaces[1], f);
    fputs(icon(ICON_ASSERT), f);
    unit__breadcrumbs(f, unit_cur);
    fputc('\n', f);
    fputc('\n', f);

    fputs(unit_spaces[2], f);
    fputs(msg, f);
    fputc('\n', f);
    if (unit->assert_file) {
        fputs(unit_spaces[2], f);
        print_text(f, "@ ", UNIT_COLOR_COMMENT UNIT_COLOR_DIM UNIT_COLOR_BOLD);
        begin_style(f, UNIT_COLOR_COMMENT UNIT_COLOR_UNDERLINE);
        fprintf(f, "%s:%d", beautify_filename(unit->assert_file), unit->assert_line);
        end_style(f);
        fputc('\n', f);
    }
    fputc('\n', f);
}

// endregion reporting

static void printer_def(int cmd, struct unit_test* unit, const char* msg) {
    switch (cmd) {
        case UNIT__PRINTER_SETUP:
            fputs(unit__opts.ascii ? "\n[ unit ] v" UNIT_VERSION "\n\n" :
                  "\n\033[1;30;42m" " ✓ηỉτ " "\033[0;30;46m" " v" UNIT_VERSION " " "\33[m\n\n", stdout);
            print_wait(stdout);
            break;
        case UNIT__PRINTER_BEGIN:
            printer_def_begin(unit);
            break;
        case UNIT__PRINTER_END:
            printer_def_end(unit);
            break;
        case UNIT__PRINTER_FAIL:
            printer_def_fail(unit, msg);
            break;
            //case UNIT__PRINTER_ASSERTION:
            //    fputs(icon(unit->assert_status), stdout);
            //    print_wait(stdout);
            //    break;
    }
}

// region tracing printer
static int trace_depth = 0;

static const char* trace_spaces(int delta) {
    return get_spaces(trace_depth + delta);
}

static void printer_tracing(int cmd, struct unit_test* unit, const char* msg) {
    FILE* f = stdout;
    switch (cmd) {
        case UNIT__PRINTER_BEGIN:
            fputs(trace_spaces(0), f);
            print_text(f, beautify_name(unit->name), UNIT_COLOR_BOLD);
            print_text(f, " {\n", UNIT_COLOR_DIM);
            ++trace_depth;
            break;
        case UNIT__PRINTER_END:
            --trace_depth;
            fputs(trace_spaces(0), f);
            print_text(f, "}\n", UNIT_COLOR_DIM);
            break;
        case UNIT__PRINTER_ECHO:
            fputs(trace_spaces(0), f);
            fputs(icon(ICON_MSG), f);
            print_text(f, msg, UNIT_COLOR_COMMENT);
            fputc('\n', f);
            break;
        case UNIT__PRINTER_FAIL:
            fputs(trace_spaces(0), f);
            fputs("    Failed: ", f);
            fputs(beautify_name(unit->name), f);
            fputc('\n', f);

            fputs(trace_spaces(0), f);
            fputs("    ", f);
            fputs(msg, f);
            fputc('\n', f);
            break;
        case UNIT__PRINTER_ASSERTION: {
            fputs(trace_spaces(0), f);
            fputs(icon(unit->assert_status), f);

            const char* cm = unit->assert_comment;
            const char* desc = (cm && cm[0] != '\0') ? cm : unit->assert_desc;
            fputs(desc, f);

            fputc('\n', f);
        }
            break;
    }
}

// endregion

static const char* doctest_get_node_type(struct unit_test* node) {
    if (node->parent) {
        if (node->parent->parent) {
            return "SubCase";
        }
        return "TestCase";
    }
    return "TestSuite";
}

static void printer_xml_doctest(int cmd, struct unit_test* node, const char* msg) {
    FILE* f = stdout;
    switch (cmd) {
        case UNIT__PRINTER_SETUP:
            fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            // binary="/absolute/path/to/test/executable"
            fprintf(f, "<unit version=\"" UNIT_VERSION "\">\n");
            fprintf(f,
                    "  <Options order_by=\"file\" rand_seed=\"0\" first=\"0\" last=\"4294967295\" abort_after=\"0\" subcase_filter_levels=\"2147483647\" case_sensitive=\"false\" no_throw=\"false\" no_skip=\"false\"/>\n");
            ++def_depth;
            break;
        case UNIT__PRINTER_SHUTDOWN: {
            int total = 0;
            int passed = 0;
            for (struct unit_test* u = unit_tests; u; u = u->next) {
                passed += u->passed;
                total += u->total;
            }
            fprintf(f, "  <OverallResultsTestCases successes=\"%d\" failures=\"%d\" expectedFailures=\"%d\" />\n",
                    passed, total - passed, 0);
        }
            --def_depth;
            fprintf(f, "</unit>\n");
            fflush(f);
            break;
        case UNIT__PRINTER_BEGIN:
            fputs(unit__spaces(0), f);
            fprintf(f, "<%s name=\"%s\" filename=\"%s\" line=\"%d\" skipped=\"%s\">\n", doctest_get_node_type(node),
                    node->name, node->file,
                    node->line, node->status == UNIT_STATUS_SKIPPED ? "true" : "false");
            ++def_depth;
            break;
        case UNIT__PRINTER_END:
            --def_depth;
            fputs(unit__spaces(0), f);
            fprintf(f, "</%s>\n", doctest_get_node_type(node));
            break;
        case UNIT__PRINTER_FAIL:
            fputs(unit__spaces(0), f);
            fprintf(f, "<Expression success=\"%s\" type=\"%s\" filename=\"%s\" line=\"%d\">\n",
                    node->status != UNIT_STATUS_FAILED ? "true" : "false", "REQUIRE",
                    beautify_filename(node->assert_file), node->assert_line);
            fputs(unit__spaces(1), f);
            fprintf(f, "<Original>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "%s\n", node->assert_desc);
            fputs(unit__spaces(1), f);
            fprintf(f, "</Original>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "<Expanded>\n");
            fputs(unit__spaces(1), f);
            fprintf(f, "%s\n", node->assert_desc);
            fputs(unit__spaces(1), f);
            fprintf(f, "</Expanded>\n");
            fputs(unit__spaces(0), f);
            fprintf(f, "</Expression>\n");
            break;
    }
}


struct unit_test* unit_tests = NULL;
struct unit_test* unit_cur = NULL;

// region утилиты для вывода
const char* unit__vbprintf(const char* fmt, va_list args) {
    static char s_buffer[4096];
    vsnprintf(s_buffer, sizeof s_buffer, fmt, args);
    return s_buffer;
}

// endregion

const char* unit__op_expl[] = {
        " is true",
        " is false",
        " == ",
        " != ",
        " >= ",
        " > ",
        " <= ",
        " < ",
};

const char* unit__op_nexpl[] = {
        " is not true",
        " is not false",
        " != ",
        " == ",
        " < ",
        " <= ",
        " > ",
        " >= ",
};

struct unit_printer* unit__printers;

#define UNIT__EACH_PRINTER(Func, ...) \
for(struct unit_printer* p = unit__printers; p; p = p->next) { p->callback(UNIT__PRINTER_ ## Func, __VA_ARGS__); }

bool unit__prepare_assert(int level, const char* file, int line, const char* comment, const char* desc) {
    unit_cur->assert_comment = comment;
    unit_cur->assert_desc = desc;
    unit_cur->assert_level = level;
    unit_cur->assert_file = file;
    unit_cur->assert_line = line;
    if (unit_cur->state & UNIT__LEVEL_REQUIRE) {
        // пропустить проверку
        unit_cur->assert_status = UNIT_STATUS_SKIPPED;
        UNIT__EACH_PRINTER(ASSERTION, unit_cur, 0);
        return false;
    }
    unit_cur->assert_status = UNIT_STATUS_SUCCESS;
    return true;
}

static void unit__fail_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* msg = unit__vbprintf(fmt, args);
    va_end(args);

    if (unit_cur->assert_level > UNIT__LEVEL_WARN) {
        unit_cur->assert_status = UNIT_STATUS_FAILED;
        unit_cur->state |= unit_cur->assert_level;
        for (struct unit_test* n = unit_cur; n; n = n->parent) {
            if (n->failing) {
                break;
            }
            n->status = UNIT_STATUS_FAILED;
        }
    }
    UNIT__EACH_PRINTER(FAIL, unit_cur, msg);
}

#define UNIT__IMPLEMENT_ASSERT(Tag, Type, FormatType, BinaryOp, UnaryOp) \
void unit__assert_ ## Tag(Type a, Type b, int op, const char* expr, const char* sa, const char* sb) { \
    bool pass = false; \
    switch(op) {                                                       \
        case UNIT__OP_TRUE: pass = (UnaryOp(a, b)); break; \
        case UNIT__OP_FALSE: pass = !(UnaryOp(a, b)); break; \
        case UNIT__OP_EQ: pass = (BinaryOp(a, b)) == 0; break; \
        case UNIT__OP_NE: pass = (BinaryOp(a, b)) != 0; break; \
        case UNIT__OP_LT: pass = (BinaryOp(a, b)) < 0; break; \
        case UNIT__OP_LE: pass = (BinaryOp(a, b)) <= 0; break; \
        case UNIT__OP_GT: pass = (BinaryOp(a, b)) > 0; break; \
        case UNIT__OP_GE: pass = (BinaryOp(a, b)) >= 0; break; \
    } \
    unit_cur->assert_status = pass ? UNIT_STATUS_SUCCESS : UNIT_STATUS_FAILED; \
    UNIT__EACH_PRINTER(ASSERTION, unit_cur, 0); \
    if (!pass) { \
        const char* expl = unit__op_expl[op];                 \
        const char* nexpl = unit__op_nexpl[op];                 \
        if (op < UNIT__OP_EQ) unit__fail_impl("Expected " UNIT_COLOR_SUCCESS "`%s`%s" UNIT_COLOR_RESET ", but got `" UNIT_COLOR_FAIL #FormatType "%s`" UNIT_COLOR_RESET, sb, expl, b, nexpl); \
        else unit__fail_impl("Expected " UNIT_COLOR_SUCCESS "`%s`%s`%s`" UNIT_COLOR_RESET ", but got " UNIT_COLOR_FAIL "`" #FormatType "%s" #FormatType "`" UNIT_COLOR_RESET, sa, expl, sb, a, nexpl, b); \
    } \
}

UNIT__FOR_ASSERTS(UNIT__IMPLEMENT_ASSERT)

/** Время **/

double unit__time(double prev) {
    struct timespec ts = {0};
#ifndef UNIT_NO_TIME
#if defined(_WIN32) && !defined(__MINGW32__)
    timespec_get(&ts, TIME_UTC);
#else // _WIN32
    clock_gettime(CLOCK_REALTIME, &ts);
#endif // !_WIN32
#endif // !UNIT_NO_TIME
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1000000000.0 - prev;
}

// region начало конец запуска каждого теста

static struct unit_test* get_last_child(struct unit_test* children) {
    while (children && children->next) {
        children = children->next;
    }
    return children;
}

static void add_child(struct unit_test* parent, struct unit_test* child) {
    //assert (!child->parent);
    if (parent) {
        struct unit_test* last = get_last_child(parent->children);
        if (last) {
            last->next = child;
        } else {
            child->next = parent->children;
            parent->children = child;
        }
    }
    child->parent = parent;
}

int unit__begin(struct unit_test* unit) {
    const bool run = !unit->skip;
    unit->state = 0;
    unit->status = run ? UNIT_STATUS_RUN : UNIT_STATUS_SKIPPED;
    unit->assert_desc = NULL;
    unit->passed = 0;
    unit->total = 0;
    if (!unit->parent) {
        add_child(unit_cur, unit);
    }
    unit_cur = unit;
    if (run && unit->type == UNIT__TYPE_TEST) {
        for (struct unit_test* u = unit_cur; u; u = u->parent) {
            u->total++;
        }
    }
    UNIT__EACH_PRINTER(BEGIN, unit, 0);
    unit->t0 = unit__time(0.0);
    return run;
}

void unit__end(struct unit_test* unit) {
    unit->elapsed = unit__time(unit->t0);
    if (unit->status == UNIT_STATUS_RUN) {
        unit->status = UNIT_STATUS_SUCCESS;
        if (unit->type == UNIT__TYPE_TEST) {
            for (struct unit_test* u = unit_cur; u; u = u->parent) {
                u->passed++;
            }
        }
    }
    UNIT__EACH_PRINTER(END, unit, 0);
    unit_cur = unit->parent;
}

void unit__echo(const char* msg) {
    UNIT__EACH_PRINTER(ECHO, unit_cur, msg);
}

struct unit_run_options unit__opts;

static void unit__init_printers(void) {
    static struct unit_printer printer;
    printer.callback = unit__opts.trace ? printer_tracing : printer_def;
    if (unit__opts.doctest_xml) {
        printer.callback = printer_xml_doctest;
    }
    unit__printers = unit__opts.quiet ? NULL : &printer;
}

#define UNIT__MSG_VERSION "unit v" UNIT_VERSION "\n"
#define UNIT__MSG_USAGE "usage: %s OPTIONS\n" \
"options:\n" \
"  --version or -v: Prints the version of `unit` library\n" \
"  --help or -h: Prints usage help message\n" \
"  --list or -l: Prints all available tests\n" \
"  --ascii: Don't use colors and fancy unicode symbols in the output\n" \
"  --quiet or -q: Disables all output\n" \
"  --short-filenames or -S: Use only basename for displaying file-pos information\n" \
"  -r=xml: Special switch prints XML report in DocTest-friendly format (for CLion test run configuration)\n" \
"  --animate or -a: Simulate waits for printing messages, just for making fancy printing animation\n"

static int unit__cmd(struct unit_run_options options) {
    if (options.version) {
        fputs(UNIT__MSG_VERSION, stdout);
        return 0;
    }
    if (options.help) {
        fprintf(stdout, UNIT__MSG_USAGE, options.program);
        return 0;
    }
    if (options.list) {
        for (struct unit_test* suite = unit_tests; suite; suite = suite->next) {
            fputs(beautify_name(suite->name), stdout);
            fputs("\n", stdout);
        }
        return 0;
    }
    return 1;
}

int unit_main(struct unit_run_options options) {
    if (!unit__cmd(options)) {
        return 0;
    }

    unit__opts = options;
    srand(options.seed);
    unit__init_printers();

    UNIT__EACH_PRINTER(SETUP, 0, 0);

    int failed = 0;
    for (struct unit_test* suite = unit_tests; suite; suite = suite->next) {
        UNIT_TRY_SCOPE(unit__begin(suite), unit__end(suite)) suite->fn();
        if (suite->status == UNIT_STATUS_FAILED) {
            ++failed;
        }
    }

    UNIT__EACH_PRINTER(SHUTDOWN, 0, 0);

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void find_bool_arg(int argc, const char** argv, int* var, const char* name, const char* alias) {
    for (int i = 0; i < argc; ++i) {
        const char* v = argv[i];
        if (v && v[0] == '-') {
            ++v;
            if (alias && alias[0] && strstr(v, alias) == v) {
                *var = 1;
                return;
            } else if (name && name[0] && v[0] == '-') {
                ++v;
                if (strstr(v, name) == v) {
                    *var = 1;
                    return;
                }
            }
        }
    }
}

static void unit__parse_args(int argc, const char** argv, struct unit_run_options* out_options) {
    find_bool_arg(argc, argv, &out_options->version, "version", "v");
    find_bool_arg(argc, argv, &out_options->help, "help", "h");
    find_bool_arg(argc, argv, &out_options->list, "list", "l");
    find_bool_arg(argc, argv, &out_options->ascii, "ascii", NULL);
    find_bool_arg(argc, argv, &out_options->trace, "trace", "t");
    find_bool_arg(argc, argv, &out_options->quiet, "quiet", "q");
    find_bool_arg(argc, argv, &out_options->animate, "animate", "a");
    find_bool_arg(argc, argv, &out_options->short_filenames, "short-filenames", "S");
    // hack to trick CLion we are DocTest library tests
    find_bool_arg(argc, argv, &out_options->doctest_xml, NULL, "r=xml");
}

static void unit__setup_args(int argc, const char** argv, struct unit_run_options* out_options) {
    *out_options = (struct unit_run_options) {0};
    out_options->seed = (unsigned) time(NULL);
    out_options->program = argc > 0 ? short_filename(argv[0]) : "<unit>";
#ifdef UNIT_DEFAULT_ARGS
    static const char* cargv[] = { UNIT_DEFAULT_ARGS };
    static const int cargc = sizeof(cargv) / sizeof(cargv[0]);
    unit__parse_args(cargc, cargv, out_options);
#endif
    unit__parse_args(argc, argv, out_options);
}

// endregion

#ifdef UNIT_MAIN
int main(int argc, const char** argv) {
    struct unit_run_options options;
    unit__setup_args(argc, argv, &options);
    return unit_main(options);
}


#endif // UNIT_MAIN

#ifdef UNIT__SELF_TEST
UNIT_SUITE(unit_internals) {
    UNIT_DESCRIBE(beautify_filename) {
        IT("short filenames") {
            CHECK_EQ(short_filename(NULL), (const char*) NULL);
            CHECK_EQ(short_filename("/path/to/file.c"), "file.c");
            CHECK_EQ(short_filename("file.c"), "file.c");
        }

        IT("depends on option") {
            const bool prev = unit__opts.short_filenames;
            unit__opts.short_filenames = 1;
            CHECK_EQ(beautify_filename("/path/to/file.c"), "file.c");
            unit__opts.short_filenames = 0;
            CHECK_EQ(beautify_filename("/path/to/file.c"), "/path/to/file.c");
            unit__opts.short_filenames = prev;
        }
    }

    UNIT_DESCRIBE(beautify_name) {
        IT("replace empty string with (anonymous)") {
            CHECK_EQ(beautify_name(NULL), "(anonymous)");
            CHECK_EQ(beautify_name(""), "(anonymous)");
            CHECK_EQ(beautify_name("name"), "name");
        }
    }

    UNIT_DESCRIBE(get_spaces) {
        IT("clamp index to available range") {
            CHECK_EQ(get_spaces(0), "");
            CHECK_EQ(get_spaces(-1), "");
            CHECK_EQ(get_spaces(9999), unit_spaces[sizeof(unit_spaces) / sizeof(unit_spaces[0]) - 1]);
        }
    }

    DESCRIBE(find_bool_arg) {
        IT("parse short form") {
            int val = 0;
            find_bool_arg(3, (const char* []) {"not-an-arg", "-a", "--along"}, &val, "", "a");
            REQUIRE_EQ(val, 1);
        }
        IT("parse long form") {
            int val = 5;
            find_bool_arg(3, (const char* []) {"", "-a", "--along"}, &val, "along", "");
            REQUIRE_EQ(val, 1);
        }
        IT("don't change on not found") {
            int val = 2;
            find_bool_arg(4, (const char* []) {"--etemp", "-nt", "-a", "--no-along"}, &val, "temp", "t");
            REQUIRE_EQ(val, 2);
        }
    }

    DESCRIBE(unit__parse_args) {
        IT("options") {
            struct unit_run_options options = {0};
            unit__parse_args(6,
                             (const char* []) {
                                     "--quiet",
                                     "--ascii",
                                     "-r=xml",
                                     "-t",
                                     "-a",
                                     "not-found",
                                     NULL
                             }, &options);
            REQUIRE_EQ(options.quiet, 1);
            REQUIRE_EQ(options.animate, 1);
            REQUIRE_EQ(options.trace, 1);
            REQUIRE_EQ(options.doctest_xml, 1);
            REQUIRE_EQ(options.ascii, 1);
        }
    }
}

#endif // UNIT__SELF_TEST

#ifdef __cplusplus
}
#endif

#endif // !UNIT__IMPLEMENTED


#endif // UNIT_IMPLEMENT

#endif // UNIT_TESTING
