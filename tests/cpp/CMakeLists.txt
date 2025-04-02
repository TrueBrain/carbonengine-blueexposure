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

gtest_discover_tests(BlueExposureCppUnitTests WORKING_DIRECTORY $<TARGET_FILE_DIR:Python3::Python>)
