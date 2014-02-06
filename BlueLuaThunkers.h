////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		August 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueLuaThunkers_h
#define BlueLuaThunkers_h

#if BLUE_WITH_LUA

#include "include/BlueTypes.h"
#include "include/IList.h"

class IList_Thunk : public IList
{
public:
	static const Be::IID& IID();
	static const BlueMethodDefinition* Defs();

	const Be::Clsid* Clsid();

	static int append( lua_State* ls );
	static int get( lua_State* ls );
	static int size( lua_State* ls );
};

#endif

#endif // BlueLuaThunkers_h