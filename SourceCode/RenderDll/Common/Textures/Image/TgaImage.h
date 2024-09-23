
#ifndef TGAIMAGE_H
#define TGAIMAGE_H

/**
 * An ImageFile subclass for reading TGA files.
 */
class CImageTgaFile : public CImageFile
{
	///
	friend class CImageFile;	// For constructor

private:
	/// Read the TGA file from the buffer.
	CImageTgaFile(byte* buf, long size);

public:
	///
	virtual ~CImageTgaFile();
};

/* Header definition. */
struct SImageHeader {
	uchar IDLength;		/* length of Identifier String */
	uchar CoMapType;		/* 0 = no map */
	uchar ImgType;		/* image type (see below for values) */
	uchar Index_lo, Index_hi;	/* index of first color map entry */
	uchar Length_lo, Length_hi;	/* number of entries in color map */
	uchar CoSize;		/* size of color map entry (15,16,24,32) */
	uchar X_org_lo, X_org_hi;	/* x origin of image */
	uchar Y_org_lo, Y_org_hi;	/* y origin of image */
	uchar Width_lo, Width_hi;	/* width of image */
	uchar Height_lo, Height_hi;	/* height of image */
	uchar PixelSize;		/* pixel size (8,16,24,32) */
	uchar AttBits;		/* 4 bits, number of attribute bits per pixel */
	uchar Rsrvd;		/* 1 bit, reserved */
	uchar OrgBit;		/* 1 bit, origin: 0=lower left, 1=upper left */
	uchar IntrLve;		/* 2 bits, interleaving flag */
};

typedef char ImageIDField[256];

/* Definitions for image types. */
#define TGA_Null 0
#define TGA_Map 1
#define TGA_RGB 2
#define TGA_Mono 3
#define TGA_RLEMap 9
#define TGA_RLERGB 10
#define TGA_RLEMono 11
#define TGA_CompMap 32
#define TGA_CompMap4 33

/* Definitions for interleave flag. */
#define TGA_IL_None 0
#define TGA_IL_Two 1
#define TGA_IL_Four 2

#endif

