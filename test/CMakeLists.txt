cmake_minimum_required(VERSION 3.1.0)

add_subdirectory(
  ${PROJECT_SOURCE_DIR}/3rdparty/googletest
  ${CMAKE_BINARY_DIR}/googletest)
find_package(Threads REQUIRED)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/cppglob.dir/src)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(cppglob_test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

if(BUILD_SHARED)
  target_link_libraries(cppglob_test PRIVATE cppglob stdc++fs gtest gtest_main Threads::Threads)
else()
  target_link_libraries(cppglob_test PRIVATE cppglob_static stdc++fs gtest gtest_main Threads::Threads)
endif()

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread")
endif()

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
