////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		August 2010
// Copyright:	CCP 2010
//

#pragma once
#ifndef IBlueDict_h
#define IBlueDict_h

#include "BlueTypes.h"

BLUE_DECLARE_INTERFACE( IBlueDict );

BLUE_INTERFACE( IBlueDict ) : public IRoot
{
	virtual size_t GetLength() const = 0;
	virtual IRoot* Subscript( const char* key ) const = 0;
	virtual bool AssignSubscript( const char* key, IRoot* value ) = 0;
	virtual const char* GetKey( size_t index ) = 0;
};

#endif // IBlueDict_h