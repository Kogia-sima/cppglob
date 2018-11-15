cmake_minimum_required(VERSION 3.1.0)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/cppglob.dir/src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(cppglob_test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
target_link_libraries(cppglob_test PRIVATE cppglob stdc++fs ${GTEST_LIBRARIES} gmock)

add_test(
  NAME test_cmd
  COMMAND $<TARGET_FILE:cppglob_test>
)

add_custom_target(run_test
  COMMAND mkdir -p coverage
  COMMAND ${CMAKE_CTEST_COMMAND} -V > ctest.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS cppglob_test
  COMMENT "Running test"
)

add_custom_target(gcov
  COMMAND echo "=============== GCOV ==============="
  COMMAND gcov -b -o ${OBJECT_DIR} ${cpp_sources}
  ${OBJECT_DIR}/*.gcno > gcov.log
  COMMAND cat gcov.log |sed '$$s/^Lines executed/TOTAL/'
  COMMAND echo "=============== END ==============="
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
  COMMENT "Display test coverage"
)
add_dependencies(gcov run_test)

add_custom_target(lcov
  COMMAND lcov -c -b ${CMAKE_SOURCE_DIR}/src -d ${OBJECT_DIR} -o coverage/lcov.log
  COMMAND genhtml --demangle-cpp -o coverage/html coverage/lcov.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating test coverage report"
)
add_dependencies(lcov run_test)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/ctest.log)
