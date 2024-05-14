#include "util/thread_name.h"

namespace cera
{
    namespace renderer
    {
        namespace threading
        {
            void set_thread_name(std::thread& thread, const char* thread_name)
            {
                THREADNAME_INFO info;
                info.dwType = 0x1000;
                info.szName = thread_name;
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
}