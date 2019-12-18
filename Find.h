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

PyObject* PyFindRoute( PyObject* pThis, PyObject* args );

PyObject* PyFindInterface( PyObject* pThis, PyObject* args );

#endif
