////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		April 2013
// Copyright:	CCP 2013
//

#include "include/StringConversions.h"

#ifndef _WIN32

#include <wchar.h>
#include <string.h>
#include "CcpCore/include/CCPMemory.h"

#ifdef __ANDROID__

size_t android_wcsrtombs( char* dest, const wchar_t** _src, size_t maxLen, mbstate_t* ps )
{
    const wchar_t* src = *_src;
    if( dest )
    {
        for( size_t i = 0; i < maxLen; ++i )
        {
            if( *src > 127 )
            {
                return -1;
            }
            *dest++ = *src;
            if( !*src )
            {
                return i;
            }
            ++src;
        }
        return maxLen;
    }
    else
    {
        for( size_t i = 0; i < maxLen; ++i )
        {
            if( *src > 127 )
            {
                return -1;
            }
            if( !*src )
            {
                return i;
            }
            ++src;
        }
        return maxLen;
    }
}

size_t android_mbsrtowcs( wchar_t* dest, const char** _src, size_t maxLen, mbstate_t* ps )
{
    const char* src = *_src;
    if( dest )
    {
        for( size_t i = 0; i < maxLen; ++i )
        {
            *dest++ = *src;
            if( !*src )
            {
                return i;
            }
            ++src;
        }
        return maxLen;
    }
    else
    {
        for( size_t i = 0; i < maxLen; ++i )
        {
            if( !*src )
            {
                return i;
            }
            ++src;
        }
        return maxLen;
    }
}

#define mbsrtowcs android_mbsrtowcs
#define wcsrtombs android_wcsrtombs

#endif

BlueConvertWideToAscii::BlueConvertWideToAscii( const wchar_t* src ) : m_converted( nullptr )
{
	Init( src );
}

BlueConvertWideToAscii::~BlueConvertWideToAscii()
{
	if( m_converted != m_buffer )
	{
		CCP_FREE( (void*)m_converted );
	}
}

void BlueConvertWideToAscii::Init( const wchar_t* src )
{
	size_t srcLen = wcslen( src );
	size_t sizeNeeded = wcsrtombs( nullptr, &src, srcLen, nullptr );
	if( sizeNeeded == (size_t)-1 )
	{
		m_converted = m_buffer;
		strcpy( m_converted, "Invalid string" );
		return;
	}

	if( sizeNeeded >= BUFFER_SIZE )
	{
		m_converted = (char*)CCP_MALLOC( "ConvertWideToAscii", sizeNeeded + 1 );
	}
	else
	{
		m_converted = m_buffer;
	}
	wcsrtombs( m_converted, &src, srcLen, nullptr );
	m_converted[sizeNeeded] = 0;
}

BlueConvertAsciiToWide::BlueConvertAsciiToWide( const char* src ) : m_converted( nullptr )
{
	Init( src );
}

BlueConvertAsciiToWide::~BlueConvertAsciiToWide()
{
	if( m_converted != m_buffer )
	{
		CCP_FREE( (void*)m_converted );
	}
}

void BlueConvertAsciiToWide::Init( const char* src )
{
	size_t srcLen = strlen( src );
	size_t sizeNeeded = mbsrtowcs( nullptr, &src, srcLen, nullptr );
	if( sizeNeeded == (size_t)-1 )
	{
		m_converted = m_buffer;
		wcscpy( m_converted, L"Invalid string" );
		return;
	}

	if( sizeNeeded >= BUFFER_SIZE )
	{
		m_converted = (wchar_t*)CCP_MALLOC( "ConvertAsciiToWide", (sizeNeeded + 1) * sizeof( wchar_t ) );
	}
	else
	{
		m_converted = m_buffer;
	}
	mbsrtowcs( m_converted, &src, srcLen, nullptr );
	m_converted[sizeNeeded] = 0;
}

#endif
