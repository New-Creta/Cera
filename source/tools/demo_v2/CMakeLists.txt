add_executable(demo_v2)

target_sources(demo_v2 PRIVATE 
    # shaders
    ${SOURCE_TOOLS_DIRECTORY}/demo_v2/private/demo.h
    ${SOURCE_TOOLS_DIRECTORY}/demo_v2/private/demo.cpp)

# Create the project filters
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/demo_v2/resources resources)
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/demo_v2/private private)

target_include_directories(demo_v2 PRIVATE ${SOURCE_RUNTIME_DIRECTORY}/cera_app/public)

target_link_libraries(demo_v2 cera_app)

# Target properties
set_target_properties(demo_v2 PROPERTIES
                      FOLDER "tools")