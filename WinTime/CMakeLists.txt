project(WinTime)

set(WINTIME_EXE "WinTime${ARCH}" CACHE STRING "Name of the exe" FORCE)
set(WINTIME_EXE_OTHERARCH "WinTime${ARCHOTHER}" CACHE STRING "Name of the exe of other bitness" FORCE)


configure_file("${WinTime_SOURCE_DIR}/config.h.in" "${WinTime_BINARY_DIR}/config.h" @ONLY)
add_executable(${WINTIME_EXE} WinTime.cpp args.hxx Time.h Time.cpp FileLog.h FileLog.cpp Console.h Console.cpp Arch.h Arch.cpp Memory.h Process.h Process.cpp "${CMAKE_CURRENT_BINARY_DIR}/config.h")

## include the binary dir to have access to config.h
target_include_directories(${WINTIME_EXE} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

