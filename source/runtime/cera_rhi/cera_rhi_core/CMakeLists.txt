add_library(cera_rhi_core STATIC)

# Sources
## private sources
target_sources(cera_rhi_core PRIVATE 
    ### resources
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/resources/rhi_resource.cpp

    ### common
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_resource_pool.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_resource_slot.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_resource_slots.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_clear_bits.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_data_driven_shader_platform_info.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_data_driven_shader_platform_registry.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_data_driven_shader_platform_registry.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_shader_format_names.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/common/rhi_shader_language_names.cpp

    ### renderer
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/renderer/rhi_renderer.cpp

    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private/rhi_globals.cpp)

## public sources
target_sources(cera_rhi_core PUBLIC 
    ### resources
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_byte_address_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_index_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_pipeline_state_object.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_resource.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_shader_resource_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_texture.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_unordered_access_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/resources/rhi_vertex_buffer.h

    ### common
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_resource_pool.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_resource_slot.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_resource_slots.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_viewport.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_scissor_rect.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_clear_bits.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_cull_mode.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_fill_mode.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_primitive_topology.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_format.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_shader_type.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_shader_platform.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_feature_level.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_data_driven_shader_platform_info.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_shader_format_names.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/common/rhi_shader_language_names.h

    ### renderer
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/renderer/rhi_renderer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/renderer/rhi_renderer_backend.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/renderer/rhi_renderer_output_window_user_data.h

    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/rhi_factory.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/rhi_globals.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public/rhi.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public public)

# Private includes
target_include_directories(cera_rhi_core PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/private)

# Public includes
target_include_directories(cera_rhi_core PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_core/public)

# Link libraries
target_link_libraries(cera_rhi_core PUBLIC cera_core)

# Target properties
set_target_properties(cera_rhi_core PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_rhi_core PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                      