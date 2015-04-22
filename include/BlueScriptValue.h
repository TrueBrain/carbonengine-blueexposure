////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		June 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueScriptValue_h
#define BlueScriptValue_h

#if BLUE_WITH_LUA

extern "C" {
#include "lua.h"
#include "lauxlib.h"
};

struct BLUEIMPORT BlueScriptValue
{
	BlueScriptValue() {}
	BlueScriptValue( lua_State* _ls, int _ix ) : ls( _ls ), ix( _ix ) {}

	lua_State* ls;
	int ix;
};

typedef lua_State* BlueScriptArguments;

#define BLUE_SCRIPT_ERROR BlueScriptValue( nullptr, -1 )
#define BLUE_SCRIPT_NO_RETURN_VALUE return BlueScriptValue( nullptr, 0 )
typedef int (*BluePropertyGetterFunction)( lua_State* ls, struct IRoot* obj );
typedef int (*BluePropertySetterFunction)( lua_State* ls, struct IRoot* obj );
typedef lua_CFunction BlueScriptCFunction;
struct BlueMethodDefinition {
	const char* ml_name;		/* The name of the built-in function/method */
	BlueScriptCFunction ml_meth;	/* The C function that implements it */
	const char* ml_doc;			/* The __doc__ attribute, or NULL */
};

#elif BLUE_WITH_PYTHON

#include <python.h>

typedef PyObject* BlueScriptValue;
typedef PyObject* BlueScriptArguments;
#define BLUE_SCRIPT_ERROR nullptr
#define BLUE_SCRIPT_NO_RETURN_VALUE Py_RETURN_NONE
typedef PyObject* (*BluePropertyGetterFunction)(PyObject*);
typedef PyObject* (*BluePropertySetterFunction)(PyObject*, PyObject*);
typedef PyMethodDef BlueMethodDefinition;
typedef PyCFunction BlueScriptCFunction;

#endif

BLUEIMPORT struct IRoot* BlueUnwrapObjectFromScriptValue( BlueScriptValue sv );

#endif // BlueScriptValue_h