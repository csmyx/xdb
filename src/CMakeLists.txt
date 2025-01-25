add_library(libxdb libxdb.cpp)
add_library(xdb::libxdb ALIAS libxdb)

set_target_properties(
  libxdb
  PROPERTIES
    OUTPUT_NAME xdb
)

target_compile_features(
  libxdb
  PUBLIC
    cxx_std_17
)

target_include_directories(
  libxdb
  PRIVATE include
  PUBLIC ${CMAKE_SOURCE_DIR}/include
)