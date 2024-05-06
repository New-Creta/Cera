# -------------------------------
# Macro that will define our source groups
# -------------------------------
MACRO(GROUPSOURCES curdir folder_name)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    FOREACH(child ${children})
           IF(IS_DIRECTORY ${curdir}/${child})
              GROUPSOURCES(${curdir}/${child} ${folder_name}/${child})
           ELSE()
              SOURCE_GROUP(${folder_name} FILES ${curdir}/${child})
           ENDIF()
    ENDFOREACH()
ENDMACRO(GROUPSOURCES)

# add_subdirectory(cera_engine)
add_subdirectory(cera_app)
add_subdirectory(cera_rhi)
add_subdirectory(cera_directx)
