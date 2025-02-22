cmake_minimum_required(VERSION 3.22)
project(minesweeper CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COLOR_DIAGNOSTICS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Wextra -Wshadow -Wno-sign-compare)

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(BUILD_DEBUG)
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif ()

include(FetchContent)
FetchContent_Declare(
    gtl
    GIT_REPOSITORY https://github.com/greg7mdp/gtl.git
    GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(gtl)

# FetchContent_Declare(
#     cereal
#     GIT_REPOSITORY https://github.com/USCiLab/cereal.git
#     GIT_TAG v1.3.2
#     GIT_SHALLOW TRUE
# )

# set(JUST_INSTALL_CEREAL ON) 

# FetchContent_MakeAvailable(cereal) 

add_executable(main main.cpp lua_interface.cpp server_io.cpp)
add_executable(nn nn.cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(LUA REQUIRED lua5.4)

target_link_libraries(main PUBLIC gtl ${LUA_STATIC_LIBRARY_DIRS})
target_include_directories(main PUBLIC ${LUA_INCLUDE_DIRS})

if (CMAKE_BUILD_TYPE MATCHES Release)
    find_package(mimalloc 2.1 REQUIRED)
    target_link_libraries(main PUBLIC mimalloc)
endif()
