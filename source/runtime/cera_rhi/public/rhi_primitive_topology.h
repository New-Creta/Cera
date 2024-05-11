#pragma once

namespace cera
{
  namespace renderer
  {
    enum class PrimitiveTopology
    {
      NONE = 0,
      POINTLIST,
      LINELIST,
      LINESTRIP,
      TRIANGLELIST,
      TRIANGLESTRIP
    };
  } // namespace renderer
} // namespace cera