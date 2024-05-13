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

add_subdirectory(cera_core)
add_subdirectory(cera_rhi/cera_rhi_core)

if(CERA_RHI_NULL)
   # disable rendering
   add_subdirectory(cera_rhi/cera_rhi_null)
   SET(CERA_RHI_PLATFORM cera_rhi_null)
elseif(WIN32)
   # use directx on windows
   add_subdirectory(cera_rhi/cera_rhi_directx)
   SET(CERA_RHI_PLATFORM cera_rhi_directx)
endif()

add_subdirectory(cera_app)

