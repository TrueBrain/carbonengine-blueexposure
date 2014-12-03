#if BLUE_WITH_LUA

#include "include/BlueExtractArgument.h"
#include "include/BlueScriptValue.h"
#include "CcpCore/include/StringConversions.h"

bool BlueExtractString( BlueScriptValue sv, std::string& val )
{
	bool ok = false;

	if( lua_isstring( sv.ls, sv.ix ) )
	{
		size_t length = 0;
		const char* buffer = lua_tolstring( sv.ls, sv.ix, &length );
		val.assign( buffer, length );
		ok = true;
	}

	return ok;
}

bool BlueExtractWString( BlueScriptValue sv, std::wstring& val )
{
	bool ok = false;

	if( lua_isstring( sv.ls, sv.ix ) )
	{
		std::string valA;
		size_t length = 0;
		const char* buffer = lua_tolstring( sv.ls, sv.ix, &length );
		valA.assign( buffer, length );
		val = CA2W( valA.c_str() );
		ok = true;
	}

	return ok;
}

bool BlueExtractBool( BlueScriptValue sv, bool& value )
{
	if( lua_isboolean( sv.ls, sv.ix ) )
	{
		value = lua_toboolean( sv.ls, sv.ix ) ? true : false;
	}
	else
	{
		return false;
	}

	return true;
}

bool BlueExtractInt( BlueScriptValue sv, int& value )
{
	if( lua_isnumber( sv.ls, sv.ix ) )
	{
		value = (int)lua_tonumber( sv.ls, sv.ix );
	}
	else
	{
		return false;
	}

	return true;
}

bool BlueExtractDouble( BlueScriptValue sv, double& value )
{
	if( lua_isnumber( sv.ls, sv.ix ) )
	{
		value = lua_tonumber( sv.ls, sv.ix );
	}
	else
	{
		return false;
	}

	return true;
}

bool BlueExtractFloat( BlueScriptValue sv, float& value )
{
	if( lua_isnumber( sv.ls, sv.ix ) )
	{
		value = (float)lua_tonumber( sv.ls, sv.ix );
	}
	else
	{
		return false;
	}

	return true;
}

#if 0


bool BlueExtractVector( PyObject* sequence, float* elements, size_t elementsCount )
{
	if( !PySequence_Check( sequence ) )
	{
		return false;
	}

	Py_ssize_t tupleCount = PySequence_Size( sequence );
	if( tupleCount < 0 || (size_t)tupleCount > elementsCount )
	{
		return false;
	}

	for( Py_ssize_t i = 0; i < tupleCount; ++i )
	{
		PyObject* item = PySequence_ITEM( sequence, i );
		bool success = BlueExtractFloat( item, elements[ i ] );
		Py_DECREF( item );

		if( !success )
		{
			return false;
		}
	}

	return true;
}

bool BlueExtractMatrix( PyObject* sequence, float* elements, size_t elementsCount )
{
	size_t dimension = 0;
	if( elementsCount == 16 )
	{
		dimension = 4;
	}
	else if( elementsCount == 9 )
	{
		dimension = 3;
	}

	if( !PySequence_Check( sequence ) )
	{
		return false;
	}

	Py_ssize_t tupleCount = PySequence_Size( sequence );
	if( tupleCount != (Py_ssize_t)dimension )
	{
		return false;
	}

	float* offset = elements;
	for( ssize_t i = 0; i < tupleCount; ++i )
	{
		PyObject* subSequence = PySequence_ITEM( sequence, i );
		bool ok = BlueExtractVector( subSequence, offset, dimension );
		Py_DECREF( subSequence );
		if( !ok )
		{
			return false;
		}
		offset += dimension;
	}

	return true;
}
#endif

// Overload for Float argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, float& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractFloat( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "float" );
		return false;
	}

	return true;
}

// Overload for Double argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, double& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractDouble( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "double" );
		return false;
	}

	return true;
}

#if 0

// Overload for PyObject* argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( PyObject* argument, PyObject*& result, unsigned int argID, std::false_type isBlueType )
{
	result = argument;
	return true;
}

#endif

// Overload for Int argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, int32_t& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractInt( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "int" );
		return false;
	}

	return true;
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, uint32_t& result, unsigned int argID, std::false_type isBlueType )
{
	int tmp;
	bool success = BlueExtractInt( argument, tmp );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "uint32_t" );
		return false;
	}

	result = tmp;

	return true;
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, uint8_t& result, unsigned int argID, std::false_type isBlueType )
{
	int tmp;
	bool success = BlueExtractInt( argument, tmp );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "uint8_t" );
		return false;
	}

	result = tmp;

	return true;
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, uint16_t& result, unsigned int argID, std::false_type isBlueType )
{
	int tmp;
	bool success = BlueExtractInt( argument, tmp );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "uint16_t" );
		return false;
	}

	result = tmp;

	return true;
}

// Overload for boolean argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, bool& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractBool( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "bool" );
		return false;
	}

	return true;
}

// Overload for signed int64 argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, int64_t& result, unsigned int argID, std::false_type isBlueType )
{
	// TODO: This is isn't proper 64bit support
	int tmp;
	bool success = BlueExtractInt( argument, tmp );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "int64_t" );
		return false;
	}

	result = tmp;

	return true;
}


// Overload for unsigned int64 argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, uint64_t& result, unsigned int argID, std::false_type isBlueType )
{
	// TODO: This is isn't proper 64bit support
	int tmp;
	bool success = BlueExtractInt( argument, tmp );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "uint64_t" );
		return false;
	}

	result = tmp;

	return true;
}

#if 0
#ifdef __APPLE__
// Clang on the Mac treats size_t as something different from uint32_t/uint64_t, whereas
// other compilers complain about redefinition of this function if we keep it in.
// Overload for unsigned size_t argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( PyObject* argument, size_t& result, unsigned int argID, std::false_type isBlueType )
{
	if( PyInt_Check(argument) )
	{
		result = (size_t)PyInt_AsSsize_t( argument );
	}
	else if( PyLong_Check(argument) )
	{
		result = (size_t)PyLong_AsUnsignedLongLong( argument );
	}
	else
	{
		PyErr_Format( PyExc_TypeError, argumentTypeMismatchString, argID, "size_t" );
		return false;
	}

	return true;

}
#endif

#ifdef _MSC_VER
bool BLUEIMPORT BlueExtractArgumentImpl( PyObject* argument, unsigned long& result, unsigned int argID, std::false_type isBlueType )
{
	// Note: An unsigned long is 32 bits in C++ and the largest number stored is thus 2^32-1.
	// However, Python is clever and when you assign a number that fits within 32 bits with sign
	// it will give it a PyInt type.  However, once you go beyond that it will make it a PyLong.
	// So, to support the full range of C++ 32bit unsigned we need to expect both PyInt and PyLong.
	if( PyInt_Check(argument) )
	{
		result = (unsigned int)PyInt_AsLong( argument );
	}
	else if( PyLong_Check(argument) )
	{
		result = (unsigned int)PyLong_AsUnsignedLong( argument );
	}
	else
	{
		PyErr_Format( PyExc_TypeError, argumentTypeMismatchString, argID, "unsigned long" );
		return false;
	}

	return true;
}

#endif

#endif

// Overload for string argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, std::string& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractString( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "string" );
		return false;
	}

	return true;
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, const char*& result, unsigned int argID, std::false_type isBlueType )
{
	if( lua_isstring( argument.ls, argument.ix ) )
	{
		size_t length = 0;
		result = lua_tolstring( argument.ls, argument.ix, &length );
		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "string" );
		return false;
	}
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, std::wstring& result, unsigned int argID, std::false_type isBlueType )
{
	bool success = BlueExtractWString( argument, result );
	if( !success )
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "wstring" );
		return false;
	}

	return true;
}

bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, const wchar_t*& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

// Overload for Matrix argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Matrix& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

bool ConvertSequenceToFloatArray( BlueScriptValue argument, float* targetElements, size_t targetElementsCount, unsigned int argID )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

// Overload for Vector2 argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector2& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

// Overload for Vector2d argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector2d& result, unsigned int argID, std::false_type isBlueType )
{
	luaL_error( argument.ls, "Not implemented" );
	return false;
}

// Overload for Vector3 argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector3& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );

		result.x = (float)lua_tonumber( ls, -3 );
		result.y = (float)lua_tonumber( ls, -2 );
		result.z = (float)lua_tonumber( ls, -1 );

		lua_pop( ls, 3 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector3" );
		return false;
	}
}

// Overload for Vector3d argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector3d& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );

		result.x = (double)lua_tonumber( ls, -3 );
		result.y = (double)lua_tonumber( ls, -2 );
		result.z = (double)lua_tonumber( ls, -1 );

		lua_pop( ls, 3 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector3d" );
		return false;
	}
}

// Overload for Vector3i argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector3i& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );

		result.x = (int)lua_tonumber( ls, -3 );
		result.y = (int)lua_tonumber( ls, -2 );
		result.z = (int)lua_tonumber( ls, -1 );

		lua_pop( ls, 3 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector3i" );
		return false;
	}
}


// Overload for Vector4 argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector4& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );
		lua_getfield( ls, argument.ix, "w" );

		result.x = (float)lua_tonumber( ls, -4 );
		result.y = (float)lua_tonumber( ls, -3 );
		result.z = (float)lua_tonumber( ls, -2 );
		result.w = (float)lua_tonumber( ls, -1 );

		lua_pop( ls, 4 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector4" );
		return false;
	}
}

// Overload for Vector4d argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Vector4d& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );
		lua_getfield( ls, argument.ix, "w" );

		result.x = (double)lua_tonumber( ls, -4 );
		result.y = (double)lua_tonumber( ls, -3 );
		result.z = (double)lua_tonumber( ls, -2 );
		result.w = (double)lua_tonumber( ls, -1 );

		lua_pop( ls, 4 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector4d" );
		return false;
	}
}

// Overload for Color argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Color& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "r" );
		lua_getfield( ls, argument.ix, "g" );
		lua_getfield( ls, argument.ix, "b" );
		lua_getfield( ls, argument.ix, "a" );

		result.r = (float)lua_tonumber( ls, -4 );
		result.g = (float)lua_tonumber( ls, -3 );
		result.b = (float)lua_tonumber( ls, -2 );
		result.a = (float)lua_tonumber( ls, -1 );

		lua_pop( ls, 4 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Color" );
		return false;
	}
}

// Overload for Quaternion argument extraction
bool BLUEIMPORT BlueExtractArgumentImpl( BlueScriptValue argument, Quaternion& result, unsigned int argID, std::false_type isBlueType )
{
	lua_State* ls = argument.ls;

	if( lua_istable( ls, argument.ix ) )
	{
		lua_getfield( ls, argument.ix, "x" );
		lua_getfield( ls, argument.ix, "y" );
		lua_getfield( ls, argument.ix, "z" );
		lua_getfield( ls, argument.ix, "w" );

		result.x = (float)lua_tonumber( ls, -4 );
		result.y = (float)lua_tonumber( ls, -3 );
		result.z = (float)lua_tonumber( ls, -2 );
		result.w = (float)lua_tonumber( ls, -1 );

		lua_pop( ls, 4 );

		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "Vector4" );
		return false;
	}
}

#if 0
PyObject* ConvertMatrixToSequence( const Matrix* m )
{
	PyObject* ret = PyTuple_New( 4 );

	// Start of floating point of array so we can iterate simply over the data
	const float* array = &m->_11;
	for( int i = 0; i < 4; ++i )
	{
		PyObject* tuple = PyTuple_New( 4 );
		// Marshall row 'i' in the array as a 4-tuple
		for( int k = 0; k < 4; ++k )
		{
			PyTuple_SET_ITEM( tuple, k, PyFloat_FromDouble( array[4*i + k ] ) );
		}
		// Collect the tuple into a tuple of rows
		PyTuple_SET_ITEM( ret, i, tuple );
	}

	return ret;
}
#endif

#endif