#pragma once

#include "SimpleTriangleRasterizer.h"					// CSimpleTriangleRasterizer

// This class is to speed up raycasting queries.
// each element in a 2D raster has a pointer (or zero) to a few elements in a second memory(elemets: void *) (0 teminated)
// The raster can be used to query elements on a 3D line - if you project the 3D World down to that raster from one side.
// To get the maximum speedup you shoudl do this from 3 sides.

template <class T>
class CRasterTable
{
public:
	//! constructor
	CRasterTable(void);

	//! destructor
	virtual ~CRasterTable(void);

	//! alloc raster, after that Phase 1 has started
	//! /param indwXSize 1..
	//! /param indwYSize 1..
	//! /return true=success, false otherwise
	bool Init(const DWORD indwXSize, const DWORD indwYSize);

	//!
	//! free data
	void DeInit(void);

	//! Has to be called in Phase 1, after Phase 2 has started
	//! /return true=success, false otherwise
	bool PreProcess(void);

	//! call this per triangle after Init and before PreProcess
	//! after PreProcess call it again for every triangle
	//! /param infX[3] x coordiantes of the 3 vertices in raster units
	//! /param infX[3] y coordiantes of the 3 vertices in raster units
	//! /param inElement element you want to store
	void PutInTriangle(float infX[3], float infY[3], T& inElement);

	//! returns pointer to zero terminated array of pointers
	//! /param iniX 0..m_dwXSize-1
	//! /param iniY 0..m_dwYSize-1
	//! /return
	T* GetElementsAt(int iniX, int iniY);

	//!
	//! /return memory consumption in bytes O(1)
	DWORD CalcMemoryConsumption(void);

	//!
	//! /param inPathFileName filename with path and extension
	void Debug(const char* inPathFileName) const;

	//!
	//! /param outPitchInBytes
	//! /return 
	DWORD* GetDebugData(DWORD& outPitchInBytes) const;

	//!
	//! /return width in raster elements
	DWORD GetWidth(void);

	//!
	//! /return height in raster elements
	DWORD GetHeight(void);

	//! to save memory and make it faster - less cache misses)
	// IF YOU WANNA USE THIS FUNCTION, CLEAR IT FIRST BECAUSE IT SIMPLY DOES NOT WORK
	void Compress(void);

	// **************************************************************

	union
	{
		DWORD_PTR*	m_pDataCounter;		//!< used in Phase 1 [m_dwXSize*m_dwYSize]
		T**			m_pDataPtr;			//!< used in Phase 2 [m_dwXSize*m_dwYSize]
	};

private:
	T*			m_pExtraMemoryPool;		//!< for the pointers to store (zero terminated)  (sorted in ascending order) [m_ExtraMemorySize]
	DWORD		m_ExtraMemorySize;		//!< extra memroy pool size in StoredElementPtr elements

	DWORD		m_dwXSize;				//!< width of the buffer 1.. if allocated, otherwise 0
	DWORD		m_dwYSize;				//!< height of the buffer 1.. if allocated, otherwise 0
	DWORD		m_DataSize;

	//! callback function
	class CPixelIncrement : public CSimpleTriangleRasterizer::IRasterizeSink
	{
	public:

		//! constructor
		CPixelIncrement(DWORD_PTR* inpBuffer, DWORD indwWidth, DWORD indwHeight)
		{
			m_pBuffer = inpBuffer;
			m_dwWidth = indwWidth;
			m_dwHeight = indwHeight;
		}

		//!
		virtual void Line(const float infXLeft, const float infXRight,
			const int iniLeft, const int iniRight, const int iniY)
		{
			CRYASSERT(iniLeft >= 0);
			CRYASSERT(iniY >= 0);
			CRYASSERT(iniRight <= (int)m_dwWidth);
			CRYASSERT(iniY < (int)m_dwHeight);

			DWORD_PTR* pMem = &m_pBuffer[iniLeft + iniY * m_dwWidth];

			for (int x = iniLeft; x < iniRight; x++)
			{
				*pMem = *pMem + 1;
				pMem++;
			}
		}

		DWORD_PTR*	m_pBuffer;				//!<
		DWORD		m_dwWidth;				//!< x= [0,m_dwWidth[
		DWORD		m_dwHeight;				//!< y= [0,m_dwHeight[
	};


	//! callback function
	template <class T>
	class CPixelAddArrayElement : public CSimpleTriangleRasterizer::IRasterizeSink
	{
	public:
		//! constructor
		CPixelAddArrayElement(T** inpBuffer, DWORD indwWidth, DWORD indwHeight, T& inElementPtr)
			: m_pBuffer(inpBuffer)
			, m_dwWidth(indwWidth)
			, m_dwHeight(indwHeight)
			, m_ElementPtr(inElementPtr)
		{
		}

		//!
		virtual void Line(const float infXLeft, const float infXRight, const int iniLeft, const int iniRight, const int iniY)
		{
			const int rowStart = iniY * m_dwWidth;

			CRYASSERT_MSG(rowStart < m_dwWidth * m_dwHeight, "pos out of bounds %d, size %d", iniLeft, m_dwWidth * m_dwHeight);
			CRYASSERT_MSG(iniLeft >= 0 && iniLeft < m_dwWidth, "iniLeft out of bounds %d, width %d", iniLeft, m_dwWidth);
			CRYASSERT_MSG(iniRight >= 0 && iniRight < m_dwWidth, "iniRight out of bounds %d, width %d", iniLeft, m_dwWidth);
			CRYASSERT_MSG(iniY >= 0 && iniY < m_dwHeight, "iniY out of bounds %d, height %d", iniLeft, m_dwHeight);

			for (int x = iniLeft; x < iniRight; ++x)
			{
				--m_pBuffer[rowStart + x];
				*m_pBuffer[rowStart + x] = m_ElementPtr;
			}
		}

		T&		m_ElementPtr;	//!< element to store
		T**		m_pBuffer;		//!< pointer to the array buffer
		DWORD	m_dwWidth;		//!< width of the buffer
		DWORD	m_dwHeight;
	};
};

// constructor
template <class T>
CRasterTable<T>::CRasterTable(void)
{
	m_pDataPtr = 0;
	m_pExtraMemoryPool = 0;
	m_ExtraMemorySize = 0;
}

// destructor
template <class T>
CRasterTable<T>::~CRasterTable(void)
{
	DeInit();
}



// free data
template <class T>
void CRasterTable<T>::DeInit(void)
{
	SAFE_DELETE_ARRAY(m_pDataCounter);
	SAFE_DELETE_ARRAY(m_pExtraMemoryPool);
	m_ExtraMemorySize = 0;
	m_dwXSize = 0;
	m_dwYSize = 0;
}


// alloc raster, after that Phase 1 has started
template <class T>
bool CRasterTable<T>::Init(const DWORD indwXSize, const DWORD indwYSize)
{
	CRYASSERT(!m_pDataCounter);
	CRYASSERT(!m_pExtraMemoryPool);

	DeInit();
	CRYASSERT(indwXSize);
	CRYASSERT(indwYSize);

	m_dwXSize = indwXSize;
	m_dwYSize = indwYSize;
	m_DataSize = m_dwXSize * m_dwYSize;

	m_pDataCounter = new DWORD_PTR[m_DataSize];
	memset(m_pDataCounter, 0, m_DataSize * sizeof(m_pDataCounter[0]));

	return(m_pDataPtr != 0);
}


// Has to be called in Phase1, after that Phase 2 has started
template <class T>
bool CRasterTable<T>::PreProcess(void)
{
	CRYASSERT(m_pDataCounter);
	CRYASSERT(!m_pExtraMemoryPool);
	CRYASSERT(!m_ExtraMemorySize);

	// extra memroy pool size in StoredElementPtr elements
	DWORD dwSum = 0;
	for (DWORD i = 0; i < m_DataSize; i++)
	{
		DWORD_PTR dwNumEl = m_pDataCounter[i];
		if (dwNumEl != 0)
			dwSum += dwNumEl + 1; // Elements + NullTermination
	}

	if (dwSum == 0)
		return true; // no information to store - no problem

	m_pExtraMemoryPool = new T[dwSum];
	m_ExtraMemorySize = dwSum;
	memset(m_pExtraMemoryPool, 0, dwSum * sizeof(m_pExtraMemoryPool[0]));

	// build the access structure for compressing the elements in a vector (pointer is in the beginning on the last element)
	// after filing in the triangles once again the pointer is on the first element
	T** pData = m_pDataPtr;
	T* dest = m_pExtraMemoryPool;

	for (DWORD i = 0; i < m_DataSize; i++)
	{
		DWORD_PTR dwNumEl = m_pDataCounter[i];
		if (dwNumEl != 0)
		{
			dest += dwNumEl;
			*pData++ = dest;
			dest++;	// Elements + NullTermination
		}
		else
			*pData++ = 0;
	}

	return true;
}


// call this per triangle after Init and before PreProcess
// after PreProcess call it again for every triangle
template <class T>
void CRasterTable<T>::PutInTriangle(float infX[3], float infY[3], T& inElement)
{
	float fU[3], fV[3];
	for (int i = 0; i < 3; i++)
	{
		fU[i] = infX[i]; 
		fV[i] = infY[i];
	}

	CSimpleTriangleRasterizer Rasterizer(m_dwXSize, m_dwYSize);
	if (!m_pExtraMemoryPool)
	{
		// Phase 1 
		CPixelIncrement pix(m_pDataCounter, m_dwXSize, m_dwYSize);
		Rasterizer.CallbackFillConservative(fU, fV, &pix);
	}
	else
	{
		// Phase 2
		CPixelAddArrayElement<T> pix(m_pDataPtr, m_dwXSize, m_dwYSize, inElement);
		Rasterizer.CallbackFillConservative(fU, fV, &pix);
	}
}

// returns pointer to zero terminated array of pointers
template <class T>
T* CRasterTable<T>::GetElementsAt(int iniX, int iniY)
{
	CRYASSERT(iniX >= 0);
	CRYASSERT(iniX < (int)m_dwXSize);
	CRYASSERT(iniY >= 0);
	CRYASSERT(iniY < (int)m_dwYSize);

	if (iniX < 0 || iniY < 0 || iniX >= (int)m_dwXSize || iniY >= (int)m_dwYSize)
		return nullptr;

	T* pRet = m_pDataPtr[iniY * m_dwXSize + iniX];
	if (pRet)
	{
		CRYASSERT_MSG(*pRet, "no pointer in the raster should point to empty list");
	}

	return(pRet);
}


// returns pointer to zero terminated array of pointers
// IF YOU WANNA USE THIS FUNCTION, CLEAR IT FIRST BECAUSE IT SIMPLY DOES NOT WORK
template <class T>
void CRasterTable<T>::Compress(void)
{
	if (!m_pExtraMemoryPool)
		return;

	T* pDestStart = 0;
	DWORD dwNewMemorySize = 0;

	for (DWORD i = 0; i < m_DataSize; i++)
	{
		T* pSrcStart = m_pDataPtr[i];	// pointer from the raster element

		if (!pSrcStart)
			continue;			// no elements there

		bool bAddBucket = true;
		T* pSrc = pSrcStart, * pDest = pDestStart;

		if (pDestStart)
		{
			while (*pSrc == *pDest)
			{
				if (*pSrc == 0)
				{
					// buckets are fully identical
					bAddBucket = false;
					break;
				}

				pDest++; pSrc++;
			}
		}

		if (bAddBucket)
		{
			if (!pDestStart)
			{
				pDestStart = m_pExtraMemoryPool;
			}
			else
			{
				while (*pDestStart)
					pDestStart++;
				pDestStart++;
			}

			// copy bucket
			T* pSrc = pSrcStart;
			*pDest = pDestStart;

			while (*pSrc)
			{
				*pDest++ = *pSrc++; 
				dwNewMemorySize++;
			}

			*pDest++ = 0;										// 0 terminate
			CRYASSERT(*pDestStart);					// no pointer in the raster should point to empty list

			m_pDataPtr[i] = pDestStart;
		}
		else
			m_pDataPtr[i] = pDestStart;
	}

	if (!dwNewMemorySize)
		return;

	T* newMem = new T[dwNewMemorySize];
	if (!newMem)
		return;

	// copy in new buffer
	memcpy(newMem, m_pExtraMemoryPool, dwNewMemorySize * sizeof(T));

	// correct the pointers to the new buffer
	for (i = 0; i < m_DataSize; i++)
	{
		if (!m_pDataPtr[i])
			continue;

		CRYASSERT_MSG(*m_pDataPtr[i], "no pointer in the raster should point to empty list");
		m_pDataPtr[i] = (T*)((DWORD_PTR)(m_pDataPtr[i]) - (DWORD_PTR)m_pExtraMemoryPool + (DWORD_PTR)newMem);
		CRYASSERT_MSG(*m_pDataPtr[i], "no pointer in the raster should point to empty list");
	}

	// set new buffer
	delete[] m_pExtraMemoryPool;
	m_pExtraMemoryPool = newMem;

	float fRatio = (float)dwNewMemorySize / (float)m_ExtraMemorySize;
}


// calculates memory consumption in bytes O(1)
template <class T>
DWORD CRasterTable<T>::CalcMemoryConsumption(void)
{
	return (m_DataSize * sizeof(m_pDataCounter[0]) + m_ExtraMemorySize * sizeof(T));
}


template <class T>
void CRasterTable<T>::Debug(const char* inPathFileName) const
{
#ifdef _TGA_LOADER_INCLUDED
	for (DWORD i = 0; i < m_DataSize; i++)
		m_pDataCounter[i] = m_pDataCounter[i] * 10 + 0x0800;

	PIX_SaveTGA32(inPathFileName, (unsigned char*)m_pDataCounter, m_dwXSize, m_dwYSize, false, false);

	for (DWORD i = 0; i < m_DataSize; i++)
		m_pDataCounter[i] = (m_pDataCounter[i] - 0x0800) / 10;
#endif
}

template <class T>
DWORD* CRasterTable<T>::GetDebugData(DWORD& outPitchInBytes) const
{
	outPitchInBytes = m_dwXSize * sizeof(DWORD_PTR);
	return m_pDataCounter;
}

template <class T>
DWORD CRasterTable<T>::GetWidth(void)
{
	return m_dwXSize;
}

template <class T>
DWORD CRasterTable<T>::GetHeight(void)
{
	return m_dwYSize;
}
