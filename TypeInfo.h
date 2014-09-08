////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#pragma once
#ifndef TypeInfo_h
#define TypeInfo_h

#include "include/BlueTypes.h"

#if BLUE_WITH_PYTHON
PyObject* PyGetTypeInfo( const Be::ClassInfo* info, long flags = 0 );
PyObject* PyGetChooserInfo( const Be::VarEntry& entry );
#endif

#endif // TypeInfo_h