/*

  {_______            {_          {______
        {__          {_ __               {__
        {__         {_  {__               {__
     {__           {__   {__               {__
 {______          {__     {__              {__
       {__       {__       {__            {__
         {_________         {______________		Expansion Unit

 RADExp - A framework for DMA interfacing with Commodore C64/C128 computers using a Raspberry Pi Zero 2 or 3A+/3B+
        - RAD-DOOM an acceleRADor for running Doom on a C64/C128
        - this file implements the necessary callbacks for Doom Generic and includes most notably
          * color reduction and creation of a multicolor C64 image
          * sound/music rendering and mixing 
          * optional MIDI music rendering

 Copyright (c) 2022, 2023 Carsten Dachsbacher <frenetic@dachsbacher.de>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/* RAD-Wolf video adapter derived from RAD-Doom; modified 2026-11-12.
 * Only RGB quantization and VIC-II presentation are retained. */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <arm_neon.h>
#include "radwolf_gray.h"
#include "radwolf_flat.h"
#define RADWOLF_VIDEO_WIDTH 320
#define RADWOLF_VIDEO_HEIGHT 200
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
static uint32_t framebuffer[320 * 200];
uint32_t *DG_ScreenBuffer;
unsigned char soundRingBuffer[16384];
static uint8_t bluenoise256[256 * 256];
extern void blitScreenDOOM(uint8_t *, uint32_t *, uint8_t *, uint8_t *);
const float invGamma = 0.96111111111f;
const int   exposure = 350;

static uint8_t ditherMode = 2;
static uint8_t selectedPreset = 0;
static uint8_t alternatePattern = 1;
static int displayStatus = 0;
static int displayPreset = 0;
static int displayHelp = 0;
static int flickerMode = 0;
static int16_t brightnessScale = 20;

#define RGB_QUANTIZE_BITS	4
#define RGB_LEVELS			( 1 << RGB_QUANTIZE_BITS )

uint8_t mapRGB2C64[ RGB_LEVELS * RGB_LEVELS * RGB_LEVELS ];

#define c64PalettePepto radwolf_c64_palette

const uint8_t ditherMatrix4x4[ 4 * 4 ] = {
	0, 12, 3, 15,
	8, 4, 11, 7,
	2, 14, 1, 13,
	10, 6, 9, 5 };

const uint8_t ditherMatrix4x4_line[ 4 * 4 ] = {
	 0,  4,  2,  6,
	 8, 12, 10, 14,
	 3,  7,  1,  5,
	11, 15,  9, 13 };

static const uint8_t ditherMatrix8x8[ 8 * 8 ] = {
	 0, 48, 12, 60,  3, 51, 15, 63, 
	32, 16, 44, 28, 35, 19, 47, 31, 
	 8, 56,  4, 52, 11, 59,  7, 55, 
	40, 24, 36, 20, 43, 27, 39, 23, 
	 2, 50, 14, 62,  1, 49, 13, 61, 
	34, 18, 46, 30, 33, 17, 45, 29, 
	10, 58,  6, 54,  9, 57,  5, 53, 
	42, 26, 38, 22, 41, 25, 37, 21 };

static const uint8_t ditherMatrix8x8_line[ 8 * 8 ] = {
	 0, 16,  4, 20,  2, 18,  6, 22, 
	32, 48, 36, 52, 34, 50, 38, 54, 
	 8, 24, 12, 28, 10, 28, 14, 30, 
	40, 56, 44, 60, 42, 58, 46, 62, 
	 3, 19,  7, 23,  1, 17,  5, 21, 
	35, 51, 39, 55, 33, 49, 37, 53, 
	11, 27, 15, 31,  9, 25, 13, 29, 
	43, 59, 47, 63, 41, 57, 45, 61 };

// rank how close color j is to color i (idx = i * 16 + j)
const uint8_t mapC64Closest[ 16 * 16 ] = {
    0x09, 0x06, 0x0B, 0x02, 0x04, 0x08, 0x0C, 0x0E, 0x0A, 0x05, 0x0F, 0x03, 0x07, 0x0D, 0x01, 0xFF,
    0x0D, 0x07, 0x03, 0x0F, 0x05, 0x0A, 0x0C, 0x0E, 0x08, 0x04, 0x0B, 0x02, 0x09, 0x06, 0x00, 0xFF,
    0x08, 0x09, 0x0B, 0x04, 0x0C, 0x0A, 0x06, 0x0E, 0x05, 0x00, 0x0F, 0x03, 0x07, 0x0D, 0x01, 0xFF,
    0x0F, 0x05, 0x0C, 0x0D, 0x07, 0x0A, 0x0E, 0x04, 0x08, 0x0B, 0x01, 0x02, 0x09, 0x06, 0x00, 0xFF,
    0x0E, 0x06, 0x0B, 0x0C, 0x02, 0x08, 0x09, 0x0A, 0x0F, 0x05, 0x03, 0x00, 0x07, 0x0D, 0x01, 0xFF,
    0x0C, 0x0F, 0x0A, 0x03, 0x08, 0x07, 0x0D, 0x0B, 0x02, 0x0E, 0x09, 0x04, 0x06, 0x01, 0x00, 0xFF,
    0x04, 0x0B, 0x09, 0x02, 0x0E, 0x08, 0x00, 0x0C, 0x0A, 0x05, 0x0F, 0x03, 0x07, 0x0D, 0x01, 0xFF,
    0x0D, 0x0F, 0x03, 0x05, 0x01, 0x0A, 0x0C, 0x08, 0x0E, 0x04, 0x02, 0x0B, 0x09, 0x06, 0x00, 0xFF,
    0x02, 0x0B, 0x09, 0x0A, 0x0C, 0x04, 0x05, 0x06, 0x0E, 0x0F, 0x03, 0x00, 0x07, 0x0D, 0x01, 0xFF,
    0x0B, 0x02, 0x08, 0x06, 0x04, 0x0C, 0x00, 0x0A, 0x05, 0x0E, 0x0F, 0x03, 0x07, 0x0D, 0x01, 0xFF,
    0x0C, 0x08, 0x0F, 0x05, 0x02, 0x04, 0x0B, 0x03, 0x0E, 0x09, 0x07, 0x06, 0x0D, 0x00, 0x01, 0xFF,
    0x09, 0x02, 0x08, 0x04, 0x06, 0x0C, 0x0A, 0x0E, 0x05, 0x00, 0x0F, 0x03, 0x07, 0x0D, 0x01, 0xFF,
    0x0A, 0x08, 0x05, 0x0F, 0x0B, 0x04, 0x0E, 0x02, 0x03, 0x09, 0x06, 0x07, 0x0D, 0x00, 0x01, 0xFF,
    0x07, 0x03, 0x0F, 0x05, 0x01, 0x0A, 0x0C, 0x08, 0x0E, 0x04, 0x0B, 0x02, 0x09, 0x06, 0x00, 0xFF,
    0x04, 0x0C, 0x06, 0x0A, 0x0B, 0x0F, 0x03, 0x08, 0x02, 0x05, 0x09, 0x07, 0x0D, 0x00, 0x01, 0xFF,
    0x03, 0x05, 0x0C, 0x0A, 0x07, 0x0D, 0x0E, 0x08, 0x04, 0x0B, 0x02, 0x09, 0x01, 0x06, 0x00, 0xFF
};

static uint8_t koalaData[ 10000 + 1 ];

int colorDistance( const int r, const int g, const int b, const int r_, const int g_, const int b_ )
{
	return ( r - r_ ) * ( r - r_ ) + ( g - g_ ) * ( g - g_ ) + ( b - b_ ) * ( b - b_ );
}

void precomputeColorQuantization()
{
	memset( koalaData, 0, 10000 + 1 );

	// rank how close color j is to color i
	#if 0
	for ( int i = 0; i < 16; i++ )
	{
		int r, g, b;
		r = c64PalettePepto[ i ][ 0 ];
		g = c64PalettePepto[ i ][ 1 ];
		b = c64PalettePepto[ i ][ 2 ];

		int err[ 16 ], idx[ 16 ];

		for ( int j = 0; j < 16; j++ )
		{
			err[ j ] = 1 << 30;
			idx[ j ] = -1;
		}

		for ( int j = 0; j < 16; j++ )
		if ( j != i )
		{
			int e = colorDistance( r, g, b, c64PalettePepto[ j ][ 0 ], c64PalettePepto[ j ][ 1 ], c64PalettePepto[ j ][ 2 ] );

			for ( int k = 0; k < 16; k++ )
			if ( e < err[ k ] )
			{
				for ( int l = 15; l >= k + 1; l -- )
				{
					idx[ l ] = idx[ l - 1 ];
					err[ l ] = err[ l - 1 ];
				}
				idx[ k ] = j;
				err[ k ] = e;
				goto colorIsRanked;
			}
		colorIsRanked:;
		}

		for ( int j = 0; j < 16; j++ )
			mapC64Closest[ i * 16 + j ] = idx[ j ];
	}
	#endif

	// map quantized RGB-values to C64-colors
	for ( int r = 0; r < RGB_LEVELS; r++ )
		for ( int g = 0; g < RGB_LEVELS; g++ )
			for ( int b = 0; b < RGB_LEVELS; b++ )
			{
				int ofs = ( ( ( r << RGB_QUANTIZE_BITS ) + g ) << RGB_QUANTIZE_BITS ) + b;

				int rr = ( ( r << ( 8 - RGB_QUANTIZE_BITS ) ) * exposure ) >> 8;
				int gg = ( ( g << ( 8 - RGB_QUANTIZE_BITS ) ) * exposure ) >> 8;
				int bb = ( ( b << ( 8 - RGB_QUANTIZE_BITS ) ) * exposure ) >> 8;

				rr = powf( (float)rr / 255.0f, invGamma ) * 255.0f;
				gg = powf( (float)gg / 255.0f, invGamma ) * 255.0f;
				bb = powf( (float)bb / 255.0f, invGamma ) * 255.0f;

				int idx = 0;
				int minError = 1 << 30;
				for ( int i = 0; i < 16; i++ )
				{
					int e = colorDistance( c64PalettePepto[ i ][ 0 ], c64PalettePepto[ i ][ 1 ], c64PalettePepto[ i ][ 2 ], rr, gg, bb );

					if ( e < minError )
					{
						idx = i;
						minError = e;
					}
				}
				mapRGB2C64[ ofs ] = idx;
			}
}


const int16x4_t quantMax = { RGB_LEVELS-1, RGB_LEVELS-1, RGB_LEVELS-1, RGB_LEVELS-1 }, quantMin = { 0, 0, 0, 0 };

int bgColor = 0;


void doImageConversion()
{
	//const uint8_t *dm;
	//uint8_t dmask, dshift, dofs, dmul = 4 * 2, dmax;

	static int fc = 0;
	fc ++;

	static int xofs = 0, yofs = 0;

	if ( ditherMode == 4 )	
	{
		if ( ( fc & 1 ) )
		{
			xofs += 137;
			yofs += 119;
		}
	} else
		xofs = yofs = 0;

	static uint8_t dynDM[ 4096 ];
	static uint8_t *dm = dynDM;
	static uint8_t lastDitherMode = 255;

	static uint8_t dshift, dmaskX, dmaskY;
	static int dmul, dofs;

	if ( ditherMode < 3 && ditherMode != lastDitherMode ) // ordered dithering
	{
		int M = 1 + ditherMode, L = M + 1;
		int sizeX = 1 << M, sizeY = 1 << L;

		uint8_t *p = dynDM;
		// matrix generation code from: https://bisqwit.iki.fi/story/howto/dither/jy/
		for ( int y = 0; y < sizeY; y++ )
		{
			for ( int x = 0; x < sizeX; x++ )
			{
				int v = 0, ofs = 0, maskX = M, maskY = L;
				if ( M == 0 || ( M > L && L != 0 ) )
				{
					int xc = x ^ ( ( y << M ) >> L ), yc = y;
					for ( int bit = 0; bit < M + L; )
					{
						v |= ( ( yc >> --maskY ) & 1 ) << bit++;
						for ( ofs += M; ofs >= L; ofs -= L )
							v |= ( ( xc >> --maskX ) & 1 ) << bit++;
					}
				} else
				{
					int xc = x, yc = y ^ ( ( x << L ) >> M );
					for ( int bit = 0; bit < M + L; )
					{
						v |= ( ( xc >> --maskX ) & 1 ) << bit++;
						for ( ofs += L; ofs >= M; ofs -= M )
							v |= ( ( yc >> --maskY ) & 1 ) << bit++;
					}
				}
				*( p++ ) = v;
			}
		}

		dm = dynDM;
		dmul = 1024 >> (int)( M + L );
		dofs = 512 / dmul - 1; 
		dmaskX = sizeX - 1;
		dmaskY = sizeY - 1;
		dshift = M;
		dmul *= 2;
	}

	if ( ditherMode >= 3 && ditherMode < 5  && ditherMode != lastDitherMode ) // blue noise
	{
		dm = bluenoise256;			
		dmaskX = dmaskY = 255; 
		dshift = 8; 
		dofs = 128; 
		dmul = 4 * 2;
	}

	lastDitherMode = ditherMode;

	static int colorOccurence[ 16 ];

	static int firstBGGuess = 1;

	if ( firstBGGuess /*|| ( testVal & 2 ) == 0*/ )
	{
		firstBGGuess = 0;
		bgColor = 0;
		memset( colorOccurence, 0, sizeof( int ) * 16 );
	} else
	//if ( fc & 1 )
	{
		int freq = colorOccurence[ 0 ];
		bgColor = 0;
		for ( int i = 1; i < 15; i++ )
			if ( colorOccurence[ i ] > freq )
			{
				freq = colorOccurence[ i ];
				bgColor = i;
			}
		memset( colorOccurence, 0, sizeof( int ) * 16 );
	}

	for ( int y = 0; y < RADWOLF_VIDEO_HEIGHT; y += 8 )
	{
		for ( int x = 0; x < RADWOLF_VIDEO_WIDTH; x += 8 )
		{
			uint8_t histo[ 16 ];
			memset( histo, 0, 16 );
			
			uint8_t blockColors[ 4 * 8 ];

			// here we map 8x8 RGB-pixels to 4x8 C64-color-pixels (without restrictions)
			for ( int c = 0; c < 8; c++ )
			{
				for ( int a = 0; a < 8; a += 2 )
				{
					uint8x8_t  rgba_2    = vld1_u8( (const uint8_t *)&DG_ScreenBuffer[ ( x + a ) + ( y + c ) * RADWOLF_VIDEO_WIDTH ] );
					uint16x8_t rgba_2_16 = vmovl_u8( rgba_2 );

					int c64color;
					int solid_color = radwolf_flat_pair(
						DG_ScreenBuffer[(x + a) + (y + c) * RADWOLF_VIDEO_WIDTH],
						DG_ScreenBuffer[(x + a + 1) + (y + c) * RADWOLF_VIDEO_WIDTH]);
					if (solid_color >= 0)
					{
						/* Only explicitly tagged surfaces bypass texture dithering.
						 * Matching tags on BOTH pixels protect object boundaries. */
						c64color = solid_color;
					} else
					{
						int16x4_t t1 = vget_low_s16 ( *(int16x8_t*)&rgba_2_16 );
						int16x4_t t2 = vget_high_s16( *(int16x8_t*)&rgba_2_16 );

						t1 = vqadd_s16( t1, t2 );

						const int32x4_t lumaWeight = { 114, 587, 299, 0 };
						int32x4_t wc = vmulq_s32( vmovl_s16( t1 ), lumaWeight );
						int lum = ( (int)wc[ 0 ] + (int)wc[ 1 ] + (int)wc[ 2 ] ) >> 11;

						t1 = vmul_s16( t1, vld1_dup_s16( &brightnessScale ) );

						int i = (a/2 + x/2) + 0;

						int curdofs, curdmul, ydisp;

						if ( lum < flickerMode )
						{
							curdofs = dofs / 1;
							curdmul = dmul / 2;
							ydisp = fc & 1;

							if ( alternatePattern )
								ydisp ^= i & 1;
						} else
						{
							curdofs = dofs / 2;
							curdmul = dmul / 1;
							if ( ditherMode >= 3 ) curdmul >>= 1; // bluenoise 
							ydisp = 0;
						}
							
						int cc = ( c + y ) * 2 + ydisp;
						i += xofs; cc += yofs;				
						int16_t ditherValue = ( dm[ ( i & dmaskX ) + ( ( cc & dmaskY ) << dshift ) ] - curdofs ) * curdmul;

						t1 = vqadd_s16( t1, vld1_dup_s16( &ditherValue ) );
						t1 >>= 5 + ( 8 - RGB_QUANTIZE_BITS ); 
						t1 = vmin_s16( t1, quantMax );
						t1 = vmax_s16( t1, quantMin );
						
						register uint16_t r, g, b;
						r = *(uint16_t *)&t1[ 2 ];
						g = *(uint16_t *)&t1[ 1 ];
						b = *(uint16_t *)&t1[ 0 ];

						int ofs = ( ( ( r << RGB_QUANTIZE_BITS ) + g ) << RGB_QUANTIZE_BITS ) + b;
						c64color = mapRGB2C64[ ofs ];
					}

					histo[ c64color ] ++;
					blockColors[ ( a >> 1 ) + c * 4 ] = c64color;
				}
			}

			// count in how many blocks we had color x
			for ( int i = 0; i < 16; i++ )
				if ( histo[ i ] )
					colorOccurence[ i ] ++;

			// now apply restrictions of the multicolor mode

			// if background color is fixed:
			//const int bgColor = 0;
			histo[ bgColor ] = 32;

			int nColors = 0;
			uint8_t map[ 16 ];
			for ( int i = 0; i < 16; i++ )
			{
				if ( histo[ i ] ) nColors ++;	// count each color occurence
				map[ i ] = i;					// initial mapping 1:1
			}

			while ( nColors > 4 )
			{
				// find and remove least frequent color
				int minV = 256, minIdx = 255;
				for ( int i = 0; i < 16; i++ )
					if ( histo[ i ] && histo[ i ] < minV )
					{
						minV = histo[ i ];
						minIdx = i;
					}
				nColors --;

				// now remap color 'minIdx' to best fit... (and add its occurence to the target color)
				uint8_t tmpCount = histo[ minIdx ];
				histo[ minIdx ] = 0;

				const uint8_t *mapColors = &mapC64Closest[ minIdx * 16 ];
				int i = 0;
				while ( i < 15 && histo[ mapColors[ i ] ] == 0 ) 
					i ++;
				map[ minIdx ] = mapColors[ i ];

				for ( int j = 0; j < 16; j++ )
					if ( histo[ j ] == 0 && map[ j ] == minIdx )
						map[ j ] = mapColors[ i ];

				histo[ mapColors[ i ] ] += tmpCount;
			};

			// determined colors for screen and color RAM
			int w = 0;

			uint8_t screenRAM = 0, colorRAM = 0;
			
			// optional (for double-buffering): choose the same color for color RAM  ($d800) is possible
			uint8_t prevColorRAM = koalaData[ 9000 + ( y / 8 ) * 40 + ( x / 8 ) ];

			// if we can keep the color RAM nibble, do that
			if ( prevColorRAM != bgColor && histo[ prevColorRAM ] )
			{
				w = 1;
				histo[ prevColorRAM ] = 0;
				colorRAM = prevColorRAM;
			}

			for ( int i = 0; i < 16; i++ )
			if ( i != bgColor && histo[ i ] )
			{
				if ( w == 2 )
				{
					screenRAM |= i << 4;
					w ++;
				} else
				if ( w == 1 )
				{
					screenRAM |= i;
					w ++;
				} else
				if ( w == 0 )
				{
					colorRAM = i;
					w ++;
				}
			}

			koalaData[ 8000 + ( y / 8 ) * 40 + ( x / 8 ) ] = screenRAM;
			koalaData[ 9000 + ( y / 8 ) * 40 + ( x / 8 ) ] = colorRAM;

			for ( int c = 0; c < 8; c++ )
			{
				register uint8_t kBitmapData = 0;
				for ( int a = 0; a < 4; a ++ )
				{
					uint8_t c64color = map[ blockColors[ a + c * 4 ] ];

					uint8_t pixelValue = 0;
					if ( c64color == colorRAM ) pixelValue = 0b11; else
					if ( c64color == (screenRAM & 15) ) pixelValue = 0b10; else
					if ( c64color == (screenRAM >> 4) ) pixelValue = 0b01; 

					// debug out: show chosen background color in top-right corner
					//if ( x > 300 && y < 20 )
					//	pixelValue = 0b00;

					kBitmapData <<= 2;
					kBitmapData |= pixelValue;
				}

				koalaData[ y * 40 + x + c ] = kBitmapData;
			}
		}
	}

	koalaData[ 10000 ] = bgColor;
}



void dg_Create(void) { DG_ScreenBuffer = framebuffer; }
static int radwolf_video_prepare(void)
{
    /* Default ordered dithering is unchanged; no external noise asset needed. */
    precomputeColorQuantization();
    return 0;
}
static int radwolf_video_present(void)
{
    uint32_t events[64]; uint8_t count = 0, mouse[4];
    doImageConversion();
    blitScreenDOOM(koalaData, events, &count, mouse);
    return 0;
}
int (*functionAddress[])(void) = {radwolf_video_prepare, radwolf_video_present};
