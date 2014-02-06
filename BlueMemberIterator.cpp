////////////////////////////////////////////////////////////////////////////////
//
// Creator:		Snorri Sturluson
// Created:		October 2012
// Copyright:	CCP 2012
//

#include "include/BlueMemberIterator.h"
#include "include/BlueUtil.h"
#include "include/IList.h"
#include "include/IBlueDict.h"
#include "include/IBlueStructureList.h"
#include "include/BlueSharedString.h"
#include <string>
#include <cmath>

BlueMemberIterator::BlueMemberIterator( IRoot *object ) : 
	mPersist_only(true), 
	mRead(false), 
	mObject(object)
{
	if( object )
	{
		mType = mObject->ClassType();
		Reset();
	}
	else 
	{
		mEntry = 0;
		mType = 0;
	}
}

BlueMemberIterator::BlueMemberIterator(  const Be::ClassInfo* clsInfo ) :
	mPersist_only( true ),
	mRead( false ),
	mObject( nullptr )
{
	mType = clsInfo;
	Reset();
}

BlueMemberIterator::BlueMemberIterator( const BlueMemberIterator &other ) : 
mPersist_only(other.mPersist_only),
	mRead(other.mRead)
{
	*this = other;
}



bool BlueMemberIterator::Eof() const
{
	return !mEntry;
}

//Reset.  if persist, point to the first persist member
bool BlueMemberIterator::Reset()
{
	mOffs = 0;
	mEntry = mType->mMemberTable;
	
	if( !mEntry->mName && !NextParent() )
	{
		return false;
	}
	
	if( !IsInteresting() )
	{
		return Next();
	}
	return true;
}

// Go to next member
bool BlueMemberIterator::Next(int n)
{
	if( Eof() )
	{
		return false;
	}
	
	for( int i = 0; i<n; i++ )
	{
		do {
			mEntry++;
			if( !mEntry->mName && !NextParent() )
			{
				return false;
			}
		} while( !IsInteresting() );
	}
	
	return true;
}


// Go to next non-empty parent, or signal EoF;
bool BlueMemberIterator::NextParent()
{
	//Move to the next parent
	do {
		const Be::ClassInfo* parentType = mType->mParentClassInfo;
		if( !parentType )
		{
			// Eof!
			mEntry = 0;
			return false;
		}

		mOffs += mType->mOffsetToParent;
		mType = parentType;
		mEntry = mType->mMemberTable;
	} while (!mEntry->mName);

	return true;
}


int BlueMemberIterator::SkipEquals(IRoot* def)
{
	if( !def )
	{
		return 0;
	}

	int skipcount = 0;
	while( !Eof() )
	{
		if( !IsEqual( Var( def ) ) )
		{
			break;
		}
		skipcount++;
		Next();
	}

	return skipcount;
}

#define FLOAT_PRECISION_ERROR 1e-6f

static bool IsArrayEqual( const float* a, const float* b, size_t numElements )
{
	for( size_t i = 0; i < numElements; ++i )
	{
		float d = a[i] - b[i];
		if( fabs( d ) >= FLOAT_PRECISION_ERROR )
		{
			return false;
		}
	}
	return true;
}

static bool CompareStrings(const char* a, const char* b)
{
	if( a == b )
	{
		return true;
	}
	else if( !a || !b )
	{
		return false;
	}
	else
	{
		return strcmp( a, b ) == 0;
	}
}


static bool CompareWStrings(const wchar_t* a, const wchar_t* b)
{
	if( a == b )
	{
		return true;
	}
	else if( !a || !b )
	{
		return false;
	}
	else
	{
		return wcscmp(a, b) == 0;
	}
}

bool BlueMemberIterator::IsEqual(const Be::Var *b)
{
	Be::Var const *a = Var();
	switch(mEntry->mType)
	{
	case Be::LONG:
		return a->mLong == b->mLong;

	case Be::BYTE:
		return a->mByte == b->mByte;

	case Be::SHORT:
		return a->mShort == b->mShort;

	case Be::FLOAT:
		{
			float d = a->mFloat - b->mFloat;
			return fabs( d ) < FLOAT_PRECISION_ERROR;
		}

	case Be::FLOATARRAY:
		return IsArrayEqual( &a->mFloat, &b->mFloat, mEntry->GetFloatArraySize() );

	case Be::DOUBLE:
		{
			double d = a->mFloat - b->mFloat;
			return fabs( d ) < 1e-12;
		}
		// return a->mDouble == b->mDouble;

	case Be::BOOL:
		return a->mBool ? b->mBool : !b->mBool;

	case Be::IROOT:
		{
			// We're not really handling the generic case of embedded objects
			// but we are checking the common case of empty lists.
			{
				IListPtr listA( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( a ) ) ) );
				IListPtr listB( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( b ) ) ) );
				if( listA && listB )
				{
					if( (listA->GetSize() == 0) && (listB->GetSize() == 0) )
					{
						return true;
					}
				}
			}

			{
				IBlueDictPtr dictA( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( a ) ) ) );
				IBlueDictPtr dictB( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( b ) ) ) );
				if( dictA && dictB )
				{
					if( (dictA->GetLength() == 0) && (dictB->GetLength() == 0) )
					{
						return true;
					}
				}
			}

			{
				IBlueStructureListPtr listA( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( a ) ) ) );
				IBlueStructureListPtr listB( BlueCastPtr( reinterpret_cast<IRoot*>( const_cast<Be::Var*>( b ) ) ) );
				if( listA && listB )
				{
					if( (listA->GetSize() == 0) && (listB->GetSize() == 0) )
					{
						return true;
					}
				}
			}
		}
		return false;

	case Be::IROOTPTR:
		return a->mIRootPtr == b->mIRootPtr;

	case Be::CHARARRAY:
		return CompareStrings((const char*)a, (const char*)b);

	case Be::CSTRING:
	case Be::REFERENCE:
		return CompareStrings(a->mCharPtr, b->mCharPtr);

	case Be::STDSTRING:
		{			
			const std::string &aStr = *reinterpret_cast<const std::string*>(a);
			const std::string &bStr = *reinterpret_cast<const std::string*>(b);
			return aStr == bStr;
		}
	case Be::WCSTRING:
	case Be::WREFERENCE:
		return CompareWStrings(a->mWCharPtr, b->mWCharPtr);
	
	case Be::STDWSTRING:
		{			
			const std::wstring &aStr = *reinterpret_cast<const std::wstring*>(a);
			const std::wstring &bStr = *reinterpret_cast<const std::wstring*>(b);
			return aStr == bStr;
		}

	case Be::INT64:
		return a->mInt64 == b->mInt64;

#if BLUE_WITH_PYTHON
	case Be::PYOBJECTPTR:
		return a->mPyObject == b->mPyObject;
#endif
	case Be::SHAREDSTRING:
		{			
			const BlueSharedString &aStr = *reinterpret_cast<const BlueSharedString*>(a);
			const BlueSharedString &bStr = *reinterpret_cast<const BlueSharedString*>(b);
			return aStr == bStr;
		}

	default:
		return false;
	}
}


bool BlueMemberIterator::IsInteresting() const
{
	CCP_ASSERT( !Eof() );
	
	if( !mPersist_only )
	{
		return true;
	}

	if( mRead )
	{
		if( mEntry->mEditFlags & (Be::PERSIST|Be::RPERSIST) )
		{
			return true;
		}
	}
	else
	{ // write
		if( mEntry->mEditFlags & Be::PERSIST )
		{
			return true;
		}
	}

	return false;
}



const Be::VarEntry* BlueMemberIterator::Entry() const
{
	return mEntry;
}

Be::Var & BlueMemberIterator::operator*() const
{
	CCP_ASSERT(!Eof()); return *Var();
}

const Be::VarEntry * BlueMemberIterator::operator->() const
{
	CCP_ASSERT(!Eof()); return mEntry;
}

BlueMemberIterator & BlueMemberIterator::operator++()
{
	Next(1); return *this;
}

BlueMemberIterator & BlueMemberIterator::operator=( const BlueMemberIterator &other )
{
	mObject = other.mObject;
	mOffs = other.mOffs;
	mType = other.mType;
	mEntry = other.mEntry;
	return *this;
}

Be::Var* BlueMemberIterator::Var( IRoot* object /*= NULL*/ ) const
{
	if( !object && !mObject )
	{
		return nullptr;
	}

	return BLUEMAPMEMBEROFFSET(object ? object : mObject.p, mEntry, mType, mOffs);
}

const Be::ClassInfo* BlueMemberIterator::Type() const
{
	return mType;
}

