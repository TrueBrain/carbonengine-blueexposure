////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		February 2013
// Copyright:	CCP 2013
//

#include "TestReferences.h"

BLUE_DEFINE( TestReferences );

const Be::ClassInfo* TestReferences::ExposeToBlue()
{
	EXPOSURE_BEGIN( TestReferences, "" )
		MAP_ATTRIBUTE
		(
			"weakRef",
			m_weakRef,
			"",
			Be::READWRITE | Be::PERSIST
		)
		MAP_ATTRIBUTE
		(
			"strongRef",
			m_strongRef,
			"",
			Be::READWRITE | Be::PERSIST
		)
	EXPOSURE_END()
}
