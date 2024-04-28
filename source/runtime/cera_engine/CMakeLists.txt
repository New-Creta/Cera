add_library(cera_engine STATIC)

# Sources
target_sources(cera_engine PRIVATE 
    # util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/util/memory_definitions.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/util/memory_helpers.h
    # render
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/vertex_types.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/mesh_factory.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/pipeline_state_object.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/constant_buffer_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/constant_buffer_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/shader_resource_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/shader_resource_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/unordered_access_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/unordered_access_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/render_target.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/resource_state_tracker.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/resource_state_tracker.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/constant_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/constant_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/byte_address_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/byte_address_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/resource.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/texture.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/vertex_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/vertex_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/index_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/index_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/command_list.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/root_signature.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/dynamic_descriptor_heap.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/dynamic_descriptor_heap.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/descriptor_allocation.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/descriptor_allocator_page.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/descriptor_allocator_page.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/descriptor_allocator.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/descriptor_allocator.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/upload_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/upload_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/command_queue.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/swapchain.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/device.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/render/d3dx12_call.cpp
    
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/gui.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/mesh.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/scene.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private/scene_node.cpp)

target_sources(cera_engine PUBLIC 
    # device
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/device/windows_declarations.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/device/windows_types.h
    # render
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/vertex_types.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/mesh_factory.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/command_list.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/resource.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/texture.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/pipeline_state_object.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/root_signature.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/descriptor_allocation.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/render_target.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/swapchain.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/command_queue.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/device.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/d3dx12_call.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/d3dx12.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/render/d3dx12_declarations.h
    # util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/util/threading/thread_helpers.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/util/threading/ts_queue.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/util/object_counter.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/util/log.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/util/types.h

    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/gui.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/scene.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/scene_node.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public/mesh.h)

set( SHADER_FILES
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/resources/ImGUI_VS.hlsl
    ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/resources/ImGUI_PS.hlsl)

target_sources(cera_engine PRIVATE ${SHADER_FILES})

set_source_files_properties( ${SHADER_FILES}
    PROPERTIES
        VS_SHADER_MODEL 6.0
        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/resources/%(Filename).h"
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>)

set_source_files_properties( ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/resources/ImGUI_VS.hlsl PROPERTIES
        VS_SHADER_TYPE Vertex
        VS_SHADER_VARIABLE_NAME g_ImGUI_VS
)

set_source_files_properties( ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/resources/ImGUI_PS.hlsl PROPERTIES
        VS_SHADER_TYPE Pixel
        VS_SHADER_VARIABLE_NAME g_ImGUI_PS
)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_engine/resources resources)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public public)

# Private includes
target_include_directories(cera_engine PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/private)
target_include_directories(cera_engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/resources/)

# Include self
target_include_directories(cera_engine PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/glm)
target_include_directories(cera_engine PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/fmt/include)
target_include_directories(cera_engine PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/imgui/include)
target_include_directories(cera_engine PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_engine/public)

target_link_libraries(cera_engine PUBLIC imgui)
target_link_libraries(cera_engine PUBLIC DirectXTex)

target_link_libraries(cera_engine PUBLIC d3d12.lib)
target_link_libraries(cera_engine PUBLIC dxgi.lib)
target_link_libraries(cera_engine PUBLIC dxguid.lib)
target_link_libraries(cera_engine PUBLIC D3DCompiler.lib )
target_link_libraries(cera_engine PUBLIC Shlwapi.lib)

# Compile definitions
target_compile_definitions(cera_engine PUBLIC -DCERA_DX12)

if(${CMAKE_HOST_WIN32})
    target_compile_definitions(cera_engine PUBLIC -DCERA_WINDOWS)
endif()

# Target properties
set_target_properties(cera_engine PROPERTIES
                      FOLDER "runtime")