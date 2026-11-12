#if defined( DOMINION ) // Enviro suit - match the existing vertex shader's TEXCOORD0
// Reuse the original effect code and correct only its mismatched input semantic.
#define TEXCOORD0_centroid TEXCOORD0
#include "enviroSuit.ps.hlsl"
#endif
