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
    get_target_property(Python_DLL_PATH Python IMPORTED_LOCATION)
    cmake_path(GET Python_DLL_PATH PARENT_PATH Python_DLL_PATH)

    file(GLOB PYTHON_RUNTIME_FILES "${Python_DLL_PATH}/*.pyd")
    list(APPEND PYTHON_RUNTIME_FILES "${Python_DLL_PATH}/Python38.zip")
    add_custom_command(TARGET BlueExposureCppUnitTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:BlueExposureCppUnitTests> $<TARGET_FILE_DIR:BlueExposureCppUnitTests>
        COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_RUNTIME_FILES} $<TARGET_FILE_DIR:BlueExposureCppUnitTests>
        COMMAND_EXPAND_LISTS
    )
endif()

# By default test discovery happens as a post-build step. However, at that point
# we may not have the required Python environment available yet. So we delay
# test discovery to the pre-test phase.
gtest_discover_tests(BlueExposureCppUnitTests)
