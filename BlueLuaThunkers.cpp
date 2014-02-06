////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		August 2013
// Copyright:	CCP 2013
//

#if BLUE_WITH_LUA

#include "BlueLuaThunkers.h"
#include "BlueLuaWrapper.h"
#include "include/BlueExposureHelpers.h"
#include "include/BlueRegistration.h"

const Be::IID& IList_Thunk::IID()
{
	static Be::IID iid( "IList" );
	return iid;
}

const Be::Clsid* IList_Thunk::Clsid()
{
	return ClassType()->mClassId;
}

const BlueMethodDefinition* IList_Thunk::Defs()
{
	static BlueMethodDefinition s_defs[] =
	{
		{ "append", append, "L.append(object) -- append object to end" },
		{ "get", get, "object = L.get(ix) -- get 0-indexed object" },
		{ "size", size, "n = L.size() -- get size of list" },
		{ 0 }
	};

	return s_defs;
}

int IList_Thunk::append( lua_State* ls )
{
	IRoot* obj = BlueGetObjectFromStack( ls, 1 );
	IRoot* val = BlueGetObjectFromStack( ls, 2 );

	IListPtr list( BlueCastPtr( obj ) );
	list->Append( val );

	return 0;
}

int IList_Thunk::get( lua_State* ls )
{
	IRoot* obj = BlueGetObjectFromStack( ls, 1 );
	
	unsigned int ix;
	BlueScriptValue sv( ls, 2 );
	if( BlueExtractArgument( sv, ix, 2 ) )
	{
		IListPtr list( BlueCastPtr( obj ) );
		IRoot* val = list->GetAt( ix );

		BlueWrapObjectForLua( ls, val );
		return 1;
	}
	else
	{
		luaL_error( ls, "Type error" );
		return 0;
	}
}

int IList_Thunk::size( lua_State* ls )
{
	IRoot* obj = BlueGetObjectFromStack( ls, 1 );

	IListPtr list( BlueCastPtr( obj ) );
	unsigned int size = list->GetSize();

	return BlueWrapReturnValue( ls, size ).ix;
}

#endif
