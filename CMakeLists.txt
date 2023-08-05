cmake_minimum_required(VERSION 3.14)

project(tatami_mtx
    VERSION 1.0.0
    DESCRIPTION "Matrix Market to tatami matrices"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(tatami_mtx INTERFACE)
add_library(tatami::tatami_mtx ALIAS tatami_mtx)

option(TATAMI_MTX_FETCH_EXTERN "Automatically fetch tatami_mtx's external dependencies." ON)
if(TATAMI_MTX_FETCH_EXTERN)
    add_subdirectory(extern)
else()
    find_package(tatami_tatami CONFIG REQUIRED)
    find_package(tatami_eminem CONFIG REQUIRED)
endif()

target_link_libraries(tatami_mtx INTERFACE tatami::tatami tatami::eminem)

# Switch between include directories depending on whether the downstream is
# using the build directly or is using the installed package.
include(GNUInstallDirs)
target_include_directories(tatami_mtx
    INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tatami_mtx>"
)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(TATAMI_MTX_TESTS "Build tatami_mtx's test suite." ON)
else()
    option(TATAMI_MTX_TESTS "Build tatami_mtx's test suite." OFF)
endif()

if(TATAMI_MTX_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Installing for find_package.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tatami_mtx)

install(TARGETS tatami_mtx 
    EXPORT tatami_mtxTargets)

install(EXPORT tatami_mtxTargets
    FILE tatami_tatami_mtxTargets.cmake
    NAMESPACE tatami::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tatami_tatami_mtx)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/tatami_tatami_mtxConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tatami_tatami_mtx)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tatami_tatami_mtxConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tatami_tatami_mtx)
