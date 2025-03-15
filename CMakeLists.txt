cmake_minimum_required(VERSION 3.19)

### these are my personal configurations, you can modify them with your own
set(CMAKE_TOOLCHAIN_FILE "/home/hugo/vcpkg_dir/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
###

project("xdb" LANGUAGES CXX ASM)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libedit REQUIRED IMPORTED_TARGET libedit)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")

if(BUILD_TESTING)
  find_package(Catch2 REQUIRED)
  add_subdirectory("test")
endif()