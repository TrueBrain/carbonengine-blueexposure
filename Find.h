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
#include "BlueSmartPy.h"

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
	IRoot* GetStepObject() const;

private:
	StepType m_stepType;
	StepValue m_value;
	IRoot* m_obj;
};

bool FindFirstRoute( IRoot* from, IRoot* to, std::vector<RouteStep>* result );
BluePy PyFindFirstRoute( IRoot* from, IRoot* to );

std::vector<IRootPtr> FindInterface( IRoot* obj, const char* iidName );

PyObject* PyFindImpl( IRoot* pThis, PyObject* args );

BluePy FindRoute( IRoot* from, IRoot* to );
bool FindReference( IRoot* from, IRoot* to );
BluePy FindAllReferences( IRoot* root );

#endif
