# Test that the library can be loaded
add_executable(test_loading test_loading.c)
target_link_libraries(test_loading dl)

# Add test
add_test(NAME test_loading COMMAND test_loading)