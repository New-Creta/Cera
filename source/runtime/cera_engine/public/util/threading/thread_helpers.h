#pragma once

#include <thread>   // For std::thread

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // For HRESULT
#include <comdef.h> // For _com_error class (used to decode HR result codes).

namespace cera
{
    namespace threading
    {
        // Set the name of an std::thread.
        // Useful for debugging.
        const DWORD MS_VC_EXCEPTION = 0x406D1388;

        // Set the name of a running thread (for debugging)
        #pragma pack( push, 8 )
        typedef struct tagTHREADNAME_INFO
        {
            DWORD  dwType;      // Must be 0x1000.
            LPCSTR szName;      // Pointer to name (in user addr space).
            DWORD  dwThreadID;  // Thread ID (-1=caller thread).
            DWORD  dwFlags;     // Reserved for future use, must be zero.
        } THREADNAME_INFO;
        #pragma pack( pop )

        inline void set_thread_name(std::thread& thread, const char* threadName)
        {
            THREADNAME_INFO info;
            info.dwType = 0x1000;
            info.szName = threadName;
            info.dwThreadID = ::GetThreadId(reinterpret_cast<HANDLE>(thread.native_handle()));
            info.dwFlags = 0;

            __try
            {
                ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
    }
}