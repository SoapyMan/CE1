//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:AABBSV.h
//	Description: shadow volume AABB functionality for overlap testings
//
//	History:
//	-Feb 15,2004:Created by Michael Glueck, code provided by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef AABBSV_H
#define AABBSV_H

#if _MSC_VER > 1000
# pragma once
#endif

struct Shadowvolume
{
	uint32 sideamount;
	uint32 nplanes;

	Plane oplanes[12];
};

namespace NAABB_SV
{
//***************************************************************************************
//***************************************************************************************
//***             Calculate a ShadowVolume using an AABB and a point-light            ***
//***************************************************************************************
//***  The planes of the AABB facing away from the point-light are the far-planes     ***
//***  of the ShadowVolume. There can be 3-6 far-planes.                              ***
//***************************************************************************************
void AABB_ReceiverShadowVolume(const Vec3& PointLight, const AABB& Occluder, Shadowvolume& sv);

//***************************************************************************************
//***************************************************************************************
//***             Calculate a ShadowVolume using an AABB and a point-light            ***
//***************************************************************************************
//***  The planes of the AABB facing the point-light are the near-planes of the       ***
//***  the ShadowVolume. There can be 1-3 near-planes.                                ***
//***  The far-plane is defined by lightrange.                                        ***
//***************************************************************************************
void AABB_ShadowVolume(const Vec3& PointLight, const AABB& Occluder, Shadowvolume& sv, f32 lightrange);

//***************************************************************************************
//***   this is the "fast" version to check if an AABB is overlapping a shadowvolume  ***
//***************************************************************************************
bool Is_AABB_In_ShadowVolume(const Shadowvolume& sv, const AABB& Receiver);

//***************************************************************************************
//***   this is the "hierarchical" check                                              ***
//***************************************************************************************
char Is_AABB_In_ShadowVolume_hierarchical(const Shadowvolume& sv, const AABB& Receiver);
}

#endif

