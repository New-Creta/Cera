#pragma once

#include "util/types.h"

#include <string>

namespace cera
{
    namespace windows
    {
        /** 
         * Determines if we are running on the Windows version or newer
         *
         * See the 'Remarks' section of https://msdn.microsoft.com/en-us/library/windows/desktop/ms724833(v=vs.85).aspx
         * for a list of MajorVersion/MinorVersion version combinations for Microsoft Windows.
         *
         * @return	Returns true if the current Windows version if equal or newer than MajorVersion
         */
        bool verify_windows_version(u32 major, u32 minor, u32 build_version);

        /** Request that the engine exit as soon as it can safely do so
         * The "reason" is not optional and should be a useful description of why the engine exit is requested
         */
        void request_engine_exit(const std::string& reason);
        /** Check if an engine exit was requested */
        bool is_engine_exit_requested();
    } // namespace windows
} // namespace cera