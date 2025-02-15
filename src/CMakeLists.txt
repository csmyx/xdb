add_library(libxdb process.cpp)
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
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
)

include(GNUInstallDirs)

install(TARGETS libxdb
  EXPORT xdb-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT xdb-targets
  FILE xdb-config.cmake
  NAMESPACE xdb::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xdb
)