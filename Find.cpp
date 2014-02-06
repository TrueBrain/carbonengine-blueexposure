////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#if BLUE_WITH_PYTHON

#include "include/IList.h"
#include "include/IBlueDict.h"
#include "include/BlueSmartPy.h"

#include <vector>
#include <unordered_set>
#include "include/BluePythonObject.h"
#include "include/BlueUtil.h"

//First a small structure to use in the recursion stack
struct StackEntry {
	StackEntry(IRoot *o, int l, bool c) : 
		obj(o->GetRootObject()), level(l), check(c) {}
	IRoot * obj;	//pointer to the IRoot
	int level;		//Recursion level of this object
	bool check;		//Should we check if we have seen it before (i.e. is this
					//a proper IRoot pointer and not an auto variable?)
};

typedef std::vector<StackEntry> objectstack_t;
typedef std::vector<Be::Clsid> idvector_t;
typedef std::unordered_set<IRoot *> rootset_t;
typedef std::vector<IRoot *> rootstack_t;


PyObject *FindSingle(IRoot *obj, const idvector_t &ids, int maxLevel, bool prune);
PyObject *FindMultiple(IRoot *obj, const idvector_t &ids, int maxLevel, bool prune, int nParents);

static bool IsMatch(IRoot *obj, const idvector_t &clsids);
static void PushChildren(IRoot *obj, int level, objectstack_t &stack);

PyObject* PyFindImpl( IRoot* pThis, PyObject* args )
{
	PyObject *clsidobj;
	int maxLevel = -1;
	int bPrune = 0;
	int nParents = -1;
	if (!PyArg_ParseTuple(args, "O|iii", &clsidobj, &maxLevel, &bPrune, &nParents))
		return NULL;

	// parse first argument.  Either a string, or a sequence of strings.
	idvector_t clsids;
	if (PyString_Check(clsidobj)) {
		Be::Clsid clsid;
		if (!clsid.InitFromString(PyString_AsString(clsidobj)))
			return 0;
		clsids.push_back(clsid);
	} else {
		BluePySeq seq(BluePy(clsidobj, true));
		if (!seq.Check()) {
			PyErr_SetString(PyExc_TypeError, "first argument must be string or sequence of strings");
			return 0;
		}
		Py_ssize_t s = seq.Size();
		for(Py_ssize_t i = 0; i< s; i++) {
			BluePyStr str = seq.Get(i);
			if (!str.Check()) {
				PyErr_SetString(PyExc_TypeError, "first argument must be string or sequence of strings");
				return 0;
			}
			Be::Clsid clsid;
			if (!clsid.InitFromString(str.Str()))
				return 0;
			clsids.push_back(clsid);
		}
	}

	if (nParents < 0)
		return FindSingle( pThis, clsids, maxLevel, !!bPrune );
	else
		return FindMultiple( pThis, clsids, maxLevel, !!bPrune, nParents );
}

PyObject *FindSingle(IRoot *obj, const idvector_t &ids, int maxLevel, bool prune)
{
	objectstack_t stack;
	rootset_t seen;

	PyObject *result = PyList_New(0);
	if (!result)
		return 0;

	//Prime the stack and start
	stack.push_back(StackEntry(obj, 0, true));  // 'true' since we must assume it's not an autovar.
	while (!stack.empty())
	{
		StackEntry e = stack.back();
		stack.pop_back();

		obj = e.obj;
		if (e.check) {
			// We need to check this, if we've seen it before
			std::pair<rootset_t::iterator, bool> res = seen.insert(obj);
			if (!res.second)
				continue;
		}

		// do we have a winner here?  See if the class matches what we are looking for.
		if (IsMatch(obj, ids)) {
			PyObject *item = BlueWrapObjectForPython(obj);
			if (!item || PyList_Append(result, item)) {
				Py_XDECREF(item);
				Py_DECREF(result);
				return 0;
			}
			Py_DECREF(item);

			if (prune)
				continue; //don't look below a match
		}

		//now, depending on the level, we go on and search the children
		if (maxLevel >= 0 && e.level >= maxLevel)
			continue;  //no, we don't go any deeper.

		PushChildren(obj, e.level, stack);
	}
	return result;
}


// A stack class to manage the recursion data in the PyFindMultiple
class RealStack
{
public:
	bool IsOnStack(IRoot *obj) const {
		return mSet.find(obj) != mSet.end();
	}

	size_t GetSize() const {return mStack.size(); }

	//pop the stack to a certain size, indicating that the next Push will be
	//at level 'size'
	void PopTo(int size)
	{
		CCP_ASSERT((size_t)size <= GetSize());
		while((size_t)size<GetSize())
			Pop();
	}

	//push an object onto the stack.  It is also added into the lookup set of
	//objects present.  Note that multiple recurrences are not allowed!
	void Push(IRoot *obj) {
		CCP_ASSERT(mStack.size() == mSet.size());
		mStack.push_back(obj);
		std::pair<rootset_t::iterator, bool> res = mSet.insert(obj);
		CCP_ASSERT(res.second); //must be the first instance
	}

	//pop an object from the stack.  It is also removed from the set of objects
	//present.
	IRoot *Pop() {
		CCP_ASSERT(mStack.size() && mStack.size() == mSet.size());
		IRoot *r = mStack.back();
		mStack.pop_back();
		size_t n = mSet.erase(r);
		CCP_ASSERT(n==1);
		return r;
	}

	//Return up to 'max' items of the current stack int top-bottom order
	PyObject *GetChain(int max) {
		PyObject *r = PyList_New(0);
		if (!r) return 0;
		//max is the number of parents.  max of 0 means just the item.
		for (int i = 0; (max<0 || i<max+1) && GetSize()-i>0; i++) {
			IRoot *obj = mStack[mStack.size()-i-1];
			PyObject *pyobj = BlueWrapObjectForPython(obj);
			if (!pyobj || PyList_Append(r, pyobj)) {
				Py_XDECREF(pyobj);
				Py_DECREF(r);
				return 0;
			}
			Py_DECREF(pyobj);
		}
		return r;
	}

private:
	rootstack_t mStack;
	rootset_t mSet;
};


PyObject *FindMultiple(IRoot *obj, const idvector_t &ids, int maxLevel, bool prune, int nParents)
{
	objectstack_t stack; //the operations stack
	RealStack rstack; //the invocation stack

	PyObject *result = PyList_New(0);
	if (!result)
		return 0;

	//Prime the stack and start
	stack.push_back(StackEntry(obj, 0, true));  // 'true' since we must assume it's not an autovar.
	while (!stack.empty())
	{
		StackEntry e = stack.back();
		stack.pop_back();

		obj = e.obj;
		rstack.PopTo(e.level);
		//do we have this object on the stack already
		if (rstack.IsOnStack(obj))
			continue;  //ok break the recursion
		rstack.Push(obj);

		// do we have a winner here?  See if the class matches what we are looking for.
		if (IsMatch(obj, ids)) {
			PyObject *item = rstack.GetChain(nParents);
			if (!item || PyList_Append(result, item)) {
				Py_XDECREF(item);
				Py_DECREF(result);
				return 0;
			}
			Py_DECREF(item);
			if (prune)
				continue;
		}

		//now, depending on the level, we go on and search the children
		if (maxLevel >= 0 && e.level >= maxLevel)
			continue;  //no, we don't go any deeper.

		PushChildren(obj, e.level, stack);
	}
	return result;
}



static bool IsMatch(IRoot *obj, const idvector_t &clsids)
{
	// do we have a winner here?  See if the class matches what we are looking for.
	const Be::ClassInfo* type = obj->ClassType();
	for (const Be::ClassInfo* other = type; other; other = other->mParentClassInfo)
		for (idvector_t::const_iterator it = clsids.begin(); it!= clsids.end() ; ++it)
			if ((*it).IsEqual(*other->mClassId))
				return true;			
	return false;
}

static void PushChildren(IRoot *obj, int level, objectstack_t &stack)
{
	const Be::ClassInfo* type = obj->ClassType();
	// go thru members, see if any is IROOT or IROOTPTR
	for (ssize_t xtraoffs = 0; type; xtraoffs += type->mOffsetToParent, type = type->mParentClassInfo)
	{
		for (const Be::VarEntry *entry = type->mMemberTable; entry->mName; entry++)
		{
			if ((entry->mEditFlags & Be::PERSIST) &&
				(entry->mType == Be::IROOT || entry->mType == Be::IROOTPTR))
			{
				Be::Var* value = BLUEMAPMEMBEROFFSET(obj, entry, type, xtraoffs);
				IRoot *child;
				bool check;
				if (entry->mType == Be::IROOTPTR) {
					child = value->mIRootPtr;
					if (!child)
						continue;
					check = true;
				} else { //IRoot, autovar.
					child = reinterpret_cast<IRoot*>( value );
					check = false;
				}
				stack.push_back(StackEntry(child, level+1, check));
			}
		}
	}

	//if we are a list, insert list members too.
	IListPtr list( BlueCastPtr( obj ) );
	if (list)
	{
		ListInfo info;
		list->GetInfo(&info);
		for (long i = 0; i < list->GetSize(); i++) {
			IRoot *item = list->GetAt(i);
			if (item)
				stack.push_back(StackEntry(item, level+1, true));
		}
	}

	IBlueDictPtr dict( BlueCastPtr( obj ) );
	if( dict )
	{
		size_t n = dict->GetLength();
		for( size_t i = 0; i < n; ++i )
		{
			const char* key = dict->GetKey( i );
			IRoot* item = dict->Subscript( key );
			if( item )
			{
				stack.push_back( StackEntry( item, level+1, true ) );
			}
		}
	}
}


#endif
