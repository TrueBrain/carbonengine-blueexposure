cmake_minimum_required(VERSION 3.16)

project(BlueExposure VERSION 1.0.0)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

add_library(BlueExposure STATIC
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
    include/BlueStdResult.cpp
    BlueStructureList.cpp
    BlueTypes.cpp
    BlueUtil.cpp
    BlueVariable.cpp
    BlueVariable.h
    Copier.cpp
    Copier.h
    Copier_Blue.cpp
    Find.cpp
    Find.h
    InterfaceDefinitions.cpp
    PythonKlass.cpp
    TypeInfo.cpp
    TypeInfo.h
)
target_sources(BlueExposure PRIVATE
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
    include/BlueStdResult.cpp
    include/BlueStdResult.h
    include/BlueStringTable.h
    include/BlueStructureList.h
    include/BlueTypes.h
    include/BlueTypeTraits.h
    include/BlueUtil.h
    include/BlueVectorTypes.h
    include/BlueWeakRef.h
    include/BlueWrapReturnValue.h
    include/BlueWrapReturnValuePython.h
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


target_compile_definitions(BlueExposure
    PUBLIC
        BLUE_WITH_PYTHON
    PRIVATE
        BLUEBUILD
)

target_link_libraries(BlueExposure PUBLIC CcpCore Python)

target_include_directories(BlueExposure
    PUBLIC
        $<INSTALL_INTERFACE:include/eve/BlueExposure>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
