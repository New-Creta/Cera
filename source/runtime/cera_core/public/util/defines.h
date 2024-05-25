#pragma once

#ifndef CERA_DEFINES
#define CERA_DEFINES

//-------------------------------------------------------------------------
// Unused parameter.
#if defined CERA_COMPILER_CLANG
#define UNUSED_PARAM(...)                                                                                                                                                                                                                                \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wunused-value\"")                                                                                                                                                              \
    {                                                                                                                                                                                                                                                    \
        __VA_ARGS__;                                                                                                                                                                                                                                     \
    }                                                                                                                                                                                                                                                    \
    _Pragma("clang diagnostic pop")
#elif defined CERA_COMPILER_MSVC
#define UNUSED_PARAM(...)                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                    \
        __VA_ARGS__;                                                                                                                                                                                                                                     \
    }
#endif

//-------------------------------------------------------------------------
// Assembly instruction to break execution.
#if defined CERA_COMPILER_CLANG
#define DEBUG_BREAK() __builtin_trap()
#elif defined CERA_COMPILER_MSVC
#define DEBUG_BREAK() __debugbreak()
#else
#error DEBUG_BREAK unsupported machine instruction ...
#endif

//-------------------------------------------------------------------------
// Func signature
#if defined CERA_COMPILER_CLANG
#define CERA_FUNC_SIG __PRETTY_FUNCTION__
#elif defined CERA_COMPILER_MSVC
#define CERA_FUNC_SIG __FUNCSIG__
#else
#error Unknown compiler
#endif

//-------------------------------------------------------------------------
// STANDARD MACROS
// ARRAY SIZE
#define CERA_ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
// BIT TWIDDLING
#define BIT(x) (1 << x)
#endif