include(GNUInstallDirs)

set(OCTF_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(OCTF_LIBRARY_DIR "${CMAKE_INSTALL_FULL_LIBDIR}")
set(OCTF_BIN_DIR "${CMAKE_INSTALL_FULL_BINDIR}")
set(OCTF_CONFIG_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}")
set(OCTF_RUN_DIR "${CMAKE_INSTALL_FULL_RUNSTATEDIR}/${PROJECT_NAME}")
set(OCTF_WORK_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${PROJECT_NAME}")
set(OCTF_DOC_DIR "${CMAKE_INSTALL_FULL_DOCDIR}")
set(OCTF_PACKAGE_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/cmake")
