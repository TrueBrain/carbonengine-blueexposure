
#ifndef _BLUEVARIABLE_H_
#define _BLUEVARIABLE_H_

#include "include/BlueTypes.h"
#include "include/BlueExtractArgument.h"
#include "include/IList.h"
#include "include/IBlueDict.h"
#include "include/IBlueStructureList.h"
#if BLUE_WITH_PYTHON
#include "include/BluePythonObject.h"
#endif
#include "include/BlueScriptCallback.h"

#include "Copier.h"


struct BlueVariable
{
	bool (*AreEqual)(const Be::VarEntry* entry, const Be::Var* a, const Be::Var* b);
	#if BLUE_WITH_PYTHON
	bool (*ConvertFromPython)(const Be::VarEntry* var, Be::Var* value, PyObject* v);
	void (*ConvertToPython)(const Be::VarEntry* entry, const Be::Var* value, PyObject*& ret);
	#endif
	bool (*Copy)(const Be::VarEntry* entry, Be::Var* dst, Be::Var* src, Copier* copier);
};

void InitializeBlueVariables();
BlueVariable* GetBlueVariableFromVarType( Be::VARTYPE type );


#endif