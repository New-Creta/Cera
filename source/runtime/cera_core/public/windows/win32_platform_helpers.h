#pragma once

#include <string>

namespace cera
{
    namespace windows
    {
        /** Request that the engine exit as soon as it can safely do so
         * The "reason" is not optional and should be a useful description of why the engine exit is requested
         */
        void request_engine_exit(const std::string& reason);
        /** Check if an engine exit was requested */
        bool is_engine_exit_requested();
    } // namespace windows
} // namespace cera