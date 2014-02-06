extern BLUEIMPORT void BlueWrapObjectForLua( lua_State* ls, IRoot* obj );

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, BlueScriptValue val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, IRoot* val )
{
	BlueScriptValue returnValue( args, 1 );
	if( val )
	{
		BlueWrapObjectForLua( args, val );
	}
	else
	{
		lua_pushnil( args );
	}

	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, bool val )
{
	BlueScriptValue returnValue( args, 1 );
	if( val )
	{
		lua_pushboolean( returnValue.ls, 1 );
	}
	else
	{
		lua_pushboolean( returnValue.ls, 0 );
	}

	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, int val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, uint32_t val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, uint64_t val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, (lua_Integer)val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, int64_t val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, (lua_Integer)val );
	return returnValue;
}

#if defined(__APPLE__)
// Clang on the Mac treats size_t as something different from uint32_t/uint64_t, whereas
// other compilers complain about redefinition of this function if we keep it in.
inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, size_t val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, val );
	return returnValue;
}
#endif

#if defined(__APPLE__) || defined(__ANDROID__)
inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, long val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushinteger( returnValue.ls, val );
	return returnValue;
}
#endif

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, float val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushnumber( returnValue.ls, val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, double val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushnumber( returnValue.ls, val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Vector2& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Vector3& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Vector4& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Color& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Quaternion& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const Matrix& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::string& val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushstring( returnValue.ls, val.c_str() );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const char* val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushstring( returnValue.ls, val );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::wstring& val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushstring( returnValue.ls, CW2A( val.c_str() ) );
	return returnValue;
}

inline BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const wchar_t* val )
{
	BlueScriptValue returnValue( args, 1 );
	lua_pushstring( returnValue.ls, CW2A( val ) );
	return returnValue;
}

template< typename T >
BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::vector<T>& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

template< typename T >
BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::list<T>& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

template< typename KeyType, typename ValueType >
BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::map<KeyType, ValueType>& val )
{
	luaL_error( args, "Not implemented" );
	return BLUE_SCRIPT_ERROR;
}

template< typename T1, typename T2 >
BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const std::pair<T1, T2>& val )
{
    lua_createtable( args, 0, 2 );
    BlueWrapReturnValue( args, val.first );
    lua_setfield( args, -2, "first" );
    BlueWrapReturnValue( args, val.second );
    lua_setfield( args, -2, "second" );
	return BlueScriptValue( args, 1 );
}


template< typename valueType>
BlueScriptValue BlueWrapReturnValueHelper( BlueScriptArguments args, valueType val, std::true_type isPointerToBlue )
{
	BlueScriptValue returnValue( args, 1 );
	BlueWrapObjectForLua( args, reinterpret_cast<IRoot*>( val ) );
	return returnValue;
}
