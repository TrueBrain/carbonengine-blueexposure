////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		April 2013
// Copyright:	CCP 2013
//
// This file provides the CW2A and CA2W macros. On Windows we simply pull in
// the ATL macros - on other platforms we provide our own implemented with
// wcsrtombs and mbsrtowcs.
//

#pragma once
#ifndef StringConversions_h
#define StringConversions_h

#ifdef _WIN32
#include <atlbase.h>
#else

#include "CcpCore/include/CcpMacros.h"
#include <stdint.h>

class BLUEIMPORT BlueConvertWideToAscii
{
public:
	BlueConvertWideToAscii( const wchar_t* src );
	~BlueConvertWideToAscii();

	operator const char*() const
	{
		return m_converted;
	}

private:
	void Init( const wchar_t* src );

private:
	static const uint32_t BUFFER_SIZE = 128;
	char* m_converted;
	char m_buffer[BUFFER_SIZE];
};


class BLUEIMPORT BlueConvertAsciiToWide
{
public:
	BlueConvertAsciiToWide( const char* src );
	~BlueConvertAsciiToWide();

	operator const wchar_t*() const
	{
		return m_converted;
	}

private:
	void Init( const char* src );

private:
	static const uint32_t BUFFER_SIZE = 128;
	wchar_t* m_converted;
	wchar_t m_buffer[BUFFER_SIZE];
};

typedef BlueConvertWideToAscii CW2A;
typedef BlueConvertAsciiToWide CA2W;

#endif


#endif // StringConversions_h