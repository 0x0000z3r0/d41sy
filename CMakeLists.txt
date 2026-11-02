cmake_minimum_required(VERSION 3.20)
project(D41SY C CXX ASM_MASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(EXE_NAME d41sy)
set(FLR_NAME d41sy-flourish)
set(BIN_NAME flourish)

file(GLOB D41SY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

add_subdirectory(sandbox)

add_executable(${FLR_NAME} source/flourish.asm)
set_target_properties(${FLR_NAME} PROPERTIES
    LINK_FLAGS "/ENTRY:flourish /SUBSYSTEM:WINDOWS /ALIGN:16 /NODEFAULTLIB"
)

add_custom_command(TARGET ${FLR_NAME} POST_BUILD
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/extract_text.ps1" "$<TARGET_FILE:${FLR_NAME}>" "${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.bin"
    COMMENT "Extracting .text section to ${BIN_NAME}.bin..."
)

add_executable(${EXE_NAME} WIN32
    ${D41SY_SOURCES}
    daisy.rc
    daisy.manifest
)
add_dependencies(${EXE_NAME} ${FLR_NAME})

target_include_directories(${EXE_NAME} PRIVATE include)
target_compile_definitions(${EXE_NAME} PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN "EXE_IMAGE_NAME=\"${EXE_NAME}.exe\"")
target_link_libraries(${EXE_NAME} PRIVATE
    d3d11
    dxgi
    dxguid
    d2d1
    dwrite
    dcomp
    windowscodecs
    user32
    gdi32
    ole32
    uuid
    advapi32
)

if(MSVC)
    target_compile_options(${EXE_NAME} PRIVATE /W4)
endif()
