////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		February 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BoolBeResult_h
#define BoolBeResult_h

template<>
struct Be::Result<bool>
{
	Result() : value( false ) {}
	Result( bool b ) : value( b ) {}
	bool value;
};

template <>
inline bool BeIsSuccess<bool>( const Be::Result<bool>& result )
{
	return result.value;
}

template <>
inline const char* BeGetErrorMessage<bool>( const Be::Result<bool>& result )
{
	return "Failed!";
}

#if BLUE_WITH_PYTHON
template <>
inline PyObject* BeGetException<bool>( const Be::Result<bool>& result )
{
	return PyExc_RuntimeError;
}
#endif


#endif // BoolBeResult_h
