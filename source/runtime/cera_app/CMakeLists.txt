add_library(cera_app STATIC)

# Sources
if(WIN32)
    ## private sources
    target_sources(cera_app PRIVATE 
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_application_helpers.cpp
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_application.cpp
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_entrypoint.cpp
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_window.cpp
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_application_helpers.h
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_application.h
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_system_includes.h
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_system_includes.cpp
        ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/windows/win32_window.h)
endif()
target_sources(cera_app PRIVATE 
    ### util
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/util/clock.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/util/clock.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/util/guard_value.h
    
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/core_application.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/core_globals.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/core_globals.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/abstract_game.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/events.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/game_engine.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/game_engine.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_application_message_handler.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_application_message_handler.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_application.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_application.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_window_definition.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_window_definition.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_window.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/generic_window.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/gui_application.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/gui_application.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/launch.cpp
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private/launch.h)
## public sources
target_sources(cera_app PUBLIC 
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public/generic_application_creation_params.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public/abstract_game.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public/events.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public/generic_entrypoint.h
    ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public/key_codes.h)

# Create the project filters
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_app/private private)
GROUPSOURCES(${SOURCE_RUNTIME_DIRECTORY}/cera_app/public public)

# Private includes
target_include_directories(cera_app PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_app/private)

# Public includes
target_include_directories(cera_app PUBLIC ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public)
target_include_directories(cera_app PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/fmt/include)

# Link libraries
target_link_libraries(cera_app PUBLIC ${CERA_RHI_PLATFORM})

if(WIN32)
    target_link_libraries(cera_app PUBLIC Shlwapi.lib)
    target_link_libraries(cera_app PUBLIC shcore.lib)
    target_link_libraries(cera_app PUBLIC dwmapi.lib)
endif()

# Target properties
set_target_properties(cera_app PROPERTIES FOLDER "runtime")

# Compile definitions
if(WIN32)
    target_compile_definitions(cera_app PUBLIC CERA_PLATFORM_WINDOWS)
endif()
                      