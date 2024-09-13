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

#include "platform.h"
#include "Cry_Geo.h"
#include "AABBSV.h"
#include "Cry_Camera.h"


BYTE BoxSides[64 * 8] = {
	0,0,0,0, 0,0,0,0, //00
	0,4,6,2, 0,0,0,4, //01
	7,5,1,3, 0,0,0,4, //02
	0,0,0,0, 0,0,0,0, //03
	0,1,5,4, 0,0,0,4, //04
	0,1,5,4, 6,2,0,6, //05
	7,5,4,0, 1,3,0,6, //06
	0,0,0,0, 0,0,0,0, //07
	7,3,2,6, 0,0,0,4, //08
	0,4,6,7, 3,2,0,6, //09
	7,5,1,3, 2,6,0,6, //0a
	0,0,0,0, 0,0,0,0, //0b
	0,0,0,0, 0,0,0,0, //0c
	0,0,0,0, 0,0,0,0, //0d
	0,0,0,0, 0,0,0,0, //0e
	0,0,0,0, 0,0,0,0, //0f
	0,2,3,1, 0,0,0,4, //10
	0,4,6,2, 3,1,0,6, //11
	7,5,1,0, 2,3,0,6, //12
	0,0,0,0, 0,0,0,0, //13
	0,2,3,1, 5,4,0,6, //14
	1,5,4,6, 2,3,0,6, //15
	7,5,4,0, 2,3,0,6, //16
	0,0,0,0, 0,0,0,0, //17
	0,2,6,7, 3,1,0,6, //18
	0,4,6,7, 3,1,0,6, //19
	7,5,1,0, 2,6,0,6, //1a
	0,0,0,0, 0,0,0,0, //1b
	0,0,0,0, 0,0,0,0, //1c
	0,0,0,0, 0,0,0,0, //1d
	0,0,0,0, 0,0,0,0, //1e
	0,0,0,0, 0,0,0,0, //1f
	7,6,4,5, 0,0,0,4, //20
	0,4,5,7, 6,2,0,6, //21
	7,6,4,5, 1,3,0,6, //22
	0,0,0,0, 0,0,0,0, //23
	7,6,4,0, 1,5,0,6, //24
	0,1,5,7, 6,2,0,6, //25
	7,6,4,0, 1,3,0,6, //26
	0,0,0,0, 0,0,0,0, //27
	7,3,2,6, 4,5,0,6, //28
	0,4,5,7, 3,2,0,6, //29
	6,4,5,1, 3,2,0,6, //2a
	0,0,0,0, 0,0,0,0, //2b
	0,0,0,0, 0,0,0,0, //2c
	0,0,0,0, 0,0,0,0, //2d
	0,0,0,0, 0,0,0,0, //2e
	0,0,0,0, 0,0,0,0, //2f
	0,0,0,0, 0,0,0,0, //30
	0,0,0,0, 0,0,0,0, //31
	0,0,0,0, 0,0,0,0, //32
	0,0,0,0, 0,0,0,0, //33
	0,0,0,0, 0,0,0,0, //34
	0,0,0,0, 0,0,0,0, //35
	0,0,0,0, 0,0,0,0, //36
	0,0,0,0, 0,0,0,0, //37
	0,0,0,0, 0,0,0,0, //38
	0,0,0,0, 0,0,0,0, //39
	0,0,0,0, 0,0,0,0, //3a
	0,0,0,0, 0,0,0,0, //3b
	0,0,0,0, 0,0,0,0, //3c
	0,0,0,0, 0,0,0,0, //3d
	0,0,0,0, 0,0,0,0, //3e
	0,0,0,0, 0,0,0,0, //3f
};


void NAABB_SV::AABB_ReceiverShadowVolume(const Vec3& PointLight, const AABB& Occluder, Shadowvolume& sv)
{
	sv.sideamount = 0;
	sv.nplanes = 0;

	//------------------------------------------------------------------------------
	//-- check if PointLight is in front of any occluder plane or inside occluder --
	//------------------------------------------------------------------------------
	uint32 front = 0;
	if (PointLight.x < Occluder.min.x)  front |= 0x01;
	if (PointLight.x > Occluder.max.x)  front |= 0x02;
	if (PointLight.y < Occluder.min.y)  front |= 0x04;
	if (PointLight.y > Occluder.max.y)  front |= 0x08;
	if (PointLight.z < Occluder.min.z)  front |= 0x10;
	if (PointLight.z > Occluder.max.z)  front |= 0x20;

	sv.sideamount = BoxSides[(front << 3) + 7];

	uint32 back = front ^ 0x3f;
	if (back & 0x01) { sv.oplanes[sv.nplanes].SetPlane(Vec3(-1, +0, +0), Occluder.min); sv.nplanes++; }
	if (back & 0x02) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+1, +0, +0), Occluder.max); sv.nplanes++; }
	if (back & 0x04) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, -1, +0), Occluder.min); sv.nplanes++; }
	if (back & 0x08) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +1, +0), Occluder.max); sv.nplanes++; }
	if (back & 0x10) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +0, -1), Occluder.min); sv.nplanes++; }
	if (back & 0x20) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +0, +1), Occluder.max); sv.nplanes++; }

	if (front == 0) return;		//light is inside occluder

	//all 8 vertices of a AABB
	Vec3 o[8] =
	{
		Vec3(Occluder.min.x,Occluder.min.y,Occluder.min.z),
			Vec3(Occluder.max.x,Occluder.min.y,Occluder.min.z),
			Vec3(Occluder.min.x,Occluder.max.y,Occluder.min.z),
			Vec3(Occluder.max.x,Occluder.max.y,Occluder.min.z),
			Vec3(Occluder.min.x,Occluder.min.y,Occluder.max.z),
			Vec3(Occluder.max.x,Occluder.min.y,Occluder.max.z),
			Vec3(Occluder.min.x,Occluder.max.y,Occluder.max.z),
			Vec3(Occluder.max.x,Occluder.max.y,Occluder.max.z)
	};

	//---------------------------------------------------------------------
	//---      find the silhouette-vertices of the occluder-AABB        ---
	//---------------------------------------------------------------------
	uint32 p0 = BoxSides[(front << 3) + 0];
	uint32 p1 = BoxSides[(front << 3) + 1];
	uint32 p2 = BoxSides[(front << 3) + 2];
	uint32 p3 = BoxSides[(front << 3) + 3];
	uint32 p4 = BoxSides[(front << 3) + 4];
	uint32 p5 = BoxSides[(front << 3) + 5];

	if (sv.sideamount == 4)
	{
		sv.oplanes[sv.nplanes + 0] = GetPlane(o[p1], o[p0], PointLight);
		sv.oplanes[sv.nplanes + 1] = GetPlane(o[p2], o[p1], PointLight);
		sv.oplanes[sv.nplanes + 2] = GetPlane(o[p3], o[p2], PointLight);
		sv.oplanes[sv.nplanes + 3] = GetPlane(o[p0], o[p3], PointLight);
	}

	if (sv.sideamount == 6)
	{
		sv.oplanes[sv.nplanes + 0] = GetPlane(o[p1], o[p0], PointLight);
		sv.oplanes[sv.nplanes + 1] = GetPlane(o[p2], o[p1], PointLight);
		sv.oplanes[sv.nplanes + 2] = GetPlane(o[p3], o[p2], PointLight);
		sv.oplanes[sv.nplanes + 3] = GetPlane(o[p4], o[p3], PointLight);
		sv.oplanes[sv.nplanes + 4] = GetPlane(o[p5], o[p4], PointLight);
		sv.oplanes[sv.nplanes + 5] = GetPlane(o[p0], o[p5], PointLight);
	}
}

void NAABB_SV::AABB_ShadowVolume(const Vec3& PointLight, const AABB& Occluder, Shadowvolume& sv, f32 lightrange)
{
	sv.sideamount = 0;
	sv.nplanes = 0;

	//------------------------------------------------------------------------------
	//-- check if PointLight is in front of any occluder plane or inside occluder --
	//------------------------------------------------------------------------------
	uint32 front = 0;
	if (PointLight.x < Occluder.min.x)  front |= 0x01;
	if (PointLight.x > Occluder.max.x)  front |= 0x02;
	if (PointLight.y < Occluder.min.y)  front |= 0x04;
	if (PointLight.y > Occluder.max.y)  front |= 0x08;
	if (PointLight.z < Occluder.min.z)  front |= 0x10;
	if (PointLight.z > Occluder.max.z)  front |= 0x20;
	if (front == 0) return; //light is inside occluder
	sv.sideamount = BoxSides[(front << 3) + 7];

	if (front & 0x01) { sv.oplanes[sv.nplanes].SetPlane(Vec3(-1, +0, +0), Occluder.min); sv.nplanes++; }
	if (front & 0x02) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+1, +0, +0), Occluder.max); sv.nplanes++; }
	if (front & 0x04) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, -1, +0), Occluder.min); sv.nplanes++; }
	if (front & 0x08) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +1, +0), Occluder.max); sv.nplanes++; }
	if (front & 0x10) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +0, -1), Occluder.min); sv.nplanes++; }
	if (front & 0x20) { sv.oplanes[sv.nplanes].SetPlane(Vec3(+0, +0, +1), Occluder.max); sv.nplanes++; }

	//all 8 vertices of a AABB
	Vec3 o[8] =
	{
		Vec3(Occluder.min.x,Occluder.min.y,Occluder.min.z),
			Vec3(Occluder.max.x,Occluder.min.y,Occluder.min.z),
			Vec3(Occluder.min.x,Occluder.max.y,Occluder.min.z),
			Vec3(Occluder.max.x,Occluder.max.y,Occluder.min.z),
			Vec3(Occluder.min.x,Occluder.min.y,Occluder.max.z),
			Vec3(Occluder.max.x,Occluder.min.y,Occluder.max.z),
			Vec3(Occluder.min.x,Occluder.max.y,Occluder.max.z),
			Vec3(Occluder.max.x,Occluder.max.y,Occluder.max.z)
	};

	//---------------------------------------------------------------------
	//---      find the silhouette-vertices of the occluder-AABB        ---
	//---------------------------------------------------------------------
	uint32 p0 = BoxSides[(front << 3) + 0];
	uint32 p1 = BoxSides[(front << 3) + 1];
	uint32 p2 = BoxSides[(front << 3) + 2];
	uint32 p3 = BoxSides[(front << 3) + 3];
	uint32 p4 = BoxSides[(front << 3) + 4];
	uint32 p5 = BoxSides[(front << 3) + 5];

	//the new center-position in world-space
	Vec3 MiddleOfOccluder = (Occluder.max + Occluder.min) * 0.5f;
	sv.oplanes[sv.nplanes] = GetPlane(GetNormalized(MiddleOfOccluder - PointLight), GetNormalized(MiddleOfOccluder - PointLight) * lightrange + PointLight);
	sv.nplanes++;

	if (sv.sideamount == 4)
	{
		sv.oplanes[sv.nplanes + 0] = GetPlane(o[p1], o[p0], PointLight);
		sv.oplanes[sv.nplanes + 1] = GetPlane(o[p2], o[p1], PointLight);
		sv.oplanes[sv.nplanes + 2] = GetPlane(o[p3], o[p2], PointLight);
		sv.oplanes[sv.nplanes + 3] = GetPlane(o[p0], o[p3], PointLight);
	}

	if (sv.sideamount == 6)
	{
		sv.oplanes[sv.nplanes + 0] = GetPlane(o[p1], o[p0], PointLight);
		sv.oplanes[sv.nplanes + 1] = GetPlane(o[p2], o[p1], PointLight);
		sv.oplanes[sv.nplanes + 2] = GetPlane(o[p3], o[p2], PointLight);
		sv.oplanes[sv.nplanes + 3] = GetPlane(o[p4], o[p3], PointLight);
		sv.oplanes[sv.nplanes + 4] = GetPlane(o[p5], o[p4], PointLight);
		sv.oplanes[sv.nplanes + 5] = GetPlane(o[p0], o[p5], PointLight);
	}
}

bool NAABB_SV::Is_AABB_In_ShadowVolume(const Shadowvolume& sv, const AABB& Receiver)
{
	uint32 pa = sv.sideamount + sv.nplanes;

	f32 d;
	const Vec3* pAABB = &Receiver.min;

	//------------------------------------------------------------------------------
	//----    check if receiver-AABB is in front of any of these planes       ------
	//------------------------------------------------------------------------------
	for (uint32 x = 0; x < pa; x++)
	{
		d = sv.oplanes[x].d;
		d += sv.oplanes[x].n.x * pAABB[(*((uint32*)&sv.oplanes[x].n.x) >> 31)].x;
		d += sv.oplanes[x].n.y * pAABB[(*((uint32*)&sv.oplanes[x].n.y) >> 31)].y;
		d += sv.oplanes[x].n.z * pAABB[(*((uint32*)&sv.oplanes[x].n.z) >> 31)].z;
		if (d > 0) return CULL_EXCLUSION;
	}
	return CULL_OVERLAP;
}

char NAABB_SV::Is_AABB_In_ShadowVolume_hierarchical(const Shadowvolume& sv, const AABB& Receiver)
{
	uint32 pa = sv.sideamount + sv.nplanes;
	const Vec3* pAABB = &Receiver.min;

	f32 dot1, dot2;
	uint32 notOverlap = 0x80000000; // will be reset to 0 if there's at least one overlapping

	//------------------------------------------------------------------------------
	//----    check if receiver-AABB is in front of any of these planes       ------
	//------------------------------------------------------------------------------
	for (uint32 x = 0; x < pa; x++)
	{
		dot1 = dot2 = sv.oplanes[x].d;
		dot1 += sv.oplanes[x].n.x * pAABB[0 + (*((uint32*)&sv.oplanes[x].n.x) >> 31)].x;
		dot2 += sv.oplanes[x].n.x * pAABB[1 - (*((uint32*)&sv.oplanes[x].n.x) >> 31)].x;
		dot1 += sv.oplanes[x].n.y * pAABB[0 + (*((uint32*)&sv.oplanes[x].n.y) >> 31)].y;
		dot2 += sv.oplanes[x].n.y * pAABB[1 - (*((uint32*)&sv.oplanes[x].n.y) >> 31)].y;
		dot1 += sv.oplanes[x].n.z * pAABB[0 + (*((uint32*)&sv.oplanes[x].n.z) >> 31)].z;
		dot2 += sv.oplanes[x].n.z * pAABB[1 - (*((uint32*)&sv.oplanes[x].n.z) >> 31)].z;
		if (!(((int32&)dot1) & 0x80000000)) return CULL_EXCLUSION;
		notOverlap &= (int32&)dot2;
	}
	if (notOverlap)	return CULL_INCLUSION;
	return CULL_OVERLAP;
}
