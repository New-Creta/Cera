add_library(cera_rhi_null STATIC)

# Sources
## private sources
target_sources(cera_rhi_null PRIVATE 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/private/rhi_null.cpp)

## public sources
target_sources(cera_rhi_null PUBLIC 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/public/rhi_null.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/public public)

# Private includes
target_include_directories(cera_rhi_null PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/private)

# Public includes
target_include_directories(cera_rhi_null PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_null/public)

# Link libraries
target_link_libraries(cera_rhi_null PUBLIC cera_rhi_core)

# Target properties
set_target_properties(cera_rhi_null PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_rhi_null PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                      