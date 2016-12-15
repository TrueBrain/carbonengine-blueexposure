////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		June 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueCallFunctionHelpers_h
#define BlueCallFunctionHelpers_h

#if BLUE_WITH_PYTHON

static bool CheckArgumentLength( BlueScriptArguments args, unsigned int length, unsigned int numOptional )
{
	// We allow this to deal with how properties are called
	if(args == NULL && length == 0)
	{
		return true;
	}

	int minNumArgs = length - numOptional;

	if( args == NULL || (PyTuple_Size(args) < minNumArgs) )
	{
		if( length == 1 )
		{
			PyErr_SetString( PyExc_TypeError, "Function expects 1 argument." );
		}
		else
		{
			PyErr_Format( PyExc_TypeError, "Function expects %i arguments.", length );
		}

		return false;
	}

	if( args && (PyTuple_Size(args) > (int)length ) )
	{
		if( length == 1 )
		{
			PyErr_Format( PyExc_TypeError, "Function expects 1 argument (%d optional).", numOptional );
		}
		else
		{
			PyErr_Format( PyExc_TypeError, "Function expects %i arguments (%d optional).", minNumArgs, numOptional );
		}

		return false;
	}

	return true;
}


template< typename FunctionType, typename ArgType >
bool BlueExtractArgumentOrNullValue( FunctionType func, BlueScriptArguments args, unsigned int ix, ArgType& arg, std::true_type allowOptionalArg )
{
	if( PyTuple_Size(args) < (int)ix )
	{
		BlueGetNullValue( arg );
		return true;
	}

	if( !BlueExtractArgument( PyTuple_GetItem( args, ix-1 ), arg, ix-1 ) )
	{
		return false; /* Error set by BlueExtractArgument */ \
	}

	return true;
}

template< typename FunctionType, typename ArgType >
bool BlueExtractArgumentOrNullValue( FunctionType func, BlueScriptArguments args, unsigned int ix, ArgType& arg, std::false_type allowOptionalArg )
{
	if( !BlueExtractArgument( PyTuple_GetItem( args, ix-1 ), arg, ix-1 ) )
	{
		return false; /* Error set by BlueExtractArgument */ \
	}

	return true;
}

#elif BLUE_WITH_LUA

static inline bool CheckArgumentLength( BlueScriptArguments args, unsigned int length, unsigned int numOptional )
{
	int minNumArgs = length - numOptional;
	int argsOnStack = lua_gettop( args );

	if( argsOnStack < minNumArgs )
	{
		if( length == 1 )
		{
			CCP_LOGERR( "Function expects 1 argument." );
		}
		else
		{
			CCP_LOGERR( "Function expects %i arguments.", length );
		}

		return false;
	}

	//if( argsOnStack > (int)length )
	//{
	//	if( length == 1 )
	//	{
	//		CCP_LOGERR( "Function expects 1 argument (%d optional).", numOptional );
	//	}
	//	else
	//	{
	//		CCP_LOGERR( "Function expects %i arguments (%d optional).", minNumArgs, numOptional );
	//	}

	//	return false;
	//}

	return true;
}


template< typename FunctionType, typename ArgType >
bool BlueExtractArgumentOrNullValue( FunctionType func, BlueScriptArguments args, unsigned int ix, ArgType& arg, std::true_type allowOptionalArg )
{
	if( lua_gettop( args ) < (int)ix )
	{
		BlueGetNullValue( arg );
		return true;
	}

	BlueScriptValue sv( args, ix  );
	if( !BlueExtractArgument( sv, arg, ix-1 ) )
	{
		return false; /* Error set by BlueExtractArgument */ \
	}

	return true;
}

template< typename FunctionType, typename ArgType >
bool BlueExtractArgumentOrNullValue( FunctionType func, BlueScriptArguments args, unsigned int ix, ArgType& arg, std::false_type allowOptionalArg )
{
	BlueScriptValue sv( args, ix );
	if( !BlueExtractArgument( sv, arg, ix-1 ) )
	{
		return false; /* Error set by BlueExtractArgument */ \
	}

	return true;
}

#endif

#endif // BlueCallFunctionHelpers_h