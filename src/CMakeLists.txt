cmake_minimum_required(VERSION 3.1.0)

if (BUILD_SHARED)
  add_library(cppglob SHARED ${cpp_sources})
  target_link_libraries(cppglob stdc++fs)
  set_target_properties(cppglob
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  cotire(cppglob)

  install(
    TARGETS cppglob
    EXPORT cppglob-config
    DESTINATION lib
    COMPONENT libs)

endif()

if (BUILD_STATIC)
  add_library(cppglob_static STATIC ${cpp_sources})
  target_link_libraries(cppglob_static stdc++fs)
  set_target_properties(cppglob_static
    PROPERTIES VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  cotire(cppglob_static)

  install(
    TARGETS cppglob_static
    EXPORT cppglob-config
    ARCHIVE DESTINATION lib
    COMPONENT libs)
endif()
