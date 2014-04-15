////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

#if BLUE_WITH_LUA

#include "BlueLuaWrapper.h"
#include "include/IBlueRtti.h"
#include "include/IBlueClasses.h"
#include "include/BlueExtractArgument.h"
#include "include/BlueSharedString.h"
#include "include/BlueScriptCallback.h"
#include "include/INotify.h"

BLUEIMPORT int BlueCreateInstanceFromLua( lua_State* ls, const Be::Clsid& clsid )
{
	return BlueLuaWrapper::CreateWrappedObject( ls, clsid );
}

BLUEIMPORT IRoot* BlueGetObjectFromStack( lua_State* ls, int idx )
{
	void* ud = lua_touserdata( ls, idx );
	if( !ud )
	{
		luaL_error( ls, "Incorrect type" );
		return nullptr;
	}

	if( !luaL_getmetafield( ls, idx, "bluetype" ) )
	{
		luaL_error( ls, "Incorrect type" );
		return nullptr;
	}

	// Remove the 'bluetype' value - we just needed to check it's presence.
	lua_pop( ls, 1 );

	BlueLuaWrapper* wrapper = reinterpret_cast<BlueLuaWrapper*>( ud );
	return wrapper->GetObject();
}

int BlueConvertValueToLua( lua_State* ls, const Be::VarEntry* entry, Be::Var* value )
{
	Be::VARTYPE type = entry->mType;
	switch( type )
	{
	case Be::LONG:
		lua_pushinteger( ls, value->mLong );
		return 1;

	case Be::BOOL:
		lua_pushboolean( ls, value->mBool );
		return 1;

	case Be::FLOAT:
		lua_pushnumber( ls, value->mFloat );
		return 1;

	case Be::DOUBLE:
		lua_pushnumber( ls, value->mDouble );
		return 1;

	case Be::CSTRING:
		lua_pushstring( ls, value->mCharPtr );
		return 1;

	case Be::STDSTRING:
		{
			const std::string& s = *reinterpret_cast<const std::string*>(value);
			lua_pushstring( ls, s.c_str() );
			return 1;
		}
	case Be::SCRIPTCALLBACK:
		{
			const BlueScriptCallback& s = *reinterpret_cast<const BlueScriptCallback*>( value );
			BlueWrapReturnValueImpl( ls, s );
			return 1;
		}
	case Be::SHAREDSTRING:
		lua_pushstring( ls, reinterpret_cast<const BlueSharedString*>( value )->c_str() );
		return 1;

	case Be::IROOTPTR:
		return BlueWrapReturnValueImpl( ls, value->mIRootPtr ).ix;

	case Be::IROOT:
		return BlueWrapReturnValueImpl( ls, reinterpret_cast<IRoot*>( value ) ).ix;

	case Be::FLOATARRAY:
		{
			const char** componentNames = nullptr;
			float* floatArray = &value->mFloat;

			if( !entry->mIID || (entry->mIID == &BlueDefaultIID) || (entry->mIID == &BlueRotationIID) )
			{
				static const char* xyzw[] = {"x", "y", "z", "w"};
				componentNames = xyzw;
			}
			else if( entry->mIID == &BlueColorIID )
			{
				static const char* rgba[] = {"r", "g", "b", "a"};
				componentNames = rgba;
			}

			if( componentNames )
			{
				lua_newtable( ls );
				for( size_t ix = 0; ix < entry->GetFloatArraySize(); ++ix )
				{
					lua_pushnumber( ls, floatArray[ix] );
					lua_setfield( ls, -2, componentNames[ix] );
				}
				return 1;
			}
			luaL_error( ls, "Unsupported float array type" );
			return 0;
		}

	default:
		luaL_error( ls, "Unsupported type" );
		return 0;
	}
}

int BlueConvertValueFromLua( lua_State* ls, int idx, const Be::VarEntry* entry, Be::Var* value ) 
{
	Be::VARTYPE type = entry->mType;
	switch( type )
	{
	case Be::LONG:
		value->mLong = (int32_t)lua_tointeger( ls, idx );
		return 1;

	case Be::BOOL:
		if( lua_isboolean( ls, idx ) )
		{
			value->mBool = lua_toboolean( ls, idx ) ? true : false;
			return 1;
		}
		else
		{
			luaL_error( ls, "Expected a boolean" );
			return 0;
		}

	case Be::FLOAT:
		value->mFloat = (float)lua_tonumber( ls, idx );
		return 1;

	case Be::FLOATARRAY:
		{
			if( !entry->mIID || (entry->mIID == &BlueDefaultIID) )
			{
				switch( entry->GetFloatArraySize() )
				{
					case 3:
						{
							Vector3& vec = *(Vector3*)&value->mFloat;
							if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
							{
								return 1;
							}
							else
							{
								luaL_error( ls, "Expected a Vector3 value" );
								return 0;
							}
						}
						break;

					case 4:
						{
							Vector4& vec = *(Vector4*)&value->mFloat;
							if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
							{
								return 1;
							}
							else
							{
								luaL_error( ls, "Expected a Vector4 value" );
								return 0;
							}
						}
						break;

					case 2:
						{
							Vector2& vec = *(Vector2*)&value->mFloat;
							if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
							{
								return 1;
							}
							else
							{
								luaL_error( ls, "Expected a Vector2 value" );
								return 0;
							}
						}
						break;
				}
			}
			else if( entry->mIID == &BlueColorIID )
			{
				Color& color = *(Color*)&value->mFloat;
				if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), color, 0, std::false_type() ) )
				{
					return 1;
				}
				else
				{
					luaL_error( ls, "Expected a color value" );
					return 0;
				}
			}
			else if( entry->mIID == &BlueRotationIID )
			{
				Quaternion& quat = *(Quaternion*)&value->mFloat;
				if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), quat, 0, std::false_type() ) )
				{
					return 1;
				}
				else
				{
					luaL_error( ls, "Expected a quaternion value" );
					return 0;
				}
			}

			luaL_error( ls, "Unsupported float array type" );
			return 0;
		}
		break;

	case Be::DOUBLE:
		value->mDouble = lua_tonumber( ls, idx );
		return 1;

	case Be::DOUBLEARRAY:
		switch( entry->GetDoubleArraySize() )
		{
			case 2:
				{
					Vector2d& vec = *(Vector2d*)&value->mDouble;
					if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
					{
						return 1;
					}
					else
					{
						luaL_error( ls, "Expected a Vector2d value" );
						return 0;
					}
				}
				break;
			case 3:
				{
					Vector3d& vec = *(Vector3d*)&value->mDouble;
					if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
					{
						return 1;
					}
					else
					{
						luaL_error( ls, "Expected a Vector3d value" );
						return 0;
					}
				}
						
			case 4:
				{
					Vector4d& vec = *(Vector4d*)&value->mDouble;
					if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
					{
						return 1;
					}
					else
					{
						luaL_error( ls, "Expected a Vector4d value" );
						return 0;
					}
				}
		}
		luaL_error( ls, "Unsupported double array type" );
		return 0;
	
	case Be::INTARRAY:
		if( entry->GetIntArraySize() == 3 )
		{
			Vector3i& vec = *(Vector3i*)&value->mLong;
			if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), vec, 0, std::false_type() ) )
			{
				return 1;
			}
			else
			{
				luaL_error( ls, "Expected a Vector3i value" );
				return 0;
			}	
		}
		luaL_error( ls, "Unsupported int array type" );
		return 0;


	case Be::CSTRING:
		value->mCharPtr = CCP_STRDUP( __FUNCTION__, lua_tostring( ls, idx ) );
		return 1;

	case Be::STDSTRING:
		{
			std::string& s = *reinterpret_cast<std::string*>(value);
			if( BlueExtractString( BlueScriptValue( ls, idx ), s ) )
			{
				return 1;
			}
			else
			{
				luaL_error( ls, "Expected a string" );
				return 0;
			}
		}
	case Be::SCRIPTCALLBACK:
		{
			BlueScriptCallback& s = *reinterpret_cast<BlueScriptCallback*>(value);
			if( BlueExtractArgumentImpl( BlueScriptValue( ls, idx ), s, 0, std::false_type() ) )
			{
				return 1;
			}
			else
			{
				luaL_error( ls, "Expected a function" );
				return 0;
			}
		}
	case Be::SHAREDSTRING:
		{
			BlueSharedString& s = *reinterpret_cast<BlueSharedString*>(value);
			std::string str;
			if( BlueExtractString( BlueScriptValue( ls, idx ), str ) )
			{
				s = BlueSharedString( str );
				return 1;
			}
			else
			{
				luaL_error( ls, "Expected a string" );
				return 0;
			}
		}
	
	case Be::IROOTPTR:
		{
			IRoot* obj = BlueGetObjectFromStack( ls, idx );
			if( entry->mIID )
			{
				// Typed assignment
				IRoot* tmp = nullptr;
				obj->QueryInterface( *entry->mIID, (void**)&tmp, BEQI_NONE );

				if( !tmp )
				{
					luaL_error( ls, "Value lacks the correct interface" );
					return 0;
				}

				if( value->mIRootPtr )
				{
					value->mIRootPtr->Unlock();
				}

				value->mIRootPtr = tmp;
			}
			else
			{
				if( value->mIRootPtr )
				{
					value->mIRootPtr->Unlock();
				}

				value->mIRootPtr = obj;
				value->mIRootPtr->Lock();
			}
		}
		return 1;

	default:
		luaL_error( ls, "Unsupported type" );
		return 0;
	}
}

BLUEIMPORT void BlueWrapObjectForLua( lua_State* ls, IRoot* obj )
{
	BlueLuaWrapper::WrapObject( ls, obj );
}

int BlueLuaWrapper::Lua__index( lua_State* ls )
{
	IRoot* obj = BlueGetObjectFromStack( ls, 1 );
	const Be::ClassInfo* p = obj->ClassType();

	const char* name = lua_tostring( ls, 2 );

	IBlueRtti* rtti = BeClasses->GetRtti( p );
	BlueRttiValue* val = rtti->FindAttribute( name );
	if( val )
	{
		if( val->mType == BlueRttiValue::var )
		{
			Be::Var* value = (Be::Var*)((uintptr_t)obj + val->mTotalOffset);
			return BlueConvertValueToLua( ls, val->mVar, value );
		}
		else if( val->mType == BlueRttiValue::pyproperty )
		{
			const Be::VarEntry *var = val->mPyProperty;
			BluePropertyGetterFunction f = var->mGetProperty;
			if( !f )
			{
				luaL_error( ls, "Object property '%s' is write-only", name );
				return 0;
			}
			return f( ls, obj );
		}
	}

	val = rtti->FindMethod( name );
	if( val )
	{
		if (val->mType == BlueRttiValue::pymethod)
		{
			lua_pushcfunction( ls, val->mPyMethod->ml_meth );
			return 1;
		}
	}

	return 0;
}

void stackdump_g(lua_State* l)
{
	int i;
	int top = lua_gettop(l);

	CCP_LOG("total in stack %d\n",top);

	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
		int t = lua_type(l, i);
		switch (t) {
		case LUA_TSTRING:  /* strings */
			CCP_LOG("string: '%s'\n", lua_tostring(l, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			CCP_LOG("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  /* numbers */
			CCP_LOG("number: %g\n", lua_tonumber(l, i));
			break;
		case LUA_TUSERDATA:
			{
				if( !luaL_getmetafield( l, i, "bluetype" ) )
				{
					CCP_LOG("userdata\n");
				}
				else
				{
					const char* bluetype = lua_tostring( l, -1 );
					lua_remove( l, -1 );
					CCP_LOG( "%s\n", bluetype );
				}
			}
			break;
		default:  /* other values */
			CCP_LOG("%s\n", lua_typename(l, t));
			break;
		}
		CCP_LOG("  ");  /* put a separator */
	}
	CCP_LOG("\n");  /* end the listing */
}
    
int BlueLuaWrapper::Lua__newindex( lua_State* ls )
{
	IRoot* obj = BlueGetObjectFromStack( ls, 1 );
	const Be::ClassInfo* p = obj->ClassType();

	const char* name = lua_tostring( ls, 2 );

	IBlueRtti* rtti = BeClasses->GetRtti( p );
	BlueRttiValue* val = rtti->FindAttribute( name );
	if( val )
	{
		if (val->mType == BlueRttiValue::var)
		{
			const Be::VarEntry *var = val->mVar;
			if ((var->mEditFlags & Be::WRITE) == 0)
			{
				luaL_error( ls, "Object attribute '%s' is read-only", name );
				return 0;
			}

			Be::Var* value = (Be::Var*)((uintptr_t)obj + val->mTotalOffset);
			int result = BlueConvertValueFromLua( ls, 3, val->mVar, value );
			if( result )
			{
				INotifyPtr notify = BlueCastPtr( obj );
				if( notify )
				{
					notify->OnModified( value );
				}
			}
			return result;
		}
		else if( val->mType == BlueRttiValue::pyproperty )
		{
			const Be::VarEntry *var = val->mPyProperty;
			BluePropertySetterFunction f = var->mSetProperty;
			if( !f )
			{
				luaL_error( ls, "Object property '%s' is read-only", name );
				return 0;
			}
			return f( ls, obj );
		}
	}

	return 0;
}

int BlueLuaWrapper::CreateWrappedObject( lua_State* ls, const Be::Clsid& clsid )
{
	IRootPtr obj;
	obj.CreateInstance( clsid );
	return WrapObject( ls, obj );
}

int BlueLuaWrapper::WrapObject( lua_State* ls, IRoot* obj )
{
	void* memory = lua_newuserdata( ls, sizeof( BlueLuaWrapper ) );
	const char* name = obj->ClassType()->mMetatableName;
	luaL_setmetatable( ls, name );

	new( memory ) BlueLuaWrapper( obj );

	return 1;
}

#endif