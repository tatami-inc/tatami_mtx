include(FetchContent)
FetchContent_Declare(
    tatami_test
    GIT_REPOSITORY https://github.com/tatami-inc/tatami_test
    GIT_TAG master
)
FetchContent_MakeAvailable(tatami_test)

add_executable(libtest src/load_matrix.cpp)
target_link_libraries(libtest tatami_mtx tatami_test)

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
