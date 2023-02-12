project(WinTimeDll)

set(WINTIME_DLL "WinTimeDll${ARCH}" CACHE STRING "Name of the injection DLL." FORCE)

add_library(${WINTIME_DLL} MODULE WinTimeDll.cpp)

target_link_libraries(${WINTIME_DLL} PRIVATE ${NAMEDPIPELIB})



if (CMAKE_GENERATOR MATCHES "Visual Studio")
  file(TO_NATIVE_PATH "${WinTime_BINARY_DIR}/$(ConfigurationName)/" WinTime_EXE_FULLPATH)
elseif(NOT GENERATOR_IS_MULTI_CONFIG)
  file(TO_NATIVE_PATH "${WinTime_BINARY_DIR}" WinTime_EXE_FULLPATH)
else()
  message(WARNING "Sorry, multiconfig generators on windows other than Visual Studio not supported yet.
          Please look for the line of this error and implement some CMake Generator expressions to copy
          DLLs to the binaries, or modify your environment for the tests to find all library DLLs.")
endif()

set(WinTime_EXE_FULLPATH_CACHE ${WinTime_EXE_FULLPATH} CACHE STRING "where is Wintime?" FORCE)


add_custom_command(TARGET ${WINTIME_DLL}
                   POST_BUILD
                   COMMAND ECHO "Create path '${WinTime_EXE_FULLPATH}'  and copy '$<TARGET_FILE:${WINTIME_DLL}>' into it"
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${WinTime_EXE_FULLPATH} ## if this runs for the first time, the target dir might not exist. So create it.
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${WINTIME_DLL}> ${WinTime_EXE_FULLPATH}
                  )
                  
## this is not a build time dependency, but a runtime dependency; so we add it here for convenience
## (do not move this line to WinTime project, since WinTimeDLL is only processed after WinTime)
add_dependencies(${WINTIME_EXE} ${WINTIME_DLL})
