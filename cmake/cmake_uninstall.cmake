set(MANIFEST "${CMAKE_BINARY_DIR}/install_manifest.txt")

if(NOT EXISTS ${MANIFEST})
  message(FATAL_ERROR "Cannot find install manifestfile. Please retry after reinstall.")
endif()

file(STRINGS ${MANIFEST} files)
foreach(file ${files})
  if(EXISTS ${file} OR IS_SYMLINK ${file})
    message(STATUS "Removing file: '${file}'")

    execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove ${file}
      OUTPUT_VARIABLE stdout
      RESULT_VARIABLE result
    )

    if(NOT "${result}" STREQUAL 0)
      message(SEND_ERROR "Failed to remove file: '${file}'.")
    endif()
  else()
    message(SEND_ERROR "File '${file}' does not exist.")
  endif()
endforeach(file)
