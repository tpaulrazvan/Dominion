/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

#ifndef __LIB_H__
#define __LIB_H__

#include <stddef.h>

/*
===============================================================================

	idLib contains stateless support classes and concrete types. Some classes
	do have static variables, but such variables are initialized once and
	read-only after initialization (they do not maintain a modifiable state).

	The interface pointers idSys, idCommon, idCVarSystem and idFileSystem
	should be set before using idLib. The pointers stored here should not
	be used by any part of the engine except for idLib.

===============================================================================
*/

class idLib
{
private:
	static bool					mainThreadInitialized;
	static ID_TLS				isMainThread;

public:
	static class idSys* 		sys;
	static class idCommon* 		common;
	static class idCVarSystem* 	cvarSystem;
	static class idFileSystem* 	fileSystem;
	static int					frameNumber;

	static void					Init();
	static void					ShutDown();

	// wrapper to idCommon functions
	static void       			Printf( VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 1, 2 );
	static void       			PrintfIf( const bool test, VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 2, 3 );
	NO_RETURN static void       Error( VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 1, 2 );
	NO_RETURN static void       FatalError( VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 1, 2 );
	static void       			Warning( VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 1, 2 );
	static void       			WarningIf( const bool test, VERIFY_FORMAT_STRING const char* fmt, ... ) ID_STATIC_ATTRIBUTE_PRINTF( 2, 3 );

	// the extra check for mainThreadInitialized is necessary for this to be accurate
	// when called by startup code that happens before idLib::Init
	static bool					IsMainThread()
	{
		return ( 0 == mainThreadInitialized ) || ( 1 == isMainThread );
	}
};


/*
===============================================================================

	Types and defines used throughout the engine.

===============================================================================
*/

typedef int						qhandle_t;

class idFile;
class idVec3;
class idVec4;

#ifndef NULL
	#define NULL					((void *)0)
#endif

#ifndef BIT
	#define BIT( num )				( 1ULL << ( num ) )
#endif

#define	MAX_STRING_CHARS		1024		// max length of a string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

#define SIZE_KB( x )						( ( (x) + 1023 ) / 1024 )
#define SIZE_MB( x )						( ( ( SIZE_KB( x ) ) + 1023 ) / 1024 )
#define SIZE_GB( x )						( ( ( SIZE_MB( x ) ) + 1023 ) / 1024 )

// Color declarations (alphabetical order)
extern const idVec4 colorAqua;
extern const idVec4 colorBlack;
extern const idVec4 colorBlue;
extern const idVec4 colorBrown;
extern const idVec4 colorBurlyWood;
extern const idVec4 colorCoral;
extern const idVec4 colorCrimson;
extern const idVec4 colorCyan;
extern const idVec4 colorDarkBlue;
extern const idVec4 colorDarkCyan;
extern const idVec4 colorDarkGoldenRod;
extern const idVec4 colorDarkKhaki;
extern const idVec4 colorDarkSalmon;
extern const idVec4 colorDarkSlateGray;
extern const idVec4 colorDkGrey;
extern const idVec4 colorDodgerBlue;
extern const idVec4 colorFuchsia;
extern const idVec4 colorGold;
extern const idVec4 colorGray;
extern const idVec4 colorGreen;
extern const idVec4 colorLightSeaGreen;
extern const idVec4 colorLightSteelBlue;
extern const idVec4 colorLtGrey;
extern const idVec4 colorLime;
extern const idVec4 colorMagenta;
extern const idVec4 colorMaroon;
extern const idVec4 colorMdGrey;
extern const idVec4 colorNavy;
extern const idVec4 colorOlive;
extern const idVec4 colorOrange;
extern const idVec4 colorPink;
extern const idVec4 colorPurple;
extern const idVec4 colorRed;
extern const idVec4 colorSilver;
extern const idVec4 colorTeal;
extern const idVec4 colorWhite;
extern const idVec4 colorYellow;


// packs color floats in the range [0,1] into an integer
dword	PackColor( const idVec3& color );
void	UnpackColor( const dword color, idVec3& unpackedColor );
dword	PackColor( const idVec4& color );
void	UnpackColor( const dword color, idVec4& unpackedColor );

// little/big endian conversion
short	BigShort( short l );
short	LittleShort( short l );
int		BigLong( int l );
int		LittleLong( int l );
float	BigFloat( float l );
float	LittleFloat( float l );
void	BigRevBytes( void* bp, int elsize, int elcount );
void	LittleRevBytes( void* bp, int elsize, int elcount );
void	LittleBitField( void* bp, int elsize );
void	Swap_Init();

bool	Swap_IsBigEndian();

// for base64
void	SixtetsForInt( byte* out, int src );
int		IntForSixtets( byte* in );

/*
================================================
idException
================================================
*/
class idException
{
public:
	static const int MAX_ERROR_LEN = 2048;

	idException( const char* text = "" )
	{
		strncpy( error, text, MAX_ERROR_LEN );
	}

	// this really, really should be a const function, but it's referenced too many places to change right now
	const char* 	GetError()
	{
		return error;
	}

protected:
	// if GetError() were correctly const this would be named GetError(), too
	char* 		GetErrorBuffer()
	{
		return error;
	}
	int			GetErrorBufferSize()
	{
		return MAX_ERROR_LEN;
	}

private:
	friend class idFatalException;
	static char error[MAX_ERROR_LEN];
};

/*
================================================
idFatalException
================================================
*/
class idFatalException
{
public:
	static const int MAX_ERROR_LEN = 2048;

	idFatalException( const char* text = "" )
	{
		strncpy( idException::error, text, MAX_ERROR_LEN );
	}

	// this really, really should be a const function, but it's referenced too many places to change right now
	const char* 	GetError()
	{
		return idException::error;
	}

protected:
	// if GetError() were correctly const this would be named GetError(), too
	char* 		GetErrorBuffer()
	{
		return idException::error;
	}
	int			GetErrorBufferSize()
	{
		return MAX_ERROR_LEN;
	}
};

/*
================================================
idNetworkLoadException
================================================
*/
class idNetworkLoadException : public idException
{
public:
	idNetworkLoadException( const char* text = "" ) : idException( text ) { }
};

/*
===============================================================================

	idLib headers.

===============================================================================
*/

// System
#include "Sys/sys_assert.h"
#include "Sys/sys_threading.h"

// memory management and arrays
#include "Heap.h"
#include "Containers/Sort.h"
#include "Containers/List.h"

// math
#include "Math/Simd.h"
#include "Math/Math.h"
#include "Math/Random.h"
#include "Math/Complex.h"
#include "Math/Vector.h"
#include "Math/VecX.h"
#include "Math/VectorI.h"
#include "Math/Matrix.h"
#include "Math/MatX.h"
#include "Math/Angles.h"
#include "Math/Quat.h"
#include "Math/Rotation.h"
#include "Math/Plane.h"
#include "Math/Pluecker.h"
#include "Math/Polynomial.h"
#include "Math/Extrapolate.h"
#include "Math/Interpolate.h"
#include "Math/Curve.h"
#include "Math/Ode.h"
#include "Math/Lcp.h"
#include "Math/SphericalHarmonics.h"

// bounding volumes
#include "BV/Sphere.h"
#include "BV/Bounds.h"
#include "BV/Box.h"

// geometry
#include "Geometry/RenderMatrix.h"
#include "Geometry/JointTransform.h"
#include "Geometry/DrawVert.h"
#include "Geometry/Winding.h"
#include "Geometry/Winding2D.h"
#include "Geometry/Surface.h"
#include "Geometry/Surface_Patch.h"
#include "Geometry/Surface_Polytope.h"
#include "Geometry/Surface_SweptSpline.h"
#include "Geometry/TraceModel.h"

// text manipulation
#include "Str.h"
#include "StrStatic.h"
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Base64.h"
#include "CmdArgs.h"

// containers
#include "Containers/Array.h"
#include "Containers/BTree.h"
#include "Containers/BinSearch.h"
#include "Containers/HashIndex.h"
#include "Containers/HashTable.h"
#include "Containers/StaticList.h"
#include "Containers/LinkList.h"
#include "Containers/Hierarchy.h"
#include "Containers/Queue.h"
#include "Containers/Stack.h"
#include "Containers/StrList.h"
#include "Containers/StrPool.h"
#include "Containers/VectorSet.h"
#include "Containers/PlaneSet.h"

// hashing
#include "Hashing/CRC32.h"
#include "Hashing/MD4.h"
#include "Hashing/MD5.h"

// misc
#include "Dict.h"
#include "LangDict.h"
#include "DataQueue.h"
#include "BitMsg.h"
#include "MapFile.h"
#include "Timer.h"
#include "Thread.h"
#include "Swap.h"
#include "Callback.h"
#include "ParallelJobList.h"
#include "SoftwareCache.h"
#include "TileMap.h" // RB

#endif	/* !__LIB_H__ */
