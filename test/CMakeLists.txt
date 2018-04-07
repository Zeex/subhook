add_executable(test_exe
  test.c
)

set_target_properties(test_exe PROPERTIES OUTPUT_NAME test)
target_link_libraries(test_exe subhook)

if(MSVC)
  set_property(TARGET test_exe
               APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

iF(BUILD_ASM_TEST)
  enable_language(ASM_NASM)

  if(CMAKE_SIZEOF_VOID_P EQUAL 4
      OR (CMAKE_COMPILER_IS_GNUCC AND (CMAKE_C_FLAGS MATCHES "\\-m32")))
    if(WIN32)
      set(asm_format "win32")
    else()
      set(asm_format "elf32")
    endif()
    set(asm_file foo_32.asm)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(WIN32)
      set(asm_format "win64")
    else()
      set(asm_format "elf64")
    endif()
    set(asm_file foo_64.asm)
  endif()

  if(WIN32)
    if("${CMAKE_ASM_NASM_COMPILER}" MATCHES "[yY][aA][sS][mM]")
      set(options "-f" "${asm_format}" "--prefix=_")
    else()
      set(options "-f${asm_format}" "--prefix" "_")
    endif()
  else()
    set(options "-f" "${asm_format}")
  endif()

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
    COMMAND "${CMAKE_ASM_NASM_COMPILER}" ${options} "-o"
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
