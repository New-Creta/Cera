add_executable(demo)

target_sources(demo PRIVATE 
    # shaders
    ${SOURCE_TOOLS_DIRECTORY}/demo/resources/PixelShader.hlsl
    ${SOURCE_TOOLS_DIRECTORY}/demo/resources/VertexShader.hlsl

    ${SOURCE_TOOLS_DIRECTORY}/demo/private/camera.h
    ${SOURCE_TOOLS_DIRECTORY}/demo/private/camera.cpp
    ${SOURCE_TOOLS_DIRECTORY}/demo/private/demo.h
    ${SOURCE_TOOLS_DIRECTORY}/demo/private/demo.cpp)

set_source_files_properties( ${SOURCE_TOOLS_DIRECTORY}/demo/resources/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties( ${SOURCE_TOOLS_DIRECTORY}/demo/resources/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

# Create the project filters
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/demo/resources resources)
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/demo/private private)

target_include_directories(demo PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public)

target_link_libraries(demo cera_app)

# Target properties
set_target_properties(demo PROPERTIES
                      FOLDER "tools")