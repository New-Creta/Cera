#include "windows/win32_platform_helpers.h"
#include "windows/win32_min.h"

#include "util/log.h"

namespace cera
{
    namespace windows
    {
        namespace internal
        {
            bool g_is_requesting_exit = false; /* Indicates that MainLoop() should be exited at the end of the current iteration */
        }

        /** Determine if this platform supports windowed mode */
        bool supports_window_mode()
        {
            return true;
        }

        /** 
         * Determines if we are running on the Windows version or newer
         *
         * See the 'Remarks' section of https://msdn.microsoft.com/en-us/library/windows/desktop/ms724833(v=vs.85).aspx
         * for a list of MajorVersion/MinorVersion version combinations for Microsoft Windows.
         *
         * @return	Returns true if the current Windows version if equal or newer than MajorVersion
         */
        bool verify_windows_version(u32 major, u32 minor, u32 build_version)
        {
            OSVERSIONINFOEX version;
            version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            version.dwMajorVersion = major;
            version.dwMinorVersion = minor;
            version.dwBuildNumber = build_version;

            ULONGLONG condition_mask = 0;
            condition_mask = VerSetConditionMask(condition_mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
            condition_mask = VerSetConditionMask(condition_mask, VER_MINORVERSION, VER_GREATER_EQUAL);
            condition_mask = VerSetConditionMask(condition_mask, VER_BUILDNUMBER,  VER_GREATER_EQUAL);

            return !!VerifyVersionInfo(&version, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, condition_mask);
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