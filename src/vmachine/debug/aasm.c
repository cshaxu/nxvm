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
	TYPE_REG8,
	TYPE_REG16,
	TYPE_PTR16,
	TYPE_SEG16,
	TYPE_IMM8,
	TYPE_IMM16,
	TYPE_MEM,
	TYPE_MEM8,
	TYPE_MEM16
} t_aasm_oprtype;
typedef enum {
	MOD_MEM,
	MOD_MEM_IMM8,
	MOD_MEM_IMM16,
	MOD_REG} t_aasm_oprmod;
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
	S16_ES,
	S16_CS,
	S16_SS,
	S16_DS
} t_aasm_oprseg16;
typedef enum {
	FIP_NONE,
	FIP_NEAR,
	FIP_FAR
} t_aasm_oprip;

typedef struct {
	t_aasm_oprtype  type;  // 0 = error; 1 = reg8; 2 = reg16; 3 = ptr16;
                           // 4 = seg16(es=rm.0,cs=rm.1,ss=rm.2,ds=rm.3);
                           // 5 = imm8; 6 = imm16; 7 = mem8; 8 = mem16; 9 = null;
	t_aasm_oprmod   mod;   // 0 = mem; 1 = mem+imm8; 2 = mem+imm16; 3 = reg; 4 = other;
	t_aasm_oprmem   mem;   // use by modrm when mod = 0,1,2; by reg/ptr/seg when mod = 3; 8 = other;
                           // 0 = [BX+SI], 1 = [BX+DI], 2 = [BP+SI], 3 = [BP+DI],
                           // 4 = [SI], 5 = [DI], 6 = [BP], 7 = [BX]
	t_aasm_oprreg8  reg8;  // use by modrm when mod = 0,1,2; by reg/ptr/seg when mod = 3; 8 = other;
                           // 0 = AX/AL, 1 = CX/CL, 2 = DX/DL, 3 = BX/BL,
                           // 4 = SP/AH, 5 = BP/CH, 6 = SI/DH, 7 = DI/BH
	t_aasm_oprreg16 reg16;
	t_aasm_oprseg16 seg16;
	t_nubit8        imm8;
	t_nubit16       imm16;
	t_nubit8        disp8;
	t_nubit16       disp16;// use as imm when type = 5,6; use by modrm as disp when mod = 0(rm = 6),1,2;
	t_aasm_oprip    fip;   // 0 = blNear; 1 = blFar; 2 = N/A;
} t_aasm_oprinfo;

static char pool[0x1000];
static t_aasm_oprinfo aopri1, aopri2;
static t_string astmt, aop, aopr1, aopr2;
static t_nubit16 avcs, avip;

#define isNone(oprinf)  ((oprinf).type  == TYPE_NONE)
#define isReg8(oprinf)  ((oprinf).type  == TYPE_REG8  && (oprinf).mod == MOD_REG)
#define isReg16(oprinf) ((oprinf).type  == TYPE_REG16 && (oprinf).mod == MOD_REG)
#define isPtr16(oprinf) ((oprinf).type  == TYPE_PTR16 && (oprinf).mod == MOD_REG)
#define isSeg16(oprinf) ((oprinf).type  == TYPE_SEG16 && (oprinf).mod == MOD_REG)
#define isImm8(oprinf)  ((oprinf).type  == TYPE_IMM8)
#define isImm16(oprinf) ((oprinf).type  == TYPE_IMM16)
#define isMem8(oprinf)  (((oprinf).type == TYPE_MEM   || (oprinf).type == TYPE_MEM8 ) && (oprinf).mod != MOD_REG)
#define isMem16(oprinf) (((oprinf).type == TYPE_MEM   || (oprinf).type == TYPE_MEM16) && (oprinf).mod != MOD_REG)
#define isNear(oprinf)  ((oprinf).fip   == FIP_NEAR)
#define isFar(oprinf)   ((oprinf).fip   == FIP_FAR)

#define isImm(oprinf)   (isImm8 (oprinf) || isImm16(oprinf))
#define isRM8(oprinf)   (isReg8 (oprinf) || isMem8 (oprinf))
#define isRM16(oprinf)  (isReg16(oprinf) || isPtr16(oprinf) || isMem16(oprinf))
#define isR16(oprinf)   (isReg16(oprinf) || isPtr16(oprinf))
#define isRM(oprinf)    (isRM8  (oprinf) || isRM16(oprinf))
#define isAL(oprinf)    (isReg8 (oprinf) && (oprinf).reg8  == R8_AL)
#define isCL(oprinf)    (isReg8 (oprinf) && (oprinf).reg8  == R8_CL)
#define isDL(oprinf)    (isReg8 (oprinf) && (oprinf).reg8  == R8_DL)
#define isAX(oprinf)    (isReg16(oprinf) && (oprinf).reg16 == R16_AX)
#define isCX(oprinf)    (isReg16(oprinf) && (oprinf).reg16 == R16_CX)
#define isDX(oprinf)    (isReg16(oprinf) && (oprinf).reg16 == R16_DX)
#define isES(oprinf)    (isSeg16(oprinf) && (oprinf).seg16 == S16_ES)
#define isCS(oprinf)    (isSeg16(oprinf) && (oprinf).seg16 == S16_CS)
#define isSS(oprinf)    (isSeg16(oprinf) && (oprinf).seg16 == S16_SS)
#define isDS(oprinf)    (isSeg16(oprinf) && (oprinf).seg16 == S16_DS)
#define isALImm8        (isAL(aopri1)    && isImm8(aopri2))
#define isAXImm8        (isAX(aopri1)    && isImm8(aopri2))
#define isAXImm16       (isAX(aopri1)    && isImm (aopri2))
#define isRM8Imm8       (isRM8 (aopri1)  && isImm8(aopri2))
#define isRM16Imm       (isRM16(aopri1)  && isImm (aopri2))
#define isRM8R8         (isRM8 (aopri1)  && isReg8(aopri2))
#define isRM16R16       (isRM16(aopri1)  && isR16 (aopri2))
#define isR8RM8         (isReg8(aopri1)  && isRM8 (aopri2))
#define isR16RM16       (isR16 (aopri1)  && isRM16(aopri2))

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
static void SetModRegRM(t_nubit8 mod, t_nubit8 reg, t_nubit8 rm) {/* todo */}

static void ES()
{
	setbyte(0x26);
	avip++;
}
static void CS()
{
	setbyte(0x2e);
	avip++;
}
static void SS()
{
	setbyte(0x36);
	avip++;
}
static void DS()
{
	setbyte(0x3e);
	avip++;
}
static void AAA()
{
	setbyte(0x37);
	avip++;
}
static void AAD()
{
	if (aopr1 || aopr2) return;
	setbyte(0xd5);
	avip++;
	SetImm8(0x0a);
}
static void AAM()
{
	if (aopr1 || aopr2) return;
	setbyte(0xd4);
	avip++;
	SetImm8(0x0a);
}
static void AAS()
{
	setbyte(0x3f);
	avip++;
}
static void ADC()
{
	t_nubit8 mod, rm;
	if (isALImm8) {
		setbyte(0x14);
		avip++;
		SetImm8(aopri2.imm8);
	} else if (isAXImm16) {
		setbyte(0x15);
		avip++;
		SetImm16(aopri2.imm16);
	} else if (isRM8Imm8) {
		setbyte(0x80);
		avip++;
		mod = (t_nubit8)aopri1.mod;
		switch (aopri1.type) {
		case TYPE_REG8:
			rm = (t_nubit8)aopri1.reg8;
			break;
		case TYPE_MEM:
		case TYPE_MEM8:
			rm = (t_nubit8)aopri1.mem;
			break;
		default: avip = 0;break;
		}
		SetModRegRM(mod, 0x02, rm);
	}
/*
#define aGROUP1(reg,op,sign) {\
if(isALImm8) {					aSINGLE(0x04+(op))					aSetByte(m2.imm)	}\
else if(isAXImm16) {			aSINGLE(0x05+(op))					aSetWord(m2.imm)	}\
else if(isRM8Imm8) {			aSINGLE(0x80)	aSetModRM(m1,(reg))	aSetByte(m2.imm)	}\
else if(isRM16Imm)\
{	if((sign) || m2.imm < 0xff80) {\
								aSINGLE(0x81)	aSetModRM(m1,(reg))	aSetWord(m2.imm)	}\
	else {						aSINGLE(0x83)	aSetModRM(m1,(reg))	aSetByte(m2.imm)	}}\
else if(isRM8R8) {				aSINGLE(0x00+(op))	aSetModRM(m1,m2.rm)					}\
else if(isRM16R16) {			aSINGLE(0x01+(op))	aSetModRM(m1,m2.rm)					}\
else if(isR8RM8) {				aSINGLE(0x02+(op))	aSetModRM(m2,m1.rm)					}\
else if(isR16RM16) {			aSINGLE(0x03+(op))	aSetModRM(m2,m1.rm)					}\
else len = 0;}
*/
}

static t_aasm_oprinfo parsearg(t_string arg)
{
	t_aasm_oprinfo info;
	memset(&info, 0x00, sizeof(t_aasm_oprinfo));
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
	if(!strcmp(aop,"es:"))        ES();
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
t_nubitcc aasm(const t_string stmt, t_nubit16 seg, t_nubit16 off)
{
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
	slen = strlen(pool);
	len = 0;
	avcs = seg;
	avip = off;
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
	}
	return len;
}
