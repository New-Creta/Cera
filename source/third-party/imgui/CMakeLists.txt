cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

project(imgui)

SET(IMGUI_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include)
SET(IMGUI_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src)

add_library(imgui STATIC)

target_sources(imgui PUBLIC 
    # device
    ${IMGUI_INCLUDE_DIRECTORY}/imgui_impl_win32.h 

    ${IMGUI_INCLUDE_DIRECTORY}/imconfig.h 
    ${IMGUI_INCLUDE_DIRECTORY}/imgui.h 
    ${IMGUI_INCLUDE_DIRECTORY}/imstb_rectpack.h
    ${IMGUI_INCLUDE_DIRECTORY}/imstb_textedit.h 
    ${IMGUI_INCLUDE_DIRECTORY}/imstb_truetype.h)

target_sources(imgui PRIVATE
    # device
    ${IMGUI_SOURCE_DIRECTORY}/imgui_impl_win32.cpp

    ${IMGUI_SOURCE_DIRECTORY}/imgui.cpp 
    ${IMGUI_SOURCE_DIRECTORY}/imgui_widgets.cpp
    ${IMGUI_SOURCE_DIRECTORY}/imgui_tables.cpp
    ${IMGUI_SOURCE_DIRECTORY}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIRECTORY}/imgui_demo.cpp)

target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIRECTORY})

set_target_properties(imgui PROPERTIES
                      FOLDER "third-party/IMGUI")