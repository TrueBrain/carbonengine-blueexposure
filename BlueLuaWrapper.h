////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueLuaWrapper_h
#define BlueLuaWrapper_h

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "include/BlueTypes.h"
struct IRoot;

class BlueLuaWrapper
{
public:
	BlueLuaWrapper( IRoot* obj ) : m_object( obj ) {}

	IRoot* GetObject() { return m_object; }

	static int CreateWrappedObject( lua_State* ls, const Be::Clsid& clsid );
	static int WrapObject( lua_State* ls, IRoot* obj );

	static int Lua__index( lua_State* ls );
	static int Lua__newindex( lua_State* ls );

private:
	IRootPtr m_object;
};

#endif // BlueLuaWrapper_h