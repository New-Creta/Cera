#pragma once

namespace cera
{
  namespace renderer
  {
    enum class ShaderType
    {
      None = 0,
      VERTEX,
      PIXEL,
      GEOMETRY,
      COMPUTE
    };
  }
} // namespace cera