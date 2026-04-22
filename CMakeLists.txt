cmake_minimum_required(VERSION 3.16)
project(ttyprland C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WLROOTS REQUIRED wlroots-0.21)
pkg_check_modules(WAYLAND REQUIRED wayland-server)
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

add_subdirectory(impl)
list(TRANSFORM TTYPR_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/impl/")
add_executable(ttyprland ${TTYPR_SOURCES})
target_compile_definitions(ttyprland PRIVATE WLR_USE_UNSTABLE)

target_include_directories(ttyprland PRIVATE
    ${WLROOTS_INCLUDE_DIRS}
    ${WAYLAND_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

target_link_libraries(ttyprland
    ${WLROOTS_LIBRARIES}
    ${WAYLAND_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
)

target_compile_options(ttyprland PRIVATE
    ${WLROOTS_CFLAGS_OTHER}
)
