#include "rhi_factory.h"
#include "rhi_null.h"
#include "rhi_globals.h"

#include "common/rhi_feature_level.h"
#include "common/rhi_shader_platform.h"

#include "util/assert.h"

namespace cera
{
    namespace renderer
    {
        namespace rhi_factory
        {
            bool is_supported()
            {
                // null rhi is always supported as no rendering features will be initialized
                return true;
            }

            std::unique_ptr<rhi> create()
            {
                return std::make_unique<rhi_null>();
            }
        } // namespace rhi_factory

        void rhi_null::initialize()
        {
#ifdef CERA_PLATFORM_WINDOWS
            g_shader_platform[feature_level::UNSPECIFIED] = shader_platform::NONE;
#endif
            CERA_ASSERT_X(!g_rhi_initialized, "rhi was already initialized!");

            g_rhi_initialized = true;
        }

        void rhi_null::shutdown()
        {
            // Nothing to implement
        }
    } // namespace renderer
}