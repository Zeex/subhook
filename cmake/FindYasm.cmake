include(FindPackageHandleStandardArgs)

find_file(YASM_EXECUTABLE yasm)
mark_as_advanced(YASM_EXECUTABLE)

find_package_handle_standard_args(Yasm
  FOUND_VAR YASM_FOUND
  REQUIRED_VARS YASM_EXECUTABLE
)