/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"

#include "../vapi.h"
#include "../vmachine.h"
#include "aasm.h"

typedef enum {
	TYPE_NONE,
	TYPE_R8,
	TYPE_R16,
	TYPE_SEG,
	TYPE_I8,
	TYPE_I16,
	TYPE_M,
	TYPE_M8,
	TYPE_M16
} t_aasm_oprtype;
typedef enum {
	MOD_M,
	MOD_M_I8,
	MOD_M_I16,
	MOD_R
} t_aasm_oprmod;
typedef enum {
	MEM_BX_SI,
	MEM_BX_DI,
	MEM_BP_SI,
	MEM_BP_DI,
	MEM_SI,
	MEM_DI,
	MEM_BP,
	MEM_BX
} t_aasm_oprmem;
typedef enum {
	R8_AL,
	R8_CL,
	R8_DL,
	R8_BL,
	R8_AH,
	R8_CH,
	R8_DH,
	R8_BH
} t_aasm_oprreg8;
typedef enum {
	R16_AX,
	R16_CX,
	R16_DX,
	R16_BX,
	R16_SP,
	R16_BP,
	R16_SI,
	R16_DI
} t_aasm_oprreg16;
typedef enum {
	SEG_ES,
	SEG_CS,
	SEG_SS,
	SEG_DS
} t_aasm_oprseg;
typedef enum {
	PTR_NEAR,
	PTR_FAR
} t_aasm_oprptr;

typedef struct {
	t_aasm_oprtype  type;  // 0 = none, 1 = reg8, 2 = reg16, 3 = seg,
                           // 4 = imm8, 5 = imm16, 6 = mem, 7 = mem8, 8 = mem16
	t_aasm_oprmod   mod;   // active when type = 1,2,3,6,7,8
	                       // 0 = mem; 1 = mem+imm8; 2 = mem+imm16; 3 = reg
	t_aasm_oprmem   mem;   // active when mod = 0,1,2
                           // 0 = [BX+SI], 1 = [BX+DI], 2 = [BP+SI], 3 = [BP+DI],
                           // 4 = [SI], 5 = [DI], 6 = [BP], 7 = [BX]
	t_aasm_oprreg8  reg8;  // active when type = 1, mod = 3
                           // 0 = AL, 1 = CL, 2 = DL, 3 = BL,
                           // 4 = AH, 5 = CH, 6 = DH, 7 = BH
	t_aasm_oprreg16 reg16; // active when type = 2, mod = 3
                           // 0 = AX, 1 = CX, 2 = DX, 3 = BX,
                           // 4 = SP, 5 = BP, 6 = SI, 7 = DI      
	t_aasm_oprseg   seg;   // active when type = 3
                           //0 = ES, 1 = CS, 2 = SS, 3 = DS
	t_nsbit8        imm8s;
	t_nubit8        imm8u;
	t_nubit16       imm16;
	t_nsbit8        disp8;
	t_nubit16       disp16;// use as imm when type = 5,6; use by modrm as disp when mod = 0(rm = 6),1,2;
	t_aasm_oprptr   isfar; // 0 = near; 1 = far
} t_aasm_oprinfo;

static char pool[0x1000];
static t_bool error;
static t_aasm_oprinfo aopri1, aopri2;
static t_string astmt, aop, aopr1, aopr2;
static t_nubit16 avcs, avip;

#define isNONE(oprinf)  ((oprinf).type  == TYPE_NONE)
#define isR8(oprinf)    ((oprinf).type  == TYPE_R8  && (oprinf).mod == MOD_R)
#define isR16(oprinf)   ((oprinf).type  == TYPE_R16 && (oprinf).mod == MOD_R)
#define isSEG(oprinf)   ((oprinf).type  == TYPE_SEG && (oprinf).mod == MOD_R)
#define isI8(oprinf)    ((oprinf).type  == TYPE_I8)
#define isI16(oprinf)   ((oprinf).type  == TYPE_I8  || (oprinf).type == TYPE_I16)
#define isM8(oprinf)    (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M8 ) && (oprinf).mod != MOD_R)
#define isM16(oprinf)   (((oprinf).type == TYPE_M   || (oprinf).type == TYPE_M16) && (oprinf).mod != MOD_R)
#define isNEAR(oprinf)  ((oprinf).isfar == PTR_NEAR)
#define isFAR(oprinf)   ((oprinf).isfar == PTR_FAR)

#define isRM8(oprinf)   (isR8 (oprinf) || isM8 (oprinf))
#define isRM16(oprinf)  (isR16(oprinf) || isM16(oprinf))
#define isRM(oprinf)    (isRM8(oprinf) || isRM16(oprinf))
#define isAL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_AL)
#define isCL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_CL)
#define isDL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_DL)
#define isBL(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_BL)
#define isAH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_AH)
#define isCH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_CH)
#define isDH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_DH)
#define isBH(oprinf)    (isR8 (oprinf) && (oprinf).reg8  == R8_BH)
#define isAX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_AX)
#define isCX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_CX)
#define isDX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_DX)
#define isBX(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_BX)
#define isSP(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_SP)
#define isBP(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_BP)
#define isSI(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_SI)
#define isDI(oprinf)    (isR16(oprinf) && (oprinf).reg16 == R16_DI)
#define isES(oprinf)    (isSEG(oprinf) && (oprinf).seg   == SEG_ES)
#define isCS(oprinf)    (isSEG(oprinf) && (oprinf).seg   == SEG_CS)
#define isSS(oprinf)    (isSEG(oprinf) && (oprinf).seg   == SEG_SS)
#define isDS(oprinf)    (isSEG(oprinf) && (oprinf).seg   == SEG_DS)

#define ARG_NONE        (isNONE(aopri1) && isNONE(aopri2))
#define ARG_RM8_R8      (isRM8 (aopri1) && isR8  (aopri2))
#define ARG_RM16_R16    (isRM16(aopri1) && isR16 (aopri2))
#define ARG_R8_RM8      (isR8  (aopri1) && isRM8 (aopri2))
#define ARG_R16_RM16    (isR16 (aopri1) && isRM16(aopri2))
#define ARG_ES          (isES  (aopri1) && isNONE(aopri2))
#define ARG_CS          (isCS  (aopri1) && isNONE(aopri2))
#define ARG_SS          (isSS  (aopri1) && isNONE(aopri2))
#define ARG_DS          (isDS  (aopri1) && isNONE(aopri2))
#define ARG_AX          (isAX  (aopri1) && isNONE(aopri2))
#define ARG_CX          (isCX  (aopri1) && isNONE(aopri2))
#define ARG_DX          (isDX  (aopri1) && isNONE(aopri2))
#define ARG_BX          (isBX  (aopri1) && isNONE(aopri2))
#define ARG_SP          (isSP  (aopri1) && isNONE(aopri2))
#define ARG_BP          (isBP  (aopri1) && isNONE(aopri2))
#define ARG_SI          (isSI  (aopri1) && isNONE(aopri2))
#define ARG_DI          (isDI  (aopri1) && isNONE(aopri2))
#define ARG_AL_I8       (isAL  (aopri1) && isI8  (aopri2))
#define ARG_CL_I8       (isCL  (aopri1) && isI8  (aopri2))
#define ARG_DL_I8       (isDL  (aopri1) && isI8  (aopri2))
#define ARG_BL_I8       (isBL  (aopri1) && isI8  (aopri2))
#define ARG_AH_I8       (isAH  (aopri1) && isI8  (aopri2))
#define ARG_CH_I8       (isCH  (aopri1) && isI8  (aopri2))
#define ARG_DH_I8       (isDH  (aopri1) && isI8  (aopri2))
#define ARG_BH_I8       (isBH  (aopri1) && isI8  (aopri2))
#define ARG_AX_I16      (isAX  (aopri1) && isI16 (aopri2))
#define ARG_CX_I16      (isCX  (aopri1) && isI16 (aopri2))
#define ARG_DX_I16      (isDX  (aopri1) && isI16 (aopri2))
#define ARG_BX_I16      (isBX  (aopri1) && isI16 (aopri2))
#define ARG_SP_I16      (isSP  (aopri1) && isI16 (aopri2))
#define ARG_BP_I16      (isBP  (aopri1) && isI16 (aopri2))
#define ARG_SI_I16      (isSI  (aopri1) && isI16 (aopri2))
#define ARG_DI_I16      (isDI  (aopri1) && isI16 (aopri2))
#define ARG_I8          (isI8  (aopri1) && isNONE(aopri2))
#define ARG_RM8_I8      (isRM8 (aopri1) && isI8  (aopri2))
#define ARG_RM16_I16    (isRM16(aopri1) && isI16 (aopri2))
#define ARG_RM16_I8     (isRM16(aopri1) && isI8  (aopri2))
#define ARG_RM16_SEG    (isRM16(aopri1) && isSEG (aopri2))
#define ARG_SEG_RM16    (isSEG (aopri1) && isRM16(aopri2))
#define ARG_RM16        (isRM16(aopri1) && isNONE(aopri2))
#define ARG_CX_AX       (isCX  (aopri1) && isAX  (aopri2))
#define ARG_DX_AX       (isDX  (aopri1) && isAX  (aopri2))
#define ARG_BX_AX       (isBX  (aopri1) && isAX  (aopri2))
#define ARG_SP_AX       (isSP  (aopri1) && isAX  (aopri2))
#define ARG_BP_AX       (isBP  (aopri1) && isAX  (aopri2))
#define ARG_SI_AX       (isSI  (aopri1) && isAX  (aopri2))
#define ARG_DI_AX       (isDI  (aopri1) && isAX  (aopri2))
#define ARG_AL_M8       (isAL  (aopri1) && isM8  (aopri2))
#define ARG_M8_AL       (isM8  (aopri1) && isAL  (aopri2))
#define ARG_AX_M16      (isAX  (aopri1) && isM16 (aopri2))
#define ARG_M16_AX      (isM16 (aopri1) && isAX  (aopri2))

#define setbyte(n) (vramVarByte(avcs, avip) = (t_nubit8)(n))
#define setword(n) (vramVarWord(avcs, avip) = (t_nubit16)(n))

static void SetImm8(t_nubit8 byte)
{
	setbyte(byte);
	avip += 1;
}
static void SetImm16(t_nubit16 word)
{
	setword(word);
	avip += 2;
}
static void SetModRegRM(t_aasm_oprinfo modrm, t_nubit8 reg)
{
	t_nubit8 modregrm = ((t_nubit8)modrm.mod << 6) | (reg << 3);
	switch(modrm.mod) {
	case MOD_M:
		modregrm |= (t_nubit8)modrm.mem;
		setbyte(modregrm); avip++;
		switch(modrm.mem) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: setword(modrm.disp16);avip += 2;break;
		case 7: break;
		default:error = 1;break;}
		break;
	case MOD_M_I8:
		modregrm |= (t_nubit8)modrm.mem;
		setbyte(modregrm); avip++;
		setbyte(modrm.disp8); avip++;
		switch(modrm.mem) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
		default:error = 1;break;}
		break;
	case MOD_M_I16:
		modregrm |= (t_nubit8)modrm.mem;
		setbyte(modregrm); avip++;
		setword(modrm.disp16); avip+= 2;
		switch(modrm.mem) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
		default:error = 1;break;}
		break;
	case 3:
		switch (modrm.type) {
		case TYPE_R8:
			modregrm |= (t_nubit8)modrm.reg8;
			setbyte(modregrm); avip++;
			switch(modrm.reg8) {
			case 0: break;
			case 1: break;
			case 2: break;
			case 3: break;
			case 4: break;
			case 5: break;
			case 6: break;
			case 7: break;
			default:error = 1;break;}
			break;
		case TYPE_R16:
			modregrm |= (t_nubit8)modrm.reg16;
			setbyte(modregrm); avip++;
			switch(modrm.reg16) {
			case 0: break;
			case 1: break;
			case 2: break;
			case 3: break;
			case 4: break;
			case 5: break;
			case 6: break;
			case 7: break;
			default:error = 1;break;}
			break;
		}
		break;
	default:error = 1;break;}
}

static void ES()
{
	if (!ARG_NONE) return;
	setbyte(0x26);
	avip++;
}
static void CS()
{
	if (!ARG_NONE) return;
	setbyte(0x2e);
	avip++;
}
static void SS()
{
	if (!ARG_NONE) return;
	setbyte(0x36);
	avip++;
}
static void DS()
{
	if (!ARG_NONE) return;
	setbyte(0x3e);
	avip++;
}
static void AAA()
{
	if (!ARG_NONE) return;
	setbyte(0x37);
	avip++;
}
static void AAD()
{
	if (!ARG_NONE) return;
	setbyte(0xd5);
	avip++;
	SetImm8(0x0a);
}
static void AAM()
{
	if (!ARG_NONE) return;
	setbyte(0xd4);
	avip++;
	SetImm8(0x0a);
}
static void AAS()
{
	if (!ARG_NONE) return;
	setbyte(0x3f);
	avip++;
}

static void ADD_RM8_R8()
{
	setbyte(0x00);
	avip++;
	SetModRegRM(aopri1, aopri2.reg8);
}
static void ADD_RM16_R16()
{
	setbyte(0x01);
	avip++;
	SetModRegRM(aopri1, aopri2.reg16);
}
static void ADD_R8_RM8()
{
	setbyte(0x02);
	avip++;
	SetModRegRM(aopri2, aopri1.reg8);
}
static void ADD_R16_RM16()
{
	setbyte(0x03);
	avip++;
	SetModRegRM(aopri2, aopri1.reg16);
}
static void ADD_AL_I8()
{
	setbyte(0x04);
	avip++;
	SetImm8(aopri2.imm8u);
}
static void ADD_AX_I16()
{
	setbyte(0x05);
	avip++;
	SetImm16(aopri2.imm16);
}
static void INS_80(t_nubit8 rid)
{
	setbyte(0x80);
	avip++;
	SetModRegRM(aopri1, rid);
	SetImm8(aopri2.imm8u);
}
static void INS_81(t_nubit8 rid)
{
	setbyte(0x81);
	avip++;
	SetModRegRM(aopri1, rid);
	SetImm16(aopri2.imm16);
}
static void INS_83(t_nubit8 rid)
{
	setbyte(0x83);
	avip++;
	SetModRegRM(aopri1, rid);
	SetImm8(aopri2.imm8s);
}

static void ADD()
{
	if      (ARG_AL_I8)    ADD_AL_I8();
	else if (ARG_AX_I16)   ADD_AX_I16();
	else if (ARG_RM8_I8)   INS_80(0x00);
	else if (ARG_RM16_I8)  INS_81(0x00);
	else if (ARG_RM16_I16) INS_83(0x00);
	else if (ARG_RM8_R8)   ADD_RM8_R8();
	else if (ARG_RM16_R16) ADD_RM16_R16();
	else if (ARG_R8_RM8)   ADD_R8_RM8();
	else if (ARG_R16_RM16) ADD_R16_RM16();
	else error = 1;
}

typedef enum {
	STATE_START,
	        STATE_BY,STATE_BYT, /* BYTE */
	STATE_W,STATE_WO,STATE_WOR, /* WORD */
	STATE_P,STATE_PT,           /* PTR */
	STATE_N,STATE_NE,STATE_NEA, /* NEAR */
	        STATE_FA,           /* FAR */
	        STATE_SH,STATE_SHO, /* SHORT */
	STATE_SHOR,
	STATE_A,                    /* AX, AH, AL, NUM */
	STATE_B,                    /* BX, BH, BL, BP, NUM */
	STATE_C,                    /* CX, CH, CL, CS, NUM */
	STATE_D,                    /* DX, DH, DL, DS, DI, NUM */
	STATE_E,                    /* ES, NUM */
	STATE_F,                    /* NUM, FAR */
	STATE_S,                    /* SS, SP, SI, SHORT */
	STATE_NUM1,                 /* NUM */
	STATE_NUM2,
	STATE_NUM3
} t_aasm_scan_state;
/*typedef enum {
	STATE_BYTE, STATE_BYTE_PTR,
} t_aasm_parse_state;*/
typedef enum {
	TOKEN_NULL,TOKEN_END,
	TOKEN_LSPAREN,TOKEN_RSPAREN,
	TOKEN_COLON,TOKEN_PLUS,TOKEN_MINUS,
	TOKEN_BYTE,TOKEN_WORD,TOKEN_PTR,
	TOKEN_NEAR,TOKEN_FAR,TOKEN_SHORT,
	TOKEN_IMM8,TOKEN_IMM16,
	TOKEN_AH,TOKEN_BH,TOKEN_CH,TOKEN_DH,
	TOKEN_AL,TOKEN_BL,TOKEN_CL,TOKEN_DL,
	TOKEN_AX,TOKEN_BX,TOKEN_CX,TOKEN_DX,
	TOKEN_SP,TOKEN_BP,TOKEN_SI,TOKEN_DI,
	TOKEN_CS,TOKEN_DS,TOKEN_ES,TOKEN_SS
} t_aasm_token;

#define tokch  (*tokptr)
#define take(n) (flagend = 0x01, token = (n))

static t_nubit8 tokimm8;
static t_nubit16 tokimm16;
static t_aasm_token gettoken(t_string str)
{
	static t_string tokptr = NULL;
	t_bool flagend = 0x00;
	t_aasm_token token = TOKEN_NULL;
	t_aasm_scan_state state = STATE_START;
	tokimm8 = 0x00;
	tokimm16 = 0x0000;
	if (str) tokptr = str;
	if (!tokptr) return token;
	do {
		switch (state) {
		case STATE_START:
			switch (tokch) {
			case '[': take(TOKEN_LSPAREN); break;
			case ']': take(TOKEN_RSPAREN); break;
			case ':': take(TOKEN_COLON);   break;
			case '+': take(TOKEN_PLUS);    break;
			case '-': take(TOKEN_MINUS);   break;
			case '0': tokimm8 = 0x00;state = STATE_NUM1;break;
			case '1': tokimm8 = 0x01;state = STATE_NUM1;break;
			case '2': tokimm8 = 0x02;state = STATE_NUM1;break;
			case '3': tokimm8 = 0x03;state = STATE_NUM1;break;
			case '4': tokimm8 = 0x04;state = STATE_NUM1;break;
			case '5': tokimm8 = 0x05;state = STATE_NUM1;break;
			case '6': tokimm8 = 0x06;state = STATE_NUM1;break;
			case '7': tokimm8 = 0x07;state = STATE_NUM1;break;
			case '8': tokimm8 = 0x08;state = STATE_NUM1;break;
			case '9': tokimm8 = 0x09;state = STATE_NUM1;break;
			case 'a': tokimm8 = 0x0a;state = STATE_A;break;
			case 'b': tokimm8 = 0x0b;state = STATE_B;break;
			case 'c': tokimm8 = 0x0c;state = STATE_C;break;
			case 'd': tokimm8 = 0x0d;state = STATE_D;break;
			case 'e': tokimm8 = 0x0e;state = STATE_E;break;
			case 'f': tokimm8 = 0x0f;state = STATE_F;break;
			case 'n': state = STATE_N;break;
			case 'p': state = STATE_P;break;
			case 's': state = STATE_S;break;
			case 'w': state = STATE_W;break;
			case ' ':
			case '\t': break;
			case '\0': tokptr--;take(TOKEN_END);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_NUM1:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
		case STATE_NUM2:
			tokimm16 = (t_nubit16)tokimm8;
			switch (tokch) {
			case '0': tokimm16 = (tokimm16 << 4) | 0x0000;state = STATE_NUM3;break;
			case '1': tokimm16 = (tokimm16 << 4) | 0x0001;state = STATE_NUM3;break;
			case '2': tokimm16 = (tokimm16 << 4) | 0x0002;state = STATE_NUM3;break;
			case '3': tokimm16 = (tokimm16 << 4) | 0x0003;state = STATE_NUM3;break;
			case '4': tokimm16 = (tokimm16 << 4) | 0x0004;state = STATE_NUM3;break;
			case '5': tokimm16 = (tokimm16 << 4) | 0x0005;state = STATE_NUM3;break;
			case '6': tokimm16 = (tokimm16 << 4) | 0x0006;state = STATE_NUM3;break;
			case '7': tokimm16 = (tokimm16 << 4) | 0x0007;state = STATE_NUM3;break;
			case '8': tokimm16 = (tokimm16 << 4) | 0x0008;state = STATE_NUM3;break;
			case '9': tokimm16 = (tokimm16 << 4) | 0x0009;state = STATE_NUM3;break;
			case 'a': tokimm16 = (tokimm16 << 4) | 0x000a;state = STATE_NUM3;break;
			case 'b': tokimm16 = (tokimm16 << 4) | 0x000b;state = STATE_NUM3;break;
			case 'c': tokimm16 = (tokimm16 << 4) | 0x000c;state = STATE_NUM3;break;
			case 'd': tokimm16 = (tokimm16 << 4) | 0x000d;state = STATE_NUM3;break;
			case 'e': tokimm16 = (tokimm16 << 4) | 0x000e;state = STATE_NUM3;break;
			case 'f': tokimm16 = (tokimm16 << 4) | 0x000f;state = STATE_NUM3;break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
		case STATE_NUM3:
			switch (tokch) {
			case '0': tokimm16 = (tokimm16 << 4) | 0x0000;break;
			case '1': tokimm16 = (tokimm16 << 4) | 0x0001;break;
			case '2': tokimm16 = (tokimm16 << 4) | 0x0002;break;
			case '3': tokimm16 = (tokimm16 << 4) | 0x0003;break;
			case '4': tokimm16 = (tokimm16 << 4) | 0x0004;break;
			case '5': tokimm16 = (tokimm16 << 4) | 0x0005;break;
			case '6': tokimm16 = (tokimm16 << 4) | 0x0006;break;
			case '7': tokimm16 = (tokimm16 << 4) | 0x0007;break;
			case '8': tokimm16 = (tokimm16 << 4) | 0x0008;break;
			case '9': tokimm16 = (tokimm16 << 4) | 0x0009;break;
			case 'a': tokimm16 = (tokimm16 << 4) | 0x000a;break;
			case 'b': tokimm16 = (tokimm16 << 4) | 0x000b;break;
			case 'c': tokimm16 = (tokimm16 << 4) | 0x000c;break;
			case 'd': tokimm16 = (tokimm16 << 4) | 0x000d;break;
			case 'e': tokimm16 = (tokimm16 << 4) | 0x000e;break;
			case 'f': tokimm16 = (tokimm16 << 4) | 0x000f;break;
			default: tokptr--;break;
			}
			take(TOKEN_IMM16);
			break;
		case STATE_A:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			case 'x': take(TOKEN_AX);break;
			case 'h': take(TOKEN_AH);break;
			case 'l': take(TOKEN_AL);break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_B:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			case 'x': take(TOKEN_BX);break;
			case 'h': take(TOKEN_BH);break;
			case 'l': take(TOKEN_BL);break;
			case 'p': take(TOKEN_BP);break;
			case 'y': state = STATE_BY;break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_C:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			case 'x': take(TOKEN_CX);break;
			case 'h': take(TOKEN_CH);break;
			case 'l': take(TOKEN_CL);break;
			case 's': take(TOKEN_CS);break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_D:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			case 'x': take(TOKEN_DX);break;
			case 'h': take(TOKEN_DH);break;
			case 'l': take(TOKEN_DL);break;
			case 's': take(TOKEN_DS);break;
			case 'i': take(TOKEN_DI);break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_E:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_NUM2;break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			case 's': take(TOKEN_ES);break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_F:
			switch (tokch) {
			case '0': tokimm8 = (tokimm8 << 4) | 0x00;state = STATE_NUM2;break;
			case '1': tokimm8 = (tokimm8 << 4) | 0x01;state = STATE_NUM2;break;
			case '2': tokimm8 = (tokimm8 << 4) | 0x02;state = STATE_NUM2;break;
			case '3': tokimm8 = (tokimm8 << 4) | 0x03;state = STATE_NUM2;break;
			case '4': tokimm8 = (tokimm8 << 4) | 0x04;state = STATE_NUM2;break;
			case '5': tokimm8 = (tokimm8 << 4) | 0x05;state = STATE_NUM2;break;
			case '6': tokimm8 = (tokimm8 << 4) | 0x06;state = STATE_NUM2;break;
			case '7': tokimm8 = (tokimm8 << 4) | 0x07;state = STATE_NUM2;break;
			case '8': tokimm8 = (tokimm8 << 4) | 0x08;state = STATE_NUM2;break;
			case '9': tokimm8 = (tokimm8 << 4) | 0x09;state = STATE_NUM2;break;
			case 'a': tokimm8 = (tokimm8 << 4) | 0x0a;state = STATE_FA;  break;
			case 'b': tokimm8 = (tokimm8 << 4) | 0x0b;state = STATE_NUM2;break;
			case 'c': tokimm8 = (tokimm8 << 4) | 0x0c;state = STATE_NUM2;break;
			case 'd': tokimm8 = (tokimm8 << 4) | 0x0d;state = STATE_NUM2;break;
			case 'e': tokimm8 = (tokimm8 << 4) | 0x0e;state = STATE_NUM2;break;
			case 'f': tokimm8 = (tokimm8 << 4) | 0x0f;state = STATE_NUM2;break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_N:
			switch (tokch) {
			case 'e': state = STATE_NE;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_P:
			switch (tokch) {
			case 't': state = STATE_PT;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_S:
			switch (tokch) {
			case 'i': take(TOKEN_SI);break;
			case 'p': take(TOKEN_SP);break;
			case 's': take(TOKEN_SS);break;
			case 'h': state = STATE_SH;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_W:
			switch (tokch) {
			case 'o': state = STATE_WO;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_BY:
			switch (tokch) {
			case 't': state = STATE_BYT;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_FA:
			tokimm16 = (t_nubit16)tokimm8;
			switch (tokch) {
			case '0': tokimm16 = (tokimm16 << 4) | 0x0000;state = STATE_NUM3;break;
			case '1': tokimm16 = (tokimm16 << 4) | 0x0001;state = STATE_NUM3;break;
			case '2': tokimm16 = (tokimm16 << 4) | 0x0002;state = STATE_NUM3;break;
			case '3': tokimm16 = (tokimm16 << 4) | 0x0003;state = STATE_NUM3;break;
			case '4': tokimm16 = (tokimm16 << 4) | 0x0004;state = STATE_NUM3;break;
			case '5': tokimm16 = (tokimm16 << 4) | 0x0005;state = STATE_NUM3;break;
			case '6': tokimm16 = (tokimm16 << 4) | 0x0006;state = STATE_NUM3;break;
			case '7': tokimm16 = (tokimm16 << 4) | 0x0007;state = STATE_NUM3;break;
			case '8': tokimm16 = (tokimm16 << 4) | 0x0008;state = STATE_NUM3;break;
			case '9': tokimm16 = (tokimm16 << 4) | 0x0009;state = STATE_NUM3;break;
			case 'a': tokimm16 = (tokimm16 << 4) | 0x000a;state = STATE_NUM3;break;
			case 'b': tokimm16 = (tokimm16 << 4) | 0x000b;state = STATE_NUM3;break;
			case 'c': tokimm16 = (tokimm16 << 4) | 0x000c;state = STATE_NUM3;break;
			case 'd': tokimm16 = (tokimm16 << 4) | 0x000d;state = STATE_NUM3;break;
			case 'e': tokimm16 = (tokimm16 << 4) | 0x000e;state = STATE_NUM3;break;
			case 'f': tokimm16 = (tokimm16 << 4) | 0x000f;state = STATE_NUM3;break;
			case 'r': take(TOKEN_FAR);break;
			default: tokptr--;take(TOKEN_IMM8);break;
			}
			break;
		case STATE_NE:
			switch (tokch) {
			case 'a': state = STATE_NEA;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_PT:
			switch (tokch) {
			case 'r': take(TOKEN_PTR);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_SH:
			switch (tokch) {
			case 'o': state = STATE_SHO;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_WO:
			switch (tokch) {
			case 'r': state = STATE_WOR;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_BYT:
			switch (tokch) {
			case 'e': take(TOKEN_BYTE);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_NEA:
			switch (tokch) {
			case 'r': take(TOKEN_NEAR);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_SHO:
			switch (tokch) {
			case 'r': state = STATE_SHOR;break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_WOR:
			switch (tokch) {
			case 'd': take(TOKEN_WORD);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		case STATE_SHOR:
			switch (tokch) {
			case 't': take(TOKEN_SHORT);break;
			default: tokptr--;error = 1;take(TOKEN_NULL);break;
			}
			break;
		default: tokptr--;error = 1;take(TOKEN_NULL);break;
		}
		tokptr++;
	} while (!flagend);
	return token;
}
static void printtoken(t_aasm_token token)
{
	switch (token) {
	case TOKEN_NULL:    vapiPrint(" NULL ");break;
	case TOKEN_END:     vapiPrint(" END ");break;
	case TOKEN_LSPAREN: vapiPrint(" _[ ");break;
	case TOKEN_RSPAREN: vapiPrint(" _] ");break;
	case TOKEN_COLON:   vapiPrint(" _: ");break;
	case TOKEN_PLUS:    vapiPrint(" _+ ");break;
	case TOKEN_MINUS:   vapiPrint(" _- ");break;
	case TOKEN_BYTE:    vapiPrint(" BYTE ");break;
	case TOKEN_WORD:    vapiPrint(" WORD ");break;
	case TOKEN_PTR:     vapiPrint(" PTR ");break;
	case TOKEN_NEAR:    vapiPrint(" NEAR ");break;
	case TOKEN_FAR:     vapiPrint(" FAR ");break;
	case TOKEN_SHORT:   vapiPrint(" SHORT ");break;
	case TOKEN_IMM8:    vapiPrint(" %02X_IMM8 ", tokimm8);break;
	case TOKEN_IMM16:   vapiPrint(" %04X_IMM16 ", tokimm16);break;
	case TOKEN_AH: vapiPrint(" AH ");break;
	case TOKEN_BH: vapiPrint(" BH ");break;
	case TOKEN_CH: vapiPrint(" CH ");break;
	case TOKEN_DH: vapiPrint(" DH ");break;
	case TOKEN_AL: vapiPrint(" AL ");break;
	case TOKEN_BL: vapiPrint(" BL ");break;
	case TOKEN_CL: vapiPrint(" CL ");break;
	case TOKEN_DL: vapiPrint(" DL ");break;
	case TOKEN_AX: vapiPrint(" AX ");break;
	case TOKEN_BX: vapiPrint(" BX ");break;
	case TOKEN_CX: vapiPrint(" CX ");break;
	case TOKEN_DX: vapiPrint(" DX ");break;
	case TOKEN_SP: vapiPrint(" SP ");break;
	case TOKEN_BP: vapiPrint(" BP ");break;
	case TOKEN_SI: vapiPrint(" SI ");break;
	case TOKEN_DI: vapiPrint(" DI ");break;
	case TOKEN_CS: vapiPrint(" CS ");break;
	case TOKEN_DS: vapiPrint(" DS ");break;
	case TOKEN_ES: vapiPrint(" ES ");break;
	case TOKEN_SS: vapiPrint(" SS ");break;
	default: vapiPrint(" ERROR ");break;
		break;
	}
}
static void matchtoken(t_aasm_token token)
{
	if (gettoken(NULL) != token) error = 1;
}
static t_aasm_oprinfo parsearg_mem()
{
	t_aasm_token token;
	t_aasm_oprinfo info;
	t_bool bx,bp,si,di,imm;
	memset(&info, 0x00, sizeof(t_aasm_oprinfo));
	bx = bp = si = di = imm = 0x00;
/*
	MEM_BX_SI,
	MEM_BX_DI,
	MEM_BP_SI,
	MEM_BP_DI,
	MEM_SI,
	MEM_DI,
	MEM_BP,
	MEM_BX
*/
	info.type = TYPE_M;
	token = gettoken(NULL);
	while (token != TOKEN_END || token != TOKEN_NULL) {
		switch (token) {
		case TOKEN_PLUS:break;
		case TOKEN_BX:
			if (bx) error = 1;
			else bx = 1;
			break;

		}
	}
	return info;
}
static t_aasm_oprinfo parsearg(t_string arg)
{
	t_aasm_token token;
	t_aasm_oprinfo info;
	memset(&info, 0x00 ,sizeof(t_aasm_oprinfo));
	token = gettoken(arg);
	switch (token) {
	case TOKEN_NULL:
	case TOKEN_END:
		info.type = TYPE_NONE;
		break;
	case TOKEN_BYTE:
		matchtoken(TOKEN_PTR);
		matchtoken(TOKEN_LSPAREN);
		info = parsearg_mem();
		break;
	}
	return info;
}
static void parse()
{
	/* parse single statement */
	t_nubitcc i = 0;
	t_string aend;
	aop = aopr1 = aopr2 = NULL;
	while(astmt[i] != '\0') {
		if (astmt[i] == ';') {
			astmt[i] = 0;
			break;
		}
		if(astmt[i] > 0x40 && astmt[i] < 0x5b) astmt[i] += 0x20;
		++i;
	}

	while (*astmt && (*astmt == ' ' || *astmt == '\t')) astmt++;
	aop = astmt;

	while (*astmt && (*astmt != ' ' && *astmt != '\t')) astmt++;
	if (*astmt) {
		*astmt = 0;
		astmt++;
		while (*astmt && (*astmt == ' ' || *astmt == '\t')) astmt++;
		if (*astmt) {
			aopr1 = astmt;
			while (*astmt && (*astmt != ',')) astmt++;
			if (*astmt) {
				aend = astmt - 1;
				while (*aend && (*aend == ' ' || *aend == '\t')) aend--;
				*(aend + 1) = 0;
				astmt++;
				while (*astmt && (*astmt == ' ' || *astmt == '\t')) astmt++;
				if (*astmt) {
					aopr2 = astmt;
					while (*astmt) astmt++;
					aend = astmt - 1;
					while (*aend && (*aend == ' ' || *aend == '\t')) aend--;
					*(aend + 1) = 0;
				}
			}
		}
	}
	aopri1 = parsearg(aopr1);
	aopri2 = parsearg(aopr2);
}
static void exec()
{
	/* assemble single statement */
	if (!aop || !aop[0]) return;
	if (!strcmp(aop, "db"))       ;
	else if(!strcmp(aop, "dw"))   ;
	else if(!strcmp(aop, "add"))  ADD();
	else if(!strcmp(aop,"es:"))   ES();
	else if(!strcmp(aop,"cs:"))   CS();
	else if(!strcmp(aop,"ss:"))   SS();
	else if(!strcmp(aop,"ds:"))   DS();
	else if(!strcmp(aop,"aaa"))   AAA();
	else if(!strcmp(aop,"aad"))   AAD();
	else if(!strcmp(aop,"aam"))   AAM();
	else if(!strcmp(aop,"aas"))   AAS();
/*	else if(!strcmp(aop,"adc"))	aGROUP1(0x02,0x10,0)
	else if(!strcmp(aop,"add"))	aGROUP1(0x00,0x00,0)
	else if(!strcmp(aop,"and"))	aGROUP1(0x04,0x20,1)
	else if(!strcmp(aop,"call")) aGROUP5_CALL
	else if(!strcmp(aop,"cbw"))	aSINGLE(0x98)
	else if(!strcmp(aop,"clc"))	aSINGLE(0xf8)
	else if(!strcmp(aop,"cld"))	aSINGLE(0xfc)
	else if(!strcmp(aop,"cli"))	aSINGLE(0xfa)
	else if(!strcmp(aop,"cmc"))	aSINGLE(0xf5)
	else if(!strcmp(aop,"cmp"))	aGROUP1(0x07,0x38,0)
	else if(!strcmp(aop,"cmpsb"))aSINGLE(0xa6)
	else if(!strcmp(aop,"cmpsw"))aSINGLE(0xa7)
	else if(!strcmp(aop,"cwd"))	aSINGLE(0x99)
	else if(!strcmp(aop,"daa"))	aSINGLE(0x27)
	else if(!strcmp(aop,"das"))	aSINGLE(0x2f)
	else if(!strcmp(aop,"dec"))	aGROUP4(0x48,0x01)
	else if(!strcmp(aop,"div"))	aGROUP3(0x06)
//	else if(!strcmp(aop,"esc"))	ESCAPE
	else if(!strcmp(aop,"hlt"))	aSINGLE(0xf4)
	else if(!strcmp(aop,"idiv"))	aGROUP3(0x07)
	else if(!strcmp(aop,"imul"))	aGROUP3(0x05)
	else if(!strcmp(aop,"in"))	aIN
	else if(!strcmp(aop,"inc"))	aGROUP4(0x40,0x00)
	else if(!strcmp(aop,"int"))	aINT
//	else if(!strcmp(aop,"intr+"))INTR
	else if(!strcmp(aop,"into"))	aSINGLE(0xce)
	else if(!strcmp(aop,"iret"))	aSINGLE(0xcf)
	else if(!strcmp(aop,"ja")
		|| !strcmp(aop,"jnbe"))	aJCC(0x77)
	else if(!strcmp(aop,"jae")
		|| !strcmp(aop,"jnb"))	aJCC(0x73)
	else if(!strcmp(aop,"jb")
		|| !strcmp(aop,"jnae"))	aJCC(0x72)
	else if(!strcmp(aop,"jbe")
		|| !strcmp(aop,"jna"))	aJCC(0x76)
	else if(!strcmp(aop,"jc"))	aJCC(0x72)
	else if(!strcmp(aop,"jcxz"))	aJCC(0xe3)
	else if(!strcmp(aop,"je")
		|| !strcmp(aop,"jz"))	aJCC(0x74)
	else if(!strcmp(aop,"jg")
		|| !strcmp(aop,"jnle"))	aJCC(0x7f)
	else if(!strcmp(aop,"jge")
		|| !strcmp(aop,"jnl"))	aJCC(0x7d)
	else if(!strcmp(aop,"jl")
		|| !strcmp(aop,"jnge"))	aJCC(0x7c)
	else if(!strcmp(aop,"jle")
		|| !strcmp(aop,"jng"))	aJCC(0x7e)
	else if(!strcmp(aop,"jmp"))	aGROUP5_JMP
	else if(!strcmp(aop,"jnc"))	aJCC(0x73)
	else if(!strcmp(aop,"jne")
		|| !strcmp(aop,"jnz"))	aJCC(0x75)
	else if(!strcmp(aop,"jno"))	aJCC(0x71)
	else if(!strcmp(aop,"jnp")
		|| !strcmp(aop,"jpo"))	aJCC(0x7b)
	else if(!strcmp(aop,"jns"))	aJCC(0x79)
	else if(!strcmp(aop,"jo"))	aJCC(0x70)
	else if(!strcmp(aop,"jp")
		|| !strcmp(aop,"jpe"))	aJCC(0x7a)
	else if(!strcmp(aop,"js"))	aJCC(0x78)
	else if(!strcmp(aop,"lahf"))	aSINGLE(0x9f)
	else if(!strcmp(aop,"lds"))	aLEA(0xc5)
	else if(!strcmp(aop,"lea"))	aLEA(0x8d)
	else if(!strcmp(aop,"les"))	aLEA(0xc4)
//	else if(!strcmp(aop,"lock"))	PREFIX(0xf0)
	else if(!strcmp(aop,"lodsb"))aSINGLE(0xac)
	else if(!strcmp(aop,"lodsw"))aSINGLE(0xad)
	else if(!strcmp(aop,"loaop"))	aLOOPCC(0xe2)
	else if(!strcmp(aop,"loaope")
		|| !strcmp(aop,"loaopz"))	aLOOPCC(0xe1)
	else if(!strcmp(aop,"loaopne")
		|| !strcmp(aop,"loaopnz"))aLOOPCC(0xe0)
//	else if(!strcmp(aop,"nmi+"))	NMI+
	else if(!strcmp(aop,"mov"))	aMOV
	else if(!strcmp(aop,"movsb"))aSINGLE(0xa4)
	else if(!strcmp(aop,"movsw"))aSINGLE(0xa5)
	else if(!strcmp(aop,"mul"))	aGROUP3(0x04)
	else if(!strcmp(aop,"neg"))	aGROUP3(0x03)
	else if(!strcmp(aop,"naop"))	aSINGLE(0x90)
	else if(!strcmp(aop,"not"))	aGROUP3(0x02)
	else if(!strcmp(aop,"or"))	aGROUP1(0x01,0x08,1)
	else if(!strcmp(aop,"out"))	aOUT
	else if(!strcmp(aop,"paop"))	aPOP
	else if(!strcmp(aop,"paopf"))	aSINGLE(0x9d)
	else if(!strcmp(aop,"push"))	aGROUP5_PUSH
	else if(!strcmp(aop,"pushf"))aSINGLE(0x9c)
	else if(!strcmp(aop,"qdx"))  aQDX
	else if(!strcmp(aop,"rcl"))	aGROUP2(0x02)
	else if(!strcmp(aop,"rcr"))	aGROUP2(0x03)
//	else if(!strcmp(aop,"rep"))	PREFIX(0xf3)
//	else if(!strcmp(aop,"repe")
//		|| !strcmp(aop,"repz"))	PREFIX(0xf3)
//	else if(!strcmp(aop,"REPNZ")
//		|| !strcmp(aop,"repnz"))	PREFIX(0xf2)
	else if(!strcmp(aop,"ret"))	aRET
	else if(!strcmp(aop,"retf"))	aRETF
	else if(!strcmp(aop,"rol"))	aGROUP2(0x00)
	else if(!strcmp(aop,"ror"))	aGROUP2(0x01)
	else if(!strcmp(aop,"sahf"))	aSINGLE(0x9e)
	else if(!strcmp(aop,"sal")
		|| !strcmp(aop,"shl"))	aGROUP2(0x04)
	else if(!strcmp(aop,"sar"))	aGROUP2(0x07)
	else if(!strcmp(aop,"sbb"))	aGROUP1(0x03,0x18,0)
	else if(!strcmp(aop,"scasb"))aSINGLE(0xae)
	else if(!strcmp(aop,"scasw"))aSINGLE(0xaf)
//	else if(!strcmp(aop,"segment+"))	SEGMENT+
	else if(!strcmp(aop,"shr"))	aGROUP2(0x05)
//	else if(!strcmp(aop,"singlestep+")) SINGLESTEP+
	else if(!strcmp(aop,"stc"))	aSINGLE(0xf9)
	else if(!strcmp(aop,"std"))	aSINGLE(0xfd)
	else if(!strcmp(aop,"sti"))	aSINGLE(0xfb)
	else if(!strcmp(aop,"stosb"))aSINGLE(0xaa)
	else if(!strcmp(aop,"stosw"))aSINGLE(0xab)
	else if(!strcmp(aop,"sub"))	aGROUP1(0x05,0x28,0)
	else if(!strcmp(aop,"test"))	aGROUP3_TEST
	else if(!strcmp(aop,"wait"))	aSINGLE(0x9b)
	else if(!strcmp(aop,"xchg"))	aXCHG
	else if(!strcmp(aop,"xlat"))	aSINGLE(0xd7)
	else if(!strcmp(aop,"xor"))	aGROUP1(0x06,0x30,1)
	else ;//*/
}

/* op opr1,opr2 ;comment \n op opr1,opr2 ;end\n */
static t_bool isprefix(t_string op)
{
	if (!strcmp(op, "es:") || !strcmp(op, "ds:") ||
		!strcmp(op, "cs:") || !strcmp(op, "ss:") ||
		!strcmp(op, "lock") || !strcmp(op, "rep") ||
		!strcmp(op, "repne") || !strcmp(op, "repnz") ||
		!strcmp(op, "repe") || !strcmp(op, "repz")) return 0x01;
	return 0x00;
}
t_nubitcc aasm(const t_string stmt, t_nubit16 seg, t_nubit16 off)
{
	t_nsbit8 oldchar;
	t_bool prefix;
	t_nubitcc i, slen, l, len;
	if (!stmt || !stmt[0]) return 0;
	STRCPY(pool, stmt);
	astmt = pool;
	slen = strlen(pool);
	if (slen >= 0x0fff) return 0;
	if (pool[slen - 1] != '\n') {
		pool[slen] = '\n';
		pool[slen + 1] = 0;
	}

	do {
		prefix = 0x00;
		while (*astmt != '\n' && *astmt != ';' && (*astmt == ' ' || *astmt == '\t')) astmt++;
		aop = astmt;
		while (*astmt != '\n' && *astmt != ';' && (*astmt != ' ' && *astmt != '\t')) astmt++;
		oldchar = *astmt;
		*astmt = 0;
		prefix = isprefix(aop);
		switch (oldchar) {
		case ';' :
		case '\n': *astmt = oldchar; break;
		case ' ' :
		case '\t':
			if (prefix) *astmt = '\n';
			else *astmt = ' ';
			break;
		}
		astmt++;
	} while (prefix);

	slen = strlen(pool);
	len = 0;
	error = 0;
	avcs = seg;
	avip = off;
	astmt = pool;
	for (i = 0;i < slen;++i) {
		if (pool[i] == '\n') {
			pool[i] = 0;
			parse();
			exec();
			l = avip - off;
			len += l;
			off = avip;
			astmt = pool + i + 1;
		}
		if (error) break;
	}
	if (error) len = 0;
	return len;
}
