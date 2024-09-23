//////////////////////////////////////////////////////////////////////
//
//	Grid
//	
//	File: grid.h
//	Description : Grid class declaration and inlined implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef grid_h
#define grid_h
#pragma once

#include "vector3d.h"
#include "vector2d.h"

struct grid {
	vector_tpl<int> ax;
	vectorf org;
	vector2df step, stepr;
	vector2d_tpl<uint> sz;
	float parity;
	float scale;
	ushort* pflags;
	ushort holeflag;
	int holepower;

	int inrange(uint ix, uint iy) {
		return ix < sz.x && iy < sz.y;
	}
	int inrange(int ix, int iy) {
		return (uint)ix < sz.x && (uint)iy < sz.y;
	}
	int getcell(uint ix, uint iy) {
		return ix < sz.x && iy < sz.y ? iy * sz.x + ix : sz.x * sz.y;
	}
	int getcell(int ix, int iy) {
		return (uint)ix < sz.x && (uint)iy < sz.y ? iy * sz.x + ix : sz.x * sz.y;
	}
};

#endif