////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		December 2012
// Copyright:	CCP 2012
//

#if BLUE_WITH_PYTHON

#include "Find.h"
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
static bool IsMatch( IRoot *obj, const Be::IID& iid );
static void PushChildren(IRoot *obj, int level, objectstack_t &stack, long flagFilter );

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

		PushChildren(obj, e.level, stack, Be::PERSIST);
	}
	return result;
}

std::vector<IRoot*> FindInterface( IRoot *obj, const char* iidName )
{
	Be::IID iid( iidName );

	objectstack_t stack;
	rootset_t seen;

	std::vector<IRoot*> result;
	
	if( !obj )
	{
		return result;
	}

	//Prime the stack and start
	stack.push_back( StackEntry( obj, 0, true ) );  // 'true' since we must assume it's not an autovar.
	while( !stack.empty() )
	{
		StackEntry e = stack.back();
		stack.pop_back();

		obj = e.obj;
		if( e.check )
		{
			// We need to check this, if we've seen it before
			std::pair<rootset_t::iterator, bool> res = seen.insert( obj );
			if( !res.second )
				continue;
		}

		// do we have a winner here?  See if the class matches what we are looking for.
		if( IsMatch( obj, iid ) )
		{
			result.push_back( obj );
		}

		PushChildren( obj, e.level, stack, 0 );
	}
	return result;
}

PyObject* PyFindInterface( PyObject* pThis, PyObject* args )
{
	PyObject* pObj;
	const char* iidName;
	if( !PyArg_ParseTuple( args, "Os", &pObj, &iidName ) )
	{
		return NULL;
	}

	IRoot* obj = BlueUnwrapObjectFromPython( pObj );
	if ( !obj )
	{
		PyErr_Format( PyExc_TypeError, "Argument 'obj' is not of type IRoot." );
		return NULL;
	}

	PyObject* result = PyList_New( 0 );
	if( !result )
	{
		return 0;
	}
	auto interfaces = FindInterface( obj, iidName );
	for( auto it = begin( interfaces ); it != end( interfaces ); ++it )
	{
		PyObject* item = BlueWrapObjectForPython( *it );
		if( !item || PyList_Append( result, item ) )
		{
			Py_XDECREF( item );
			Py_DECREF( result );
			return 0;
		}
		Py_DECREF( item );
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

		PushChildren(obj, e.level, stack, Be::PERSIST);
	}
	return result;
}



static bool IsMatch( IRoot *obj, const idvector_t &clsids )
{
	// do we have a winner here?  See if the class matches what we are looking for.
	const Be::ClassInfo* type = obj->ClassType();
	for( const Be::ClassInfo* other = type; other; other = other->mParentClassInfo )
		for( idvector_t::const_iterator it = clsids.begin(); it != clsids.end(); ++it )
			if( ( *it ).IsEqual( *other->mClassId ) )
				return true;
	return false;
}

static bool IsMatch( IRoot *obj, const Be::IID& iid )
{
	auto type = obj->ClassType();
	for( auto other = type; other; other = other->mParentClassInfo )
	{
		for( auto entry = type->mInterfaceTable; entry->mIID; entry++ )
		{
			if( entry->mIID->IsEqual( iid ) )
			{
				return true;
			}
		}
	}
	return false;
}

static void PushChildren(IRoot *obj, int level, objectstack_t &stack, long flagFilter)
{
	const Be::ClassInfo* type = obj->ClassType();
	// go thru members, see if any is IROOT or IROOTPTR
	for (ssize_t xtraoffs = 0; type; xtraoffs += type->mOffsetToParent, type = type->mParentClassInfo)
	{
		for (const Be::VarEntry *entry = type->mMemberTable; entry->mName; entry++)
		{
			if ((!flagFilter || (entry->mEditFlags & flagFilter)) && entry->mSize &&
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



namespace
{

class RouteItem
{
public:
	enum ItemType
	{
		ATTRIBUTE,
		INDEX,
	};

	RouteItem( IRoot* parent )
		:m_parent( parent ),
		m_type( parent->ClassType() ),
		m_entry( nullptr ),
		m_offset( 0 ),
		m_listIndex( -1 ),
		m_dictIndex( -1 ),
		m_value( nullptr ),
		m_list( nullptr ),
		m_dict( nullptr )
	{
		IListPtr list( BlueCastPtr( m_parent ) );
		m_list = list.p;
		IBlueDictPtr dict( BlueCastPtr( m_parent ) );
		m_dict = dict.p;
	}

	IRoot* Value() const
	{
		return m_value ? m_value->GetRootObject() : nullptr;
	}

	PyObject* GetPathItem() const
	{
		if( !m_value )
		{
			return nullptr;
		}
		if( m_dictIndex >= 0 )
		{
			return Py_BuildValue( "(Nis)", BlueWrapObjectForPython( m_parent ), INDEX, m_dict->GetKey( m_dictIndex ) );
		}
		else if( m_listIndex >= 0 )
		{
			return Py_BuildValue( "(Nii)", BlueWrapObjectForPython( m_parent ), INDEX, m_listIndex );
		}
		else
		{
			return Py_BuildValue( "(Nis)", BlueWrapObjectForPython( m_parent ), ATTRIBUTE, m_entry->mName );
		}
	}

	RouteStep GetRouteStep()
	{
		if( m_dictIndex >= 0 )
		{
			RouteStep::StepValue value = { std::string( m_dict->GetKey( m_dictIndex ) ), 0, RouteStep::AttributeRef() };
			return RouteStep(RouteStep::StepType::KEY, value, m_parent);
		}
		else if( m_listIndex >= 0 )
		{
			RouteStep::StepValue value = { std::string(), m_listIndex, RouteStep::AttributeRef() };
			return RouteStep(RouteStep::StepType::INDEX, value, m_parent);
		}
		else
		{
			RouteStep::AttributeRef attrRef = { m_type, m_entry, m_offset };
			RouteStep::StepValue value = { std::string(), 0, attrRef };
			return RouteStep( RouteStep::StepType::ATTRIBUTE, value, m_parent );
		}
	}

	bool Next()
	{
		m_value = nullptr;
		if( NextAttribute() )
		{
			return true;
		}
		if( NextListItem() )
		{
			return true;
		}
		if( NextDictItem() )
		{
			return true;
		}
		return false;
	}
private:
	bool NextAttribute()
	{
		for( ; m_type; m_offset += m_type->mOffsetToParent, m_type = m_type->mParentClassInfo )
		{
			if( !m_entry )
			{
				m_entry = m_type->mMemberTable;
			}
			else
			{
				++m_entry;
			}
			for( ; m_entry->mName; m_entry++ )
			{
				if( m_entry->mType == Be::IROOT || m_entry->mType == Be::IROOTPTR )
				{
					if( !m_entry->mGetProperty )
					{
						Be::Var* value = BLUEMAPMEMBEROFFSET( m_parent, m_entry, m_type, m_offset );
						if( m_entry->mType == Be::IROOTPTR) 
						{
							m_value = value->mIRootPtr;
						} 
						else 
						{ 
							m_value = reinterpret_cast<IRoot*>( value );
						}
						if( m_value )
						{
							return true;
						}
					}
				}
			}
			m_entry = nullptr;
		}
		return false;
	}

	bool NextListItem()
	{
		if( m_list )
		{
			for( ++m_listIndex; m_listIndex < m_list->GetSize(); ++m_listIndex ) 
			{
				m_value = m_list->GetAt( m_listIndex );
				if (m_value)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool NextDictItem()
	{
		if( m_dict )
		{
			ssize_t n = ssize_t( m_dict->GetLength() );
			for( ++m_dictIndex; m_dictIndex < n; ++m_dictIndex )
			{
				const char* key = m_dict->GetKey( m_dictIndex );
				m_value = m_dict->Subscript( key );
				if( m_value )
				{
					return true;
				}
			}
		}
		return false;
	}

	IRoot* m_parent;
	const Be::ClassInfo* m_type;
	const Be::VarEntry* m_entry;
	ptrdiff_t m_offset;
	ssize_t m_listIndex;
	ssize_t m_dictIndex;
	IRoot* m_value;
	IList* m_list;
	IBlueDict* m_dict;
};

}

RouteStep::RouteStep() :
	m_stepType( StepType::INDEX ),
	m_value(),
	m_obj( nullptr )
{
}

RouteStep::RouteStep( StepType stepType, const StepValue& value, IRoot* root ) :
	m_stepType( stepType ),
	m_obj( root )
{
	switch( stepType )
	{
	case RouteStep::StepType::ATTRIBUTE: 
		m_value.attribute = value.attribute;
		break;
	case RouteStep::StepType::INDEX: 
		m_value.index = value.index;
		break;
	case RouteStep::StepType::KEY: 
		m_value.key = value.key;
		break;
	default:
		break;
	}
}


RouteStep::RouteStep( const RouteStep& ref ) :
	RouteStep( ref.m_stepType, ref.m_value, ref.m_obj )
{
}

RouteStep::~RouteStep()
{
}

IRoot* RouteStep::GetNextObject( IRoot* parent )
{
	if( !parent )
	{
		parent = m_obj;
	}
	switch( m_stepType )
	{
	case StepType::ATTRIBUTE:
		{
			const Be::ClassInfo* type = parent->ClassType();
			for( const Be::ClassInfo* current = type; current; current = current->mParentClassInfo )
			{
				if( current == m_value.attribute.type )
				{
					Be::Var* var = BLUEMAPMEMBEROFFSET( parent, m_value.attribute.entry, m_value.attribute.type, m_value.attribute.offset );
					if( m_value.attribute.entry->mType == Be::IROOTPTR )
					{
						return var->mIRootPtr;
					}
					else
					{
						return reinterpret_cast<IRoot*>( var );
					}
					break;
				}
			}
		}
		break;
	case StepType::INDEX:
		{
			IListPtr list( BlueCastPtr( parent ) );
			if( list && list->GetSize() > m_value.index )
			{
				return list->GetAt( m_value.index );
			}
		}
		break;
	case StepType::KEY:
		{	
			IBlueDictPtr dict( BlueCastPtr( parent ) );
			if( dict && dict.p )
			{
				return dict.p->Subscript( m_value.key.c_str() );
			}
		}
		break;
	default:
		break;
	}
	return nullptr;
}

bool FindFirstRoute( IRoot* from, IRoot* to, std::vector<RouteStep>* result )
{
	if( !from || !to )
	{
		return false;
	}

	from = from->GetRootObject();
	to = to->GetRootObject();
	if( from == to )
	{
		return true;
	}

	std::vector<RouteItem> stack;
	std::unordered_set<IRoot*> visited;

	stack.push_back( RouteItem( from ) );

	while( !stack.empty() )
	{
		RouteItem& item = stack.back();
		if( !item.Next() )
		{
			stack.pop_back();
			continue;
		}

		auto value = item.Value();

		if( value == to )
		{
			if( result != nullptr )
			{
				for( auto it = begin( stack ); it != end( stack ); ++it )
				{
					result->push_back( it->GetRouteStep() );
				}
			}
			return true;
		}
		else
		{
			auto inserted = visited.insert( value );
			if( inserted.second )
			{
				stack.push_back( RouteItem( value ) );
			}
		}
	}
	return false;
}

PyObject* FindRoute( IRoot* from, IRoot* to )
{
	if( !from || !to )
	{
		return PyErr_SetString( PyExc_ValueError, "both from and to parameters cannot be None" ), nullptr;
	}

	PyObject* result = PyList_New( 0 );

	from = from->GetRootObject();
	to = to->GetRootObject();
	if( from == to )
	{
		PyObject* path = PyList_New( 0 );
		PyList_Append( result, path );
		Py_DECREF( path );
		return result;
	}

	std::vector<RouteItem> stack;
	std::unordered_set<IRoot*> visited;

	stack.push_back( RouteItem( from ) );

	while( !stack.empty() )
	{
		RouteItem& item = stack.back();
		if( !item.Next() )
		{
			stack.pop_back();
			continue;
		}

		auto value = item.Value();

		if( value == to )
		{
			PyObject* path = PyList_New( Py_ssize_t( stack.size() ) );
			for( size_t i = 0; i < stack.size(); ++i )
			{
				PyList_SET_ITEM( path, i, stack[i].GetPathItem() );
			}
			PyList_Append( result, path );
			Py_DECREF( path );
		}
		else
		{
			auto inserted = visited.insert( value );
			if( inserted.second )
			{
				stack.push_back( RouteItem( value ) );
			}
		}
	}
	return result;
}

PyObject* PyFindRoute( PyObject* pThis, PyObject* args )
{
	PyObject *pFrom, *pTo;
	if( !PyArg_ParseTuple( args, "OO", &pFrom, &pTo ) )
	{
		return NULL;
	}

	IRoot* from = BlueUnwrapObjectFromPython( pFrom );
	IRoot* to = BlueUnwrapObjectFromPython( pTo );
	if( !from || !to )
	{
		PyErr_Format( PyExc_TypeError, "Argument 'from' or 'to' are not of type IRoot." );
		return NULL;
	}
	
	return FindRoute( from, to );
}

bool FindReference( IRoot* from, IRoot* to )
{
	if( from == to )
	{
		return true;
	}
	if( !from || !to )
	{
		return false;
	}

	objectstack_t stack;
	rootset_t seen;

	//Prime the stack and start
	stack.push_back( StackEntry( from, 0, true ) ); // 'true' since we must assume it's not an autovar.
	while( !stack.empty() )
	{
		StackEntry e = stack.back();
		stack.pop_back();

		from = e.obj;
		if( e.check )
		{
			// We need to check this, if we've seen it before
			std::pair<rootset_t::iterator, bool> res = seen.insert( from );
			if( !res.second )
				continue;
		}

		// do we have a winner here?  See if the class matches what we are looking for.
		if( from == to )
		{
			return true;
		}

		PushChildren( from, e.level, stack, 0 );
	}
	return false;
}

#endif
