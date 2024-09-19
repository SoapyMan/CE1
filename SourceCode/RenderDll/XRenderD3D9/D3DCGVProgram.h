/*=============================================================================
  D3DCGVProgram.h : Direct3D CG programs interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
	* Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DCGVPROGRAM_H__
#define __D3DCGVPROGRAM_H__

#include <Cg/cgD3D9.h>
#include <direct.h>

#define CG_VP_CACHE_VER    3.4

#define VSCONST_0_025_05_1 28
#define VSCONST_FOG 29

#define PSCONST_HDR_FOGCOLOR 31

class CCGVProgram_D3D : public CVProgram
{
	SCGScript* m_Script;
	SCGScript* m_PosScript;
	SCGScript* m_SubroutinesScript;
	SCGScript* m_DeclarationsScript;
	SCGScript* m_CoreScript;
	SCGScript* m_InputParmsScript;

	// FX support
	std::vector<SFXStruct> m_Functions;
	CName m_EntryFunc;

	bool ActivateCacheItem(SShaderCacheHeaderItem* pItem);
	bool CreateCacheItem(int nMask, byte* pData, int nLen);

	TArray<SCGParam4f> m_ParamsNoObj;
	TArray<SCGParam4f> m_ParamsObj;
	TArray<SCGMatrix> m_MatrixObj;
	TArray<SArrayPointer*> m_Pointers;

	int m_CurInst;
	struct SCacheInstance
	{
		int m_Mask;
		SShaderCache* m_pCache;
		SCacheInstance()
		{
			m_Mask = 0;
			m_pCache = nullptr;
		}
		int Size()
		{
			int nSize = sizeof(*this);
			return nSize;
		}
	};

	struct SCGInstance
	{
		int m_Mask;
		int m_LightMask;
		CName m_PosScriptName;
		TArray<SCGParam4f>* m_ParamsNoObj;
		TArray<SCGMatrix>* m_MatrixNoObj;
		TArray<SCGParam4f>* m_ParamsObj;
		TArray<SCGMatrix>* m_MatrixObj;
		TArray<SCGBindConst>* m_BindConstants;
		TArray<SCGBind>* m_BindVars;
		int m_nCacheID;
		union
		{
			CGprogram m_CGProgram;
			void* m_pHandle;
		};
		SCGInstance()
		{
			m_Mask = 0;
			m_LightMask = 0;
			m_ParamsNoObj = nullptr;
			m_ParamsObj = nullptr;
			m_MatrixNoObj = nullptr;
			m_MatrixObj = nullptr;
			m_BindConstants = nullptr;
			m_BindVars = nullptr;
			m_pHandle = nullptr;
			m_nCacheID = -1;
		}
		int Size()
		{
			int nSize = sizeof(*this);
			if (m_ParamsNoObj)
				nSize += m_ParamsNoObj->GetMemoryUsage() + 12;
			if (m_ParamsObj)
				nSize += m_ParamsObj->GetMemoryUsage() + 12;
			if (m_MatrixNoObj)
				nSize += m_MatrixNoObj->GetMemoryUsage() + 12;
			if (m_MatrixObj)
				nSize += m_MatrixObj->GetMemoryUsage() + 12;
			if (m_BindConstants)
				nSize += m_BindConstants->GetMemoryUsage() + 12;
			if (m_BindVars)
				nSize += m_BindVars->GetMemoryUsage() + 12;

			return nSize;
		}
	};
	TArray<SCGInstance> m_Insts;
	TArray<SCacheInstance> m_InstCache;

public:

	int m_dwFrame;
	TArray<SCGParam4f> m_Params_Inst;

	virtual int Size()
	{
		int nSize = sizeof(*this);
		for (int i = 0; i < m_Insts.GetSize(); i++)
		{
			if (i < m_Insts.Num())
			{
				SCGInstance* cgi = &m_Insts[i];
				nSize += cgi->Size();
			}
			else
				nSize += sizeof(SCGInstance);
		}
		if (m_Script)
			nSize += m_Script->Size(false);
		if (m_PosScript)
			nSize += m_PosScript->Size(false);
		if (m_SubroutinesScript)
			nSize += m_SubroutinesScript->Size(false);
		if (m_DeclarationsScript)
			nSize += m_DeclarationsScript->Size(false);
		if (m_CoreScript)
			nSize += m_CoreScript->Size(false);
		if (m_InputParmsScript)
			nSize += m_InputParmsScript->Size(false);
		nSize += m_ParamsObj.GetMemoryUsage();
		nSize += m_ParamsNoObj.GetMemoryUsage();
		nSize += m_MatrixObj.GetMemoryUsage();

		return nSize;
	}

	void mfSaveCGFile(const char* scr)
	{
		if (CRenderer::CV_r_shaderssave < 1)
			return;
		char name[1024];
		if (m_nMaskGen)
			sprintf(name, "%s$%x(%x).cg", m_Name.c_str(), m_Insts[m_CurInst].m_LightMask, m_nMaskGen);
		else
			sprintf(name, "%s.cg", m_Name.c_str());
		FILE* fp = fopen(name, "w");
		if (fp)
		{
			fprintf(fp, scr);
			fclose(fp);
		}
	}

	CCGVProgram_D3D()
	{
		mfInit();
		m_CGProfileType = CG_PROFILE_VS_1_1;
	}
	void mfInit()
	{
#if defined(USE_CG)
		if (!gcpRendD3D->m_CGContext)
		{
			cgD3D9SetDevice(gcpRendD3D->mfGetD3DDevice());
			gcpRendD3D->m_CGContext = cgCreateContext();
			CRYASSERT(gcpRendD3D->m_CGContext);
#ifdef _DEBUG
			cgD3D9EnableDebugTracing(true);
#endif
		}
#endif
		m_dwFrame = 1;
		m_CurInst = -1;
		m_Script = nullptr;
		m_SubroutinesScript = nullptr;
		m_CoreScript = nullptr;
		m_InputParmsScript = nullptr;
		m_PosScript = nullptr;
		m_DeclarationsScript = nullptr;
		m_bCGType = true;
	}
	void mfFree();

	void mfGetSrcFileName(char* srcname, int nSize);
	void mfGetDstFileName(char* dstname, int nSize, bool bUseASCIICache);

	TArray<SCGParam4f>* mfGetParams(int Type)
	{
		if (!Type)
			return &m_ParamsNoObj;
		else
			return &m_ParamsObj;
	}

	void mfBind();

	void mfUnbind();

	int mfGetCacheInstanceID(int Mask, const char* name = nullptr)
	{
		int i;

		for (i = 0; i < m_InstCache.Num(); i++)
		{
			if (m_InstCache[i].m_Mask == Mask)
				return i;
		}
		if (!name)
			return -1;
		int nNum = m_InstCache.Num();
		SCacheInstance ci;
		ci.m_pCache = gRenDev->m_cEF.OpenCacheFile(name, (float)CG_VP_CACHE_VER);
		ci.m_Mask = Mask;
		m_InstCache.AddElem(ci);

		return nNum;
	}
	int mfGetCGInstanceID(int Type, CVProgram* pPosVP, int LightMask)
	{
		CCGVProgram_D3D* pVP = (CCGVProgram_D3D*)pPosVP;
		SCGInstance* cgc;
		CName PosName;
		SCGScript* posScr = pVP->m_PosScript;
		if (posScr)
			PosName = posScr->m_Name;
		else
			PosName = CName("None");
		if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
		{
			cgc = &m_Insts[m_CurInst];
			if (cgc->m_Mask == Type && cgc->m_LightMask == LightMask)
			{
				if (!pVP || !posScr || PosName == cgc->m_PosScriptName)
					return m_CurInst;
			}
		}
		m_dwFrame++;
		int i;
		for (i = 0; i < m_Insts.Num(); i++)
		{
			cgc = &m_Insts[i];
			if (cgc->m_Mask == Type && cgc->m_LightMask == LightMask && (!pVP || !posScr || PosName == cgc->m_PosScriptName))
			{
				m_CurInst = i;
				return i;
			}
		}
		SCGInstance cg;
		cg.m_Mask = Type;
		cg.m_LightMask = LightMask;
		cg.m_BindConstants = nullptr;
		cg.m_BindVars = nullptr;
		cg.m_pHandle = 0;
		cg.m_CGProgram = nullptr;
		cg.m_ParamsNoObj = nullptr;
		cg.m_MatrixNoObj = nullptr;
		cg.m_ParamsObj = nullptr;
		cg.m_MatrixObj = nullptr;
		if (Type & VPVST_CLIPPLANES3)
		{
			if (!cg.m_ParamsNoObj)
				cg.m_ParamsNoObj = new TArray<SCGParam4f>;
			SCGParam4f pr;
			pr.m_nComponents = 1;
			pr.m_Name = "ClipPlane";
			SParamComp_ClipPlane p;
			pr.m_Flags = PF_CANMERGED;
			pr.m_Comps[0] = SParamComp::mfAdd(&p);
			cg.m_ParamsNoObj->AddElem(pr);
		}
		if (Type & VPVST_TCMASK)
		{
			if (!cg.m_ParamsNoObj)
				cg.m_ParamsNoObj = new TArray<SCGParam4f>;
			int nm = VPVST_TCM0;
			int ngol = VPVST_TCGOL0;
			int ngrm = VPVST_TCGRM0;
			int ngsm = VPVST_TCGSM0;
			int ngnm = VPVST_TCGNM0;
			for (int i = 0; i < 4; i++)
			{
				char str[128];
				if (Type & (ngol << i))
				{
					SCGParam4f pr;
					pr.m_nComponents = 4;
					pr.m_dwBind = 0;
					sprintf(str, "MatrixTCG%d", i);
					pr.m_Name = str;
					SParamComp_MatrixTCG p;
					pr.m_Flags = PF_CANMERGED;
					p.m_Stage = i;
					p.m_Row = 0;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 1;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 2;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 3;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);
				}
				if (Type & ((ngrm | ngsm) << i))
				{
					SCGParam4f pr;
					pr.m_nComponents = 1;
					pr.m_dwBind = 0;
					pr.m_Name = "CameraPos";
					SParamComp_OSCameraPos p;
					pr.m_Flags = PF_CANMERGED;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					if (!cg.m_MatrixNoObj)
						cg.m_MatrixNoObj = new TArray<SCGMatrix>;
					SCGMatrix m;
					m.m_eCGParamType = ECGP_Matr_View;
					m.m_Name = "ModelView";
					cg.m_MatrixNoObj->AddElem(m);
					m.m_eCGParamType = ECGP_Matr_View_IT;
					m.m_Name = "ModelViewIT";
					cg.m_MatrixNoObj->AddElem(m);
				}
				if (Type & (ngnm << i))
				{
					if (!cg.m_MatrixNoObj)
						cg.m_MatrixNoObj = new TArray<SCGMatrix>;
					SCGMatrix m;
					m.m_eCGParamType = ECGP_Matr_View_IT;
					m.m_Name = "ModelViewIT";
					cg.m_MatrixNoObj->AddElem(m);
				}
				if (Type & (nm << i))
				{
					SCGParam4f pr;
					pr.m_nComponents = 4;
					pr.m_dwBind = 0;
					sprintf(str, "MatrixTCM%d", i);
					pr.m_Name = str;
					SParamComp_MatrixTCM p;
					pr.m_Flags = PF_CANMERGED;
					p.m_Stage = i;

					p.m_Row = 0;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 1;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 2;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);

					p.m_Row = 3;
					pr.m_Comps[0] = SParamComp::mfAdd(&p);
					cg.m_ParamsNoObj->AddElem(pr);
				}
			}
		}

		if (pVP && posScr)
		{
			cg.m_PosScriptName = PosName;
			if (cg.m_PosScriptName != m_PosScript->m_Name && pVP->m_ParamsNoObj.Num())
			{
				if (!cg.m_ParamsNoObj)
					cg.m_ParamsNoObj = new TArray<SCGParam4f>;
				for (int i = 0; i < pVP->m_ParamsNoObj.Num(); i++)
				{
					cg.m_ParamsNoObj->AddElem(pVP->m_ParamsNoObj[i]);
					cg.m_ParamsNoObj->Get(cg.m_ParamsNoObj->Num() - 1).m_dwBind = 0;
				}
				cg.m_ParamsNoObj->Shrink();
			}
			if (cg.m_PosScriptName != m_PosScript->m_Name && pVP->m_ParamsObj.Num())
			{
				if (!cg.m_ParamsObj)
					cg.m_ParamsObj = new TArray<SCGParam4f>;
				for (int i = 0; i < pVP->m_ParamsObj.Num(); i++)
				{
					cg.m_ParamsObj->AddElem(pVP->m_ParamsObj[i]);
					cg.m_ParamsObj->Get(cg.m_ParamsObj->Num() - 1).m_dwBind = 0;
				}
				cg.m_ParamsObj->Shrink();
			}
		}
		if (cg.m_ParamsNoObj)
		{
			TArray<SCGParam4f> ObjP;
			gRenDev->m_cEF.mfCheckObjectDependParams(cg.m_ParamsNoObj, &ObjP);
			if (ObjP.Num())
			{
				cg.m_ParamsObj = new TArray<SCGParam4f>;
				cg.m_ParamsObj->Copy(ObjP);
			}
		}
		m_Insts.AddElem(cg);
		m_CurInst = m_Insts.Num() - 1;
		return m_CurInst;
	}

	char* mfGetObjectCode(CGprogram cgPr)
	{
		const char* code = cgGetProgramString(cgPr, CG_COMPILED_PROGRAM);
		size_t size = strlen(code) + 1;
		char* str = new char[size];
		cryMemcpy(str, code, size);
		return str;
	}

	char* mfLoadCG(const char* prog_text);

	LPD3DXBUFFER mfLoad(const char* prog_text)
	{
		// Load and create vertex shader
		HRESULT hr;
		LPD3DXBUFFER pCode;
		LPD3DXBUFFER pBuffer = nullptr;
		hr = D3DXAssembleShader(prog_text, strlen(prog_text), nullptr, nullptr, 0, &pCode, &pBuffer);
		if (FAILED(hr))
		{
			Warning(0, 0, "WARNING: CCGVProgram_D3D::mfLoad: Could not assemble vertex shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
			if (pBuffer != nullptr)
			{
				TCHAR* pstr;
				TCHAR strOut[4096];
				TCHAR* pstrOut;
				// Need to replace \n with \r\n so edit box shows newlines properly
				pstr = (TCHAR*)pBuffer->GetBufferPointer();
				strOut[0] = '\0';
				pstrOut = strOut;
				for (int i = 0; i < 4096; i++)
				{
					if (*pstr == '\n')
						*pstrOut++ = '\r';
					*pstrOut = *pstr;
					if (*pstr == '\0')
						break;
					if (i == 4095)
						*pstrOut = '\0';
					pstrOut++;
					pstr++;
				}
				// remove any blank lines at the end
				while (strOut[lstrlen(strOut) - 1] == '\n' || strOut[lstrlen(strOut) - 1] == '\r')
				{
					strOut[lstrlen(strOut) - 1] = '\0';
				}
				Warning(0, 0, "WARNING: CCGVProgram_D3D::mfLoad: Shader script error (%s)\n", strOut);
				SAFE_RELEASE(pBuffer);
			}
		}
		if (pCode && !(m_Flags & VPFI_PRECACHEPHASE))
			hr = gcpRendD3D->mfGetD3DDevice()->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), (IDirect3DVertexShader9**)&m_Insts[m_CurInst].m_pHandle);
		if (FAILED(hr))
		{
			Warning(0, 0, "CCGVProgram_D3D::mfLoad: Could not create vertex shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
			return nullptr;
		}
		return pCode;
	}

	SCGBind* mfGetParameterBind(const char* Name)
	{
		CName nm = CName(Name, eFN_Add);
		if (!m_Insts[m_CurInst].m_BindVars)
			m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;

		int i;
		for (i = 0; i < m_Insts[m_CurInst].m_BindVars->Num(); i++)
		{
			if (nm == m_Insts[m_CurInst].m_BindVars->Get(i).m_Name)
				return &m_Insts[m_CurInst].m_BindVars->Get(i);
		}
		return nullptr;
	}

	void mfParameteri(SCGBind* ParamBind, const float* v)
	{
		int i;
		if (!ParamBind)
			return;

		if (!ParamBind->m_dwBind || ParamBind->m_dwFrameCreated != m_dwFrame)
		{
			ParamBind->m_dwFrameCreated = m_dwFrame;
			if (m_Insts[m_CurInst].m_BindVars)
			{
				for (i = 0; i < m_Insts[m_CurInst].m_BindVars->Num(); i++)
				{
					SCGBind* p = &m_Insts[m_CurInst].m_BindVars->Get(i);
					if (p->m_Name == ParamBind->m_Name)
					{
						ParamBind->m_dwBind = p->m_dwBind;
						ParamBind->m_nBindComponents = p->m_nComponents;
						if (!ParamBind->m_dwBind)
							ParamBind->m_dwBind = 65536;
						break;
					}
				}
				if (i == m_Insts[m_CurInst].m_BindVars->Num())
					ParamBind->m_dwBind = -1;
			}
			else
				ParamBind->m_dwBind = -1;
			if (ParamBind->m_dwBind == -1 && CRenderer::CV_r_shaderssave >= 2)
				iLog->Log("Warning: couldn't find parameter %s for vertex shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
		}
		if ((int)ParamBind->m_dwBind == -1)
			return;
		int n;
		int iparms[4];
		if (ParamBind->m_dwBind == 65536)
			n = 0;
		else
			n = ParamBind->m_dwBind;
		if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
		{
			m_CurParams[n][0] = v[0];
			m_CurParams[n][1] = v[1];
			m_CurParams[n][2] = v[2];
			m_CurParams[n][3] = v[3];

			iparms[0] = (int)v[0];
			iparms[1] = (int)v[1];
			iparms[2] = (int)v[2];
			iparms[3] = (int)v[3];
			gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstantI(n, iparms, 1);
		}
		v += 4;
	}

	void mfParameter(SCGBind* ParamBind, const float* v, int nComps)
	{
		if (!ParamBind)
		{
			return;
		}

		if (!ParamBind->m_dwBind || ParamBind->m_dwFrameCreated != m_dwFrame)
		{
			ParamBind->m_dwFrameCreated = m_dwFrame;
			if (m_Insts[m_CurInst].m_BindVars)
			{
				int i;
				for (i = 0; i < m_Insts[m_CurInst].m_BindVars->Num(); i++)
				{
					SCGBind* p = &m_Insts[m_CurInst].m_BindVars->Get(i);
					if (p->m_Name == ParamBind->m_Name)
					{
						CRYASSERT(p->m_nComponents <= nComps);
						ParamBind->m_dwBind = p->m_dwBind;
						if (ParamBind->m_dwBind == 0)
							ParamBind->m_dwBind = 65536;
						ParamBind->m_nBindComponents = p->m_nComponents;
						break;
					}
				}
				if (i == m_Insts[m_CurInst].m_BindVars->Num())
					ParamBind->m_dwBind = -1;
			}
			else
				ParamBind->m_dwBind = -1;
			if (ParamBind->m_dwBind == -1 && CRenderer::CV_r_shaderssave >= 2)
				iLog->Log("Warning: couldn't find parameter %s for vertex shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
		}
		if (ParamBind->m_dwBind == -1)
			return;
		if (ParamBind->m_dwBind == 65536)
		{
			gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstantF(0, v, 4);
			//memcpy(&m_CurParams[0][0], v, 4*4*sizeof(float));
		}
		else
		{
			for (int i = 0; i < ParamBind->m_nBindComponents; i++)
			{
				int n = ParamBind->m_dwBind + i;
				if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
				{
					m_CurParams[n][0] = v[0];
					m_CurParams[n][1] = v[1];
					m_CurParams[n][2] = v[2];
					m_CurParams[n][3] = v[3];
					gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstantF(n, v, 1);
				}
				v += 4;
			}
		}
	}

	void mfParameter4i(SCGBind* ParamBind, const vec4_t v)
	{
		mfParameteri(ParamBind, v);
	}

	void mfParameter4f(SCGBind* ParamBind, const vec4_t v)
	{
		mfParameter(ParamBind, v, 1);
	}
	void mfParameter4f(const char* Name, const float* v)
	{
		SCGBind* pBind = mfGetParameterBind(Name);
		if (pBind)
			mfParameter4f(pBind, v);
	}


	void mfParameterStateMatrix(SCGMatrix* ParamBind)
	{
		CD3D9Renderer* r = gcpRendD3D;

		LPDIRECT3DDEVICE9 dv = r->mfGetD3DDevice();
		D3DXMATRIXA16 matWorldViewProj;
		switch (ParamBind->m_eCGParamType)
		{
		case ECGP_Matr_ViewProj:
		{
			if (!(r->m_RP.m_ObjFlags & FOB_TRANS_MASK))
			{
				if (r->m_RP.m_PersFlags & RBPF_WASWORLDSPACE)
					break;
				r->m_RP.m_PersFlags |= RBPF_WASWORLDSPACE;
				m_FrameObj = -1;
			}
			else
			{
				if (r->m_RP.m_PersFlags & RBPF_WASWORLDSPACE)
					m_FrameObj = -1;
				r->m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;
			}

			if (m_FrameObj != r->m_RP.m_FrameObject)
			{
				m_FrameObj = r->m_RP.m_FrameObject;
				mathMatrixTranspose((float*)&matWorldViewProj, r->m_RP.m_pCurObject->GetVPMatrix().GetData(), g_CpuFlags);
				mfParameter(ParamBind, (float*)&matWorldViewProj, 4);
			}
		}
		break;
		case ECGP_Matr_World:
		{
			D3DXMatrixIdentity(&matWorldViewProj);
			mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
		}
		break;
		case ECGP_Matr_View_IT:
		{
			D3DXMatrixMultiply((D3DXMATRIXA16*)&r->m_ViewMatrix(0, 0), (D3DXMATRIXA16*)&r->m_RP.m_pCurObject->m_Matrix(0, 0), (D3DXMATRIXA16*)&r->m_CameraMatrix(0, 0));
			D3DXMATRIXA16* matView = (D3DXMATRIXA16*)r->m_ViewMatrix.GetData();
			D3DXMatrixInverse(&matWorldViewProj, nullptr, matView);
			mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
		}
		break;
		case ECGP_Matr_View:
		{
			D3DXMatrixMultiply((D3DXMATRIXA16*)&r->m_ViewMatrix(0, 0), (D3DXMATRIXA16*)&r->m_RP.m_pCurObject->m_Matrix(0, 0), (D3DXMATRIXA16*)&r->m_CameraMatrix(0, 0));
			D3DXMATRIXA16* matView = (D3DXMATRIXA16*)r->m_ViewMatrix.GetData();
			D3DXMatrixTranspose(&matWorldViewProj, matView);
			mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
		}
		break;
		case ECGP_Matr_View_I:
		{
			D3DXMatrixMultiply((D3DXMATRIXA16*)&r->m_ViewMatrix(0, 0), (D3DXMATRIXA16*)&r->m_RP.m_pCurObject->m_Matrix(0, 0), (D3DXMATRIXA16*)&r->m_CameraMatrix(0, 0));
			D3DXMATRIXA16* matView = (D3DXMATRIXA16*)r->m_ViewMatrix.GetData();
			D3DXMatrixInverse(&matWorldViewProj, nullptr, matView);
			D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
			mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
		}
		break;
		case ECGP_Matr_View_T:
		{
			D3DXMatrixMultiply((D3DXMATRIXA16*)&r->m_ViewMatrix(0, 0), (D3DXMATRIXA16*)&r->m_RP.m_pCurObject->m_Matrix(0, 0), (D3DXMATRIXA16*)&r->m_CameraMatrix(0, 0));
			D3DXMATRIXA16* matView = (D3DXMATRIXA16*)r->m_ViewMatrix.GetData();
			mfParameter(ParamBind, (float*)matView, 4);
		}
		break;
		case ECGP_Matr_Obj:
		{
			Matrix44* m = &gRenDev->m_RP.m_pCurObject->m_Matrix;
			mfParameter(ParamBind, m->GetData(), 4);
		}
		break;
		case ECGP_Matr_Obj_I:
		{
			Matrix44* m = &gRenDev->m_RP.m_pCurObject->GetInvMatrix();
			mfParameter(ParamBind, m->GetData(), 4);
		}
		break;
		default:
			Warning(0, 0, "Unknown matrix state type %d int CG program '%s'", m_Name.c_str());
			CRYASSERT(0);
		}
	}


	void mfDelInst()
	{
		if (m_Insts[m_CurInst].m_CGProgram && (INT_PTR)m_Insts[m_CurInst].m_CGProgram != -1)
		{
			if (m_Insts[m_CurInst].m_BindConstants)
				delete m_Insts[m_CurInst].m_BindConstants;
			if (m_Insts[m_CurInst].m_BindVars)
				delete m_Insts[m_CurInst].m_BindVars;
			if (m_Insts[m_CurInst].m_ParamsNoObj)
				delete m_Insts[m_CurInst].m_ParamsNoObj;
			if (m_Insts[m_CurInst].m_ParamsObj)
				delete m_Insts[m_CurInst].m_ParamsObj;
			if (m_Insts[m_CurInst].m_MatrixNoObj)
				delete m_Insts[m_CurInst].m_MatrixNoObj;
			if (m_Insts[m_CurInst].m_MatrixObj)
				delete m_Insts[m_CurInst].m_MatrixObj;
			IDirect3DVertexShader9* pVS = (IDirect3DVertexShader9*)m_Insts[m_CurInst].m_pHandle;
			if (pVS)
			{
				pVS->Release();
				m_Insts[m_CurInst].m_pHandle = nullptr;
			}
		}
		m_Insts[m_CurInst].m_CGProgram = nullptr;
	}

	bool mfIsValid(int Num) const { return (m_Insts[Num].m_CGProgram != nullptr); }
	SCGScript* mfGenerateScriptVP(CVProgram* pPosVP);
	char* mfGenerateTCScript(char* Script, int nt);
	void mfCompileVertAttributes(char* scr, SShader* ef);
	void mfSetVariables(TArray<SCGParam4f>* Vars);
	void mfPrecacheLights(int nMask);

public:
	char* mfCreateAdditionalVP(CVProgram* pPosVP);
	void mfCompileParam4f(char* scr, SShader* ef, TArray<SCGParam4f>* Params);
	void mfCompileParamStateMatrix(char* scr, SShader* ef, TArray<SCGMatrix>* Params);
	bool mfActivate(CVProgram* pPosVP);

	void mfConstructFX(std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char* entryFunc);
	void mfAddFXParameter(SFXParam* pr, const char* ParamName, SShader* ef);
	bool mfGetFXParamNameByID(int nParam, char* ParamName);

public:
	virtual ~CCGVProgram_D3D();
	virtual void Release();
	virtual bool mfCompile(char* scr);
	virtual bool mfSet(bool bStat, SShaderPassHW* slw = nullptr, int nSetPointers = 1);
	virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
	virtual void mfReset();
	virtual void mfPrecache();
	virtual bool mfHasPointer(ESrcPointer ePtr);
	virtual void mfGatherFXParameters(const char* buf, SShaderPassHW* pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader* ef);
	virtual void mfPostLoad();
	virtual int  mfVertexFormat(bool& bUseTangents, bool& bUseLM);
	void mfSetStateMatrices()
	{
		//PROFILE_FRAME(Shader_VShadersMatr);
		for (SCGMatrix& tm : m_MatrixObj)
			mfParameterStateMatrix(&tm);

		if (m_Insts[m_CurInst].m_MatrixObj)
		{
			for (SCGMatrix& tm : *m_Insts[m_CurInst].m_MatrixObj)
				mfParameterStateMatrix(&tm);
		}
	}
	static int m_nResetDeviceFrame;
	static void mfSetGlobalParams();


	static SCGScript* mfAddNewScript(const char* Name, const char* Script)
	{
		int i;
		if (Name)
		{
			CName nm = CName(Name);
			if (nm.GetIndex())
			{
				for (i = 0; i < m_CGScripts.Num(); i++)
				{
					SCGScript* scr = m_CGScripts[i];
					if (!scr || !scr->m_Name.GetIndex())
						continue;
					if (nm == scr->m_Name)
					{
						if (!Script || stricmp(Script, scr->m_Script) == 0)
							return scr;
						delete[] scr->m_Script;
						break;
					}
				}
			}
		}
		if (!Script)
		{
			if (Name)
				Warning(0, 0, "Error: CCGVProgram_GL::mfAddNewScript: Couldn't find CG script for name '%s'", Name);
			return nullptr;
		}

		SCGScript* scr = new SCGScript;
		if (Name)
			scr->m_Name = CName(Name, eFN_Add);

		size_t len = strlen(Script) + 1;
		scr->m_Script = new char[len];
		strcpy(scr->m_Script, Script);

		if (Name)
			m_CGScripts.AddElem(scr);

		return scr;
	}
	static void mfDeleteSharedScripts()
	{
		int i;

		for (i = 0; i < m_CGScripts.Num(); i++)
		{
			SCGScript* scr = m_CGScripts[i];
			if (!scr)
				continue;
			SAFE_DELETE_ARRAY(scr->m_Script);
			delete scr;
			m_CGScripts[i] = nullptr;
		}
	}

	static TArray<SCGScript*> m_CGScripts;
	static vec4_t m_CurParams[256];
};


#endif  // __D3DCGVPROGRAMS_H__
