/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2025 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop



/*
================================================================================================

	idBinaryImage

================================================================================================
*/

#include "RenderCommon.h"
#include "DXT/DXTCodec.h"
#include "Color/ColorSpace.h"

#include "../libs/mesa/format_r11g11b10f.h"

idCVar image_highQualityCompression( "image_highQualityCompression", "0", CVAR_BOOL, "Use high quality (slow) compression" );

#if defined( DOMINION ) // BLUE TEXTURES FIX
/*
========================
PadCompressedImage

Replicate the nearest valid texel into complete compression blocks, keeping padding out of the image extent.
========================
*/
static void PadCompressedImage( const byte* source, const int width, const int height, byte* destination, const int paddedWidth, const int paddedHeight )
{
	for( int y = 0; y < paddedHeight; ++y )
	{
		const byte* sourceRow = source + Min( y, height - 1 ) * width * 4;
		byte* destinationRow = destination + y * paddedWidth * 4;
		memcpy( destinationRow, sourceRow, width * 4 );
		for( int x = width; x < paddedWidth; ++x )
		{
			memcpy( destinationRow + x * 4, sourceRow + ( width - 1 ) * 4, 4 );
		}
	}
}

/*
========================
idBinaryImage::ConvertNonBlockAlignedDXT

Decode every mip when the backend cannot create this BC base size. Crop block padding and retain
YCoCg/normal-map channel encoding.

NOTE: Did not add BC6H / BC7 support here because BFG .bimages were not encoded with those.
========================
*/
bool idBinaryImage::ConvertNonBlockAlignedDXT()
{
	if( ( fileData.format != FMT_DXT1 && fileData.format != FMT_DXT5 ) ||
			( ( fileData.width & 3 ) == 0 && ( fileData.height & 3 ) == 0 ) )
	{
		return false;
	}

	const int blockBytes = fileData.format == FMT_DXT1 ? 8 : 16;
	for( int i = 0; i < images.Num(); ++i )
	{
		const idBinaryImageData& img = images[i];
		if( img.level < 0 || img.level >= fileData.numLevels || img.level >= 31 || fileData.width < 1 || fileData.height < 1 )
		{
			idLib::Error( "Invalid BC mip header in %s", GetName() );
			return false;
		}
		const int width = Max( 1, fileData.width >> img.level );
		const int height = Max( 1, fileData.height >> img.level );
		const int64 storageBytes = ( ( int64( width ) + 3 ) / 4 ) * ( ( int64( height ) + 3 ) / 4 ) * blockBytes;
		const int64 decodedBytes = ( ( int64( width ) + 3 ) & ~int64( 3 ) ) * ( ( int64( height ) + 3 ) & ~int64( 3 ) ) * 4;
		if( storageBytes != img.dataSize || decodedBytes > INT_MAX || img.data == NULL ||
				( img.width != width && !( fileData.textureType == DTT_CUBIC && width < 4 && img.width == 4 ) ) ||
				( img.height != height && !( fileData.textureType == DTT_CUBIC && height < 4 && img.height == 4 ) ) )
		{
			idLib::Error( "Invalid BC mip payload in %s (level %d)", GetName(), img.level );
			return false;
		}
	}

	idDxtDecoder decoder;
	for( int i = 0; i < images.Num(); ++i )
	{
		idBinaryImageData& img = images[i];
		const int width = Max( 1, fileData.width >> img.level );
		const int height = Max( 1, fileData.height >> img.level );
		const int paddedWidth = ( width + 3 ) & ~3;
		const int paddedHeight = ( height + 3 ) & ~3;
		idTempArray<byte> decoded( paddedWidth * paddedHeight * 4 );
		if( fileData.format == FMT_DXT1 )
		{
			decoder.DecompressImageDXT1( img.data, decoded.Ptr(), paddedWidth, paddedHeight );
		}
		else
		{
			decoder.DecompressImageDXT5( img.data, decoded.Ptr(), paddedWidth, paddedHeight );
		}
		img.Alloc( width * height * 4 );
		img.width = width;
		img.height = height;
		for( int y = 0; y < height; ++y )
		{
			memcpy( img.data + y * width * 4, decoded.Ptr() + y * paddedWidth * 4, width * 4 );
		}
	}
	fileData.format = FMT_RGBA8;
	return true;
}
#endif

/*
========================
idBinaryImage::Load2DFromMemory
========================
*/
void idBinaryImage::Load2DFromMemory( int width, int height, const byte* pic_const, int numLevels, textureFormat_t& textureFormat, textureColor_t& colorFormat, bool gammaMips )
{
	fileData.textureType = DTT_2D;
	fileData.format = textureFormat;
	fileData.colorFormat = colorFormat;
	fileData.width = width;
	fileData.height = height;
	fileData.numLevels = numLevels;

	common->LoadPacifierBinarizeInfo( va( "(%d x %d)", width, height ) );

	byte* pic = ( byte* )Mem_Alloc( width * height * 4, TAG_TEMP );
	memcpy( pic, pic_const, width * height * 4 );

	if( colorFormat == CFM_YCOCG_DXT5 )
	{
		// convert the image data to YCoCg and use the YCoCgDXT5 compressor
		idColorSpace::ConvertRGBToCoCg_Y( pic, pic, width, height );
	}
	else if( colorFormat == CFM_NORMAL_DXT5 )
	{
		// Blah, HQ swizzles automatically, Fast doesn't
		if( !image_highQualityCompression.GetBool() )
		{
			for( int i = 0; i < width * height; i++ )
			{
				pic[i * 4 + 3] = pic[i * 4 + 0];
				pic[i * 4 + 0] = 0;
				pic[i * 4 + 2] = 0;
			}
		}
	}
	else if( colorFormat == CFM_GREEN_ALPHA )
	{
		for( int i = 0; i < width * height; i++ )
		{
			pic[i * 4 + 1] = pic[i * 4 + 3];
			pic[i * 4 + 0] = 0;
			pic[i * 4 + 2] = 0;
			pic[i * 4 + 3] = 0;
		}
	}

	int	scaledWidth = width;
	int scaledHeight = height;
	images.SetNum( numLevels );
	for( int level = 0; level < images.Num(); level++ )
	{
		idBinaryImageData& img = images[ level ];

		common->LoadPacifierBinarizeMiplevel( level + 1, numLevels );

		// Images that are going to be DXT compressed and aren't multiples of 4 need to be
		// padded out before compressing.
		byte* dxtPic = pic;
		int	dxtWidth = 0;
		int	dxtHeight = 0;
		if( textureFormat == FMT_DXT5 || textureFormat == FMT_DXT1 || textureFormat == FMT_BC6H || textureFormat == FMT_BC7 )
		{
			if( ( scaledWidth & 3 ) || ( scaledHeight & 3 ) )
			{
				dxtWidth = ( scaledWidth + 3 ) & ~3;
				dxtHeight = ( scaledHeight + 3 ) & ~3;
#if defined( DOMINION ) // BLUE TEXTURES FIX
				dxtPic = ( byte* )Mem_Alloc( dxtWidth * dxtHeight * 4, TAG_IMAGE );
				PadCompressedImage( pic, scaledWidth, scaledHeight, dxtPic, dxtWidth, dxtHeight );
#else
				dxtPic = ( byte* )Mem_ClearedAlloc( dxtWidth * 4 * dxtHeight, TAG_IMAGE );
				for( int i = 0; i < scaledHeight; i++ )
				{
					memcpy( dxtPic + i * dxtWidth * 4, pic + i * scaledWidth * 4, scaledWidth * 4 );
				}
#endif
			}
			else
			{
				dxtPic = pic;
				dxtWidth = scaledWidth;
				dxtHeight = scaledHeight;
			}
		}

		img.level = level;
		img.destZ = 0;
		img.width = scaledWidth;
		img.height = scaledHeight;

		// compress data or convert floats as necessary
		if( textureFormat == FMT_DXT1 )
		{
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight / 2 );
			if( image_highQualityCompression.GetBool() )
			{
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1HQ", width, height ) );

				dxt.CompressImageDXT1HQ( dxtPic, img.data, dxtWidth, dxtHeight );
			}
			else
			{
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1Fast", width, height ) );

				dxt.CompressImageDXT1Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
		}
		else if( textureFormat == FMT_DXT5 )
		{
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight );
			if( colorFormat == CFM_NORMAL_DXT5 )
			{
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - NormalMapDXT5HQ", width, height ) );

					dxt.CompressNormalMapDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - NormalMapDXT5Fast", width, height ) );

					dxt.CompressNormalMapDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
			else if( colorFormat == CFM_YCOCG_DXT5 )
			{
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - YCoCgDXT5HQ", width, height ) );

					dxt.CompressYCoCgDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - YCoCgDXT5Fast", width, height ) );

					dxt.CompressYCoCgDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
			else
			{
				fileData.colorFormat = colorFormat = CFM_DEFAULT;
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5HQ", width, height ) );

					dxt.CompressImageDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5Fast", width, height ) );

					dxt.CompressImageDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
		}
		else if( textureFormat == FMT_BC6H )
		{
#if !defined(USE_INTRINSICS_SSE) && !defined(USE_INTRINSICS_NEON)

			// RB: store as R11G11B10 because the generic fallback is too slow
			fileData.format = textureFormat = FMT_R11G11B10F;
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
#else
			img.Alloc( dxtWidth * dxtHeight );
			idDxtEncoder dxt;

			if( image_highQualityCompression.GetBool() )
			{
#if defined( DOMINION ) // BC6H Dimensions Fix
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6HQ", width, height ) );
				dxt.CompressImageR11G11B10_BC6HQ( dxtPic, img.data, dxtWidth, dxtHeight );
#else
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6HQ", width, width ) );
				dxt.CompressImageR11G11B10_BC6HQ( dxtPic, img.data, dxtWidth, dxtWidth );
#endif
			}
			else
			{
#if defined( DOMINION ) // BC6H Dimensions Fix
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6Fast", width, height ) );
#else
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6Fast", width, width ) );
#endif
				dxt.CompressImageR11G11B10_BC6Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
#endif
		}
		else if( textureFormat == FMT_LUM8 || textureFormat == FMT_INT8 )
		{
			// LUM8 and INT8 just read the red channel
			img.Alloc( scaledWidth * scaledHeight );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i * 4 ];
			}
		}
		else if( textureFormat == FMT_ALPHA )
		{
			// ALPHA reads the alpha channel
			img.Alloc( scaledWidth * scaledHeight );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i * 4 + 3 ];
			}
		}
		else if( textureFormat == FMT_L8A8 )
		{
			// L8A8 reads the alpha and red channels
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for( int i = 0; i < img.dataSize / 2; i++ )
			{
				img.data[ i * 2 + 0 ] = pic[ i * 4 + 0 ];
				img.data[ i * 2 + 1 ] = pic[ i * 4 + 3 ];
			}
		}
		else if( textureFormat == FMT_RGB565 )
		{
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for( int i = 0; i < img.dataSize / 2; i++ )
			{
				unsigned short color = ( ( pic[ i * 4 + 0 ] >> 3 ) << 11 ) | ( ( pic[ i * 4 + 1 ] >> 2 ) << 5 ) | ( pic[ i * 4 + 2 ] >> 3 );
				img.data[ i * 2 + 0 ] = ( color >> 8 ) & 0xFF;
				img.data[ i * 2 + 1 ] = color & 0xFF;
			}
		}
		else if( textureFormat == FMT_RG16F )
		{
			// RB: copy it as it was a RGBA8 because of the same size
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
		}
		else if( textureFormat == FMT_R11G11B10F )
		{
			// RB: copy it as it was a RGBA8 because of the same size
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
		}
		else
		{
			fileData.format = textureFormat = FMT_RGBA8;
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
		}

		// if we had to pad to quads, free the padded version
		if( pic != dxtPic )
		{
			Mem_Free( dxtPic );
			dxtPic = NULL;
		}

		// downsample for the next level
		byte* shrunk = NULL;
		if( textureFormat == FMT_R11G11B10F || textureFormat == FMT_BC6H )
		{
			shrunk = R_MipMapR11G11B10F( pic, scaledWidth, scaledHeight );
		}
		else if( gammaMips )
		{
			shrunk = R_MipMapWithGamma( pic, scaledWidth, scaledHeight );
		}
		else
		{
			shrunk = R_MipMap( pic, scaledWidth, scaledHeight );
		}
		Mem_Free( pic );
		pic = shrunk;

		scaledWidth = Max( 1, scaledWidth >> 1 );
		scaledHeight = Max( 1, scaledHeight >> 1 );
	}

	Mem_Free( pic );
}


/*
========================
RB idBinaryImage::Load2DAtlasMipchainFromMemory
========================
*/
void idBinaryImage::Load2DAtlasMipchainFromMemory( int width, int height, const byte* pic_const, int numLevels, textureFormat_t& textureFormat, textureColor_t& colorFormat )
{
	int sourceWidth = width * ( 2.0f / 3.0f ); // RB

	fileData.textureType = DTT_2D;
	fileData.format = textureFormat;
	fileData.colorFormat = CFM_DEFAULT;
	fileData.width = sourceWidth;
	fileData.height = height;
	fileData.numLevels = numLevels;

	common->LoadPacifierBinarizeInfo( va( "(%d x %d)", width, height ) );

	byte* sourcePic = ( byte* )Mem_Alloc( width * height * 4, TAG_TEMP );
	memcpy( sourcePic, pic_const, width * height * 4 );

	if( colorFormat == CFM_YCOCG_DXT5 )
	{
		// convert the image data to YCoCg and use the YCoCgDXT5 compressor
		idColorSpace::ConvertRGBToCoCg_Y( sourcePic, sourcePic, width, height );
	}
	else if( colorFormat == CFM_NORMAL_DXT5 )
	{
		// Blah, HQ swizzles automatically, Fast doesn't
		if( !image_highQualityCompression.GetBool() )
		{
			for( int i = 0; i < width * height; i++ )
			{
				sourcePic[i * 4 + 3] = sourcePic[i * 4 + 0];
				sourcePic[i * 4 + 0] = 0;
				sourcePic[i * 4 + 2] = 0;
			}
		}
	}
	else if( colorFormat == CFM_GREEN_ALPHA )
	{
		for( int i = 0; i < width * height; i++ )
		{
			sourcePic[i * 4 + 1] = sourcePic[i * 4 + 3];
			sourcePic[i * 4 + 0] = 0;
			sourcePic[i * 4 + 2] = 0;
			sourcePic[i * 4 + 3] = 0;
		}
	}

	images.SetNum( numLevels );

	const int numColors = 5;
	static idVec4 colors[numColors] = { colorBlue, colorCyan, colorGreen, colorYellow, colorRed };

	for( int level = 0; level < images.Num(); level++ )
	{
		idBinaryImageData& img = images[ level ];

		// RB: create shrunk image which is a copy of the sub image in the atlas
		idVec4 rect = R_CalculateMipRect( sourceWidth, level );

		int	scaledWidth = rect.z;
		int scaledHeight = rect.w;

		byte* pic = ( byte* )Mem_Alloc( scaledWidth * scaledHeight * 4, TAG_TEMP );

		for( int x = rect.x; x < ( rect.x + rect.z ); x++ )
			//for( int x = 0; x < rect.z; x++ )
		{
			for( int y = rect.y; y < ( rect.y + rect.w ); y++ )
				//for( int y = 0; y < rect.w; y++ )
			{
				int sx = x - rect.x;
				int sy = y - rect.y;

#if 1
				pic[( sy * scaledWidth + sx ) * 4 + 0] = sourcePic[( y * width + x ) * 4 + 0];
				pic[( sy * scaledWidth + sx ) * 4 + 1] = sourcePic[( y * width + x ) * 4 + 1];
				pic[( sy * scaledWidth + sx ) * 4 + 2] = sourcePic[( y * width + x ) * 4 + 2];
				pic[( sy * scaledWidth + sx ) * 4 + 3] = sourcePic[( y * width + x ) * 4 + 3];
#else
				int colorIdx = level % numColors;
				float color[3];
				color[0] = colors[ colorIdx ].x;
				color[0] = colors[ colorIdx ].y;
				color[0] = colors[ colorIdx ].z;

				uint32_t value = float3_to_r11g11b10f( color );

				union
				{
					uint32	i;
					byte	b[4];
				} tmp;

				tmp.i = value;

				//*( uint32_t* )pic[( sy * scaledWidth + sx ) * 3] = value;

				pic[( sy * scaledWidth + sx ) * 4 + 0] = tmp.b[0];
				pic[( sy * scaledWidth + sx ) * 4 + 1] = tmp.b[1];
				pic[( sy * scaledWidth + sx ) * 4 + 2] = tmp.b[2];
				pic[( sy * scaledWidth + sx ) * 4 + 3] = tmp.b[3];
#endif
			}
		}

		common->LoadPacifierBinarizeProgressIncrement( rect.x * rect.y );
		// RB end

		common->LoadPacifierBinarizeMiplevel( level + 1, numLevels );

		// Images that are going to be DXT compressed and aren't multiples of 4 need to be
		// padded out before compressing.
		byte* dxtPic = pic;
		int	dxtWidth = 0;
		int	dxtHeight = 0;
		if( textureFormat == FMT_DXT5 || textureFormat == FMT_DXT1 || textureFormat == FMT_BC6H || textureFormat == FMT_BC7 )
		{
			if( ( scaledWidth & 3 ) || ( scaledHeight & 3 ) )
			{
				dxtWidth = ( scaledWidth + 3 ) & ~3;
				dxtHeight = ( scaledHeight + 3 ) & ~3;
#if defined( DOMINION ) // BLUE TEXTURES FIX
				dxtPic = ( byte* )Mem_Alloc( dxtWidth * dxtHeight * 4, TAG_IMAGE );
				PadCompressedImage( pic, scaledWidth, scaledHeight, dxtPic, dxtWidth, dxtHeight );
#else
				dxtPic = ( byte* )Mem_ClearedAlloc( dxtWidth * 4 * dxtHeight, TAG_IMAGE );
				for( int i = 0; i < scaledHeight; i++ )
				{
					memcpy( dxtPic + i * dxtWidth * 4, pic + i * scaledWidth * 4, scaledWidth * 4 );
				}
#endif
			}
			else
			{
				dxtPic = pic;
				dxtWidth = scaledWidth;
				dxtHeight = scaledHeight;
			}
		}

		img.level = level;
		img.destZ = 0;
		img.width = scaledWidth;
		img.height = scaledHeight;

		// compress data or convert floats as necessary
		if( textureFormat == FMT_DXT1 )
		{
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight / 2 );
			if( image_highQualityCompression.GetBool() )
			{
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1HQ", width, height ) );

				dxt.CompressImageDXT1HQ( dxtPic, img.data, dxtWidth, dxtHeight );
			}
			else
			{
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1Fast", width, height ) );

				dxt.CompressImageDXT1Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
		}
		else if( textureFormat == FMT_DXT5 )
		{
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight );
			if( colorFormat == CFM_NORMAL_DXT5 )
			{
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - NormalMapDXT5HQ", width, height ) );

					dxt.CompressNormalMapDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - NormalMapDXT5Fast", width, height ) );

					dxt.CompressNormalMapDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
			else if( colorFormat == CFM_YCOCG_DXT5 )
			{
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - YCoCgDXT5HQ", width, height ) );

					dxt.CompressYCoCgDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - YCoCgDXT5Fast", width, height ) );

					dxt.CompressYCoCgDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
			else
			{
				fileData.colorFormat = colorFormat = CFM_DEFAULT;
				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5HQ", width, height ) );

					dxt.CompressImageDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5Fast", width, height ) );

					dxt.CompressImageDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
		}
		else if( textureFormat == FMT_BC6H )
		{
#if !defined(USE_INTRINSICS_SSE) && !defined(USE_INTRINSICS_NEON)
			// RB: store as R11G11B10 because the generic fallback is too slow
			fileData.format = textureFormat = FMT_R11G11B10F;
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
#else
			img.Alloc( dxtWidth * dxtHeight );
			idDxtEncoder dxt;

			if( image_highQualityCompression.GetBool() )
			{
#if defined( DOMINION ) // BC6H Dimensions Fix
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6HQ", width, height ) );
#else
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6HQ", width, width ) );
#endif
				dxt.CompressImageR11G11B10_BC6HQ( dxtPic, img.data, dxtWidth, dxtHeight );
			}
			else
			{
#if defined( DOMINION ) // BC6H DImensions Fix
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6Fast", width, height ) );
#else
				common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6Fast", width, width ) );
#endif
				dxt.CompressImageR11G11B10_BC6Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
#endif
		}
		else if( textureFormat == FMT_LUM8 || textureFormat == FMT_INT8 )
		{
			// LUM8 and INT8 just read the red channel
			img.Alloc( scaledWidth * scaledHeight );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i * 4 ];
			}
		}
		else if( textureFormat == FMT_ALPHA )
		{
			// ALPHA reads the alpha channel
			img.Alloc( scaledWidth * scaledHeight );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i * 4 + 3 ];
			}
		}
		else if( textureFormat == FMT_L8A8 )
		{
			// L8A8 reads the alpha and red channels
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for( int i = 0; i < img.dataSize / 2; i++ )
			{
				img.data[ i * 2 + 0 ] = pic[ i * 4 + 0 ];
				img.data[ i * 2 + 1 ] = pic[ i * 4 + 3 ];
			}
		}
		else if( textureFormat == FMT_RGB565 )
		{
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for( int i = 0; i < img.dataSize / 2; i++ )
			{
				unsigned short color = ( ( pic[ i * 4 + 0 ] >> 3 ) << 11 ) | ( ( pic[ i * 4 + 1 ] >> 2 ) << 5 ) | ( pic[ i * 4 + 2 ] >> 3 );
				img.data[ i * 2 + 0 ] = ( color >> 8 ) & 0xFF;
				img.data[ i * 2 + 1 ] = color & 0xFF;
			}
		}
		else if( textureFormat == FMT_R11G11B10F )
		{
			// RB: copy it as it was a RGBA8 because of the same size
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
		}
		else
		{
			fileData.format = textureFormat = FMT_RGBA8;
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for( int i = 0; i < img.dataSize; i++ )
			{
				img.data[ i ] = pic[ i ];
			}
		}

		// if we had to pad to quads, free the padded version
		if( pic != dxtPic )
		{
			Mem_Free( dxtPic );
			dxtPic = NULL;
		}

		Mem_Free( pic );
	}

	Mem_Free( sourcePic );
}

/*
========================
PadImageTo4x4

DXT Compression requres a complete 4x4 block, even if the GPU will only be sampling
a subset of it, so pad to 4x4 with replicated texels to maximize compression.
========================
*/
static void PadImageTo4x4( const byte* src, int width, int height, byte dest[64] )
{
	// we probably will need to support this for non-square images, but I'll address
	// that when needed
	assert( width <= 4 && height <= 4 );
	assert( width > 0 && height > 0 );

	for( int y = 0 ; y < 4 ; y++ )
	{
		int	sy = y % height;
		for( int x = 0 ; x < 4 ; x++ )
		{
			int	sx = x % width;
			for( int c = 0 ; c < 4 ; c++ )
			{
				dest[( y * 4 + x ) * 4 + c] = src[( sy * width + sx ) * 4 + c];
			}
		}
	}
}

/*
========================
idBinaryImage::LoadCubeFromMemory
========================
*/
void idBinaryImage::LoadCubeFromMemory( int width, const byte* pics[6], int numLevels, textureFormat_t& textureFormat, bool gammaMips )
{
	common->LoadPacifierBinarizeInfo( va( "cube (%d)", width ) );

	fileData.textureType = DTT_CUBIC;
	fileData.format = textureFormat;
	fileData.colorFormat = CFM_DEFAULT;
	fileData.height = fileData.width = width;
	fileData.numLevels = numLevels;

	images.SetNum( fileData.numLevels * 6 );

	for( int side = 0; side < 6; side++ )
	{
		const byte* orig = pics[side];
		const byte* pic = orig;
		int	scaledWidth = fileData.width;

		for( int level = 0; level < fileData.numLevels; level++ )
		{
			// compress data or convert floats as necessary
			idBinaryImageData& img = images[ level * 6 + side ];

			common->LoadPacifierBinarizeMiplevel( level, fileData.numLevels );

#if defined( DOMINION ) // BLUE TEXTURES FIX
			// pad any partial cube-face block, including odd-sized faces, with nearest-edge texels.
			const bool compressed = textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 || textureFormat == FMT_BC6H || textureFormat == FMT_BC7;
			const int padSize = compressed ? ( scaledWidth + 3 ) & ~3 : scaledWidth;
			byte* paddedCube = NULL;
			const byte* padSrc = pic;
			if( padSize != scaledWidth )
			{
				paddedCube = ( byte* )Mem_Alloc( padSize * padSize * 4, TAG_IMAGE );
				PadCompressedImage( pic, scaledWidth, scaledWidth, paddedCube, padSize, padSize );
				padSrc = paddedCube;
			}
#else
			// handle padding blocks less than 4x4 for the DXT compressors
			ALIGN16( byte padBlock[64] );
			int		padSize;
			const byte* padSrc;

			if( scaledWidth < 4 && ( textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 || textureFormat == FMT_BC6H || textureFormat == FMT_BC7 ) )
			{
				PadImageTo4x4( pic, scaledWidth, scaledWidth, padBlock );
				padSize = 4;
				padSrc = padBlock;
			}
			else
			{
				padSize = scaledWidth;
				padSrc = pic;
			}
#endif

			img.level = level;
			img.destZ = side;
#if defined( DOMINIOn ) // BLUE TEXTURES FIX
			img.width = scaledWidth;
			img.height = scaledWidth;
#else
			img.width = padSize;
			img.height = padSize;
#endif

			if( textureFormat == FMT_DXT1 )
			{
				img.Alloc( padSize * padSize / 2 );
				idDxtEncoder dxt;

				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1HQ", width, width ) );

					dxt.CompressImageDXT1HQ( padSrc, img.data, padSize, padSize );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT1Fast", width, width ) );

					dxt.CompressImageDXT1Fast( padSrc, img.data, padSize, padSize );
				}
			}
			else if( textureFormat == FMT_DXT5 )
			{
				img.Alloc( padSize * padSize );
				idDxtEncoder dxt;

				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5HQ", width, width ) );

					dxt.CompressImageDXT5HQ( padSrc, img.data, padSize, padSize );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - DXT5Fast", width, width ) );

					dxt.CompressImageDXT5Fast( padSrc, img.data, padSize, padSize );
				}
			}
			else if( textureFormat == FMT_BC6H )
			{
#if !defined(USE_INTRINSICS_SSE) && !defined(USE_INTRINSICS_NEON)

				// RB: store as R11G11B10 because the generic fallback is too slow
				fileData.format = textureFormat = FMT_R11G11B10F;
				img.Alloc( scaledWidth * scaledWidth * 4 ); // DOMINION - blue Textures fix
				//	img.Alloc( padSize * padSize * 4 );	// commented out because of the above
				for( int i = 0; i < img.dataSize; i++ )
				{
					img.data[ i ] = pic[ i ];
				}
#else
				img.Alloc( padSize * padSize );
				idDxtEncoder dxt;

				if( image_highQualityCompression.GetBool() )
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6HQ", width, width ) );

					dxt.CompressImageR11G11B10_BC6HQ( padSrc, img.data, padSize, padSize );
				}
				else
				{
					common->LoadPacifierBinarizeInfo( va( "(%d x %d) - BC6Fast", width, width ) );

					dxt.CompressImageR11G11B10_BC6Fast( padSrc, img.data, padSize, padSize );
				}
#endif
			}
			else if( textureFormat == FMT_R11G11B10F )
			{
				// RB: copy it as it was a RGBA8 because of the same size
				img.Alloc( padSize * padSize * 4 );
				for( int i = 0; i < img.dataSize; i++ )
				{
					img.data[ i ] = pic[ i ];
				}
			}
			else
			{
				fileData.format = textureFormat = FMT_RGBA8;
				img.Alloc( scaledWidth * scaledWidth * 4 );	// DOMINION - blue Textures fix
				// img.Alloc( padSize * padSize * 4 ); // commented out because of the above
				memcpy( img.data, pic, img.dataSize );
			}

#if defined( DOMINION ) // blue textures fix
			// the temporary padded face is no longer needed after compression
			if( paddedCube != NULL )
			{
				Mem_Free( paddedCube );
			}
#endif

			// downsample for the next level
			byte* shrunk = NULL;
			if( textureFormat == FMT_R11G11B10F || textureFormat == FMT_BC6H )
			{
				shrunk = R_MipMapR11G11B10F( pic, scaledWidth, scaledWidth );
			}
			else if( gammaMips )
			{
				shrunk = R_MipMapWithGamma( pic, scaledWidth, scaledWidth );
			}
			else
			{
				shrunk = R_MipMap( pic, scaledWidth, scaledWidth );
			}
			if( pic != orig )
			{
				Mem_Free( ( void* )pic );
				pic = NULL;
			}
			pic = shrunk;

			scaledWidth = Max( 1, scaledWidth >> 1 );
		}
		if( pic != orig )
		{
			// free the down sampled version
			Mem_Free( ( void* )pic );
			pic = NULL;
		}
	}
}

/*
========================
idBinaryImage::WriteGeneratedFile
========================
*/
ID_TIME_T idBinaryImage::WriteGeneratedFile( ID_TIME_T sourceFileTime )
{
	idStr binaryFileName;
	MakeGeneratedFileName( binaryFileName );
	idFileLocal file( fileSystem->OpenFileWrite( binaryFileName, "fs_basepath" ) );
	if( file == NULL )
	{
		idLib::Warning( "idBinaryImage: Could not open file '%s'", binaryFileName.c_str() );
		return FILE_NOT_FOUND_TIMESTAMP;
	}
	//idLib::Printf( "Writing %s: %ix%i\n", binaryFileName.c_str(), fileData.width, fileData.height );

	fileData.headerMagic = BIMAGE_MAGIC;
	fileData.sourceFileTime = sourceFileTime;

	file->WriteBig( fileData.sourceFileTime );
	file->WriteBig( fileData.headerMagic );
	file->WriteBig( fileData.textureType );
	file->WriteBig( fileData.format );
	file->WriteBig( fileData.colorFormat );
	file->WriteBig( fileData.width );
	file->WriteBig( fileData.height );
	file->WriteBig( fileData.numLevels );

	for( int i = 0; i < images.Num(); i++ )
	{
		idBinaryImageData& img = images[ i ];
		file->WriteBig( img.level );
		file->WriteBig( img.destZ );
		file->WriteBig( img.width );
		file->WriteBig( img.height );
		file->WriteBig( img.dataSize );
		file->Write( img.data, img.dataSize );
	}
	return file->Timestamp();
}

/*
==========================
idBinaryImage::LoadFromGeneratedFile

Load the preprocessed image from the generated folder.
==========================
*/
ID_TIME_T idBinaryImage::LoadFromGeneratedFile( ID_TIME_T sourceFileTime )
{
	idStr binaryFileName;
	MakeGeneratedFileName( binaryFileName );
	idFileLocal bFile = fileSystem->OpenFileRead( binaryFileName );
	if( bFile == NULL )
	{
		return FILE_NOT_FOUND_TIMESTAMP;
	}
	if( LoadFromGeneratedFile( bFile, sourceFileTime ) )
	{
		return bFile->Timestamp();
	}
	return FILE_NOT_FOUND_TIMESTAMP;
}

/*
==========================
idBinaryImage::LoadFromGeneratedFile

Load the preprocessed image from the generated folder.
==========================
*/
bool idBinaryImage::LoadFromGeneratedFile( idFile* bFile, ID_TIME_T sourceTimeStamp )
{
#if defined( DOMINION ) // Blue Textures Fix
	if( bFile->Read( &fileData, sizeof( fileData ) ) != sizeof( fileData ) )
#else
	if( bFile->Read( &fileData, sizeof( fileData ) ) <= 0 )
#endif
	{
		return false;
	}
	idSwapClass<bimageFile_t> swap;
	swap.Big( fileData.sourceFileTime );
	swap.Big( fileData.headerMagic );
	swap.Big( fileData.textureType );
	swap.Big( fileData.format );
	swap.Big( fileData.colorFormat );
	swap.Big( fileData.width );
	swap.Big( fileData.height );
	swap.Big( fileData.numLevels );

	if( fileData.headerMagic != BIMAGE_MAGIC_BFG && fileData.headerMagic != BIMAGE_MAGIC )
	{
		return false;
	}

#if defined( DOMINION ) // Blue Textures Fix
	// Validate the cache's allocation-driving fields in release builds too. A mip chain cannot
	// contain more levels than its base dimensions allow; current bimages store 2D images or cubes.
	if( ( fileData.textureType != DTT_2D && fileData.textureType != DTT_CUBIC ) || fileData.format <= FMT_NONE || fileData.format > FMT_RGBA16S ||
			fileData.colorFormat < CFM_DEFAULT || fileData.colorFormat > CFM_GREEN_ALPHA || fileData.width < 1 || fileData.height < 1 || fileData.numLevels < 1 || fileData.numLevels > 31 ||
			( fileData.textureType == DTT_CUBIC && fileData.width != fileData.height ) )
	{
		return false;
	}

	int maxLevels = 1;
	for( int dimension = Max( fileData.width, fileData.height ); dimension > 1; dimension >>= 1 )
	{
		++maxLevels;
	}
	const int faces = fileData.textureType == DTT_CUBIC ? 6 : 1;
	const int64 remainingBytes = int64( bFile->Length() ) - bFile->Tell();
	if( fileData.numLevels > maxLevels || remainingBytes < int64( faces ) * fileData.numLevels * ( int64( sizeof( bimageImage_t ) ) + 1 ) )
	{
		return false;
	}
	const bool blockCompressed = fileData.format == FMT_DXT1 || fileData.format == FMT_DXT5 || fileData.format == FMT_BC6H || fileData.format == FMT_BC7 || fileData.format == FMT_ETC1_RGB8_OES;
	const int bitsPerPixel = BitsForFormat( static_cast<textureFormat_t>( fileData.format ) );
	uint32 seenLevels[6] = {};
#endif

	// RB: source might be from .resources, so we ignore the time stamp and assume a release build
	if( !fileSystem->InProductionMode() && ( sourceTimeStamp != FILE_NOT_FOUND_TIMESTAMP ) && ( sourceTimeStamp != 0 ) && ( sourceTimeStamp != fileData.sourceFileTime ) )
	{
		return false;
	}
	// RB end

	int numImages = fileData.numLevels;
	if( fileData.textureType == DTT_CUBIC )
	{
		numImages *= 6;
	}

	images.SetNum( numImages );

	int finalFormat = fileData.format;

	for( int i = 0; i < numImages; i++ )
	{
		idBinaryImageData& img = images[ i ];
#if defined( DOMINION ) // Blue Textures Fix
		if( bFile->Read( &img, sizeof( bimageImage_t ) ) != sizeof( bimageImage_t ) )
#else
		if( bFile->Read( &img, sizeof( bimageImage_t ) ) <= 0 )
#endif
		{
			return false;
		}
		idSwapClass<bimageImage_t> swap;
		swap.Big( img.level );
		swap.Big( img.destZ );
		swap.Big( img.width );
		swap.Big( img.height );
		swap.Big( img.dataSize );

#if defined( DOMINION ) // Blue Textures Fix
		// Each face/level must occur exactly once. Validate logical extents, legacy tiny cube
		// headers, exact payload size and available bytes before allocating or uploading anything.
		if( img.level < 0 || img.level >= fileData.numLevels || img.destZ < 0 || img.destZ >= faces || img.width < 1 || img.height < 1 || img.dataSize < 1 )
		{
			return false;
		}
		const int logicalWidth = Max( 1, fileData.width >> img.level );
		const int logicalHeight = Max( 1, fileData.height >> img.level );
		const bool legacyCube = blockCompressed && fileData.textureType == DTT_CUBIC;
		// Bound multiplication before calculating byte counts, including malicious large float images.
		if( !blockCompressed && int64( logicalWidth ) * logicalHeight > INT_MAX / Max( 1, bitsPerPixel / 8 ) )
		{
			return false;
		}
		// Bound the complete payload before multiplying rows, including large uncompressed formats.
		// Even a 64-bit product can overflow when both dimensions come from a malformed header.
		const int64 rowBytes = blockCompressed ? ( ( int64( logicalWidth ) + 3 ) / 4 ) * ( bitsPerPixel * 2 ) : int64( logicalWidth ) * ( bitsPerPixel / 8 );
		const int64 rows = blockCompressed ? ( int64( logicalHeight ) + 3 ) / 4 : logicalHeight;
		if( rowBytes < 1 || rowBytes > INT_MAX / rows )
		{
			return false;
		}
		const int64 expectedBytes = rowBytes * rows;
		const int64 allocationBytes = fileData.format == FMT_RGB565 ? expectedBytes * 2 : expectedBytes;
		if( ( img.width != logicalWidth && !( legacyCube && logicalWidth < 4 && img.width == 4 ) ) || ( img.height != logicalHeight && !( legacyCube && logicalHeight < 4 && img.height == 4 ) ) ||
				expectedBytes != img.dataSize || allocationBytes > INT_MAX || expectedBytes > int64( bFile->Length() ) - bFile->Tell() || ( seenLevels[img.destZ] & ( 1u << img.level ) ) != 0 )
		{
			return false;
		}
		seenLevels[img.destZ] |= 1u << img.level;
#else
		assert( img.level >= 0 && img.level < fileData.numLevels );
		assert( img.destZ == 0 || fileData.textureType == DTT_CUBIC );
		assert( img.dataSize > 0 );

		// DXT images need to be padded to 4x4 block sizes, but the original image
		// sizes are still retained, so the stored data size may be larger than
		// just the multiplication of dimensions
		assert( img.dataSize >= img.width * img.height * BitsForFormat( ( textureFormat_t )fileData.format ) / 8 );
#endif

#if ( defined( __APPLE__ ) && defined( USE_VULKAN ) ) || defined( USE_NVRHI )
		int imgfile_dataSize = img.dataSize;

		// SRS - Allocate 2x memory to prepare for in-place conversion from FMT_RGB565 to FMT_RGBA8
		if( ( textureFormat_t )fileData.format == FMT_RGB565 )
		{
			img.Alloc( img.dataSize * 2 );
		}
		// SRS - For compressed formats, match allocation to what NVRHI expects for the texture's mip variants
		else if( ( textureFormat_t )fileData.format == FMT_DXT1 || ( textureFormat_t )fileData.format == FMT_DXT5 || ( textureFormat_t )fileData.format == FMT_BC6H || ( textureFormat_t )fileData.format == FMT_BC7 )
		{
#if defined( DOMINION ) // Blue Textures Fix
			// The old allocation workaround enlarged dataSize for rounded GPU mips but left the extra bytes unread.
			// Logical GPU extents now match the file, so preserve and validate the actual compressed payload size.
			const int blockBytes = fileData.format == FMT_DXT1 ? 8 : 16;
			const int64 expectedBytes = ( ( int64( img.width ) + 3 ) / 4 ) * ( ( int64( img.height ) + 3 ) / 4 ) * blockBytes;
			if( img.width < 1 || img.height < 1 || expectedBytes != imgfile_dataSize )
			{
				return false;
			}
			img.Alloc( imgfile_dataSize );
#else
			int rowPitch = GetRowPitch( ( textureFormat_t )fileData.format, img.width );
			int mipRows = ( ( ( ( fileData.height + 3 ) & ~3 ) >> img.level ) + 3 ) / 4;
			img.Alloc( Max( img.dataSize, rowPitch * mipRows ) );
#endif
		}
		else
		{
			img.Alloc( img.dataSize );
		}

		if( img.data == NULL )
		{
			return false;
		}

		// SRS - Read image data using actual on-disk data size
#if defined( DOMINION ) // Blue Textures Fix
		if( bFile->Read( img.data, imgfile_dataSize ) != imgfile_dataSize )
#else
		if( bFile->Read( img.data, imgfile_dataSize ) <= 0 )
#endif
		{
			return false;
		}

		// SRS - Convert FMT_RGB565 16-bits to FMT_RGBA8 32-bits in place using pre-allocated space
		if( ( textureFormat_t )fileData.format == FMT_RGB565 )
		{
			//SRS - Make sure we have an integer number of RGBA8 storage slots
			assert( img.dataSize % 4 == 0 );
			for( int pixelIndex = img.dataSize / 2 - 2; pixelIndex >= 0; pixelIndex -= 2 )
			{
#if 1
				// SRS - Option 1: Scale and shift algorithm
				uint16 pixelValue_rgb565 = img.data[pixelIndex + 0] << 8 | img.data[pixelIndex + 1];
				img.data[pixelIndex * 2 + 0] = ( ( ( pixelValue_rgb565 ) >> 11 ) * 527 + 23 ) >> 6;
				img.data[pixelIndex * 2 + 1] = ( ( ( pixelValue_rgb565 & 0x07E0 ) >>  5 ) * 259 + 33 ) >> 6;
				img.data[pixelIndex * 2 + 2] = ( ( ( pixelValue_rgb565 & 0x001F ) ) * 527 + 23 ) >> 6;
#else
				// SRS - Option 2: Shift and combine algorithm - is this faster?
				uint8 pixelValue_rgb565_hi = img.data[pixelIndex + 0];
				uint8 pixelValue_rgb565_lo = img.data[pixelIndex + 1];
				img.data[pixelIndex * 2 + 0] = ( pixelValue_rgb565_hi & 0xF8 ) | ( pixelValue_rgb565_hi          >> 5 );
				img.data[pixelIndex * 2 + 1] = ( pixelValue_rgb565_hi        << 5 ) | ( ( pixelValue_rgb565_lo & 0xE0 ) >> 3 ) | ( ( pixelValue_rgb565_hi & 0x07 ) >> 1 );
				img.data[pixelIndex * 2 + 2] = ( pixelValue_rgb565_lo        << 3 ) | ( ( pixelValue_rgb565_lo & 0x1F ) >> 2 );
#endif
				img.data[pixelIndex * 2 + 3] = 0xFF;
			}

			finalFormat = FMT_RGBA8;
		}
#else
		img.Alloc( img.dataSize );
		if( img.data == NULL )
		{
			return false;
		}

		if( bFile->Read( img.data, img.dataSize ) <= 0 )
		{
			return false;
		}
#endif
	}

	fileData.format = finalFormat;

	return true;
}

/*
==========================
idBinaryImage::MakeGeneratedFileName
==========================
*/
void idBinaryImage::MakeGeneratedFileName( idStr& gfn )
{
	GetGeneratedFileName( gfn, GetName() );
}
/*
==========================
idBinaryImage::GetGeneratedFileName
==========================
*/
void idBinaryImage::GetGeneratedFileName( idStr& gfn, const char* name )
{
	gfn.Format( "generated/images/%s.bimage", name );
	gfn.Replace( "(", "/" );
	gfn.Replace( ",", "/" );
	gfn.Replace( ")", "" );
	gfn.Replace( " ", "" );
}


