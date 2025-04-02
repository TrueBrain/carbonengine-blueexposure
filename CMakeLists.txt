cmake_minimum_required(VERSION 3.21)

project(BlueExposure VERSION 1.0.0)

include(cmake/CcpGlobalSettings.cmake)
include(cmake/CcpTargetConfigurations.cmake)

find_package(carbon-core CONFIG REQUIRED)
find_package(Python3 COMPONENTS Development REQUIRED)

ccp_add_library(BlueExposure STATIC
    BlueClasses.cpp
    BlueClasses.h
    BlueClasses_Blue.cpp
    BlueExposure.cpp
    BlueExtractArgument.cpp
    BlueListUtil.cpp
    BlueMemberIterator.cpp
    BluePythonThunkers.cpp
    BluePythonThunkers.h
    BluePyWrap.cpp
    BluePyWrap.h
    BlueRegistration.cpp
    BlueRegistrationPython.cpp
    BlueScriptCallback.cpp
    BlueSharedString.cpp
    BlueStdResult.cpp
    BlueStructureList.cpp
    BlueTypes.cpp
    BlueUtil.cpp
    BlueVariable.cpp
    Copier.cpp
    Copier_Blue.cpp
    Find.cpp
    InterfaceDefinitions.cpp
    PythonKlass.cpp
    TypeInfo.cpp
    TypeInfo.h
)

set(PUBLIC_HEADER_FILES
    include/BlueCallFunction.h
    include/BlueCallFunctionHelpers.h
    include/BlueCallFunctionWithBeResult.h
    include/BlueClass.h
    include/BlueDict.h
    include/BlueExposure.h
    include/BlueExposureHelpers.h
    include/BlueExposureHelpersPython.h
    include/BlueExposureMacros.h
    include/BlueExposureMacrosDeprecated.h
    include/BlueExposureMacrosPython.h
    include/BlueExposureTypeSignature.h
    include/BlueExtractArgument.h
    include/BlueExtractArgumentPython.h
    include/BlueFunctionTraits.h
    include/BlueListUtil.h
    include/BlueMemberFunctionAsCallable.h
    include/BlueMemberFunctionTraits.h
    include/BlueMemberIterator.h
    include/BluePythonObject.h
    include/BlueRegistration.h
    include/BlueScriptCallback.h
    include/BlueScriptValue.h
    include/BlueSharedString.h
    include/BlueSmartPtr.h
    include/BlueSmartPy.h
    include/BlueStdResult.h
    include/BlueStringTable.h
    include/BlueStructureList.h
    include/BlueTypes.h
    include/BlueTypeTraits.h
    include/BlueUtil.h
    include/BlueVariable.h
    include/BlueVectorTypes.h
    include/BlueWeakRef.h
    include/BlueWrapReturnValue.h
    include/BlueWrapReturnValuePython.h
    include/Copier.h
    include/Find.h
    include/IBlueClasses.h
    include/IBlueDict.h
    include/IBlueRtti.h
    include/IBlueStructureList.h
    include/ICopier.h
    include/ICustomPersist.h
    include/IInitialize.h
    include/IList.h
    include/INotify.h
    include/IPythonMethods.h
    include/IPythonNumeric.h
    include/PythonKlass.h
    include/StringBeResult.h
)

target_sources(BlueExposure PRIVATE ${PUBLIC_HEADER_FILES})

target_compile_definitions(BlueExposure
    PUBLIC
        BLUE_WITH_PYTHON
    PRIVATE
        BLUEBUILD
)

target_link_libraries(BlueExposure PUBLIC CcpCore Python3::Python)

target_include_directories(BlueExposure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (WIN32)
        find_program(BUILT_PYTHON_EXECUTABLE python REQUIRED NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    elseif(APPLE)
        find_program(BUILT_PYTHON_EXECUTABLE python3 REQUIRED NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    endif()

    get_filename_component(PYTHON_TOOL_ROOT_DIR ${BUILT_PYTHON_EXECUTABLE} DIRECTORY)

    option(BUILD_TESTING "Build tests. Enabled by default." ON)
    include(CTest)
    if(BUILD_TESTING)
	    add_subdirectory(tests)
    endif()
endif()

# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
    install(
        TARGETS BlueExposure
        EXPORT BlueExposureTargets
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )
    # Install rule for available public headers
    install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)

        install(
                EXPORT BlueExposureTargets
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-blueexposure
                FILE carbon-blueexposure.cmake
        )
        install(
                FILES carbon-blueexposureConfig.cmake
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-blueexposure
        )
    ENDIF()
endif()
