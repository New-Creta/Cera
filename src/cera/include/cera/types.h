#pragma once

#include <stdint.h>

using s8 = int8_t;
using u8 = uint8_t;
using s16 = int16_t;
using u16 = uint16_t;
using s32 = int32_t; 
using u32 = uint32_t;
using s64 = int64_t; 
using u64 = uint64_t;
using hash_id = uint32_t;

using c8 = char;
using c16 = wchar_t;

using f32 = float;
using f64 = double;

using ulong = unsigned long; 
using dword = unsigned long; // for win32

// Generic errors for the few cases errors are handled
enum cera_error
{
    CERA_ERR_OK = 0,
    CERA_ERR_FAILED = 1
};

// allow single threaded platforms to avoid use of atomic
#if CERA_WEB || CERA_WINDOWS
    #define CERA_SINGLE_THREADED 1
    #else
    #define CERA_SINGLE_THREADED 0
#endif

#if CERA_SINGLE_THREADED
    typedef u8     a_u8;
    typedef u32    a_u32;
    typedef u64    a_u64;
    typedef bool   a_bool;
    typedef s32    a_s32;
    #define cera_atomic_load(a) a
#else
    #include <atomic>
    typedef std::atomic<uint8_t>  a_u8;
    typedef std::atomic<uint32_t> a_u32;
    typedef std::atomic<uint64_t> a_u64;
    typedef std::atomic<bool>     a_bool;
    typedef std::atomic<s32>      a_s32;
    #define cera_atomic_load(a) a.load()
#endif

// compiler
#ifdef _MSC_VER
    #define cera_inline __forceinline
    #define cera_deprecated __declspec(deprecated)
    #define cera_debug_break __debug_break()
#else
    #define cera_inline inline __attribute__((always_inline))
    #define cera_deprecated __attribute__((deprecated))
    #define cera_debug_break __builtin_trap()
#endif