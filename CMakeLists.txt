cmake_minimum_required(VERSION 3.12)

project(WinTimeAll VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)

## requires a 'project()' call first!
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(ARCH "32")
  set(ARCHOTHER "64")
else()
  set(ARCH "64")
  set(ARCHOTHER "32")
endif()

message(STATUS "Target Arch is ${ARCH}")

## the order needs to be exactly this (due to internal references)
add_subdirectory(NamedPipeLib)
add_subdirectory(WinTime)
add_subdirectory(WinTimeDll) ## needs to go last, since it refs WinTime

add_subdirectory(ExampleTarget)

## run that here again (already done in WinTime), but now WinTimeDll data is available
file(REMOVE "${WinTime_BINARY_DIR}/config.h")
configure_file("${WinTime_SOURCE_DIR}/config.h.in" "${WinTime_BINARY_DIR}/config.h" @ONLY)
