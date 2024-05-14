cmake_minimum_required(VERSION 3.28)

project(cera)

option(CERA_RHI_NULL "Disable renderer initialization" OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DCERA_PLATFORM_X64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DCERA_PLATFORM_X86)
else()
	message( FATAL_ERROR "Unknown CPU architecture, CMAKE will exit." )
endif()

if(MSVC)
	# Remove the always defined _WINDOWS flag
	# CPP
	string (REPLACE "/D_WINDOWS" "" CMAKE_CUSTOM_CPP_FLAGS ${CMAKE_CXX_FLAGS})
	string (REPLACE "/DWIN32" "" CMAKE_CUSTOM_CPP_FLAGS ${CMAKE_CUSTOM_CPP_FLAGS}) 
	SET(CMAKE_CXX_FLAGS "${CMAKE_CUSTOM_CPP_FLAGS}" CACHE STRING "" FORCE)
	# C
	string (REPLACE "/D_WINDOWS" "" CMAKE_CUSTOM_C_FLAGS ${CMAKE_C_FLAGS})
	string (REPLACE "/DWIN32" "" CMAKE_CUSTOM_C_FLAGS ${CMAKE_CUSTOM_C_FLAGS})
	SET(CMAKE_C_FLAGS "${CMAKE_CUSTOM_C_FLAGS}" CACHE STRING "" FORCE)

	add_definitions(-DCERA_COMPILER_MSVC)
endif()

# Use C++ 17 as a standard
SET(CMAKE_CXX_STANDARD 17)
# Debug postfix
SET(CMAKE_DEBUG_POSTFIX _d)
# Enable folder structure in the IDE
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Some easy access to folders within this repository
SET(SOURCE_THIRDPARTY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/third-party)
SET(SOURCE_RUNTIME_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/runtime)
SET(SOURCE_TOOLS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/tools)

MESSAGE(STATUS ${SOURCE_THIRDPARTY_DIRECTORY})
MESSAGE(STATUS ${SOURCE_RUNTIME_DIRECTORY})
MESSAGE(STATUS ${SOURCE_TOOLS_DIRECTORY})

# Specify the usage of UNICODE
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

add_subdirectory(${SOURCE_THIRDPARTY_DIRECTORY})
add_subdirectory(${SOURCE_RUNTIME_DIRECTORY})
add_subdirectory(${SOURCE_TOOLS_DIRECTORY})