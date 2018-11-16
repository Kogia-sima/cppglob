cmake_minimum_required(VERSION 3.1.0)

add_library(cppglob ${cpp_sources})
target_link_libraries(cppglob stdc++fs)
set_target_properties(cppglob
  PROPERTIES VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

cotire(cppglob)

if (BUILD_SHARED_LIBS)
  install(
    TARGETS cppglob
    EXPORT cppglob-config
    DESTINATION lib
    COMPONENT libs)
else()
  install(
    TARGETS cppglob
    EXPORT cppglob-config
    ARCHIVE DESTINATION lib
    COMPONENT libs)
endif()