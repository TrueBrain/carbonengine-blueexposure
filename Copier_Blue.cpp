////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		October 2012
// Copyright:	CCP 2012
//

#include "Copier.h"
#include "include/BlueExposureMacros.h"

BLUE_DEFINE_NONEXPOSED( Copier );

const Be::ClassInfo* Copier::ExposeToBlue()
{
	EXPOSURE_BEGIN( Copier, "A Copier copies Blue objects" )
		MAP_INTERFACE(ICopier)
	EXPOSURE_END()
}