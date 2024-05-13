#pragma once

#include "rhi_resource_slot.h"
#include "util/hash.h"

namespace cera
{
    namespace renderer
    {
        struct CreatePipelineStateDesc
        {
            CreatePipelineStateDesc()
                : rasterizer_state(ResourceSlot::make_invalid()), blend_state(ResourceSlot::make_invalid()),
                  depth_stencil_state(ResourceSlot::make_invalid()), num_render_targets(0)
            {
            }

            ResourceSlot rasterizer_state;
            ResourceSlot blend_state;
            ResourceSlot depth_stencil_state;
            s32 num_render_targets;
        };
    } // namespace renderer
} // namespace cera

namespace std
{
    template <> struct hash<cera::renderer::CreatePipelineStateDesc>
    {
        size_t operator()(const cera::renderer::CreatePipelineStateDesc& createPipelineStateCommand) const
        {
            size_t seed = 0;

            hash_combine(seed, createPipelineStateCommand.rasterizer_state.slot_id());
            hash_combine(seed, createPipelineStateCommand.blend_state.slot_id());
            hash_combine(seed, createPipelineStateCommand.depth_stencil_state.slot_id());
            hash_combine(seed, createPipelineStateCommand.num_render_targets);

            return static_cast<size_t>(seed);
        }
    };
} // namespace std