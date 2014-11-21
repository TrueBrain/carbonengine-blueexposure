////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		Aug 2012
// Copyright:	CCP 2012
//
//
// BlueStructureList is used for persisting and exposing to Python lists of
// structures, similar to BlueList for storing Blue objects. Note that these
// have to be plain data structures, composed of floats and ints - there is
// no support for pointers or std::strings, or things like that.
//
// When saving structure lists to black files, the whole list is written in
// one go, meaning that loading from black file is very efficient - the whole
// list is read in one go. This again relies on the structures being plain
// data structures - having virtual functions is absolutely not supported.
//

#ifndef BlueStructureList_h
#define BlueStructureList_h

#include "BlueExposureMacros.h"
#include "IBlueStructureList.h"
#include "ICopier.h"

BLUE_DECLARE( StructureList );

#if BLUE_WITH_PYTHON

BLUEIMPORT PyObject* BlueStructureList_StructurePyObject( IBlueStructureList* structureList, uint8_t* item );
BLUEIMPORT void BlueStructureList_PyObjectToStructure( IBlueStructureList* structureList, PyObject* value, uint8_t* item );
BLUEIMPORT PyObject* BlueStructureList_PyGetStructureDefinition( IBlueStructureList* structureList );

#endif

BLUEIMPORT float BlueFloat16To32( uint16_t in );
BLUEIMPORT uint16_t BlueFloat32To16( float in );

namespace BlueStructureListHelpers
{
template <typename N>
struct IsBool { static const bool isBool = false; };

template <>
struct IsBool<bool> { static const bool isBool = true; };
}

template <typename T>
class BlueStructureList : 
	public IBlueStructureList,
	public ICopierCustomAssignment
{

	// Because STL vector has a smart specialization for std::vector<bool> we
	// cannot use it with BlueStructureList. In case one ever really needs it
	// he can write a specialization BlueStructureList<bool>.
	static_assert( !BlueStructureListHelpers::IsBool<T>::isBool, "BlueStructureList does not support bool as item type" );
public:
	typedef TrackableStdVector<T> vector_t;
	typedef typename vector_t::const_iterator const_iterator;
	typedef typename vector_t::iterator iterator;
	typedef typename vector_t::value_type value_type;
	typedef BlueStructureList<T> ClassDef;

	BlueStructureList( IRoot* lockobj = nullptr ) :
		m_structureDefinition( nullptr ),
		m_memberCount( 0 ),
		m_items("BlueStructureList"),
		m_defaultValue( nullptr )
	{
	}

	~BlueStructureList()
	{}

	void SetStructureDefinition( BlueStructureDefinition* sd )
	{
		m_structureDefinition = sd;

		BlueStructureDefinition* memberDef = sd;
		m_memberCount = 0;
		while( memberDef->m_name )
		{
			++m_memberCount;
			++memberDef;
		}
	}

	void SetDefaultValue( const T* defaultValue )
	{
		m_defaultValue = defaultValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IBlueStructureList methods
	///
	virtual BlueStructureDefinition* GetStructureDefinition() 
	{
		return m_structureDefinition;
	}

	virtual size_t GetMemberCount()
	{
		return m_memberCount;
	}

	virtual size_t GetStructureSize() 
	{
		return sizeof( T );
	}

	virtual size_t GetSize() 
	{
		return m_items.size();
	}

	virtual const void* GetDefaultValue()
	{
		return m_defaultValue;
	}

	virtual void* GetAt( size_t ix ) 
	{
		return &m_items[ix];
	}

	virtual bool Append( const  void* val ) 
	{
		const T* p = static_cast<const T*>( val );
		m_items.push_back( *p );
		return true;
	}

	virtual bool Insert( size_t ix, const void* val ) 
	{
		const T* p = static_cast<const T*>( val );
		iterator it = m_items.begin() + ix;
		m_items.insert( it, *p );
		return true;
	}

	virtual bool Remove( size_t ix ) 
	{
		iterator it = m_items.begin() + ix;
		m_items.erase( it );
		return true;
	}

	virtual void Clear() 
	{
		m_items.clear();
	}

	virtual void Resize( size_t numItems )
	{
		m_items.resize( numItems );
	}

	//////////////////////////////////////////////////////////////////////////
	// ICopierCustomAssignment

	// --------------------------------------------------------------------------------------
	// Description:
	//   Provides support for copying a BlueStructureList to another, through CloneTo() or CopyTo()
	// @@ ICopierCustomAssignment
	// --------------------------------------------------------------------------------------
	bool AssignTo(
		ICopierCustomAssignment* other,
		ICopier* copier
		)
	{
		ClassDef* dest = static_cast<ClassDef*>( other );

		dest->m_items = m_items;

		return true;
	}

	//
	//////////////////////////////////////////////////////////////////////////

	const_iterator begin() const
	{
		return m_items.begin();
	}

	const_iterator end() const
	{
		return m_items.end();
	}

	iterator begin()
	{
		return m_items.begin();
	}

	iterator end()
	{
		return m_items.end();
	}

	size_t size() const
	{
		return m_items.size();
	}

	bool empty() const
	{
		return m_items.empty();
	}

	const T& operator[]( size_t ix ) const
	{
		return m_items[ix];
	}

	T& operator[]( size_t ix )
	{
		return m_items[ix];
	}

	void clear()
	{
		m_items.clear();
	}

#if BLUE_WITH_PYTHON
	//////////////////////////////////////////////////////////////////////////
	// Python exposure
	//

	// Note that MAP_METHOD_AND_WRAP chokes on a templated class so
	// we have to create these thunkers manually.


	static PyObject* PyClear( PyObject* self, PyObject* args )
	{
		ClassDef* pThis = BluePythonCast<ClassDef*>( self );

		pThis->clear();

		Py_RETURN_NONE;
	}

	static PyObject* PyAppend( PyObject* self, PyObject* args )
	{
		ClassDef* pThis = BluePythonCast<ClassDef*>( self );

		PyObject* obj;
		if( !PyArg_ParseTuple( args, "O", &obj ) )
		{
			return nullptr;
		}

		T item;

		try
		{
			BlueStructureList_PyObjectToStructure( pThis, obj, (uint8_t*)&item );
			pThis->Append( &item );
		}
		catch( std::exception& e )
		{
			PyErr_Format( PyExc_TypeError, "%s", e.what() );
			return nullptr;
		}
		Py_RETURN_NONE;
	}

	static PyObject* PyRemoveAt( PyObject* self, PyObject* args )
	{
		ClassDef* pThis = BluePythonCast<ClassDef*>( self );

		long index;
		if( !PyArg_ParseTuple( args, "l", &index ) )
		{
			return nullptr;
		}

		if( index < 0 || index >= long( pThis->size() ) )
		{
			PyErr_Format( PyExc_IndexError, "Structure list removeAt index %ld out of range", index );
			return nullptr;
		}
		else
		{
			pThis->Remove( size_t( index ) );
		}

		Py_RETURN_NONE;
	}

	static PyObject* PyGetStructureDefinition( PyObject* self, PyObject* args )
	{
		ClassDef* pThis = BluePythonCast<ClassDef*>( self );

		if( !PyArg_ParseTuple( args, "" ) )
		{
			return nullptr;
		}

		return BlueStructureList_PyGetStructureDefinition( pThis );
	}

	static PyObject* PyGetDefaultValue( PyObject* self, PyObject* args )
	{
		ClassDef* pThis = BluePythonCast<ClassDef*>( self );

		if( !PyArg_ParseTuple( args, "" ) )
		{
			return nullptr;
		}

		auto defaultValue = pThis->GetDefaultValue();
		if( !defaultValue )
		{
			Py_RETURN_NONE;
		}
		return BlueStructureList_StructurePyObject( pThis, (uint8_t*)defaultValue );
	}
#endif

	EXPOSE_TO_BLUE()
	{
		EXPOSURE_BEGIN_IMP( ClassDef, "", GetStructureListClsid() )
			MAP_INTERFACE( IBlueStructureList )
			MAP_INTERFACE( ICopierCustomAssignment )

			MAP_METHOD
			( 
				"clear", 
				PyClear, 
				"Remove all items from the list"
			)

			MAP_METHOD
			(
				"append",
				PyAppend,
				"Appends a value to the list"
			)

			MAP_METHOD
			(
				"removeAt",
				PyRemoveAt,
				"Removes element at the given index\nArguments:\nindex - element index to remove"
			)

			MAP_METHOD
			(
				"GetStructureDefinition",
				PyGetStructureDefinition,
				"Gets the structure definition"
			)

			MAP_METHOD
			(
				"GetDefaultValue",
				PyGetDefaultValue,
				"Returns a tuple that is considered a default item value for the list or None if the\n"
				"list does not have a specific default value"
			)
		EXPOSURE_END()
	}


private:
	BlueStructureDefinition* m_structureDefinition;
	size_t m_memberCount;
	vector_t m_items;
	const T* m_defaultValue;
};

#endif // BlueStructureList_h
