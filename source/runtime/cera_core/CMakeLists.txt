add_library(cera_core STATIC)

# Sources
## private sources
if(WIN32)
target_sources(cera_core PUBLIC
        ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/windows/win32_platform_helpers.h)
    target_sources(cera_core PRIVATE 
        ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/windows/win32_platform_helpers.cpp)
endif()
target_sources(cera_core PRIVATE 
    ### util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/hash.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/string_op.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/defines.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/blob.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/blob_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/blob_reader.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/blob_writer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/pointer_math.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/color.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/memory_size.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/log.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/assert.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/private/util/types.cpp)

## public sources
target_sources(cera_core PUBLIC 
    ### util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/hash.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/string_op.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/defines.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/blob.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/blob_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/blob_reader.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/blob_writer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/pointer_math.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/color.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/memory_size.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/log.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/assert.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/util/types.h
    
    ${SOURCE_RUNTIME_DIRECTORY}/cera_core/public/core_platform.h)

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
                      