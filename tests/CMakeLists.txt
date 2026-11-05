# Test that the library can be loaded
add_executable(test_loading test_loading.c)
target_link_libraries(test_loading dl)
add_test(NAME test_loading COMMAND test_loading)

# Smoke test for openat() support on a .fex file
add_executable(test_openat test_openat.c)
target_link_libraries(test_openat dl)
add_test(NAME test_openat COMMAND test_openat)
set_tests_properties(test_openat PROPERTIES
    ENVIRONMENT "LD_PRELOAD=${CMAKE_BINARY_DIR}/src/libfex.so"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# ASCII/file read smoke test that exercises fopen/fread against a .fex file
add_executable(test_ascii_reader test_ascii_reader.c)
target_link_libraries(test_ascii_reader dl)
add_test(NAME test_ascii_reader COMMAND test_ascii_reader ${CMAKE_SOURCE_DIR}/tests/fixtures/simple_test.fex)
set_tests_properties(test_ascii_reader PROPERTIES
    ENVIRONMENT "LD_PRELOAD=${CMAKE_BINARY_DIR}/src/libfex.so"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
