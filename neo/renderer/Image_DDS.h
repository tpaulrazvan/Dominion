/*
 * DDS layout and flags from dhewm3 renderer/Image.h (GPL-3.0-or-later).
 * Loading is adapted to Dominion's image upload boundary in BinaryImage.cpp.
 */
#ifndef __IMAGE_DDS_H__
#define __IMAGE_DDS_H__

#if defined( DOMINION ) // Original assets - native DDS headers
const unsigned int DDSF_PITCH = 0x00000008;
const unsigned int DDSF_MIPMAPCOUNT = 0x00020000;
const unsigned int DDSF_ALPHAPIXELS = 0x00000001;
const unsigned int DDSF_FOURCC = 0x00000004;
const unsigned int DDSF_RGB = 0x00000040;
const unsigned int DDSF_ID_INDEXCOLOR = 0x10000000;

#define DDS_MAKEFOURCC( a, b, c, d ) ( ( a ) | ( ( b ) << 8 ) | ( ( c ) << 16 ) | ( ( d ) << 24 ) )

struct ddsFilePixelFormat_t
{
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwFourCC;
	unsigned int dwRGBBitCount;
	unsigned int dwRBitMask;
	unsigned int dwGBitMask;
	unsigned int dwBBitMask;
	unsigned int dwABitMask;
};

struct ddsFileHeader_t
{
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwHeight;
	unsigned int dwWidth;
	unsigned int dwPitchOrLinearSize;
	unsigned int dwDepth;
	unsigned int dwMipMapCount;
	unsigned int dwReserved1[11];
	ddsFilePixelFormat_t ddspf;
	unsigned int dwCaps1;
	unsigned int dwCaps2;
	unsigned int dwReserved2[3];
};
#endif
#endif
