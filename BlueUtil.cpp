#include "include/BlueUtil.h"
#include "include/IBlueDict.h"
#include "include/IBlueStructureList.h"
#include "include/IBlueClasses.h"
#include "BluePyWrap.h"
#include "BluePythonThunkers.h"
#include <cmath>

static CcpLogChannel_t s_chOS = CCP_LOG_DEFINE_CHANNEL( "OS" );


//--------------------------------------------------------------------
// Debug aid
//--------------------------------------------------------------------
#ifdef _DEBUG
#if 0
bool GetBlueObjectRepr(IRoot* obj, PyObject* pyobj, char* reprbuff, size_t bufsize)
{
	// Get class id
	const Be::Clsid* clsid = obj->ClassType()->mClassId;
	
	// See if there is a __repr__ method on it
	IPythonMethodsPtr meth(obj);

	PyObject* repr = NULL;
	bool handled = false;
	
	if (meth)
	{
		repr = meth->Repr(&handled);
	}

	// See if there is a valid "name" member
	BlueString name("name", obj);
	BlueWString wname("name", obj);

	// Get blue object refcount
	obj = obj->GetRootObject();
	BlueLockData* ld = NULL;
	
	if ((obj->GetFlags() & BLUERT_AUTOVAR) == 0)
	{
		ld = BlueInternalGetLockData(obj);
	}
	
	// Ex.:
	//triui.UIWindow, refcount=5, pyrefs=1, deco=no, name="", rot="", repr=""
	sprintf_s(
		reprbuff, bufsize,
		"%s.%s, refcount=%d, pyrefs=%d, deco=%s, name=\"%s\", wname=\"%S\", repr=\"%s\"",
		clsid->GetModule(), clsid->GetName(),
		obj->GetRefCount(),
		pyobj ? pyobj->ob_refcnt : -1,
		ld && ld->mPythonKlass ? "yes" : "no",
		name?name.c_str():"",
		wname?wname.c_str():L"",
		repr ? PyString_AS_STRING(repr) : ""
		);

	Py_XDECREF(repr);
	return true;
}

#endif
#endif


Be::Var* BeMapMemberOffset(
	IRoot* obj, 
	const Be::VarEntry* var, 
	const Be::ClassInfo* typeinfo,
	ssize_t xtraoffs
	)
{
	obj = obj->GetRootObject();

	return 
		(Be::Var*)((char*)(((uintptr_t)obj - 
		typeinfo->mInterfaceTable->mOffset + xtraoffs)) + (var)->mOffset);
}


//--------------------------------------------------------------------
// Performance timer / Timer helper routines
//--------------------------------------------------------------------

long TimeInMs(Be::Time time)
{
	time /= 10000;

	CCP_ASSERT(time <= LONG_MAX);

	return (long)time;
}


double TimeAsDouble(Be::Time time)
{
	double sec;

	// can either be UTC time or stopwatch time
	if (TimeIsUTC(time))
	{
#ifdef _MSC_VER
		_fpreset();
#endif
		Be::Time seconds = time / 10000000;
		sec = (double)seconds;

		time = (time - (seconds * 10000000));
		sec += (time / 10000) / 1000.0;
	}
	else
	{
#ifdef _MSC_VER
		_fpreset();
#endif
		Be::Time seconds = time / 10000000;
		sec = (double)seconds;

		time = (time - (seconds * 10000000));
		sec += time / 10000000.0;
	}

	return sec;
}

float TimeAsFloat( Be::Time time )
{
	return (float)TimeAsDouble( time );
}

Be::Time TimeFromDouble(double time)
{
	double intpart, fracpart;
	fracpart = modf(time, &intpart);

	Be::Time t = (Be::Time)(fracpart * 10000000.0);
	t += ((Be::Time)intpart) * 10000000;
	
	return t;
}
Be::Time TimeFromMS(long time)
{
	Be::Time t = (Be::Time)(time * 10000);
	return t;
}

bool TimeIsUTC(Be::Time time)
{
	// if time is later than the 18th century, let's
	// rule it UTC

	// UTC time starts at year 1600
	// so it's seconds * min * hours * days * years * 200
	const Be::Time EIGHTEENTHCENTURY =
		//10000000 * 60 * 60 * 24 * 365 * 200
		63072000000000000;

	if (time > EIGHTEENTHCENTURY)
		return true;
	else
		return false;
}



//--------------------------------------------------------------------
// Performance timer
//--------------------------------------------------------------------
BeTimer::BeTimer(const char *msg)
{
	mMsg = msg;
	Reset();
}


void BeTimer::Reset()
{
	mStart = CcpGetTimestamp();
}


void BeTimer::LogTime(const char *msg) const
{
	double s = GetSeconds();

	if (s > 1.0)
		CCP_LOG_CH( s_chOS, "%s: %.3f sec.\n", msg ? msg : "ticks", s);
	else
		CCP_LOG_CH( s_chOS, "%s: %.3f ms.\n", msg ? msg : "ticks", s * 1000.0);
}


Be::Time BeTimer::GetTime() const
{
	int64_t elap = GetCycles();
	
	elap *= 100;
	elap /= (GetFreq() / 100000);

	return elap;
}

int64_t BeTimer::GetCycles() const
{
	Be::Time now = CcpGetTimestamp();
	return now - mStart;
}

int64_t BeTimer::GetFreq()
{
	return CcpGetTimestampFrequency();
}

double BeTimer::GetSeconds() const
{
	return (double)GetCycles() / (double)GetFreq();
}




IRoot* BluePtrAssign(IRoot** pp, IRoot* lp)
{
	if (lp != NULL)
		lp->Lock();
	IRoot *pTmp = *pp;
	if (pTmp) {
		*pp = NULL;
		pTmp->Unlock();
	}
	*pp = lp;
	return lp;
}


IRoot* BlueQIPtrAssign(
	IRoot** pp, IRoot* lp, 
	const Be::IID& riid, 
	BLUEQIOPT options
	)
{
	IRoot* pTemp = *pp, *pNew = NULL;
	*pp = NULL;
	if (lp != NULL)
		lp->QueryInterface(riid, (void**)&pNew, options);
	if (pTemp)
		pTemp->Unlock();
	*pp = pNew;
	return *pp;
}

bool BlueCreateInstance(const Be::Clsid& clsid, const Be::IID& riid, void** ppv)
{
	return BeClasses->CreateInstance(clsid, riid, ppv);
}


#if BLUE_WITH_PYTHON
BLUEIMPORT void BlueInitializePyType( PyTypeObject* pyType, const Be::Clsid* clsid, const char* doc, const Be::InterfaceEntry* interfaces, newfunc pyNew )
{
	CCP_ASSERT( pyType );
	CCP_ASSERT( interfaces );
	CCP_ASSERT( clsid );
	CCP_ASSERT( clsid->GetModule() );
	CCP_ASSERT( clsid->GetName() );

	std::string name = clsid->GetModule();
	name += ".";
	name += clsid->GetName();

	bool isPlain = true;

	static Be::IID pythonNumericType( "IPythonNumeric" );
	static Be::IID listType( "IList" );
	static Be::IID blueDictType( "IBlueDict" );
	static Be::IID blueStructureListType( "IBlueStructureList" );


	const Be::InterfaceEntry* p = interfaces;
	while( p->mIID )
	{
		if( p->mIID->IsEqual( pythonNumericType ) )
		{
			BlueWrapper::InitializeNumericTypeObject( pyType, name.c_str() );
			isPlain = false;
			break;
		}

		if( p->mIID->IsEqual( listType ) )
		{
			BlueWrapper::InitializeListTypeObject( pyType, name.c_str() );
			isPlain = false;
			break;
		}

		if( p->mIID->IsEqual( blueDictType ) )
		{
			BlueWrapper::InitializeDictTypeObject( pyType, name.c_str() );
			isPlain = false;
			break;
		}

		if( p->mIID->IsEqual( blueStructureListType ) )
		{
			BlueWrapper::InitializeStructureListTypeObject( pyType, name.c_str() );
			isPlain = false;
			break;
		}

		// Note that we don't support an object exposing more than one of
		// numeric, list and dict protocols simultaneously.

		++p;
	}

	if( isPlain )
	{
		BlueWrapper::InitializeStandardTypeObject( pyType, name.c_str() );
	}

	pyType->tp_new = pyNew;
	pyType->tp_doc = doc;
}

// Helper function for implementing the object instantiation function
// set to Python type objects
BLUEIMPORT PyObject* BlueCreateInstanceFromPython( const Be::Clsid& clsid, PyObject* args, PyObject* kwds )
{
	IRootPtr obj;

	const Be::ClassRegistration* cr = BeClasses->GetClassRegistration( clsid );
	CCP_ASSERT( cr );

	static Be::IID irootType( "IRoot" );
	if( !cr->mCreateFn( irootType, (void**)&obj ) )
	{
		Py_RETURN_NONE;
	}

	BluePythonObject* ret =  BlueWrapObjectForPython(obj);

	if( PyObject_HasAttrString( ret, "__init__" ) )
	{
		PyObject* initCall = PyObject_GetAttrString( ret, "__init__" );
		PyObject* initResult = PyObject_CallObject( initCall, args );
		Py_XDECREF(initCall);
		if (!initResult)
		{
			Py_DECREF(ret);
			return NULL;
		}
		Py_XDECREF(initResult);
	}

	return ret;
}
#endif


std::vector<std::wstring> GetSplitCommandLine()
{
	std::vector<std::wstring> res;

#ifdef _WIN32
	const wchar_t *line = GetCommandLineW();

	int numArgs;
	LPWSTR *words = CommandLineToArgvW(line, &numArgs);
	for( int i = 0; i < numArgs; ++i )
	{
		res.push_back( words[i] );
	}
	LocalFree( words );
#else
	// TODO: Implement
#endif

	return res;

}

#ifdef _WIN32
bool WINAPI QueryPerformanceCounterCCP( LARGE_INTEGER* li )
{
	static LARGE_INTEGER last = {0};
	BOOL ok = QueryPerformanceCounter(li);
	if( !ok )
	{
		return false;
	}

	if( li->QuadPart > last.QuadPart )
	{
		last = *li;
	}
	else
	{
		*li = last;
	}
	return true;
}

bool WINAPI QueryPerformanceFrequencyCCP( LARGE_INTEGER *lpFrequency )
{
	return QueryPerformanceFrequency( lpFrequency ) == TRUE;
}
#endif

const char* GetPlatformToolset()
{
	return CCP_STRINGIZE( PLATFORM_TOOLSET );
}