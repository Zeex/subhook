find_program(YASM_EXECUTABLE yasm)
if(YASM_EXECUTABLE)
  message("Yasm executable: ${YASM_EXECUTABLE}")
else()
  message(FATAL_ERROR "Yasm is required to compile the test program")
endif()

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
  endif()
endif()

if(WIN32)
  set(asm_format "win${BITS}")
elseif(APPLE)
  set(asm_format "macho${BITS}")
else()
  set(asm_format "elf${BITS}")
endif()

set(options "-f" "${asm_format}")
if(APPLE
    OR (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUBHOOK_FORCE_32BIT)))
  list(APPEND options "--prefix=_")
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND options "-DUSE_PLT")
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
  COMMAND "${YASM_EXECUTABLE}" ${options} "-o"
    "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
    "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${asm_file}"
)

add_executable(test_exe
  test.c
  "${CMAKE_CURRENT_BINARY_DIR}/${asm_file}.obj"
)

set_target_properties(test_exe PROPERTIES OUTPUT_NAME test)

if(SUBHOOK_FORCE_32BIT)
  if(APPLE)
    set_target_properties(test_exe PROPERTIES OSX_ARCHITECTURES i386)
  endif()
  if(UNIX)
    set_property(TARGET test_exe APPEND_STRING PROPERTY
                 COMPILE_FLAGS " -m32")
    set_property(TARGET test_exe APPEND_STRING PROPERTY LINK_FLAGS " -m32")
  endif()
endif()

target_link_libraries(test_exe subhook)

if(MSVC)
  set_property(TARGET test_exe
               APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

if(APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT SUBHOOK_FORCE_32BIT)
  set_property(TARGET test_exe APPEND_STRING PROPERTY
               LINK_FLAGS " -Wl,-no_pie")
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
