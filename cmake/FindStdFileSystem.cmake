if(MSVC)
  # avoid erro on find_package_handle_standard_args
  set(STDFILESYSTEM_LIBRARY "__")
else()
  set(STDFILESYSTEM_LIBRARY stdc++fs)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(STDFILESYSTEM DEFAULT_MSG STDFILESYSTEM_LIBRARY)

if (MSVC)
  set(STDFILESYSTEM_LIBRARY "")
endif()

mark_as_advanced(STDFILESYSTEM_FOUND STDFILESYSTEM_LIBRARY)
