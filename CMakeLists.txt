cmake_minimum_required(VERSION 3.2)

project(cera)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# cera
set(cera_include_dir "${CMAKE_CURRENT_LIST_DIR}/src/cera/include")
set(cera_src_dir "${CMAKE_CURRENT_LIST_DIR}/src/cera/src")

set(cera_include_files "${cera_include_dir}/cera/types.h")
set(cera_source_files "${cera_src_dir}/types.cpp")

add_library(cera ${cera_include_files} ${cera_source_files})

set_property(TARGET cera PROPERTY CXX_STANDARD 17)
set_property(TARGET cera PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(cera PRIVATE ${cera_include_dir})

set_target_properties( cera PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set_target_properties( cera PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set_target_properties( cera PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# sandbox
set(common_include_dir "${CMAKE_CURRENT_LIST_DIR}/examples/common/include")
set(common_src_dir "${CMAKE_CURRENT_LIST_DIR}/examples/common/src")
set(sandbox_include_dir "${CMAKE_CURRENT_LIST_DIR}/examples/sandbox/include")
set(sandbox_src_dir "${CMAKE_CURRENT_LIST_DIR}/examples/sandbox/src")

set(sandbox_include_files "${common_include_dir}/entry.h")
                        
set(sandbox_source_files "${common_src_dir}/win32/entry.cpp"
                        "${sandbox_src_dir}/sandbox.cpp")

add_executable(sandbox ${sandbox_include_files} ${sandbox_source_files})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

set_property(TARGET sandbox PROPERTY CXX_STANDARD 17)
set_property(TARGET sandbox PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(sandbox PRIVATE ${cera_include_dir})
target_include_directories(sandbox PRIVATE ${common_include_dir})
target_include_directories(sandbox PRIVATE ${common_include_dir}/win32)
target_include_directories(sandbox PRIVATE ${sandbox_include_dir})

target_link_libraries(sandbox PRIVATE cera)

set_target_properties(sandbox PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set_target_properties(sandbox PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set_target_properties(sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")