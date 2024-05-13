#pragma once

#include "util/types.h"
#include "util/log.h"

#include <string>

namespace cera
{
    namespace string_operations
    {
        std::string to_multibyte(const tchar* wide_character_buffer, size_t size);
    }
}