/*
	All credits goes to Holger Gruen (AMD)
	
	He's author of that CHS implementation
*/

#define FILTER_SIZE 11
#define FS FILTER_SIZE
#define FS2 ( FILTER_SIZE / 2 )

#define BLOCKER_FILTER_SIZE 11
#define BFS BLOCKER_FILTER_SIZE
#define BFS2 ( BLOCKER_FILTER_SIZE / 2 )
#define SUN_WIDTH 350.0

// Four control matrices for a dynamic cubic bezier filter
// weights matrix .
static const float C3 [11][11] =
{ { 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
{ 1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 },
};
static const float C2 [11][11] =
{ { 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 },
{ 0.0 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
};
static const float C1 [11][11] =
{ { 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,1.0 ,1.0 ,1.0 ,1.0 ,1.0 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.2 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
};

static const float C0 [11][11] =
{ { 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.8 ,0.8 ,0.8 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.8 ,1.0 ,0.8 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.8 ,0.8 ,0.8 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
{ 0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 ,0.0 },
};
// Compute dynamic weight at a certain row , column of the matrix .
float Fw ( int r , int c , float fL )
{
	return (1.0 - fL )*(1.0 - fL )*(1.0 - fL ) * C0 [r ][ c] +
	fL * fL * fL * C3 [r ][ c] +
	3.0 * (1.0 - fL )*(1.0 - fL )* fL * C1 [r ][ c ]+
	3.0 * fL * fL *(1.0 - fL ) * C2 [r ][ c ];
}

// ==============================================================
// This shader computes the contact hardening shadow filter .
// ==============================================================
float shadowchs ( float3 tc )
{
	float s = 0.0;
	float2 stc = (SMAP_size * tc.xy) + float2(0.5, 0.5);
	float2 tcs = floor ( stc );
	float2 fc;
	int row;
	int col;
	float w = 0.0;
	float avgBlockerDepth = 0.0;
	float blockerCount = 0.0;
	float fRatio ;
	float4 v1 [FS2 + 1];
	float2 v0 [FS2 + 1];
	float2 off;
	fc = stc - tcs;
	tc.xy = tc - (fc * (1.0/SMAP_size));
	
	// Find number of blockers and sum up blocker depth .
	for ( row = -BFS2; row <= BFS2; row += 2)
	{
		for ( col = -BFS2; col <= BFS2; col += 2)
		{
			float4 d4 = s_smap.GatherRed(smp_nofilter, tc.xy, int2(col, row));
			float4 b4 = (tc.zzzz <= d4) ? (0.0).xxxx : (1.0).xxxx;
			blockerCount += dot(b4, (1.0).xxxx);
			avgBlockerDepth += dot(d4, b4);
		}
	}
	// Compute ratio using formulas from PCSS .
	if(blockerCount > 0.0)
	{
		avgBlockerDepth /= blockerCount;
		fRatio = saturate(((tc.z - avgBlockerDepth) * SUN_WIDTH) / avgBlockerDepth);
		fRatio *= fRatio ;
	}
	else
	{
		fRatio = 0.0;
	}
	
	// Sum up weights of dynamic filter matrix .
	for(row = 0; row < FS ; ++ row)
	{
		for(col = 0; col < FS; ++col)
		{
			w += Fw(row, col, fRatio);
		}
	}
	
	// Filter shadow map samples using the dynamic weights .
	[unroll(FILTER_SIZE)] for(row = -FS2; row <= FS2; row += 2)
	{
		for(col = -FS2; col <= FS2; col += 2)
		{
			v1 [( col + FS2 )/2] = s_smap.GatherCmpRed(smp_smap, tc.xy, tc.z, int2(col, row));
			
			if( col == - FS2 )
			{
				s += ( 1 - fc.y ) * ( v1 [0].w *
					( Fw ( row + FS2 ,0 , fRatio ) -
					Fw ( row + FS2 ,0 , fRatio ) * fc.x ) + v1 [0].z *
					( fc.x * ( Fw ( row + FS2 ,0 , fRatio ) -
					Fw ( row + FS2 ,1 , fRatio ) ) +
					Fw ( row + FS2 ,1 , fRatio ) ) );
					
				s += ( fc.y ) * ( v1 [0].x * (
					Fw ( row + FS2 ,0 , fRatio ) -
					Fw ( row + FS2 ,0 , fRatio ) * fc.x ) +
					v1 [0].y * ( fc.x *( Fw ( row + FS2 ,0 , fRatio ) -
					Fw ( row + FS2 ,1 , fRatio ) ) +
					Fw ( row + FS2 ,1 , fRatio ) ) );
				
				if( row > - FS2 )
				{
					s += ( 1 - fc.y ) * ( v0 [0].x *
						( Fw ( row + FS2 -1 ,0 , fRatio ) -
						Fw ( row + FS2 -1 ,0 , fRatio ) * fc.x ) +
						v0 [0].y *
						( fc .x * ( Fw ( row + FS2 -1 ,0 , fRatio ) -
						Fw ( row + FS2 -1 ,1 , fRatio ) ) +
						Fw ( row + FS2 -1 ,1 , fRatio ) ) );
						
					s += ( fc . y ) * ( v1 [0].w *
						( Fw ( row + FS2 -1 ,0 , fRatio ) -
						Fw ( row + FS2 -1 ,0 , fRatio ) * fc.x ) +
						v1 [0].z *
						( fc.x * ( Fw ( row + FS2 -1 ,0 , fRatio ) -
						Fw ( row + FS2 -1 ,1 , fRatio ) ) +
						Fw ( row + FS2 -1 ,1 , fRatio ) ) );
				}
			}
			else if( col == FS2 )
			{
				s += ( 1 - fc.y ) * ( v1 [ FS2 ].w * ( fc.x *
					( Fw ( row + FS2 ,FS -2 , fRatio ) -
					Fw ( row + FS2 , FS -1 , fRatio ) ) +
					Fw ( row + FS2 , FS -1 , fRatio ) ) + v1 [ FS2 ].z * fc.x *
					Fw ( row + FS2 , FS -1 , fRatio ) );
				
				s += ( fc.y ) * ( v1 [ FS2 ]. x * ( fc .x *
					( Fw ( row + FS2 ,FS -2 , fRatio ) -
					Fw ( row + FS2 , FS -1 , fRatio ) ) +
					Fw ( row + FS2 , FS -1 , fRatio ) ) + v1 [ FS2 ].y * fc.x *
					Fw ( row + FS2 , FS -1 , fRatio ) );
					
				if( row > - FS2 )
				{
					s += ( 1 - fc.y ) * ( v0 [ FS2 ].x * ( fc.x *
						( Fw ( row + FS2 -1 , FS -2 , fRatio ) -
						Fw ( row + FS2 -1 , FS -1 , fRatio ) ) +
						Fw ( row + FS2 -1 , FS -1 , fRatio ) ) +
						v0 [ FS2 ]. y* fc.x * Fw ( row + FS2 -1 , FS -1 , fRatio ) );
						
					s += ( fc . y ) * ( v1 [ FS2 ]. w * ( fc.x *
						( Fw ( row + FS2 -1 , FS -2 , fRatio ) -
						Fw ( row + FS2 -1 , FS -1 , fRatio ) ) +
						Fw ( row + FS2 -1 , FS -1 , fRatio ) ) +
						v1 [ FS2 ]. z* fc.x* Fw ( row + FS2 -1 , FS -1 , fRatio ) );
				}
			}
			else
			{
				s += ( 1 - fc.y ) * ( v1 [( col + FS2 )/2].w * ( fc.x *
					( Fw ( row + FS2 , col + FS2 -1 , fRatio ) -
					Fw ( row + FS2 , col + FS2 +0 , fRatio ) ) +
					Fw ( row + FS2 , col + FS2 +0 , fRatio ) ) +
					v1 [( col + FS2 )/2]. z * ( fc.x *
					( Fw ( row + FS2 , col + FS2 -0 , fRatio ) -
					Fw ( row + FS2 , col + FS2 +1 , fRatio ) ) +
					Fw ( row + FS2 , col + FS2 +1 , fRatio ) ) );
				
				s += ( fc.y ) * ( v1 [( col + FS2 )/2].x * ( fc.x *
					( Fw ( row + FS2 , col + FS2 -1 , fRatio ) -
					Fw ( row + FS2 , col + FS2 +0 , fRatio ) ) +
					Fw ( row + FS2 , col + FS2 +0 , fRatio ) ) +
					v1 [( col + FS2 )/2].y * ( fc.x *
					( Fw ( row + FS2 , col + FS2 -0 , fRatio ) -
					Fw ( row + FS2 , col + FS2 +1 , fRatio ) ) +
					Fw ( row + FS2 , col + FS2 +1 , fRatio ) ) );
					
				if( row > - FS2 )
				{
					s += ( 1 - fc.y ) * ( v0 [( col + FS2 )/2]. x * ( fc.x *
						( Fw ( row + FS2 -1 , col + FS2 -1 , fRatio ) -
						Fw ( row + FS2 -1 , col + FS2 +0 , fRatio ) ) +
						Fw ( row + FS2 -1 , col + FS2 +0 , fRatio ) ) +
						v0 [( col + FS2 )/2]. y * ( fc .x *
						( Fw ( row + FS2 -1 , col + FS2 -0 , fRatio ) -
						Fw ( row + FS2 -1 , col + FS2 +1 , fRatio ) ) +
						Fw ( row + FS2 -1 , col + FS2 +1 , fRatio ) ) );
					
					s += ( fc.y ) * ( v1 [( col + FS2 )/2].w * ( fc.x *
						( Fw ( row + FS2 -1 , col + FS2 -1 , fRatio ) -
						Fw ( row + FS2 -1 , col + FS2 +0, fRatio ) ) +
						Fw ( row + FS2 -1 , col + FS2 +0, fRatio ) ) +
						v1 [( col + FS2 )/2]. z * ( fc.x *
						( Fw ( row + FS2 -1 , col + FS2 -0 , fRatio ) -
						Fw ( row + FS2 -1 , col + FS2 +1 , fRatio ) ) +
						Fw ( row + FS2 -1 , col + FS2 +1 , fRatio ) ) );
				}
			}
			if( row != FS2 )
			{
				v0 [( col + FS2 )/2] = v1 [( col + FS2 )/2]. xy;
			}
		}
	}
	return s / w;
}
