////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		January 2013
// Copyright:	CCP 2013
//
// Originally implemented by Dan Speed

#pragma once
#ifndef BlueTypeTraits_h
#define BlueTypeTraits_h

#include <type_traits>
#include "BlueClass.h"
#include "BlueSharedString.h"

struct IRoot;
struct Vector2;
struct Vector2d;
struct Vector3;
struct Vector3d;
struct Vector3i;
struct Vector4;
struct Vector4d;
struct Matrix;
struct Quaternion;
struct Color;
class BlueScriptCallback;

// Automatic Be::VARTYPE extraction by passing in a C++ type, use like this:
// Be::VARTYPE t = TypeTraits<long>::VARTYPE_VALUE;
// Note that instead of 'long' you can use most basic C++ types!

template<typename T> class BlueTypeTraits
{
public:
	// Note that we don't want to define a VARTYPE_VALUE here,
	// it is defined by the specializations below. Leaving
	// the non-specialized template without a VARTYPE_VALUE
	// means we get a compilation error on unsupported types.
};

#define BLUE_DEFINE_TYPE_TRAIT( cppType_, varType_ ) \
	template<> struct BlueTypeTraits< cppType_ > \
{ \
	static const Be::VARTYPE VARTYPE_VALUE = varType_; \
}

BLUE_DEFINE_TYPE_TRAIT( int64_t, Be::INT64 );
BLUE_DEFINE_TYPE_TRAIT( uint64_t, Be::INT64 );
#if defined(_MSC_VER) || defined(__ANDROID__)
BLUE_DEFINE_TYPE_TRAIT( long, Be::LONG );
BLUE_DEFINE_TYPE_TRAIT( unsigned long, Be::LONG );
#endif
BLUE_DEFINE_TYPE_TRAIT( int32_t, Be::LONG );
BLUE_DEFINE_TYPE_TRAIT( uint32_t, Be::LONG );
BLUE_DEFINE_TYPE_TRAIT( short, Be::SHORT );
BLUE_DEFINE_TYPE_TRAIT( unsigned short, Be::SHORT );
BLUE_DEFINE_TYPE_TRAIT( float, Be::FLOAT );
BLUE_DEFINE_TYPE_TRAIT( double, Be::DOUBLE );
BLUE_DEFINE_TYPE_TRAIT( bool, Be::BOOL );
BLUE_DEFINE_TYPE_TRAIT( char, Be::BYTE );
BLUE_DEFINE_TYPE_TRAIT( unsigned char, Be::BYTE );
BLUE_DEFINE_TYPE_TRAIT( Vector2, Be::FLOATARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector2d, Be::DOUBLEARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector3, Be::FLOATARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector3d, Be::DOUBLEARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector3i, Be::INTARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector4, Be::FLOATARRAY );
BLUE_DEFINE_TYPE_TRAIT( Vector4d, Be::DOUBLEARRAY );
BLUE_DEFINE_TYPE_TRAIT( Matrix, Be::FLOATARRAY );
BLUE_DEFINE_TYPE_TRAIT( Color, Be::FLOATARRAY );
BLUE_DEFINE_TYPE_TRAIT( Quaternion, Be::FLOATARRAY );
#if BLUE_WITH_PYTHON
BLUE_DEFINE_TYPE_TRAIT( PyObject*, Be::PYOBJECTPTR );
#endif
BLUE_DEFINE_TYPE_TRAIT( char*, Be::CSTRING );
BLUE_DEFINE_TYPE_TRAIT( wchar_t*, Be::WCSTRING );
BLUE_DEFINE_TYPE_TRAIT( std::string, Be::STDSTRING );
BLUE_DEFINE_TYPE_TRAIT( std::wstring, Be::STDWSTRING );
#ifdef __APPLE__
// clang on mac treats size_t as a totally separate type
BLUE_DEFINE_TYPE_TRAIT( size_t, sizeof( size_t ) == 32 ? Be::LONG : Be::INT64 );
BLUE_DEFINE_TYPE_TRAIT( ssize_t, sizeof( ssize_t ) == 32 ? Be::LONG : Be::INT64 );
#endif

#ifdef _WIN32
BLUE_DEFINE_TYPE_TRAIT( HWND, Be::LONG );
#endif

BLUE_DEFINE_TYPE_TRAIT( BlueScriptCallback, Be::SCRIPTCALLBACK );
BLUE_DEFINE_TYPE_TRAIT( BlueSharedString, Be::SHAREDSTRING );

extern BLUEIMPORT const Be::IID BlueDefaultIID;
extern BLUEIMPORT const Be::IID BlueRotationIID;
extern BLUEIMPORT const Be::IID BlueDirectionIID;
extern BLUEIMPORT const Be::IID BlueColorIID;
extern BLUEIMPORT const Be::IID BlueMatrixIID;
extern BLUEIMPORT const Be::IID BlueTimeIID;

//////////////////////////////////////////////////////////////////////////
// Useful Type-traits
//////////////////////////////////////////////////////////////////////////

template< typename T >
struct is_be_result
{
	typedef std::false_type type;
};

template<typename T>
struct is_be_result< Be::Result<T> >
{
	typedef std::true_type type;
};

template< typename T >
struct remove_const_and_reference
{
	typedef
		typename std::remove_const<
			typename std::remove_reference< T >::type
		>::type
		type;
};

template< typename T >
struct is_blue_type
{
	typedef
		typename std::is_base_of<
			IRoot,
			typename std::remove_pointer< T >::type
		>::type
		type;
};

template< typename T >
struct is_blue_type<Be::Optional<T> >
{
	typedef typename is_blue_type<T>::type type;
};

template< typename T >
struct is_pointer_to_blue
{
	typedef
		typename std::integral_constant<
			bool,
			std::is_base_of<
				IRoot, 
				typename std::remove_pointer< T >::type
			>::value 
			&& std::is_pointer<T>::value
		>::type
		type;
};

template< typename T >
struct is_pointer_to_pointer_to_blue
{
	typedef
		typename std::integral_constant<
			bool,
			std::is_base_of<
				IRoot, 
				typename std::remove_pointer< 
					typename std::remove_pointer< T >::type>::type
			>::value
			&& std::is_pointer<typename std::remove_pointer< T >::type>::value
		>::type
		type;
};

template< typename T >
struct is_char_array
{
	typedef
		typename std::integral_constant<
			bool,
			std::is_array<T>::value 
			&& std::is_same<
				typename std::remove_extent<
					typename remove_const_and_reference<T>::type
				>::type, 
				char
			>::value
		>::type
		type;
};

//////////////////////////////////////////////////////////////////////////
// Blue Type Trait Functions
//////////////////////////////////////////////////////////////////////////

// These are extended to handle the list and pointer types in blue

template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl3( const T&, std::false_type isPointerToBlueObject )
{
	// If you get a cryptic template compilation error that points to the
	// line below, it is probably because you are using MAP_ATTRIBUTE on
	// an unsupported type. This could be the result of exposing a variable
	// of a type that has only been forward-declared and isn't fully
	// known. In particular, Blue objects and interfaces must be fully
	// declared (include the header file) so the compiler can figure out
	// that they inherit from IRoot.
	return BlueTypeTraits<T>::VARTYPE_VALUE;
}


// For raw pointers to blue objects to be able to replace MAPIROOTPTR macro.
// Used alot in the UI framework
template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl3( const T&, std::true_type isPointerToBlueObject )
{
	return Be::IROOTPTR;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl2( const T& v, std::false_type isCharacterArray )
{
	return GetVarTypeForVariableImpl3( v, typename is_pointer_to_blue<T>::type() );
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl2( const T&, std::true_type isCharacterArray )
{
	return Be::CHARARRAY;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl( const T&, std::true_type isEnum )
{
	return Be::LONG;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableImpl( const T& v, std::false_type isEnum )
{
	return GetVarTypeForVariableImpl2( v, typename is_char_array<T>::type() );
}

template<typename T> static Be::VARTYPE GetVarTypeForVariable( const T& v )
{
	return GetVarTypeForVariableImpl( v, typename std::is_enum<T>::type() );
}

// This override is for parent-locked objects, not blue pointers
// for P<classname>
template<typename T> static Be::VARTYPE GetVarTypeForVariable( const RootParentLock<T>& )
{
	return Be::IROOT;
}
// for O<classname>
template<typename T> static Be::VARTYPE GetVarTypeForVariable( const RootRefLock<T>& )
{
	return Be::IROOT;
}
// for C<classname>
template<typename T> static Be::VARTYPE GetVarTypeForVariable( const RootNoLock<T>& )
{
	return Be::IROOT;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableWithChooserImpl( const T&, std::true_type isEnum )
{
	return Be::LONG;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableWithChooserImpl( const T& v, std::false_type isEnum )
{
	// If you get a cryptic template compilation error that points to the
	// line below, it is probably because you are using MAP_ATTRIBUTE on
	// an unsupported type. This could be the result of exposing a variable
	// of a type that has only been forward-declared and isn't fully
	// known. In particular, Blue objects and interfaces must be fully
	// declared (include the header file) so the compiler can figure out
	// that they inherit from IRoot.
	return BlueTypeTraits<T>::VARTYPE_VALUE;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariableWithChooser( const T& v )
{
	return GetVarTypeForVariableWithChooserImpl( v, typename std::is_enum<T>::type() );
}

// Since the template matching for the generic case will be taken in preference to type-casting, we
// need to provide implementations that catch the main blue pointer types

static Be::VARTYPE GetVarTypeForVariable( const IRootPtr& )
{
	return Be::IROOTPTR;
}

template<typename T> static Be::VARTYPE GetVarTypeForVariable( const BluePtr<T>& )
{
	return Be::IROOTPTR;
}

// Specialization for weak references
template<typename T> static Be::VARTYPE GetVarTypeForVariable( const BlueWeakRef<T>& )
{
	return Be::IROOTWEAKREF;
}

namespace BlueListUtils
{
class BlueListBase;
}

template<typename T> inline const Be::IID* GetBlueIIDBluePointerHelper( const T& t, std::true_type isRawBluePointer, std::true_type isBlueList )
{
	return nullptr;
}

template<typename T> inline const Be::IID* GetBlueIIDBluePointerHelper( const T& t, std::true_type isRawBluePointer, std::false_type isBlueList )
{
	return &BlueInterfaceIID<typename std::remove_pointer<T>::type>();
}

template<typename T> inline const Be::IID* GetBlueIIDHelper( const T& t, std::true_type isRawBluePointer )
{
	return GetBlueIIDBluePointerHelper( t, isRawBluePointer, typename std::is_base_of<BlueListUtils::BlueListBase, typename std::remove_pointer<T>::type>::type() );
}

template<typename T> inline const Be::IID* GetBlueIIDHelper( const T& t, std::false_type isRawBluePointer )
{
	return nullptr;
}


// Most types don't have a Blue interfaceID
template<typename T> inline const Be::IID* GetBlueIID( const T& t )
{
	return GetBlueIIDHelper( t, typename is_pointer_to_blue<T>::type() );
}

template<typename T> inline const Be::IID* GetBlueIID( const BluePtr<T>& )
{
	return &BlueInterfaceIID<T>();
}

// We provide explicit default IIDs for those 'vector types' that have built in blue exposure support
template<> inline const Be::IID* GetBlueIID( const Quaternion& )
{
	return &BlueRotationIID;
}


template<> inline const Be::IID* GetBlueIID( const Color& )
{
	return &BlueColorIID;
}

template<> inline const Be::IID* GetBlueIID( const Matrix& )
{
	return &BlueMatrixIID;
}

#endif // BlueTypeTraits_h