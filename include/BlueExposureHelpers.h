////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		February 2013
// Copyright:	CCP 2013
//

#pragma once
#ifndef BlueExposureHelpers_h
#define BlueExposureHelpers_h

#include "BlueFunctionTraits.h"
#include "BlueExtractArgument.h"
#include "BlueExposureMacros.h"

#if BLUE_WITH_LUA
#include "BlueExposureHelpersLua.h"
#elif BLUE_WITH_PYTHON
#include "BlueExposureHelpersPython.h"
#endif

// Get the VARTYPE for the return value of the property getter function.
// This is a specialization for the case where the return type is a Be::Result,
// meaning we use the type of the first argument - expected to be a reference.
template< typename Class, typename FunctionType, FunctionType getter>
Be::VARTYPE BlueGetPropertyTypeBeResult( std::true_type isBeResult )
{
	typedef 
		typename remove_const_and_reference<
			typename function_traits<FunctionType>::arg1_type
		>::type
		PropertyType;

	PropertyType* var = nullptr;
	return GetVarTypeForVariable( *var );
}

// Get the VARTYPE for the return value of the property getter function.
// This is a specialization for the case where the return type is not a Be::Result,
// meaning we use the return type of the function.
template< typename Class, typename FunctionType, FunctionType getter>
Be::VARTYPE BlueGetPropertyTypeBeResult( std::false_type isBeResult )
{
	typedef 
		typename function_traits<FunctionType>::return_type 
		PropertyType;

	typedef
		typename remove_const_and_reference<PropertyType>::type
		StrippedPropertyType;

	StrippedPropertyType* var = nullptr;

	// This looks like we're dereferencing a nullptr, but the function doesn't
	// actually do anything with it. We're really just doing this to route it
	// to the right template specialization.
	return GetVarTypeForVariable( *var );
}

// Get the VARTYPE for the return value of the property getter function.
// Note that a Be::Result return type implies that we look at the type
// of the first argument and use that instead.
template< typename Class, typename FunctionType, FunctionType getter>
Be::VARTYPE BlueGetPropertyType()
{
	typedef
		typename function_traits<FunctionType>::return_type
		return_type;

	return BlueGetPropertyTypeBeResult<Class, FunctionType, getter>( typename is_be_result<return_type>::type() );
}


#endif // BlueExposureHelpers_h