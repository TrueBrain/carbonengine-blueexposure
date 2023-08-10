find_package(GTest REQUIRED)
add_executable(BlueExposureCppUnitTests
    blns.cpp
    main.cpp
    TestBlueSmartPy.cpp
)
target_precompile_headers(BlueExposureCppUnitTests
    PRIVATE
        <Python.h>
        <gtest/gtest.h>
)
target_link_libraries(
    BlueExposureCppUnitTests
    BlueExposure
    GTest::GTest
    Python
)

# Unfortunately the Python runtime is required for test discovery to work. It's not possible to override the working
# directory for discovery itself - the `WORKING_DIRECTORY` attribute there is for discovered tests. Therefore, we need
# to place it in the output folder.
get_target_property(LIBPYTHON Python IMPORTED_LOCATION)
add_custom_command(TARGET BlueExposureCppUnitTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:BlueExposureCppUnitTests> ${LIBPYTHON}
    COMMAND_EXPAND_LISTS
    VERBATIM
)

# Test discovery needs to be delayed until we are about to run tests as otherwise the above post-build step won't have
# run. Additionally, we set the working directory for the tests to be the folder in which all built-in Python C
# extensions are located. That way we can avoid copying them around.
cmake_path(GET LIBPYTHON ROOT_PATH PYTHON_BIN_FOLDER)
gtest_discover_tests(BlueExposureCppUnitTests
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY ${PYTHON_BIN_FOLDER}
)
