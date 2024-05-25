#include "windows/win32_platform_helpers.h"

#include "util/log.h"

namespace cera
{
    namespace windows
    {
        namespace internal
        {
            bool g_is_requesting_exit = false; /* Indicates that MainLoop() should be exited at the end of the current iteration */
        }

        /** Request that the engine exit as soon as it can safely do so
         * The "reason" is not optional and should be a useful description of why the engine exit is requested
         */
        void request_engine_exit(const std::string& reason)
        {
            log::info("Engine exit was requested with reason: {}", reason);

            internal::g_is_requesting_exit = true;
        }
        /** Check if an engine exit was requested */
        bool is_engine_exit_requested()
        {
            return internal::g_is_requesting_exit;
        }
    } // namespace windows
} // namespace cera