add_library(cera_core STATIC)

# Sources
## private sources
target_sources(cera_core PRIVATE 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/memory_size.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/log.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/assert.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/types.cpp)

## public sources
target_sources(cera_core PUBLIC 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/memory_size.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/log.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/assert.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/types.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_core/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_core/public public)

# Private includes
target_include_directories(cera_core PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private)

# Public includes
target_include_directories(cera_core PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public)
target_include_directories(cera_core PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/fmt/include)

# Target properties
set_target_properties(cera_core PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_core PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                      