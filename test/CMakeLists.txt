add_executable(test_exe
  test.c
)

set_target_properties(test_exe PROPERTIES OUTPUT_NAME test)
target_link_libraries(test_exe subhook)

if(MSVC)
  set_property(TARGET test_exe
               APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

iF(SUBHOOK_BUILD_ASM_TEST)
  find_program(YASM_EXECUTABLE yasm)
  if(YASM_EXECUTABLE)
    message("Yasm executable: ${YASM_EXECUTABLE}")
  else()
    message(FATAL_ERROR "Yasm is required to compile asm_test")
  endif()

  if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(COMPILER_IS_GCC_OR_CLANG TRUE)
  endif()
  if ((COMPILER_IS_GCC_OR_CLANG AND CMAKE_C_FLAGS MATCHES "\\-m32")
      OR (APPLE AND CMAKE_OSX_ARCHITECTURES MATCHES "i386"))
    set(BITS 32)
  else()
    set(BITS 64)
  endif()

  if(WIN32)
    set(asm_format "win32${BITS}")
  elseif(APPLE)
    set(asm_format "macho${BITS}")
  else()
    set(asm_format "elf${BITS}")
  endif()
  set(asm_file foo_${BITS}.asm)

  if(WIN32 OR APPLE)
    set(options "-f" "${asm_format}" "--prefix=_")
  else()
    set(options "-f" "${asm_format}")
  endif()

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
    COMMAND "${YASM_EXECUTABLE}" ${options} "-o"
      "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
      "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
    MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
  )

  add_executable(asm_test_exe
    asm_test.c
    "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
  )

  set_target_properties(asm_test_exe PROPERTIES OUTPUT_NAME asm_test)
  target_link_libraries(asm_test_exe subhook)

  if(MSVC)
    set_property(TARGET asm_test_exe
                 APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
  elseif(APPLE)
    set_property(TARGET asm_test_exe
                 APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-no_pie \"${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj\"")
  endif()
endif()

add_test(NAME test COMMAND $<TARGET_FILE:test_exe>)

set(expected_output "\
Testing initial install[\r\n]+\
foo_hooked\\(\\) called[\r\n]+\
foo\\(\\) called[\r\n]+\
Testing re-install[\r\n]+\
foo_hooked\\(\\) called[\r\n]+\
foo\\(\\) called[\r\n]+\
Testing trampoline[\r\n]+\
foo_hooked_tr\\(\\) called[\r\n]+\
foo\\(\\) called[\r\n]+\
")
set_tests_properties(test PROPERTIES
                     PASS_REGULAR_EXPRESSION "${expected_output}")

if(WIN32 AND NOT SUBHOOK_STATIC)
  set_tests_properties(test PROPERTIES
                       ENVIRONMENT PATH=$<TARGET_FILE_DIR:subhook>)
endif()
