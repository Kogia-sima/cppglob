cmake_minimum_required(VERSION 3.1.0)

find_package(StdFileSystem)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/src/CMakeFiles/cppglob.dir)

function(cppglob_test name source)
  add_executable(${name} ${source} ${CMAKE_CURRENT_SOURCE_DIR}/doctest.cpp)

  if(BUILD_SHARED)
    target_link_libraries(${name} PRIVATE cppglob ${STDFILESYSTEM_LIBRARY})
  else()
    target_link_libraries(${name} PRIVATE cppglob_static ${STDFILESYSTEM_LIBRARY})
  endif()

  if(WITH_COTIRE)
    cotire(${name})
  endif()

  add_test(
    NAME "${name}_test"
    COMMAND $<TARGET_FILE:${name}>
  )
endfunction()

cppglob_test(posix ${CMAKE_CURRENT_SOURCE_DIR}/posix.cpp)
cppglob_test(windows ${CMAKE_CURRENT_SOURCE_DIR}/windows.cpp)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(REGEX MATCH "^[0-9]*" gcc_major ${CMAKE_CXX_COMPILER_VERSION})
    message(STATUS "GCC major version: ${gcc_major}")
    if ("${gcc_major}" STREQUAL "")
      set(gcov_cmd "gcov")
    else()
      set(gcov_cmd "gcov-${gcc_major}")
    endif()
  else()
    set(gcov_cmd "gcov")
  endif()

  add_custom_target(reset_counter
    COMMAND lcov -d ${OBJECT_DIR} --zerocounters
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Resetting test counter"
  )

  add_custom_target(run_test
    COMMAND mkdir -p coverage
    COMMAND ${CMAKE_CTEST_COMMAND} -V > ctest.log
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS cppglob_test reset_counter
    COMMENT "Running test"
  )

  add_custom_target(lcov
    COMMAND lcov --gcov-tool ${gcov_cmd} -c -b ${CMAKE_SOURCE_DIR}/src -d ${OBJECT_DIR} -o coverage.info > lcov.log
    COMMAND lcov --remove coverage.info '/usr/*' -o coverage.info >> lcov.log
    COMMAND genhtml --demangle-cpp -o coverage coverage.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS run_test
    COMMENT "Generating test coverage report"
  )

  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_BINARY_DIR}/coverage.info
    ${CMAKE_BINARY_DIR}/coverage
    ${CMAKE_BINARY_DIR}/ctest.log
    ${CMAKE_BINARY_DIR}/lcov.log)
endif()
