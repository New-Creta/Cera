#pragma once

#include "resources/rhi_resource.h"
#include "resource_descriptions/create_raster_state_desc.h"

#include "common/rhi_fill_mode.h"
#include "common/rhi_cull_mode.h"

namespace cera
{
  namespace renderer
  {
    class RasterState : public rhi_resource
    {
    public:
      RESOURCE_CLASS_TYPE(RasterState);

      virtual ~RasterState();

      FillMode fill_mode() const;
      CullMode cull_mode() const;
      s32 front_ccw() const;
      s32 depth_bias() const;
      f32 depth_bias_clamp() const;
      f32 sloped_scale_depth_bias() const;
      s32 depth_clip_enable() const;
      s32 multisample() const;
      s32 aa_lines() const;
      s32 forced_sample_count() const;
      ConservativeRasterizationMode conservative_raster() const;

    protected:
      RasterState(CreateRasterStateDesc&& desc);

    private:
      CreateRasterStateDesc m_desc;
    };
  } // namespace renderer
} // namespace cera