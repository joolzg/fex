# Build the LD_PRELOAD shared library
add_library(fex SHARED fex_preload.c)

# Link with required libraries
target_link_libraries(fex dl pthread)

# Set library properties
set_target_properties(fex PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Optional: Build a test executable that uses the library
add_executable(test_app test_app.c)
target_link_libraries(test_app dl)