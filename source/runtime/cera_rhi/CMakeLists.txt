add_library(cera_rhi STATIC)

# Sources
## private sources
target_sources(cera_rhi PRIVATE 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi_resource_pool.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi_resource_slot.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi_resource_slots.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi_renderer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi_clear_bits.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi.cpp)

## public sources
target_sources(cera_rhi PUBLIC 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_resource_pool.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_resource_slot.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_resource_slots.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_viewport.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_scissor_rect.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_clear_bits.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_cull_mode.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_fill_mode.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_primitive_topology.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_byte_address_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_format.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_index_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_pipeline_state_object.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_resource.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_shader_type.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_shader_resource_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_texture.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_unordered_access_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_vertex_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_renderer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_renderer_backend.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_renderer_info.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_renderer_output_window_user_data.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public public)

# Private includes
target_include_directories(cera_rhi PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private)

# Public includes
target_include_directories(cera_rhi PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public)
target_include_directories(cera_rhi PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public)

# Link libraries
target_link_libraries(cera_rhi PUBLIC cera_core)

# Target properties
set_target_properties(cera_rhi PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_rhi PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                      