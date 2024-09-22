#ifndef _LEAFBUFFER_H_
#define _LEAFBUFFER_H_

class CIndexedMesh;
struct SMRendTexVert;
union UCol;
struct GeomInfo;

//! structure for tangent basises storing
struct TangData
{
	Vec3 tangent;
	Vec3 binormal;
	Vec3 tnormal;
};

struct CLeafBuffer
{
	using PrepareBufferCb = bool (*)(CLeafBuffer* Data, bool bNeedTangents);

	//! constructor
	//! /param szSource this pointer is stored - make sure the memory stays
	CLeafBuffer(const char* szSource);

	//! destructor
	~CLeafBuffer();

	// --------------------------------------------------------------

	static CLeafBuffer	m_RootGlobal;
	static CLeafBuffer	m_Root;

	CLeafBuffer*		m_Next;
	CLeafBuffer*		m_Prev;
	CLeafBuffer*		m_NextGlobal;
	CLeafBuffer*		m_PrevGlobal;

	_inline void Unlink()
	{
		if (!m_Next || !m_Prev)
			return;
		m_Next->m_Prev = m_Prev;
		m_Prev->m_Next = m_Next;
		m_Next = m_Prev = nullptr;
	}
	_inline void Link(CLeafBuffer* Before)
	{
		if (m_Next || m_Prev)
			return;
		m_Next = Before->m_Next;
		Before->m_Next->m_Prev = this;
		Before->m_Next = this;
		m_Prev = Before;
	}

	_inline void UnlinkGlobal()
	{
		if (!m_NextGlobal || !m_PrevGlobal)
			return;
		m_NextGlobal->m_PrevGlobal = m_PrevGlobal;
		m_PrevGlobal->m_NextGlobal = m_NextGlobal;
		m_NextGlobal = m_PrevGlobal = nullptr;
	}
	_inline void LinkGlobal(CLeafBuffer* Before)
	{
		if (m_NextGlobal || m_PrevGlobal)
			return;
		m_NextGlobal = Before->m_NextGlobal;
		Before->m_NextGlobal->m_PrevGlobal = this;
		Before->m_NextGlobal = this;
		m_PrevGlobal = Before;
	}

	_inline void SetVertexContainer(CLeafBuffer* pBuf)
	{
		m_pVertexContainer = pBuf;
	}
	_inline CLeafBuffer* GetVertexContainer(void)
	{
		if (m_pVertexContainer)
			return m_pVertexContainer;
		return this;
	}

	_inline byte* GetNormalPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		byte* pData;
		SBufInfoTable* pOffs;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
		}
		if (pOffs->OffsNormal)
		{
			return &pData[Id * Stride + pOffs->OffsNormal];
		}

		Stride = sizeof(Vec3);
		return (byte*)&lb->m_TempNormals[Id];
	}

	_inline byte* GetPosPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
			Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
			Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
		}
		return &pData[Id * Stride];
	}

	_inline byte* GetBinormalPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		Stride = sizeof(SPipTangents);
		return &pData[Id * Stride + 12];
	}
	_inline byte* GetTangentPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		Stride = sizeof(SPipTangents);
		return &pData[Id * Stride];
	}
	_inline byte* GetTNormalPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
		}
		Stride = sizeof(SPipTangents);
		return &pData[Id * Stride + 24];
	}

	_inline byte* GetColorPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		SBufInfoTable* pOffs;
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
		}
		if (pOffs->OffsColor)
		{
			return &pData[Id * Stride + pOffs->OffsColor];
		}

		Stride = sizeof(UCol);
		return (byte*)&lb->m_TempColors[Id];
	}

	_inline byte* GetSecColorPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		SBufInfoTable* pOffs;
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
			Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
		}
		if (pOffs->OffsSecColor)
		{
			return &pData[Id * Stride + pOffs->OffsSecColor];
		}
		return nullptr;
	}

	_inline byte* GetUVPtr(int& Stride, int Id = 0, bool bSys = true)
	{
		CLeafBuffer* lb = GetVertexContainer();
		SBufInfoTable* pOffs;
		byte* pData;
		if (bSys)
		{
			pData = (byte*)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
			Stride = m_VertexSize[m_pSecVertBuffer->m_vertexformat];
		}
		else
		{
			pData = (byte*)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
			pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
			Stride = m_VertexSize[m_pVertexBuffer->m_vertexformat];
		}
		if (pOffs->OffsTC)
		{
			return &pData[Id * Stride + pOffs->OffsTC];
		}

		Stride = sizeof(SMRendTexVert);
		return (byte*)&lb->m_TempTexCoords[Id];
	}

	char*				m_sSource;	//!< pointer to the give name in the constructor call

	Vec3*				m_TempNormals;					
	SMRendTexVert*		m_TempTexCoords;	
	UCol*				m_TempColors;
	UCol*				m_TempSecColors;
	Vec3*				m_pLoadedColors;
	void*				m_pCustomData;			//!< userdata, useful for PrepareBufferCallback, currently this is used only for CDetailGrass

	PrepareBufferCb		m_prepareBufferCb;
	CVertexBuffer*		m_pVertexBuffer;		//!< video memory
	CLeafBuffer*		m_pVertexContainer;		//!<
	CVertexBuffer*		m_pSecVertBuffer;		//!< system memory

	list2<ushort>*		m_pIndicesPreStrip;
	uint*				m_arrVertStripMap;		//!<
	list2<CMatInfo>*	m_pMats;				//!<
	uint*				m_arrVtxMap;			//!< [Anton] mapping table leaf buffer vertex idx->original vertex idx

	uint				m_UpdateVBufferMask;	//!<
	uint                m_UpdateFrame;
	uint                m_SortFrame;			//!< to prevent unneccessary sorting during one frame
	int					m_SecVertCount;			//!< number of vertices in m_pSecVertBuffer

	SVertexStream       m_Indices;
	TArray <ushort>     m_SecIndices;
	int                 m_NumIndices;

	int					m_nVertexFormat;
	int					m_nPrimetiveType;		//!< R_PRIMV_TRIANGLES, R_PRIMV_TRIANGLE_STRIP, R_PRIMV...

	float				m_fMinU, m_fMinV, m_fMaxU, m_fMaxV;	//!< only needed for fur rendering (room for improvement)

	Vec3				m_vBoxMin, m_vBoxMax;	//!< used for hw occlusion test

	int					m_nClientTextureBindID;
	bool				m_bMaterialsWasCreatedInRenderer : 1;
	bool				m_bOnlyVideoBuffer : 1;
	bool				m_bDynamic : 1;

	static int SetTexType(struct TextureMap3* tm);

	
	//! /param Flags ????
	//! /return memory consumption of the object in bytes
	int		Size(int Flags);
	void	SerializeMemBuffer(uchar* pSerialBuffer, uchar* pData, int nSize, bool bSaveToBuffer);
	bool	LoadMaterial(int m, const char* szFileName, const char* szFolderName, list2<CMatInfo>& lstMatTable, IRenderer* pRenderer, MAT_ENTITY* me, bool bFake);
	bool	IsEmpty() { return !m_SecVertCount || !m_pSecVertBuffer || !m_SecIndices.Num(); }

	// compact buffer
	int		FindInBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F& opt, SPipTangents& origBasis, uint nMatInfo, uint* uiInfo, struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F* _vbuff, SPipTangents* _vbasis, int _vcount, list2<unsigned short>* pHash, TArray<uint>& ShareNewInfo);
	void	CompactBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F* _vbuff, SPipTangents* _tbuff, int* _vcount, TArray<unsigned short>* pindices, bool bShareVerts[128], uint* uiInfo);

	void	SaveTexCoords(byte* pD, SBufInfoTable* pOffs, int Size);
	void	SaveColors(byte* pD, SBufInfoTable* pOffs, int Size);
	void	SaveSecColors(byte* pD, SBufInfoTable* pOffs, int Size);
	void	SaveNormals(byte* pD, SBufInfoTable* pOffs, int Size);

	//! /param StripType e.g. STRIPTYPE_NONE,STRIPTYPE_ONLYLISTS,STRIPTYPE_SINGLESTRIP,STRIPTYPE_MULTIPLESTRIPS,STRIPTYPE_DEFAULT (in IRenderer.h)
	void	StripifyMesh(int inStripType);
	void	SortTris();

	bool	ReCreateSystemBuffer(int VertFormat);
	void	UpdateDynBufPtr(int VertFormat);

	virtual void CalcFaceNormals(void);
	virtual void AddRE(CCObject* pObj, IShader* pEf, int nNumSort = 0, IShader* pStateEff = 0);

	virtual bool CheckUpdate(int VertFormat, int Flags, bool bNeedAddNormals);
	virtual bool CreateTangBuffer();
	virtual void ReleaseShaders();
	virtual void InvalidateVideoBuffer(int flags = -1) { m_UpdateVBufferMask |= flags; };
	virtual void CopyTo(CLeafBuffer* pDst, bool bUseSysBuf);

	virtual void CreateBuffer(CIndexedMesh* pTriData, bool bStripifyAndShareVerts, bool bRemoveNoDrawFaces = true, bool bKeepRemapTable = false);
	virtual bool CreateBuffer(struct VertexBufferSource* pSource);

	virtual int GetAllocatedBytes(bool bVideoMem);

	virtual void Unload();
	virtual void UpdateCustomLighting(float fBackSideLevel, Vec3 vStatObjAmbientColor, const Vec3& vLight, bool bCalcLighting);

	virtual void AddRenderElements(CCObject* pObj = 0, int DLightMask = 0, int nTemplate = -1, int nFogVolumeID = 0, int nSortId = 0, IMatInfo* pIMatInfo = nullptr);

	virtual unsigned short* GetIndices(int* pIndicesCount);
	virtual void DestroyIndices();

	virtual void UpdateVidIndices(const ushort* pNewInds, int nInds);
	virtual void UpdateSysIndices(const ushort* pNewInds, int nInds);

	virtual void UpdateSysVertices(void* pNewVertices, int nNewVerticesCount);
	virtual void UpdateVidVertices(void* pNewVertices, int nNewVerticesCount);
	virtual bool CreateSysVertices(int nVerts, int VertFormat);
	virtual void CreateVidVertices(int nVerts, int VertFormat);

	virtual void SetRECustomData(float* pfCustomData, float fFogScale = 0, float fAlpha = 1);
	virtual void SetChunk(IShader* pShader, int nFirstVertId, int nVertCount, int nFirstIndexId, int nIndexCount, int nMatID = 0, bool bForceInitChunk = false);

	virtual void SetShader(IShader* pShader, int nCustomTID = 0);

	virtual void* GetSecVerticesPtr(int* pVerticesCount);

	virtual void AllocateSystemBuffer(int nVertCount);
	virtual void FreeSystemBuffer(void);
	virtual bool Serialize(int& nPos, uchar* pSaveBuffer, bool bSaveToBuffer,char* szFolderName, char* szFileName, double& dCIndexedMesh__LoadMaterial);

	virtual void DrawImmediately(void);
	virtual bool UpdateTangBuffer(SPipTangents* pBasis);

	virtual void RenderDebugLightPass(const Matrix44& mat, int nLightMask, float fAlpha);
	virtual void Render(const struct SRendParams& rParams, CCObject* pObj, TArray<int>& ShaderTemplates, int e_overlay_geometry, bool bNotCurArea, IMatInfo* pMaterial, bool bSupportDefaultShaderTemplates);
};

#endif // _LEAFBUFFER_H_
