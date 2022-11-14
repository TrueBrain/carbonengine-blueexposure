////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#pragma once
#ifndef Find_h
#define Find_h

#include "BlueTypes.h"

class RouteStep
{
public:
	enum StepType
	{
		ATTRIBUTE,
		INDEX,
		KEY
	};
	struct AttributeRef
	{
		const Be::ClassInfo* type;
		const Be::VarEntry* entry;
		ptrdiff_t offset;
	};
	struct StepValue
	{
		std::string key;
		ssize_t index;
		AttributeRef attribute;
	};

	RouteStep();
	RouteStep( StepType stepType, const StepValue& value, IRoot* root );
	RouteStep( const RouteStep& ref );
	~RouteStep();
	
	IRoot* GetNextObject( IRoot* parent=nullptr );

private:
	StepType m_stepType;
	StepValue m_value;
	IRoot* m_obj;
};

bool FindFirstRoute( IRoot* from, IRoot* to, std::vector<RouteStep>* result );
std::vector<IRoot*> FindInterface( IRoot* obj, const char* iidName );

PyObject* PyFindImpl( IRoot* pThis, PyObject* args );

PyObject* PyFindRoute( PyObject* pThis, PyObject* args );

PyObject* PyFindInterface( PyObject* pThis, PyObject* args );

bool FindReference( IRoot* from, IRoot* to );
PyObject* FindAllReferences( IRoot* root );

#endif
