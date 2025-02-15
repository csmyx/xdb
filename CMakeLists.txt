cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "/home/hugo/vcpkg_dir/vcpkg/scripts/buildsystems/vcpkg.cmake")
project("xdb" LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libedit REQUIRED IMPORTED_TARGET libedit)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")

if(BUILD_TESTING)
  find_package(Catch2 REQUIRED)
  add_subdirectory("test")
endif()