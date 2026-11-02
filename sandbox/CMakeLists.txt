cmake_minimum_required(VERSION 3.20)
project(FlourishSandbox C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(flourish-demo flourish.c)
target_include_directories(flourish-demo PRIVATE ../include)
target_compile_definitions(flourish-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

add_executable(loader-demo loader.c)
target_compile_definitions(loader-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(loader-demo PRIVATE advapi32)

add_executable(memfile-demo memfile.c)
target_compile_definitions(memfile-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
