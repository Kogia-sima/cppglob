cmake_minimum_required(VERSION 3.1.0)

# google test
find_package(GTest)

if (NOT GTEST_FOUND)
  if (NOT EXISTS ${PROJECT_SOURCE_DIR}/3rdparty/googletest/googletest)
    message(FATAL_ERROR "googletest submodule not found. Please run `git submodule update --init --recursive` to fetch the googletest source code")
  endif()
  add_subdirectory(
    ${PROJECT_SOURCE_DIR}/3rdparty/googletest
    ${CMAKE_BINARY_DIR}/googletest)
  set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
  set(GTEST_BOTH_LIBRARIES gtest gtest_main)
endif()

find_package(Threads REQUIRED)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/src/CMakeFiles/cppglob.dir)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(cppglob_test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

if(BUILD_SHARED)
  target_link_libraries(cppglob_test PRIVATE cppglob stdc++fs ${GTEST_BOTH_LIBRARIES} Threads::Threads)
else()
  target_link_libraries(cppglob_test PRIVATE cppglob_static stdc++fs ${GTEST_BOTH_LIBRARIES} Threads::Threads)
endif()

add_test(
  NAME test_cmd
  COMMAND $<TARGET_FILE:cppglob_test>
)


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
