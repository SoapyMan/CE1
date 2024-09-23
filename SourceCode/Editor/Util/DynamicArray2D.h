// DynamicArray2D.h: Interface of the class CDynamicArray.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYNAMICARRAY_H__3B36BCC2_AB88_11D1_8F4C_D0B67CC10B05__INCLUDED_)
#define AFX_DYNAMICARRAY_H__3B36BCC2_AB88_11D1_8F4C_D0B67CC10B05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDynamicArray2D  
{
public:
	CDynamicArray2D(uint iDimension1, uint iDimension2);
	void ScaleImage(CDynamicArray2D *pDestination);
	virtual ~CDynamicArray2D();
	float **m_Array;

private:
	uint m_Dimension1;
	uint m_Dimension2;
};

#endif // !defined(AFX_DYNAMICARRAY_H__3B36BCC2_AB88_11D1_8F4C_D0B67CC10B05__INCLUDED_)
