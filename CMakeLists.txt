cmake_minimum_required(VERSION 3.12)

project(WinTimeAll VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)

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
add_subdirectory(WinTime)

add_subdirectory(ExampleTarget)

