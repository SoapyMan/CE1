
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

#include "stdafx.h"
//#include "CryAnimationBase.h"
#include "CrySkinRigidBasis.h"

#define FOR_TEST 0

#if FOR_TEST
#include "CryAnimation.h"
#include "CVars.h"
#endif

// returns the size of the skin, the number of bases being calculated
// by this skin. The bases are calculated into a 0-base continuous array
// tangents may be divided into subskins, each having different number of bases
// to skin, based on the performance consideration (strip mining)
unsigned CrySkinRigidBasis::size()const
{
	return m_numDestBases;
}

// does the same as the base class init() but also remembers the number of bases (numVerts/2)
// for future reference
void CrySkinRigidBasis::init(unsigned numVerts, unsigned numAux, unsigned numSkipBones, unsigned numBones)
{
	m_numDestBases = numVerts >> 1;
	CrySkinBase::init(numVerts, numAux, numSkipBones, numBones);
}


void CrySkinRigidBasis::CStatistics::initSetDests(const CrySkinRigidBasis* pSkin)
{
	const CrySkinAuxInt* pAux = &pSkin->m_arrAux[0];
	const Vertex* pVertex = &pSkin->m_arrVertices[0];
	setDests.clear();
	arrNumLinks.clear();

	for (unsigned nBone = pSkin->m_numSkipBones; nBone < pSkin->m_numBones; ++nBone)
	{
		// each bone has a group of (always rigid) vertices

		// this is to take into account two groups: non-flipped and flipped tangents
		for (int t = 0; t < 2; ++t)
		{
			// for each actual basis, we have two vertices
			const Vertex* pGroupEnd = pVertex + (*pAux++ << 1);
			for (; pVertex < pGroupEnd; pVertex += 2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				CRYASSERT(nDestOffset < pSkin->m_numDestBases * sizeof(SPipTangentsA) && nDestOffset % sizeof(SPipTangentsA) == 0);
				unsigned nDest = nDestOffset / sizeof(SPipTangentsA);
				addDest(nDest);
			}
		}
	}
}

void CrySkinRigidBasis::CStatistics::addDest(unsigned nDest)
{
	if (arrNumLinks.size() < nDest + 1)
		arrNumLinks.resize(nDest + 1, 0);
	++arrNumLinks[nDest];
	setDests.insert(nDest);
}


// does the skinning out of the given array of global matrices:
// calculates the bases and fills the PipVertices in
void CrySkinRigidBasis::skin(const Matrix44* pBones, SPipTangentsA* pDest)const
{
#ifdef DEFINE_PROFILER_FUNCTION
	DEFINE_PROFILER_FUNCTION();
#endif
#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
		const Matrix44* pBone = pBones + m_numSkipBones, * pBonesEnd = pBones + m_numBones;
		const CrySkinAuxInt* pAux = &m_arrAux[0];
		const Vertex* pVertex = &m_arrVertices[0];

		for (; pBone != pBonesEnd; ++pBone)
		{
			// each bone has a group of (always rigid) vertices

			// for each actual basis, we have two vertices
			const Vertex* pGroupEnd = pVertex + (*pAux++ << 1);
			for (; pVertex < pGroupEnd; pVertex += 2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				CRYASSERT(nDestOffset < m_numDestBases * sizeof(SPipTangentsA));
				SPipTangentsA& rDest = *(SPipTangentsA*)(UINT_PTR(pDest) + nDestOffset);

				//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
				Vec3d vTang = pBone->TransformVectorOLD(pVertex[0].pt);
				//Vec3d vTang = GetTransposed44(*pBone)*(pVertex[0].pt);

				//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
				Vec3d vBinorm = pBone->TransformVectorOLD(pVertex[1].pt);
				//Vec3d vBinorm = GetTransposed44(*pBone)*(pVertex[1].pt);

				rDest.m_Tangent = vTang;
				rDest.m_Binormal = vBinorm;
				rDest.m_TNormal = vTang ^ vBinorm;
			}

			// the flipped version
			pGroupEnd = pVertex + (*pAux++ << 1);
			for (; pVertex < pGroupEnd; pVertex += 2)
			{
				unsigned nDestOffset = pVertex[0].nDest & 0xFFFFFF;
				CRYASSERT(nDestOffset < m_numDestBases * sizeof(SPipTangentsA));
				SPipTangentsA& rDest = *(SPipTangentsA*)(UINT_PTR(pDest) + nDestOffset);
				//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
				Vec3d vTang = pBone->TransformVectorOLD(pVertex[0].pt);
				//Vec3d vTang = GetTransposed44(*pBone)*(pVertex[0].pt);

				//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
				Vec3d vBinorm = pBone->TransformVectorOLD(pVertex[1].pt);
				//Vec3d vBinorm = GetTransposed44(*pBone)*(pVertex[1].pt);

				rDest.m_Tangent = vTang;
				rDest.m_Binormal = vBinorm;
				rDest.m_TNormal = vBinorm ^ vTang;
			}
		}
	}
}

// returns the number of bytes occupied by this structure and all its contained objects
unsigned CrySkinRigidBasis::sizeofThis()const
{
	return CrySkinBase::sizeofThis() + sizeof(CrySkinRigidBasis) - sizeof(CrySkinBase);
}

unsigned CrySkinRigidBasis::Serialize(bool bSave, void* pBuffer, unsigned nBufSize)
{
	if (bSave)
	{
		unsigned nWrittenBytes = CrySkinBase::Serialize_PC(true, pBuffer, nBufSize);
		if (nWrittenBytes)
		{
			if (pBuffer)
				*(unsigned*)(((char*)pBuffer) + nWrittenBytes) = m_numDestBases;

			return sizeof(unsigned) + nWrittenBytes;
		}
		else
		{
			// error
			return 0;
		}
	}
	else
	{
		unsigned nReadBytes = CrySkinBase::Serialize_PC(false, pBuffer, nBufSize);
		if (nReadBytes)
		{
			if (nBufSize - nReadBytes >= sizeof(unsigned))
			{
				m_numDestBases = *(unsigned*)(((char*)pBuffer) + nReadBytes);
				return nReadBytes + sizeof(unsigned);
			}
			else
			{
				//error - perhaps not the tang stream
				m_numDestBases = 0;
				clear();
				return 0;
			}
		}
		else
		{
			//error
			return 0;
		}
	}
}
