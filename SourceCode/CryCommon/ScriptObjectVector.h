// ScriptObjectVector.h: interface for the CScriptObjectVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTVECTOR_H__C72BCA75_0CD6_47F2_9812_177BF43A2018__INCLUDED_)
#define AFX_SCRIPTOBJECTVECTOR_H__C72BCA75_0CD6_47F2_9812_177BF43A2018__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "IScriptSystem.h"

/*! this calss map an 3d vector to a LUA table with x,y,z members
*/
class CScriptObjectVector :
	public _SmartScriptObject
{
public:
	CScriptObjectVector()
	{
	}
	CScriptObjectVector(IScriptSystem* pScriptSystem, bool bCreateEmpty = false)
		: _SmartScriptObject(pScriptSystem, bCreateEmpty)
	{
	}

	void Set(const Vec3& v)
	{
		if (m_pSO->BeginSetGetChain())
		{
			m_pSO->SetValueChain("x", v.x);
			m_pSO->SetValueChain("y", v.y);
			m_pSO->SetValueChain("z", v.z);
			m_pSO->EndSetGetChain();
		}
	}
	Vec3 Get() const
	{
		Vec3 v(0, 0, 0);
		if (m_pSO->BeginSetGetChain())
		{
			m_pSO->GetValueChain("x", v.x);
			m_pSO->GetValueChain("y", v.y);
			m_pSO->GetValueChain("z", v.z);
			m_pSO->EndSetGetChain();
		}
		else CRYASSERT(0 && "validate before calling Get()");

		return v;
	}

	Vec3 operator=(const CScriptObjectVector& vec)
	{
		Set(vec.Get());
		return *this;
	}

	CScriptObjectVector& operator=(const Vec3& v3)
	{
		Set(v3);
		return *this;
	}

	operator Vec3() const
	{
		return Get();
	}
};

/*! this calss map an "color" to a LUA table with x,y,z members
*/
class CScriptObjectColor :
	public _SmartScriptObject
{
public:
	CScriptObjectColor(IScriptSystem* pScriptSystem, bool bCreateEmpty = false) :_SmartScriptObject(pScriptSystem, bCreateEmpty)
	{
	}

	void Set(const Vec3& v)
	{
		m_pSO->SetAt(1, v.x);
		m_pSO->SetAt(2, v.y);
		m_pSO->SetAt(3, v.z);
	}

	Vec3 Get() const
	{
		Vec3 v(0, 0, 0);
		m_pSO->GetAt(1, v.x);
		m_pSO->GetAt(2, v.y);
		m_pSO->GetAt(3, v.z);
		return v;
	}

	Vec3 operator=(const CScriptObjectColor& vec)
	{
		Set(vec.Get());
		return *this;
	}

	CScriptObjectColor& operator=(const Vec3& v3)
	{
		Set(v3);
		return *this;
	}

	operator Vec3() const
	{
		return Get();
	}
};

#endif // !defined(AFX_SCRIPTOBJECTVECTOR_H__C72BCA75_0CD6_47F2_9812_177BF43A2018__INCLUDED_)
