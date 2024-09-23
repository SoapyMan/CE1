#ifndef _STATICIB_H_
#define _STATICIB_H_

/////////////////////////////
// D. Sim Dietrich Jr.
// sim.dietrich@nvidia.com
//////////////////////

template < class IndexType > class StaticIB
{
private:

	LPDIRECT3DINDEXBUFFER9 mpIB;

	uint mIndexCount;
	bool    mbLocked;
	IndexType* m_pLockedData;

public:

	uint GetIndexCount() const
	{
		return mIndexCount;
	}

	StaticIB(const LPDIRECT3DDEVICE9 pD3D, const uint& theIndexCount)
	{
		mpIB = 0;
		mbLocked = false;

		mIndexCount = theIndexCount;

		HRESULT hr = pD3D->CreateIndexBuffer(mIndexCount * sizeof(IndexType), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &mpIB, nullptr);

		CRYASSERT((hr == D3D_OK) && (mpIB));
	}

	LPDIRECT3DINDEXBUFFER9 GetInterface() const { return mpIB; }

	IndexType* Lock(const uint& theLockCount, uint& theStartIndex)
	{
		// Ensure there is enough space in the IB for this data
		CRYASSERT(theLockCount <= mIndexCount);

		if (mbLocked)
			return m_pLockedData;

		if (mpIB)
		{
			DWORD dwFlags = D3DLOCK_DISCARD;
			DWORD dwSize = 0;

			HRESULT hr = mpIB->Lock(0, 0, reinterpret_cast<void**>(&m_pLockedData), dwFlags);

			CRYASSERT(hr == D3D_OK);
			CRYASSERT(m_pLockedData != 0);
			mbLocked = true;
			theStartIndex = 0;
		}

		return m_pLockedData;
	}

	void Unlock()
	{
		if ((mbLocked) && (mpIB))
		{
			HRESULT hr = mpIB->Unlock();
			CRYASSERT(hr == D3D_OK);
			mbLocked = false;
		}
	}

	~StaticIB()
	{
		Unlock();
		if (mpIB)
		{
			mpIB->Release();
		}
	}

};

typedef StaticIB< ushort > StaticIB16;
typedef StaticIB< uint > StaticIB32;

#endif  _STATICIB_H_
