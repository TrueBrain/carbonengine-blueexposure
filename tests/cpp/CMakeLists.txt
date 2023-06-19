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

if(WIN32)
    add_custom_command(TARGET BlueExposureCppUnitTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:BlueExposureCppUnitTests> $<TARGET_FILE_DIR:BlueExposureCppUnitTests>
        COMMAND_EXPAND_LISTS
    )
endif()

gtest_discover_tests(BlueExposureCppUnitTests)
