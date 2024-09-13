#include "stdafx.h"										// precompiled headers
#include "SimpleTriangleRasterizer.h"					// CSimpleTriangleRastizer
#include <math.h>										// sqrt()

#define FLT_MAX 3.402823466e+38F

void CSimpleTriangleRasterizer::lambertHorizlineConservative(float fx1, float fx2, int yy, IRasterizeSink* inpSink)
{
	const int x1 = clamp((int)floorf(fx1), m_iMinX, m_iMaxX+1);
	const int x2 = clamp((int)floorf(fx2 + 1.0f), m_iMinX, m_iMaxX+1);

	inpSink->Line(fx1, fx2, x1, x2, yy);
}

void CSimpleTriangleRasterizer::lambertHorizlineSubpixelCorrect(float fx1, float fx2, int yy, IRasterizeSink* inpSink)
{
	const int x1 = clamp((int)floorf(fx1 + 0.5f), m_iMinX, m_iMaxX+1);
	const int x2 = clamp((int)floorf(fx2 + 0.5f), m_iMinX, m_iMaxX+1);

	inpSink->Line(fx1, fx2, x1, x2, yy);
}

// optimizable
void CSimpleTriangleRasterizer::CopyAndSortY(const float infX[3], const float infY[3], float outfX[3], float outfY[3])
{
	outfX[0] = infX[0]; outfY[0] = infY[0];
	outfX[1] = infX[1]; outfY[1] = infY[1];
	outfX[2] = infX[2]; outfY[2] = infY[2];

	// Sort the coordinates, so that (x[1], y[1]) becomes the highest coord
	float tmp;

	if (outfY[0] > outfY[1])
	{
		if (outfY[1] > outfY[2])
		{
			tmp = outfY[0]; outfY[0] = outfY[1]; outfY[1] = tmp;
			tmp = outfX[0]; outfX[0] = outfX[1]; outfX[1] = tmp;
			tmp = outfY[1]; outfY[1] = outfY[2]; outfY[2] = tmp;
			tmp = outfX[1]; outfX[1] = outfX[2]; outfX[2] = tmp;

			if (outfY[0] > outfY[1])
			{
				tmp = outfY[0]; outfY[0] = outfY[1]; outfY[1] = tmp;
				tmp = outfX[0]; outfX[0] = outfX[1]; outfX[1] = tmp;
			}
		}
		else
		{
			tmp = outfY[0]; outfY[0] = outfY[1]; outfY[1] = tmp;
			tmp = outfX[0]; outfX[0] = outfX[1]; outfX[1] = tmp;

			if (outfY[1] > outfY[2])
			{
				tmp = outfY[1]; outfY[1] = outfY[2]; outfY[2] = tmp;
				tmp = outfX[1]; outfX[1] = outfX[2]; outfX[2] = tmp;
			}
		}
	}
	else if (outfY[1] > outfY[2])
	{
		tmp = outfY[1]; outfY[1] = outfY[2]; outfY[2] = tmp;
		tmp = outfX[1]; outfX[1] = outfX[2]; outfX[2] = tmp;

		if (outfY[0] > outfY[1])
		{
			tmp = outfY[0]; outfY[0] = outfY[1]; outfY[1] = tmp;
			tmp = outfX[0]; outfX[0] = outfX[1]; outfX[1] = tmp;
		}
	}
}

void CSimpleTriangleRasterizer::CallbackFillRectConservative(float _x[3], float _y[3], IRasterizeSink* inpSink)
{
	inpSink->Triangle(m_iMinY);

	const float fMinX = crymin(_x[0], crymin(_x[1], _x[2]));
	const float fMaxX = crymax(_x[0], crymax(_x[1], _x[2]));
	const float fMinY = crymin(_y[0], crymin(_y[1], _y[2]));
	const float fMaxY = crymax(_y[0], crymax(_y[1], _y[2]));

	const int iMinX = crymax(m_iMinX, (int)floorf(fMinX));
	const int iMaxX = crymin(m_iMaxX + 1, (int)ceilf(fMaxX));
	const int iMinY = crymax(m_iMinY, (int)floorf(fMinY));
	const int iMaxY = crymin(m_iMaxY + 1, (int)ceilf(fMaxY));

	for (int y = iMinY; y < iMaxY; y++)
		inpSink->Line(fMinX, fMaxX, iMinX, iMaxX, y);
}

void CSimpleTriangleRasterizer::CallbackFillConservative(float _x[3], float _y[3], IRasterizeSink* inpSink)
{
	float x[3], y[3];

	CopyAndSortY(_x, _y, x, y);

	// Calculate interpolation steps
	float fX1toX2step = 0.0f; if (y[1] - y[0] != 0.0f) fX1toX2step = (x[1] - x[0]) / (float)(y[1] - y[0]);
	float fX1toX3step = 0.0f; if (y[2] - y[0] != 0.0f) fX1toX3step = (x[2] - x[0]) / (float)(y[2] - y[0]);
	float fX2toX3step = 0.0f; if (y[2] - y[1] != 0.0f) fX2toX3step = (x[2] - x[1]) / (float)(y[2] - y[1]);

	float fX1toX2 = x[0], fX1toX3 = x[0], fX2toX3 = x[1];
	bool bFirstLine = true;
	bool bTriangleCallDone = false;

	// Go through the scanlines of the triangle
	int yy = (int)floorf(y[0]);			// was floor

	for (; yy <= (int)floorf(y[2]); yy++)
		//	for(yy=m_iMinY; yy<=m_iMaxY; yy++)		// juhu
	{
		float fSubPixelYStart = 0.0f, fSubPixelYEnd = 1.0f;
		float start, end;

		// first line
		if (bFirstLine)
		{
			fSubPixelYStart = y[0] - floorf(y[0]);
			start = x[0]; end = x[0]; bFirstLine = false;
		}
		else if (yy <= (int)floorf(y[1])) // top part without middle corner line
		{
			start = crymin(fX1toX2, fX1toX3); end = crymax(fX1toX2, fX1toX3);
		}
		else
		{
			start = crymin(fX2toX3, fX1toX3); end = crymax(fX2toX3, fX1toX3);
		}

		// middle corner line
		if (yy == (int)floorf(y[1]))
		{
			fSubPixelYEnd = y[1] - floorf(y[1]);

			fX1toX3 += fX1toX3step * (fSubPixelYEnd - fSubPixelYStart);
			start = crymin(start, fX1toX3); end = crymax(end, fX1toX3);
			start = crymin(start, x[1]); end = crymax(end, x[1]);

			fSubPixelYStart = fSubPixelYEnd; fSubPixelYEnd = 1.0f;
		}

		// last line
		if (yy == (int)floorf(y[2]))
		{
			start = crymin(start, x[2]); end = crymax(end, x[2]);
		}
		else
		{
			// top part without middle corner line
			if (yy < (int)floorf(y[1]))
			{
				fX1toX2 += fX1toX2step * (fSubPixelYEnd - fSubPixelYStart);
				start = crymin(start, fX1toX2); end = crymax(end, fX1toX2);
			}
			else
			{
				fX2toX3 += fX2toX3step * (fSubPixelYEnd - fSubPixelYStart);
				start = crymin(start, fX2toX3); end = crymax(end, fX2toX3);
			}

			fX1toX3 += fX1toX3step * (fSubPixelYEnd - fSubPixelYStart);
			start = crymin(start, fX1toX3); end = crymax(end, fX1toX3);
		}

		if (yy >= m_iMinY && yy <= m_iMaxY)
		{
			if (!bTriangleCallDone) { inpSink->Triangle(yy); bTriangleCallDone = true; }

			lambertHorizlineConservative(start, end, yy, inpSink);
		}
	}
}



void CSimpleTriangleRasterizer::CallbackFillSubpixelCorrect(float _x[3], float _y[3], IRasterizeSink* inpSink)
{
	float x[3], y[3];

	CopyAndSortY(_x, _y, x, y);

	if (y[0] - floorf(y[0]) == 0.0f)
		y[0] -= 0.0001f;

	// Calculate interpolation steps
	float fX1toX2step = 0.0f; if (y[1] - y[0] != 0.0f)fX1toX2step = (x[1] - x[0]) / (float)(y[1] - y[0]);
	float fX1toX3step = 0.0f; if (y[2] - y[0] != 0.0f)fX1toX3step = (x[2] - x[0]) / (float)(y[2] - y[0]);
	float fX2toX3step = 0.0f; if (y[2] - y[1] != 0.0f)fX2toX3step = (x[2] - x[1]) / (float)(y[2] - y[1]);

	float fX1toX2 = x[0], fX1toX3 = x[0], fX2toX3 = x[1];
	bool bFirstLine = true;
	bool bTriangleCallDone = false;

	y[0] -= 0.5f; y[1] -= 0.5f; y[2] -= 0.5f;

	int yy = (int)floorf(y[0]);

	for (; yy <= (int)floorf(y[2]); yy++)
	{
		float fSubPixelYStart = 0.0f, fSubPixelYEnd = 1.0f;
		float start, end;

		// first line
		if (bFirstLine)
		{
			fSubPixelYStart = y[0] - floorf(y[0]);
			start = x[0]; end = x[0]; bFirstLine = false;
		}
		else
		{
			// top part without middle corner line
			if (yy <= (int)floorf(y[1]))
			{
				start = crymin(fX1toX2, fX1toX3); end = crymax(fX1toX2, fX1toX3);
			}
			else
			{
				start = crymin(fX2toX3, fX1toX3); end = crymax(fX2toX3, fX1toX3);
			}
		}

		// middle corner line
		if (yy == (int)floorf(y[1]))
		{
			fSubPixelYEnd = y[1] - floorf(y[1]);

			fX1toX3 += fX1toX3step * (fSubPixelYEnd - fSubPixelYStart);

			fSubPixelYStart = fSubPixelYEnd; fSubPixelYEnd = 1.0f;
		}

		// last line
		if (yy != (int)floorf(y[2]))
		{
			// top part without middle corner line
			if (yy < (int)floorf(y[1]))
				fX1toX2 += fX1toX2step * (fSubPixelYEnd - fSubPixelYStart);
			else
				fX2toX3 += fX2toX3step * (fSubPixelYEnd - fSubPixelYStart);

			fX1toX3 += fX1toX3step * (fSubPixelYEnd - fSubPixelYStart);
		}

		if (start != end)
			if (yy >= m_iMinY && yy <= m_iMaxY)
			{
				if (!bTriangleCallDone) { inpSink->Triangle(yy); bTriangleCallDone = true; }

				lambertHorizlineSubpixelCorrect(start, end, yy, inpSink);
			}
	}
}




// shrink triangle by n pixel, optimizable
void CSimpleTriangleRasterizer::ShrinkTriangle(float inoutfX[3], float inoutfY[3], float infAmount)
{
	float fX[3] = { inoutfX[0], inoutfX[1], inoutfX[2] };
	float fY[3] = { inoutfY[0], inoutfY[1], inoutfY[2] };

	/*
	// move edge to opposing vertex
	float dx,dy,fLength;

	for(int a=0;a<3;a++)
	{
		int b=a+1;if(b>=3)b=0;
		int c=b+1;if(c>=3)c=0;

		dx=fX[a]-(fX[b]+fX[c])*0.5f;
		dy=fY[a]-(fY[b]+fY[c])*0.5f;
		fLength=(float)sqrt(dx*dx+dy*dy);
		if(fLength>1.0f)
		{
			dx/=fLength;dy/=fLength;
			inoutfX[b]+=dx;inoutfY[b]+=dy;
			inoutfX[c]+=dx;inoutfY[c]+=dy;
		}
	}
	*/

	/*
	// move vertex to opposing edge
	float dx,dy,fLength;

	for(int a=0;a<3;a++)
	{
		int b=a+1;if(b>=3)b=0;
		int c=b+1;if(c>=3)c=0;

		dx=fX[a]-(fX[b]+fX[c])*0.5f;
		dy=fY[a]-(fY[b]+fY[c])*0.5f;
		fLength=(float)sqrt(dx*dx+dy*dy);
		if(fLength>1.0f)
		{
			dx/=fLength;dy/=fLength;
			inoutfX[a]-=dx;inoutfY[a]-=dy;
		}
	}
	*/

	// move vertex to get edges shifted perpendicual for 1 unit	
	for (int a = 0; a < 3; a++)
	{
		float dx1, dy1, dx2, dy2, fLength;

		int b = a + 1; if (b >= 3)b = 0;
		int c = b + 1; if (c >= 3)c = 0;

		dx1 = fX[b] - fX[a];
		dy1 = fY[b] - fY[a];
		fLength = (float)sqrt(dx1 * dx1 + dy1 * dy1);
		if (infAmount > 0)if (fLength < infAmount)continue;
		if (fLength == 0.0f)continue;
		dx1 /= fLength; dy1 /= fLength;

		dx2 = fX[c] - fX[a];
		dy2 = fY[c] - fY[a];
		fLength = (float)sqrt(dx2 * dx2 + dy2 * dy2);
		if (infAmount > 0)if (fLength < infAmount)continue;
		if (fLength == 0.0f)continue;
		dx2 /= fLength; dy2 /= fLength;

		inoutfX[a] += (dx1 + dx2) * infAmount; inoutfY[a] += (dy1 + dy2) * infAmount;
	}
}
