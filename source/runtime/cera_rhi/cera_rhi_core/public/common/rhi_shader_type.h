#pragma once

namespace cera
{
  namespace renderer
  {
    enum class ShaderType
    {
      NONE = 0,
      VERTEX,
      PIXEL,
      GEOMETRY,
      COMPUTE
    };
  }
} // namespace cera