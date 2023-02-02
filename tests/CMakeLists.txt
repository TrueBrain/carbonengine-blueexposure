cmake_minimum_required(VERSION 3.16)

project(BlueExposureTest VERSION 1.0.0)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

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

target_precompile_headers(BlueExposureTest PRIVATE StdAfx.h)

if(APPLE)
    target_link_libraries(BlueExposureTest PRIVATE "-framework Foundation")
endif()

target_compile_definitions(BlueExposureTest PUBLIC BLUEBUILD BLUE_WITH_PYTHON)

target_link_libraries(BlueExposureTest
    PRIVATE BlueExposure
    PUBLIC Python
)

target_include_directories(BlueExposureTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
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
    install(
        TARGETS BlueExposureTest
        EXPORT BlueExposureTestTargets
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        LIBRARY DESTINATION test/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )
endif()
if(WIN32)
    install(
        TARGETS BlueExposureTest
        EXPORT BlueExposureTestTargets
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        RUNTIME DESTINATION test/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )
endif()