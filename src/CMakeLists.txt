cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)

find_package(StdFileSystem)


if (BUILD_SHARED)
  add_library(cppglob SHARED ${cpp_sources})
  target_link_libraries(cppglob ${STDFILESYSTEM_LIBRARY})
  set_target_properties(cppglob
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  if (WITH_COTIRE)
    cotire(cppglob)
  endif()

  install(
    TARGETS cppglob
    EXPORT cppglob-config
    DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    COMPONENT libs)

endif()

if (BUILD_STATIC)
  add_definitions("-DCPPGLOB_STATIC")
  add_library(cppglob_static STATIC ${cpp_sources})
  target_link_libraries(cppglob_static ${STDFILESYSTEM_LIBRARY})
  set_target_properties(cppglob_static
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  if (WITH_COTIRE)
    cotire(cppglob_static)
  endif()

  install(
    TARGETS cppglob_static
    EXPORT cppglob-config
    ARCHIVE DESTINATION lib
    COMPONENT libs)
endif()

# create pkg-config file
if (STDFILESYSTEM_LIBRARY)
  set(STDFILESYSTEM_LDFLAGS "-l${STDFILESYSTEM_LIBRARY}")
else()
  set(STDFILESYSTEM_LDFLAGS "")
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cppglob.pc.in
  ${CMAKE_BINARY_DIR}/cppglob.pc
  @ONLY)

install(
  FILES ${CMAKE_BINARY_DIR}/cppglob.pc
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
