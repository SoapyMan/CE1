
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: 
//
//  Description:  
//
//	History:
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_SKIN_HDR_
#define _CRY_SKIN_HDR_

#include "CrySkinTypes.h"
#include "CrySkinBase.h"

//////////////////////////////////////////////////////////////////////////
// the optimized skinner; built with the CrySkinBuilder class instance,
// destroyed with the Release()
// This is the full skinner: it skins into a memory with garbage in it
class CrySkinFull : public CrySkinBase
{
public:
	friend class CrySkinBuilder;

	// does the skinning out of the given array of global matrices
	void skin(const Matrix44* pBones, Vec3* pDest);
	// Skins skipping the translation components of bone matrices
	void skinAsVec3d16(const Matrix44* pBones, Vec3dA16* pDest);

	// takes each offset and includes it into the bbox of corresponding bone
	void computeBoneBBoxes(CryBBoxA16* pBBox);

	void scale(float fScale)
	{
		scaleVertices(fScale);
	}

	// validates the skin against the given geom info
	void validate(const class ICrySkinSource* pGeometry);

	// this structure contains the statistical information about this skin; its calculation
	// may take significant time and should not be used in game run time (only for debugging purposes
	// and to output statistics in the tools)
	class CStatistics : public CrySkinBase::CStatistics
	{
	public:
		CStatistics(const CrySkinFull* pSkin) :
			CrySkinBase::CStatistics(pSkin)
		{
			initSetDests(pSkin);
		}

		void initSetDests(const CrySkinFull* pSkin);
		void addDest(unsigned nDest);

		// destination vertex set
		std::set<unsigned> setDests;
		// the number of links per each vertex
		std::vector<unsigned> arrNumLinks;
	};

	friend class CStatistics;
};

#endif