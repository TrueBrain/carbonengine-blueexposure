#include "Copier.h"
#include "BluePyWrap.h"
#include "include/INotify.h"
#include "include/IInitialize.h"
#include "include/BlueListUtil.h"
#if BLUE_WITH_PYTHON
#include "include/PythonKlass.h"
#endif
#include "CcpCore/include/CCPLog.h"

//finally obsoleted.  Same as copy (copyto used to do a blind deep copy)
bool Copier::CloneTo(IRoot* source, IRoot** dest)
{
	return CopyTo(source, dest);
}


//The copier outer layer.  It handles the generation of a destination class
//if required, and optionally looks into maps to get previously created
//objects.
bool Copier::CopyTo(IRoot* source, IRoot** _dest)
{
	IRootPtr dest(*_dest);

	const Be::Clsid* sclsid = source->ClassType()->mClassId;

	if (!dest)
	{
		// We are expected to fill out the destination pointer.  First, if we
		// are cloning, see if we already created a destination matching this source.

		//Get the final iroot, necessary for comparisons
		source = BlueFinalIRoot(source);
			
		if (mPointers.get()) {
			tMapIter it = mPointers.get()->find(source);
			if (it != mPointers.get()->end()) {
				// Yes, we found it.  Just return it
				*_dest = (*it).second;
				(*_dest)->Lock();
				// We don't know what interface the caller expects. He has to
				// query it.
				return true;
			}
		}

		//Well, create an empty one, then.
		dest.CreateInstance(*sclsid);
		if (!dest)
			return false;

		// Good.  If we created it, put it into the pointer array.
		// must do this now, before recursing.
		if (!mPointers.get()) 
			//lazy generation of map
			mPointers = std::auto_ptr<tMap>(CCP_NEW( "Copier/mPointers" ) tMap);
		mPointers->insert( tPair(source, dest));

	}
	else
	{
		// assert same classes
		const Be::Clsid* destclsid = dest->ClassType()->mClassId;

		if (!sclsid->IsEqual(*destclsid))
		{
			CCP_LOGERR(
				"In CopyTo, 'source' and 'dest must be of same type. "
				"Source is %s.%s, dest is %s.%s",
				sclsid->GetModule(), sclsid->GetName(),
				destclsid->GetModule(), destclsid->GetName()
				);

			return false;
		}
	}
	
	// We now have a destination to copy to.
	mLevel++;
	bool result = CopyToInternal(source, dest);
	mLevel--;
	if (!result)
		return false;
	
	if (!*_dest)
		*_dest = dest.Detach();

	//after a single copying operation, clear the pointer array
	if (mLevel == 0 && mPointers.get())
		mPointers.get()->clear();
	return true;
}


// Actually handles the copying of data.  dest must exist
bool Copier::CopyToInternal(IRoot* source, IRoot* dest) 
{
	INotifyPtr notify( BlueCastPtr( dest ) );	//for notifications of change to the dest
	IInitializePtr init( BlueCastPtr( dest ) );
	if( init )
	{
		// New style proper initialization - turn off notifications
		notify = (INotify*)0;
	}

	char *sourceFinal = (char*)BlueFinalIRoot(source);
	char *destFinal   = (char*)BlueFinalIRoot(dest);
	
	//Iterate over chained types in class
	const Be::ClassInfo* type;
	ssize_t offset = 0;
	for (type = dest->ClassType();
		 type;
		 offset += type->mOffsetToParent, type = type->mParentClassInfo)
	{
		const ssize_t interfaceOffs = type->mInterfaceTable->mOffset;
		char * const srcBase  = sourceFinal + offset - interfaceOffs;
		char * const destBase = destFinal   + offset - interfaceOffs;

		//Iterate over members
		const Be::VarEntry* memTable;
		for(memTable = type->mMemberTable; memTable->mType!=Be::INVALID; memTable++) {
			if (!(memTable->mEditFlags & Be::PERSIST))
				continue;
			
			Be::Var * const src = (Be::Var*)(srcBase  + memTable->mOffset);
			Be::Var * const dst = (Be::Var*)(destBase + memTable->mOffset);
			
			//This compare ensures that the on-modify callback is only made when we really
			//change things.
			if (memTable->mType != Be::CHARARRAY) {
				//hm, we want to avoid IROOT types too! but then we get callbacks.
				if (!memcmp(src, dst, memTable->mSize))
					continue;
			} else {
				//and compare arrays as strings
				if (!strncmp((const char *)src, (const char *)dst, memTable->mSize))
					continue;
			}
				
			switch(memTable->mType)
			{
			case Be::LONG:
				dst->mLong = src->mLong;
				break;

			case Be::BYTE:
				dst->mByte = src->mByte;
				break;

			case Be::SHORT:
				dst->mShort = src->mShort;
				break;

			case Be::FLOAT:
				dst->mFloat = src->mFloat;
				break;

			case Be::FLOATARRAY:
				memcpy( (void*)&dst->mFloat, (void*)&src->mFloat, memTable->mSize);
				break;

			case Be::DOUBLEARRAY:
				memcpy( (void*)&dst->mDouble, (void*)&src->mDouble, memTable->mSize);
				break;

			case Be::INTARRAY:
				memcpy( (void*)&dst->mLong, (void*)&src->mLong, memTable->mSize);
				break;

			case Be::DOUBLE:
				dst->mDouble = src->mDouble;
				break;

			case Be::BOOL:
				dst->mBool = src->mBool;
				break;
			case Be::INT64:
				dst->mInt64 = src->mInt64;
				break;
			case Be::CHARARRAY:
				memcpy(dst, src, memTable->mSize);
				break;
			case Be::CSTRING:
			case Be::REFERENCE:
				CCP_FREE(dst->mCharPtr);
				dst->mCharPtr = CCP_STRDUP( __FUNCTION__, src->mCharPtr);
				break;
			case Be::STDSTRING:
				{
					const std::string &srcString = *reinterpret_cast<const std::string*>(src);
					std::string &dstString = *reinterpret_cast<std::string*>(dst);
					dstString = srcString;
				}
				break; 
			case Be::WCSTRING:
			case Be::WREFERENCE:
				CCP_FREE(dst->mWCharPtr);
				dst->mWCharPtr = CCP_WSTRDUP( __FUNCTION__, src->mWCharPtr);
				break;
			case Be::STDWSTRING:
				{
					const std::wstring &srcString = *reinterpret_cast<const std::wstring*>(src);
					std::wstring &dstString = *reinterpret_cast<std::wstring*>(dst);
					dstString = srcString;
				}
				break; 
			case Be::IROOT:
				//Internal copyto
				if ( !CopyToInternal( reinterpret_cast<IRoot*>( src ), reinterpret_cast<IRoot*>( dst ) ) )
					return false;
				break;
			case Be::IROOTPTR: 
				{
					// Release any destination object after we finish
					IRootPtr dstOld;
					dstOld.Attach( dst->mIRootPtr ); // Don't INCREF what was there
					// unlock the old destination object when we go out of scope
					// NB: This should prevent things from blowing up when you reference the same IRootPtr multiple times in the same class
					// which can be useful behavior for dealing with legacy code
					dst->mIRootPtr = NULL;

					if (!src->mIRootPtr)
						break;
					IRootPtr tmp;
					if (!CopyTo(src->mIRootPtr, &tmp))
						return false;
					//Query the correct interface
					static Be::IID irootType( "IRoot" );
					const Be::IID &iid = memTable->mIID ? *memTable->mIID : irootType;
					if (!tmp->QueryInterface(iid, (void**)&dst->mIRootPtr))
						return false;
					break; 
				}
			
			case Be::IROOTWEAKREF:
				{
					BlueWeakRefBase* srcWeakRef = reinterpret_cast<BlueWeakRefBase*>( src );
					BlueWeakRefBase* dstWeakRef = reinterpret_cast<BlueWeakRefBase*>( dst );

					*dstWeakRef = *srcWeakRef;
				}
				break;

#if BLUE_WITH_PYTHON
			case Be::PYOBJECTPTR:
				dst->mPyObject = src->mPyObject;
				Py_XINCREF(dst->mPyObject);
				break;
#endif

			case Be::SHAREDSTRING:
				{
					const BlueSharedString &srcString = *reinterpret_cast<const BlueSharedString*>(src);
					BlueSharedString &dstString = *reinterpret_cast<BlueSharedString*>(dst);
					dstString = srcString;
				}
				break; 

			default:
				CCP_LOGERR( "Unknown blue type in member %s", memTable->mName );
				return false;
			}
			if (notify && !notify->OnModified(dst))
				return false;
		}
	}

	// Copy data that wasn't exposed as regular properties
	ICopierCustomAssignmentPtr srcprop( BlueCastPtr( source ) );
	if (srcprop && !srcprop->AssignTo(ICopierCustomAssignmentPtr( BlueCastPtr( dest ) ), this))
		return false;

#if BLUE_WITH_PYTHON
	//copy the deco of the class.
	BlueLockData *ld = BlueInternalHasLockData(source);
	if (ld && ld->mPythonKlass && !ld->mPythonKlass->CopyTo(source, dest))
		return false;
#endif

	if( init )
	{
		return init->Initialize();
	}

	return true;
}
