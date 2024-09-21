
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
#include "MathUtils.h"
#include "CrySkinBuilderBase.h"
#include "CrySkinFull.h"

#define FOR_TEST 0

// takes each offset and includes it into the bbox of corresponding bone
void CrySkinFull::computeBoneBBoxes(CryBBoxA16* pBBoxes)
{
	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];
	CryBBoxA16* pBBox = pBBoxes + m_numSkipBones, * pBBoxEnd = pBBoxes + m_numBones;

	for (; pBBox != pBBoxEnd; ++pBBox)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		Vertex* pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex)
			pBBox->include(pVertex->pt);

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			pBBox->include(pVertex->pt);

		// process the smooth vertices that were the second time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			pBBox->include(pVertex->pt);
	}
}

//////////////////////////////////////////////////////////////////////////
// does the skinning out of the given array of global matrices
void CrySkinFull::skin(const Matrix44* pBones, Vec3d* pDest)
{
	//PROFILE_FRAME_SELF(PureSkin);
#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
		const Matrix44* pBone = pBones + m_numSkipBones, * pBonesEnd = pBones + m_numBones;
		CrySkinAuxInt* pAux = &m_arrAux[0];
		Vertex* pVertex = &m_arrVertices[0];


#ifdef _DEBUG
		TFixedArray<float> arrW;
		arrW.reinit(m_numDests, 0);
#endif

		for (; pBone != pBonesEnd; ++pBone)
		{
			// each bone has a group of vertices

			// first process the rigid vertices
			Vertex* pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex)
			{
				pDest[pVertex->nDest] = pBone->TransformPointOLD(pVertex->pt);
#ifdef _DEBUG
				CRYASSERT(arrW[pVertex->nDest] == 0);
				arrW[pVertex->nDest] = 1;
#endif
			}

			// process the smooth1 vertices that were the first time met
			pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			{
				transformWPoint(pDest[*pAux], *pBone, *pVertex);
#ifdef _DEBUG
				CRYASSERT(arrW[*pAux] == 0);
				arrW[*pAux] = pVertex->fWeight;
#endif
			}

			// process the smooth vertices that were the first time met
			pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			{
				addWPoint(pDest[*pAux], *pBone, *pVertex);
#ifdef _DEBUG
				CRYASSERT(arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
				arrW[*pAux] += pVertex->fWeight;
				CRYASSERT(arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
#endif
			}
		}
		/*#ifdef _DEBUG
			for (unsigned i = 0; i < m_numDests; ++i)
				CRYASSERT (arrW[i] > 0.995f && arrW[i] < 1.005f);
		#endif
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
// does the skinning out of the given array of global matrices
void CrySkinFull::skinAsVec3d16(const Matrix44* pBones, Vec3dA16* pDest)
{
	//PROFILE_FRAME_SELF(PureSkin);
#if FOR_TEST
	for (int i = 0; i < g_GetCVars()->ca_TestSkinningRepeats(); ++i)
#endif
	{
		const Matrix44* pBone = pBones + m_numSkipBones, * pBonesEnd = pBones + m_numBones;
		CrySkinAuxInt* pAux = &m_arrAux[0];
		Vertex* pVertex = &m_arrVertices[0];


#ifdef _DEBUG
		TFixedArray<float> arrW;
		arrW.reinit(m_numDests, 0);
#endif

		for (; pBone != pBonesEnd; ++pBone)
		{
			// each bone has a group of vertices

			// first process the rigid vertices
			Vertex* pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex)
			{
				//CHANGED_BY_IVO  - INVALID CHANGE, PLEASE REVISE
				pDest[pVertex->nDest].v = pBone->TransformVectorOLD(pVertex->pt);
				// Temporary fixed by Sergiy. A new operation in the Matrix must be made
				//pDest[pVertex->nDest].v = GetTransposed44(*pBone) * (pVertex->pt);
				//transformVectorNoTrans (pDest[pVertex->nDest].v, pVertex->pt, *pBone);
#ifdef _DEBUG
				CRYASSERT(arrW[pVertex->nDest] == 0);
				arrW[pVertex->nDest] = 1;
#endif
			}

			// process the smooth1 vertices that were the first time met
			pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			{
				transformWVector(pDest[*pAux].v, *pBone, *pVertex);
#ifdef _DEBUG
				CRYASSERT(arrW[*pAux] == 0);
				arrW[*pAux] = pVertex->fWeight;
#endif
			}

			// process the smooth vertices that were the first time met
			pGroupEnd = pVertex + *pAux++;
			for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
			{
				addWVector(pDest[*pAux].v, *pBone, *pVertex);
#ifdef _DEBUG
				CRYASSERT(arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
				arrW[*pAux] += pVertex->fWeight;
				CRYASSERT(arrW[*pAux] > 0 && arrW[*pAux] < 1.005f);
#endif
			}
		}
#ifdef _DEBUG
		for (unsigned i = 0; i < m_numDests; ++i)
			CRYASSERT(arrW[i] > 0.995f && arrW[i] < 1.005f);
#endif
	}
}

void CrySkinFull::CStatistics::addDest(unsigned nDest)
{
	if (arrNumLinks.size() < nDest + 1)
		arrNumLinks.resize(nDest + 1, 0);
	++arrNumLinks[nDest];
	setDests.insert(nDest);
}

void CrySkinFull::CStatistics::initSetDests(const CrySkinFull* pSkin)
{
	const CrySkinAuxInt* pAux = &pSkin->m_arrAux[0];
	const Vertex* pVertex = &pSkin->m_arrVertices[0];

	arrNumLinks.clear();

	for (unsigned nBone = pSkin->m_numSkipBones; nBone < pSkin->m_numBones; ++nBone)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		const Vertex* pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex)
		{
			unsigned nDest = pVertex->nDest;
			addDest(nDest);
			CRYASSERT(arrNumLinks[nDest] == 1);
		}

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			addDest(nDest);
			CRYASSERT(arrNumLinks[nDest] == 1);
			//pVertex->fWeight is the weight of the vertex
		}

		// process the smooth vertices that were the second/etc time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			addDest(nDest);
			CRYASSERT(arrNumLinks[nDest] > 1);
			// pVertex->fWeight contains the weight of the vertex
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// validates the skin against the given geom info
#if defined (_DEBUG)
void CrySkinFull::validate(const ICrySkinSource* pGeometry)
{
	TElementaryArray<unsigned> arrNumLinks("CrySkinFull::validate.arrNumLinks");
	arrNumLinks.reinit(pGeometry->numVertices(), 0);

	CrySkinAuxInt* pAux = &m_arrAux[0];
	Vertex* pVertex = &m_arrVertices[0];

	for (unsigned nBone = m_numSkipBones; nBone < m_numBones; ++nBone)
	{
		// each bone has a group of vertices

		// first process the rigid vertices
		Vertex* pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex)
		{
			unsigned nDest = pVertex->nDest;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			CRYASSERT(arrNumLinks[nDest] == 0);
			arrNumLinks[nDest] = 1;
			CRYASSERT(rLink.size() == 1);
			CRYASSERT(rLink[0].Blending == 1);
			CRYASSERT(rLink[0].BoneID == nBone);
		}

		// process the smooth1 vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			CRYASSERT(arrNumLinks[nDest]++ == 0);
			CRYASSERT(rLink.size() > 1);
			float fLegacyWeight = rLink.getBoneWeight(nBone);
			CRYASSERT(pVertex->fWeight == fLegacyWeight);
		}

		// process the smooth vertices that were the first time met
		pGroupEnd = pVertex + *pAux++;
		for (; pVertex < pGroupEnd; ++pVertex, ++pAux)
		{
			unsigned nDest = *pAux;
			const CryVertexBinding& rLink = pGeometry->getLink(nDest);
			CRYASSERT(arrNumLinks[nDest]++ > 0);
			CRYASSERT(arrNumLinks[nDest] <= rLink.size());
			CRYASSERT(rLink.size() > 1);
			float fLegacyWeight = rLink.getBoneWeight(nBone);
			CRYASSERT(rLink.hasBoneWeight(nBone, pVertex->fWeight));
		}
	}

	for (unsigned nVert = 0; nVert < pGeometry->numVertices(); ++nVert)
		CRYASSERT(arrNumLinks[nVert] == pGeometry->getLink(nVert).size());
}
#endif
