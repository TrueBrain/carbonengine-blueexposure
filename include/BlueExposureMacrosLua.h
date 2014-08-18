////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

BLUEIMPORT int BlueCreateInstanceFromLua( lua_State* ls, const Be::Clsid& clsid );

#define EXPOSE_TO_BLUE() \
	static const Be::ClassInfo* ClassType_() { return ExposeToBlue(); } \
	static const Be::Clsid* Clsid() { return ClassType_()->mClassId; } \
	static const Be::IID* IID() { return ClassType_()->mIID; } \
	\
	IRoot* GetRawRoot() const { return reinterpret_cast<IRoot*>((uintptr_t)this + ClassType_()->mInterfaceTable->mOffset); } \
	\
	const Be::ClassInfo* ClassType() const { return ClassType_(); } \
	\
	static int LuaNew( lua_State* ls ) \
	{ \
		return BlueCreateInstanceFromLua( ls, *Clsid() ); \
	} \
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
	_classinfo.mMetatableName = CCP_STRDUP( "metatableName", metatableName.c_str() ); \
	_classinfo.mCreateWrapper = LuaNew; \
	\
	s_classInfo = &_classinfo; \
	return &_classinfo;


#define MAP_METHOD( nameString, pyFunc, docString )

#define MAP_METHOD_AND_WRAP( name, functionName, docString ) \
{\
	BlueMethodDefinition d = { name, &BlueMethodHelper<_Class, decltype(&_Class::functionName), &_Class::functionName>, docString };\
	s_methods.push_back( d );\
}

#define MAP_METHOD_AND_WRAP_OPTIONAL_ARGS( name, functionName, numOptional, docString ) \
{\
	BlueMethodDefinition d = { name, &BlueMethodHelperWithOptionalArgs<decltype(&_Class::functionName), &_Class::functionName, numOptional>, docString };\
	s_methods.push_back( d );\
}

#define MAP_METHOD_AS_METHOD( nameString, pyFunc, docString )
#define MAP_PROPERTY(nameString, getterFunction, setterFunction, docString) \
{\
	static Be::VarEntry d = { \
		nameString, \
		BlueGetPropertyType<_Class, decltype(&_Class::getterFunction), &_Class::getterFunction>(), \
		0, \
		0, \
		GetBlueReturnTypeIID(&_Class::getterFunction), \
		(docString), \
		Be::READWRITE, \
		nullptr, \
		BluePropertyGetter<_Class, decltype(&_Class::getterFunction), &_Class::getterFunction>, \
		BluePropertySetter<_Class, decltype(&_Class::setterFunction), &_Class::setterFunction> }; \
 	s_attributes.push_back( d );\
}

#define MAP_PROPERTY_READONLY(nameString, getterFunction, docString) \
{\
	static Be::VarEntry d = { \
		nameString, \
		BlueGetPropertyType<_Class, decltype(&_Class::getterFunction), &_Class::getterFunction>(), \
		0, \
		0, \
		GetBlueReturnTypeIID(&_Class::getterFunction), \
		(docString), \
		Be::READ, \
		nullptr, \
		BluePropertyGetter<_Class, decltype(&_Class::getterFunction), &_Class::getterFunction>, \
		nullptr }; \
 	s_attributes.push_back( d );\
}


#if defined(_MSC_VER) || defined(__ORBIS__)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

// Use this macro to implement a standard module init function
#define BLUE_STANDARD_MODULE_INIT( moduleName ) \
	const char* g_moduleName = #moduleName; \
	extern "C" int DLLEXPORT luaopen_ ## moduleName( lua_State* ls ) \
	{ \
		BeClasses->RegisterClasses( BlueRegistration::GetClassRegs() ); \
		BlueRegisterClasses( ls, g_moduleName, BlueRegistration::GetClassRegs() ); \
		BlueRegisterFunctions( ls, g_moduleName, BlueRegistration::GetFuncRegs() ); \
		return 1; \
	}

#define BLUE_DECLARE_EXCEPTION_EX( name, ... ) __VA_ARGS__ const char* CCP_CONCATENATE( BlueGetException, name )();
#define BLUE_DECLARE_EXCEPTION( name ) BLUE_DECLARE_EXCEPTION_EX( name )

#define BLUE_GET_EXCEPTION( name ) ( CCP_CONCATENATE( BlueGetException, name )() )

#define BLUE_DEFINE_EXCEPTION( name, parent ) \
	const char* CCP_CONCATENATE( BlueGetException, name )() \
	{ \
	return #name; \
	} \
	BLUE_REGISTER_EXCEPTION( name, CCP_CONCATENATE( BlueGetException, name ) )
