////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueExposureHelpersLua_h
#define BlueExposureHelpersLua_h
#include "BlueFunctionTraits.h"
#include "BlueTypeTraits.h"

BLUEIMPORT IRoot* BlueGetObjectFromStack( lua_State* ls, int idx );

template< typename FnType, FnType fn >
int BlueFunctionHelper( lua_State* ls )
{
	typedef typename function_traits<FnType>::return_type return_type;
	BlueScriptValue result = BlueCallFunctionAndWrapReturn( fn, ls, typename std::is_void<return_type>::type() );
	return result.ix;
}

template< typename class_type, typename memFnType, memFnType memFn >
int BlueMethodHelper( lua_State* ls )
{
	typedef typename function_traits<memFnType>::return_type return_type;

	// Extract the class pointer
	class_type* pThis = dynamic_cast<class_type*>( BlueGetObjectFromStack( ls, 1 ) );
	lua_remove( ls, 1 );

	BlueMemberFunctionAsCallable<memFnType> callable( pThis, memFn );

	// We always need to deal with void carefully.
	// so start by overloading specifically for that special case
	BlueScriptValue result = BlueCallFunctionAndWrapReturn(
		callable,
		ls,
		typename std::is_void<return_type>::type() );

	return result.ix;
}

template< typename memFnType, memFnType memFn, int numOptional >
int BlueMethodHelperWithOptionalArgs( lua_State* ls )
{
	typedef typename function_traits<memFnType>::return_type return_type;
	typedef typename function_traits<memFnType>::class_type class_type;

	// Extract the class pointer
	class_type* pThis = dynamic_cast<class_type*>( BlueGetObjectFromStack( ls, 1 ) );
	lua_remove( ls, 1 );

	BlueFunctionWithOptionalArguments< BlueMemberFunctionAsCallable< memFnType >, numOptional > callable( pThis, memFn );

	// We always need to deal with void carefully.
	// so start by overloading specifically for that special case
	BlueScriptValue result = BlueCallFunctionAndWrapReturn(
		callable,
		ls,
		typename std::is_void<return_type>::type() );
	
	return result.ix;
}

// Wrapper for a property setter function
template< typename class_type, typename setterFnType, setterFnType setterFn >
int BluePropertySetter( lua_State* ls, IRoot* obj )
{
	class_type* pThis = dynamic_cast<class_type*>( obj );
	lua_remove( ls, 1 );

	BlueScriptValue sv( ls, 2 );
	if( !BlueCallPropertySetter( setterFn, pThis, sv, typename is_be_result<typename function_traits<setterFnType>::return_type>::type() ) )
	{
		luaL_error( ls, "error" ); // TODO
	}
	return 0;
}

// Wrapper for a property getter function
template< typename class_type, typename getterFnType, getterFnType getterFn >
int BluePropertyGetter( lua_State* ls, IRoot* obj )
{
	class_type* pThis;
	bool isOK = obj->QueryInterface( BlueInterfaceIID<class_type>(), (void**)&pThis );
	CCP_ASSERT( isOK );

	BlueScriptValue sv = BlueCallPropertyGetter( getterFn, pThis, ls, typename is_be_result<typename function_traits<getterFnType>::return_type>::type() );
	
	// Remove the reference added the QueryInterface call above
	obj->Unlock();

	return sv.ix;
}


#endif // BlueExposureHelpersLua_h
