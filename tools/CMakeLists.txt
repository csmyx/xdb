add_executable(xdb xdb.cpp)
target_link_libraries(xdb PRIVATE xdb::libxdb PkgConfig::libedit fmt::fmt)

include(GNUInstallDirs)
install(
  TARGETS xdb
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(demo demo.cpp)
