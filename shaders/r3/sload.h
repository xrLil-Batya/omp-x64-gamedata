#ifndef SLOAD_H
#define SLOAD_H

#include "common.h"

#ifdef	MSAA_ALPHATEST_DX10_1
#if MSAA_SAMPLES == 2
static const float2 MSAAOffsets[2] = { float2(4,4), float2(-4,-4) };
#endif
#if MSAA_SAMPLES == 4
static const float2 MSAAOffsets[4] = { float2(-2,-6), float2(6,-2), float2(-6,2), float2(2,6) };
#endif
#if MSAA_SAMPLES == 8
static const float2 MSAAOffsets[8] = { float2(1,-3), float2(-1,3), float2(5,1), float2(-3,-5), 
								               float2(-5,5), float2(-7,-1), float2(3,7), float2(7,-7) };
#endif
#endif	//	MSAA_ALPHATEST_DX10_1

//////////////////////////////////////////////////////////////////////////////////////////
// Bumped surface loader                //
//////////////////////////////////////////////////////////////////////////////////////////
struct	surface_bumped
{
	float4	base;
	float3	normal;
	float	gloss;
	float	height;

};
float3 derive_Z(float2 nrm)
{
	float3 normal = (0.0, 0.0, 0.0); //Initialize, empty normal crashes on some hardware
	normal.xy = nrm.xy * 2.0 - 1.0; //Bring it to -1-1 range
	normal.z = sqrt(1.0 - dot(normal.xy, normal.xy)); //Z sign reconstruction
	return normal;
}
float4 tbase( float2 tc )
{
   return	s_base.Sample( smp_base, tc);
}

#if defined(ALLOW_STEEPPARALLAX) && defined(USE_STEEPPARALLAX)

//Always remember to check defines, variables, and shit.... god
#define PARALLAX_NEAR_PLANE 0.01
#define PARALLAX_FAR_PLANE 35
#define PARALLAX_DEPTH 0.045


//Ok, we can comment old gsc parallax now.
//We need to change input, to p_bumped struct like in GSC shader
//We also change name of this function to GSC 
//Time to change input to stuff from p_bumped.
void UpdateTC( inout p_bumped I)
{
	//Here's "limited" range of parallax. We use linear depth (z vector of view space position) to do that
	if ((I.position.z > PARALLAX_NEAR_PLANE) && (I.position.z < PARALLAX_FAR_PLANE))
	{
		//That M1/M2/M3 stuff is our TBN matrix (we aligin tangent normals/vectors to to just geometry normals
		float3 eye = normalize(mul(float3x3(I.M1.x, I.M2.x, I.M3.x,
			I.M1.y, I.M2.y, I.M3.y,
			I.M1.z, I.M2.z, I.M3.z), -I.position.xyz));

		// steps minmax and refines minmax
		int4 steps = int4(3, 10, 7, 16); // 3..10, 7..16

		bool need_disp_lerp = true;
		bool need_refine = true; //Thats refinement steps, used to smoothout raymarched results

		float view_angle = abs(dot(float3(0.0, 0.0, 1.0), eye));

		float layer_step = rcp(lerp(steps.y, steps.x, view_angle));

		float2 tc_step = layer_step * eye.xy * PARALLAX_DEPTH;

		//Now, we have to change this huita. p.tcdbump is our "tiled" texture coordinate
		//I.tcdh is our "normal" texcoord, lets see above
		float2 displaced_tc = I.tcdh;

		float curr_disp, curr_layer = 0.0;

		do
		{
			displaced_tc -= tc_step;
			curr_disp = 1 - s_bumpX.SampleLevel(smp_base, displaced_tc, 0).w; //Our heightmap sampler 
			curr_layer += layer_step;
		} while (curr_layer < curr_disp);

		if (need_refine)
		{
			displaced_tc += tc_step;
			curr_layer -= layer_step;

			float refine_steps = lerp(steps.w, steps.z, view_angle);

			tc_step /= refine_steps;
			layer_step /= refine_steps;

			do
			{
				displaced_tc -= tc_step;
				curr_disp = 1.0 - s_bumpX.SampleLevel(smp_base, displaced_tc, 0).w;
				curr_layer += layer_step;
			} while (curr_layer < curr_disp);
		}

		if (need_disp_lerp)
		{
			float2 displaced_tc_prev = displaced_tc + tc_step;

			float after_depth = curr_disp - curr_layer;
			float before_depth = 1.0 - s_bumpX.SampleLevel(smp_base, displaced_tc_prev, 0).w - curr_layer + layer_step; //Another sampler name

			float weight = after_depth / (after_depth - before_depth);

			displaced_tc = lerp(displaced_tc, displaced_tc_prev, weight);
		}
		//Same as previous texcoord, but we will add tiling here
		I.tcdh = displaced_tc;
		
		//Tiling for detail/tiled textures
	#if defined(USE_TDETAIL) && defined(USE_STEEPPARALLAX)
		I.tcdbump = displaced_tc * dt_params; //Now its correct
	#endif		
	}
}

#elif	defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)

void UpdateTC( inout p_bumped I)
{
	float3	 eye = mul (float3x3(I.M1.x, I.M2.x, I.M3.x,
								 I.M1.y, I.M2.y, I.M3.y,
								 I.M1.z, I.M2.z, I.M3.z), -I.position.xyz);
								 
	float	height	= s_bumpX.Sample( smp_base, I.tcdh).w;	//
			//height  /= 2;
			//height  *= 0.8;
			height	= height*(parallax.x) + (parallax.y);	//
	float2	new_tc  = I.tcdh + height * normalize(eye);	//

	//	Output the result
	I.tcdh.xy	= new_tc;
}

#else	//	USE_PARALLAX

void UpdateTC( inout p_bumped I)
{
	;
}

#endif	//	USE_PARALLAX

surface_bumped sload_i( p_bumped I)
{
	surface_bumped	S;
   
	UpdateTC(I);	//	All kinds of parallax are applied here.

	float4 	Nu	= s_bump.Sample( smp_base, I.tcdh );		// IN:	normal.gloss
	float4 	NuE	= s_bumpX.Sample( smp_base, I.tcdh);	// IN:	normal_error.height

	S.base		= tbase(I.tcdh);				//	IN:  rgb.a
	S.normal	= derive_Z(Nu.wz);	//	(Nu.wzyx - .5h) + (E-.5)
	S.gloss		= Nu.x;					//	S.gloss = Nu.x*Nu.x;
	S.height	= NuE.w;
	//S.height	= 0;

	return S;
}

surface_bumped sload_i( p_bumped I, float2 pixeloffset )
{
	surface_bumped	S;
   
   // apply offset
#ifdef	MSAA_ALPHATEST_DX10_1
   I.tcdh.xy += pixeloffset.x * ddx(I.tcdh.xy) + pixeloffset.y * ddy(I.tcdh.xy);
#endif

	UpdateTC(I);	//	All kinds of parallax are applied here.

	float4 	Nu	= s_bump.Sample( smp_base, I.tcdh );		// IN:	normal.gloss
	float4 	NuE	= s_bumpX.Sample( smp_base, I.tcdh);	// IN:	normal_error.height

	S.base		= tbase(I.tcdh);				//	IN:  rgb.a
	S.normal	= derive_Z(Nu.wz);	//	(Nu.wzyx - .5h) + (E-.5)
	S.gloss		= Nu.x;					//	S.gloss = Nu.x*Nu.x;
	S.height	= NuE.w;
	//S.height	= 0;

	return S;
}

surface_bumped sload ( p_bumped I)
{
      surface_bumped      S   = sload_i	(I);
	//	S.normal.z			*=	0.5;		//. make bump twice as contrast (fake, remove me if possible)
      return              S;
}

surface_bumped sload ( p_bumped I, float2 pixeloffset )
{
      surface_bumped      S   = sload_i	(I, pixeloffset );
	//	S.normal.z			*=	0.5;		//. make bump twice as contrast (fake, remove me if possible)
      return              S;
}

#endif
