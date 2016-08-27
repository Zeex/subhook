add_executable(test test.cpp)
target_link_libraries(test subhook)

if(MSVC)
  set_property(TARGET test
               APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

add_test(NAME test COMMAND $<TARGET_FILE:test>)

file(READ test.out output)
string(STRIP "${output}" output)
set_tests_properties(test PROPERTIES PASS_REGULAR_EXPRESSION "${output}")

if(WIN32 AND NOT SUBHOOK_STATIC)
  set_tests_properties(test PROPERTIES
                       ENVIRONMENT PATH=$<TARGET_FILE_DIR:subhook>)
endif()
