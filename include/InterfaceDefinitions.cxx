////////////////////////////////////////////////////////////////////////////////
//
// Creator:     Snorri Sturluson
// Created:     January 2013
// Copyright:   CCP 2013
//

#include "BlueExposureMacros.h"
BLUE_DEFINE_INTERFACE_IMPL( IRoot );
BLUE_DEFINE_INTERFACE( INotify );
BLUE_DEFINE_INTERFACE( IInitialize );
BLUE_DEFINE_INTERFACE( IBlueClasses );
BLUE_DEFINE_INTERFACE( IList );
BLUE_DEFINE_INTERFACE( IBlueDict );
BLUE_DEFINE_INTERFACE( IBlueStructureList );
BLUE_DEFINE_INTERFACE( IListNotify );
BLUE_DEFINE_INTERFACE( IWeakObject );
BLUE_DEFINE_INTERFACE( IPythonMethods );
BLUE_DEFINE_INTERFACE( IPythonNumeric );
BLUE_DEFINE_INTERFACE( ICopier );
BLUE_DEFINE_INTERFACE( ICopierCustomAssignment );
BLUE_DEFINE_INTERFACE( ICustomPersist );

BLUE_DEFINE_CLSID( "blue", List );
BLUE_DEFINE_CLSID( "blue", Dict );
BLUE_DEFINE_CLSID( "blue", StructureList );
