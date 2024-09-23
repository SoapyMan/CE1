
#ifndef __INV_CMAP_H__
#define __INV_CMAP_H__

extern void shInverseColormap(int colors, SRGBPixel* colormap,
	int rbits, int gbits, int bbits, byte*& rgbmap,
	ulong* dist_buf = nullptr);

#endif // __INV_CMAP_H__
