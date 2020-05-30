find_package(Yasm REQUIRED)

if (CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUBHOOK_FORCE_32BIT)
  set(BITS 32)
else()
  set(BITS 64)
endif()

set(asm_file foo_${BITS}.asm)
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT SUBHOOK_FORCE_32BIT)
  if(WIN32)
    set(asm_file foo_64_win.asm)
  elseif(UNIX)
    set(asm_file foo_64_unix.asm)
    set(asm_obj_file ${asm_file}.o)
  endif()
endif()
if(NOT asm_obj_file)
  set(asm_obj_file ${asm_file}.obj)
endif()

if(WIN32)
  set(asm_format "win${BITS}")
elseif(APPLE)
  set(asm_format "macho${BITS}")
else()
  set(asm_format "elf${BITS}")
endif()

set(options "-f" "${asm_format}")
if(APPLE OR (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUBHOOK_FORCE_32BIT)))
  list(APPEND options "--prefix=_")
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND options "-DUSE_PLT")
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${asm_obj_file}"
  COMMAND "${YASM_EXECUTABLE}" ${options} "-o"
    "${CMAKE_CURRENT_BINARY_DIR}/${asm_obj_file}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
)

add_executable(subhook_test_exe
  test.c
  "${CMAKE_CURRENT_BINARY_DIR}/${asm_obj_file}"
)
set_target_properties(subhook_test_exe PROPERTIES OUTPUT_NAME test)

enable_language(CXX)
add_executable(subhook_cxx_test_exe
  test.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/${asm_obj_file}"
)
set_target_properties(subhook_cxx_test_exe PROPERTIES OUTPUT_NAME test++)

foreach(target subhook_test_exe subhook_cxx_test_exe)
  if(SUBHOOK_FORCE_32BIT)
    if(APPLE)
      set_target_properties(${target} PROPERTIES OSX_ARCHITECTURES i386)
    endif()
    if(UNIX)
      set_property(TARGET ${target} APPEND_STRING PROPERTY
                   COMPILE_FLAGS " -m32")
      set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -m32")
    endif()
  endif()

  target_link_libraries(${target} subhook)

  if(MSVC)
    set_property(TARGET ${target}
                 APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
  endif()

  if(APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT SUBHOOK_FORCE_32BIT)
    set_property(TARGET ${target} APPEND_STRING PROPERTY
                 LINK_FLAGS " -Wl,-no_pie")
  endif()

  add_test(NAME ${target}_test COMMAND $<TARGET_FILE:${target}>)

  set(expected_output
"Testing initial install
foo_hooked\\(\\) called
foo\\(\\) called
Testing re-install
foo_hooked\\(\\) called
foo\\(\\) called
Testing trampoline
foo_hooked_tr\\(\\) called
foo\\(\\) called
")
  set_tests_properties(${target}_test PROPERTIES
                       PASS_REGULAR_EXPRESSION "${expected_output}")

  if(WIN32 AND NOT SUBHOOK_STATIC)
    set_tests_properties(${target}_test PROPERTIES
                         ENVIRONMENT PATH=$<TARGET_FILE_DIR:subhook>)
  endif()
endforeach()
