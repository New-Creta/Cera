#pragma once

#include "util/log.h"

namespace cera
{
		template<typename FormatString, typename... Args>
		void cera_assert(const FormatString& format, const Args&... args)
		{
      log::critical(format, std::forward<Args>(args));
    }
} // namespace cera

#ifdef CERA_ENABLE_ASSERTS
  #define CERA_ASSERT(...)          \
    cera::cera_assert(__VA_ARGS__); \
    while(true)                     \
    {                               \
      DEBUG_BREAK();                \
    }
  #define CERA_ASSERT_X(cond, ...)  \
    if(!(cond))                     \
    {                               \
      CERA_ASSERT(__VA_ARGS__);     \
    }
#else
  #define CERA_ASSERT(...)
  #define CERA_ASSERT_X(cond, ...)
#endif