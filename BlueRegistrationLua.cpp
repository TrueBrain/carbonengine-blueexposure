////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

#if BLUE_WITH_LUA

#include "include/BlueRegistration.h"

#include "BlueLuaWrapper.h"
#include "BlueLuaThunkers.h"

BLUE_REGISTER_THUNKER(IList_Thunk::Defs(), IList_Thunk::IID());

namespace
{
	// Ensures that a table exists as a global named 'moduleName'
	void CreateOrGetModule( lua_State* ls, const char* moduleName )
	{
		lua_getglobal( ls, moduleName );

		if( !lua_istable( ls, -1 ) )
		{
			lua_pop( ls, 1 ); // Pop the non-table.
			lua_newtable( ls );
			lua_pushvalue( ls, -1 );
			lua_setglobal( ls, moduleName);
		}
	}
}

extern BLUEIMPORT void BlueRegisterFunctions( lua_State* ls, const char* moduleName, const FuncRegsVector& funcRegs )
{
	CreateOrGetModule( ls, moduleName );

	for( auto it = funcRegs.begin(); it != funcRegs.end(); ++it )
	{
		lua_CFunction func = it->ml_meth;
		const char* name = it->ml_name;

		lua_pushcfunction( ls, func );
		lua_setfield( ls, -2, name );
	}
}

BLUEIMPORT void BlueRegisterClasses( lua_State* ls, const char* moduleName, const ClassRegsVector& classRegs )
{
	CreateOrGetModule( ls, moduleName );

	for( auto it = classRegs.begin(); it != classRegs.end(); ++it )
	{
		const char* name = it->mType->mClassId->GetName();

		std::string metatableName = it->mType->mMetatableName;

		if( !luaL_newmetatable( ls, metatableName.c_str() ) )
		{
			CCP_LOGWARN( "Couldn't create metatable named %s", metatableName.c_str() );
			continue;
		}

		// Create a second metatable that gets set as the metatable for the first one.
		// It has just one metamethod - the __call method. This allows creation of instances
		// with the following syntax:
		//   obj = module.ClassName()
		metatableName += "Factory";

		if( !luaL_newmetatable( ls, metatableName.c_str() ) )
		{
			CCP_LOGWARN( "Couldn't create metatable named %s", metatableName.c_str() );
			continue;
		}

		lua_pushcfunction( ls, it->mType->mCreateWrapper );
		lua_setfield( ls, -2, "__call" );

		lua_setmetatable( ls, -2 );

		// Set the __index metamethod, for reading attributes and methods
		lua_pushcfunction( ls, BlueLuaWrapper::Lua__index );
		lua_setfield( ls, -2, "__index" );

		// Set the __newindex metamethod, for writing attributes
		lua_pushcfunction( ls, BlueLuaWrapper::Lua__newindex );
		lua_setfield( ls, -2, "__newindex" );

		lua_pushstring( ls, name );
		lua_setfield( ls, -2, "bluetype" );

		lua_setfield( ls, -2, name );
	}
}

BLUEIMPORT void BlueRegisterObjectsToModule( lua_State* ls, const char* moduleName, ObjectRegsVector& objectRegs )
{
	CreateOrGetModule( ls, moduleName );

	for( auto it = objectRegs.begin(); it != objectRegs.end(); ++it )
	{
		if( *it->second )
		{
			BlueWrapObjectForLua( ls, *it->second );
			lua_setfield( ls, -2, it->first );
		}
		else
		{
			CCP_LOGERR( "Attempting to register '%s' to module but objects is a nullptr", it->first );
		}
	}
}

BLUEIMPORT void BlueRegisterInterfaceMethods( lua_State* ls, const ThunkerRegsVector& thunkerRegs )
{
	ThunkerRegsVector& globalRegs = BlueRegistration::GetGlobalThunkerRegs();
	for( ThunkerRegsVector::const_iterator it = thunkerRegs.begin(); it != thunkerRegs.end(); ++it )
	{
		globalRegs.push_back( *it );
	}
}

#endif
