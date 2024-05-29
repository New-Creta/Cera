add_library(cera_rhi_directx STATIC)

# Sources
## private sources
target_sources(cera_rhi_directx PRIVATE 
    ### descriptors
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocation.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocator_page.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocator.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/dynamic_descriptor_heap.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocation.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocator_page.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/descriptor_allocator.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/descriptors/dynamic_descriptor_heap.cpp

    ### dxgi
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_adapter.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_adapter_manager.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_format.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_util.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_adapter.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_adapter_manager.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_format.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/dxgi/dxgi_util.h

    ### resources
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/byte_address_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/clear_state.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/constant_buffer_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/constant_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/index_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/pipeline_state_object.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/raster_state.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/render_target.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/resource_state_tracker.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/resource_state_tracker.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/resource.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/root_signature.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/shader_library.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/shader_resource_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/texture.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/unordered_access_view.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/upload_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/vertex_buffer.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/byte_address_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/clear_state.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/constant_buffer_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/constant_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/index_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/pipeline_state_object.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/raster_state.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/render_target.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/resource_state_tracker.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/resource.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/root_signature.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/shader_library.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/shader_resource_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/texture.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/unordered_access_view.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/upload_buffer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/resources/vertex_buffer.cpp

    ### wrl
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/wrl/comobject.h

    ### util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/thread_name.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/thread_name.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/vertex_types.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/vertex_types.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/queue.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/windows_types.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/util/state_controller.h

    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/command_list.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/command_list.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/command_queue.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/command_queue.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_call.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_call.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_device.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_device.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_swapchain.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_swapchain.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_util.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/directx_renderer.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_adapter_child.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_device_child.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_windows_target_settings.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_windows_target_settings.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_windows_config.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_windows_config.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private/rhi_directx.cpp)

## public sources
target_sources(cera_rhi_directx PUBLIC 
    ### resource descriptions
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_clear_state_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_index_buffer_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_pipeline_state_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_raster_state_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_resource_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_texture_desc.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/resource_descriptions/create_vertex_buffer_desc.h

    ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public/rhi_directx.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public public)

# Private includes
target_include_directories(cera_rhi_directx PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/private)

# Public includes
target_include_directories(cera_rhi_directx PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_rhi/cera_rhi_directx/public)
target_include_directories(cera_rhi_directx PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/directxextra/include)
target_include_directories(cera_rhi_directx PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/directxtex)

# Link libraries
target_link_libraries(cera_rhi_directx PUBLIC cera_rhi_core)
target_link_libraries(cera_rhi_directx PUBLIC DirectXTex)
target_link_libraries(cera_rhi_directx PUBLIC DirectXExtra)

# Target properties
set_target_properties(cera_rhi_directx PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_rhi_directx PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                    
target_compile_definitions(cera_rhi_directx PUBLIC CERA_D3D12_CORE_ENABLED=1)