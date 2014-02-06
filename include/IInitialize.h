////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		January 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef IInitialize_h
#define IInitialize_h

BLUE_INTERFACE( IInitialize ) : public IRoot
{
	virtual bool Initialize() = 0;
};

#endif // IInitialize_h