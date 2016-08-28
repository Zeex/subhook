add_executable(test_program test.cpp)

set_target_properties(test_program PROPERTIES OUTPUT_NAME test)
target_link_libraries(test_program subhook)

if(MSVC)
  set_property(TARGET test_program
               APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

add_test(NAME test COMMAND $<TARGET_FILE:test_program>)

file(READ test.out output)
string(STRIP "${output}" output)
set_tests_properties(test PROPERTIES PASS_REGULAR_EXPRESSION "${output}")

if(WIN32 AND NOT SUBHOOK_STATIC)
  set_tests_properties(test PROPERTIES
                       ENVIRONMENT PATH=$<TARGET_FILE_DIR:subhook>)
endif()
