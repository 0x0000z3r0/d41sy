cmake_minimum_required(VERSION 3.20)
project(Sandbox C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include_directories(../include)

add_executable(report-demo report.c)
target_compile_definitions(report-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

add_executable(loader-demo loader.c ../source/loader.c ../source/report.c)
target_compile_definitions(loader-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(loader-demo PRIVATE advapi32)

add_executable(memfile-demo memfile.c)
target_compile_definitions(memfile-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

add_executable(privilege-demo privilege.c ../source/privilege.c ../source/report.c)
target_compile_definitions(privilege-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(privilege-demo PRIVATE advapi32)

add_executable(sysinfo-demo sysinfo.c)
target_compile_definitions(sysinfo-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(sysinfo-demo PRIVATE ntdll)

add_executable(ioctl-demo ioctl.c)
target_compile_definitions(ioctl-demo PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(ioctl-demo PRIVATE ntdll)