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

#ifndef __MATH_MATH_H__
#define __MATH_MATH_H__

/*
===============================================================================

  Math

===============================================================================
*/

#ifdef INFINITUM
	#undef INFINITUM
#endif

#ifdef FLOAT_EPSILON
	#undef FLOAT_EPSILON
#endif

#ifdef DBL_EPSILON
	#undef DBL_EPSILON
#endif

#define DEG2RAD(a)				( (a) * idMath::M_DEG2RAD )
#define RAD2DEG(a)				( (a) * idMath::M_RAD2DEG )

#define SEC2MS(t)				( idMath::Ftoi( (t) * idMath::M_SEC2MS ) )
#define MS2SEC(t)				( (t) * idMath::M_MS2SEC )

#define	ANGLE2SHORT(x)			( idMath::Ftoi( (x) * 65536.0f / 360.0f ) & 65535 )
#define	SHORT2ANGLE(x)			( (x) * ( 360.0f / 65536.0f ) )

#define	ANGLE2BYTE(x)			( idMath::Ftoi( (x) * 256.0f / 360.0f ) & 255 )
#define	BYTE2ANGLE(x)			( (x) * ( 360.0f / 256.0f ) )

#define C_FLOAT_TO_INT( x )		(int)(x)


#define C_FLOAT_TO_ULONG(x) (unsigned long)(x)

/*
================================================================================================

	two-complements integer bit layouts

================================================================================================
*/

#define INT8_SIGN_BIT		7
#define INT16_SIGN_BIT		15
#define INT32_SIGN_BIT		31
#define INT64_SIGN_BIT		63

#define INT8_SIGN_MASK		( 1 << INT8_SIGN_BIT )
#define INT16_SIGN_MASK		( 1 << INT16_SIGN_BIT )
#define INT32_SIGN_MASK		( 1UL << INT32_SIGN_BIT )
#define INT64_SIGN_MASK		( 1ULL << INT64_SIGN_BIT )

/*
================================================================================================

	integer sign bit tests

================================================================================================
*/

// If this was ever compiled on a system that had 64 bit unsigned ints,
// it would fail.
compile_time_assert( sizeof( unsigned int ) == 4 );

#define OLD_INT32_SIGNBITSET(i)		(static_cast<const unsigned int>(i) >> INT32_SIGN_BIT)
#define OLD_INT32_SIGNBITNOTSET(i)	((~static_cast<const unsigned int>(i)) >> INT32_SIGN_BIT)

// Unfortunately, /analyze can't figure out that these always return
// either 0 or 1, so this extra wrapper is needed to avoid the static
// alaysis warning.

ID_INLINE_EXTERN int INT32_SIGNBITSET( int i )
{
	int	r = OLD_INT32_SIGNBITSET( i );
	assert( r == 0 || r == 1 );
	return r;
}

ID_INLINE_EXTERN int INT32_SIGNBITNOTSET( int i )
{
	int	r = OLD_INT32_SIGNBITNOTSET( i );
	assert( r == 0 || r == 1 );
	return r;
}

/*
================================================================================================

	floating point bit layouts according to the IEEE 754-1985 and 754-2008 standard

================================================================================================
*/

#define IEEE_FLT16_MANTISSA_BITS	10
#define IEEE_FLT16_EXPONENT_BITS	5
#define IEEE_FLT16_EXPONENT_BIAS	15
#define IEEE_FLT16_SIGN_BIT			15
#define IEEE_FLT16_SIGN_MASK		( 1U << IEEE_FLT16_SIGN_BIT )

#define IEEE_FLT_MANTISSA_BITS		23
#define IEEE_FLT_EXPONENT_BITS		8
#define IEEE_FLT_EXPONENT_BIAS		127
#define IEEE_FLT_SIGN_BIT			31
#define IEEE_FLT_SIGN_MASK			( 1UL << IEEE_FLT_SIGN_BIT )	// Keep the L or not ??

#define IEEE_DBL_MANTISSA_BITS		52
#define IEEE_DBL_EXPONENT_BITS		11
#define IEEE_DBL_EXPONENT_BIAS		1023
#define IEEE_DBL_SIGN_BIT			63
#define IEEE_DBL_SIGN_MASK			( 1ULL << IEEE_DBL_SIGN_BIT )

#define IEEE_DBLE_MANTISSA_BITS		63
#define IEEE_DBLE_EXPONENT_BITS		15
#define IEEE_DBLE_EXPONENT_BIAS		0
#define IEEE_DBLE_SIGN_BIT			79

/*
================================================================================================

	floating point sign bit tests

================================================================================================
*/

#define IEEE_FLT_SIGNBITSET( a )	(reinterpret_cast<const unsigned int &>(a) >> IEEE_FLT_SIGN_BIT)
#define IEEE_FLT_SIGNBITNOTSET( a )	((~reinterpret_cast<const unsigned int &>(a)) >> IEEE_FLT_SIGN_BIT)
#define IEEE_FLT_ISNOTZERO( a )		(reinterpret_cast<const unsigned int &>(a) & ~(1u<<IEEE_FLT_SIGN_BIT))

/*
================================================================================================

	floating point special value tests

================================================================================================
*/

/*
========================
IEEE_FLT_IS_NAN
========================
*/
ID_INLINE_EXTERN bool IEEE_FLT_IS_NAN( float x )
{
	return x != x;
}

/*
========================
IEEE_FLT_IS_INF
========================
*/
ID_INLINE_EXTERN bool IEEE_FLT_IS_INF( float x )
{
	return x == x && x * 0 != x * 0;
}

/*
========================
IEEE_FLT_IS_INF_NAN
========================
*/
ID_INLINE_EXTERN bool IEEE_FLT_IS_INF_NAN( float x )
{
	return x * 0 != x * 0;
}

/*
========================
IEEE_FLT_IS_IND
========================
*/
ID_INLINE_EXTERN bool IEEE_FLT_IS_IND( float x )
{
	return	( reinterpret_cast<const unsigned int&>( x ) == 0xffc00000 );
}

/*
========================
IEEE_FLT_IS_DENORMAL
========================
*/
ID_INLINE_EXTERN bool IEEE_FLT_IS_DENORMAL( float x )
{
	return ( ( reinterpret_cast<const unsigned int&>( x ) & 0x7f800000 ) == 0x00000000 &&
			 ( reinterpret_cast<const unsigned int&>( x ) & 0x007fffff ) != 0x00000000 );
}


/*
========================
IsNAN
========================
*/template<class type>
ID_INLINE_EXTERN bool IsNAN( const type& v )
{
	for( int i = 0; i < v.GetDimension(); i++ )
	{
		const float f = v.ToFloatPtr()[i];
		if( IEEE_FLT_IS_NAN( f ) || IEEE_FLT_IS_INF( f ) || IEEE_FLT_IS_IND( f ) )
		{
			return true;
		}
	}
	return false;
}

/*
========================
IsValid
========================
*/
template<class type>
ID_INLINE_EXTERN bool IsValid( const type& v )
{
	for( int i = 0; i < v.GetDimension(); i++ )
	{
		const float f = v.ToFloatPtr()[i];
		if( IEEE_FLT_IS_NAN( f ) || IEEE_FLT_IS_INF( f ) || IEEE_FLT_IS_IND( f ) || IEEE_FLT_IS_DENORMAL( f ) )
		{
			return false;
		}
	}
	return true;
}

/*
========================
IsValid

This should be a template ???
========================
*/
template<>
ID_INLINE
bool IsValid( const float& f )  	// these parameter must be a reference for the function to be considered a specialization
{
	return !( IEEE_FLT_IS_NAN( f ) || IEEE_FLT_IS_INF( f ) || IEEE_FLT_IS_IND( f ) || IEEE_FLT_IS_DENORMAL( f ) );
}

/*
========================
IsNAN

This should be a template ???
========================
*/
template<>
ID_INLINE
bool IsNAN( const float& f )  	// these parameter must be a reference for the function to be considered a specialization
{
	if( IEEE_FLT_IS_NAN( f ) || IEEE_FLT_IS_INF( f ) || IEEE_FLT_IS_IND( f ) )
	{
		return true;
	}
	return false;
}

/*
========================
IsInRange

Returns true if any scalar is greater than the range or less than the negative range.
========================
*/
template<class type>
ID_INLINE
bool IsInRange( const type& v, const float range )
{
	for( int i = 0; i < v.GetDimension(); i++ )
	{
		const float f = v.ToFloatPtr()[i];
		if( f > range || f < -range )
		{
			return false;
		}
	}
	return true;
}


/*
================================================================================================

	MinIndex/MaxIndex

================================================================================================
*/
template<class T> ID_INLINE int	MaxIndex( T x, T y )
{
	return ( x > y ) ? 0 : 1;
}
template<class T> ID_INLINE int	MinIndex( T x, T y )
{
	return ( x < y ) ? 0 : 1;
}

template<class T> ID_INLINE T	Max3( T x, T y, T z )
{
	return ( x > y ) ? ( ( x > z ) ? x : z ) : ( ( y > z ) ? y : z );
}
template<class T> ID_INLINE T	Min3( T x, T y, T z )
{
	return ( x < y ) ? ( ( x < z ) ? x : z ) : ( ( y < z ) ? y : z );
}
template<class T> ID_INLINE int	Max3Index( T x, T y, T z )
{
	return ( x > y ) ? ( ( x > z ) ? 0 : 2 ) : ( ( y > z ) ? 1 : 2 );
}
template<class T> ID_INLINE int	Min3Index( T x, T y, T z )
{
	return ( x < y ) ? ( ( x < z ) ? 0 : 2 ) : ( ( y < z ) ? 1 : 2 );
}

/*
================================================================================================

	Sign/Square/Cube

================================================================================================
*/
template<class T> ID_INLINE T	Sign( T f )
{
	return ( f > 0 ) ? 1 : ( ( f < 0 ) ? -1 : 0 );
}
template<class T> ID_INLINE T	Square( T x )
{
	return x * x;
}
template<class T> ID_INLINE T	Cube( T x )
{
	return x * x * x;
}

// ADD Templated CLAMP ???
#if 0
template<class T>
inline T idMath::Clamp( const T& i, const T& mi, const T& ma )
{
	return i > ma ? ma : ( i < mi ? mi : i );
}
#endif

#if 0
inline float idMath::IntBitsToFloat( int i )
{
	union bit
	{
		int i = 0;
		float f;
	} u;
	memset( &u, 0, sizeof( union bit ) );
	u.i = i;
	return u.f;
}
// NETLizard
//ID_INLINE float IntBitsToFloat( uint32_t bits )
//{
//	return std::bit_cast<float>(bits);
//}
#endif

class idMath
{
public:

	static void					Init();

	static float				RSqrt( float x );			// reciprocal square root, returns huge number when x == 0.0

	static float				InvSqrt( float x );			// inverse square root with 32 bits precision, returns huge number when x == 0.0
	static float				InvSqrt16( float x );		// inverse square root with 16 bits precision, returns huge number when x == 0.0

	static float				Sqrt( float x );			// square root with 32 bits precision
	static float				Sqrt16( float x );			// square root with 16 bits precision

	static float				Sin( float a );				// sine with 32 bits precision

	// Documentation from BOOM3
	// https://github.com/libretro/boom3/commit/e87ec65e93dcb4d486417a195e622d638ec39077
	/*	Commit
		idlib: document what the *16 math functions actually do
		Two comments in Math.h describe behaviour the code does not have.

		FtoiFast is named for, and its MSVC branch comments, round-to-nearest
		via fistp. Every other target - which is every target we ship - falls
		through to a plain (int) cast, i.e. truncation toward zero. This is
		visible in ANGLE2SHORT/ANGLE2BYTE, so network angle quantisation does
		not match the original 32-bit Windows build. It is consistent across
		our targets and so not a determinism problem, and changing it would
		alter behaviour relative to shipped builds, so the behaviour is left
		alone and only the comment is corrected. (Also fixes 'rouding'.)

		The 'maximum absolute error' figures on the *16 trig functions are the
		minimax polynomial's error in double. Evaluated in float, as the code
		does, rounding of the Horner chain dominates and measured error is
		~1e-07 - about 100x the quoted 2.3082e-09 for Sin16/Cos16, and 13x the
		quoted figure for ATan16. Measured against libm over the full float
		exponent range.

		Sin16/Cos16 additionally lose accuracy with argument magnitude, because
		range reduction uses a float TWO_PI that is 2.78e-08 off 2*pi and
		reduction scales that by the quotient. Measured: 2.27e-04 at |a|~2^11,
		5.99e-02 at |a|~2^19, 8.74e-01 at |a|~2^23. idMath::Sin holds at
		3.2e-08 throughout because it calls libm, which reduces properly.

		In practice this is minor: the callers passing unbounded time-derived
		arguments - AI.cpp fly bob and aim error, Weapon.cpp trace jitter - are
		all cosmetic, and see ~3.8e-04 error at an hour of playtime. Documented
		rather than changed, with a pointer to Sin/Cos for callers that care.

		No functional change.
	*/

	/*
		The "maximum absolute error" figures on the *16 trig functions below are
		the error of the underlying minimax polynomial evaluated in double. The
		code evaluates in float, so float rounding of the Horner chain dominates
		and the measured error is ~1e-07, roughly two orders of magnitude larger.

		Separately, Sin16/Cos16 reduce range with a -= floorf(a/TWO_PI)*TWO_PI.
		TWO_PI as a float is 2.78e-08 off 2*pi, and reduction multiplies that by
		the quotient, so phase error grows linearly with |a|: ~2e-04 at |a|~2^11
		and ~0.9 at |a|~2^24, by which point the result is meaningless. Callers
		passing an unbounded time-derived argument (see AI.cpp and Weapon.cpp)
		should prefer Sin/Cos, which call libm and reduce properly.
	*/

	static float				Sin16( float a );			// sine with 16 bits precision, maximum absolute error is 2.3082e-09

	static float				Cos( float a );				// cosine with 32 bits precision
	static float				Cos16( float a );			// cosine with 16 bits precision, maximum absolute error is 2.3082e-09

	static void					SinCos( float a, float& s, float& c );		// sine and cosine with 32 bits precision
	static void					SinCos16( float a, float& s, float& c );	// sine and cosine with 16 bits precision

	// NOTE: other sin funcs in doom 3 may have bugs, review before if want to bring them back!

	static float				Tan( float a );				// tangent with 32 bits precision
	static float				Tan16( float a );			// tangent with 16 bits precision, maximum absolute error is 1.8897e-08

	static float				ASin( float a );			// arc sine with 32 bits precision, input is clamped to [-1, 1] to avoid a silent NaN
	static float				ASin16( float a );			// arc sine with 16 bits precision, maximum absolute error is 6.7626e-05

	static float				ACos( float a );			// arc cosine with 32 bits precision, input is clamped to [-1, 1] to avoid a silent NaN
	static float				ACos16( float a );			// arc cosine with 16 bits precision, maximum absolute error is 6.7626e-05

	static float				ATan( float a );			// arc tangent with 32 bits precision
	static float				ATan16( float a );			// arc tangent with 16 bits precision, maximum absolute error is 1.3593e-08

	static float				ATan( float y, float x );	// arc tangent with 32 bits precision
	static float				ATan16( float y, float x );	// arc tangent with 16 bits precision, maximum absolute error is 1.3593e-08

	static float				Pow( float x, float y );	// x raised to the power y with 32 bits precision
	static float				Pow16( float x, float y );	// x raised to the power y with 16 bits precision

	static float				Exp( float f );				// e raised to the power f with 32 bits precision
	static float				Exp16( float f );			// e raised to the power f with 16 bits precision

	static float				Log( float f );				// natural logarithm with 32 bits precision
	static float				Log16( float f );			// natural logarithm with 16 bits precision

	static int					IPow( int x, int y );		// integral x raised to the power y
	static int					ILog2( float f );			// integral base-2 logarithm of the floating point value
	static int					ILog2( int i );				// integral base-2 logarithm of the integer value

	static int					BitsForFloat( float f );	// minumum number of bits required to represent ceil( f )
	static int					BitsForInteger( int i );	// minumum number of bits required to represent i
	static int					MaskForFloatSign( float f );// returns 0x00000000 if x >= 0.0f and returns 0xFFFFFFFF if x <= -0.0f
	static int					MaskForIntegerSign( int i );// returns 0x00000000 if x >= 0 and returns 0xFFFFFFFF if x < 0
	static int					FloorPowerOfTwo( int x );	// round x down to the nearest power of 2
	static int					CeilPowerOfTwo( int x );	// round x up to the nearest power of 2
	static bool					IsPowerOfTwo( int x );		// returns true if x is a power of 2
	static int					BitCount( int x );			// returns the number of 1 bits in x
	static int					BitReverse( int x );		// returns the bit reverse of x

	static int					Abs( int x );				// returns the absolute value of the integer value (for reference only)
	static float				Fabs( float f );			// returns the absolute value of the floating point value
	static float				Floor( float f );			// returns the largest integer that is less than or equal to the given value
	static float				Ceil( float f );			// returns the smallest integer that is greater than or equal to the given value
	static float				Rint( float f );			// returns the nearest integer

	static float				Frac( float f );			// f - Floor( f )


	// xyzz: should we check for NaNs ???
	static int					Ftoi( float f );			// float to int conversion
	static char					Ftoi8( float f );			// float to char conversion
	static short				Ftoi16( float f );			// float to short conversion
	static unsigned short		Ftoui16( float f );			// float to unsigned short conversion
	static byte					Ftob( float f );			// float to byte conversion, the result is clamped to the range [0-255]

	static signed char			ClampChar( int i );
	static signed short			ClampShort( int i );
	static int					ClampInt( int min, int max, int value );
	static float				ClampFloat( float min, float max, float value );
	static byte					ClampByte( byte min, byte max, int value );	// Revelation!


	static float				AngleNormalize360( float angle );
	static float				AngleNormalize180( float angle );
	static float				AngleDelta( float angle1, float angle2 );

	static int					FloatToBits( float f, int exponentBits, int mantissaBits );
	static float				BitsToFloat( int i, int exponentBits, int mantissaBits );

	static int					FloatHash( const float* array, const int numFloats );
	// add vector hash
//	public static int VectorHash(Vector3 v)
//	{
//		int hash = 0;
//
//		hash ^= (int)v.X;
//		hash ^= (int)v.Y;
//		hash ^= (int)v.Z;
//
//		return hash;
//	}

	static float				LerpToWithScale( const float cur, const float dest, const float scale );

	static int					gcd( int a, int b );	// TwelveEyes --> used in dhewm3-sdk-tfphobos to get aspect ratio
	// TODO:: add lcm


	static float				MinNormalizeMax( float number, float max, float min );	// Biel Bestué de Luna --> dhewm3 fork / branch new_movement

	// NETLizard (needed ? IsNearlyZero, isZero (separate) )
	inline bool IsZero( float f )
	{
		return fabs( f ) < FLOAT_EPSILON;
	}

	static const float			PI;							// pi
	static const float			TWO_PI;						// pi * 2
	static const float			HALF_PI;					// pi / 2
	static const float			ONEFOURTH_PI;				// pi / 4
	static const float			ONEOVER_PI;					// 1 / pi
	static const float			ONEOVER_TWOPI;				// 1 / pi * 2
	static const float			E;							// e
	static const float			SQRT_TWO;					// sqrt( 2 )
	static const float			SQRT_THREE;					// sqrt( 3 )
	static const float			SQRT_1OVER2;				// sqrt( 1 / 2 )
	static const float			SQRT_1OVER3;				// sqrt( 1 / 3 )
	static const float			M_DEG2RAD;					// degrees to radians multiplier
	static const float			M_RAD2DEG;					// radians to degrees multiplier
	static const float			M_SEC2MS;					// seconds to milliseconds multiplier
	static const float			M_MS2SEC;					// milliseconds to seconds multiplier
	static const float			INFINITUM;					// huge number which should be larger than any valid number used
	static const float			FLOAT_EPSILON;				// smallest positive number such that 1.0+FLOAT_EPSILON != 1.0
	static const float                        DBL_EPSILON;  // smallest positive number such that 1.0+DBL_EPSILON != 1.0  (REVELATION)
	static const float			FLT_SMALLEST_NON_DENORMAL;	// smallest non-denormal 32-bit floating point value

#if defined(USE_INTRINSICS_SSE)
	static const __m128				SIMD_SP_zero;
	static const __m128				SIMD_SP_255;
	static const __m128				SIMD_SP_min_char;
	static const __m128				SIMD_SP_max_char;
	static const __m128				SIMD_SP_min_short;
	static const __m128				SIMD_SP_max_short;
	static const __m128				SIMD_SP_smallestNonDenorm;
	static const __m128				SIMD_SP_tiny;
	static const __m128				SIMD_SP_rsqrt_c0;
	static const __m128				SIMD_SP_rsqrt_c1;
#endif

private:
	enum
	{
		LOOKUP_BITS				= 8,
		EXP_POS					= 23,
		EXP_BIAS				= 127,
		LOOKUP_POS				= ( EXP_POS - LOOKUP_BITS ),
		SEED_POS				= ( EXP_POS - 8 ),
		SQRT_TABLE_SIZE			= ( 2 << LOOKUP_BITS ),
		LOOKUP_MASK				= ( SQRT_TABLE_SIZE - 1 )
	};

	union _flint
	{
		dword					i;
		float					f;
	};

	static dword				iSqrt[SQRT_TABLE_SIZE];
	static bool					initialized;
};

#if 0 // NETLizard

void remove_fraction( float& f, int prec )
{
	if( prec <= 0 )
	{
		f = float( int( f ) );
	}
	else
	{
		std::string str = idStr::va( "%f", f );
#define _DECIMAL_CASE(x) \
			case x: {\
				dec::decimal<x, dec::half_up_round_policy> d##x(str); \
				f = (float)d##x.getAsDouble(); \
		break; \
					}

		switch( prec )
		{
				_DECIMAL_CASE( 1 );
				_DECIMAL_CASE( 2 );
				_DECIMAL_CASE( 3 );
				_DECIMAL_CASE( 4 );
				_DECIMAL_CASE( 5 );
				_DECIMAL_CASE( 6 );
		};
	}
}
#if 0
#define _DECIMAL_CASE(x) \
			case x: {\
				dec::decimal<x, dec::half_up_round_policy> d##x(str); d##x *= pow(10, x); \
						int i = d##x.getAsInteger(); idStr n = idStr::va("%06d", i); n.insert(n.length() - x, ".");  sscanf(n, "%f", &f); printf("%s\n", n.c_str());\
				 \
		break; \
					}
#endif

#endif

ID_INLINE byte CLAMP_BYTE( int x )
{
	return ( ( x ) < 0 ? ( 0 ) : ( ( x ) > 255 ? 255 : ( byte )( x ) ) );
}


ID_INLINE float idMath::RSqrt( float x )
{
	int i;
	float y, r;

	y = x * 0.5f;
	i = *reinterpret_cast< int* >( &x );
	i = 0x5f3759df - ( i >> 1 );
	r = *reinterpret_cast< float* >( &i );
	r = r * ( 1.5f - r * r * y );
	return r;
#if 0
	uint32_t i = std::bit_cast<uint32_t>( x );
	i = 0x5f3759dfu - ( i >> 1 );

	float r = std::bit_cast<float>( i );
#endif
}


/*
========================
idMath::InvSqrt
========================
*/
ID_INLINE float idMath::InvSqrt( float x )
{

	return ( x > FLT_SMALLEST_NON_DENORMAL ) ? sqrtf( 1.0f / x ) : INFINITUM;

}

/*
========================
idMath::InvSqrt16
========================
*/
ID_INLINE float idMath::InvSqrt16( float x )
{
	return ( x > FLT_SMALLEST_NON_DENORMAL ) ? sqrtf( 1.0f / x ) : INFINITUM;
}

/*
========================
idMath::Sqrt
========================
*/
ID_INLINE float idMath::Sqrt( float x )
{
	return ( x >= 0.0f ) ?  x * InvSqrt( x ) : 0.0f;
}

/*
========================
idMath::Sqrt16
========================
*/
ID_INLINE float idMath::Sqrt16( float x )
{
	return ( x >= 0.0f ) ?  x * InvSqrt16( x ) : 0.0f;
}

/*
========================
idMath::Frac
========================
*/
ID_INLINE float idMath::Frac( float f )
{
	return f - floorf( f );
}

/*
========================
idMath::Sin
========================
*/
ID_INLINE float idMath::Sin( float a )
{
	return sinf( a );
}

/*
========================
idMath::Sin16
========================
*/
ID_INLINE float idMath::Sin16( float a )
{
	float s;

	if( ( a < 0.0f ) || ( a >= TWO_PI ) )
	{
		a -= floorf( a * ONEOVER_TWOPI ) * TWO_PI;
	}
#if 1
	if( a < PI )
	{
		if( a > HALF_PI )
		{
			a = PI - a;
		}
	}
	else
	{
		if( a > PI + HALF_PI )
		{
			a = a - TWO_PI;
		}
		else
		{
			a = PI - a;
		}
	}
#else
	a = PI - a;
	if( fabsf( a ) >= HALF_PI )
	{
		a = ( ( a < 0.0f ) ? -PI : PI ) - a;
	}
#endif
	s = a * a;
	return a * ( ( ( ( ( -2.39e-08f * s + 2.7526e-06f ) * s - 1.98409e-04f ) * s + 8.3333315e-03f ) * s - 1.666666664e-01f ) * s + 1.0f );
}

/*
========================
idMath::Cos
========================
*/
ID_INLINE float idMath::Cos( float a )
{
	return cosf( a );
}

/*
========================
idMath::Cos16
========================
*/
ID_INLINE float idMath::Cos16( float a )
{
	float s, d;

	if( ( a < 0.0f ) || ( a >= TWO_PI ) )
	{
		a -= floorf( a * ONEOVER_TWOPI ) * TWO_PI;
	}
#if 1
	if( a < PI )
	{
		if( a > HALF_PI )
		{
			a = PI - a;
			d = -1.0f;
		}
		else
		{
			d = 1.0f;
		}
	}
	else
	{
		if( a > PI + HALF_PI )
		{
			a = a - TWO_PI;
			d = 1.0f;
		}
		else
		{
			a = PI - a;
			d = -1.0f;
		}
	}
#else
	a = PI - a;
	if( fabsf( a ) >= HALF_PI )
	{
		a = ( ( a < 0.0f ) ? -PI : PI ) - a;
		d = 1.0f;
	}
	else
	{
		d = -1.0f;
	}
#endif
	s = a * a;
	return d * ( ( ( ( ( -2.605e-07f * s + 2.47609e-05f ) * s - 1.3888397e-03f ) * s + 4.16666418e-02f ) * s - 4.999999963e-01f ) * s + 1.0f );
}

/*
========================
idMath::SinCos
========================
*/
ID_INLINE void idMath::SinCos( float a, float& s, float& c )
{
#if defined(_MSC_VER) && defined(_M_IX86)
	_asm
	{
		fld		a
		fsincos
		mov		ecx, c
		mov		edx, s
		fstp	dword ptr [ecx]
		fstp	dword ptr [edx]
	}
#else
	// DG: non-MSVC version
	s = sinf( a );
	c = cosf( a );
	// DG end
#endif
}

/*
========================
idMath::SinCos16
========================
*/
ID_INLINE void idMath::SinCos16( float a, float& s, float& c )
{
	float t, d;

	if( ( a < 0.0f ) || ( a >= TWO_PI ) )
	{
		a -= floorf( a * ONEOVER_TWOPI ) * TWO_PI;
	}
#if 1
	if( a < PI )
	{
		if( a > HALF_PI )
		{
			a = PI - a;
			d = -1.0f;
		}
		else
		{
			d = 1.0f;
		}
	}
	else
	{
		if( a > PI + HALF_PI )
		{
			a = a - TWO_PI;
			d = 1.0f;
		}
		else
		{
			a = PI - a;
			d = -1.0f;
		}
	}
#else
	a = PI - a;
	if( fabsf( a ) >= HALF_PI )
	{
		a = ( ( a < 0.0f ) ? -PI : PI ) - a;
		d = 1.0f;
	}
	else
	{
		d = -1.0f;
	}
#endif
	t = a * a;
	s = a * ( ( ( ( ( -2.39e-08f * t + 2.7526e-06f ) * t - 1.98409e-04f ) * t + 8.3333315e-03f ) * t - 1.666666664e-01f ) * t + 1.0f );
	c = d * ( ( ( ( ( -2.605e-07f * t + 2.47609e-05f ) * t - 1.3888397e-03f ) * t + 4.16666418e-02f ) * t - 4.999999963e-01f ) * t + 1.0f );
}

/*
========================
idMath::Tan
========================
*/
ID_INLINE float idMath::Tan( float a )
{
	return tanf( a );
}

/*
========================
idMath::Tan16
========================
*/
ID_INLINE float idMath::Tan16( float a )
{
	float s;
	bool reciprocal;

	if( ( a < 0.0f ) || ( a >= PI ) )
	{
		a -= floorf( a * ONEOVER_PI ) * PI;
	}
#if 1
	if( a < HALF_PI )
	{
		if( a > ONEFOURTH_PI )
		{
			a = HALF_PI - a;
			reciprocal = true;
		}
		else
		{
			reciprocal = false;
		}
	}
	else
	{
		if( a > HALF_PI + ONEFOURTH_PI )
		{
			a = a - PI;
			reciprocal = false;
		}
		else
		{
			a = HALF_PI - a;
			reciprocal = true;
		}
	}
#else
	a = HALF_PI - a;
	if( fabsf( a ) >= ONEFOURTH_PI )
	{
		a = ( ( a < 0.0f ) ? -HALF_PI : HALF_PI ) - a;
		reciprocal = false;
	}
	else
	{
		reciprocal = true;
	}
#endif
	s = a * a;
	s = a * ( ( ( ( ( ( 9.5168091e-03f * s + 2.900525e-03f ) * s + 2.45650893e-02f ) * s + 5.33740603e-02f ) * s + 1.333923995e-01f ) * s + 3.333314036e-01f ) * s + 1.0f );
	if( reciprocal )
	{
		return 1.0f / s;
	}
	else
	{
		return s;
	}
}

/*
========================
idMath::ASin
========================
*/
ID_INLINE float idMath::ASin( float a )
{
	if( a <= -1.0f )
	{
		return -HALF_PI;
	}
	if( a >= 1.0f )
	{
		return HALF_PI;
	}
	return asinf( a );
}

/*
========================
idMath::ASin16
========================
*/
ID_INLINE float idMath::ASin16( float a )
{
	if( a < 0.0f )
	{
		if( a <= -1.0f )
		{
			return -HALF_PI;
		}
		a = fabsf( a );
		return ( ( ( -0.0187293f * a + 0.0742610f ) * a - 0.2121144f ) * a + 1.5707288f ) * idMath::Sqrt( 1.0f - a ) - HALF_PI; // wasm uses sqrtf
	}
	else
	{
		if( a >= 1.0f )
		{
			return HALF_PI;
		}
		return HALF_PI - ( ( ( -0.0187293f * a + 0.0742610f ) * a - 0.2121144f ) * a + 1.5707288f ) * idMath::Sqrt( 1.0f - a ); // wasm uses sqrtf
	}
}

/*
========================
idMath::ACos
========================
*/
ID_INLINE float idMath::ACos( float a )
{
	if( a <= -1.0f )
	{
		return PI;
	}
	if( a >= 1.0f )
	{
		return 0.0f;
	}
	return acosf( a );
}

/*
========================
idMath::ACos16
========================
*/
ID_INLINE float idMath::ACos16( float a )
{
	if( a < 0.0f )
	{
		if( a <= -1.0f )
		{
			return PI;
		}
		a = fabsf( a );
		return PI - ( ( ( -0.0187293f * a + 0.0742610f ) * a - 0.2121144f ) * a + 1.5707288f ) * idMath::Sqrt( 1.0f - a ); // wasm sqrtf
	}
	else
	{
		if( a >= 1.0f )
		{
			return 0.0f;
		}
		return ( ( ( -0.0187293f * a + 0.0742610f ) * a - 0.2121144f ) * a + 1.5707288f ) * idMath::Sqrt( 1.0f - a ); // wasm sqr
	}
}

/*
========================
idMath::ATan
========================
*/
ID_INLINE float idMath::ATan( float a )
{
	// assert( fabs( y ) > idMath::FLT_SMALLEST_NON_DENORMAL || fabs( x ) > idMath::FLT_SMALLEST_NON_DENORMAL );
	// I think this is in OG BFG, but as of right now I havent diffed that, this line uncommented appears to be there and commented out in SnowFall
	return atanf( a );
}

/*
========================
idMath::ATan16
========================
*/
ID_INLINE float idMath::ATan16( float a )
{
	float s;
	if( fabsf( a ) > 1.0f )
	{
		a = 1.0f / a;
		s = a * a;
		s = - ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
						  * s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
		if( a < 0.0f )
		{
			return s - HALF_PI;
		}
		else
		{
			return s + HALF_PI;
		}
	}
	else
	{
		s = a * a;
		return ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
						   * s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
	}
}

/*
========================
idMath::ATan
========================
*/
ID_INLINE float idMath::ATan( float y, float x )
{
	// SRS - Don't need this assertion since atan2f(y,x) handles x=0, y=0 and x=0, y>0 or y<0 cases properly
	// SRS - This assertion can cause game to stop prematurely when _DEBUG is defined and asserts are enabled
	//assert( fabs( y ) > idMath::FLT_SMALLEST_NON_DENORMAL || fabs( x ) > idMath::FLT_SMALLEST_NON_DENORMAL );
	return atan2f( y, x );
}

/*
========================
idMath::ATan16
========================
*/
ID_INLINE float idMath::ATan16( float y, float x )
{
	assert( fabs( y ) > idMath::FLT_SMALLEST_NON_DENORMAL || fabs( x ) > idMath::FLT_SMALLEST_NON_DENORMAL );

	float a, s;
	if( fabsf( y ) > fabsf( x ) )
	{
		a = x / y;
		s = a * a;
		s = - ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
						  * s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
		if( a < 0.0f )
		{
			return s - HALF_PI;
		}
		else
		{
			return s + HALF_PI;
		}
	}
	else
	{
		a = y / x;
		s = a * a;
		return ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
						   * s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
	}
}

/*
========================
idMath::Pow
========================
*/
ID_INLINE float idMath::Pow( float x, float y )
{
	return powf( x, y );
}

/*
========================
idMath::Pow16
========================
*/
ID_INLINE float idMath::Pow16( float x, float y )
{
	return Exp16( y * Log16( x ) );
}

/*
========================
idMath::Exp
========================
*/
ID_INLINE float idMath::Exp( float f )
{
	return expf( f );
}

/*
========================
idMath::Exp16
========================
*/
ID_INLINE float idMath::Exp16( float f )
{
	float x = f * 1.44269504088896340f;		// multiply with ( 1 / log( 2 ) )
#if 1
	int i = *reinterpret_cast<int*>( &x );
	int s = ( i >> IEEE_FLT_SIGN_BIT );
	int e = ( ( i >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
	int m = ( i & ( ( 1 << IEEE_FLT_MANTISSA_BITS ) - 1 ) ) | ( 1 << IEEE_FLT_MANTISSA_BITS );
	i = ( ( m >> ( IEEE_FLT_MANTISSA_BITS - e ) ) & ~( e >> INT32_SIGN_BIT ) ) ^ s;
#else
	int i = ( int ) x;
	if( x < 0.0f )
	{
		i--;
	}
#endif
	int exponent = ( i + IEEE_FLT_EXPONENT_BIAS ) << IEEE_FLT_MANTISSA_BITS;
	float y = *reinterpret_cast<float*>( &exponent );
	x -= ( float ) i;
	if( x >= 0.5f )
	{
		x -= 0.5f;
		y *= 1.4142135623730950488f;	// multiply with sqrt( 2 )
	}
	float x2 = x * x;
	float p = x * ( 7.2152891511493f + x2 * 0.0576900723731f );
	float q = 20.8189237930062f + x2;
	x = y * ( q + p ) / ( q - p );
	return x;
}

/*
========================
idMath::Log
========================
*/
ID_INLINE float idMath::Log( float f )
{
	return logf( f );
}

/*
========================
idMath::Log16
========================
*/
ID_INLINE float idMath::Log16( float f )
{
	int i = *reinterpret_cast<int*>( &f );
	int exponent = ( ( i >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
	i -= ( exponent + 1 ) << IEEE_FLT_MANTISSA_BITS;	// get value in the range [.5, 1>
	float y = *reinterpret_cast<float*>( &i );
	y *= 1.4142135623730950488f;						// multiply with sqrt( 2 )
	y = ( y - 1.0f ) / ( y + 1.0f );
	float y2 = y * y;
	y = y * ( 2.000000000046727f + y2 * ( 0.666666635059382f + y2 * ( 0.4000059794795f + y2 * ( 0.28525381498f + y2 * 0.2376245609f ) ) ) );
	y += 0.693147180559945f * ( ( float )exponent + 0.5f );
	return y;
}

/*
========================
idMath::IPow
========================
*/
ID_INLINE int idMath::IPow( int x, int y )
{
#if 0 // fix
	assert( y >= 0 );

	int result = 1;
	for( ; y > 0; y-- )
	{
		result *= x;
	}
	return result;
#endif
	int r;
	for( r = x; y > 1; y-- )
	{
		r *= x;
	}
	return r;
}

#if 0
ID_INLINE int idMath::ILog2( float f )
{
	return ( ( ( *reinterpret_cast<int*>( &f ) ) >> IEEE_FLT_MANTISSA_BITS )
			 & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
}

ID_INLINE int idMath::ILog2( int i )
{
	return ILog2( ( float )i );
}
ID_INLINE int idMath::ILog2( int i )
{
	assert( i > 0 );
	return std::bit_width( static_cast<unsigned int>( i ) ) - 1;
}
#endif

/*
========================
idMath::ILog2
========================
*/
ID_INLINE int idMath::ILog2( float f )
{
	return ( ( ( *reinterpret_cast<int*>( &f ) ) >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
}

/*
========================
idMath::ILog2
========================
*/
ID_INLINE int idMath::ILog2( int i )
{
	return ILog2( ( float )i );
}

/*
========================
idMath::BitsForFloat
========================
*/
ID_INLINE int idMath::BitsForFloat( float f )
{
	return ILog2( f ) + 1;
}

/*
========================
idMath::BitsForInteger
========================
*/
ID_INLINE int idMath::BitsForInteger( int i )
{
	return ILog2( ( float )i ) + 1;
}

/*
========================
idMath::MaskForFloatSign
========================
*/
ID_INLINE int idMath::MaskForFloatSign( float f )
{
	return ( ( *reinterpret_cast<int*>( &f ) ) >> IEEE_FLT_SIGN_BIT );
}

/*
========================
idMath::MaskForIntegerSign
========================
*/
ID_INLINE int idMath::MaskForIntegerSign( int i )
{
	return ( i >> INT32_SIGN_BIT );
}

/*
========================
idMath::FloorPowerOfTwo
========================
*/
ID_INLINE int idMath::FloorPowerOfTwo( int x )
{
	// consider using this!!!
	//  return x > 0 ? static_cast<int>( std::bit_floor( static_cast<unsigned int>( x ) ) ) : 0;

	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x >> 1;
}

/*
========================
idMath::CeilPowerOfTwo
========================
*/
ID_INLINE int idMath::CeilPowerOfTwo( int x )
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

/*
========================
idMath::IsPowerOfTwo
========================
*/
ID_INLINE bool idMath::IsPowerOfTwo( int x )
{
	return ( x & ( x - 1 ) ) == 0 && x > 0;
}

/*
========================
idMath::BitCount
========================
*/
ID_INLINE int idMath::BitCount( int x )
{
	x -= ( ( x >> 1 ) & 0x55555555 );
	x = ( ( ( x >> 2 ) & 0x33333333 ) + ( x & 0x33333333 ) );
	x = ( ( ( x >> 4 ) + x ) & 0x0f0f0f0f );
	x += ( x >> 8 );
	return ( ( x + ( x >> 16 ) ) & 0x0000003f );
}

/*
========================
idMath::BitReverse
========================
*/
ID_INLINE int idMath::BitReverse( int x )
{
#if 0
	uint32_t x = static_cast<uint32_t>( value );

	x = ( ( x >> 1 ) & 0x55555555u ) | ( ( x & 0x55555555u ) << 1 );
	x = ( ( x >> 2 ) & 0x33333333u ) | ( ( x & 0x33333333u ) << 2 );
	x = ( ( x >> 4 ) & 0x0f0f0f0fu ) | ( ( x & 0x0f0f0f0fu ) << 4 );
	x = ( ( x >> 8 ) & 0x00ff00ffu ) | ( ( x & 0x00ff00ffu ) << 8 );

	x = ( x >> 16 ) | ( x << 16 );
	return std::bit_cast<int32_t>( x );
#endif
	x = ( ( ( x >> 1 ) & 0x55555555 ) | ( ( x & 0x55555555 ) << 1 ) );
	x = ( ( ( x >> 2 ) & 0x33333333 ) | ( ( x & 0x33333333 ) << 2 ) );
	x = ( ( ( x >> 4 ) & 0x0f0f0f0f ) | ( ( x & 0x0f0f0f0f ) << 4 ) );
	x = ( ( ( x >> 8 ) & 0x00ff00ff ) | ( ( x & 0x00ff00ff ) << 8 ) );
	return ( ( x >> 16 ) | ( x << 16 ) );
}

/*
========================
idMath::Abs
========================
*/
ID_INLINE int idMath::Abs( int x )
{
#if 1
	return abs( x );
#else
	int y = x >> INT32_SIGN_BIT;
	return ( ( x ^ y ) - y );
#endif
}

/*
========================
idMath::Fabs
========================
*/
ID_INLINE float idMath::Fabs( float f )
{
#if 1
	return fabsf( f );
#else
	int tmp = *reinterpret_cast<int*>( &f );
	tmp &= 0x7FFFFFFF;
	return *reinterpret_cast<float*>( &tmp );
#endif
}

/*
========================
idMath::Floor
========================
*/
ID_INLINE float idMath::Floor( float f )
{
	return floorf( f );
}

/*
========================
idMath::Ceil
========================
*/
ID_INLINE float idMath::Ceil( float f )
{
	return ceilf( f );
}

/*
========================
idMath::Rint
========================
*/
ID_INLINE float idMath::Rint( float f )
{
#if 0
	if( fabsf( f ) >= 8388608.0f )
	{
		return f;
	}

	return floorf( f + 0.5f );
#endif
	return floorf( f + 0.5f );
}


/*
========================
idMath::Ftoi
========================
*/
ID_INLINE int idMath::Ftoi( float f )
{
	// If a converted result is larger than the maximum signed doubleword integer,
	// the floating-point invalid exception is raised, and if this exception is masked,
	// the indefinite integer value (80000000H) is returned.
#if defined(USE_INTRINSICS_SSE)
	__m128 x = _mm_load_ss( &f );
	return _mm_cvttss_si32( x );
#elif 0 // round chop (C/C++ standard)
	int i, s, e, m, shift;
	i = *reinterpret_cast<int*>( &f );
	s = i >> IEEE_FLT_SIGN_BIT;
	e = ( ( i >> IEEE_FLT_MANTISSA_BITS ) & ( ( 1 << IEEE_FLT_EXPONENT_BITS ) - 1 ) ) - IEEE_FLT_EXPONENT_BIAS;
	m = ( i & ( ( 1 << IEEE_FLT_MANTISSA_BITS ) - 1 ) ) | ( 1 << IEEE_FLT_MANTISSA_BITS );
	shift = e - IEEE_FLT_MANTISSA_BITS;
	return ( ( ( ( m >> -shift ) | ( m << shift ) ) & ~( e >> INT32_SIGN_BIT ) ) ^ s ) - s;
#else
	// If a converted result is larger than the maximum signed doubleword integer the result is undefined.
	return C_FLOAT_TO_INT( f );
#endif
}

// BOOM 3 Documentation
// https://github.com/libretro/boom3/commit/e87ec65e93dcb4d486417a195e622d638ec39077
/*
	NOTE: despite the name and the comment in the MSVC branch below, this is
	NOT round-to-nearest anywhere except 32-bit MSVC. Every other target falls
	through to the plain (int) cast at the bottom, which truncates toward zero.

	That difference is visible in ANGLE2SHORT/ANGLE2BYTE above, so network
	angle quantisation here does not match the original 32-bit Windows build.
	It is however consistent across all targets we ship, so it is not a
	determinism hazard, and changing it now would alter behaviour relative to
	builds already in the wild. Left as-is deliberately.
*/

#if 0
ID_INLINE int idMath::FtoiFast( float f )
{
//	from revelator clean src latest. WRONG but keep here for note when we bring back!
	// If a converted result is larger than the maximum signed doubleword integer,
	// the floating-point invalid exception is raised, and if this exception is masked,
	// the indefinite integer value (80000000H) is returned.
	__m128 x = _mm_load_ss( &f );
	return _mm_cvttss_si32( x );
}
#endif

#if 0 // revelation changes to those from OG D3


ID_INLINE unsigned long idMath::Ftol( float f )
{
	__m128 x = _mm_load_ss( &f );
	return static_cast<unsigned long>( _mm_cvttss_si32( x ) );
	//return static_cast<unsigned long>(_mm_cvttss_si32(x));
}

ID_INLINE unsigned long idMath::FtolFast( float f )
{
	return Ftol( f );

#endif

	/*
	========================
	idMath::Ftoi8
	========================
	*/
	ID_INLINE char idMath::Ftoi8( float f )
	{
#if defined(USE_INTRINSICS_SSE)
		__m128 x = _mm_load_ss( &f );
		x = _mm_max_ss( x, SIMD_SP_min_char );
		x = _mm_min_ss( x, SIMD_SP_max_char );
		return static_cast<char>( _mm_cvttss_si32( x ) );
#else
		// The converted result is clamped to the range [-128,127].
		int i = C_FLOAT_TO_INT( f );
		if( i < -128 )
		{
			return -128;
		}
		else if( i > 127 )
		{
			return 127;
		}
		return static_cast<char>( i );
#endif
	}

	/*
	========================
	idMath::Ftoi16
	========================
	*/
	ID_INLINE short idMath::Ftoi16( float f )
	{
#if defined(USE_INTRINSICS_SSE)
		__m128 x = _mm_load_ss( &f );
		x = _mm_max_ss( x, SIMD_SP_min_short );
		x = _mm_min_ss( x, SIMD_SP_max_short );
		return static_cast<short>( _mm_cvttss_si32( x ) );
#else
		// The converted result is clamped to the range [-32768,32767].
		int i = C_FLOAT_TO_INT( f );
		if( i < -32768 )
		{
			return -32768;
		}
		else if( i > 32767 )
		{
			return 32767;
		}
		return static_cast<short>( i );
#endif
	}

	/*
	========================
	idMath::Ftoui16
	========================
	*/
	ID_INLINE unsigned short idMath::Ftoui16( float f )
	{
		// TO DO - SSE ??

		// The converted result is clamped to the range [-32768,32767].
		int i = C_FLOAT_TO_INT( f );
		if( i < 0 )
		{
			return 0;
		}
		else if( i > 65535 )
		{
			return 65535;
		}
		return static_cast<unsigned short>( i );
	}

	/*
	========================
	idMath::Ftob
	========================
	*/
	ID_INLINE byte idMath::Ftob( float f )
	{
		// If a converted result is negative the value (0) is returned and if the
		// converted result is larger than the maximum byte the value (255) is returned.

#if defined(USE_INTRINSICS_SSE)
		__m128 x = _mm_load_ss( &f );
		x = _mm_max_ss( x, SIMD_SP_zero );
		x = _mm_min_ss( x, SIMD_SP_255 );
		return static_cast<byte>( _mm_cvttss_si32( x ) );
#else
		// The converted result is clamped to the range [0,255].
		int i = C_FLOAT_TO_INT( f );
		if( i < 0 )
		{
			return 0;
		}
		else if( i > 255 )
		{
			return 255;
		}
		return static_cast<byte>( i );
#endif
	}

	/*
	========================
	idMath::ClampChar
	========================
	*/
	ID_INLINE signed char idMath::ClampChar( int i )
	{
		if( i < -128 )
		{
			return -128;
		}
		if( i > 127 )
		{
			return 127;
		}
		return static_cast<signed char>( i );
	}

	/*
	========================
	idMath::ClampShort
	========================
	*/
	ID_INLINE signed short idMath::ClampShort( int i )
	{
		if( i < -32768 )
		{
			return -32768;
		}
		if( i > 32767 )
		{
			return 32767;
		}
		return static_cast<signed short>( i );
	}

	/*
	========================
	idMath::ClampInt
	========================
	*/
	ID_INLINE int idMath::ClampInt( int min, int max, int value )
	{
		if( value < min )
		{
			return min;
		}
		if( value > max )
		{
			return max;
		}
		return value;
	}

	/*
	========================
	idMath::ClampFloat
	========================
	*/
	ID_INLINE float idMath::ClampFloat( float min, float max, float value )
	{
		return Max( min, Min( max, value ) );
	}

// method from RevelatioN!
	ID_INLINE byte idMath::ClampByte( byte min, byte max, int value )
	{
		if( value < min )
		{
			return min;
		}
		if( value > max )
		{
			return max;
		}
		return value;
	}

	/*
	========================
	idMath::AngleNormalize360

	/// -n- On ajuste l'angle pour pour qu'il soit positif et ne fasse pas plus d'un tour de cercle.
	//	-n- We adjust the angle to be positive and no more than 360 degrees.
	========================
	*/
	ID_INLINE float idMath::AngleNormalize360( float angle )
	{
		if( ( angle >= 360.0f ) || ( angle < 0.0f ) )
		{
			angle -= floorf( angle * ( 1.0f / 360.0f ) ) * 360.0f;
		}
		return angle;
	}

	/*
	========================
	idMath::AngleNormalize180

	/// -n- Retourne soit une valeur entre 0 et 180, soit une valeur entre -179 et -1 (si l'angle dépasse 180).
	//	-n- Returns a value that is either between 0 and 180, or between -179 and -1 (if the angle exceeds 180)
	========================
	*/
	ID_INLINE float idMath::AngleNormalize180( float angle )
	{
		angle = AngleNormalize360( angle );
		if( angle > 180.0f )
		{
			angle -= 360.0f;
		}
		return angle;
	}

	/*
	========================
	idMath::AngleDelta
	========================
	*/
	ID_INLINE float idMath::AngleDelta( float angle1, float angle2 )
	{
		return AngleNormalize180( angle1 - angle2 );
	}

	/*
	========================
	idMath::FloatHash
	========================
	*/
	ID_INLINE int idMath::FloatHash( const float * array, const int numFloats )
	{
		int i, hash = 0;
		const int* ptr;

		ptr = reinterpret_cast<const int*>( array );
		for( i = 0; i < numFloats; i++ )
		{
			hash ^= ptr[i];
		}
		return hash;

#if 0 // djoom3
		public static int FloatHash( final float[] array, final int numFloats )
		{
			int i, hash = 0;
//	const int *ptr;

//	ptr = reinterpret_cast<const int *>( array );
			for( i = 0; i < numFloats; i++ )
			{
//		hash ^= ptr[i];
				hash ^= Float.floatToIntBits( array[i] );
			}
			return hash;
		}
#endif
#if 0 // proposal 
		uint32_t hash = 0;

		for( int i = 0; i < numFloats; i++ )
		{
			hash ^= std::bit_cast<uint32_t>( array[i] );
		}

		return std::bit_cast<int32_t>( hash );
#endif
	}

// kotlin overload for the above. note that kotlin canonicalizes nans with toBits !!!
//
//fun FloatHash(x: Float, y : Float, z : Float) : Int{
//	return x.toBits() xor y.toBits() xor z.toBits()
//}


	template< typename T >
	ID_INLINE
	T Lerp( const T from, const T to, float f )
	{
		return from + ( ( to - from ) * f );
	}

	//	This should be a template ? ? ?
	template<>
	ID_INLINE
	int Lerp( const int from, const int to, float f )
	{
		return idMath::Ftoi( ( float ) from + ( ( ( float ) to - ( float ) from ) * f ) );
	}


	/*
	========================
	LerpToWithScale

	Lerps from "cur" to "dest", scaling the delta to change by "scale"
	If the delta between "cur" and "dest" is very small, dest is returned to prevent denormals.
	========================
	*/
	inline float idMath::LerpToWithScale( const float cur, const float dest, const float scale )
	{
		float delta = dest - cur;
		if( delta > -1.0e-6f && delta < 1.0e-6f )
		{
			return dest;
		}
		return cur + ( dest - cur ) * scale;
	}

// TwelveEyes -->
	ID_INLINE int idMath::gcd( int a, int b )
	{
		int R;
		while( ( a % b ) > 0 )
		{
			R = a % b;
			a = b;
			b = R;
		}
		return b;
	}
// <---

// BieldBdeLuna
	/*
	================
	blMathUtils::MinNormalizeMax
	================
	*     //////////////////////////////////////////////////////////////////
	*    //      return a normalized value between a min and a max       //
	*   //////////////////////////////////////////////////////////////////
	*/
	ID_INLINE float idMath::MinNormalizeMax( float number, float max, float min )
	{
		float interval, incorporated;

		interval = max - min;
		incorporated = number - min;

		if( incorporated <= 0.0f )
		{
			return 0.0f;
		}
		else
		{
			return ( ( 1 / interval ) * incorporated );
		}

		// xyzz: same mathematical operation is in Interpolate.h ???

		// xyzz: inverselerp ??

		// xyzz
		// consider reimplementation as something NormalizeRange ? better name
		// example: but must match biel's intentions
		/*
			if( max <= min ) {
		    return 0.0f;
		}

		return ClampFloat( 0.0f, 1.0f, ( value - min ) / ( max - min ) );
		*/
	}

#endif /* !__MATH_MATH_H__ */
