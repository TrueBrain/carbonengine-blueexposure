////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#pragma once
#ifndef Find_h
#define Find_h

PyObject* PyFindImpl( IRoot* pThis, PyObject* args );

PyObject* FindRoute( IRoot* from, IRoot* to );

PyObject *FindInterface( IRoot *obj, const char* iidName );

#endif
