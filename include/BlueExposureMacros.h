////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		January 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BLUE_EXPOSURE_H
#define BLUE_EXPOSURE_H

#include "BlueTypes.h"
#include "BlueTypeTraits.h"
#include "BlueMemberFunctionTraits.h"
#include "BlueCallFunction.h"
#include "BlueExposureTypeSignature.h"
#include "BlueRegistration.h"
#include "BlueExposureHelpers.h"
#include "BlueWeakRef.h"
#if BLUE_WITH_PYTHON
#include "BluePythonObject.h"
#endif
#include "BlueUtil.h"
#include <string>

// Define a blue class
#define BLUE_CLASS_IMPL( U ) \
	BLUE_DECLARE( U ); \
	template<> struct BlueClassTypeTraits< U >												\
	{																						\
		static const Be::Clsid&				Class()		{ return Get ## U ## Clsid();	}	\
	};

#define BLUE_CLASS( U ) BLUE_CLASS_IMPL(U ) class U

#define BLUE_BLUEIMPORT_CLASS( U ) BLUE_CLASS_IMPL(U ) class BLUEIMPORT U

#define BLUE_DEFINE_CLSID( _modulename, _classname )\
	const Be::Clsid& Get##_classname##Clsid() \
	{ \
		static Be::Clsid s_clsid( _modulename, #_classname ); \
		return s_clsid; \
	}

// Define a Blue class without registering it. Use this for non-standard registration
#define BLUE_DEFINE_NO_REGISTER( _classname ) \
	BLUE_DEFINE_CLSID( g_moduleName, _classname ); \
	BLUE_DEFINE_INTERFACE_IMPL( _classname );

// Define a Blue class and register it
#define BLUE_DEFINE( _classname ) \
	BLUE_DEFINE_NO_REGISTER( _classname ); \
	BLUE_REGISTER_CLASS( _classname )

// Define a Blue class that lives in a different module
#define BLUE_DEFINE_CLASS_FROM_MODULE( _module, _classname ) \
	BLUE_DEFINE_CLSID( _module, _classname ); \
	BLUE_DEFINE_INTERFACE_IMPL( _classname ); \
	BLUE_REGISTER_CLASS( _classname )

// Define an abstract Blue class
#define BLUE_DEFINE_ABSTRACT( _classname ) \
	BLUE_DEFINE_NO_REGISTER( _classname ); \
	BLUE_REGISTER_ABSTRACT_CLASS( _classname )

// Define a Blue class that is not exposed to Python
#define BLUE_DEFINE_NONEXPOSED( _classname ) \
	BLUE_DEFINE_NO_REGISTER( _classname ); \
	BLUE_REGISTER_CLASS_NO_PYTHON_CONSTRUCTOR( _classname )

#define BLUE_DEFINE_INTERFACE_IMPL( _interface )\
	const Be::IID& Get##_interface##IID()\
	{\
		static Be::IID s_iid( #_interface );\
		return s_iid;\
	}\
	template<> const Be::IID& BlueInterfaceIID< _interface >()\
	{\
		return Get ## _interface ## IID();\
	}

#define BLUE_DEFINE_INTERFACE( _interface )\
	BLUE_DECLARE_INTERFACE( _interface );\
	BLUE_DEFINE_INTERFACE_IMPL( _interface )

//similar to offsetof() macro.  static_cast<>(0) returns 0, so we must offset.
#define BLUE_INTERFACEOFFSET(_basename) \
	((ssize_t)static_cast<_basename*>(reinterpret_cast<_Class*>(uintptr_t(0x1000))) - 0x1000)

// Get the offset of a member of a class. Note that there are cases where this won't work in C++,
// but in the real world this just works.
// See:
// http://www.bookofbrilliantthings.com/book/eic/offsetof
// http://stackoverflow.com/questions/1129894/why-cant-you-use-offsetof-on-non-pod-strucutures-in-c
//
#define BLUE_MEMBEROFFSET( _class, _member ) \
	((ssize_t)&reinterpret_cast<const volatile char&>((((_class *)0x1000)->_member)) - 0x1000)

#define BLUE_MEMBERSIZE(_member) \
	sizeof(reinterpret_cast<_Class*>(0)->_member)


// Begin the ExposeToBlue method implementation with this macro
#define EXPOSURE_BEGIN( classname, docstring )\
	EXPOSURE_BEGIN_IMP( classname, docstring, Get##classname##Clsid() )

// End the ExposeToBlue method implementation with this macro if the class has no exposed Blue parents
#define EXPOSURE_END()\
	EXPOSURE_END_IMPL(NULL, 0)

// End the ExposeToBlue method implementation with this macro if the class has a Blue parent
#define EXPOSURE_CHAINTO(_parentclass)\
	EXPOSURE_END_IMPL(_parentclass::ClassType_(), BLUE_INTERFACEOFFSET(_parentclass))


// Used by Blue2.0 Exposure macros below
#define MAP_MEMBER(_name, _type, _member, _iid, _desc, _flags, _chooser)\
{_name, _type, BLUE_MEMBEROFFSET(_Class, _member), BLUE_MEMBERSIZE(_member), _iid,\
	_desc, _flags, _chooser, nullptr, nullptr}

// This code below is scary, since it would appear to dereference a NULL pointer
// It never actually does that though, because the templated function never bothers to look at it, only the deduced type

#define MAP_ATTRIBUTE( attributeName, member, docString, _flags )\
{\
	_Class* temp = NULL;\
	static Be::VarEntry d = MAP_MEMBER( attributeName, VarTypeForVariable<decltype(temp->member)>::type, member, GetBlueIID(temp->member), docString, _flags, NULL );\
	s_attributes.push_back( d );\
}

#define MAP_ATTRIBUTE_AS_CUSTOM_BINARY_BLOCK( attributeName )\
{\
	_Class* temp = NULL;\
	static Be::VarEntry d = {attributeName, Be::BINARYBLOCK, 0, 0, nullptr, "", Be::PERSISTONLY, nullptr, nullptr, nullptr};\
	s_attributes.push_back( d );\
}

#define MAP_ATTRIBUTE_WITH_CHOOSER( attributeName, member, docString, _flags, _chooser )\
{\
	_Class* temp = NULL;\
	static Be::VarEntry d = MAP_MEMBER( attributeName, VarTypeForVariable<decltype(temp->member)>::type, member, GetBlueIID(temp->member), docString, _flags, _chooser );\
	s_attributes.push_back( d );\
}

#define MAP_INTERFACE(_iface) \
{\
	Be::InterfaceEntry entry = {&Get##_iface##IID(), BLUE_INTERFACEOFFSET(_iface)}; \
	s_interfaces.push_back( entry ); \
}


//////////////////////////////////////////////////////////////////////////
// GetBlueReturnTypeID
// This function figures out the IID of the return type of a function,
// used to get sensible IIDs for Blue-exposed properties
//////////////////////////////////////////////////////////////////////////
template <typename fnType> static const Be::IID* GetBlueReturnTypeIID( const fnType& )
{
	typedef typename remove_const_and_reference< typename function_traits< fnType >::return_type >::type R;
	// Although it looks like we're dereferencing a NULL pointer, we really aren't - this is done
	// so the compiler can select the correct templated version of GetBlueIID()
	R* pR = NULL;
	return GetBlueIID( *pR );
}


#if BLUE_WITH_PYTHON
#include "BlueExposureMacrosPython.h"
#elif BLUE_WITH_LUA
#include "BlueExposureMacrosLua.h"
#else
#define EXPOSE_TO_BLUE() \
	static const Be::ClassInfo* ClassType_() { return ExposeToBlue(); } \
	static const Be::Clsid* Clsid() { return ClassType_()->mClassId; } \
	static const Be::IID* IID() { return ClassType_()->mIID; } \
	\
	IRoot* GetRawRoot() const { return reinterpret_cast<IRoot*>((uintptr_t)this + ClassType_()->mInterfaceTable->mOffset); } \
	\
	const Be::ClassInfo* ClassType() const { return ClassType_(); } \
	\
	static const Be::ClassInfo* ExposeToBlue()

#define EXPOSURE_BEGIN_IMP(_classname, _doc, _classid)\
	static Be::ClassInfo* s_classInfo = nullptr; \
	if( s_classInfo ) { return s_classInfo; } \
	typedef _classname _Class;\
	std::string metatableName = _classid.GetModule(); \
	metatableName += "."; \
	metatableName += _classid.GetName(); \
	\
	const char* const _tmpdoc = _doc;\
	const Be::Clsid& _tmpclsid = _classid;\
	static Be::IID s_iid(#_classname); \
	static Be::IID s_iroot("IRoot"); \
	\
	static std::vector<BlueMethodDefinition> s_methods;\
	static std::vector<Be::VarEntry> s_attributes; \
	static std::vector<Be::InterfaceEntry> s_interfaces; \
	Be::InterfaceEntry rootEntry = {&s_iroot, BLUE_INTERFACEOFFSET(_Class)}; \
	s_interfaces.push_back( rootEntry ); \
	Be::InterfaceEntry myEntry = {&s_iid, BLUE_INTERFACEOFFSET(_Class)}; \
	s_interfaces.push_back( myEntry );

#define EXPOSURE_END_IMPL(_parentclasstype, _parentoffs)\
	BlueMethodDefinition methodsEndItem = { 0 };\
	s_methods.push_back( methodsEndItem ); \
	Be::VarEntry attributesEndItem = { 0 }; \
	s_attributes.push_back( attributesEndItem ); \
	Be::InterfaceEntry interfacesEnd = { 0 }; \
	s_interfaces.push_back( interfacesEnd ); \
	\
	static Be::ClassInfo _classinfo; \
	_classinfo.mClassId = &_tmpclsid;  \
	_classinfo.mIID = &s_iid; \
	_classinfo.mDescription = _tmpdoc; \
	_classinfo.mInterfaceTable = &s_interfaces[0]; \
	_classinfo.mMemberTable = &s_attributes[0]; \
	_classinfo.mPyMethodTable = &s_methods[0]; \
	_classinfo.mParentClassInfo = _parentclasstype; \
	_classinfo.mOffsetToParent = _parentoffs; \
	_classinfo.mRtti = nullptr; \
	_classinfo.mFunctionSignatures = nullptr; \
	\
	s_classInfo = &_classinfo; \
	return &_classinfo;

#define MAP_METHOD( nameString, pyFunc, docString )
#define MAP_METHOD_AND_WRAP( name, functionName, docString )
#define MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( name, functionName, numOptional, docString )
#define MAP_METHOD_AS_METHOD( nameString, pyFunc, docString )
#define MAP_PROPERTY(nameString, getterFunction, setterFunction, docString)
#define MAP_PROPERTY_READONLY(nameString, getterFunction, docString)
#define BLUE_STANDARD_MODULE_INIT( moduleName )
#define BLUE_DECLARE_EXCEPTION_EX( name, ... ) __VA_ARGS__ const char* CCP_CONCATENATE( BlueGetException, name )();
#define BLUE_DECLARE_EXCEPTION( name ) BLUE_DECLARE_EXCEPTION_EX( name )
#define BLUE_GET_EXCEPTION( name ) ( CCP_CONCATENATE( BlueGetException, name )() )
#define BLUE_DEFINE_EXCEPTION( name, parent ) \
	const char* CCP_CONCATENATE( BlueGetException, name )() \
	{ \
	return #name; \
	} \
	BLUE_REGISTER_EXCEPTION( name, CCP_CONCATENATE( BlueGetException, name ) )

#endif

#endif
