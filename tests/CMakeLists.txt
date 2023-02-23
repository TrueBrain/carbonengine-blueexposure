add_library(BlueExposureTest SHARED
    BlueExposureTest.cpp
    TestAttributes.cpp
    TestAttributes_Blue.cpp
    TestCallbacks.cpp
    TestCallbacks_Blue.cpp
    TestFunctions.cpp
    TestMethods.cpp
    TestMethods_Blue.cpp
    TestMultipleInterfaces.cpp
    TestMultipleInterfaces_Blue.cpp
    TestOptionalArgs.cpp
    TestOptionalArgs_Blue.cpp
    TestProperties.cpp
    TestProperties_Blue.cpp
    TestReferences.cpp
    TestReferences_Blue.cpp
)
target_sources(BlueExposureTest PRIVATE
    BoolBeResult.h
    StdAfx.h
    TestAttributes.h
    TestCallbacks.h
    TestMethods.h
    TestMultipleInterfaces.h
    TestOptionalArgs.h
    TestProperties.h
    TestReferences.h
)

if(APPLE)
    target_link_libraries(BlueExposureTest PRIVATE "-framework Foundation")
endif()

target_compile_definitions(BlueExposureTest PUBLIC BLUEBUILD BLUE_WITH_PYTHON)

target_link_libraries(BlueExposureTest
    PRIVATE BlueExposure
    PUBLIC Python
)

set_target_properties(BlueExposureTest PROPERTIES
    FOLDER "Tests"
    OUTPUT_NAME "_blueexposuretest"
)

set(TEST_DESTINATION test/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/)

if(APPLE)
    # on macOS we like to still use the .so naming convention, without a prefix
    set_target_properties(BlueExposureTest
        PROPERTIES
            PREFIX ""
            SUFFIX ".so"
    )
endif()
if(WIN32)
    set_target_properties(BlueExposureTest PROPERTIES SUFFIX ".pyd")
endif()

set(PYTHON_EXECUTABLE "${BRANCH_ROOT_DIR}/vendor/python/3.8.1+ccp-stackless/bin/Windows/x64/v141/python.exe")
add_test(NAME python-tests
    COMMAND ${PYTHON_EXECUTABLE} -m unittest discover -v
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
)
set_tests_properties(python-tests PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
