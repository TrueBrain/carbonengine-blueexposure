////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		November 2011
// Copyright:	CCP 2011
//

#pragma once
#ifndef BlueClasses_h
#define BlueClasses_h

#include "include/BlueExposureMacros.h"
#include "include/IBlueClasses.h"
#include "include/BlueUtil.h"

BLUE_DECLARE( BlueClasses );

BLUE_CLASS( BlueClasses ) : public IBlueClasses
{
public:
	EXPOSE_TO_BLUE();

	// Constructor is private

	static bool Initialize();

	//////////////////////////////////////////////////////////////////////////
	// IBlueClasses
	
	// Register classes from a null-terminated vector of class registrations
	virtual void RegisterClasses( const Be::ClassRegistration table[] );

	// Register classes from a std::vector of class registrations (null-termination
	// not required)
	virtual void RegisterClasses( std::vector<Be::ClassRegistration>& table	);

	// Unregister classes from a null-terminated vector of class registrations
	virtual void UnregisterClasses(	const Be::ClassRegistration table[] );

	// Unregister classes from a std::vector of class registrations (null-termination
	// not required)
	virtual void UnregisterClasses(	std::vector<Be::ClassRegistration>& table );

	// Get a class registration from a class ID.
	virtual const Be::ClassRegistration* GetClassRegistration( const Be::Clsid& clsid );

	// Find a class ID from the class module and name.
	virtual bool FindClsid( Be::Clsid& clsid, const char* module, const char* name );
	virtual const Be::Clsid* FindClsid( const char* name );

	// Create an instance of the class from the class ID, cast to the given interface.
	virtual bool CreateInstance( const Be::Clsid& clsid, const Be::IID& riid, void** ppv );
	virtual bool CreateInstanceFromName( const char* className, const Be::IID& riid, void** ppv );

	// Works like IUnknown::QueryInterface
	virtual bool QueryThisInterface(
		void* pThis,
		const Be::IID& riid, 
		void** ppv,
		const Be::ClassInfo* type,
		BLUEQIOPT options = BEQI_NONE
		);

	// Another convenience function
	virtual const Be::VarEntry* FindVariable(
		const char* name, 
		const Be::ClassInfo* type,
		ptrdiff_t* xtraoffset
		);

	virtual void UpdateObjectCount( IRoot* obj, int inst, int lock );

	// Copies data from 'source' into '*dest'. If '*dest' is NULL, a new 
	// instance of same class as 'source' is created. If '*dest' is valid,
	// the data is copied into the object.
	virtual bool CopyTo(
		IRoot* source,
		IRoot** dest
		);

	// Same as CopyTo, except that it preserves topology of multiply instanced
	// Child objects.  (CopyTo will resolve such multiple instances in the source
	// object into separate copies in the destination)
	virtual bool CloneTo(
		IRoot* source,
		IRoot** dest
		);

	// Process pending deletes. This may be throttled back to limit the time spent
	// in one call.
	virtual void ProcessPendingDeletes();

	// Process all pending deletes, without regard for the time taken.
	virtual void ProcessAllPendingDeletes();

	virtual void SetPendingDeletesEnabled( bool b );
	virtual bool IsPendingDeletesEnabled() const;

	// Register a set of methods that are added to any object with the given interface
	virtual void RegisterThunker(
		const BlueMethodDefinition* defs,
		const Be::IID& riid
		);
	
	virtual IBlueRtti* GetRtti( const Be::ClassInfo *ci );
	//
	//////////////////////////////////////////////////////////////////////////

protected:
	BlueClasses( IRoot* lockobj = NULL );
	~BlueClasses();

private:

	// generic thunkers
	struct GenericThunker
	{
		const BlueMethodDefinition* mDefs;
		const Be::IID* mIID;
	};

	typedef TrackableStdVector<GenericThunker> Thunkers;
	typedef Thunkers::iterator ThunkIt;
	Thunkers mGenericThunkers;


	// Wrapper around the Be::Clsid pointer. Without this we can't guarantee
	// the Clsid isn't copied around when used in the map.
	struct CIDP {
		CIDP(const Be::Clsid *id) : mID(id) {}
		bool operator < (const CIDP &other) const {return *mID < *other.mID;}
		const Be::Clsid *mID;
	};
	typedef TrackableStdMap<CIDP, const Be::ClassRegistration*> ClassReg;
	typedef ClassReg::iterator ClassRegIt;
	ClassReg m_classes;

	typedef TrackableStdHashMap<std::string, const Be::ClassRegistration*> ClassNameReg;
	typedef ClassNameReg::iterator ClassNameRegIt;
	ClassNameReg m_classesByName;

	void RegisterSingleClass( const Be::ClassRegistration* cr );
	void UnregisterSingleClass( const Be::ClassRegistration* cr );
	const Be::ClassRegistration* GetClassRegImpl( const Be::Clsid& clsid );

	// Maximum time, in milliseconds, to spend on processing pending deletes
	float m_maxTimeForPendingDeletes;

	// Upper limit on number of pending deletes
	unsigned int m_maxPendingDeletes;
	
#if BLUE_WITH_PYTHON
	static PyObject* PyCreateInstance( PyObject* self, PyObject* args );
	static PyObject* PyGetClassTypes( PyObject* self, PyObject* args );
	static PyObject* PyLiveCount( PyObject* self, PyObject* args );
#endif

	std::string GetPersistedClassesReport();

#if BLUE_LIVELIST_ENABLED
	// We want to keep a list of all the objects alive:
	struct AliveInstEntry {
		AliveInstEntry() : mLock(0) {}
		int mLock;
	};
	typedef TrackableStdMap<IRoot *, AliveInstEntry> AliveByInst_t;
	typedef AliveByInst_t::iterator AliveByInst_i;
	AliveByInst_t mAliveByInst;

	// Objects may be created and destroyed on differed threads
	mutable CcpMutex m_aliveTrackingMutex;

	static PyObject* PyLiveList( PyObject* self, PyObject* args );
#endif

};

TYPEDEF_BLUECLASS( BlueClasses );

#endif
