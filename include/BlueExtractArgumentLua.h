#include "BlueExposureHelpersLua.h"

static const char* const argumentTypeMismatchString = "Could not match argument %d to expected type: %s.";

BLUEIMPORT IRoot* BlueGetObjectFromStack( lua_State* ls, int idx );

// Template for IRoot* types
template< typename T >
bool BlueExtractArgumentImpl( BlueScriptValue argument, T& result, unsigned int argID,  std::true_type isBlueType )
{
	IRoot* obj = BlueGetObjectFromStack( argument.ls, argument.ix );
	typedef typename std::remove_const< typename std::remove_pointer< T >::type >::type TWithoutTheStar;
	bool isOK = obj->QueryInterface( BlueInterfaceIID<TWithoutTheStar>(), (void**)&result );
	if( isOK )
	{
		obj->Unlock();
		return true;
	}
	else
	{
		return false;
	}
}

template< typename T>
bool BlueExtractArgumentImpl( BlueScriptValue argument, std::vector<T>& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

template< typename KeyType, typename ValueType>
bool BlueExtractArgumentImpl( BlueScriptValue argument, std::map<KeyType, ValueType>& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

