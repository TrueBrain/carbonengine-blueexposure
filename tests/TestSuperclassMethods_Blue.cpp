#include "TestSuperclassMethods.h"

BLUE_DEFINE( TestSuperclassMethods );

const Be::ClassInfo* TestSuperclassMethods::ExposeToBlue()
{
	EXPOSURE_BEGIN( TestSuperclassMethods, "TestSuperclassMethods is used to help test Blue exposure" )
		MAP_INTERFACE( TestSuperclassMethods )
	EXPOSURE_CHAINTO( TestMethods )
}
