////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjshadow.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: shadow maps
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "StatObj.h"
#include "../RenderDll/Common/Shadow_Renderer.h"

void CStatObj::PrepareShadowMaps(const Vec3d& obj_pos, ShadowMapLightSource* pLSource)
{
	if (!GetCVars()->e_shadow_maps)
		return;

	int nTexSize = GetCVars()->e_max_shadow_map_size;
	while (nTexSize > GetRadius() * 200)
		nTexSize /= 2;

	if (nTexSize < 16)
		nTexSize = 16; // in case of error

	pLSource->m_LightFrustums.Reset();

	// define new frustum
	pLSource->m_LightFrustums.Add({});

	ShadowMapFrustum& lof = pLSource->m_LightFrustums[pLSource->m_LightFrustums.Count()-1];
	lof.pOwnerGroup = this;
	lof.nTexSize = nTexSize;
	GetRenderer()->MakeShadowMapFrustum(lof, pLSource, obj_pos + GetCenter()/*Vec3d(0,0,GetCenterZ())*/, this, EST_DEPTH_BUFFER);

	// make depth textures
	//for( f=0; f < pLSource->m_LightFrustums.Count(); f++)
	//  GetRenderer()->PrepareDepthMap(&pLSource->m_LightFrustums[f], true);
}

void CStatObj::MakeShadowMaps(const Vec3d vSunPos)
{
	if (!GetCVars()->e_shadow_maps)
		return;

	if(!m_pSMLSource)
		m_pSMLSource = new ShadowMapLightSource;
	m_pSMLSource->nDLightId = -1;
	m_pSMLSource->fRadius = 500000;
	m_pSMLSource->vSrcPos = GetNormalized(vSunPos) * 10000;//0;
	PrepareShadowMaps(Vec3d(0, 0, 0), m_pSMLSource);
}
