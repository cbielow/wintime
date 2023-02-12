project(WinTime)

set(WINTIME_EXE "WinTime${ARCH}" CACHE STRING "Name of the exe" FORCE)
set(WINTIME_EXE_OTHERARCH "WinTime${ARCHOTHER}" CACHE STRING "Name of the exe of other bitness" FORCE)


## run that here, so the file is present... (but run configure_file later, since we need WinTimeDLL in there as well)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h" "")
add_executable(${WINTIME_EXE} WinTime.cpp args.hxx Time.h Time.cpp FileLog.h FileLog.cpp Console.h Console.cpp Arch.h Arch.cpp Process.h Process.cpp "${CMAKE_CURRENT_BINARY_DIR}/config.h")

target_link_libraries(${WINTIME_EXE} PRIVATE ${NAMEDPIPELIB})

## include the binary dir to have access to config.h
target_include_directories(${WINTIME_EXE} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

