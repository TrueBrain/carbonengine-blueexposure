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
    Python3::Python
)

if (CMAKE_IMPORT_LIBRARY_SUFFIX)
    add_custom_command(TARGET BlueExposureCppUnitTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:BlueExposureCppUnitTests> $<TARGET_FILE_DIR:BlueExposureCppUnitTests>
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
endif ()

# Test discovery needs to be delayed until we are about to run tests as otherwise the above post-build step won't have
# run. Additionally, we set the working directory for the tests to match the interpreter.
# That way we can avoid copying them around.
gtest_discover_tests(
    BlueExposureCppUnitTests
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY $<TARGET_FILE_DIR:Python3::Interpreter>
)
