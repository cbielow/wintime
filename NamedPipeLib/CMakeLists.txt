project(NamedPipeLib)

set(NAMEDPIPELIB "NamedPipeLib${ARCH}" CACHE STRING "Name of the Lib" FORCE)

add_library(${NAMEDPIPELIB} STATIC NamedPipeClient.h NamedPipeClient.cpp NamedPipeServer.h NamedPipeServer.cpp Defs.h Memory.h)

## make sure everyone linking to NamedPipeLib can also use its headers
target_include_directories(${NAMEDPIPELIB} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})