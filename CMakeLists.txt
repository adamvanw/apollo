cmake_minimum_required(VERSION 3.28)
project(apollo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add static linking flags here:
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static")
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Or, for MSVC runtime (Windows):
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(vendor)
add_subdirectory(src)
