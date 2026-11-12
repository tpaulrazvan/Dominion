// OLD GUIs - Menu model lighting; compiled/registered for DOMINION.
#if defined( DOMINION ) // OLD GUIs - Menu model lighting
/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2024 Robert Beckebans

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

#include "global_inc.hlsl"
#include "renderParmSet10.inc.hlsl"



// *INDENT-OFF*
Texture2D				t_Normal			: register( t0 VK_DESCRIPTOR_SET( 1 ) );
Texture2D				t_Specular			: register( t1 VK_DESCRIPTOR_SET( 1 ) );
Texture2D				t_BaseColor			: register( t2 VK_DESCRIPTOR_SET( 1 ) );

Texture2D				t_LightFalloff		: register( t3 VK_DESCRIPTOR_SET( 2 ) );
Texture2D				t_LightProjection	: register( t4 VK_DESCRIPTOR_SET( 2 ) );

SamplerState			s_Material : register( s0 VK_DESCRIPTOR_SET( 3 ) ); // for the normal/specular/basecolor
SamplerState 			s_Lighting : register( s1 VK_DESCRIPTOR_SET( 3 ) ); // for sampling the jitter

SamplerState s_Diffuse : register( s4 VK_DESCRIPTOR_SET( 3 ) ); // Preserve explicit nearest on diffuse images.

struct PS_IN
{
	float4 position		: SV_Position;
	float4 texcoord0	: TEXCOORD0_centroid;
	float4 texcoord1	: TEXCOORD1_centroid;
	float4 texcoord2	: TEXCOORD2_centroid;
	float4 texcoord3	: TEXCOORD3_centroid;
	float4 texcoord4	: TEXCOORD4_centroid;
	float4 texcoord5	: TEXCOORD5_centroid;
	float4 texcoord6	: TEXCOORD6_centroid;
	float4 color		: COLOR0;
};

struct PS_OUT
{
	float4 color : SV_Target0;
};
// *INDENT-ON*

// Original Doom 3 ARB2 interaction: display-space Lambert lighting and its
// specular-table approximation. Decode Dominion's YCoCg texture storage only;
// do not apply the gameplay HDR/sRGB/GGX lighting conversions.
void main( PS_IN fragment, out PS_OUT result )
{
	float4 bumpMap = t_Normal.Sample( s_Material, fragment.texcoord1.xy );
	float3 normal;
	normal.xy = bumpMap.wy - 0.5;
	normal.z = sqrt( abs( dot( normal.xy, normal.xy ) - 0.25 ) );
	normal = normalize( normal );
	float3 light = normalize( fragment.texcoord0.xyz );
	float3 view = normalize( fragment.texcoord6.xyz );
	float nDotL = saturate( dot( normal, light ) );
	float nDotH = saturate( dot( normal, normalize( light + view ) ) );
	// Matches Doom 3's _specularTable generation in Image_init.cpp.
	float specular = saturate( ( nDotH - 0.75 ) * 4.0 );
	specular *= specular;
	float3 diffuse = ConvertYCoCgToRGB( t_BaseColor.Sample( s_Diffuse, fragment.texcoord4.xy ) );
	float3 gloss = t_Specular.Sample( s_Material, fragment.texcoord5.xy ).rgb;
	float3 projection = idtex2Dproj( s_Lighting, t_LightProjection, fragment.texcoord3 ).rgb;
	float3 falloff = idtex2Dproj( s_Lighting, t_LightFalloff, fragment.texcoord2 ).rgb;
	float3 surface = diffuse * pc.rpDiffuseModifier.rgb + gloss * ( 2.0 * specular ) * pc.rpSpecularModifier.rgb;
	result.color = float4( surface * nDotL * projection * falloff * fragment.color.rgb, 1.0 );
}
#endif
