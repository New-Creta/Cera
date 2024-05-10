add_library(cera_rhi STATIC)

# Sources
## private sources
target_sources(cera_rhi PRIVATE 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/private/rhi.cpp)

## public sources
target_sources(cera_rhi PUBLIC 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_byte_address_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_format.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_index_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_pipeline_state_object.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_resource.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_shader_resource_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_texture.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_unordered_access_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/public/rhi_vertex_buffer.h
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
                      