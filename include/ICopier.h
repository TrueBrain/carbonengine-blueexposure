////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#pragma once
#ifndef ICopier_h
#define ICopier_h

#include "BlueTypes.h"

BLUE_INTERFACE(ICopier) : public IRoot
{
	virtual bool CopyTo(IRoot* source, IRoot** dest) = 0;
};

// --------------------------------------------------------------------------------------
//  Description:
//    Provides the ability to extend the behavior of copying of Blue objects, beyond
//    the built in Be::VarTypes. This is used for copying of Dicts, Lists, etc. as well
//    as other information that is not intended to be persisted.
// --------------------------------------------------------------------------------------
BLUE_INTERFACE(ICopierCustomAssignment) : public IRoot
{
	// Custom assignment function to a pre-created copy
	// 'other' is guaranteed to be of the same type as your class
	virtual bool AssignTo(
		ICopierCustomAssignment* other,
		ICopier* copier
		) = 0;
};

#endif
