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
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
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
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _CPUMACRO_H_
#define _CPUMACRO_H_

#define SETZN16(W) \
    ICPU._Zero = (W) != 0; \
    ICPU._Negative = (uint8) ((W) >> 8);

#define SETZN8(W) \
    ICPU._Zero = (W); \
    ICPU._Negative = (W);

STATIC INLINE void FASTCALL ADC8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    
    if (CheckDecimal())
    {
	uint8 A1 = (Registers.A.W) & 0xF;
	uint8 A2 = (Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 += W1 + CheckCarry();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if (~(Registers.AL ^ Work8) &
	    (Work8 ^ Ans8) & 0x80)
	     SetOverflow ();
	else
	    ClearOverflow();
	Registers.AL = Ans8;
	SETZN8 (Registers.AL);
    }
    else
    {
	uint16 Ans16 = Registers.AL + Work8 + CheckCarry();

	ICPU._Carry = Ans16 >= 0x100;

	if (~(Registers.AL ^ Work8) & 
	     (Work8 ^ (uint8) Ans16) & 0x80)
	    SetOverflow();
	else
	    ClearOverflow();
	Registers.AL = (uint8) Ans16;
	SETZN8 (Registers.AL);

    }
}

STATIC INLINE void FASTCALL ADC16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    if (CheckDecimal())
    {
	uint8 A1 = (Registers.A.W) & 0xF;
	uint8 A2 = (Registers.A.W >> 4) & 0xF;
	uint8 A3 = (Registers.A.W >> 8) & 0xF;
	uint8 A4 = (Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 += W1 + CheckCarry ();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    A3++;
	}

	A3 += W3;
	if (A3 > 9)
	{
	    A3 -= 10;
	    A4++;
	}

	A4 += W4;
	if (A4 > 9)
	{
	    A4 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if (~(Registers.A.W ^ Work16) &
	    (Work16 ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	Registers.A.W = Ans16;
	SETZN16 (Registers.A.W);
    }
    else
    {
	uint32 Ans32 = Registers.A.W + Work16 + CheckCarry();

	ICPU._Carry = Ans32 >= 0x10000;

	if (~(Registers.A.W ^ Work16) &
	    (Work16 ^ (uint16) Ans32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	Registers.A.W = (uint16) Ans32;
	SETZN16 (Registers.A.W);
    }
}

STATIC INLINE void FASTCALL AND16 (long OpAddress)
{
    Registers.A.W &= S9xGetWord (OpAddress);
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL AND8 (long OpAddress)
{
    Registers.AL &= S9xGetByte (OpAddress);
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL A_ASL16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    ICPU._Carry = (Registers.AH & 0x80) != 0;
    Registers.A.W <<= 1;
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL A_ASL8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    ICPU._Carry = (Registers.AL & 0x80) != 0;
    Registers.AL <<= 1;
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL ASL16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    ICPU._Carry = (Work16 & 0x8000) != 0;
    Work16 <<= 1;
    S9xSetWord (Work16, OpAddress);
    SETZN16 (Work16);
}

STATIC INLINE void FASTCALL ASL8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    ICPU._Carry = (Work8 & 0x80) != 0;
    Work8 <<= 1;
    S9xSetByte (Work8, OpAddress);
    SETZN8 (Work8);
}

STATIC INLINE void FASTCALL BIT16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);
    ICPU._Overflow = (Work16 & 0x4000) != 0;
    ICPU._Negative = (uint8) (Work16 >> 8);
    ICPU._Zero = (Work16 & Registers.A.W) != 0;
}

STATIC INLINE void FASTCALL BIT8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    ICPU._Overflow = (Work8 & 0x40) != 0;
    ICPU._Negative = Work8;
    ICPU._Zero = Work8 & Registers.AL;
}

STATIC INLINE void FASTCALL CMP16 (long OpAddress)
{
    int32 Int32 = (long) Registers.A.W -
	    (long) S9xGetWord (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC INLINE void FASTCALL CMP8 (long OpAddress)
{
    int32 Int32 = (short) Registers.AL -
	    (short) S9xGetByte (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC INLINE void FASTCALL CMX16 (long OpAddress)
{
    int32 Int32 = (long) Registers.X.W -
	    (long) S9xGetWord (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC INLINE void FASTCALL CMX8 (long OpAddress)
{
    int32 Int32 = (short) Registers.XL -
	    (short) S9xGetByte (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC INLINE void FASTCALL CMY16 (long OpAddress)
{
    int32 Int32 = (long) Registers.Y.W -
	    (long) S9xGetWord (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN16 ((uint16) Int32);
}

STATIC INLINE void FASTCALL CMY8 (long OpAddress)
{
    int32 Int32 = (short) Registers.YL -
	    (short) S9xGetByte (OpAddress);
    ICPU._Carry = Int32 >= 0;
    SETZN8 ((uint8) Int32);
}

STATIC INLINE void FASTCALL A_DEC16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.A.W--;
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL A_DEC8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.AL--;
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL DEC16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress) - 1;
    S9xSetWord (Work16, OpAddress);
    SETZN16 (Work16);
}

STATIC INLINE void FASTCALL DEC8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress) - 1;
    S9xSetByte (Work8, OpAddress);
    SETZN8 (Work8);
}

STATIC INLINE void FASTCALL EOR16 (long OpAddress)
{
    Registers.A.W ^= S9xGetWord (OpAddress);
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL EOR8 (long OpAddress)
{
    Registers.AL ^= S9xGetByte (OpAddress);
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL A_INC16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.A.W++;
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL A_INC8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.AL++;
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL INC16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress) + 1;
    S9xSetWord (Work16, OpAddress);
    SETZN16 (Work16);
}

STATIC INLINE void FASTCALL INC8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress) + 1;
    S9xSetByte (Work8, OpAddress);
    SETZN8 (Work8);
}

STATIC INLINE void FASTCALL LDA16 (long OpAddress)
{
    Registers.A.W = S9xGetWord (OpAddress);
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL LDA8 (long OpAddress)
{
    Registers.AL = S9xGetByte (OpAddress);
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL LDX16 (long OpAddress)
{
    Registers.X.W = S9xGetWord (OpAddress);
    SETZN16 (Registers.X.W);
}

STATIC INLINE void FASTCALL LDX8 (long OpAddress)
{
    Registers.XL = S9xGetByte (OpAddress);
    SETZN8 (Registers.XL);
}

STATIC INLINE void FASTCALL LDY16 (long OpAddress)
{
    Registers.Y.W = S9xGetWord (OpAddress);
    SETZN16 (Registers.Y.W);
}

STATIC INLINE void FASTCALL LDY8 (long OpAddress)
{
    Registers.YL = S9xGetByte (OpAddress);
    SETZN8 (Registers.YL);
}

STATIC INLINE void FASTCALL A_LSR16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    ICPU._Carry = Registers.AL & 1;
    Registers.A.W >>= 1;
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL A_LSR8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    ICPU._Carry = Registers.AL & 1;
    Registers.AL >>= 1;
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL LSR16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    ICPU._Carry = Work16 & 1;
    Work16 >>= 1;
    S9xSetWord (Work16, OpAddress);
    SETZN16 (Work16);
}

STATIC INLINE void FASTCALL LSR8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    ICPU._Carry = Work8 & 1;
    Work8 >>= 1;
    S9xSetByte (Work8, OpAddress);
    SETZN8 (Work8);
}

STATIC INLINE void FASTCALL ORA16 (long OpAddress)
{
    Registers.A.W |= S9xGetWord (OpAddress);
    SETZN16 (Registers.A.W);
}

STATIC INLINE void FASTCALL ORA8 (long OpAddress)
{
    Registers.AL |= S9xGetByte (OpAddress);
    SETZN8 (Registers.AL);
}

STATIC INLINE void FASTCALL A_ROL16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = (Registers.A.W << 1) | CheckCarry();
    ICPU._Carry = Work32 >= 0x10000;
    Registers.A.W = (uint16) Work32;
    SETZN16 ((uint16) Work32);
}

STATIC INLINE void FASTCALL A_ROL8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = Registers.AL;
    Work16 <<= 1;
    Work16 |= CheckCarry();
    ICPU._Carry = Work16 >= 0x100;
    Registers.AL = (uint8) Work16;
    SETZN8 ((uint8) Work16);
}

STATIC INLINE void FASTCALL ROL16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress);
    Work32 <<= 1;
    Work32 |= CheckCarry();
    ICPU._Carry = Work32 >= 0x10000;
    S9xSetWord ((uint16) Work32, OpAddress);
    SETZN16 ((uint16) Work32);
}

STATIC INLINE void FASTCALL ROL8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress);
    Work16 <<= 1;
    Work16 |= CheckCarry ();
    ICPU._Carry = Work16 >= 0x100;
    S9xSetByte ((uint8) Work16, OpAddress);
    SETZN8 ((uint8) Work16);
}

STATIC INLINE void FASTCALL A_ROR16 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = Registers.A.W;
    Work32 |= (int) CheckCarry() << 16;
    ICPU._Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    Registers.A.W = (uint16) Work32;
    SETZN16 ((uint16) Work32);
}

STATIC INLINE void FASTCALL A_ROR8 ()
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = Registers.AL | ((uint16) CheckCarry() << 8);
    ICPU._Carry = (uint8) Work16 & 1;
    Work16 >>= 1;
    Registers.AL = (uint8) Work16;
    SETZN8 ((uint8) Work16);
}

STATIC INLINE void FASTCALL ROR16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress);
    Work32 |= (int) CheckCarry() << 16;
    ICPU._Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    S9xSetWord ((uint16) Work32, OpAddress);
    SETZN16 ((uint16) Work32);
}

STATIC INLINE void FASTCALL ROR8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress);
    Work16 |= (int) CheckCarry () << 8;
    ICPU._Carry = (uint8) (Work16 & 1);
    Work16 >>= 1;
    S9xSetByte ((uint8) Work16, OpAddress);
    SETZN8 ((uint8) Work16);
}

STATIC INLINE void FASTCALL SBC16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    if (CheckDecimal())
    {
	uint8 A1 = (Registers.A.W) & 0xF;
	uint8 A2 = (Registers.A.W >> 4) & 0xF;
	uint8 A3 = (Registers.A.W >> 8) & 0xF;
	uint8 A4 = (Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	A3 -= W3;
	A4 -= W4;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    A3--;
	}
	if (A3 > 9)
	{
	    A3 += 10;
	    A4--;
	}
	if (A4 > 9)
	{
	    A4 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if ((Registers.A.W ^ Work16) &
	    (Registers.A.W ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	Registers.A.W = Ans16;
	SETZN16 (Registers.A.W);
    }
    else
    {

	int32 Int32 = (long) Registers.A.W - (long) Work16 + (long) CheckCarry() - 1;

	ICPU._Carry = Int32 >= 0;

	if ((Registers.A.W ^ Work16) &
	    (Registers.A.W ^ (uint16) Int32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow ();
	Registers.A.W = (uint16) Int32;
	SETZN16 (Registers.A.W);
    }
}

STATIC INLINE void FASTCALL SBC8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    if (CheckDecimal())
    {
	uint8 A1 = (Registers.A.W) & 0xF;
	uint8 A2 = (Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if ((Registers.AL ^ Work8) &
	    (Registers.AL ^ Ans8) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	Registers.AL = Ans8;
	SETZN8 (Registers.AL);
    }
    else
    {
	int32 Int32 = (short) Registers.AL - (short) Work8 + (short) CheckCarry() - 1;

	ICPU._Carry = Int32 >= 0;
	if ((Registers.AL ^ Work8) &
	    (Registers.AL ^ (uint8) Int32) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	Registers.AL = (uint8) Int32;
	SETZN8 (Registers.AL);
    }
}

STATIC INLINE void FASTCALL STA16 (long OpAddress)
{
    S9xSetWord (Registers.A.W, OpAddress);
}

STATIC INLINE void FASTCALL STA8 (long OpAddress)
{
    S9xSetByte (Registers.AL, OpAddress);
}

STATIC INLINE void FASTCALL STX16 (long OpAddress)
{
    S9xSetWord (Registers.X.W, OpAddress);
}

STATIC INLINE void FASTCALL STX8 (long OpAddress)
{
    S9xSetByte (Registers.XL, OpAddress);
}

STATIC INLINE void FASTCALL STY16 (long OpAddress)
{
    S9xSetWord (Registers.Y.W, OpAddress);
}

STATIC INLINE void FASTCALL STY8 (long OpAddress)
{
    S9xSetByte (Registers.YL, OpAddress);
}

STATIC INLINE void FASTCALL STZ16 (long OpAddress)
{
    S9xSetWord (0, OpAddress);
}

STATIC INLINE void FASTCALL STZ8 (long OpAddress)
{
    S9xSetByte (0, OpAddress);
}

STATIC INLINE void FASTCALL TSB16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    ICPU._Zero = (Work16 & Registers.A.W) != 0;
    Work16 |= Registers.A.W;
    S9xSetWord (Work16, OpAddress);
}

STATIC INLINE void FASTCALL TSB8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    ICPU._Zero = Work8 & Registers.AL;
    Work8 |= Registers.AL;
    S9xSetByte (Work8, OpAddress);
}

STATIC INLINE void FASTCALL TRB16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    ICPU._Zero = (Work16 & Registers.A.W) != 0;
    Work16 &= ~Registers.A.W;
    S9xSetWord (Work16, OpAddress);
}

STATIC INLINE void FASTCALL TRB8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    ICPU._Zero = Work8 & Registers.AL;
    Work8 &= ~Registers.AL;
    S9xSetByte (Work8, OpAddress);
}
#endif
