/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/
/* 26.March 2000 PeT changed set_irq_line */


#include "windows.h"
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "tube.h"
#include "beebmem.h"

#define LSB_FIRST

#include "osd_cpu.h"
#include "i86.h"

/*************************************
 *
 *  Interrupt line constants
 *
 *************************************/

enum
{
	/* line states */
	CLEAR_LINE,				/* clear (a fired, held or pulsed) line */
	ASSERT_LINE1,				/* assert an interrupt immediately */
	ASSERT_LINE4,				/* assert an interrupt immediately */
	INPUT_LINE_IRQ4,
	INPUT_LINE_NMI
};

/* All pre-i286 CPUs have a 1MB address space */
#define AMASK 0xfffff

/* I86 registers */
typedef union
{									   /* eight general registers */
	UINT16 w[8];					   /* viewed as 16 bits registers */
	UINT8 b[16];					   /* or as 8 bit registers */
}
i86basicregs;

typedef struct
{
	i86basicregs regs;
	UINT32 pc;
	UINT32 prevpc;
	UINT32 base[4];
	UINT16 sregs[4];
	UINT16 flags;
	int (*irq_callback) (int irqline);
	int AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal;		/* 0 or non-0 valued flags */
	UINT8 ParityVal;
	UINT8 TF, IF;				   /* 0 or 1 valued flags */
	UINT8 MF;						   /* V30 mode flag */
	UINT8 int_vector;
	INT8 nmi_state;
	INT8 irq_state;
	INT8 test_state;	/* PJB 03/05 */
	int extra_cycles;       /* extra cycles for interrupts */
}
i86_Regs;

int i86_ICount;

static i86_Regs I;
static UINT32 prefix_base;	        /* base address of the latest prefix segment */
static UINT8 seg_prefix;		    /* prefix segment indicator */

static UINT8 parity_table[256];

/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

struct i86_timing
{
	int		id;

	UINT8	exception, iret;								/* exception, IRET */
	UINT8	int3, int_imm, into_nt, into_t;					/* INTs */
	UINT8	override;										/* segment overrides */
	UINT8	flag_ops, lahf, sahf;							/* flag operations */
	UINT8	aaa, aas, aam, aad;								/* arithmetic adjusts */
	UINT8	daa, das;										/* decimal adjusts */
	UINT8	cbw, cwd;										/* sign extension */
	UINT8	hlt, load_ptr, lea, nop, wait, xlat;			/* misc */

	UINT8	jmp_short, jmp_near, jmp_far;					/* direct JMPs */
	UINT8	jmp_r16, jmp_m16, jmp_m32;						/* indirect JMPs */
	UINT8	call_near, call_far;							/* direct CALLs */
	UINT8	call_r16, call_m16, call_m32;					/* indirect CALLs */
	UINT8	ret_near, ret_far, ret_near_imm, ret_far_imm;	/* returns */
	UINT8	jcc_nt, jcc_t, jcxz_nt, jcxz_t;					/* conditional JMPs */
	UINT8	loop_nt, loop_t, loope_nt, loope_t;				/* loops */

	UINT8	in_imm8, in_imm16, in_dx8, in_dx16;				/* port reads */
	UINT8	out_imm8, out_imm16, out_dx8, out_dx16;			/* port writes */

	UINT8	mov_rr8, mov_rm8, mov_mr8;						/* move, 8-bit */
	UINT8	mov_ri8, mov_mi8;								/* move, 8-bit immediate */
	UINT8	mov_rr16, mov_rm16, mov_mr16;					/* move, 16-bit */
	UINT8	mov_ri16, mov_mi16;								/* move, 16-bit immediate */
	UINT8	mov_am8, mov_am16, mov_ma8, mov_ma16;			/* move, AL/AX memory */
	UINT8	mov_sr, mov_sm, mov_rs, mov_ms;					/* move, segment registers */
	UINT8	xchg_rr8, xchg_rm8;								/* exchange, 8-bit */
	UINT8	xchg_rr16, xchg_rm16, xchg_ar16;				/* exchange, 16-bit */

	UINT8	push_r16, push_m16, push_seg, pushf;			/* pushes */
	UINT8	pop_r16, pop_m16, pop_seg, popf;				/* pops */

	UINT8	alu_rr8, alu_rm8, alu_mr8;						/* ALU ops, 8-bit */
	UINT8	alu_ri8, alu_mi8, alu_mi8_ro;					/* ALU ops, 8-bit immediate */
	UINT8	alu_rr16, alu_rm16, alu_mr16;					/* ALU ops, 16-bit */
	UINT8	alu_ri16, alu_mi16, alu_mi16_ro;				/* ALU ops, 16-bit immediate */
	UINT8	alu_r16i8, alu_m16i8, alu_m16i8_ro;				/* ALU ops, 16-bit w/8-bit immediate */
	UINT8	mul_r8, mul_r16, mul_m8, mul_m16;				/* MUL */
	UINT8	imul_r8, imul_r16, imul_m8, imul_m16;			/* IMUL */
	UINT8	div_r8, div_r16, div_m8, div_m16;				/* DIV */
	UINT8	idiv_r8, idiv_r16, idiv_m8, idiv_m16;			/* IDIV */
	UINT8	incdec_r8, incdec_r16, incdec_m8, incdec_m16;	/* INC/DEC */
	UINT8	negnot_r8, negnot_r16, negnot_m8, negnot_m16;	/* NEG/NOT */

	UINT8	rot_reg_1, rot_reg_base, rot_reg_bit;			/* reg shift/rotate */
	UINT8	rot_m8_1, rot_m8_base, rot_m8_bit;				/* m8 shift/rotate */
	UINT8	rot_m16_1, rot_m16_base, rot_m16_bit;			/* m16 shift/rotate */

	UINT8	cmps8, rep_cmps8_base, rep_cmps8_count;			/* CMPS 8-bit */
	UINT8	cmps16, rep_cmps16_base, rep_cmps16_count;		/* CMPS 16-bit */
	UINT8	scas8, rep_scas8_base, rep_scas8_count;			/* SCAS 8-bit */
	UINT8	scas16, rep_scas16_base, rep_scas16_count;		/* SCAS 16-bit */
	UINT8	lods8, rep_lods8_base, rep_lods8_count;			/* LODS 8-bit */
	UINT8	lods16, rep_lods16_base, rep_lods16_count;		/* LODS 16-bit */
	UINT8	stos8, rep_stos8_base, rep_stos8_count;			/* STOS 8-bit */
	UINT8	stos16, rep_stos16_base, rep_stos16_count;		/* STOS 16-bit */
	UINT8	movs8, rep_movs8_base, rep_movs8_count;			/* MOVS 8-bit */
	UINT8	movs16, rep_movs16_base, rep_movs16_count;		/* MOVS 16-bit */

	void *	check1;											/* marker to make sure we line up */

	UINT8	ins8, rep_ins8_base, rep_ins8_count;			/* (80186) INS 8-bit */
	UINT8	ins16, rep_ins16_base, rep_ins16_count;			/* (80186) INS 16-bit */
	UINT8	outs8, rep_outs8_base, rep_outs8_count;			/* (80186) OUTS 8-bit */
	UINT8	outs16, rep_outs16_base, rep_outs16_count;		/* (80186) OUTS 16-bit */
	UINT8	push_imm, pusha, popa;							/* (80186) PUSH immediate, PUSHA/POPA */
	UINT8	imul_rri8, imul_rmi8;							/* (80186) IMUL immediate 8-bit */
	UINT8	imul_rri16, imul_rmi16;							/* (80186) IMUL immediate 16-bit */
	UINT8	enter0, enter1, enter_base, enter_count, leave;	/* (80186) ENTER/LEAVE */
	UINT8	bound;											/* (80186) BOUND */

	void *	check2;											/* marker to make sure we line up */
};
//-> [INCLUDED i86time.cpp]
/* these come from the Intel 80186 datasheet */
static const struct i86_timing i186_cycles =
{
	80186,

	45,28,			/* exception, IRET */
	 0, 2, 4, 3,	/* INTs */
	 2,				/* segment overrides */
	 2, 2, 3,		/* flag operations */
	 8, 7,19,15,	/* arithmetic adjusts */
	 4, 4,			/* decimal adjusts */
	 2, 4,			/* sign extension */
	 2,18, 6, 2, 6,11,	/* misc */

	14,14,14,		/* direct JMPs */
	11,17,26,		/* indirect JMPs */
	15,23,			/* direct CALLs */
	13,19,38,		/* indirect CALLs */
	16,22,18,25,	/* returns */
	 4,13, 5,15,	/* conditional JMPs */
	 6,16, 6,16,	/* loops */

	10,10, 8, 8,	/* port reads */
	 9, 9, 7, 7,	/* port writes */

	 2, 9,12,		/* move, 8-bit */
	 3,12,			/* move, 8-bit immediate */
	 2, 9,12,		/* move, 16-bit */
	 4,13,			/* move, 16-bit immediate */
	 8, 8, 9, 9,	/* move, AL/AX memory */
	 2,11, 2,11,	/* move, segment registers */
	 4,17,			/* exchange, 8-bit */
	 4,17, 3,		/* exchange, 16-bit */

	10,16, 9, 9,	/* pushes */
	10,20, 8, 8,	/* pops */

	 3,10,10,		/* ALU ops, 8-bit */
	 4,16,10,		/* ALU ops, 8-bit immediate */
	 3,10,10,		/* ALU ops, 16-bit */
	 4,16,10,		/* ALU ops, 16-bit immediate */
	 4,16,10,		/* ALU ops, 16-bit w/8-bit immediate */
	26,35,32,41,	/* MUL */
	25,34,31,40,	/* IMUL */
	29,38,35,44,	/* DIV */
	44,53,50,59,	/* IDIV */
	 3, 3,15,15,	/* INC/DEC */
	 3, 3,10,10,	/* NEG/NOT */

	 2, 5, 1,		/* reg shift/rotate */
	15,17, 1,		/* m8 shift/rotate */
	15,17, 1,		/* m16 shift/rotate */

	22, 5,22,		/* CMPS 8-bit */
	22, 5,22,		/* CMPS 16-bit */
	15, 5,15,		/* SCAS 8-bit */
	15, 5,15,		/* SCAS 16-bit */
	12, 6,11,		/* LODS 8-bit */
	12, 6,11,		/* LODS 16-bit */
	10, 6, 9,		/* STOS 8-bit */
	10, 6, 9,		/* STOS 16-bit */
	14, 8, 8,		/* MOVS 8-bit */
	14, 8, 8,		/* MOVS 16-bit */

	(void *)-1,		/* marker to make sure we line up */

	14, 8, 8,		/* (80186) INS 8-bit */
	14, 8, 8,		/* (80186) INS 16-bit */
	14, 8, 8,		/* (80186) OUTS 8-bit */
	14, 8, 8,		/* (80186) OUTS 16-bit */
	14,68,83,		/* (80186) PUSH immediate, PUSHA/POPA */
	22,29,			/* (80186) IMUL immediate 8-bit */
	25,32,			/* (80186) IMUL immediate 16-bit */
	15,25,4,16, 8,	/* (80186) ENTER/LEAVE */
	33,				/* (80186) BOUND */

	(void *)-1		/* marker to make sure we line up */
};
//<- [END OF i85time.cpp


static struct i86_timing cycles;

extern int i386_dasm_one(char *buffer, UINT32 eip, int addr_size, int op_size);

extern unsigned char TubeintStatus;
extern unsigned char TubeNMIStatus;

void io_write_byte_8(offs_t address, UINT8 data);
UINT8 io_read_byte_8(offs_t address);

void DoDMA(void);
void MemoryDump(int addr, int count);

int dis_count = 0;
int trace_186 = 0;

UINT8 *                     opcode_base;
offs_t						opcode_mask = 0xfffff;			/* mask to apply to the opcode address */

/* ----- opcode and opcode argument reading ----- */
INLINE UINT8  cpu_readop(offs_t A)			{ return (opcode_base[(A) & opcode_mask]); }
INLINE UINT8  cpu_readop_arg(offs_t A)			{ return (opcode_base[(A) & opcode_mask]); }

#define change_pc(pc)																	\


// address_space active_address_space[ADDRESS_SPACES];/* address space data */


INLINE void program_write_byte_8(offs_t address, UINT8 data)
{
    if (address < 0xf0000)
    {
		opcode_base[address] = data;
    }
}

INLINE UINT8 program_read_byte_8(offs_t address)
{
    return opcode_base[address];
}

#include "ea.h"

#undef PREFIX
#undef PREFIX86

#define PREFIX(name) i186##name
#define PREFIX86(name) i186##name
#define PREFIX186(name) i186##name

#include "instr86.h"
//-- #include "instr86.cpp"
//-> [INCLUDED WHOLE FILE HERE]
/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

/*
 * file will be included in all cpu variants
 * put non i86 instructions in own files (i286, i386, nec)
 * function renaming will be added when neccessary
 * timing value should move to separate array
 */

void Dis186(void);

#undef ICOUNT

#define ICOUNT i86_ICount

static void PREFIX86(_interrupt)(unsigned int_num)
{
	unsigned dest_seg, dest_off;
	WORD ip = I.pc - I.base[CS];

    if (int_num == 65535)
	{
		return;
		int_num = (*I.irq_callback)(0);
	}

	dest_off = ReadWord(int_num*4);
	dest_seg = ReadWord(int_num*4+2);

	if ( (dest_off == 0) && (dest_seg == 0) ) return;

	PREFIX(_pushf());
	I.TF = I.IF = 0;
	PUSH(I.sregs[CS]);
	PUSH(ip);
	I.sregs[CS] = (WORD)dest_seg;
	I.base[CS] = SegBase(CS);
	I.pc = (I.base[CS] + dest_off) & AMASK;

    CHANGE_PC(I.pc);

//    WriteLog("Entering Interrupt 0x%02x @ ", int_num);
//    WriteLog("[%04x:%04x] AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x DS=%04x ES=%04x SS:SP = %04x:%04x", 
//        I.sregs[CS], (I.pc - I.base[CS]) & 0xffff, 
//        I.regs.w[AX], I.regs.w[BX], I.regs.w[CX], I.regs.w[DX], I.regs.w[SI], I.regs.w[DI], I.sregs[DS], I.sregs[ES],
//		I.sregs[SS], I.regs.w[SP]);
    
    I.extra_cycles += cycles.exception;
}

static void PREFIX86(_trap)(void)
{
	PREFIX(_instruction)[FETCHOP]();
	PREFIX(_interrupt)(1);
}

static void PREFIX86(_rotate_shift_Byte)(unsigned ModRM, unsigned count)
{
	unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_base : cycles.rot_m8_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_1 : cycles.rot_m8_1;

		switch (ModRM & 0x38)
		{
		case 0x00:	/* ROL eb,1 */
			I.CarryVal = src & 0x80;
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			I.OverVal = (src^dst)&0x80;
			break;
		case 0x08:	/* ROR eb,1 */
			I.CarryVal = src & 0x01;
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			I.OverVal = (src^dst)&0x80;
			break;
		case 0x10:	/* RCL eb,1 */
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			I.OverVal = (src^dst)&0x80;
			break;
		case 0x18:	/* RCR eb,1 */
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = (src^dst)&0x80;
			break;
		case 0x20:	/* SHL eb,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			I.OverVal = (src^dst)&0x80;
			I.AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x28:	/* SHR eb,1 */
			dst = src >> 1;
			PutbackRMByte(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = src & 0x80;
			I.AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x38:	/* SAR eb,1 */
			dst = ((INT8)src) >> 1;
			PutbackRMByte(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = 0;
			I.AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		}
	}
	else
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_base + cycles.rot_reg_bit : cycles.rot_m8_base + cycles.rot_m8_bit;

		switch (ModRM & 0x38)
		{
		case 0x00:	/* ROL eb,count */
			for (; count > 0; count--)
			{
				I.CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x08:	/* ROR eb,count */
			for (; count > 0; count--)
			{
				I.CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x10:	/* RCL eb,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + CF;
				SetCFB(dst);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x18:	/* RCR eb,count */
			for (; count > 0; count--)
			{
				dst = (CF<<8)+dst;
				I.CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x20:
		case 0x30:	/* SHL eb,count */
			dst <<= count;
			SetCFB(dst);
			I.AuxVal = 1;
			SetSZPF_Byte(dst);
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x28:	/* SHR eb,count */
			dst >>= count-1;
			I.CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Byte(dst);
			I.AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x38:	/* SAR eb,count */
			dst = ((INT8)dst) >> (count-1);
			I.CarryVal = dst & 0x1;
			dst = ((INT8)((BYTE)dst)) >> 1;
			SetSZPF_Byte(dst);
			I.AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		}
	}
}

static void PREFIX86(_rotate_shift_Word)(unsigned ModRM, unsigned count)
{
	unsigned src = GetRMWord(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_base : cycles.rot_m16_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_1 : cycles.rot_m16_1;

		switch (ModRM & 0x38)
		{
#if 0
		case 0x00:	/* ROL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			I.OverVal = !(!(tmp & 0x4000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x08:	/* ROR ew,1 */
			I.CarryVal = tmp & 0x01;
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			I.OverVal = !(!(tmp & 0x8000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x10:	/* RCL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			I.OverVal = (tmp ^ (tmp << 1)) & 0x8000;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x18:	/* RCR ew,1 */
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			I.OverVal = !(!(tmp & 0x8000)) != CF;
			I.CarryVal = tmp & 0x01;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x20:	/* SHL ew,1 */
		case 0x30:
			tmp <<= 1;

            SetCFW(tmp);
			SetOFW_Add(tmp,tmp2,tmp2);
			I.AuxVal = 1;
			SetSZPF_Word(tmp);

			PutbackRMWord(ModRM,tmp);
			break;
		case 0x28:	/* SHR ew,1 */
			I.CarryVal = tmp & 0x01;
			I.OverVal = tmp & 0x8000;

			tmp2 = tmp >> 1;

			SetSZPF_Word(tmp2);
			I.AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
			case 0x38:	/* SAR ew,1 */
			I.CarryVal = tmp & 0x01;
			I.OverVal = 0;

			tmp2 = (tmp >> 1) | (tmp & 0x8000);

			SetSZPF_Word(tmp2);
			I.AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
#else
		case 0x00:	/* ROL ew,1 */
			I.CarryVal = src & 0x8000;
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			I.OverVal = (src^dst)&0x8000;
			break;
		case 0x08:	/* ROR ew,1 */
			I.CarryVal = src & 0x01;
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			I.OverVal = (src^dst)&0x8000;
			break;
		case 0x10:	/* RCL ew,1 */
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			I.OverVal = (src^dst)&0x8000;
			break;
		case 0x18:	/* RCR ew,1 */
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = (src^dst)&0x8000;
			break;
		case 0x20:	/* SHL ew,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			I.OverVal = (src^dst)&0x8000;
			I.AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x28:	/* SHR ew,1 */
			dst = src >> 1;
			PutbackRMWord(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = src & 0x8000;
			I.AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x38:	/* SAR ew,1 */
			dst = ((INT16)src) >> 1;
			PutbackRMWord(ModRM,dst);
			I.CarryVal = src & 0x01;
			I.OverVal = 0;
			I.AuxVal = 1;
			SetSZPF_Word(dst);
			break;
#endif
		}
	}
	else
	{
		ICOUNT -= (ModRM >= 0xc0) ? cycles.rot_reg_base + cycles.rot_reg_bit : cycles.rot_m8_base + cycles.rot_m16_bit;

		switch (ModRM & 0x38)
		{
		case 0x00:	/* ROL ew,count */
			for (; count > 0; count--)
			{
				I.CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x08:	/* ROR ew,count */
			for (; count > 0; count--)
			{
				I.CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x10:  /* RCL ew,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + CF;
				SetCFW(dst);
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x18:	/* RCR ew,count */
			for (; count > 0; count--)
			{
				dst = dst + (CF << 16);
				I.CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x20:
		case 0x30:	/* SHL ew,count */
			dst <<= count;
			SetCFW(dst);
			I.AuxVal = 1;
			SetSZPF_Word(dst);
			PutbackRMWord(ModRM,dst);
			break;
		case 0x28:	/* SHR ew,count */
			dst >>= count-1;
			I.CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Word(dst);
			I.AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		case 0x38:	/* SAR ew,count */
			dst = ((INT16)dst) >> (count-1);
			I.CarryVal = dst & 0x01;
			dst = ((INT16)((WORD)dst)) >> 1;
			SetSZPF_Word(dst);
			I.AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		}
	}
}

static void PREFIX(rep)(int flagval)
{
    /* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
         loop  to continue for CMPS and SCAS instructions. */

	unsigned next = FETCHOP;
	unsigned count = I.regs.w[CX];

    switch(next)
    {
	case 0x26:  /* ES: */
		seg_prefix=TRUE;
		prefix_base=I.base[ES];
		ICOUNT -= cycles.override;
		PREFIX(rep)(flagval);
		break;
	case 0x2e:  /* CS: */
		seg_prefix=TRUE;
		prefix_base=I.base[CS];
		ICOUNT -= cycles.override;
		PREFIX(rep)(flagval);
		break;
	case 0x36:  /* SS: */
		seg_prefix=TRUE;
		prefix_base=I.base[SS];
		ICOUNT -= cycles.override;
		PREFIX(rep)(flagval);
		break;
	case 0x3e:  /* DS: */
		seg_prefix=TRUE;
		prefix_base=I.base[DS];
		ICOUNT -= cycles.override;
		PREFIX(rep)(flagval);
		break;
#ifndef I86
	case 0x6c:	/* REP INSB */
		ICOUNT -= cycles.rep_ins8_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			PutMemB(ES,I.regs.w[DI],read_port(I.regs.w[DX]));
			I.regs.w[DI] += I.DirVal;
			ICOUNT -= cycles.rep_ins8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0x6d:  /* REP INSW */
		ICOUNT -= cycles.rep_ins16_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			PutMemB(ES,I.regs.w[DI],read_port(I.regs.w[DX]));
			PutMemB(ES,I.regs.w[DI]+1,read_port(I.regs.w[DX]+1));
			I.regs.w[DI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_ins16_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0x6e:  /* REP OUTSB */
		ICOUNT -= cycles.rep_outs8_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			write_port(I.regs.w[DX],GetMemB(DS,I.regs.w[SI]));
			I.regs.w[SI] += I.DirVal; /* GOL 11/27/01 */
			ICOUNT -= cycles.rep_outs8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0x6f:  /* REP OUTSW */
		ICOUNT -= cycles.rep_outs16_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			write_port(I.regs.w[DX],GetMemB(DS,I.regs.w[SI]));
			write_port(I.regs.w[DX]+1,GetMemB(DS,I.regs.w[SI]+1));
			I.regs.w[SI] += 2 * I.DirVal; /* GOL 11/27/01 */
			ICOUNT -= cycles.rep_outs16_count;
		}
		I.regs.w[CX]=count;
		break;
#endif
	case 0xa4:  /* REP MOVSB */
		ICOUNT -= cycles.rep_movs8_base;
		for (; count > 0; count--)
		{
			BYTE tmp;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			tmp = GetMemB(DS,I.regs.w[SI]);
			PutMemB(ES,I.regs.w[DI], tmp);
			I.regs.w[DI] += I.DirVal;
			I.regs.w[SI] += I.DirVal;
			ICOUNT -= cycles.rep_movs8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xa5:  /* REP MOVSW */
		ICOUNT -= cycles.rep_movs16_base;
		for (; count > 0; count--)
		{
			WORD tmp;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			tmp = GetMemW(DS,I.regs.w[SI]);
			PutMemW(ES,I.regs.w[DI], tmp);
			I.regs.w[DI] += 2 * I.DirVal;
			I.regs.w[SI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_movs16_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xa6:  /* REP(N)E CMPSB */
		ICOUNT -= cycles.rep_cmps8_base;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
		{
			unsigned dst, src;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			dst = GetMemB(ES, I.regs.w[DI]);
			src = GetMemB(DS, I.regs.w[SI]);
		    SUBB(src,dst); /* opposite of the usual convention */
			I.regs.w[DI] += I.DirVal;
			I.regs.w[SI] += I.DirVal;
			ICOUNT -= cycles.rep_cmps8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xa7:  /* REP(N)E CMPSW */
		ICOUNT -= cycles.rep_cmps16_base;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
		{
			unsigned dst, src;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			dst = GetMemW(ES, I.regs.w[DI]);
			src = GetMemW(DS, I.regs.w[SI]);
		    SUBW(src,dst); /* opposite of the usual convention */
			I.regs.w[DI] += 2 * I.DirVal;
			I.regs.w[SI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_cmps16_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xaa:  /* REP STOSB */
		ICOUNT -= cycles.rep_stos8_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
			I.regs.w[DI] += I.DirVal;
			ICOUNT -= cycles.rep_stos8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xab:  /* REP STOSW */
		ICOUNT -= cycles.rep_stos16_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
			PutMemB(ES,I.regs.w[DI]+1,I.regs.b[AH]);
			I.regs.w[DI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_stos16_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xac:  /* REP LODSB */
		ICOUNT -= cycles.rep_lods8_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			I.regs.b[AL] = GetMemB(DS,I.regs.w[SI]);
			I.regs.w[SI] += I.DirVal;
			ICOUNT -= cycles.rep_lods8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xad:  /* REP LODSW */
		ICOUNT -= cycles.rep_lods16_base;
		for (; count > 0; count--)
		{
			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			I.regs.w[AX] = GetMemW(DS,I.regs.w[SI]);
			I.regs.w[SI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_lods16_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xae:  /* REP(N)E SCASB */
		ICOUNT -= cycles.rep_scas8_base;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
		{
			unsigned src, dst;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			src = GetMemB(ES, I.regs.w[DI]);
			dst = I.regs.b[AL];
		    SUBB(dst,src);
			I.regs.w[DI] += I.DirVal;
			ICOUNT -= cycles.rep_scas8_count;
		}
		I.regs.w[CX]=count;
		break;
	case 0xaf:  /* REP(N)E SCASW */
		ICOUNT -= cycles.rep_scas16_base;
		for (I.ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--)
		{
			unsigned src, dst;

			if (ICOUNT <= 0) { I.pc = I.prevpc; break; }
			src = GetMemW(ES, I.regs.w[DI]);
			dst = I.regs.w[AX];
		    SUBW(dst,src);
			I.regs.w[DI] += 2 * I.DirVal;
			ICOUNT -= cycles.rep_scas16_count;
		}
		I.regs.w[CX]=count;
		break;
	default:
		if (trace_186) Dis186();
		PREFIX(_instruction)[next]();
	}
}

static void PREFIX86(_add_br8)(void)    /* Opcode 0x00 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
	ADDB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_add_wr16)(void)    /* Opcode 0x01 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
	ADDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_add_r8b)(void)    /* Opcode 0x02 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    ADDB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_add_r16w)(void)    /* Opcode 0x03 */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
	ADDW(dst,src);
	RegWord(ModRM)=dst;
}


static void PREFIX86(_add_ald8)(void)    /* Opcode 0x04 */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    ADDB(dst,src);
	I.regs.b[AL]=dst;
}


static void PREFIX86(_add_axd16)(void)    /* Opcode 0x05 */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
	ADDW(dst,src);
	I.regs.w[AX]=dst;
}


static void PREFIX86(_push_es)(void)    /* Opcode 0x06 */
{
	ICOUNT -= cycles.push_seg;
	PUSH(I.sregs[ES]);
}


static void PREFIX86(_pop_es)(void)    /* Opcode 0x07 */
{
	POP(I.sregs[ES]);
	I.base[ES] = SegBase(ES);
	ICOUNT -= cycles.pop_seg;
}

static void PREFIX86(_or_br8)(void)    /* Opcode 0x08 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
	ORB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_or_wr16)(void)    /* Opcode 0x09 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
    ORW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_or_r8b)(void)    /* Opcode 0x0a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    ORB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_or_r16w)(void)    /* Opcode 0x0b */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
    ORW(dst,src);
    RegWord(ModRM)=dst;
}

static void PREFIX86(_or_ald8)(void)    /* Opcode 0x0c */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    ORB(dst,src);
	I.regs.b[AL]=dst;
}

static void PREFIX86(_or_axd16)(void)    /* Opcode 0x0d */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
	ORW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_push_cs)(void)    /* Opcode 0x0e */
{
	ICOUNT -= cycles.push_seg;
	PUSH(I.sregs[CS]);
}

/* Opcode 0x0f invalid */

static void PREFIX86(_adc_br8)(void)    /* Opcode 0x10 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
    src+=CF;
    ADDB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_adc_wr16)(void)    /* Opcode 0x11 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
    src+=CF;
    ADDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_adc_r8b)(void)    /* Opcode 0x12 */
{
    DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
	src+=CF;
    ADDB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_adc_r16w)(void)    /* Opcode 0x13 */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
	src+=CF;
    ADDW(dst,src);
    RegWord(ModRM)=dst;
}

static void PREFIX86(_adc_ald8)(void)    /* Opcode 0x14 */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    src+=CF;
    ADDB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_adc_axd16)(void)    /* Opcode 0x15 */
{
	DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
	src+=CF;
    ADDW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_push_ss)(void)    /* Opcode 0x16 */
{
	PUSH(I.sregs[SS]);
	ICOUNT -= cycles.push_seg;
}

static void PREFIX86(_pop_ss)(void)    /* Opcode 0x17 */
{
	POP(I.sregs[SS]);
	I.base[SS] = SegBase(SS);
	ICOUNT -= cycles.pop_seg;
	if (trace_186) Dis186();
	PREFIX(_instruction)[FETCHOP](); /* no interrupt before next instruction */
}

static void PREFIX86(_sbb_br8)(void)    /* Opcode 0x18 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
    src+=CF;
    SUBB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_sbb_wr16)(void)    /* Opcode 0x19 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
    src+=CF;
	SUBW(dst,src);
	PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_sbb_r8b)(void)    /* Opcode 0x1a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
	src+=CF;
    SUBB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_sbb_r16w)(void)    /* Opcode 0x1b */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
	src+=CF;
    SUBW(dst,src);
    RegWord(ModRM)= dst;
}

static void PREFIX86(_sbb_ald8)(void)    /* Opcode 0x1c */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    src+=CF;
    SUBB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_sbb_axd16)(void)    /* Opcode 0x1d */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
	src+=CF;
    SUBW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_push_ds)(void)    /* Opcode 0x1e */
{
	PUSH(I.sregs[DS]);
	ICOUNT -= cycles.push_seg;
}

static void PREFIX86(_pop_ds)(void)    /* Opcode 0x1f */
{
	POP(I.sregs[DS]);
	I.base[DS] = SegBase(DS);
	ICOUNT -= cycles.push_seg;
}

static void PREFIX86(_and_br8)(void)    /* Opcode 0x20 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
    ANDB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_and_wr16)(void)    /* Opcode 0x21 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
    ANDW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_and_r8b)(void)    /* Opcode 0x22 */
{
    DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    ANDB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_and_r16w)(void)    /* Opcode 0x23 */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
	ANDW(dst,src);
    RegWord(ModRM)=dst;
}

static void PREFIX86(_and_ald8)(void)    /* Opcode 0x24 */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    ANDB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_and_axd16)(void)    /* Opcode 0x25 */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
    ANDW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_es)(void)    /* Opcode 0x26 */
{
	seg_prefix=TRUE;
	prefix_base=I.base[ES];
	ICOUNT -= cycles.override;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_daa)(void)    /* Opcode 0x27 */
{
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		I.regs.b[AL] = tmp = I.regs.b[AL] + 6;
		I.AuxVal = 1;
		I.CarryVal |= tmp & 0x100;
	}

	if (CF || (I.regs.b[AL] > 0x9f))
	{
		I.regs.b[AL] += 0x60;
		I.CarryVal = 1;
	}

	SetSZPF_Byte(I.regs.b[AL]);
	ICOUNT -= cycles.daa;
}

static void PREFIX86(_sub_br8)(void)    /* Opcode 0x28 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
    SUBB(dst,src);
    PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_sub_wr16)(void)    /* Opcode 0x29 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
    SUBW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_sub_r8b)(void)    /* Opcode 0x2a */
{
    DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
	SUBB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_sub_r16w)(void)    /* Opcode 0x2b */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
    SUBW(dst,src);
    RegWord(ModRM)=dst;
}

static void PREFIX86(_sub_ald8)(void)    /* Opcode 0x2c */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    SUBB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_sub_axd16)(void)    /* Opcode 0x2d */
{
	DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
    SUBW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_cs)(void)    /* Opcode 0x2e */
{
    seg_prefix=TRUE;
	prefix_base=I.base[CS];
	ICOUNT -= cycles.override;
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_das)(void)    /* Opcode 0x2f */
{
	unsigned tmpAL=I.regs.b[AL];
	if (AF || ((I.regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		I.regs.b[AL] = tmp = I.regs.b[AL] - 6;
		I.AuxVal = 1;
		I.CarryVal |= tmp & 0x100;
	}

	if (CF || (tmpAL > 0x9f))
	{
		I.regs.b[AL] -= 0x60;
		I.CarryVal = 1;
	}

	SetSZPF_Byte(I.regs.b[AL]);
	ICOUNT -= cycles.das;
}

static void PREFIX86(_xor_br8)(void)    /* Opcode 0x30 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_mr8;
    XORB(dst,src);
	PutbackRMByte(ModRM,dst);
}

static void PREFIX86(_xor_wr16)(void)    /* Opcode 0x31 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_mr16;
	XORW(dst,src);
    PutbackRMWord(ModRM,dst);
}

static void PREFIX86(_xor_r8b)(void)    /* Opcode 0x32 */
{
    DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    XORB(dst,src);
    RegByte(ModRM)=dst;
}

static void PREFIX86(_xor_r16w)(void)    /* Opcode 0x33 */
{
    DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
    XORW(dst,src);
	RegWord(ModRM)=dst;
}

static void PREFIX86(_xor_ald8)(void)    /* Opcode 0x34 */
{
	DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    XORB(dst,src);
	I.regs.b[AL] = dst;
}

static void PREFIX86(_xor_axd16)(void)    /* Opcode 0x35 */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
    XORW(dst,src);
	I.regs.w[AX]=dst;
}

static void PREFIX86(_ss)(void)    /* Opcode 0x36 */
{
	seg_prefix=TRUE;
	prefix_base=I.base[SS];
	ICOUNT -= cycles.override;
	if (trace_186) Dis186();
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_aaa)(void)    /* Opcode 0x37 */
{
	unsigned ALcarry=1;
	if (I.regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((I.regs.b[AL] & 0xf) > 9))
    {
		I.regs.b[AL] += 6;
		I.regs.b[AH] += ALcarry;
		I.AuxVal = 1;
		I.CarryVal = 1;
    }
	else
	{
		I.AuxVal = 0;
		I.CarryVal = 0;
    }
	I.regs.b[AL] &= 0x0F;
	ICOUNT -= cycles.aaa;
}

static void PREFIX86(_cmp_br8)(void)    /* Opcode 0x38 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    SUBB(dst,src);
}

static void PREFIX86(_cmp_wr16)(void)    /* Opcode 0x39 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
    SUBW(dst,src);
}

static void PREFIX86(_cmp_r8b)(void)    /* Opcode 0x3a */
{
    DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    SUBB(dst,src);
}

static void PREFIX86(_cmp_r16w)(void)    /* Opcode 0x3b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
    SUBW(dst,src);
}

static void PREFIX86(_cmp_ald8)(void)    /* Opcode 0x3c */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    SUBB(dst,src);
}

static void PREFIX86(_cmp_axd16)(void)    /* Opcode 0x3d */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
    SUBW(dst,src);
}

static void PREFIX86(_ds)(void)    /* Opcode 0x3e */
{
	seg_prefix=TRUE;
	prefix_base=I.base[DS];
	ICOUNT -= cycles.override;
	if (trace_186) Dis186();
	PREFIX(_instruction)[FETCHOP]();
}

static void PREFIX86(_aas)(void)    /* Opcode 0x3f */
{
	unsigned ALcarry=1;
	if (I.regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((I.regs.b[AL] & 0xf) > 9))
    {
		I.regs.b[AL] -= 6;
		I.regs.b[AH] -= 1;
		I.AuxVal = 1;
		I.CarryVal = 1;
    }
	else
	{
		I.AuxVal = 0;
		I.CarryVal = 0;
    }
	I.regs.b[AL] &= 0x0F;
	ICOUNT -= cycles.aas;
}

#define IncWordReg(Reg) 					\
{											\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	SetOFW_Add(tmp1,tmp,1); 				\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1; 					\
	ICOUNT -= cycles.incdec_r16;			\
}

static void PREFIX86(_inc_ax)(void)    /* Opcode 0x40 */
{
    IncWordReg(AX);
}

static void PREFIX86(_inc_cx)(void)    /* Opcode 0x41 */
{
    IncWordReg(CX);
}

static void PREFIX86(_inc_dx)(void)    /* Opcode 0x42 */
{
	IncWordReg(DX);
}

static void PREFIX(_inc_bx)(void)    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

static void PREFIX86(_inc_sp)(void)    /* Opcode 0x44 */
{
    IncWordReg(SP);
}

static void PREFIX86(_inc_bp)(void)    /* Opcode 0x45 */
{
	IncWordReg(BP);
}

static void PREFIX86(_inc_si)(void)    /* Opcode 0x46 */
{
	IncWordReg(SI);
}

static void PREFIX86(_inc_di)(void)    /* Opcode 0x47 */
{
    IncWordReg(DI);
}

#define DecWordReg(Reg) 					\
{ 											\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; 					\
    SetOFW_Sub(tmp1,1,tmp); 				\
    SetAF(tmp1,tmp,1); 						\
	SetSZPF_Word(tmp1);						\
	I.regs.w[Reg]=tmp1; 					\
	ICOUNT -= cycles.incdec_r16;			\
}

static void PREFIX86(_dec_ax)(void)    /* Opcode 0x48 */
{
    DecWordReg(AX);
}

static void PREFIX86(_dec_cx)(void)    /* Opcode 0x49 */
{
	DecWordReg(CX);
}

static void PREFIX86(_dec_dx)(void)    /* Opcode 0x4a */
{
	DecWordReg(DX);
}

static void PREFIX86(_dec_bx)(void)    /* Opcode 0x4b */
{
	DecWordReg(BX);
}

static void PREFIX86(_dec_sp)(void)    /* Opcode 0x4c */
{
    DecWordReg(SP);
}

static void PREFIX86(_dec_bp)(void)    /* Opcode 0x4d */
{
    DecWordReg(BP);
}

static void PREFIX86(_dec_si)(void)    /* Opcode 0x4e */
{
    DecWordReg(SI);
}

static void PREFIX86(_dec_di)(void)    /* Opcode 0x4f */
{
    DecWordReg(DI);
}

static void PREFIX86(_push_ax)(void)    /* Opcode 0x50 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[AX]);
}

static void PREFIX86(_push_cx)(void)    /* Opcode 0x51 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[CX]);
}

static void PREFIX86(_push_dx)(void)    /* Opcode 0x52 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[DX]);
}

static void PREFIX86(_push_bx)(void)    /* Opcode 0x53 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[BX]);
}

static void PREFIX86(_push_sp)(void)    /* Opcode 0x54 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[SP]);
}

static void PREFIX86(_push_bp)(void)    /* Opcode 0x55 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[BP]);
}


static void PREFIX86(_push_si)(void)    /* Opcode 0x56 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[SI]);
}

static void PREFIX86(_push_di)(void)    /* Opcode 0x57 */
{
	ICOUNT -= cycles.push_r16;
	PUSH(I.regs.w[DI]);
}

static void PREFIX86(_pop_ax)(void)    /* Opcode 0x58 */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[AX]);
}

static void PREFIX86(_pop_cx)(void)    /* Opcode 0x59 */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[CX]);
}

static void PREFIX86(_pop_dx)(void)    /* Opcode 0x5a */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[DX]);
}

static void PREFIX86(_pop_bx)(void)    /* Opcode 0x5b */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[BX]);
}

static void PREFIX86(_pop_sp)(void)    /* Opcode 0x5c */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[SP]);
}

static void PREFIX86(_pop_bp)(void)    /* Opcode 0x5d */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[BP]);
}

static void PREFIX86(_pop_si)(void)    /* Opcode 0x5e */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[SI]);
}

static void PREFIX86(_pop_di)(void)    /* Opcode 0x5f */
{
	ICOUNT -= cycles.pop_r16;
	POP(I.regs.w[DI]);
}

static void PREFIX86(_jo)(void)    /* Opcode 0x70 */
{
	int tmp = (int)((INT8)FETCH);
	if (OF)
	{
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jno)(void)    /* Opcode 0x71 */
{
	int tmp = (int)((INT8)FETCH);
	if (!OF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jb)(void)    /* Opcode 0x72 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnb)(void)    /* Opcode 0x73 */
{
	int tmp = (int)((INT8)FETCH);
	if (!CF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jz)(void)    /* Opcode 0x74 */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnz)(void)    /* Opcode 0x75 */
{
	int tmp = (int)((INT8)FETCH);
	if (!ZF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jbe)(void)    /* Opcode 0x76 */
{
	int tmp = (int)((INT8)FETCH);
    if (CF || ZF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnbe)(void)    /* Opcode 0x77 */
{
	int tmp = (int)((INT8)FETCH);
	 if (!(CF || ZF)) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_js)(void)    /* Opcode 0x78 */
{
	int tmp = (int)((INT8)FETCH);
    if (SF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jns)(void)    /* Opcode 0x79 */
{
	int tmp = (int)((INT8)FETCH);
    if (!SF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jp)(void)    /* Opcode 0x7a */
{
	int tmp = (int)((INT8)FETCH);
    if (PF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnp)(void)    /* Opcode 0x7b */
{
	int tmp = (int)((INT8)FETCH);
    if (!PF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jl)(void)    /* Opcode 0x7c */
{
	int tmp = (int)((INT8)FETCH);
    if ((SF!=OF)&&!ZF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnl)(void)    /* Opcode 0x7d */
{
	int tmp = (int)((INT8)FETCH);
    if (ZF||(SF==OF)) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jle)(void)    /* Opcode 0x7e */
{
	int tmp = (int)((INT8)FETCH);
    if (ZF||(SF!=OF)) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_jnle)(void)    /* Opcode 0x7f */
{
	int tmp = (int)((INT8)FETCH);
    if ((SF==OF)&&!ZF) {
		I.pc += tmp;
		ICOUNT -= cycles.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.jcc_nt;
}

static void PREFIX86(_80pre)(void)    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	unsigned dst = GetRMByte(ModRM);
    unsigned src = FETCH;

    switch (ModRM & 0x38)
	{
    case 0x00:  /* ADD eb,d8 */
        ADDB(dst,src);
        PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x08:  /* OR eb,d8 */
        ORB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x10:  /* ADC eb,d8 */
        src+=CF;
        ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x18:  /* SBB eb,b8 */
		src+=CF;
		SUBB(dst,src);
        PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x20:  /* AND eb,d8 */
        ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x28:  /* SUB eb,d8 */
        SUBB(dst,src);
        PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x30:  /* XOR eb,d8 */
        XORB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
    case 0x38:  /* CMP eb,d8 */
        SUBB(dst,src);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8_ro;
		break;
	}
}


static void PREFIX86(_81pre)(void)    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
    unsigned src = FETCH;
    src+= (FETCH << 8);

	switch (ModRM & 0x38)
    {
    case 0x00:  /* ADD ew,d16 */
		ADDW(dst,src);
		PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x08:  /* OR ew,d16 */
        ORW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
	case 0x10:	/* ADC ew,d16 */
		src+=CF;
		ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x18:  /* SBB ew,d16 */
        src+=CF;
		SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x20:  /* AND ew,d16 */
        ANDW(dst,src);
		PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x28:  /* SUB ew,d16 */
        SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16;
		break;
    case 0x38:  /* CMP ew,d16 */
        SUBW(dst,src);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16_ro;
		break;
    }
}

static void PREFIX86(_82pre)(void)	 /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;

	switch (ModRM & 0x38)
	{
	case 0x00:	/* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x08:	/* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x10:	/* ADC eb,d8 */
		src+=CF;
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x18:	/* SBB eb,d8 */
        src+=CF;
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x20:	/* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x28:	/* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x30:	/* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8;
		break;
	case 0x38:	/* CMP eb,d8 */
		SUBB(dst,src);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8_ro;
		break;
	}
}

static void PREFIX86(_83pre)(void)    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
    unsigned dst = GetRMWord(ModRM);
    unsigned src = (WORD)((INT16)((INT8)FETCH));

	switch (ModRM & 0x38)
    {
    case 0x00:  /* ADD ew,d16 */
        ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
    case 0x08:  /* OR ew,d16 */
		ORW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
    case 0x10:  /* ADC ew,d16 */
        src+=CF;
		ADDW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
	case 0x18:	/* SBB ew,d16 */
		src+=CF;
        SUBW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
	case 0x20:	/* AND ew,d16 */
        ANDW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
    case 0x28:  /* SUB ew,d16 */
		SUBW(dst,src);
		PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
    case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
        PutbackRMWord(ModRM,dst);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8;
		break;
    case 0x38:  /* CMP ew,d16 */
        SUBW(dst,src);
        ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_r16i8 : cycles.alu_m16i8_ro;
		break;
    }
}

static void PREFIX86(_test_br8)(void)    /* Opcode 0x84 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr8 : cycles.alu_rm8;
    ANDB(dst,src);
}

static void PREFIX86(_test_wr16)(void)    /* Opcode 0x85 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_rr16 : cycles.alu_rm16;
	ANDW(dst,src);
}

static void PREFIX86(_xchg_br8)(void)    /* Opcode 0x86 */
{
    DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.xchg_rr8 : cycles.xchg_rm8;
    RegByte(ModRM)=dst;
    PutbackRMByte(ModRM,src);
}

static void PREFIX86(_xchg_wr16)(void)    /* Opcode 0x87 */
{
    DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.xchg_rr16 : cycles.xchg_rm16;
    RegWord(ModRM)=dst;
    PutbackRMWord(ModRM,src);
}

static void PREFIX86(_mov_br8)(void)    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
    BYTE src = RegByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_rr8 : cycles.mov_mr8;
    PutRMByte(ModRM,src);
}

static void PREFIX86(_mov_wr16)(void)    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
    WORD src = RegWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_rr16 : cycles.mov_mr16;
    PutRMWord(ModRM,src);
}

static void PREFIX86(_mov_r8b)(void)    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
    BYTE src = GetRMByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_rr8 : cycles.mov_rm8;
    RegByte(ModRM)=src;
}

static void PREFIX86(_mov_r16w)(void)    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_rr8 : cycles.mov_rm16;
	RegWord(ModRM)=src;
}

static void PREFIX86(_mov_wsreg)(void)    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_rs : cycles.mov_ms;
	if (ModRM & 0x20) return;	/* HJB 12/13/98 1xx is invalid */
	PutRMWord(ModRM,I.sregs[(ModRM & 0x38) >> 3]);
}

static void PREFIX86(_lea)(void)    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
	ICOUNT -= cycles.lea;
	(void)(*GetEA[ModRM])();
	RegWord(ModRM)=EO;	/* HJB 12/13/98 effective offset (no segment part) */
}

static void PREFIX86(_mov_sregw)(void)    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
    WORD src = GetRMWord(ModRM);

	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_sr : cycles.mov_sm;
    switch (ModRM & 0x38)
    {
    case 0x00:  /* mov es,ew */
		I.sregs[ES] = src;
		I.base[ES] = SegBase(ES);
		break;
    case 0x18:  /* mov ds,ew */
		I.sregs[DS] = src;
		I.base[DS] = SegBase(DS);
		break;
    case 0x10:  /* mov ss,ew */
		I.sregs[SS] = src;
		I.base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		if (trace_186) Dis186();
		PREFIX(_instruction)[FETCHOP]();
		break;
    case 0x08:  /* mov cs,ew */
		break;  /* doesn't do a jump far */
    }
}

static void PREFIX86(_popw)(void)    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
    WORD tmp;
	POP(tmp);
	ICOUNT -= (ModRM >= 0xc0) ? cycles.pop_r16 : cycles.pop_m16;
	PutRMWord(ModRM,tmp);
}


#define XchgAXReg(Reg) 				\
{ 									\
    WORD tmp; 						\
	tmp = I.regs.w[Reg]; 			\
	I.regs.w[Reg] = I.regs.w[AX]; 	\
	I.regs.w[AX] = tmp; 			\
	ICOUNT -= cycles.xchg_ar16; 	\
}


static void PREFIX86(_nop)(void)    /* Opcode 0x90 */
{
    /* this is XchgAXReg(AX); */
	ICOUNT -= cycles.nop;
}

static void PREFIX86(_xchg_axcx)(void)    /* Opcode 0x91 */
{
	XchgAXReg(CX);
}

static void PREFIX86(_xchg_axdx)(void)    /* Opcode 0x92 */
{
    XchgAXReg(DX);
}

static void PREFIX86(_xchg_axbx)(void)    /* Opcode 0x93 */
{
	XchgAXReg(BX);
}

static void PREFIX86(_xchg_axsp)(void)    /* Opcode 0x94 */
{
	XchgAXReg(SP);
}

static void PREFIX86(_xchg_axbp)(void)    /* Opcode 0x95 */
{
    XchgAXReg(BP);
}

static void PREFIX86(_xchg_axsi)(void)    /* Opcode 0x96 */
{
	XchgAXReg(SI);
}

static void PREFIX86(_xchg_axdi)(void)    /* Opcode 0x97 */
{
	XchgAXReg(DI);
}

static void PREFIX86(_cbw)(void)    /* Opcode 0x98 */
{
	ICOUNT -= cycles.cbw;
	I.regs.b[AH] = (I.regs.b[AL] & 0x80) ? 0xff : 0;
}

static void PREFIX86(_cwd)(void)    /* Opcode 0x99 */
{
	ICOUNT -= cycles.cwd;
	I.regs.w[DX] = (I.regs.b[AH] & 0x80) ? 0xffff : 0;
}

static void PREFIX86(_call_far)(void)
{
    unsigned tmp, tmp2;
	WORD ip;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp2 = FETCH;
	tmp2 += FETCH << 8;

	ip = I.pc - I.base[CS];
	PUSH(I.sregs[CS]);
	PUSH(ip);

	I.sregs[CS] = (WORD)tmp2;
	I.base[CS] = SegBase(CS);
	I.pc = (I.base[CS] + (WORD)tmp) & AMASK;
	ICOUNT -= cycles.call_far;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_wait)(void)    /* Opcode 0x9b */
{
/* PJB 03/05 */
        if(!I.test_state) ICOUNT -= cycles.wait;
        else{
            ICOUNT -= cycles.wait;
            I.pc --;
        }
}

static void PREFIX86(_pushf)(void)    /* Opcode 0x9c */
{
	ICOUNT -= cycles.pushf;
	// This crashes the VS.NET2003 compiler
	// PUSH( CompressFlags() | 0xf000 );
	UINT32 f = CompressFlags() | 0xf000;
	PUSH( f );
}

static void PREFIX86(_popf)(void)    /* Opcode 0x9d */
{
	unsigned tmp;
    POP(tmp);
	ICOUNT -= cycles.popf;
    ExpandFlags(tmp);

	if (I.TF) PREFIX(_trap)();

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (I.IF && I.irq_state)
		PREFIX(_interrupt)(12);
}

static void PREFIX86(_sahf)(void)    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (I.regs.b[AH] & 0xd5);
	ICOUNT -= cycles.sahf;
    ExpandFlags(tmp);
}

static void PREFIX86(_lahf)(void)    /* Opcode 0x9f */
{
	I.regs.b[AH] = CompressFlags() & 0xff;
	ICOUNT -= cycles.lahf;
}


static void PREFIX86(_mov_aldisp)(void)    /* Opcode 0xa0 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= cycles.mov_am8;
	I.regs.b[AL] = GetMemB(DS, addr);
}

static void PREFIX86(_mov_axdisp)(void)    /* Opcode 0xa1 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= cycles.mov_am16;
	I.regs.b[AL] = GetMemB(DS, addr);
	I.regs.b[AH] = GetMemB(DS, addr+1);
}

static void PREFIX86(_mov_dispal)(void)    /* Opcode 0xa2 */
{
    unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= cycles.mov_ma8;
	PutMemB(DS, addr, I.regs.b[AL]);
}

static void PREFIX86(_mov_dispax)(void)    /* Opcode 0xa3 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= cycles.mov_ma16;
	PutMemB(DS, addr, I.regs.b[AL]);
	PutMemB(DS, addr+1, I.regs.b[AH]);
}

static void PREFIX86(_movsb)(void)    /* Opcode 0xa4 */
{
	BYTE tmp = GetMemB(DS,I.regs.w[SI]);
	PutMemB(ES,I.regs.w[DI], tmp);
	I.regs.w[DI] += I.DirVal;
	I.regs.w[SI] += I.DirVal;
	ICOUNT -= cycles.movs8;
}

static void PREFIX86(_movsw)(void)    /* Opcode 0xa5 */
{
	WORD tmp = GetMemW(DS,I.regs.w[SI]);
	PutMemW(ES,I.regs.w[DI], tmp);
	I.regs.w[DI] += 2 * I.DirVal;
	I.regs.w[SI] += 2 * I.DirVal;
	ICOUNT -= cycles.movs16;
}

static void PREFIX86(_cmpsb)(void)    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, I.regs.w[DI]);
	unsigned src = GetMemB(DS, I.regs.w[SI]);
    SUBB(src,dst); /* opposite of the usual convention */
	I.regs.w[DI] += I.DirVal;
	I.regs.w[SI] += I.DirVal;
	ICOUNT -= cycles.cmps8;
}

static void PREFIX86(_cmpsw)(void)    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, I.regs.w[DI]);
	unsigned src = GetMemW(DS, I.regs.w[SI]);
	SUBW(src,dst); /* opposite of the usual convention */
	I.regs.w[DI] += 2 * I.DirVal;
	I.regs.w[SI] += 2 * I.DirVal;
	ICOUNT -= cycles.cmps16;
}

static void PREFIX86(_test_ald8)(void)    /* Opcode 0xa8 */
{
    DEF_ald8(dst,src);
	ICOUNT -= cycles.alu_ri8;
    ANDB(dst,src);
}

static void PREFIX86(_test_axd16)(void)    /* Opcode 0xa9 */
{
    DEF_axd16(dst,src);
	ICOUNT -= cycles.alu_ri16;
    ANDW(dst,src);
}

static void PREFIX86(_stosb)(void)    /* Opcode 0xaa */
{
	PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
	I.regs.w[DI] += I.DirVal;
	ICOUNT -= cycles.stos8;
}

static void PREFIX86(_stosw)(void)    /* Opcode 0xab */
{
	PutMemB(ES,I.regs.w[DI],I.regs.b[AL]);
	PutMemB(ES,I.regs.w[DI]+1,I.regs.b[AH]);
	I.regs.w[DI] += 2 * I.DirVal;
	ICOUNT -= cycles.stos16;
}

static void PREFIX86(_lodsb)(void)    /* Opcode 0xac */
{
	I.regs.b[AL] = GetMemB(DS,I.regs.w[SI]);
	I.regs.w[SI] += I.DirVal;
	ICOUNT -= cycles.lods8;
}

static void PREFIX86(_lodsw)(void)    /* Opcode 0xad */
{
	I.regs.w[AX] = GetMemW(DS,I.regs.w[SI]);
	I.regs.w[SI] += 2 * I.DirVal;
	ICOUNT -= cycles.lods16;
}

static void PREFIX86(_scasb)(void)    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, I.regs.w[DI]);
	unsigned dst = I.regs.b[AL];
    SUBB(dst,src);
	I.regs.w[DI] += I.DirVal;
	ICOUNT -= cycles.scas8;
}

static void PREFIX86(_scasw)(void)    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, I.regs.w[DI]);
	unsigned dst = I.regs.w[AX];
    SUBW(dst,src);
	I.regs.w[DI] += 2 * I.DirVal;
	ICOUNT -= cycles.scas16;
}

static void PREFIX86(_mov_ald8)(void)    /* Opcode 0xb0 */
{
	I.regs.b[AL] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_cld8)(void)    /* Opcode 0xb1 */
{
	I.regs.b[CL] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_dld8)(void)    /* Opcode 0xb2 */
{
	I.regs.b[DL] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_bld8)(void)    /* Opcode 0xb3 */
{
	I.regs.b[BL] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_ahd8)(void)    /* Opcode 0xb4 */
{
	I.regs.b[AH] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_chd8)(void)    /* Opcode 0xb5 */
{
	I.regs.b[CH] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_dhd8)(void)    /* Opcode 0xb6 */
{
	I.regs.b[DH] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_bhd8)(void)    /* Opcode 0xb7 */
{
	I.regs.b[BH] = FETCH;
	ICOUNT -= cycles.mov_ri8;
}

static void PREFIX86(_mov_axd16)(void)    /* Opcode 0xb8 */
{
	I.regs.b[AL] = FETCH;
	I.regs.b[AH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_cxd16)(void)    /* Opcode 0xb9 */
{
	I.regs.b[CL] = FETCH;
	I.regs.b[CH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_dxd16)(void)    /* Opcode 0xba */
{
	I.regs.b[DL] = FETCH;
	I.regs.b[DH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_bxd16)(void)    /* Opcode 0xbb */
{
	I.regs.b[BL] = FETCH;
	I.regs.b[BH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_spd16)(void)    /* Opcode 0xbc */
{
	I.regs.b[SPL] = FETCH;
	I.regs.b[SPH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_bpd16)(void)    /* Opcode 0xbd */
{
	I.regs.b[BPL] = FETCH;
	I.regs.b[BPH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_sid16)(void)    /* Opcode 0xbe */
{
	I.regs.b[SIL] = FETCH;
	I.regs.b[SIH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_mov_did16)(void)    /* Opcode 0xbf */
{
	I.regs.b[DIL] = FETCH;
	I.regs.b[DIH] = FETCH;
	ICOUNT -= cycles.mov_ri16;
}

static void PREFIX86(_ret_d16)(void)    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(I.pc);
	I.pc = (I.pc + I.base[CS]) & AMASK;
	I.regs.w[SP]+=count;
	ICOUNT -= cycles.ret_near_imm;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_ret)(void)    /* Opcode 0xc3 */
{
	POP(I.pc);
	I.pc = (I.pc + I.base[CS]) & AMASK;
	ICOUNT -= cycles.ret_near;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_les_dw)(void)    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
    WORD tmp = GetRMWord(ModRM);

    RegWord(ModRM)= tmp;
	I.sregs[ES] = GetnextRMWord;
	I.base[ES] = SegBase(ES);
	ICOUNT -= cycles.load_ptr;
}

static void PREFIX86(_lds_dw)(void)    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
    WORD tmp = GetRMWord(ModRM);

    RegWord(ModRM)=tmp;
	I.sregs[DS] = GetnextRMWord;
	I.base[DS] = SegBase(DS);
	ICOUNT -= cycles.load_ptr;
}

static void PREFIX86(_mov_bd8)(void)    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_ri8 : cycles.mov_mi8;
	PutImmRMByte(ModRM);
}

static void PREFIX86(_mov_wd16)(void)    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? cycles.mov_ri16 : cycles.mov_mi16;
	PutImmRMWord(ModRM);
}

static void PREFIX86(_retf_d16)(void)    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;

	POP(I.pc);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
	I.pc = (I.pc + I.base[CS]) & AMASK;
	I.regs.w[SP]+=count;
	ICOUNT -= cycles.ret_far_imm;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_retf)(void)    /* Opcode 0xcb */
{
	POP(I.pc);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
	I.pc = (I.pc + I.base[CS]) & AMASK;
	ICOUNT -= cycles.ret_far;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_int3)(void)    /* Opcode 0xcc */
{
	ICOUNT -= cycles.int3;
	PREFIX(_interrupt)(3);
}

static void PREFIX86(_int)(void)    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	ICOUNT -= cycles.int_imm;
	PREFIX(_interrupt)(int_num);
}

static void PREFIX86(_into)(void)    /* Opcode 0xce */
{
	if (OF) {
		ICOUNT -= cycles.into_t;
		PREFIX(_interrupt)(4);
	} else ICOUNT -= cycles.into_nt;
}

static void PREFIX86(_iret)(void)    /* Opcode 0xcf */
{
	ICOUNT -= cycles.iret;
	POP(I.pc);
	POP(I.sregs[CS]);
	I.base[CS] = SegBase(CS);
	I.pc = (I.pc + I.base[CS]) & AMASK;
    PREFIX(_popf)();
	CHANGE_PC(I.pc);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (I.IF && I.irq_state)
		PREFIX(_interrupt)(12);
}

static void PREFIX86(_rotshft_b)(void)    /* Opcode 0xd0 */
{
	PREFIX(_rotate_shift_Byte)(FETCHOP,1);
}


static void PREFIX86(_rotshft_w)(void)    /* Opcode 0xd1 */
{
	PREFIX(_rotate_shift_Word)(FETCHOP,1);
}


static void PREFIX86(_rotshft_bcl)(void)    /* Opcode 0xd2 */
{
	PREFIX(_rotate_shift_Byte)(FETCHOP,I.regs.b[CL]);
}

static void PREFIX86(_rotshft_wcl)(void)    /* Opcode 0xd3 */
{
	PREFIX(_rotate_shift_Word)(FETCHOP,I.regs.b[CL]);
}

/* OB: Opcode works on NEC V-Series but not the Variants              */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.              */
static void PREFIX86(_aam)(void)    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;

	ICOUNT -= cycles.aam;
	if (mult == 0)
		PREFIX(_interrupt)(0);
	else
	{
		I.regs.b[AH] = I.regs.b[AL] / mult;
		I.regs.b[AL] %= mult;

		SetSZPF_Word(I.regs.w[AX]);
	}
}

static void PREFIX86(_aad)(void)    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;

	ICOUNT -= cycles.aad;

	I.regs.b[AL] = I.regs.b[AH] * mult + I.regs.b[AL];
	I.regs.b[AH] = 0;

	SetZF(I.regs.b[AL]);
	SetPF(I.regs.b[AL]);
	I.SignVal = 0;
}


static void PREFIX86(_xlat)(void)    /* Opcode 0xd7 */
{
	unsigned dest = I.regs.w[BX]+I.regs.b[AL];

	ICOUNT -= cycles.xlat;
	I.regs.b[AL] = GetMemB(DS, dest);
}

static void PREFIX86(_escape)(void)    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	ICOUNT -= cycles.nop;
    GetRMByte(ModRM);
}

static void PREFIX86(_loopne)(void)    /* Opcode 0xe0 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

    if (!ZF && tmp) {
		ICOUNT -= cycles.loop_t;
		I.pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.loop_nt;
}

static void PREFIX86(_loope)(void)    /* Opcode 0xe1 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

	if (ZF && tmp) {
		ICOUNT -= cycles.loope_t;
		 I.pc += disp;
/* ASG - can probably assume this is safe
         CHANGE_PC(I.pc);*/
	 } else ICOUNT -= cycles.loope_nt;
}

static void PREFIX86(_loop)(void)    /* Opcode 0xe2 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = I.regs.w[CX]-1;

	I.regs.w[CX]=tmp;

    if (tmp) {
		ICOUNT -= cycles.loop_t;
		I.pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else ICOUNT -= cycles.loop_nt;
}

static void PREFIX86(_jcxz)(void)    /* Opcode 0xe3 */
{
	int disp = (int)((INT8)FETCH);

	if (I.regs.w[CX] == 0) {
		ICOUNT -= cycles.jcxz_t;
		I.pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(I.pc);*/
	} else
		ICOUNT -= cycles.jcxz_nt;
}

static void PREFIX86(_inal)(void)    /* Opcode 0xe4 */
{
	unsigned port = FETCH;

	ICOUNT -= cycles.in_imm8;
	I.regs.b[AL] = read_port(port);
}

static void PREFIX86(_inax)(void)    /* Opcode 0xe5 */
{
	unsigned port = FETCH;

	ICOUNT -= cycles.in_imm16;
	I.regs.b[AL] = read_port(port);
	I.regs.b[AH] = read_port(port+1);
}

static void PREFIX86(_outal)(void)    /* Opcode 0xe6 */
{
	unsigned port = FETCH;

	ICOUNT -= cycles.out_imm8;
	write_port(port, I.regs.b[AL]);
}

static void PREFIX86(_outax)(void)    /* Opcode 0xe7 */
{
	unsigned port = FETCH;

	ICOUNT -= cycles.out_imm16;
	write_port(port, I.regs.b[AL]);
	write_port(port+1, I.regs.b[AH]);
}

static void PREFIX86(_call_d16)(void)    /* Opcode 0xe8 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = I.pc - I.base[CS];
	PUSH(ip);
	ip += tmp;
	I.pc = (ip + I.base[CS]) & AMASK;
	ICOUNT -= cycles.call_near;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_jmp_d16)(void)    /* Opcode 0xe9 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = I.pc - I.base[CS] + tmp;
	I.pc = (ip + I.base[CS]) & AMASK;
	ICOUNT -= cycles.jmp_near;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_jmp_far)(void)    /* Opcode 0xea */
{
	unsigned tmp,tmp1;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp1 = FETCH;
	tmp1 += FETCH << 8;

	I.sregs[CS] = (WORD)tmp1;
	I.base[CS] = SegBase(CS);
	I.pc = (I.base[CS] + tmp) & AMASK;
	ICOUNT -= cycles.jmp_far;
	CHANGE_PC(I.pc);
}

static void PREFIX86(_jmp_d8)(void)    /* Opcode 0xeb */
{
	int tmp = (int)((INT8)FETCH);
	I.pc += tmp;
/* ASG - can probably assume this is safe
    CHANGE_PC(I.pc);*/
	ICOUNT -= cycles.jmp_short;
}

static void PREFIX86(_inaldx)(void)    /* Opcode 0xec */
{
	ICOUNT -= cycles.in_dx8;
	I.regs.b[AL] = read_port(I.regs.w[DX]);
}

static void PREFIX86(_inaxdx)(void)    /* Opcode 0xed */
{
	unsigned port = I.regs.w[DX];

	ICOUNT -= cycles.in_dx16;
	I.regs.b[AL] = read_port(port);
	I.regs.b[AH] = read_port(port+1);
}

static void PREFIX86(_outdxal)(void)    /* Opcode 0xee */
{
	ICOUNT -= cycles.out_dx8;
	write_port(I.regs.w[DX], I.regs.b[AL]);
}

static void PREFIX86(_outdxax)(void)    /* Opcode 0xef */
{
	unsigned port = I.regs.w[DX];

	ICOUNT -= cycles.out_dx16;
	write_port(port, I.regs.b[AL]);
	write_port(port+1, I.regs.b[AH]);
}

/* I think thats not a V20 instruction...*/
static void PREFIX86(_lock)(void)    /* Opcode 0xf0 */
{
	ICOUNT -= cycles.nop;
	if (trace_186) Dis186();
	PREFIX(_instruction)[FETCHOP]();  /* un-interruptible */
}

static void PREFIX(_repne)(void)    /* Opcode 0xf2 */
{
	 PREFIX(rep)(0);
}

static void PREFIX(_repe)(void)    /* Opcode 0xf3 */
{
	 PREFIX(rep)(1);
}

static void PREFIX86(_hlt)(void)    /* Opcode 0xf4 */
{
	I.pc--;
	ICOUNT = 0;
}

static void PREFIX86(_cmc)(void)    /* Opcode 0xf5 */
{
	ICOUNT -= cycles.flag_ops;
	I.CarryVal = !CF;
}

static void PREFIX86(_f6pre)(void)
{
	/* Opcode 0xf6 */
	unsigned ModRM = FETCH;
    unsigned tmp = (unsigned)GetRMByte(ModRM);
    unsigned tmp2;


    switch (ModRM & 0x38)
    {
    case 0x00:  /* TEST Eb, data8 */
    case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri8 : cycles.alu_mi8_ro;
		tmp &= FETCH;

		I.CarryVal = I.OverVal = I.AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;

    case 0x10:  /* NOT Eb */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.negnot_r8 : cycles.negnot_m8;
		PutbackRMByte(ModRM,~tmp);
		break;

	 case 0x18:  /* NEG Eb */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.negnot_r8 : cycles.negnot_m8;
        tmp2=0;
        SUBB(tmp2,tmp);
        PutbackRMByte(ModRM,tmp2);
		break;
    case 0x20:  /* MUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.mul_r8 : cycles.mul_m8;
		{
			unsigned result;
			tmp2 = I.regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (unsigned)tmp2*tmp;
			I.regs.w[AX]=(WORD)result;

			SetZF(I.regs.w[AX]);
			I.CarryVal = I.OverVal = (I.regs.b[AH] != 0);
		}
		break;
	 case 0x28:  /* IMUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.imul_r8 : cycles.imul_m8;
		{
			INT16 result;

			tmp2 = (unsigned)I.regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (INT16)((INT8)tmp2)*(INT16)((INT8)tmp);
			I.regs.w[AX]=(WORD)result;

			SetZF(I.regs.w[AX]);

			I.CarryVal = I.OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
    case 0x30:  /* DIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.div_r8 : cycles.div_m8;
		{
			unsigned result;

			result = I.regs.w[AX];

			if (tmp)
			{
				if ((result / tmp) > 0xff)
				{
					PREFIX(_interrupt)(0);
					break;
				}
				else
				{
					I.regs.b[AH] = result % tmp;
					I.regs.b[AL] = result / tmp;
				}
			}
			else
			{
				PREFIX(_interrupt)(0);
				break;
			}
		}
		break;
    case 0x38:  /* IDIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.idiv_r8 : cycles.idiv_m8;
		{

			INT16 result;

			result = I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT16)((INT8)tmp);

				if ((result /= (INT16)((INT8)tmp)) > 0xff)
				{
					PREFIX(_interrupt)(0);
					break;
				}
				else
				{
					I.regs.b[AL] = (unsigned char) result;
					I.regs.b[AH] = tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(0);
				break;
			}
		}
		break;
    }
}


static void PREFIX86(_f7pre)(void)
{
	/* Opcode 0xf7 */
	unsigned ModRM = FETCH;
	 unsigned tmp = GetRMWord(ModRM);
    unsigned tmp2;


    switch (ModRM & 0x38)
    {
    case 0x00:  /* TEST Ew, data16 */
    case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.alu_ri16 : cycles.alu_mi16_ro;
		tmp2 = FETCH;
		tmp2 += FETCH << 8;

		tmp &= tmp2;

		I.CarryVal = I.OverVal = I.AuxVal = 0;
		SetSZPF_Word(tmp);
		break;

    case 0x10:  /* NOT Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.negnot_r16 : cycles.negnot_m16;
		tmp = ~tmp;
		PutbackRMWord(ModRM,tmp);
		break;

    case 0x18:  /* NEG Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.negnot_r16 : cycles.negnot_m16;
        tmp2 = 0;
        SUBW(tmp2,tmp);
        PutbackRMWord(ModRM,tmp2);
		break;
    case 0x20:  /* MUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.mul_r16 : cycles.mul_m16;
		{
			UINT32 result;
			tmp2 = I.regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (UINT32)tmp2*tmp;
			I.regs.w[AX]=(WORD)result;
            result >>= 16;
			I.regs.w[DX]=result;

			SetZF(I.regs.w[AX] | I.regs.w[DX]);
			I.CarryVal = I.OverVal = (I.regs.w[DX] != 0);
		}
		break;

    case 0x28:  /* IMUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.imul_r16 : cycles.imul_m16;
		{
			INT32 result;

			tmp2 = I.regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (INT32)((INT16)tmp2)*(INT32)((INT16)tmp);
			I.CarryVal = I.OverVal = (result >> 15 != 0) && (result >> 15 != -1);

			I.regs.w[AX]=(WORD)result;
			result = (WORD)(result >> 16);
			I.regs.w[DX]=result;

			SetZF(I.regs.w[AX] | I.regs.w[DX]);
		}
		break;
	 case 0x30:  /* DIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.div_r16 : cycles.div_m16;
		{
			UINT32 result;

			result = (I.regs.w[DX] << 16) + I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % tmp;
				if ((result / tmp) > 0xffff)
				{
					PREFIX(_interrupt)(0);
					break;
				}
				else
				{
					I.regs.w[DX]=tmp2;
					result /= tmp;
					I.regs.w[AX]=result;
				}
			}
			else
			{
				PREFIX(_interrupt)(0);
				break;
			}
		}
		break;
    case 0x38:  /* IDIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.idiv_r16 : cycles.idiv_m16;
		{
			INT32 result;

			result = (I.regs.w[DX] << 16) + I.regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT32)((INT16)tmp);
				if ((result /= (INT32)((INT16)tmp)) > 0xffff)
				{
					PREFIX(_interrupt)(0);
					break;
				}
				else
				{
					I.regs.w[AX]=result;
					I.regs.w[DX]=tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(0);
				break;
			}
		}
		break;
    }
}


static void PREFIX86(_clc)(void)    /* Opcode 0xf8 */
{
	ICOUNT -= cycles.flag_ops;
	I.CarryVal = 0;
}

static void PREFIX86(_stc)(void)    /* Opcode 0xf9 */
{
	ICOUNT -= cycles.flag_ops;
	I.CarryVal = 1;
}

static void PREFIX86(_cli)(void)    /* Opcode 0xfa */
{
	ICOUNT -= cycles.flag_ops;
	SetIF(0);
}

static void PREFIX86(_sti)(void)    /* Opcode 0xfb */
{

//    WriteLog("Doing _sti - irq_state = %d", I.irq_state);
    ICOUNT -= cycles.flag_ops;
	SetIF(1);
	if (trace_186) Dis186();
	PREFIX(_instruction)[FETCHOP](); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if (I.irq_state)
    {
//        trace_186 = 1;
        PREFIX(_interrupt)(12);
    }
}

static void PREFIX86(_cld)(void)    /* Opcode 0xfc */
{
	ICOUNT -= cycles.flag_ops;
	SetDF(0);
}

static void PREFIX86(_std)(void)    /* Opcode 0xfd */
{
	ICOUNT -= cycles.flag_ops;
	SetDF(1);
}

static void PREFIX86(_fepre)(void)    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMByte(ModRM);
    unsigned tmp1;

	ICOUNT -= (ModRM >= 0xc0) ? cycles.incdec_r8 : cycles.incdec_m8;
    if ((ModRM & 0x38) == 0)  /* INC eb */
	 {
		tmp1 = tmp+1;
		SetOFB_Add(tmp1,tmp,1);
    }
	 else  /* DEC eb */
    {
		tmp1 = tmp-1;
		SetOFB_Sub(tmp1,1,tmp);
    }

    SetAF(tmp1,tmp,1);
    SetSZPF_Byte(tmp1);

    PutbackRMByte(ModRM,(BYTE)tmp1);
}


static void PREFIX86(_ffpre)(void)    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
    unsigned tmp;
    unsigned tmp1;
    WORD ip;

    switch(ModRM & 0x38)
    {
    case 0x00:  /* INC ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.incdec_r16 : cycles.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp+1;

		SetOFW_Add(tmp1,tmp,1);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

    case 0x08:  /* DEC ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.incdec_r16 : cycles.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp-1;

		SetOFW_Sub(tmp1,1,tmp);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

    case 0x10:  /* CALL ew */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.call_r16 : cycles.call_m16;
		tmp = GetRMWord(ModRM);
		ip = I.pc - I.base[CS];
		PUSH(ip);
		I.pc = (I.base[CS] + (WORD)tmp) & AMASK;
		CHANGE_PC(I.pc);
		break;

	case 0x18:  /* CALL FAR ea */
		ICOUNT -= cycles.call_m32;
		tmp = I.sregs[CS];	/* HJB 12/13/98 need to skip displacements of EA */
		tmp1 = GetRMWord(ModRM);
		ip = I.pc - I.base[CS];
		PUSH(tmp);
		PUSH(ip);
		I.sregs[CS] = GetnextRMWord;
		I.base[CS] = SegBase(CS);
		I.pc = (I.base[CS] + tmp1) & AMASK;
		CHANGE_PC(I.pc);
		break;

    case 0x20:  /* JMP ea */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.jmp_r16 : cycles.jmp_m16;
		ip = GetRMWord(ModRM);
		I.pc = (I.base[CS] + ip) & AMASK;
		CHANGE_PC(I.pc);
		break;

    case 0x28:  /* JMP FAR ea */
		ICOUNT -= cycles.jmp_m32;

		I.pc = GetRMWord(ModRM);
		I.sregs[CS] = GetnextRMWord;
		I.base[CS] = SegBase(CS);
		I.pc = (I.pc + I.base[CS]) & AMASK;
		CHANGE_PC(I.pc);
		break;

    case 0x30:  /* PUSH ea */
		ICOUNT -= (ModRM >= 0xc0) ? cycles.push_r16 : cycles.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	 }
}


static void PREFIX86(_invalid)(void)
{
	 /* makes the cpu loops forever until user resets it */
	/*{ extern int debug_key_pressed; debug_key_pressed = 1; } */
//	WriteLog("Illegal instruction 0x%.2x at 0x%.5x\n",PEEKBYTE(I.pc), I.pc);

    I.pc--;
	ICOUNT -= 10;

}


/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/


static void PREFIX186(_pusha)(void)    /* Opcode 0x60 */
{
	unsigned tmp=I.regs.w[SP];

	ICOUNT -= cycles.pusha;
	PUSH(I.regs.w[AX]);
	PUSH(I.regs.w[CX]);
	PUSH(I.regs.w[DX]);
	PUSH(I.regs.w[BX]);
    PUSH(tmp);
	PUSH(I.regs.w[BP]);
	PUSH(I.regs.w[SI]);
	PUSH(I.regs.w[DI]);
}

static void PREFIX186(_popa)(void)    /* Opcode 0x61 */
{
	 unsigned tmp;

	ICOUNT -= cycles.popa;
	POP(I.regs.w[DI]);
	POP(I.regs.w[SI]);
	POP(I.regs.w[BP]);
	POP(tmp);
	POP(I.regs.w[BX]);
	POP(I.regs.w[DX]);
	POP(I.regs.w[CX]);
	POP(I.regs.w[AX]);
}

static void PREFIX186(_bound)(void)    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (INT16)GetRMWord(ModRM);
    int high= (INT16)GetnextRMWord;
	int tmp= (INT16)RegWord(ModRM);
	if (tmp<low || tmp>high) {
		/* OB: on NECs CS:IP points to instruction
           FOLLOWING the BOUND instruction ! */
		I.pc-=2;
		PREFIX86(_interrupt)(5);
	}
	ICOUNT -= cycles.bound;
}

static void PREFIX186(_push_d16)(void)    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;

	ICOUNT -= cycles.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

static void PREFIX186(_imul_d16)(void)    /* Opcode 0x69 */
{
	DEF_r16w(dst,src);
	unsigned src2=FETCH;
	src+=(FETCH<<8);

	ICOUNT -= (ModRM >= 0xc0) ? cycles.imul_rri16 : cycles.imul_rmi16;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	I.CarryVal = I.OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}


static void PREFIX186(_push_d8)(void)    /* Opcode 0x6a */
{
	unsigned tmp = (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= cycles.push_imm;
	PUSH(tmp);
}

static void PREFIX186(_imul_d8)(void)    /* Opcode 0x6b */
{
	DEF_r16w(dst,src);
	unsigned src2= (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= (ModRM >= 0xc0) ? cycles.imul_rri8 : cycles.imul_rmi8;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	I.CarryVal = I.OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}

static void PREFIX186(_insb)(void)    /* Opcode 0x6c */
{
	ICOUNT -= cycles.ins8;
	PutMemB(ES,I.regs.w[DI],read_port(I.regs.w[DX]));
	I.regs.w[DI] += I.DirVal;
}

static void PREFIX186(_insw)(void)    /* Opcode 0x6d */
{
	ICOUNT -= cycles.ins16;
	PutMemB(ES,I.regs.w[DI],read_port(I.regs.w[DX]));
	PutMemB(ES,I.regs.w[DI]+1,read_port(I.regs.w[DX]+1));
	I.regs.w[DI] += 2 * I.DirVal;
}

static void PREFIX186(_outsb)(void)    /* Opcode 0x6e */
{
	ICOUNT -= cycles.outs8;
	write_port(I.regs.w[DX],GetMemB(DS,I.regs.w[SI]));
	I.regs.w[SI] += I.DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_outsw)(void)    /* Opcode 0x6f */
{
	ICOUNT -= cycles.outs16;
	write_port(I.regs.w[DX],GetMemB(DS,I.regs.w[SI]));
	write_port(I.regs.w[DX]+1,GetMemB(DS,I.regs.w[SI]+1));
	I.regs.w[SI] += 2 * I.DirVal; /* GOL 11/27/01 */
}

static void PREFIX186(_rotshft_bd8)(void)    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Byte)(ModRM,count);
}

static void PREFIX186(_rotshft_wd8)(void)    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Word)(ModRM,count);
}

static void PREFIX186(_enter)(void)    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;	 unsigned i,level;

	nb += FETCH << 8;
	level = FETCH;
	ICOUNT -= (level == 0) ? cycles.enter0 : (level == 1) ? cycles.enter1 : cycles.enter_base + level * cycles.enter_count;
	PUSH(I.regs.w[BP]);
	I.regs.w[BP]=I.regs.w[SP];
	I.regs.w[SP] -= nb;
	for (i=1;i<level;i++)
		PUSH(GetMemW(SS,I.regs.w[BP]-i*2));
	if (level) PUSH(I.regs.w[BP]);
}

static void PREFIX186(_leave)(void)    /* Opcode 0xc9 */
{
	ICOUNT -= cycles.leave;
	I.regs.w[SP]=I.regs.w[BP];
	POP(I.regs.w[BP]);
}

//<- [END OF INCLUDED FILE instr86.cpp]

void i86_init(void)
{
	unsigned int i, j, c;
	BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1)
				c++;

		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ((i & 0x38) >> 3);
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = (WREGS) (i & 7);
		Mod_RM.RM.b[i] = (BREGS) reg_name[i & 7];
	}

}

void i86_reset(void)
{
	memset(&I, 0, sizeof (I));

	I.sregs[CS] = 0xf000;
	I.base[CS] = SegBase(CS);
	I.pc = 0xffff0 & AMASK;
	ExpandFlags(I.flags);

	change_pc(I.pc);

	cycles = i186_cycles;
}

void i86_exit(void)
{
	/* nothing to do ? */
}

/* ASG 971222 -- added these interface functions */

static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (I.nmi_state == state)
			return;

		/* on a rising edge, signal the NMI */
		if (state != CLEAR_LINE)
		{
			I.nmi_state = state;
			PREFIX(_interrupt)(I86_NMI_INT_VECTOR);
		}
	}
	else
	{
        if (I.irq_state != state)
        {
    		/* if the IF is set, signal an interrupt */
	    	if ( (state != CLEAR_LINE) && I.IF)
            {
	            I.irq_state = state;
                PREFIX(_interrupt)(12);
            }
            else
            {
                I.irq_state = CLEAR_LINE;
            }
        }
	}
}

/* PJB 03/05 */
//->
//--static void set_test_line(int state)
//--{
//--        I.test_state = !state;
//--}
//<-

void Dis186(void)

{
char buffer[256];
char buff[256];
int le, i;

	le = i386_dasm_one(buffer, I.pc, 0, 0) & 0xff;
            
    sprintf(buff, "[%04x:%04x] AX=%04x BX=%04x CX=%04x DX=%04x SI=%04x DI=%04x DS=%04x ES=%04x SS:SP = %04x:%04x ", 
        I.sregs[CS], (I.pc - I.base[CS]) & 0xffff, 
        I.regs.w[AX], I.regs.w[BX], I.regs.w[CX], I.regs.w[DX], I.regs.w[SI], I.regs.w[DI], I.sregs[DS], I.sregs[ES],
		I.sregs[SS], I.regs.w[SP]);
	
	for (i = 0; i < le; ++i) 
	{
		sprintf(buff + strlen(buff), "%02x ", ReadByte(I.pc + i));
	}
        
	for (i = le; i < 10; ++i) 
	{
		sprintf(buff + strlen(buff), "   ");
	}

	sprintf(buff + strlen(buff), "%s", buffer); 
//--    WriteLog(buff);
	printf(buff);

}

int i186_execute(int num_cycles)
{
static int lastInt = 0;

	/* adjust for any interrupts that came in */
	i86_ICount = num_cycles;
	i86_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	/* run until we're out */
	while (i86_ICount > 0)
	{

		seg_prefix = FALSE;
		I.prevpc = I.pc;

        TABLE186;

        if (TubeintStatus & (1<<R1))
            set_irq_line(INPUT_LINE_IRQ4, ASSERT_LINE1);

        if (TubeintStatus & (1<<R4))
            set_irq_line(INPUT_LINE_IRQ4, ASSERT_LINE4);

        if (TubeintStatus == 0)
            set_irq_line(INPUT_LINE_IRQ4, CLEAR_LINE);
        
        lastInt = TubeintStatus;

// DRQ and DMA transfer
        
        if (TubeNMIStatus) DoDMA();

	}

	/* adjust for any interrupts that came in */
	i86_ICount -= I.extra_cycles;
	I.extra_cycles = 0;

	return num_cycles - i86_ICount;
}

offs_t i186_dasm(char *buffer, offs_t pc, UINT8 *oprom, UINT8 *opram, int bytes)
{
	sprintf(buffer, "$%02X", cpu_readop(pc));
	return 1;
}

extern unsigned char ReadTubeFromParasiteSide(unsigned char IOAddr);
extern void WriteTubeFromParasiteSide(unsigned char IOAddr,unsigned char IOData);


UINT8 io_read_byte_8(offs_t address)
{
int i;
UINT8 data;

    if ( (address >= 0x80) && (address <= 0x8f) )
    {
        i = (address - 0x80) / 2;
        data = ReadTubeFromParasiteSide(i);
    }
    else
	{
        data = 0xff;
	}

    return data;
}

int dma_src_segment = 0x00;
int dma_src_offset = 0x00;
offs_t dma_src = 0x00;
int dma_dst_segment = 0x00;
int dma_dst_offset = 0x00;
offs_t dma_dst = 0x00;
int dma_count = 0x00;
int dma_control = 0x00;

void io_write_byte_8(offs_t address, UINT8 data)
{
int i;

    if ( (address >= 0xffc0) && (address <= 0xffcf) )
    {

        switch (address - 0xffc0)
        {
        case 0x00 :
            dma_src_offset = (dma_src_offset & 0xff00) | data;
            break;
        case 0x01 :
            dma_src_offset = (dma_src_offset & 0xff) | (data << 8);
	        dma_src = ((dma_src_segment & 0x0f) << 16) + dma_src_offset;
            break;
        case 0x02 :
            dma_src_segment = (dma_src_segment & 0xff00) | data;
            break;
        case 0x03 :
            dma_src_segment = (dma_src_segment & 0xff) | (data << 8);
	        dma_src = ((dma_src_segment & 0x0f) << 16) + dma_src_offset;
            break;
        case 0x04 :
            dma_dst_offset = (dma_dst_offset & 0xff00) | data;
            break;
        case 0x05 :
            dma_dst_offset = (dma_dst_offset & 0xff) | (data << 8);
	        dma_dst = ((dma_dst_segment & 0x0f) << 16) + dma_dst_offset;
            break;
        case 0x06 :
            dma_dst_segment = (dma_dst_segment & 0xff00) | data;
            break;
        case 0x07 :
            dma_dst_segment = (dma_dst_segment & 0xff) | (data << 8);
	        dma_dst = ((dma_dst_segment & 0x0f) << 16) + dma_dst_offset;
            break;
        case 0x08 :
            dma_count = (dma_count & 0xff00) | data;
            break;
        case 0x09 :
            dma_count = (dma_count & 0xff) | (data << 8);
            break;
        case 0x0a :
            dma_control = (dma_control & 0xff00) | data;
			break;
        case 0x0b :
            dma_control = (dma_control & 0xff) | (data << 8);
			break;
        case 0x0c :
            break;
        case 0x0d :
            break;
        case 0x0e :
            break;
        case 0x0f :
            break;
        }
        return;
    }

    if ( (address >= 0x80) && (address <= 0x8f) )
    {
        i = (address - 0x80) / 2;

        WriteTubeFromParasiteSide( i, data);

        return;

    }

}

void i86_main(void)

{
FILE *f;
char path[256];

	//printf("****** got to i86_main\n");

    opcode_base = (unsigned char *) malloc(0x100000L);


	//printf("%ld\n", opcode_base);

    memset(opcode_base, 0, 0x100000L);    



	//printf("(*** The memset worked\n");

	strcpy(path, RomPath);
//--	strcat(path, "/BeebFile/bios.rom");
	strcat(path, "other/bios.rom");


	//printf("got filename %s\n", path);
	pINFO(dL"Loading M512 BIOS ROM: '%s'.", dR, path);

    f = fopen(path, "rb");
    fseek(f, 0, SEEK_SET);
    fread(opcode_base + 0xf0000L, 16384, 1, f);
    fseek(f, 0, SEEK_SET);
    fread(opcode_base + 0xf4000L, 16384, 1, f);
    fseek(f, 0, SEEK_SET);
    fread(opcode_base + 0xf8000L, 16384, 1, f);
    fseek(f, 0, SEEK_SET);
    fread(opcode_base + 0xfc000L, 16384, 1, f);
    fclose(f);

	//printf("****** loaded %s \n", path);

	//printf("calling i86_inti()\n");
    i86_init();
	//printf("calling i86_reset()\n");
    i86_reset();
	//printf("calling ResetTube()\n");
    R1Status=0;
    ResetTube();

}

void MemoryDump(int addr, int count)
{
	int a, b;
	int s, e;
	int v;
	char info[80];

	s = addr & 0xffff0;
	e = (addr + count - 1) | 0xf;
	if (e > 0xfffff)
		e = 0xfffff;
	for (a = s; a < e; a += 16)
	{
		sprintf(info, "%04X  ", a);

		for (b = 0; b < 16; ++b)
		{
			sprintf(info+strlen(info), "%02X ", program_read_byte_8(a + b));
		}

		for (b = 0; b < 16; ++b)
		{
			v = program_read_byte_8(a + b);
			if (v < 32 || v > 127)
				v = '.';
			sprintf(info+strlen(info), "%c", v);
		}

//--		WriteLog("%s", info);
		printf("%s", info);
	}

}

void DoDMA(void)

{
UINT8 data;

    if (dma_src < 0x100)		// I/O to MEM
    {
        data = io_read_byte_8(dma_src);
        program_write_byte_8(dma_dst, data);
		dma_dst++;

    }
    else						// MEM to I/O
    {
		data = program_read_byte_8(dma_src);
		io_write_byte_8(dma_dst, data);
		dma_src++;
    }
}
