/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar_Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to usecopymodify and distribute Snes9x in both binary and
 * source formfor non-commercial purposesis hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is'without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co.Limited and its subsidiary companies.
 */
/**********************************************************************************************/
/* CPU-S9xOpcodes.CPP                                                                            */
/* This file contains all the opcodes                                                         */
/**********************************************************************************************/

#include "cpuexec.h"
#include "cpuaddr.h"
#include "cpuops.h"
#include "cpumacro.h"
#include "apu.h"

#define ApuSync() do { \
	CPU.Cycles = CPU.NextEvent; \
	if (CPU.APU_APUExecuting) { \
		ICPU.CPUExecuting = FALSE; \
		do \
		{ \
			APU_EXECUTE1 (); \
		} while (CPU.APU_Cycles < CPU.NextEvent); \
		ICPU.CPUExecuting = TRUE; \
	} \
} while (0);


/* ADC *************************************************************************************** */
static void Op69M1 ()
{
    long OpAddress = Immediate8 ();
    ADC8 (OpAddress);
}

static void Op69M0 ()
{
    long OpAddress = Immediate16 ();
    ADC16 (OpAddress);
}

static void Op65M1 ()
{
    long OpAddress = Direct ();
    ADC8 (OpAddress);
}

static void Op65M0 ()
{
    long OpAddress = Direct ();
    ADC16 (OpAddress);
}

static void Op75M1 ()
{
    long OpAddress = DirectIndexedX ();
    ADC8 (OpAddress);
}

static void Op75M0 ()
{
    long OpAddress = DirectIndexedX ();
    ADC16 (OpAddress);
}

static void Op72M1 ()
{
    long OpAddress = DirectIndirect ();
    ADC8 (OpAddress);
}

static void Op72M0 ()
{
    long OpAddress = DirectIndirect ();
    ADC16 (OpAddress);
}

static void Op61M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    ADC8 (OpAddress);
}

static void Op61M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    ADC16 (OpAddress);
}

static void Op71M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    ADC8 (OpAddress);
}

static void Op71M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    ADC16 (OpAddress);
}

static void Op67M1 ()
{
    long OpAddress = DirectIndirectLong ();
    ADC8 (OpAddress);
}

static void Op67M0 ()
{
    long OpAddress = DirectIndirectLong ();
    ADC16 (OpAddress);
}

static void Op77M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    ADC8 (OpAddress);
}

static void Op77M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    ADC16 (OpAddress);
}

static void Op6DM1 ()
{
    long OpAddress = Absolute ();
    ADC8 (OpAddress);
}

static void Op6DM0 ()
{
    long OpAddress = Absolute ();
    ADC16 (OpAddress);
}

static void Op7DM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ADC8 (OpAddress);
}

static void Op7DM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ADC16 (OpAddress);
}

static void Op79M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    ADC8 (OpAddress);
}

static void Op79M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    ADC16 (OpAddress);
}

static void Op6FM1 ()
{
    long OpAddress = AbsoluteLong ();
    ADC8 (OpAddress);
}

static void Op6FM0 ()
{
    long OpAddress = AbsoluteLong ();
    ADC16 (OpAddress);
}

static void Op7FM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    ADC8 (OpAddress);
}

static void Op7FM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    ADC16 (OpAddress);
}

static void Op63M1 ()
{
    long OpAddress = StackRelative ();
    ADC8 (OpAddress);
}

static void Op63M0 ()
{
    long OpAddress = StackRelative ();
    ADC16 (OpAddress);
}

static void Op73M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    ADC8 (OpAddress);
}

static void Op73M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    ADC16 (OpAddress);
}

/**********************************************************************************************/

/* AND *************************************************************************************** */
static void Op29M1 ()
{
    Registers.AL &= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.AL);
}

static void Op29M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W &= *(uint16 *) CPU.PC;
#else
    Registers.A.W &= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.A.W);
}

static void Op25M1 ()
{
    long OpAddress = Direct ();
    AND8 (OpAddress);
}

static void Op25M0 ()
{
    long OpAddress = Direct ();
    AND16 (OpAddress);
}

static void Op35M1 ()
{
    long OpAddress = DirectIndexedX ();
    AND8 (OpAddress);
}

static void Op35M0 ()
{
    long OpAddress = DirectIndexedX ();
    AND16 (OpAddress);
}

static void Op32M1 ()
{
    long OpAddress = DirectIndirect ();
    AND8 (OpAddress);
}

static void Op32M0 ()
{
    long OpAddress = DirectIndirect ();
    AND16 (OpAddress);
}

static void Op21M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    AND8 (OpAddress);
}

static void Op21M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    AND16 (OpAddress);
}

static void Op31M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    AND8 (OpAddress);
}

static void Op31M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    AND16 (OpAddress);
}

static void Op27M1 ()
{
    long OpAddress = DirectIndirectLong ();
    AND8 (OpAddress);
}

static void Op27M0 ()
{
    long OpAddress = DirectIndirectLong ();
    AND16 (OpAddress);
}

static void Op37M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    AND8 (OpAddress);
}

static void Op37M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    AND16 (OpAddress);
}

static void Op2DM1 ()
{
    long OpAddress = Absolute ();
    AND8 (OpAddress);
}

static void Op2DM0 ()
{
    long OpAddress = Absolute ();
    AND16 (OpAddress);
}

static void Op3DM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    AND8 (OpAddress);
}

static void Op3DM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    AND16 (OpAddress);
}

static void Op39M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    AND8 (OpAddress);
}

static void Op39M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    AND16 (OpAddress);
}

static void Op2FM1 ()
{
    long OpAddress = AbsoluteLong ();
    AND8 (OpAddress);
}

static void Op2FM0 ()
{
    long OpAddress = AbsoluteLong ();
    AND16 (OpAddress);
}

static void Op3FM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    AND8 (OpAddress);
}

static void Op3FM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    AND16 (OpAddress);
}

static void Op23M1 ()
{
    long OpAddress = StackRelative ();
    AND8 (OpAddress);
}

static void Op23M0 ()
{
    long OpAddress = StackRelative ();
    AND16 (OpAddress);
}

static void Op33M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    AND8 (OpAddress);
}

static void Op33M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    AND16 (OpAddress);
}
/**********************************************************************************************/

/* ASL *************************************************************************************** */
static void Op0AM1 ()
{
    A_ASL8 ();
}

static void Op0AM0 ()
{
    A_ASL16 ();
}

static void Op06M1 ()
{
    long OpAddress = Direct ();
    ASL8 (OpAddress);
}

static void Op06M0 ()
{
    long OpAddress = Direct ();
    ASL16 (OpAddress);
}

static void Op16M1 ()
{
    long OpAddress = DirectIndexedX ();
    ASL8 (OpAddress);
}

static void Op16M0 ()
{
    long OpAddress = DirectIndexedX ();
    ASL16 (OpAddress);
}

static void Op0EM1 ()
{
    long OpAddress = Absolute ();
    ASL8 (OpAddress);
}

static void Op0EM0 ()
{
    long OpAddress = Absolute ();
    ASL16 (OpAddress);
}

static void Op1EM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ASL8 (OpAddress);
}

static void Op1EM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ASL16 (OpAddress);
}
/**********************************************************************************************/

/* BIT *************************************************************************************** */
static void Op89M1 ()
{
    ICPU._Zero = Registers.AL & *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void Op89M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    ICPU._Zero = (Registers.A.W & *(uint16 *) CPU.PC) != 0;
#else
    ICPU._Zero = (Registers.A.W & (*CPU.PC + (*(CPU.PC + 1) << 8))) != 0;
#endif	
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    CPU.PC += 2;
}

static void Op24M1 ()
{
    long OpAddress = Direct ();
    BIT8 (OpAddress);
}

static void Op24M0 ()
{
    long OpAddress = Direct ();
    BIT16 (OpAddress);
}

static void Op34M1 ()
{
    long OpAddress = DirectIndexedX ();
    BIT8 (OpAddress);
}

static void Op34M0 ()
{
    long OpAddress = DirectIndexedX ();
    BIT16 (OpAddress);
}

static void Op2CM1 ()
{
    long OpAddress = Absolute ();
    BIT8 (OpAddress);
}

static void Op2CM0 ()
{
    long OpAddress = Absolute ();
    BIT16 (OpAddress);
}

static void Op3CM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    BIT8 (OpAddress);
}

static void Op3CM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    BIT16 (OpAddress);
}
/**********************************************************************************************/

/* CMP *************************************************************************************** */
static void OpC9M1 ()
{
    int32 Int32 = (int) Registers.AL - (int) *CPU.PC++;
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC9M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) Registers.A.W - (long) *(uint16 *) CPU.PC;
#else
    int32 Int32 = (long) Registers.A.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC5M1 ()
{
    long OpAddress = Direct ();
    CMP8 (OpAddress);
}

static void OpC5M0 ()
{
    long OpAddress = Direct ();
    CMP16 (OpAddress);
}

static void OpD5M1 ()
{
    long OpAddress = DirectIndexedX ();
    CMP8 (OpAddress);
}

static void OpD5M0 ()
{
    long OpAddress = DirectIndexedX ();
    CMP16 (OpAddress);
}

static void OpD2M1 ()
{
    long OpAddress = DirectIndirect ();
    CMP8 (OpAddress);
}

static void OpD2M0 ()
{
    long OpAddress = DirectIndirect ();
    CMP16 (OpAddress);
}

static void OpC1M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    CMP8 (OpAddress);
}

static void OpC1M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    CMP16 (OpAddress);
}

static void OpD1M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    CMP8 (OpAddress);
}

static void OpD1M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    CMP16 (OpAddress);
}

static void OpC7M1 ()
{
    long OpAddress = DirectIndirectLong ();
    CMP8 (OpAddress);
}

static void OpC7M0 ()
{
    long OpAddress = DirectIndirectLong ();
    CMP16 (OpAddress);
}

static void OpD7M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    CMP8 (OpAddress);
}

static void OpD7M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    CMP16 (OpAddress);
}

static void OpCDM1 ()
{
    long OpAddress = Absolute ();
    CMP8 (OpAddress);
}

static void OpCDM0 ()
{
    long OpAddress = Absolute ();
    CMP16 (OpAddress);
}

static void OpDDM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    CMP8 (OpAddress);
}

static void OpDDM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    CMP16 (OpAddress);
}

static void OpD9M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    CMP8 (OpAddress);
}

static void OpD9M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    CMP16 (OpAddress);
}

static void OpCFM1 ()
{
    long OpAddress = AbsoluteLong ();
    CMP8 (OpAddress);
}

static void OpCFM0 ()
{
    long OpAddress = AbsoluteLong ();
    CMP16 (OpAddress);
}

static void OpDFM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    CMP8 (OpAddress);
}

static void OpDFM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    CMP16 (OpAddress);
}

static void OpC3M1 ()
{
    long OpAddress = StackRelative ();
    CMP8 (OpAddress);
}

static void OpC3M0 ()
{
    long OpAddress = StackRelative ();
    CMP16 (OpAddress);
}

static void OpD3M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    CMP8 (OpAddress);
}

static void OpD3M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    CMP16 (OpAddress);
}

/**********************************************************************************************/

/* CMX *************************************************************************************** */
static void OpE0X1 ()
{
    int32 Int32 = (int) Registers.XL - (int) *CPU.PC++;
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpE0X0 ()
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) Registers.X.W - (long) *(uint16 *) CPU.PC;
#else
    int32 Int32 = (long) Registers.X.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpE4X1 ()
{
    long OpAddress = Direct ();
    CMX8 (OpAddress);
}

static void OpE4X0 ()
{
    long OpAddress = Direct ();
    CMX16 (OpAddress);
}

static void OpECX1 ()
{
    long OpAddress = Absolute ();
    CMX8 (OpAddress);
}

static void OpECX0 ()
{
    long OpAddress = Absolute ();
    CMX16 (OpAddress);
}

/**********************************************************************************************/

/* CMY *************************************************************************************** */
static void OpC0X1 ()
{
    int32 Int32 = (int) Registers.YL - (int) *CPU.PC++;
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC0X0 ()
{
#ifdef FAST_LSB_WORD_ACCESS    
    int32 Int32 = (long) Registers.Y.W - (long) *(uint16 *) CPU.PC;
#else
    int32 Int32 = (long) Registers.Y.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC4X1 ()
{
    long OpAddress = Direct ();
    CMY8 (OpAddress);
}

static void OpC4X0 ()
{
    long OpAddress = Direct ();
    CMY16 (OpAddress);
}

static void OpCCX1 ()
{
    long OpAddress = Absolute ();
    CMY8 (OpAddress);
}

static void OpCCX0 ()
{
    long OpAddress = Absolute ();
    CMY16 (OpAddress);
}

/**********************************************************************************************/

/* DEC *************************************************************************************** */
static void Op3AM1 ()
{
    A_DEC8 ();
}

static void Op3AM0 ()
{
    A_DEC16 ();
}

static void OpC6M1 ()
{
    long OpAddress = Direct ();
    DEC8 (OpAddress);
}

static void OpC6M0 ()
{
    long OpAddress = Direct ();
    DEC16 (OpAddress);
}

static void OpD6M1 ()
{
    long OpAddress = DirectIndexedX ();
    DEC8 (OpAddress);
}

static void OpD6M0 ()
{
    long OpAddress = DirectIndexedX ();
    DEC16 (OpAddress);
}

static void OpCEM1 ()
{
    long OpAddress = Absolute ();
    DEC8 (OpAddress);
}

static void OpCEM0 ()
{
    long OpAddress = Absolute ();
    DEC16 (OpAddress);
}

static void OpDEM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    DEC8 (OpAddress);
}

static void OpDEM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    DEC16 (OpAddress);
}

/**********************************************************************************************/

/* EOR *************************************************************************************** */
static void Op49M1 ()
{
    Registers.AL ^= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.AL);
}

static void Op49M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W ^= *(uint16 *) CPU.PC;
#else
    Registers.A.W ^= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.A.W);
}

static void Op45M1 ()
{
    long OpAddress = Direct ();
    EOR8 (OpAddress);
}

static void Op45M0 ()
{
    long OpAddress = Direct ();
    EOR16 (OpAddress);
}

static void Op55M1 ()
{
    long OpAddress = DirectIndexedX ();
    EOR8 (OpAddress);
}

static void Op55M0 ()
{
    long OpAddress = DirectIndexedX ();
    EOR16 (OpAddress);
}

static void Op52M1 ()
{
    long OpAddress = DirectIndirect ();
    EOR8 (OpAddress);
}

static void Op52M0 ()
{
    long OpAddress = DirectIndirect ();
    EOR16 (OpAddress);
}

static void Op41M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    EOR8 (OpAddress);
}

static void Op41M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    EOR16 (OpAddress);
}

static void Op51M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    EOR8 (OpAddress);
}

static void Op51M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    EOR16 (OpAddress);
}

static void Op47M1 ()
{
    long OpAddress = DirectIndirectLong ();
    EOR8 (OpAddress);
}

static void Op47M0 ()
{
    long OpAddress = DirectIndirectLong ();
    EOR16 (OpAddress);
}

static void Op57M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    EOR8 (OpAddress);
}

static void Op57M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    EOR16 (OpAddress);
}

static void Op4DM1 ()
{
    long OpAddress = Absolute ();
    EOR8 (OpAddress);
}

static void Op4DM0 ()
{
    long OpAddress = Absolute ();
    EOR16 (OpAddress);
}

static void Op5DM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    EOR8 (OpAddress);
}

static void Op5DM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    EOR16 (OpAddress);
}

static void Op59M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    EOR8 (OpAddress);
}

static void Op59M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    EOR16 (OpAddress);
}

static void Op4FM1 ()
{
    long OpAddress = AbsoluteLong ();
    EOR8 (OpAddress);
}

static void Op4FM0 ()
{
    long OpAddress = AbsoluteLong ();
    EOR16 (OpAddress);
}

static void Op5FM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    EOR8 (OpAddress);
}

static void Op5FM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    EOR16 (OpAddress);
}

static void Op43M1 ()
{
    long OpAddress = StackRelative ();
    EOR8 (OpAddress);
}

static void Op43M0 ()
{
    long OpAddress = StackRelative ();
    EOR16 (OpAddress);
}

static void Op53M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    EOR8 (OpAddress);
}

static void Op53M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    EOR16 (OpAddress);
}

/**********************************************************************************************/

/* INC *************************************************************************************** */
static void Op1AM1 ()
{
    A_INC8 ();
}

static void Op1AM0 ()
{
    A_INC16 ();
}

static void OpE6M1 ()
{
    long OpAddress = Direct ();
    INC8 (OpAddress);
}

static void OpE6M0 ()
{
    long OpAddress = Direct ();
    INC16 (OpAddress);
}

static void OpF6M1 ()
{
    long OpAddress = DirectIndexedX ();
    INC8 (OpAddress);
}

static void OpF6M0 ()
{
    long OpAddress = DirectIndexedX ();
    INC16 (OpAddress);
}

static void OpEEM1 ()
{
    long OpAddress = Absolute ();
    INC8 (OpAddress);
}

static void OpEEM0 ()
{
    long OpAddress = Absolute ();
    INC16 (OpAddress);
}

static void OpFEM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    INC8 (OpAddress);
}

static void OpFEM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    INC16 (OpAddress);
}

/**********************************************************************************************/
/* LDA *************************************************************************************** */
static void OpA9M1 ()
{
    Registers.AL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.AL);
}

static void OpA9M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W = *(uint16 *) CPU.PC;
#else
    Registers.A.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.A.W);
}

static void OpA5M1 ()
{
    long OpAddress = Direct ();
    LDA8 (OpAddress);
}

static void OpA5M0 ()
{
    long OpAddress = Direct ();
    LDA16 (OpAddress);
}

static void OpB5M1 ()
{
    long OpAddress = DirectIndexedX ();
    LDA8 (OpAddress);
}

static void OpB5M0 ()
{
    long OpAddress = DirectIndexedX ();
    LDA16 (OpAddress);
}

static void OpB2M1 ()
{
    long OpAddress = DirectIndirect ();
    LDA8 (OpAddress);
}

static void OpB2M0 ()
{
    long OpAddress = DirectIndirect ();
    LDA16 (OpAddress);
}

static void OpA1M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    LDA8 (OpAddress);
}

static void OpA1M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    LDA16 (OpAddress);
}

static void OpB1M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    LDA8 (OpAddress);
}

static void OpB1M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    LDA16 (OpAddress);
}

static void OpA7M1 ()
{
    long OpAddress = DirectIndirectLong ();
    LDA8 (OpAddress);
}

static void OpA7M0 ()
{
    long OpAddress = DirectIndirectLong ();
    LDA16 (OpAddress);
}

static void OpB7M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    LDA8 (OpAddress);
}

static void OpB7M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    LDA16 (OpAddress);
}

static void OpADM1 ()
{
    long OpAddress = Absolute ();
    LDA8 (OpAddress);
}

static void OpADM0 ()
{
    long OpAddress = Absolute ();
    LDA16 (OpAddress);
}

static void OpBDM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LDA8 (OpAddress);
}

static void OpBDM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LDA16 (OpAddress);
}

static void OpB9M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    LDA8 (OpAddress);
}

static void OpB9M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    LDA16 (OpAddress);
}

static void OpAFM1 ()
{
    long OpAddress = AbsoluteLong ();
    LDA8 (OpAddress);
}

static void OpAFM0 ()
{
    long OpAddress = AbsoluteLong ();
    LDA16 (OpAddress);
}

static void OpBFM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    LDA8 (OpAddress);
}

static void OpBFM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    LDA16 (OpAddress);
}

static void OpA3M1 ()
{
    long OpAddress = StackRelative ();
    LDA8 (OpAddress);
}

static void OpA3M0 ()
{
    long OpAddress = StackRelative ();
    LDA16 (OpAddress);
}

static void OpB3M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    LDA8 (OpAddress);
}

static void OpB3M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    LDA16 (OpAddress);
}

/**********************************************************************************************/

/* LDX *************************************************************************************** */
static void OpA2X1 ()
{
    Registers.XL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.XL);
}

static void OpA2X0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.X.W = *(uint16 *) CPU.PC;
#else
    Registers.X.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.X.W);
}

static void OpA6X1 ()
{
    long OpAddress = Direct ();
    LDX8 (OpAddress);
}

static void OpA6X0 ()
{
    long OpAddress = Direct ();
    LDX16 (OpAddress);
}

static void OpB6X1 ()
{
    long OpAddress = DirectIndexedY ();
    LDX8 (OpAddress);
}

static void OpB6X0 ()
{
    long OpAddress = DirectIndexedY ();
    LDX16 (OpAddress);
}

static void OpAEX1 ()
{
    long OpAddress = Absolute ();
    LDX8 (OpAddress);
}

static void OpAEX0 ()
{
    long OpAddress = Absolute ();
    LDX16 (OpAddress);
}

static void OpBEX1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    LDX8 (OpAddress);
}

static void OpBEX0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    LDX16 (OpAddress);
}
/**********************************************************************************************/

/* LDY *************************************************************************************** */
static void OpA0X1 ()
{
    Registers.YL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.YL);
}

static void OpA0X0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.Y.W = *(uint16 *) CPU.PC;
#else
    Registers.Y.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.Y.W);
}

static void OpA4X1 ()
{
    long OpAddress = Direct ();
    LDY8 (OpAddress);
}

static void OpA4X0 ()
{
    long OpAddress = Direct ();
    LDY16 (OpAddress);
}

static void OpB4X1 ()
{
    long OpAddress = DirectIndexedX ();
    LDY8 (OpAddress);
}

static void OpB4X0 ()
{
    long OpAddress = DirectIndexedX ();
    LDY16 (OpAddress);
}

static void OpACX1 ()
{
    long OpAddress = Absolute ();
    LDY8 (OpAddress);
}

static void OpACX0 ()
{
    long OpAddress = Absolute ();
    LDY16 (OpAddress);
}

static void OpBCX1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LDY8 (OpAddress);
}

static void OpBCX0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LDY16 (OpAddress);
}
/**********************************************************************************************/

/* LSR *************************************************************************************** */
static void Op4AM1 ()
{
    A_LSR8 ();
}

static void Op4AM0 ()
{
    A_LSR16 ();
}

static void Op46M1 ()
{
    long OpAddress = Direct ();
    LSR8 (OpAddress);
}

static void Op46M0 ()
{
    long OpAddress = Direct ();
    LSR16 (OpAddress);
}

static void Op56M1 ()
{
    long OpAddress = DirectIndexedX ();
    LSR8 (OpAddress);
}

static void Op56M0 ()
{
    long OpAddress = DirectIndexedX ();
    LSR16 (OpAddress);
}

static void Op4EM1 ()
{
    long OpAddress = Absolute ();
    LSR8 (OpAddress);
}

static void Op4EM0 ()
{
    long OpAddress = Absolute ();
    LSR16 (OpAddress);
}

static void Op5EM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LSR8 (OpAddress);
}

static void Op5EM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    LSR16 (OpAddress);
}

/**********************************************************************************************/

/* ORA *************************************************************************************** */
static void Op09M1 ()
{
    Registers.AL |= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SETZN8 (Registers.AL);
}

static void Op09M0 ()
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W |= *(uint16 *) CPU.PC;
#else
    Registers.A.W |= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SETZN16 (Registers.A.W);
}

static void Op05M1 ()
{
    long OpAddress = Direct ();
    ORA8 (OpAddress);
}

static void Op05M0 ()
{
    long OpAddress = Direct ();
    ORA16 (OpAddress);
}

static void Op15M1 ()
{
    long OpAddress = DirectIndexedX ();
    ORA8 (OpAddress);
}

static void Op15M0 ()
{
    long OpAddress = DirectIndexedX ();
    ORA16 (OpAddress);
}

static void Op12M1 ()
{
    long OpAddress = DirectIndirect ();
    ORA8 (OpAddress);
}

static void Op12M0 ()
{
    long OpAddress = DirectIndirect ();
    ORA16 (OpAddress);
}

static void Op01M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    ORA8 (OpAddress);
}

static void Op01M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    ORA16 (OpAddress);
}

static void Op11M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    ORA8 (OpAddress);
}

static void Op11M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    ORA16 (OpAddress);
}

static void Op07M1 ()
{
    long OpAddress = DirectIndirectLong ();
    ORA8 (OpAddress);
}

static void Op07M0 ()
{
    long OpAddress = DirectIndirectLong ();
    ORA16 (OpAddress);
}

static void Op17M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    ORA8 (OpAddress);
}

static void Op17M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    ORA16 (OpAddress);
}

static void Op0DM1 ()
{
    long OpAddress = Absolute ();
    ORA8 (OpAddress);
}

static void Op0DM0 ()
{
    long OpAddress = Absolute ();
    ORA16 (OpAddress);
}

static void Op1DM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ORA8 (OpAddress);
}

static void Op1DM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ORA16 (OpAddress);
}

static void Op19M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    ORA8 (OpAddress);
}

static void Op19M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    ORA16 (OpAddress);
}

static void Op0FM1 ()
{
    long OpAddress = AbsoluteLong ();
    ORA8 (OpAddress);
}

static void Op0FM0 ()
{
    long OpAddress = AbsoluteLong ();
    ORA16 (OpAddress);
}

static void Op1FM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    ORA8 (OpAddress);
}

static void Op1FM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    ORA16 (OpAddress);
}

static void Op03M1 ()
{
    long OpAddress = StackRelative ();
    ORA8 (OpAddress);
}

static void Op03M0 ()
{
    long OpAddress = StackRelative ();
    ORA16 (OpAddress);
}

static void Op13M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    ORA8 (OpAddress);
}

static void Op13M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    ORA16 (OpAddress);
}

/**********************************************************************************************/

/* ROL *************************************************************************************** */
static void Op2AM1 ()
{
    A_ROL8 ();
}

static void Op2AM0 ()
{
    A_ROL16 ();
}

static void Op26M1 ()
{
    long OpAddress = Direct ();
    ROL8 (OpAddress);
}

static void Op26M0 ()
{
    long OpAddress = Direct ();
    ROL16 (OpAddress);
}

static void Op36M1 ()
{
    long OpAddress = DirectIndexedX ();
    ROL8 (OpAddress);
}

static void Op36M0 ()
{
    long OpAddress = DirectIndexedX ();
    ROL16 (OpAddress);
}

static void Op2EM1 ()
{
    long OpAddress = Absolute ();
    ROL8 (OpAddress);
}

static void Op2EM0 ()
{
    long OpAddress = Absolute ();
    ROL16 (OpAddress);
}

static void Op3EM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ROL8 (OpAddress);
}

static void Op3EM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ROL16 (OpAddress);
}
/**********************************************************************************************/

/* ROR *************************************************************************************** */
static void Op6AM1 ()
{
    A_ROR8 ();
}

static void Op6AM0 ()
{
    A_ROR16 ();
}

static void Op66M1 ()
{
    long OpAddress = Direct ();
    ROR8 (OpAddress);
}

static void Op66M0 ()
{
    long OpAddress = Direct ();
    ROR16 (OpAddress);
}

static void Op76M1 ()
{
    long OpAddress = DirectIndexedX ();
    ROR8 (OpAddress);
}

static void Op76M0 ()
{
    long OpAddress = DirectIndexedX ();
    ROR16 (OpAddress);
}

static void Op6EM1 ()
{
    long OpAddress = Absolute ();
    ROR8 (OpAddress);
}

static void Op6EM0 ()
{
    long OpAddress = Absolute ();
    ROR16 (OpAddress);
}

static void Op7EM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ROR8 (OpAddress);
}

static void Op7EM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    ROR16 (OpAddress);
}
/**********************************************************************************************/

/* SBC *************************************************************************************** */
static void OpE9M1 ()
{
    long OpAddress = Immediate8 ();
    SBC8 (OpAddress);
}

static void OpE9M0 ()
{
    long OpAddress = Immediate16 ();
    SBC16 (OpAddress);
}

static void OpE5M1 ()
{
    long OpAddress = Direct ();
    SBC8 (OpAddress);
}

static void OpE5M0 ()
{
    long OpAddress = Direct ();
    SBC16 (OpAddress);
}

static void OpF5M1 ()
{
    long OpAddress = DirectIndexedX ();
    SBC8 (OpAddress);
}

static void OpF5M0 ()
{
    long OpAddress = DirectIndexedX ();
    SBC16 (OpAddress);
}

static void OpF2M1 ()
{
    long OpAddress = DirectIndirect ();
    SBC8 (OpAddress);
}

static void OpF2M0 ()
{
    long OpAddress = DirectIndirect ();
    SBC16 (OpAddress);
}

static void OpE1M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    SBC8 (OpAddress);
}

static void OpE1M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    SBC16 (OpAddress);
}

static void OpF1M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    SBC8 (OpAddress);
}

static void OpF1M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    SBC16 (OpAddress);
}

static void OpE7M1 ()
{
    long OpAddress = DirectIndirectLong ();
    SBC8 (OpAddress);
}

static void OpE7M0 ()
{
    long OpAddress = DirectIndirectLong ();
    SBC16 (OpAddress);
}

static void OpF7M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    SBC8 (OpAddress);
}

static void OpF7M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    SBC16 (OpAddress);
}

static void OpEDM1 ()
{
    long OpAddress = Absolute ();
    SBC8 (OpAddress);
}

static void OpEDM0 ()
{
    long OpAddress = Absolute ();
    SBC16 (OpAddress);
}

static void OpFDM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    SBC8 (OpAddress);
}

static void OpFDM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    SBC16 (OpAddress);
}

static void OpF9M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    SBC8 (OpAddress);
}

static void OpF9M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    SBC16 (OpAddress);
}

static void OpEFM1 ()
{
    long OpAddress = AbsoluteLong ();
    SBC8 (OpAddress);
}

static void OpEFM0 ()
{
    long OpAddress = AbsoluteLong ();
    SBC16 (OpAddress);
}

static void OpFFM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    SBC8 (OpAddress);
}

static void OpFFM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    SBC16 (OpAddress);
}

static void OpE3M1 ()
{
    long OpAddress = StackRelative ();
    SBC8 (OpAddress);
}

static void OpE3M0 ()
{
    long OpAddress = StackRelative ();
    SBC16 (OpAddress);
}

static void OpF3M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    SBC8 (OpAddress);
}

static void OpF3M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    SBC16 (OpAddress);
}
/**********************************************************************************************/

/* STA *************************************************************************************** */
static void Op85M1 ()
{
    long OpAddress = Direct ();
    STA8 (OpAddress);
}

static void Op85M0 ()
{
    long OpAddress = Direct ();
    STA16 (OpAddress);
}

static void Op95M1 ()
{
    long OpAddress = DirectIndexedX ();
    STA8 (OpAddress);
}

static void Op95M0 ()
{
    long OpAddress = DirectIndexedX ();
    STA16 (OpAddress);
}

static void Op92M1 ()
{
    long OpAddress = DirectIndirect ();
    STA8 (OpAddress);
}

static void Op92M0 ()
{
    long OpAddress = DirectIndirect ();
    STA16 (OpAddress);
}

static void Op81M1 ()
{
    long OpAddress = DirectIndexedIndirect ();
    STA8 (OpAddress);
#ifdef noVAR_CYCLES
    if (CheckIndex ())
	CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op81M0 ()
{
    long OpAddress = DirectIndexedIndirect ();
    STA16 (OpAddress);
#ifdef noVAR_CYCLES
    if (CheckIndex ())
	CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op91M1 ()
{
    long OpAddress = DirectIndirectIndexed ();
    STA8 (OpAddress);
}

static void Op91M0 ()
{
    long OpAddress = DirectIndirectIndexed ();
    STA16 (OpAddress);
}

static void Op87M1 ()
{
    long OpAddress = DirectIndirectLong ();
    STA8 (OpAddress);
}

static void Op87M0 ()
{
    long OpAddress = DirectIndirectLong ();
    STA16 (OpAddress);
}

static void Op97M1 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    STA8 (OpAddress);
}

static void Op97M0 ()
{
    long OpAddress = DirectIndirectIndexedLong ();
    STA16 (OpAddress);
}

static void Op8DM1 ()
{
    long OpAddress = Absolute ();
    STA8 (OpAddress);
}

static void Op8DM0 ()
{
    long OpAddress = Absolute ();
    STA16 (OpAddress);
}

static void Op9DM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    STA8 (OpAddress);
}

static void Op9DM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    STA16 (OpAddress);
}

static void Op99M1 ()
{
    long OpAddress = AbsoluteIndexedY ();
    STA8 (OpAddress);
}

static void Op99M0 ()
{
    long OpAddress = AbsoluteIndexedY ();
    STA16 (OpAddress);
}

static void Op8FM1 ()
{
    long OpAddress = AbsoluteLong ();
    STA8 (OpAddress);
}

static void Op8FM0 ()
{
    long OpAddress = AbsoluteLong ();
    STA16 (OpAddress);
}

static void Op9FM1 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    STA8 (OpAddress);
}

static void Op9FM0 ()
{
    long OpAddress = AbsoluteLongIndexedX ();
    STA16 (OpAddress);
}

static void Op83M1 ()
{
    long OpAddress = StackRelative ();
    STA8 (OpAddress);
}

static void Op83M0 ()
{
    long OpAddress = StackRelative ();
    STA16 (OpAddress);
}

static void Op93M1 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    STA8 (OpAddress);
}

static void Op93M0 ()
{
    long OpAddress = StackRelativeIndirectIndexed ();
    STA16 (OpAddress);
}
/**********************************************************************************************/

/* STX *************************************************************************************** */
static void Op86X1 ()
{
    long OpAddress = Direct ();
    STX8 (OpAddress);
}

static void Op86X0 ()
{
    long OpAddress = Direct ();
    STX16 (OpAddress);
}

static void Op96X1 ()
{
    long OpAddress = DirectIndexedY ();
    STX8 (OpAddress);
}

static void Op96X0 ()
{
    long OpAddress = DirectIndexedY ();
    STX16 (OpAddress);
}

static void Op8EX1 ()
{
    long OpAddress = Absolute ();
    STX8 (OpAddress);
}

static void Op8EX0 ()
{
    long OpAddress = Absolute ();
    STX16 (OpAddress);
}
/**********************************************************************************************/

/* STY *************************************************************************************** */
static void Op84X1 ()
{
    long OpAddress = Direct ();
    STY8 (OpAddress);
}

static void Op84X0 ()
{
    long OpAddress = Direct ();
    STY16 (OpAddress);
}

static void Op94X1 ()
{
    long OpAddress = DirectIndexedX ();
    STY8 (OpAddress);
}

static void Op94X0 ()
{
    long OpAddress = DirectIndexedX ();
    STY16 (OpAddress);
}

static void Op8CX1 ()
{
    long OpAddress = Absolute ();
    STY8 (OpAddress);
}

static void Op8CX0 ()
{
    long OpAddress = Absolute ();
    STY16 (OpAddress);
}
/**********************************************************************************************/

/* STZ *************************************************************************************** */
static void Op64M1 ()
{
    long OpAddress = Direct ();
    STZ8 (OpAddress);
}

static void Op64M0 ()
{
    long OpAddress = Direct ();
    STZ16 (OpAddress);
}

static void Op74M1 ()
{
    long OpAddress = DirectIndexedX ();
    STZ8 (OpAddress);
}

static void Op74M0 ()
{
    long OpAddress = DirectIndexedX ();
    STZ16 (OpAddress);
}

static void Op9CM1 ()
{
    long OpAddress = Absolute ();
    STZ8 (OpAddress);
}

static void Op9CM0 ()
{
    long OpAddress = Absolute ();
    STZ16 (OpAddress);
}

static void Op9EM1 ()
{
    long OpAddress = AbsoluteIndexedX ();
    STZ8 (OpAddress);
}

static void Op9EM0 ()
{
    long OpAddress = AbsoluteIndexedX ();
    STZ16 (OpAddress);
}

/**********************************************************************************************/

/* TRB *************************************************************************************** */
static void Op14M1 ()
{
    long OpAddress = Direct ();
    TRB8 (OpAddress);
}

static void Op14M0 ()
{
    long OpAddress = Direct ();
    TRB16 (OpAddress);
}

static void Op1CM1 ()
{
    long OpAddress = Absolute ();
    TRB8 (OpAddress);
}

static void Op1CM0 ()
{
    long OpAddress = Absolute ();
    TRB16 (OpAddress);
}
/**********************************************************************************************/

/* TSB *************************************************************************************** */
static void Op04M1 ()
{
    long OpAddress = Direct ();
    TSB8 (OpAddress);
}

static void Op04M0 ()
{
    long OpAddress = Direct ();
    TSB16 (OpAddress);
}

static void Op0CM1 ()
{
    long OpAddress = Absolute ();
    TSB8 (OpAddress);
}

static void Op0CM0 ()
{
    long OpAddress = Absolute ();
    TSB16 (OpAddress);
}

/**********************************************************************************************/

/* Branch Instructions *********************************************************************** */
#ifndef SA1_OPCODES
#define BranchCheck0()\
    if (CPU.BranchSkip)\
    {\
		CPU.BranchSkip = FALSE;\
		if (!Settings.SoundSkipMethod)\
			if( CPU.PC - CPU.PCBase > OpAddress)\
			    return;\
    }

#define BranchCheck1()\
    if (CPU.BranchSkip)\
    {\
		CPU.BranchSkip = FALSE;\
		if (!Settings.SoundSkipMethod) {\
			if( CPU.PC - CPU.PCBase > OpAddress)\
			    return;\
		} else if (Settings.SoundSkipMethod == 1) {\
			return;\
		} else if (Settings.SoundSkipMethod == 3) {\
			if( CPU.PC - CPU.PCBase > OpAddress)\
			    return;\
			else\
				CPU.PC = CPU.PCBase + OpAddress;\
		}\
	}

#define BranchCheck2()\
    if (CPU.BranchSkip)\
    {\
		CPU.BranchSkip = FALSE;\
		if (!Settings.SoundSkipMethod) {\
			if( CPU.PC - CPU.PCBase > OpAddress)\
			    return;\
		} else if (Settings.SoundSkipMethod == 1) {\
			CPU.PC = CPU.PCBase + OpAddress;\
		} else if (Settings.SoundSkipMethod == 3) {\
			if (CPU.PC - CPU.PCBase > OpAddress)\
			    return;\
			else\
				CPU.PC = CPU.PCBase + OpAddress;\
		}\
    }

#else
#define BranchCheck0()
#define BranchCheck1()
#define BranchCheck2()
#endif

#ifdef CPU_SHUTDOWN
#ifndef SA1_OPCODES
INLINE void CPUShutdown()
{
    if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
    {
	// Don't skip cycles with a pending NMI or IRQ - could cause delayed
	// interrupt. Interrupts are delayed for a few cycles alreadybut
	// the delay could allow the shutdown code to cycle skip again.
	// Was causing screen flashing on Top Gear 3000.

	if (CPU.WaitCounter == 0 && 
	    !(CPU.Flags & (IRQ_PENDING_FLAG | NMI_FLAG)))
	{
	    CPU.WaitAddress = NULL;
#ifdef USE_SA1
	    if (Settings.SA1) {
	    	S9xSA1ExecuteDuringSleep ();
	    }
#endif
	    ApuSync();
	}
	else
	if (CPU.WaitCounter >= 2)
	    CPU.WaitCounter = 1;
	else
	    CPU.WaitCounter--;
    }
}
#else
INLINE void CPUShutdown()
{
    if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
    {
	if (CPU.WaitCounter >= 1)
	{
	    SA1.Executing = FALSE;
	    SA1.CPUExecuting = FALSE;
	}
	else
	    CPU.WaitCounter++;
    }
}
#endif
#else
#define CPUShutdown()
#endif

/* BCC */
static void Op90 ()
{
    long OpAddress = Relative ();
    BranchCheck0 ();
    if (!CheckCarry())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BCS */
static void OpB0 ()
{
    long OpAddress = Relative ();
    BranchCheck0 ();
    if (CheckCarry())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BEQ */
static void OpF0 ()
{
    long OpAddress = Relative ();
    BranchCheck2 ();
    if (CheckZero())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BMI */
static void Op30 ()
{
    long OpAddress = Relative ();
    BranchCheck1 ();
    if (CheckNegative())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BNE */
static void OpD0 ()
{
    long OpAddress = Relative ();
    BranchCheck1 ();
    if (!CheckZero())
    {
	CPU.PC = CPU.PCBase + OpAddress;

#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BPL */
static void Op10 ()
{
    long OpAddress = Relative ();
    BranchCheck1 ();
    if (!CheckNegative())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BRA */
static void Op80 ()
{
    long OpAddress = Relative ();
    CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
    CPU.Cycles++;
#endif
#endif
    CPUShutdown();
}

/* BVC */
static void Op50 ()
{
    long OpAddress = Relative ();
    BranchCheck0 ();
    if (!CheckOverflow())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}

/* BVS */
static void Op70 ()
{
    long OpAddress = Relative ();
    BranchCheck0 ();
    if (CheckOverflow())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown();
    }
}
/**********************************************************************************************/

/* ClearFlag Instructions ******************************************************************** */
/* CLC */
static void Op18 ()
{
    ClearCarry ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLD */
static void OpD8 ()
{
    ClearDecimal ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLI */
static void Op58 ()
{
    ClearIRQ ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
/*    CHECK_FOR_IRQ(); */
}

/* CLV */
static void OpB8 ()
{
    ClearOverflow();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* DEX/DEY *********************************************************************************** */
static void OpCAX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.XL--;
    SETZN8 (Registers.XL);
}

static void OpCAX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.X.W--;
    SETZN16 (Registers.X.W);
}

static void Op88X1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.YL--;
    SETZN8 (Registers.YL);
}

static void Op88X0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.Y.W--;
    SETZN16 (Registers.Y.W);
}
/**********************************************************************************************/

/* INX/INY *********************************************************************************** */
static void OpE8X1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.XL++;
    SETZN8 (Registers.XL);
}

static void OpE8X0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.X.W++;
    SETZN16 (Registers.X.W);
}

static void OpC8X1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.YL++;
    SETZN8 (Registers.YL);
}

static void OpC8X0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.Y.W++;
    SETZN16 (Registers.Y.W);
}

/**********************************************************************************************/

/* NOP *************************************************************************************** */
static void OpEA ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif

}
/**********************************************************************************************/

/* PUSH Instructions ************************************************************************* */
#define PUSHW(w) \
    S9xSetWord (w, Registers.S.W - 1);\
    Registers.S.W -= 2;
#define PUSHB(b)\
    S9xSetByte (b, Registers.S.W--);

static void OpF4 ()
{
    long OpAddress = Absolute ();
    PUSHW (OpAddress);
}

static void OpD4 ()
{
    long OpAddress = DirectIndirect ();
    PUSHW (OpAddress);
}

static void Op62 ()
{
    long OpAddress = RelativeLong ();
    PUSHW (OpAddress);
}

static void Op48M1 ()
{
    PUSHB (Registers.AL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op48M0 ()
{
    PUSHW (Registers.A.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op8B ()
{
    PUSHB (Registers.DB);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op0B ()
{
    PUSHW (Registers.D.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op4B ()
{
    PUSHB (Registers.PB);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op08 ()
{
    S9xPackStatus ();
    PUSHB (Registers.PL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX1 ()
{
    PUSHB (Registers.XL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX0 ()
{
    PUSHW (Registers.X.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX1 ()
{
    PUSHB (Registers.YL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX0 ()
{
    PUSHW (Registers.Y.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* PULL Instructions ************************************************************************* */
#define PullW(w) \
	w = S9xGetWord (Registers.S.W + 1); \
	Registers.S.W += 2;

#define PullB(b)\
	b = S9xGetByte (++Registers.S.W);

static void Op68M1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.AL);
    SETZN8 (Registers.AL);
}

static void Op68M0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.A.W);
    SETZN16 (Registers.A.W);
}

static void OpAB ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.DB);
    SETZN8 (Registers.DB);
    ICPU.ShiftedDB = (Registers.DB & 0xff) << 16;
}

/* PHP */
static void Op2B ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.D.W);
    SETZN16 (Registers.D.W);
}

/* PLP */
static void Op28 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.PL);
    S9xUnpackStatus ();

    if (CheckIndex())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
/*     CHECK_FOR_IRQ();*/
}

static void OpFAX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.XL);
    SETZN8 (Registers.XL);
}

static void OpFAX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.X.W);
    SETZN16 (Registers.X.W);
}

static void Op7AX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.YL);
    SETZN8 (Registers.YL);
}

static void Op7AX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.Y.W);
    SETZN16 (Registers.Y.W);
}

/**********************************************************************************************/

/* SetFlag Instructions ********************************************************************** */
/* SEC */
static void Op38 ()
{
    SetCarry();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* SED */
static void OpF8 ()
{
    SetDecimal();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    missing.decimal_mode = 1;
}

/* SEI */
static void Op78 ()
{
    SetIRQ ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* Transfer Instructions ********************************************************************* */
/* TAX8 */
static void OpAAX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.AL;
    SETZN8 (Registers.XL);
}

/* TAX16 */
static void OpAAX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.A.W;
    SETZN16 (Registers.X.W);
}

/* TAY8 */
static void OpA8X1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.YL = Registers.AL;
    SETZN8 (Registers.YL);
}

/* TAY16 */
static void OpA8X0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.Y.W = Registers.A.W;
    SETZN16 (Registers.Y.W);
}

static void Op5B ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.D.W = Registers.A.W;
    SETZN16 (Registers.D.W);
}

static void Op1B ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.S.W = Registers.A.W;
    if (CheckEmulation())
	Registers.SH = 1;
}

static void Op7B ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.D.W;
    SETZN16 (Registers.A.W);
}

static void Op3B ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.S.W;
    SETZN16 (Registers.A.W);
}

static void OpBAX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.SL;
    SETZN8 (Registers.XL);
}

static void OpBAX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.S.W;
    SETZN16 (Registers.X.W);
}

static void Op8AM1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.AL = Registers.XL;
    SETZN8 (Registers.AL);
}

static void Op8AM0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.X.W;
    SETZN16 (Registers.A.W);
}

static void Op9A ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.S.W = Registers.X.W;
    if (CheckEmulation())
	Registers.SH = 1;
}

static void Op9BX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.YL = Registers.XL;
    SETZN8 (Registers.YL);
}

static void Op9BX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.Y.W = Registers.X.W;
    SETZN16 (Registers.Y.W);
}

static void Op98M1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.AL = Registers.YL;
    SETZN8 (Registers.AL);
}

static void Op98M0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.Y.W;
    SETZN16 (Registers.A.W);
}

static void OpBBX1 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.YL;
    SETZN8 (Registers.XL);
}

static void OpBBX0 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.Y.W;
    SETZN16 (Registers.X.W);
}

/**********************************************************************************************/

/* XCE *************************************************************************************** */
static void OpFB ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif

    uint8 A1 = (ICPU._Carry & 0xff);
    uint8 A2 = Registers.PH;
    ICPU._Carry = A2 & 1;
    Registers.PH = A1;

    if (CheckEmulation())
    {
	SetFlags(MemoryFlag | IndexFlag);
	Registers.SH = 1;
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
}
/**********************************************************************************************/

/* BRK *************************************************************************************** */
static void Op00 ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** BRK");
#endif

#ifndef SA1_OPCODES
    CPU.BRKTriggered = TRUE;
#endif

    if (!CheckEmulation())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase + 1);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE6));
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFFE));
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* BRL ************************************************************************************** */
static void Op82 ()
{
    long OpAddress = RelativeLong ();
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
}
/**********************************************************************************************/

/* IRQ *************************************************************************************** */
void S9xOpcode_IRQ ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** IRQ");
#endif
    if (!CheckEmulation())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEE));
#endif
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFE));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}

/**********************************************************************************************/

/* NMI *************************************************************************************** */
void S9xOpcode_NMI ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** NMI");
#endif
    if (!CheckEmulation())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEA));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFA));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* COP *************************************************************************************** */
static void Op02 ()
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** COP");
#endif	
    if (!CheckEmulation())
    {
	PUSHB (Registers.PB);
	PUSHW (CPU.PC - CPU.PCBase + 1);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE4));
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PUSHW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PUSHB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFF4));
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* JML *************************************************************************************** */
static void OpDC ()
{
    long OpAddress = AbsoluteIndirectLong ();
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}

static void Op5C ()
{
    long OpAddress = AbsoluteLong ();
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
}
/**********************************************************************************************/

/* JMP *************************************************************************************** */
static void Op4C ()
{
    long OpAddress = Absolute ();
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
#if defined(CPU_SHUTDOWN) && defined(SA1_OPCODES)
    CPUShutdown();
#endif
}

static void Op6C ()
{
    long OpAddress = AbsoluteIndirect ();
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
}

static void Op7C ()
{
    long OpAddress = AbsoluteIndexedIndirect ();
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* JSL/RTL *********************************************************************************** */
static void Op22 ()
{
    long OpAddress = AbsoluteLong ();
    PUSHB (Registers.PB);
    PUSHW (CPU.PC - CPU.PCBase - 1);
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
}

static void Op6B ()
{
    PullW (Registers.PC);
    PullB (Registers.PB);
    ICPU.ShiftedPB = (Registers.PB & 0xff) << 16;
    S9xSetPCBase (ICPU.ShiftedPB + ((Registers.PC + 1) & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* JSR/RTS *********************************************************************************** */
static void Op20 ()
{
    long OpAddress = Absolute ();
    PUSHW (CPU.PC - CPU.PCBase - 1);
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpFC ()
{
    long OpAddress = AbsoluteIndexedIndirect ();
    PUSHW (CPU.PC - CPU.PCBase - 1);
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op60 ()
{
    PullW (Registers.PC);
    S9xSetPCBase (ICPU.ShiftedPB + ((Registers.PC + 1) & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE * 3;
#endif
}

/**********************************************************************************************/

/* MVN/MVP *********************************************************************************** */
static void Op54X1 ()
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = (Registers.DB & 0xff) << 16;
    SrcBank = *CPU.PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W),
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.XL++;
    Registers.YL++;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op54X0 ()
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = (Registers.DB & 0xff) << 16;
    SrcBank = *CPU.PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W),
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.X.W++;
    Registers.Y.W++;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op44X1 ()
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = (Registers.DB & 0xff) << 16;
    SrcBank = *CPU.PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W),
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.XL--;
    Registers.YL--;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op44X0 ()
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = (Registers.DB & 0xff) << 16;
    SrcBank = *CPU.PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W),
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.X.W--;
    Registers.Y.W--;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

/**********************************************************************************************/

/* REP/SEP *********************************************************************************** */
static void OpC2 ()
{
    uint8 Work8 = ~*CPU.PC++;
    Registers.PL &= Work8;
    ICPU._Carry &= Work8;
    ICPU._Overflow &= (Work8 >> 6);
    ICPU._Negative &= Work8;
    ICPU._Zero |= ~Work8 & Zero;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
    if (CheckEmulation())
    {
	SetFlags(MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
/*    CHECK_FOR_IRQ(); */
}

static void OpE2 ()
{
    uint8 Work8 = *CPU.PC++;
    Registers.PL |= Work8;
    ICPU._Carry |= Work8 & 1;
    ICPU._Overflow |= (Work8 >> 6) & 1;
    ICPU._Negative |= Work8;
    if (Work8 & Zero)
	ICPU._Zero = 0;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
    if (CheckEmulation())
    {
	SetFlags(MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
}
/**********************************************************************************************/

/* XBA *************************************************************************************** */
static void OpEB ()
{
    uint8 Work8 = Registers.AL;
    Registers.AL = Registers.AH;
    Registers.AH = Work8;

    SETZN8 (Registers.AL);
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* RTI *************************************************************************************** */
static void Op40 ()
{
    PullB (Registers.PL);
    S9xUnpackStatus ();
    PullW (Registers.PC);
    if (!CheckEmulation())
    {
	PullB (Registers.PB);
	ICPU.ShiftedPB = (Registers.PB & 0xff) << 16;
    }
    else
    {
	SetFlags (MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    S9xSetPCBase (ICPU.ShiftedPB + Registers.PC);
    
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    S9xFixCycles();
/*    CHECK_FOR_IRQ(); */
}

/**********************************************************************************************/

/* STP/WAI/DB ******************************************************************************** */
// WAI
static void OpCB ()
{
    if (CPU.IRQActive)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 2;
#endif
#endif
    }
    else
    {
	CPU.WaitingForInterrupt = TRUE;
	CPU.PC--;
#ifdef CPU_SHUTDOWN
#ifndef SA1_OPCODES
	if (Settings.Shutdown)
	{
		ApuSync();
	}
#else
	if (Settings.Shutdown)
	{
	    SA1.CPUExecuting = FALSE;
	    SA1.Executing = FALSE;
	}
#endif
#endif
    }
}

// STP
static void OpDB ()
{
    CPU.PC--;
    CPU.Flags |= DEBUG_MODE_FLAG;
}

// Reserved S9xOpcode
static void Op42 ()
{
#if defined(CPU_SHUTDOWN) && !defined(SA1_OPCODES)

	CPU.WaitAddress = NULL;
	if (Settings.SA1) S9xSA1ExecuteDuringSleep ();

	ApuSync();#ifdef VAR_CYCLES
	CPU.Cycles += CPU.MemSpeed;
#endif

	//relative
	int8 b = *CPU.PC;
	CPU.PC++;

	int8 BranchOffset = ((b & 0xF) | 0xF0);
	uint16 OpAddress = (int)(CPU.PC - CPU.PCBase) + BranchOffset;

	// Assume we're going to branch
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
	CPU.Cycles++;
#endif

	switch (b >> 4) {
	case 0x1: //BPL
		BranchCheck1 ();
		if (!CheckNegative ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0x3: //BMI
		BranchCheck1 ();
		if (CheckNegative ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0x5: //BVC
		BranchCheck0 ();
		if (!CheckOverflow ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0x7: //BVS
		BranchCheck0 ();
		if (CheckOverflow ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
			}
		return;
	case 0x8: //BRA
		CPU.PC = CPU.PCBase + OpAddress;
#		ifdef VAR_CYCLES
			CPU.Cycles += ONE_CYCLE;
#		else
			CPU.Cycles++;
#		endif
		CPUShutdown ();
		return;
	case 0x9: //BCC
		BranchCheck0 ();
	    if (!CheckCarry ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0xB: //BCS
		BranchCheck0 ();
		if (CheckCarry ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0xD: //BNE
		BranchCheck1 ();
		if (!CheckZero ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
			CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
		}
		return;
	case 0xF: //BEQ
		BranchCheck2 ();
		if (CheckZero ()) {
			CPU.PC = CPU.PCBase + OpAddress;
#			ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#			else
				CPU.Cycles++;
#			endif
			CPUShutdown ();
			}
		return;
#ifdef DEBUG
	default:
		printf("Invalid Op42 branch type %hx\n", b >> 4);
		return;
#endif
	}
#endif
}

/**********************************************************************************************/

/**********************************************************************************************/
/* CPU-S9xOpcodes Definitions                                                                    */
/**********************************************************************************************/
struct SOpcodes S9xOpcodesM1X1[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X1},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X1},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X1},    {Op85M1},    {Op86X1},
    {Op87M1},    {Op88X1},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX1},    {Op8DM1},    {Op8EX1},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X1},    {Op95M1},
    {Op96X1},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX1},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X1},    {OpA1M1},    {OpA2X1},    {OpA3M1},    {OpA4X1},
    {OpA5M1},    {OpA6X1},    {OpA7M1},    {OpA8X1},    {OpA9M1},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM1},    {OpAEX1},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X1},    {OpB5M1},    {OpB6X1},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM1},
    {OpBEX1},    {OpBFM1},    {OpC0X1},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X1},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X1},    {OpC9M1},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X1},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X1},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X1},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM1X0[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X0},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X0},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X0},    {Op85M1},    {Op86X0},
    {Op87M1},    {Op88X0},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX0},    {Op8DM1},    {Op8EX0},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X0},    {Op95M1},
    {Op96X0},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX0},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X0},    {OpA1M1},    {OpA2X0},    {OpA3M1},    {OpA4X0},
    {OpA5M1},    {OpA6X0},    {OpA7M1},    {OpA8X0},    {OpA9M1},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM1},    {OpAEX0},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X0},    {OpB5M1},    {OpB6X0},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM1},
    {OpBEX0},    {OpBFM1},    {OpC0X0},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X0},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X0},    {OpC9M1},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X0},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X0},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X0},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM0X0[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X0},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X0},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X0},    {Op85M0},    {Op86X0},
    {Op87M0},    {Op88X0},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX0},    {Op8DM0},    {Op8EX0},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X0},    {Op95M0},
    {Op96X0},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX0},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X0},    {OpA1M0},    {OpA2X0},    {OpA3M0},    {OpA4X0},
    {OpA5M0},    {OpA6X0},    {OpA7M0},    {OpA8X0},    {OpA9M0},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM0},    {OpAEX0},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X0},    {OpB5M0},    {OpB6X0},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM0},
    {OpBEX0},    {OpBFM0},    {OpC0X0},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X0},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X0},    {OpC9M0},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X0},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X0},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X0},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};

struct SOpcodes S9xOpcodesM0X1[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X1},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X1},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X1},    {Op85M0},    {Op86X1},
    {Op87M0},    {Op88X1},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX1},    {Op8DM0},    {Op8EX1},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X1},    {Op95M0},
    {Op96X1},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX1},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X1},    {OpA1M0},    {OpA2X1},    {OpA3M0},    {OpA4X1},
    {OpA5M0},    {OpA6X1},    {OpA7M0},    {OpA8X1},    {OpA9M0},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM0},    {OpAEX1},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X1},    {OpB5M0},    {OpB6X1},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM0},
    {OpBEX1},    {OpBFM0},    {OpC0X1},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X1},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X1},    {OpC9M0},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X1},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X1},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X1},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};

