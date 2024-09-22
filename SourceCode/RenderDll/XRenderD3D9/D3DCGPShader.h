/*=============================================================================
  D3DCGPShader.h : Direct3D9 CG pixel shaders interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
	* Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DCGPSHADER_H__
#define __D3DCGPSAHDER_H__

#include <direct.h>

enum CGprofilePS : int
{
	CG_PROFILE_PS_2_0,
	CG_PROFILE_PS_2_X,
	CG_PROFILE_PS_3_0
};

#define CG_FP_CACHE_VER    3.4

#define GL_OFFSET_TEXTURE_2D_MATRIX_NV      0x86E1

class CCGPShader_D3D : public CPShader
{
	SCGScript* m_DeclarationsScript;
	SCGScript* m_CoreScript;
	SCGScript* m_InputParmsScript;
	SCGScript* m_SubroutinesScript;

	bool ActivateCacheItem(SShaderCacheHeaderItem* pItem);
	bool CreateCacheItem(int nMask, byte* pData, int nLen);

	int m_CurInst;
	int m_dwFrame;
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
		void* m_pHandle;

		int m_Mask;
		int m_LightMask;
		CName m_PosScriptName;
		TArray<SCGParam4f>* m_ParamsNoObj;
		TArray<SCGParam4f>* m_ParamsObj;
		TArray<SCGMatrix>* m_MatrixObj;
		TArray<SCGBindConst>* m_BindConstants;
		TArray<SCGBind>* m_BindVars;
		int m_nCacheID;
		SCGInstance()
		{
			m_Mask = 0;
			m_LightMask = 0;
			m_ParamsNoObj = nullptr;
			m_ParamsObj = nullptr;
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
	TArray<SCGParam4f> m_ParamsNoObj;
	TArray<SCGParam4f> m_ParamsObj;

public:
	// FX support
	std::vector<SFXStruct> m_Functions;
	CName m_EntryFunc;

	virtual int Size()
	{
		int nSize = sizeof(*this);
		if (m_DeclarationsScript)
			nSize += m_DeclarationsScript->Size(false);
		if (m_CoreScript)
			nSize += m_CoreScript->Size(false);
		if (m_InputParmsScript)
			nSize += m_InputParmsScript->Size(false);
		if (m_SubroutinesScript)
			nSize += m_SubroutinesScript->Size(false);
		nSize += m_ParamsNoObj.GetMemoryUsage();
		nSize += m_ParamsObj.GetMemoryUsage();
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
		return nSize;
	}
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
		ci.m_pCache = gRenDev->m_cEF.OpenCacheFile(name, (float)CG_FP_CACHE_VER);
		ci.m_Mask = Mask;
		m_InstCache.AddElem(ci);

		return nNum;
	}
	int mfGetCGInstanceID(int Type, int LightMask)
	{
		SCGInstance* cgi;
		if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
		{
			cgi = &m_Insts[m_CurInst];
			if (cgi->m_Mask == Type && cgi->m_LightMask == LightMask)
				return m_CurInst;
		}
		m_dwFrame++;
		int i;
		for (i = 0; i < m_Insts.Num(); i++)
		{
			cgi = &m_Insts[i];
			if (cgi->m_Mask == Type && cgi->m_LightMask == LightMask)
			{
				m_CurInst = i;
				return i;
			}
		}
		SCGInstance cg;
		cg.m_Mask = Type;
		cg.m_LightMask = LightMask;
		cg.m_pHandle = nullptr;
		cg.m_ParamsNoObj = nullptr;
		cg.m_ParamsObj = nullptr;
		cg.m_BindConstants = nullptr;
		cg.m_BindVars = nullptr;
		cg.m_nCacheID = -1;

		m_CurInst = m_Insts.Num();
		m_Insts.AddElem(cg);
		return m_CurInst;
	}

	TArray<SCGParam4f>* mfGetParams(int Type)
	{
		if (!Type)
			return &m_ParamsNoObj;
		else
			return &m_ParamsObj;
	}

	CCGPShader_D3D(CGprofilePS ProfType)
	{
		m_CGProfileType = ProfType;
		mfInit();
	}
	CCGPShader_D3D()
	{
		mfInit();
		m_CGProfileType = CG_PROFILE_PS_2_0;
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
	void mfInit()
	{
		m_dwFrame = 1;
		m_CurInst = -1;
		m_CoreScript = nullptr;
		m_InputParmsScript = nullptr;
		m_SubroutinesScript = nullptr;
		m_DeclarationsScript = nullptr;
		m_bCGType = true;
	}

	void mfBind();
	void mfUnbind();

	char* mfLoadCG_Int(char* prog_text);

	char* mfLoadCG(char* prog_text)
	{
		const int renderFeatures = gRenDev->GetFeatures();
		// Test adding source text to context
		char* pOut = mfLoadCG_Int(prog_text);
		if (!pOut
			&& (m_Flags & PSFI_AUTOENUMTC) && m_CGProfileType != CG_PROFILE_PS_3_0 && m_CGProfileType != CG_PROFILE_PS_2_X
			&& (renderFeatures & RFT_HW_PS20))
		{
			if ((renderFeatures & RFT_HW_MASK) == RFT_HW_GFFX)
				m_CGProfileType = CG_PROFILE_PS_2_X;
			else
				m_CGProfileType = CG_PROFILE_PS_2_0;

			pOut = mfLoadCG_Int(prog_text);
			if (!pOut && (renderFeatures & RFT_HW_PS30))
			{
				m_CGProfileType = CG_PROFILE_PS_3_0;
				pOut = mfLoadCG_Int(prog_text);
			}
		}
		if (!pOut)
		{
			m_nFailed++;
			Warning(0, 0, "Couldn't create CG pixel program '%s'", m_Name.c_str());
			mfSaveCGFile(prog_text);
		}
		return pOut;
	}

	const char* mfSkipLine(const char* prog)
	{
		while (*prog != 0xa) { prog++; }
		prog++;
		return prog;
	}
	const char* mfGetLine(const char* prog, char* line)
	{
		while (*prog == 0x20 || *prog == 0x9) { prog++; }
		while (*prog != 0xa) { *line++ = *prog; prog++; }
		*line = 0;
		prog++;
		return prog;
	}

	const char* mfGetTexInstruction(const char* prog, int& Op0, int& Op1)
	{
		Op0 = -1;
		Op1 = -1;
		char line[128];
		const char* str = prog;
		while (str = mfGetLine(str, line))
		{
			if (!line[0])
				continue;
			if (!strncmp(line, "def", 3))
				continue;
			if (strncmp(line, "tex", 3) != 0)
				return nullptr;
			int n = 0;
			while (line[n] != 0x20 && line[n] != 0x9 && line[n] != 0) { n++; }
			if (line[n] == 0)
				return str;
			while (line[n] == 0x20 || line[n] == 0x9) { n++; }
			if (line[n] == 0)
				return str;
			CRYASSERT(line[n] == 't');
			Op0 = atoi(&line[n + 1]);
			n += 2;
			while (line[n] != 0x20 && line[n] != 0x9 && line[n] != 0) { n++; }
			if (line[n] == 0)
				return str;
			while (line[n] == 0x20 || line[n] == 0x9) { n++; }
			if (line[n] == 0)
				return str;
			CRYASSERT(line[n] == 't');
			Op1 = atoi(&line[n + 1]);
			return str;
		}
		return nullptr;
	}
	LPD3DXBUFFER mfLoad(const char* prog_text)
	{
		// Assemble and create pixel shader
		HRESULT hr;
		LPD3DXBUFFER pCode;
		LPD3DXBUFFER pBuffer = nullptr;
		char* pNewText = nullptr;
		if (m_Insts[m_CurInst].m_Mask & VPVST_CLIPPLANES3)
		{
			const char* str;
			if (str = strstr(prog_text, "ps.1."))
			{
				int Op0, Op1;
				str = mfSkipLine(str);
				while (!strncmp(str, "def", 3))
				{
					str = mfSkipLine(str);
				}
				const char* sLastStr = str;
				while (str = mfGetTexInstruction(str, Op0, Op1))
				{
					if (Op0 == 3 || Op1 == 3)
						break;
					sLastStr = str;
				}
				if (!str)
				{
					size_t size = strlen(prog_text) + strlen("texkill t3\n") + 1;
					pNewText = new char[size];
					memcpy(pNewText, prog_text, sLastStr - prog_text);
					strcpy(&pNewText[sLastStr - prog_text], "texkill t3\n");
					strcpy(&pNewText[sLastStr - prog_text + strlen("texkill t3\n")], &prog_text[sLastStr - prog_text]);
				}
			}
		}
		if (pNewText)
			hr = D3DXAssembleShader(pNewText, strlen(pNewText), nullptr, nullptr, 0, &pCode, &pBuffer);
		else
			hr = D3DXAssembleShader(prog_text, strlen(prog_text), nullptr, nullptr, 0, &pCode, &pBuffer);
		if (FAILED(hr))
		{
			Warning(0, 0, "WARNING: CCGPShader_D3D::mfLoad: Could not assemble pixel shader '%s'(0x%x) (%s)\n", m_Name.c_str(), m_nMaskGen, gcpRendD3D->D3DError(hr));
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
				Warning(0, 0, "WARNING: CCGPShader_D3D::mfLoad: Shader script error (%s)\n", strOut);
				SAFE_RELEASE(pBuffer);
				SAFE_DELETE_ARRAY(pNewText);
			}
			return nullptr;
		}
		if (pCode && !(m_Flags & PSFI_PRECACHEPHASE))
			hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), (IDirect3DPixelShader9**)&m_Insts[m_CurInst].m_pHandle);
		SAFE_RELEASE(pBuffer);
		SAFE_DELETE_ARRAY(pNewText);
		if (FAILED(hr))
		{
			Warning(0, 0, "CCGPShader_D3D::mfLoad: Could not create pixel shader '%s'(0x%x) (%s)\n", m_Name.c_str(), m_nMaskGen, gcpRendD3D->D3DError(hr));
			return nullptr;
		}
		return pCode;
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
				iLog->Log("Warning: couldn't find parameter %s for pixel shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
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
			gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstantI(n, iparms, 1);
		}
		v += 4;
	}

	void mfParameter(SCGBind* ParamBind, const float* v, int nComps)
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
						CRYASSERT(p->m_nComponents <= nComps);
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
				iLog->Log("Warning: couldn't find parameter %s for pixel shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
		}
		if ((int)ParamBind->m_dwBind == -1)
			return;
		if ((ParamBind->m_dwBind & 0xffff) == GL_OFFSET_TEXTURE_2D_MATRIX_NV)
		{
			int tmu = (ParamBind->m_dwBind >> 28) - 1;
			float parm[4];
			float fScaleX = CRenderer::CV_r_embm;
			float fScaleY = CRenderer::CV_r_embm;
			if (gcpRendD3D->m_bHackEMBM && gcpRendD3D->m_RP.m_TexStages[tmu].Texture && gcpRendD3D->m_RP.m_TexStages[tmu].Texture->m_eTT == eTT_Rectangle)
			{
				fScaleX *= (float)gcpRendD3D->GetWidth();
				fScaleY *= (float)gcpRendD3D->GetHeight();
			}
			parm[0] = v[0] * fScaleX;
			parm[1] = v[1] * fScaleX;
			parm[2] = v[2] * fScaleY;
			parm[3] = v[3] * fScaleY;
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT00, FLOATtoDWORD(parm[0]));
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT01, FLOATtoDWORD(parm[1]));
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT10, FLOATtoDWORD(parm[2]));
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT11, FLOATtoDWORD(parm[3]));
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVLSCALE, FLOATtoDWORD(4.0f));
			gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVLOFFSET, FLOATtoDWORD(0.0f));
		}
		else
		{
			int nR;
			if (ParamBind->m_dwBind == 65536)
				nR = 0;
			else
				nR = ParamBind->m_dwBind;
			for (int i = 0; i < ParamBind->m_nBindComponents; i++)
			{
				int n = nR + i;
				if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
				{
					m_CurParams[n][0] = v[0];
					m_CurParams[n][1] = v[1];
					m_CurParams[n][2] = v[2];
					m_CurParams[n][3] = v[3];
					gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstantF(n, v, 1);
				}
				v += 4;
			}
		}
	}
	void mfParameter4i(SCGBind* ParamBind, const float* v)
	{
		mfParameteri(ParamBind, v);
	}
	void mfParameter4f(SCGBind* ParamBind, const float* v)
	{
		mfParameter(ParamBind, v, 1);
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
	void mfParameter4f(const char* Name, const float* v)
	{
		SCGBind* pBind = mfGetParameterBind(Name);
		if (pBind)
			mfParameter4f(pBind, v);
	}

	virtual void mfEnable()
	{
	}
	virtual void mfDisable()
	{
	}

	void mfFree();

	void mfDelInst()
	{
		if (m_Insts[m_CurInst].m_pHandle && (INT_PTR)m_Insts[m_CurInst].m_pHandle != -1)
		{
			if (m_Insts[m_CurInst].m_BindConstants)
				delete m_Insts[m_CurInst].m_BindConstants;
			if (m_Insts[m_CurInst].m_BindVars)
				delete m_Insts[m_CurInst].m_BindVars;
			if (m_Insts[m_CurInst].m_ParamsNoObj)
				delete m_Insts[m_CurInst].m_ParamsNoObj;
			IDirect3DPixelShader9* pVS = (IDirect3DPixelShader9*)m_Insts[m_CurInst].m_pHandle;
			SAFE_RELEASE(pVS);
		}
		m_Insts[m_CurInst].m_pHandle = nullptr;
	}

	void mfSetVariables(TArray<SCGParam4f>* Parms);

	bool mfIsValid(int Num) const { return (m_Insts[m_CurInst].m_pHandle != nullptr); }
	void mfGetSrcFileName(char* srcname, int nSize);
	void mfGetDstFileName(char* dstname, int nSize, bool bUseASCIICache);
	void mfPrecacheLights(int nMask);

public:
	char* mfGenerateScriptPS();
	char* mfCreateAdditionalPS();
	void mfCompileParam4f(char* scr, SShader* ef, TArray<SCGParam4f>* Params);
	bool mfActivate();
	void mfConstructFX(std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char* entryFunc);
	void mfAddFXParameter(SFXParam* pr, const char* ParamName, SShader* ef);
	bool mfGetFXParamNameByID(int nParam, char* ParamName);

public:
	virtual ~CCGPShader_D3D();
	virtual void Release();
	virtual bool mfCompile(char* scr);
	virtual bool mfSet(bool bStat, SShaderPassHW* slw = nullptr, int nFlags = 0);
	virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
	virtual void mfReset();
	virtual void mfPrecache();
	virtual bool mfIsCombiner() { return false; }
	virtual void mfGatherFXParameters(const char* buf, SShaderPassHW* pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader* ef);
	virtual void mfPostLoad();

	static vec4_t m_CurParams[32];
};


#endif  // __GLCGPSHADER_H__
