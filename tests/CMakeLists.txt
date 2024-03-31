include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/ec4fed93217bc2830959bb8e86798c1d86956949.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(libtest src/load_matrix.cpp)

target_link_libraries(libtest gtest_main tatami_mtx)

include(CheckIncludeFiles)
check_include_files(filesystem HAVE_CXX_FS)
if (NOT HAVE_CXX_FS) 
    target_link_libraries(libtest stdc++fs) 
endif()

target_compile_options(libtest PRIVATE -Wall -Wextra -Wpedantic -Werror)

set(CODE_COVERAGE OFF CACHE BOOL "Enable coverage testing")
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libtest PRIVATE -O0 -g --coverage)
    target_link_options(libtest PRIVATE --coverage)
endif()

# Making the tests discoverable.
include(GoogleTest)
gtest_discover_tests(libtest)
