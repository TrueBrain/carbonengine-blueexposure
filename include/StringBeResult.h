////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		July 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef StringBeResult_h
#define StringBeResult_h

namespace Be
{
	template<>
	struct Result<std::string>
	{
		Result() {}
		Result( const std::string& s ) : value( s ) {}
		Result( const Result& other ) : value( other.value ) {}
		std::string value;
	};

	template <>
	inline bool IsSuccess<std::string>( const Result<std::string>& result )
	{
		return result.value.empty();
	}

	template <>
	inline const char* GetErrorMessage<std::string>( const Result<std::string>& result )
	{
		return result.value.c_str();
	}

#if BLUE_WITH_PYTHON
	template <>
	inline PyObject* GetException<std::string>( const Result<std::string>& result )
	{
		return PyExc_RuntimeError;
	}
#endif

}

#endif // StringBeResult_h