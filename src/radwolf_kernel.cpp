/* Derived from RAD-Doom rad_doom.cpp; modified for RAD-Wolf, 2026-11-12. */
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

// RAD-Wolf: no external demo intro or media dependencies.

// required for C128
#define FORCE_RESET_VECTORS

#include "rad_doom.h"
#include "rad_doom_defs.h"
#include "c64screen.h"
#include "linux/kernel.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <circle/usertimer.h>

static const char DRIVE[] = "SD:";
static const char FILENAME_CONFIG[] = "SD:RAD/rad.cfg";

// REU (sort of legacy of the actual RAD software)
#include "rad_reu.h"
#define REU_MAX_SIZE_KB	(16384)
u8 mempool[ REU_MAX_SIZE_KB * 1024 + 8192 ] AAA = {0};

// low-level communication code
u64 armCycleCounter;
#include "lowlevel_dma.h"

static u16 resetVector = 0xFCE2;

// emulate GAME-cartridge to start C128 (also works on C64) with custom reset-vector => forces C128 in C64 mode
void startForcedResetVectors()
{
	register u32 g2, g3;

	const u8 romh[] = { 
		0x4c, 0x0a, 0xe5, 0x4c, 0x00, 0xe5, 0x52, 0x52,
		0x42, 0x59, 0x43, 0xfe, 0xe2, 0xfc, 0x48, 0xff };

	CACHE_PRELOAD_INSTRUCTION_CACHE( (void*)startForcedResetVectors, 1024 * 4 );
	CACHE_PRELOADL1STRM( romh );
	FORCE_READ_LINEARa( (void*)startForcedResetVectors, 1024 * 4, 65536 );
	FORCE_READ_LINEARa( (void*)romh, 16, 1024 );

	OUT_GPIO( DMA_OUT );
	OUT_GPIO( GAME_OUT );

	WAIT_FOR_CPU_HALFCYCLE
	WAIT_FOR_VIC_HALFCYCLE
	RESTART_CYCLE_COUNTER
	WAIT_UP_TO_CYCLE( 100 );
	SET_GPIO( bLATCH_A_OE | bIRQ_OUT | bOE_Dx | bRW_OUT | bDMA_OUT | bDIR_Dx );
	INP_GPIO_RW();
	INP_GPIO_IRQ();

	CLR_GPIO( bGAME_OUT );
	CLR_GPIO( bMPLEX_SEL );

	DELAY( 1 << 20 );
	SET_GPIO( bRESET_OUT );
	INP_GPIO( RESET_OUT );

	u32 nCycles = 0, nRead = 0;
	while ( 1 )
	{
		WAIT_FOR_CPU_HALFCYCLE
		RESTART_CYCLE_COUNTER
		WAIT_UP_TO_CYCLE( 50 );
		RESTART_CYCLE_COUNTER
		WAIT_UP_TO_CYCLE( WAIT_FOR_SIGNALS );
		g2 = read32( ARM_GPIO_GPLEV0 );

		SET_GPIO( bMPLEX_SEL );
		WAIT_UP_TO_CYCLE( WAIT_CYCLE_MULTIPLEXER );
		g3 = read32( ARM_GPIO_GPLEV0 );
		CLR_GPIO( bMPLEX_SEL );

		if ( nCycles ++ > 100000 )
		{
			OUT_GPIO( RESET_OUT );
			CLR_GPIO( bRESET_OUT );
			DELAY( 1 << 18 );
			SET_GPIO( bRESET_OUT );
			INP_GPIO( RESET_OUT );
			nRead = nCycles = 0;
		}

		if ( ROMH_ACCESS && CPU_READS_FROM_BUS )
		{
			u8 d = 0;
			if ( ADDRESS0to7 == 0xfc || ADDRESS0to7 == 0xfd ) nRead ++;
			if ( ADDRESS0to7 == 0xfc ) d = resetVector & 255;
			if ( ADDRESS0to7 == 0xfd ) d = resetVector >> 8;

			{
				register u32 DD = ( ( d ) & 255 ) << D0;
				write32( ARM_GPIO_GPCLR0, ( D_FLAG & ( ~DD ) ) | bOE_Dx | bDIR_Dx );
				write32( ARM_GPIO_GPSET0, DD );
				SET_BANK2_OUTPUT
				WAIT_UP_TO_CYCLE( WAIT_CYCLE_READ );
				SET_GPIO( bOE_Dx | bDIR_Dx );
			}

			if ( nRead >= 2 )
			{
				WAIT_FOR_VIC_HALFCYCLE
				SET_GPIO( bGAME_OUT );
				break;
			}
		}
		WAIT_FOR_VIC_HALFCYCLE
		RESET_CPU_CYCLE_COUNTER
	}
}


#include "rad_doom_hijack.h"

unsigned fiqRegOffset;
u32		 fiqRegMask;
u32		 temperature;

extern "C" void radwolf_wolf_main(void); // RADWOLF_WOLF_ENTRY
extern "C" void dg_Create(void); // RADWOLF_FRAMEBUFFER

CTimer				*pTimer;

u64 lastTick = 0, firstTick = 0;
u32 firstTimer = 1;

#define CLOCKHZ	1000000

u64 globalTime = 0;

extern "C" uint64_t GetuSec()
{
	InstructionSyncBarrier ();
	u64 nCNTPCT, nCNTFRQ;
	asm volatile ("mrs %0, CNTPCT_EL0" : "=r" (nCNTPCT));
	asm volatile ("mrs %0, CNTFRQ_EL0" : "=r" (nCNTFRQ));
	return (unsigned) (nCNTPCT * CLOCKHZ / nCNTFRQ);
}

extern "C" uint64_t GetTickCount()
{
	if ( firstTimer )
	{
		firstTimer = 0;
		firstTick = lastTick = GetuSec();
		return 10;
	}
	u64 t = GetuSec();
	u64 r = t - firstTick;
	return r / 1000;
}

CUserTimer 			*sidtimer;
CInterruptSystem	*pInterrupt;

extern "C" void prepareC64()
{
	f_unmount( "SD:" );
	DisableIRQs();

	// only called to initialize timing values
	REU_SIZE_KB = 128;
	initREU( mempool );

	#ifdef FORCE_RESET_VECTORS
	resetVector = 0xfce2;
	#endif

#ifndef SHOW_INTRO
	checkIfMachineRunning();		
	DELAY( 1 << 27 );
	hijackDOOM( false );
#endif

	EnableIRQs();
}

#define SOUND_PLAY_IRQ
#ifdef RADWOLF_BAREMETAL
#include "radwolf_audio_config.h"
#endif
#define SAMPLE_RATE 22050
#define SOUND_RINGBUF_SIZE	16384

uint64_t soundFirstuSecTick = 0;

extern const unsigned char *mahoneyLUT;


#ifdef SOUND_PLAY_IRQ
#define PEEK( a, v ) { emuReadByteREU_p1( g2, a ); emuReadByteREU_p2( g2 ); emuReadByteREU_p3( g2, v, false ); }
#define POKE( a, v ) { emuWriteByteREU_p1( g2, a, v ); emuWriteByteREU_p2( g2, false ); }

static int nextMouseUpdate = 0, mouseReadStep = 0;
		extern "C" void handleMouseUpdate( uint8_t *mouseData );

void sidSamplePlayIRQ( CUserTimer *pSIDTimer, void *pParam )
{
	uint64_t curTickuSec = GetuSec();
	uint64_t curSoundPos = ( curTickuSec - soundFirstuSecTick ) * SAMPLE_RATE / 1000000ULL;

	extern unsigned char soundRingBuffer[ SOUND_RINGBUF_SIZE ];
	unsigned char s = soundRingBuffer[ curSoundPos & (SOUND_RINGBUF_SIZE-1) ];

	register u32 g2;
	WAIT_FOR_CPU_HALFCYCLE
	WAIT_FOR_VIC_HALFCYCLE
	RESTART_CYCLE_COUNTER

#ifdef RADWOLF_BAREMETAL
	if ( radwolf_sid2_address )
	{
		unsigned char right = radwolf_right_ring[ curSoundPos & (SOUND_RINGBUF_SIZE-1) ];
		// SID2 first: SID1 may also respond to writes at the secondary address.
		POKE( radwolf_sid2_address, right );
		// Restore SID1's left sample with a separate, synchronized bus cycle.
		WAIT_FOR_CPU_HALFCYCLE
		WAIT_FOR_VIC_HALFCYCLE
		RESTART_CYCLE_COUNTER
	}
	POKE( radwolf_sid1_address, s );
#else
	POKE( DAC_ADDRESS, s );
#endif

	pSIDTimer->Start( USER_CLOCKHZ / 22050 );
}
#endif

// Circle CUserTimer::Stop disconnects the IRQ and clears initialization.
// RADWOLF_SID_TIMER_RESTART
static bool sidSamplePlayerStopped = false;

extern "C" void startSIDSamplePlayer()
{
#ifdef SOUND_PLAY_IRQ
	if ( sidSamplePlayerStopped )
	{
		if ( !sidtimer->Initialize() )
			return;
		sidSamplePlayerStopped = false;
	}
	sidtimer->Start( 100 );
#endif
}

extern "C" void stopSIDSamplePlayer()
{
#ifdef SOUND_PLAY_IRQ
	if ( !sidSamplePlayerStopped )
	{
		sidtimer->Stop();
		sidSamplePlayerStopped = true;
	}
#endif
}

extern "C" void radMountFileSystem();
extern "C" int (*functionAddress[])(void);

void  CRAD::Run( void )
{
	gpioInit();
	m_EMMC.Initialize();

	setDefaultTimings( AUTO_TIMING_RPI3PLUS_C64C128 );
	readConfig( logger, DRIVE, FILENAME_CONFIG );

	OUT_GPIO( RESET_OUT );
	CLR_GPIO( bRESET_OUT );
	DELAY( 1 << 25 );
	SET_GPIO( bRESET_OUT );
	INP_GPIO( RESET_OUT );

	pTimer = &m_Timer;
	pInterrupt = &m_Interrupt;

	u64 nCNTFRQ;
	asm volatile ( "mrs %0, CNTFRQ_EL0" : "=r" ( nCNTFRQ ) );
	assert( nCNTFRQ % HZ == 0 );
	u64 m_nClockTicksPerHZTick = nCNTFRQ / HZ;
	u64 nCNTPCT;
	asm volatile ( "mrs %0, CNTPCT_EL0" : "=r" ( nCNTPCT ) );
	asm volatile ( "msr CNTP_CVAL_EL0, %0" :: "r" ( nCNTPCT + m_nClockTicksPerHZTick ) );
	asm volatile ( "msr CNTP_CTL_EL0, %0" :: "r" ( 1 ) );

	FATFS mFileSystem;
	if ( f_mount( &mFileSystem, "SD:", 1 ) != FR_OK )
		logger->Write( "RD", LogError, "failed mounting partition 'SD:'" );

	m_CPUThrottle.SetSpeed( CPUSpeedMaximum );

	sidtimer = new CUserTimer( &m_Interrupt, sidSamplePlayIRQ, this, !true );
	sidtimer->Initialize();
	dg_Create(); // RADWOLF_FRAMEBUFFER_INIT

// Initialize the C64/SID directly, then remount game storage.
	prepareC64();
	radMountFileSystem();
	(*functionAddress[0])();

	radwolf_wolf_main(); // RADWOLF_START

}

extern "C" void radMountFileSystem()
{
	// FatFs retains this pointer after f_mount returns.
	static FATFS mFileSystem;
	if ( f_mount( &mFileSystem, "SD:", 1 ) != FR_OK )
		logger->Write( "RD", LogError, "failed mounting partition 'SD:'" );
}

extern "C" void radUnmountFileSystem()
{
	f_unmount( "SD:" );
}

__attribute__((optimize("align-functions=256"))) void CRAD::FIQHandler( void *pParam )
{
}

int main( void )
{
	CRAD kernel;
	if ( kernel.Initialize() )
		kernel.Run();

	halt();
	return EXIT_HALT;
}

