cmake_minimum_required(VERSION 3.10)

project(DirectXExtra VERSION 1.0)

# Add the header-only library
add_library(DirectXExtra INTERFACE)
# Setup source
target_sources(DirectXExtra PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include/d3dx12.h)
# Setup directory within IDE
set_target_properties(DirectXExtra PROPERTIES
                        FOLDER "third-party/DIRECTXEXTRA")