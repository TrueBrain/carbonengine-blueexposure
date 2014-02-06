////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   August 2013
//    Copyright: CCP 2013
//

#include "include/BlueScriptCallback.h"

#if BLUE_WITH_PYTHON
namespace
{

std::string FormatTraceback( PyObject *tb )
{
	std::string result = "Traceback (most recent call first):\n";
	PyObject* t = tb;
	while( t && PyObject_IsTrue( t ) )
	{
		std::string filename = "<none>";
		PyObject* frame = PyObject_GetAttrString( t, "tb_frame" );
		if( frame ) 
		{
			PyObject* code = PyObject_GetAttrString( frame, "f_code" );
			if( code ) 
			{
				PyObject* fname = PyObject_GetAttrString( code, "co_filename" );
				if( fname )
				{
					filename = std::string( PyString_AsString( fname ) );
					Py_DECREF( fname );
				}
				Py_DECREF( code );
			}
			Py_DECREF( frame );
		}
		PyErr_Clear();
		int line = 0;
		PyObject* lineno = PyObject_GetAttrString( t, "tb_lineno" );
		if( lineno )
		{
			line = int( PyInt_AsLong( lineno ) );
			Py_DECREF( lineno );
		}
		PyErr_Clear();
		char clineno[16];
		sprintf_s( clineno, "%i\n", line );
		result += filename + ":" + clineno;
		PyObject* tmp = t;
		t = PyObject_GetAttrString( t, "tb_next" );
		Py_DECREF( tmp );
	}
	Py_XDECREF( t );
	result += "Traceback end\n";
	PyErr_Clear();
	return result;
}

std::string FormatExceptionFallback( PyObject *type, PyObject *val, PyObject *tb )
{
	PyErr_Clear();

	std::string result = "Simple exception format:\n";
	PyObject* s = PyObject_Repr( type );
	if( s )
	{
		result += std::string( "Type: " ) + PyString_AsString( s ) + "\n";
		Py_DECREF( s );
	}
	if( val ) 
	{
		s = PyObject_Repr( val );
		if( s )
		{
			result += std::string( "Value: " ) + PyString_AsString( s ) + "\n";
			Py_DECREF( s );
		}
	}
	if( tb )
	{
		result += FormatTraceback( tb );
	}
	result += "Simple exception end\n";
	PyErr_Clear();

	return result;
}

std::string FormatException( PyObject* type, PyObject* val, PyObject* tb )
{
	//import the traceback module
	PyObject* module = PyImport_ImportModule( "traceback" );
	if( !module )
	{
		return FormatExceptionFallback( type, val, tb );
	}
	PyObject* lines = PyObject_CallMethod( module, const_cast<char*>( "format_exception" ), const_cast<char*>( "OOO" ), type, val? val : Py_None, tb ? tb : Py_None );
	Py_DECREF( module );
	if( !lines ) 
	{
		return FormatExceptionFallback( type, val, tb );
	}
	PyObject* str = PyString_FromString( "" );
	if( !str )
	{
		Py_DECREF( lines );
		return FormatExceptionFallback( type, val, tb );
	}
	PyObject* linesJoined = PyObject_CallMethod( str, const_cast<char*>( "join" ), const_cast<char*>( "O" ), lines );
	Py_DECREF( str );
	Py_DECREF( lines );
	if( !linesJoined )
	{
		return FormatExceptionFallback( type, val, tb );
	}
	std::string result = PyString_AsString( linesJoined );
	Py_DECREF( linesJoined );
	PyErr_Clear();
	return result;
}

}
#endif

BlueScriptCallbackStatus::BlueScriptCallbackStatus( Status hasException )
	:m_hasException( hasException ),
	m_muteException( false ),
	m_exceptionReported( false )
#if BLUE_WITH_PYTHON
	, m_type( nullptr ),
	m_value( nullptr ),
	m_traceback( nullptr )
#endif
{
#if BLUE_WITH_PYTHON
	if( hasException == EXCEPTION )
	{
		PyErr_Fetch( &m_type, &m_value, &m_traceback );
	}
#endif
}

BlueScriptCallbackStatus::~BlueScriptCallbackStatus()
{
	ReportException();
#if BLUE_WITH_PYTHON
	Py_XDECREF( m_type );
	Py_XDECREF( m_value );
	Py_XDECREF( m_traceback );
#endif
}

BlueScriptCallbackStatus::BlueScriptCallbackStatus( const BlueScriptCallbackStatus& other )
	:m_hasException( other.m_hasException ),
	m_muteException( other.m_muteException ),
	m_exceptionReported( other.m_exceptionReported )
#if BLUE_WITH_PYTHON
	, m_type( other.m_type ),
	m_value( other.m_value ),
	m_traceback( other.m_traceback )
#endif
{
#if BLUE_WITH_PYTHON
	Py_XINCREF( m_type );
	Py_XINCREF( m_value );
	Py_XINCREF( m_traceback );
#endif
}

BlueScriptCallbackStatus& BlueScriptCallbackStatus::operator=( const BlueScriptCallbackStatus& other )
{
	if( this == &other )
	{
		return *this;
	}
#if BLUE_WITH_PYTHON
	Py_XDECREF( m_type );
	Py_XDECREF( m_value );
	Py_XDECREF( m_traceback );
#endif
	m_hasException = other.m_hasException;
	m_muteException = other.m_muteException;
	m_exceptionReported = other.m_exceptionReported;
#if BLUE_WITH_PYTHON
	m_type = other.m_type;
	m_value = other.m_value;
	m_traceback = other.m_traceback;

	Py_XINCREF( m_type );
	Py_XINCREF( m_value );
	Py_XINCREF( m_traceback );
#endif
	return *this;
}

BlueScriptCallbackStatus::operator bool() const
{
	return m_hasException == OK;
}

void BlueScriptCallbackStatus::MuteException()
{
	m_muteException = true;
}

void BlueScriptCallbackStatus::ReportException()
{
	if( m_exceptionReported || m_muteException || !m_hasException )
	{
		return;
	}
	m_exceptionReported = true;

#if BLUE_WITH_PYTHON
	if( !m_type && !m_value && !m_traceback ) 
	{
		return;
	}
	std::string exception = FormatException( m_type, m_value, m_traceback );
	CCP_LOGERR( "%s", exception.c_str() );
#endif
}

BLUEIMPORT BlueScriptCallback::BlueScriptCallback()
#if BLUE_WITH_LUA
	:m_callback( 0, 0 ),
	m_refCount( nullptr )
#elif BLUE_WITH_PYTHON
	:m_callback( nullptr )
#endif
{
}

BLUEIMPORT BlueScriptCallback::BlueScriptCallback( const BlueScriptCallback& other )
	:m_callback( other.m_callback )
{
#if BLUE_WITH_LUA
	m_refCount = other.m_refCount;
	if( m_refCount )
	{
		++m_refCount[0];
	}
#elif BLUE_WITH_PYTHON
	Py_XINCREF( m_callback );
#endif
}

BLUEIMPORT BlueScriptCallback::~BlueScriptCallback()
{
	Destroy();
}

BLUEIMPORT BlueScriptCallback& BlueScriptCallback::operator=( const BlueScriptCallback& other )
{
#if BLUE_WITH_LUA
	if( m_refCount == other.m_refCount )
	{
		return *this;
	}
#elif BLUE_WITH_PYTHON
	if( m_callback == other.m_callback )
	{
		return *this;
	}
#endif
	Destroy();
	m_callback = other.m_callback;
#if BLUE_WITH_LUA
	m_refCount = other.m_refCount;
	if( m_refCount )
	{
		++m_refCount[0];
	}
#elif BLUE_WITH_PYTHON
	Py_XINCREF( m_callback );
#endif
	return *this;
}

BLUEIMPORT bool BlueScriptCallback::IsValid() const
{
#if BLUE_WITH_LUA
	return m_refCount != nullptr;
#elif BLUE_WITH_PYTHON
	return m_callback != nullptr;
#endif
}

BLUEIMPORT void BlueScriptCallback::Destroy()
{
#if BLUE_WITH_LUA
	if( IsValid() )
	{
		if( --m_refCount[0] == 0 )
		{
			luaL_unref( m_callback.ls, LUA_REGISTRYINDEX, m_callback.ix );
			CCP_DELETE m_refCount;
		}
		m_refCount = nullptr;
		m_callback.ls = nullptr;
		m_callback.ix = 0;
	}
#elif BLUE_WITH_PYTHON
	Py_XDECREF( m_callback );
	m_callback = nullptr;
#endif
}

BLUEIMPORT BlueScriptCallbackStatus BlueScriptCallback::CallVoid()
{
	if( !IsValid() )
	{
		return BlueScriptCallbackStatus( BlueScriptCallbackStatus::CALL_ERROR );
	}

#if BLUE_WITH_LUA
	lua_rawgeti( m_callback.ls, LUA_REGISTRYINDEX, m_callback.ix );
	if( lua_pcall( m_callback.ls, 0, 0, 0 ) )
	{
		return BlueScriptCallbackStatus::EXCEPTION;
	}
	return BlueScriptCallbackStatus::OK;
#elif BLUE_WITH_PYTHON
	PyObject* ret = PyObject_CallFunctionObjArgs( m_callback, nullptr );
	if( ret )
	{
		Py_DECREF( ret );
		return BlueScriptCallbackStatus( BlueScriptCallbackStatus::OK );
	}
	else
	{
		PyErr_Clear();
	}
	return BlueScriptCallbackStatus( BlueScriptCallbackStatus::EXCEPTION );
#endif
}

BlueScriptValue BlueWrapReturnValueImpl( BlueScriptArguments args, const BlueScriptCallback& val )
{
#if BLUE_WITH_LUA
	BlueScriptValue returnValue( args, 1 );
	if( val.IsValid() )
	{
		lua_rawgeti( val.m_callback.ls, LUA_REGISTRYINDEX, val.m_callback.ix );
	}
	else
	{
		lua_pushnil( args );
	}
	return returnValue;
#elif BLUE_WITH_PYTHON
	if( val.m_callback )
	{
		Py_INCREF( val.m_callback );
		return val.m_callback;
	}
	else
	{
		Py_RETURN_NONE;
	}
#endif
}

bool BlueExtractArgumentImpl( BlueScriptValue argument, BlueScriptCallback& result, unsigned int argID, std::false_type isBlueType )
{
#if BLUE_WITH_LUA
	if( lua_isfunction( argument.ls, argument.ix ) || lua_isnil( argument.ls, argument.ix ) )
	{
		result.Destroy();
		if( lua_isfunction( argument.ls, argument.ix ) )
		{
			result.m_callback.ls = argument.ls;
			result.m_callback.ix = luaL_ref( argument.ls, LUA_REGISTRYINDEX );
			result.m_refCount = CCP_NEW( "BlueScriptCallback::m_refCount" ) uint32_t;
			result.m_refCount[0] = 1;
		}
		return true;
	}
	else
	{
		luaL_error( argument.ls, argumentTypeMismatchString, argID, "function" );
		return false;
	}
#elif BLUE_WITH_PYTHON
	if( !PyCallable_Check( argument ) && argument != Py_None )
	{
		PyErr_Format( PyExc_TypeError, 
			"Could not match argument %i to expected type: function expected.", argID );
		return false;
	}
	result.Destroy();
	if( argument != Py_None )
	{
		result.m_callback = argument;
		Py_INCREF( result.m_callback );
	}
	return true;
#endif
}
