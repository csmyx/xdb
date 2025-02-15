add_executable(xdb xdb.cpp)
target_link_libraries(xdb PRIVATE xdb::libxdb PkgConfig::libedit)

include(GNUInstallDirs)
install(
  TARGETS xdb
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(demo demo.cpp)
