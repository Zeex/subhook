add_executable(test_prog "test.c")
target_link_libraries(test_prog "subhook")

add_test(NAME test COMMAND "$<TARGET_FILE:test_prog>")

file(READ "test.out" output)
string(STRIP "${output}" output)
set_tests_properties(test PROPERTIES PASS_REGULAR_EXPRESSION "${output}")

if(WIN32 AND NOT SUBHOOK_STATIC)
	set_tests_properties(test PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:subhook>")
endif()