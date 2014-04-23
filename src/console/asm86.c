/* This file is a part of Neko's ASM86 Project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "asm86.h"

/* GENERAL */
typedef struct {
	unsigned char type;	// 0 = error; 1 = reg8; 2 = reg16; 3 = ptr16;
						// 4 = seg16(es=0,cs=1,ss=2,ds=3);
						// 5 = imm8; 6 = imm16; 7 = mem8; 8 = mem16; 9 = null;
	unsigned char mod;	// 0 = mem; 1 = mem+imm8; 2 = mem+imm16; 3 = reg; 4 = other;
	unsigned char rm;	// use by modrm when mod = 0,1,2; by reg/ptr/seg when mod = 3; 8 = other;
						// 0 = BX+SI, 1 = BX+DI, 2 = BP+SI, 3 = BP+DI,
						// 4 = SI, 5 = DI, 6 = BP, 7 = BX
	unsigned char reg;	// use by modrm when mod = 0,1,2; by reg/ptr/seg when mod = 3; 8 = other;
						// 0 = AX/AL, 1 = CX/CL, 2 = DX/DL, 3 = BX/CL,
						// 4 = SP/AH, 5 = BP/CH, 6 = SI/DH, 7 = DI/BH
	unsigned short imm;	// use as imm when type = 5,6; use by modrm as disp when mod = 0(rm = 6),1,2;
	unsigned char blFar;	// 0 = blNear; 1 = blFar; 2 = N/A;
} ModRM;
static int ishexdigit(char c)
{
	if( (c >= 0x30 && c <= 0x39) || 
		(c >= 0x41 && c <= 0x46) || 
		(c >= 0x61 && c <= 0x66)) return 1;
	else return 0;
}
static int chartohexdigit(char c)
{
	if(c >= 0x30 && c <= 0x39) return c-0x30; 
	else if(c >= 0x41 && c <= 0x46) return c-0x37;
	else if(c >= 0x61 && c <= 0x66) return c-0x57;
	else return -1;
}
#define isError(m)	((m).type == 0)
#define isReg8(m)	((m).type == 1 && (m).mod == 3)
#define isReg16(m)	((m).type == 2 && (m).mod == 3)
#define isPtr16(m)	((m).type == 3 && (m).mod == 3)
#define isSeg16(m)	((m).type == 4 && (m).mod == 3)
#define isImm8(m)	((m).type == 5)
#define isImm16(m)	((m).type == 6)
#define isMem8(m)	((m).type == 7 && (m).mod != 3)
#define isMem16(m)	((m).type == 8 && (m).mod != 3)
#define isNull(m)	((m).type == 9)
#define isNear(m)	((m).blFar  == 0)
#define isFar(m)	((m).blFar  == 1)
#define isImm(m)	(isImm8 (m) || isImm16(m))
#define isRM8(m)	(isReg8 (m) || isMem8 (m))
#define isRM16(m)	(isReg16(m) || isPtr16(m) || isMem16(m))
#define isR16(m)	(isReg16(m) || isPtr16(m))
#define isRM(m)		(isRM8  (m) || isRM16(m))
#define isAL(m)		(isReg8 (m) && (m).rm == 0)
#define isAX(m)		(isReg16(m) && (m).rm == 0)
#define isCL(m)		(isReg8 (m) && (m).rm == 1)
#define isCX(m)		(isReg16(m) && (m).rm == 1)
#define isDL(m)		(isReg8 (m) && (m).rm == 2)
#define isDX(m)		(isReg16(m) && (m).rm == 2)
#define isES(m)		(isSeg16(m) && (m).rm == 0)
#define isCS(m)		(isSeg16(m) && (m).rm == 1)
#define isSS(m)		(isSeg16(m) && (m).rm == 2)
#define isDS(m)		(isSeg16(m) && (m).rm == 3)
#define isALImm8	(isAL(m1) && isImm8(m2))
#define isAXImm8	(isAX(m1) && isImm8(m2))
#define isAXImm16	(isAX(m1) && isImm(m2))
#define isRM8Imm8	(isRM8(m1) && isImm8(m2))
#define isRM16Imm	(isRM16(m1) && isImm(m2))
#define isRM8R8		(isRM8(m1) && isReg8(m2))
#define isRM16R16	(isRM16(m1) && isR16(m2))
#define isR8RM8		(isReg8(m1) && isRM8(m2))
#define isR16RM16	(isR16(m1) && isRM16(m2))
#define isShort(ins,off,len) (((off)+(len) < 0x0080 && (((ins) < (off)+(len)+0x0080) || ((ins) >= (unsigned short)((off)+(len)-0x0080)))) ||\
	((off)+(len) >= 0x0080 && (off)+(len) <= 0xff80 && (((ins) >= (off)+(len)-0x0080) && ((ins) < (off)+(len)+0x0080))) ||\
	((off)+(len) > 0xff80 && (((ins) < (unsigned short)((off)+(len)+0x0080)) || ((ins) <= 0xffff && (ins) >= (off)+(len)-0x0080))))
#define dispLen(m)	((((m).mod == 0 && (m).rm == 6) || (m).mod == 2)?2:(((m).mod == 1)?1:0))

/*#define dumpArg {\
	fprintf(stdout,"ARG REPORT:byte=%d,word=%d,blNear=%d,blFar=%d\n",byte,word,blNear,blFar);\
	fprintf(stdout,"ax=%d,bx=%d,cx=%d,dx=%d,sp=%d,bp=%d,si=%d,di=%d\n",ax,bx,cx,dx,sp,bp,si,di);\
	fprintf(stdout,"ah=%d,al=%d,bh=%d,bl=%d,ch=%d,cl=%d,dh=%d,dl=%d\n",ah,al,bh,bl,ch,cl,dh,dl);\
	fprintf(stdout,"cs=%d,ds=%d,es=%d,ss=%d,imm8=%d,nsum=%x,nnsum=%d\nsparen=%d\n",cs,ds,es,ss,imm8,nsum,nnsum,sparen);\
	fprintf(stdout,"type = %d, mod = %d, rm = %d, imm = %x\n",a.type,a.mod,a.rm,a.imm);}

#undef dumpArg*/

#define aSetByte(n)	{*(loc+len++) = (unsigned char)(n);}
#define aSetWord(n)	{*(loc+len++) = (unsigned char)(n);*(loc+len++) = (unsigned char)((n)>>8);}
#define aSetModRM(modrm,reg) {\
	*(loc+len++) = ((modrm).mod<<6) + ((reg)<<3) + ((modrm).rm);\
	if(dispLen(modrm)) {\
			*(loc+len++) = (unsigned char)((modrm).imm);\
			if(dispLen(modrm) == 2) *(loc+len++) = (unsigned char)((modrm).imm>>8);}}
#define aSINGLE(op)	aSetByte(op)
#define aAACC(op) {	if(a1) len = 0; else {aSetByte(op)	aSetByte(0x0a)}}
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
#define aGROUP2(reg) {\
	if(isImm(m2)) m2.rm = (reg);\
	if(isRM8Imm8 && m2.imm == 1) {	aSINGLE(0xd0)	aSetModRM(m1,(reg))					}\
	else if(isRM8(m1) && isCL(m2)){	aSINGLE(0xd2)	aSetModRM(m1,(reg))					}\
	else if(isRM16(m1) && isImm8(m2) && m2.imm == 1)\
	{								aSINGLE(0xd1)	aSetModRM(m1,(reg))					}\
	else if(isRM16(m1) && isCL(m2))\
	{								aSINGLE(0xd3)	aSetModRM(m1,(reg))					}\
	else len = 0;}
#define aGROUP3_TEST {\
	if(isALImm8) {					aSINGLE(0xa8)						aSetByte(m2.imm)	}\
	else if(isAXImm16) {			aSINGLE(0xa9)						aSetWord(m2.imm)	}\
	else if(isRM8Imm8) {			aSINGLE(0xf6)	aSetModRM(m1,0x00)	aSetByte(m2.imm)	}\
	else if(isRM16Imm) {			aSINGLE(0xf7)	aSetModRM(m1,0x00)	aSetWord(m2.imm)	}\
	else if(isRM8R8) {				aSINGLE(0x84)	aSetModRM(m1,m2.rm)					}\
	else if(isRM16R16) {			aSINGLE(0x85)	aSetModRM(m1,m2.rm)					}\
	else len = 0;}
#define aGROUP3(reg) {\
	if(isRM8(m1)) {					aSINGLE(0xf6)	aSetModRM(m1,(reg))					}\
	else if(isRM16(m1)) {			aSINGLE(0xf7)	aSetModRM(m1,(reg))					}\
	else len = 0;}
#define aGROUP4(op,reg) {\
	if(isR16(m1)) {					aSINGLE((op)+m1.rm)									}\
	else if(isRM8(m1)) {			aSINGLE(0xfe)	aSetModRM(m1,(reg))					}\
	else if(isRM16(m1)) {			aSINGLE(0xff)	aSetModRM(m1,(reg))					}\
	else len = 0;}
#define aGROUP5_CALL {\
	if(isImm(m1) && isNull(m2))\
	{	if(!isFar(m1)) {			aSINGLE(0xe8)						aSetWord(m1.imm-0x03-offset)}\
		else {						aSINGLE(0x9a)						aSetWord(m1.imm)	aSetWord(cs)		}}\
	else if(isImm(m1) && isImm(m2) && !isNear(m1))\
	{								aSINGLE(0x9a)						aSetWord(m2.imm)	aSetWord(m1.imm)	}\
	else if(isRM16(m1))\
	{	if(isFar(m1)) {				aSINGLE(0xff)	aSetModRM(m1,0x03)					}\
		else {						aSINGLE(0xff)	aSetModRM(m1,0x02)					}}\
	else len = 0;}
#define aGROUP5_JMP {\
	if(isImm(m1) && isNull(m2))\
	{	if(isFar(m1)) {				aSINGLE(0xea)						aSetWord(m1.imm)	aSetWord(cs)		}\
		else \
		{	if(isNear(m1) || !isShort(m1.imm,offset,0x02))\
			{						aSINGLE(0xe9)						aSetWord(m1.imm-offset-0x03)}\
			else {					aSINGLE(0xeb)						aSetByte(m1.imm-offset-0x02)}}}\
	else if(isRM16(m1))\
	{	if(isFar(m1)) {				aSINGLE(0xff)	aSetModRM(m1,0x05)					}\
		else {						aSINGLE(0xff)	aSetModRM(m1,0x04)					}}\
	else if(isImm(m1) && isImm(m2) && !isNear(m1))\
	{								aSINGLE(0xea)						aSetWord(m2.imm)	aSetWord(m1.imm)	}\
	else len = 0;}
#define aGROUP5_PUSH {\
	if(!isNull(m2)) len = 0;\
	if(isCS(m1))					aSINGLE(0x0e)										\
	else if(isSS(m1))				aSINGLE(0x16)										\
	else if(isDS(m1))				aSINGLE(0x1e)										\
	else if(isES(m1))				aSINGLE(0x06)										\
	else if(isR16(m1))				aSINGLE(0x50+m1.rm)									\
	else if(isRM16(m1)) {			aSINGLE(0xff)	aSetModRM(m1,0x06)					}\
	else len = 0;}
#define aIN {\
	if(isALImm8) {					aSINGLE(0xe4)						aSetByte(m2.imm)	}\
	else if(isAXImm8) {				aSINGLE(0xe5)						aSetByte(m2.imm)	}\
	else if(isAL(m1) && isDX(m2))	aSINGLE(0xec)										\
	else if(isAX(m1) && isDX(m2))	aSINGLE(0xed)										\
	else len = 0;}
#define aINT {\
	if(isImm8(m1) && isNull(m2))\
	{	if(m1.imm == 3)				aSINGLE(0xcc)										\
		else {						aSINGLE(0xcd)						aSetByte(m1.imm)	}}\
	else len = 0;}
#define aJCC(op) {\
	if(isImm(m1) && isNull(m2) && !isFar(m1) && !isNear(m1) && isShort(m1.imm,offset,0x02))\
	{								aSINGLE(op)							aSetByte(m1.imm-offset-0x02)}}
#define aLEA(op) {\
	if(isR16(m1) && isMem16(m2) && !isNear(m2) && !isFar(m2))\
	{								aSINGLE(op)		aSetModRM(m2,m1.rm)					}}
#define aLOOPCC(op) {\
	if(isImm(m1) && isNull(m2) && !isFar(m1) && !isNear(m1) && isShort(m1.imm,offset,0x02))\
	{								aSINGLE(op)							aSetByte(m1.imm-offset-0x02)}}
#define aMOV {\
	if(isAL(m1) && isMem8(m2) && m2.mod == 0 && m2.rm == 6)\
	{								aSINGLE(0xa0)						aSetWord(m2.imm)	}\
	else if(isAX(m1) && isMem16(m2) && m2.mod == 0 && m2.rm == 6)\
	{								aSINGLE(0xa1)						aSetWord(m2.imm)	}\
	else if(isMem8(m1) && isAL(m2) && m1.mod == 0 && m1.rm == 6)\
	{								aSINGLE(0xa2)						aSetWord(m1.imm)	}\
	else if(isMem16(m1) && isAX(m2) && m1.mod == 0 && m1.rm == 6)\
	{								aSINGLE(0xa3)						aSetWord(m1.imm)	}\
	else if(isRM8(m1) && isReg8(m2)) {aSINGLE(0x88)	aSetModRM(m1,m2.rm)					}\
	else if(isRM16(m1) && isR16(m2))\
	{								aSINGLE(0x89)	aSetModRM(m1,m2.rm)					}\
	else if(isReg8(m1) && isRM8(m2))\
	{								aSINGLE(0x8a)	aSetModRM(m2,m1.rm)					}\
	else if(isR16(m1) && isRM16(m2))\
	{								aSINGLE(0x8b)	aSetModRM(m2,m1.rm)					}\
	else if(isRM16(m1) && isSeg16(m2))\
	{								aSINGLE(0x8c)	aSetModRM(m1,m2.rm)					}\
	else if(isSeg16(m1) && isRM(m2))\
	{								aSINGLE(0x8e)	aSetModRM(m2,m1.rm)					}\
	else if(isReg8(m1) && isImm8(m2))\
	{								aSINGLE(0xb0+m1.rm)					aSetByte(m2.imm)	}\
	else if(isR16(m1) && isImm(m2))\
	{								aSINGLE(0xb8+m1.rm)					aSetWord(m2.imm)	}\
	else if(isRM8(m1) && isImm8(m2))\
	{								aSINGLE(0xc6)	aSetModRM(m1,0x00)	aSetByte(m2.imm)	}\
	else if(isRM16(m1) && isImm(m2))\
	{								aSINGLE(0xc7)	aSetModRM(m1,0x00)	aSetWord(m2.imm)	}\
	else len = 0;}
#define aOUT {\
	if(isImm8(m1) && isAL(m2)) {	aSINGLE(0xe6)						aSetByte(m1.imm)	}\
	else if(isImm8(m1) && isAX(m2))\
	{								aSINGLE(0xe7)						aSetByte(m1.imm)	}\
	else if(isDX(m1) && isAL(m2))	aSINGLE(0xee)										\
	else if(isDX(m1) && isAX(m2))	aSINGLE(0xef)										\
	else len = 0;}
#define aPOP {\
	if(isDS(m1))					aSINGLE(0x1f)										\
	else if(isES(m1))				aSINGLE(0x07)										\
	else if(isSS(m1))				aSINGLE(0x17)										\
	else if(isCS(m1))				aSINGLE(0x0f)	/* 'aPOP CS' ONLY FOR 8086*/			\
	else if(isR16(m1))				aSINGLE(0x58+m1.rm)									\
	else if(isRM16(m1)) {			aSINGLE(0x8f)	aSetModRM(m1,0x00)					}\
	else len = 0;\
	if(!isNull(m2)) len = 0;}
#define aRET {\
	if(isImm(m1)) {					aSINGLE(0xc2)						aSetWord(m1.imm)	}\
	else 							aSINGLE(0xc3)										}
#define aRETF {\
	if(isImm(m1)) {					aSINGLE(0xca)						aSetWord(m1.imm)	}\
	else 							aSINGLE(0xcb)										}
#define aXCHG {\
	if(isR16(m1) && isAX(m2))		aSINGLE(0x90+m1.rm)									\
	else if(isR8RM8) {				aSINGLE(0x86)	aSetModRM(m2,m1.rm)					}\
	else if(isRM8R8) {				aSINGLE(0x86)	aSetModRM(m1,m2.rm)					}\
	else if(isR16RM16) {			aSINGLE(0x87)	aSetModRM(m2,m1.rm)					}\
	else if(isRM16R16) {			aSINGLE(0x87)	aSetModRM(m1,m2.rm)					}\
	else len = 0;}

typedef enum {START,A1,B1,C1,D1,E1,F1,N1,P1,S1,W1,NUM} States;

static ModRM aArgParse(const char *s)
{
	ModRM a;
	States state;
	unsigned short n,nn,nsum,nnsum;
	int p,f;
	int sparen;
	int cs,ds,es,ss;int bs16;
	int ah,al,bh,bl,ch,cl,dh,dl;int bx8;
	int ax,cx,dx,bx,sp,bp,si,di;int bx16;int bp16;
	int imm8,sign; // sign = {0,plus},{1,minus}
	int byte,word,blNear,blFar;
	a.type = 9;
	a.mod = 4;
	a.rm = 8;
	a.imm = 0;
	a.blFar = 0;
	if(!s) return a;
	n = nn = nsum = nnsum = 0;
	p = f = 0;
	sparen = 0;
	cs = ds = es = ss = 0;
	ah = al = bh = bl = ch = cl = dh = dl = 0;
	ax = cx = dx = bx = sp = bp = si = di = 0;
	imm8 = sign = 0;
	byte = word = blNear = blFar = 0;
	state = START;
	while(!f) {
		switch(state) {
		case START:
			switch(s[p]) {
			case '\0':
			case '\t':
			case '\n':
			case '\r':
			case '\f':
			case ' ': break;
			case '+': sign = 0;break;
			case '-': sign = 1;break;
			case '[':
			case ']': sparen = 1;break;
			case 'a': state = A1;break;
			case 'b': state = B1;break;
			case 'c': state = C1;break;
			case 'd': state = D1;break;
			case 'e': state = E1;break;
			case 'f': state = F1;break;
			case 'n': state = N1;break;
			case 's': state = S1;break;
			case 'w': state = W1;break;
			case '0': n = 0;nn = 1;state = NUM;break;
			case '1': n = 1;nn = 1;state = NUM;break;
			case '2': n = 2;nn = 1;state = NUM;break;
			case '3': n = 3;nn = 1;state = NUM;break;
			case '4': n = 4;nn = 1;state = NUM;break;
			case '5': n = 5;nn = 1;state = NUM;break;
			case '6': n = 6;nn = 1;state = NUM;break;
			case '7': n = 7;nn = 1;state = NUM;break;
			case '8': n = 8;nn = 1;state = NUM;break;
			case '9': n = 9;nn = 1;state = NUM;break;
			case 'p':
				p++;
				if(s[p] == 't') p++;
				else {f = 1;break;}
				if(s[p] != 'r') f = 1;
				break;
			default: f = 1;break;
			}
			//fprintf(stdout,"f in start=%d, char = '%c'%d\n",f,s[p],s[p]);
			break;
		case A1:
			state = START;
			switch(s[p]) {
			case 'h': ah = 1;break;
			case 'l': al = 1;break;
			case 'x': ax = 1;break;
			default:
				if(ishexdigit(s[p])) {
					n = (0xa<<4)+chartohexdigit(s[p]);
					nn = 2;
					state = NUM;
				} else {
					n = 0xa;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
			}
			break;
		case B1:
			state = START;
			switch(s[p]) {
			case 'h': bh = 1;break;
			case 'l': bl = 1;break;
			case 'x': bx = 1;break;
			case 'p': bp = 1;break;
			case 'y':
				if(byte || word) {f = 1;break;}
				p++;
				if(s[p] == 't') p++;
				else {byte = 1;p--;break;}
				if(s[p] == 'e') byte = 1;
				else f = 1;
				break;
			default:
				if(ishexdigit(s[p])) {
					n = (0xb<<4)+chartohexdigit(s[p]);
					nn = 2;
					state = NUM;
				} else {
					n = 0xb;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
				break;
			}
			//fprintf(stdout,"f in b1=%d\n",f);
			break;
		case C1:
			state = START;
			switch(s[p]) {
			case 'h': ch = 1;break;
			case 'l': cl = 1;break;
			case 'x': cx = 1;break;
			case 's': cs = 1;break;
			default:
				if(ishexdigit(s[p])) {
					n = (0xc<<4)+chartohexdigit(s[p]);
					nn = 2;
					state = NUM;
				} else {
					n = 0xc;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
				break;
			}
			break;
		case D1:
			state = START;
			switch(s[p]) {
			case 'h': dh = 1;break;
			case 'l': dl = 1;break;
			case 'x': dx = 1;break;
			case 's': ds = 1;break;
			case 'i': di = 1;break;
			default:
				if(ishexdigit(s[p])) {
					n = (0xd<<4)+chartohexdigit(s[p]);
					nn = 2;
					state = NUM;
				} else {
					n = 0xd;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
				break;
			}
			break;
		case E1:
			state = START;
			if(s[p] == 's') es = 1;
			else if(ishexdigit(s[p])) {
				n = (0xe<<4)+chartohexdigit(s[p]);
				nn = 2;
				state = NUM;
			} else {
					n = 0xe;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
			break;
		case F1:
			state = START;
			if(ishexdigit(s[p])) {
				if(s[p] == 'a' && s[p+1] == 'r') {
					if(blFar || blNear || byte || word) f = 1;
					else blFar = 1;
					p++;
				} else {
					n = (0xf<<4)+chartohexdigit(s[p]);
					nn = 2;
					state = NUM;
				}
			} else {
					n = 0xf;
					nn = 1;
					state = NUM;
					--p;
				}
				break;
			//fprintf(stdout,"f in f1=%d\n",f);
			break;
		case N1:
			state = START;
			if(s[p] == 'e' && s[p+1] == 'a' && s[p+2] == 'r') {
				if(blFar || blNear || byte || word) f = 1;
				else blNear = 1;
				p += 2;
			} else f = 1;
			break;
		case S1:
			state = START;
			switch(s[p]) {
			case 'i': si = 1;break;
			case 'p': sp = 1;break;
			case 's': ss = 1;break;
			default: 
				if(s[p] == 'h' && s[p+1] == 'o' && s[p+2] == 'r' && s[p+3] == 't') {
					if(blFar || blNear || byte || word) f = 1;
					p += 3;
				} else f = 1;
				break;
			}
			//fprintf(stdout,"f in s1=%d\n",f);
			break;
		case W1:
			if(byte || word) {f = 1;break;}
			state = START;
			if(s[p] == 'o') {
				p++;
				if(s[p] == 'r') p++;
				else {word = 1;p--;break;}
				if(s[p] == 'd') {word = 1;p++;}
				else f = 1;
			} else f = 1;
			break;
		case NUM:
			if(ishexdigit(s[p])) {
				n = (n<<4)+chartohexdigit(s[p]);
				nn++;
				//fprintf(stdout,"n=%x,nn=%d,nsum=%x,nnsum=%d\n",n,nn,nsum,nnsum);
			} else {
				state = START;
				//fprintf(stdout,"NUM:n=%x,nn=%d\n",n,nn);
				if(sign) nsum -= n;
				else nsum += n;
				if(nnsum < nn) nnsum = nn;
				if(nnsum == 2) nsum &= 0xff;
				else if(nnsum > 4) f = 1;
				//fprintf(stdout,"f in num=%d\n",f);
				//fprintf(stdout,"n=%x,nn=%d,nsum=%x,nnsum=%d\n",n,nn,nsum,nnsum);
				n = 0;
				nn = 0;
				--p;
			}
			break;
		default:
			break;
		}
		if(s[p] == '\0') break;
		++p;
	}
	if(nnsum == 1 || nnsum == 2) {
		imm8 = 1;
		nnsum = 2;
	} else if(nnsum == 3 || nnsum == 4) {
		imm8 = 0;
		nnsum = 4;
	}
	if(cs == 1 || ds == 1 || es == 1 || ss == 1) bs16 = 1;
	else bs16 = 0;
	if(ax == 1 || bx == 1 || cx == 1 || dx == 1) bx16 = 1;
	else bx16 = 0;
	if(sp == 1 || bp == 1 || si == 1 || di == 1) bp16 = 1;
	else bp16 = 0;
	if(ah == 1 || al == 1 || bh == 1 || bl == 1 || 
		ch == 1 || cl == 1 || dh == 1 || dl == 1) bx8 = 1;
	else bx8 = 0;
	a.imm = nsum;
	if(blFar && !blNear) a.blFar = 1;
	else if(!blFar && blNear) a.blFar = 0;
	else if(!blFar && !blNear) a.blFar = 2;
	else f = 1;
	if(sparen) {
		// type -> mod -> rm
		if(byte && !word) a.type = 7;
		else a.type = 8;
		if(nnsum == 0) a.mod = 0;
		else if(nnsum == 2) a.mod = 1;
		else if(nnsum == 4) {
			if(a.imm > 0xff) a.mod = 2;
			else a.mod = 1;
		} else f = 1;
		if(bx && !bp && si && !di) a.rm = 0;
		else if(bx && !bp && !si && di) a.rm = 1;
		else if(!bx && bp && si && !di) a.rm = 2;
		else if(!bx && bp && !si && di) a.rm = 3;
		else if(!bx && !bp && si && !di) a.rm = 4;
		else if(!bx && !bp && !si && di) a.rm = 5;
		else if(!bx && bp && !si && !di){a.rm = 6;
			if(nnsum == 0) a.mod = 1;
		} else if(bx && !bp && !si && !di) a.rm = 7;
		else if(!bx && !bp && !si && !di && nnsum) {
			a.rm = 6;
			a.mod = 0;
		} else f = 1;
		if(bx8 || ax || sp || cx || dx) f = 1;
	} else {
		// type -> reg | imm
		a.mod = 3;
		if(bx8 && !bx16 && !bp16 && !bs16 && !nnsum) {
			a.type = 1;
			if(al) a.rm = 0;
			if(cl) a.rm = 1;
			if(dl) a.rm = 2;
			if(bl) a.rm = 3;
			if(ah) a.rm = 4;
			if(ch) a.rm = 5;
			if(dh) a.rm = 6;
			if(bh) a.rm = 7;
		} else if(!bx8 && bx16 && !bp16 && !bs16 && !nnsum) {
			a.type = 2;
			if(ax) a.rm = 0;
			if(cx) a.rm = 1;
			if(dx) a.rm = 2;
			if(bx) a.rm = 3;
		} else if(!bx8 && !bx16 && bp16 && !bs16 && !nnsum) {
			a.type = 3;
			if(sp) a.rm = 4;
			if(bp) a.rm = 5;
			if(si) a.rm = 6;
			if(di) a.rm = 7;
		} else if(!bx8 && !bx16 && !bp16 && bs16 && !nnsum) {
			a.type = 4;
			if(es) a.rm = 0;
			if(cs) a.rm = 1;
			if(ss) a.rm = 2;
			if(ds) a.rm = 3;
		} else if(!bx8 && !bx16 && !bp16 && !bs16 && nnsum) {
			if(nnsum == 2) a.type = 5;
			else if(nnsum == 4) a.type = 6;
			else f = 1;
		} else f = 1;
	}
	if(f) a.type = 0;
#ifdef dumpArg
	dumpArg
#endif
	return a;
}
static int aPrefix(const char *s,unsigned char *loc)
{
	if(!s) return 0;
	if(!strcmp(s,"lock")) *loc = 0xf0;
	else if(!strcmp(s,"REPNZ") || !strcmp(s,"repnz"))
		*loc = 0xf2;
	else if(!strcmp(s,"rep") || !strcmp(s,"repe") || !strcmp(s,"repz"))
		*loc = 0xf3;
	else return 0;
	return 1;
}
static int aDB(const char *db,unsigned char *loc)
{
	int i = 0;
	int flag = 1;
	unsigned char res = 0;
	if(!db) return 0;
	while(db[i] != '\0') {
		if(ishexdigit(db[i])) res = (res<<4)+chartohexdigit(db[i]);
		else flag = 0;
		++i;
	}
	if(flag && i < 3) {
		*(loc) = res;
	}
	else flag = 0;
	return flag;
}
static int aDW(const char *dw,unsigned char *loc)
{
	int i = 0;
	int flag = 1;
	unsigned short res = 0;
	if(!dw) return 0;
	while(dw[i] != '\0') {
		if(ishexdigit(dw[i])) res = (res<<4)+chartohexdigit(dw[i]);
		else flag = 0;
		++i;
	}
	if(flag && i < 5) {
		*(loc) = (unsigned char)res;
		*(loc+1) = (res>>8);
	} else flag = 0;
	return flag;
}
static int aOpCode(const char *op,const char *a1,const char *a2,
	unsigned char *loc,unsigned short offset,unsigned short cs)
{
	int len = 0;
	ModRM m1,m2;
	m1 = aArgParse(a1);
	m2 = aArgParse(a2);
	if(!op || isError(m1) || isError(m2) ) return len;
	/*if(!strcmp(op,"x")) {
		fprintf(stdout,"type = %d, mod = %d, rm = %d, imm = %x\n",m1.type,m1.mod,m1.rm,m1.imm);
	} else */
	if(!strcmp(op,"es:"))		aSINGLE(0x26)
	else if(!strcmp(op,"cs:"))	aSINGLE(0x2e)
	else if(!strcmp(op,"ss:"))	aSINGLE(0x36)
	else if(!strcmp(op,"ds:"))	aSINGLE(0x3e)
	else if(!strcmp(op,"aaa"))	aSINGLE(0x37)
	else if(!strcmp(op,"aad"))	aAACC(0xd5)
	else if(!strcmp(op,"aam"))	aAACC(0xd4)
	else if(!strcmp(op,"aas"))	aSINGLE(0x3f)
	else if(!strcmp(op,"adc"))	aGROUP1(0x02,0x10,0)
	else if(!strcmp(op,"add"))	aGROUP1(0x00,0x00,0)
	else if(!strcmp(op,"and"))	aGROUP1(0x04,0x20,1)
	else if(!strcmp(op,"call")) aGROUP5_CALL
	else if(!strcmp(op,"cbw"))	aSINGLE(0x98)
	else if(!strcmp(op,"clc"))	aSINGLE(0xf8)
	else if(!strcmp(op,"cld"))	aSINGLE(0xfc)
	else if(!strcmp(op,"cli"))	aSINGLE(0xfa)
	else if(!strcmp(op,"cmc"))	aSINGLE(0xf5)
	else if(!strcmp(op,"cmp"))	aGROUP1(0x07,0x38,0)
	else if(!strcmp(op,"cmpsb"))aSINGLE(0xa6)
	else if(!strcmp(op,"cmpsw"))aSINGLE(0xa7)
	else if(!strcmp(op,"cwd"))	aSINGLE(0x99)
	else if(!strcmp(op,"daa"))	aSINGLE(0x27)
	else if(!strcmp(op,"das"))	aSINGLE(0x2f)
	else if(!strcmp(op,"dec"))	aGROUP4(0x48,0x01)
	else if(!strcmp(op,"div"))	aGROUP3(0x06)
//	else if(!strcmp(op,"esc"))	ESCAPE
	else if(!strcmp(op,"hlt"))	aSINGLE(0xf4)
	else if(!strcmp(op,"idiv"))	aGROUP3(0x07)
	else if(!strcmp(op,"imul"))	aGROUP3(0x05)
	else if(!strcmp(op,"in"))	aIN
	else if(!strcmp(op,"inc"))	aGROUP4(0x40,0x00)
	else if(!strcmp(op,"int"))	aINT
//	else if(!strcmp(op,"intr+"))INTR
	else if(!strcmp(op,"into"))	aSINGLE(0xce)
	else if(!strcmp(op,"iret"))	aSINGLE(0xcf)
	else if(!strcmp(op,"ja")
		|| !strcmp(op,"jnbe"))	aJCC(0x77)
	else if(!strcmp(op,"jae")
		|| !strcmp(op,"jnb"))	aJCC(0x73)
	else if(!strcmp(op,"jb")
		|| !strcmp(op,"jnae"))	aJCC(0x72)
	else if(!strcmp(op,"jbe")
		|| !strcmp(op,"jna"))	aJCC(0x76)
	else if(!strcmp(op,"jc"))	aJCC(0x72)
	else if(!strcmp(op,"jcxz"))	aJCC(0xe3)
	else if(!strcmp(op,"je")
		|| !strcmp(op,"jz"))	aJCC(0x74)
	else if(!strcmp(op,"jg")
		|| !strcmp(op,"jnle"))	aJCC(0x7f)
	else if(!strcmp(op,"jge")
		|| !strcmp(op,"jnl"))	aJCC(0x7d)
	else if(!strcmp(op,"jl")
		|| !strcmp(op,"jnge"))	aJCC(0x7c)
	else if(!strcmp(op,"jle")
		|| !strcmp(op,"jng"))	aJCC(0x7e)
	else if(!strcmp(op,"jmp"))	aGROUP5_JMP
	else if(!strcmp(op,"jnc"))	aJCC(0x73)
	else if(!strcmp(op,"jne")
		|| !strcmp(op,"jnz"))	aJCC(0x75)
	else if(!strcmp(op,"jno"))	aJCC(0x71)
	else if(!strcmp(op,"jnp")
		|| !strcmp(op,"jpo"))	aJCC(0x7b)
	else if(!strcmp(op,"jns"))	aJCC(0x79)
	else if(!strcmp(op,"jo"))	aJCC(0x70)
	else if(!strcmp(op,"jp")
		|| !strcmp(op,"jpe"))	aJCC(0x7a)
	else if(!strcmp(op,"js"))	aJCC(0x78)
	else if(!strcmp(op,"lahf"))	aSINGLE(0x9f)
	else if(!strcmp(op,"lds"))	aLEA(0xc5)
	else if(!strcmp(op,"lea"))	aLEA(0x8d)
	else if(!strcmp(op,"les"))	aLEA(0xc4)
//	else if(!strcmp(op,"lock"))	PREFIX(0xf0)
	else if(!strcmp(op,"lodsb"))aSINGLE(0xac)
	else if(!strcmp(op,"lodsw"))aSINGLE(0xad)
	else if(!strcmp(op,"loop"))	aLOOPCC(0xe2)
	else if(!strcmp(op,"loope")
		|| !strcmp(op,"loopz"))	aLOOPCC(0xe1)
	else if(!strcmp(op,"loopne")
		|| !strcmp(op,"loopnz"))aLOOPCC(0xe0)
//	else if(!strcmp(op,"nmi+"))	NMI+
	else if(!strcmp(op,"mov"))	aMOV
	else if(!strcmp(op,"movsb"))aSINGLE(0xa4)
	else if(!strcmp(op,"movsw"))aSINGLE(0xa5)
	else if(!strcmp(op,"mul"))	aGROUP3(0x04)
	else if(!strcmp(op,"neg"))	aGROUP3(0x03)
	else if(!strcmp(op,"nop"))	aSINGLE(0x90)
	else if(!strcmp(op,"not"))	aGROUP3(0x02)
	else if(!strcmp(op,"or"))	aGROUP1(0x01,0x08,1)
	else if(!strcmp(op,"out"))	aOUT
	else if(!strcmp(op,"pop"))	aPOP
	else if(!strcmp(op,"popf"))	aSINGLE(0x9d)
	else if(!strcmp(op,"push"))	aGROUP5_PUSH
	else if(!strcmp(op,"pushf"))aSINGLE(0x9c)
	else if(!strcmp(op,"rcl"))	aGROUP2(0x02)
	else if(!strcmp(op,"rcr"))	aGROUP2(0x03)
//	else if(!strcmp(op,"rep"))	PREFIX(0xf3)
//	else if(!strcmp(op,"repe")
//		|| !strcmp(op,"repz"))	PREFIX(0xf3)
//	else if(!strcmp(op,"REPNZ")
//		|| !strcmp(op,"repnz"))	PREFIX(0xf2)
	else if(!strcmp(op,"ret"))	aRET
	else if(!strcmp(op,"retf"))	aRETF
	else if(!strcmp(op,"rol"))	aGROUP2(0x00)
	else if(!strcmp(op,"ror"))	aGROUP2(0x01)
	else if(!strcmp(op,"sahf"))	aSINGLE(0x9e)
	else if(!strcmp(op,"sal")
		|| !strcmp(op,"shl"))	aGROUP2(0x04)
	else if(!strcmp(op,"sar"))	aGROUP2(0x07)
	else if(!strcmp(op,"sbb"))	aGROUP1(0x03,0x18,0)
	else if(!strcmp(op,"scasb"))aSINGLE(0xae)
	else if(!strcmp(op,"scasw"))aSINGLE(0xaf)
//	else if(!strcmp(op,"segment+"))	SEGMENT+
	else if(!strcmp(op,"shr"))	aGROUP2(0x05)
//	else if(!strcmp(op,"singlestep+")) SINGLESTEP+
	else if(!strcmp(op,"stc"))	aSINGLE(0xf9)
	else if(!strcmp(op,"std"))	aSINGLE(0xfd)
	else if(!strcmp(op,"sti"))	aSINGLE(0xfb)
	else if(!strcmp(op,"stosb"))aSINGLE(0xaa)
	else if(!strcmp(op,"stosw"))aSINGLE(0xab)
	else if(!strcmp(op,"sub"))	aGROUP1(0x05,0x28,0)
	else if(!strcmp(op,"test"))	aGROUP3_TEST
	else if(!strcmp(op,"wait"))	aSINGLE(0x9b)
	else if(!strcmp(op,"xchg"))	aXCHG
	else if(!strcmp(op,"xlat"))	aSINGLE(0xd7)
	else if(!strcmp(op,"xor"))	aGROUP1(0x06,0x30,1)
	else len = 0;
	return len;
}

/*	read asmStmt, locCS;
	use locMemory, locSegment, locOffset;
	write (unsigned char *)(locMemory+(locSegment<<4)+locOffset)*/
int assemble(const char *asmStmt,unsigned short locCS,
	void *locMemory,
	unsigned short locSegment,
	unsigned short locOffset)
{
	unsigned char *loc = (unsigned char *)(((unsigned char *)locMemory)+(locSegment<<4)+locOffset);
	int len = 0,flag;
	char copy[0x100];
	char *prefix = NULL;
	char *db = NULL;
	char *dw = NULL;
	char *opcode = NULL;
	char *arg1 = NULL;
	char *arg2 = NULL;
	int i = 0;
	strcpy(copy,asmStmt);
	while(copy[i] != '\0') {
		if(copy[i] == ';' || copy[i] == '\n') {
			copy[i] = '\0';
			break;
		}
		if(copy[i] > 0x40 && copy[i] < 0x5b)
			copy[i] += 0x20;
		i++;
	}
	prefix = strtok(copy," ,\t\n\r\f");
	if(!prefix) return len;
	while(aPrefix(prefix,loc+len)) {
		len++;
		prefix = strtok(NULL," ,\t\n\r\f");
	}
	opcode = prefix;
	prefix = NULL;
	if(!opcode) return len;
	if(!strcmp(opcode,"db")) {
		db = strtok(NULL," ,\t\n\r\f");
		while(db) {
			if(!aDB(db,loc+len)) {len = 0;break;}
			len++;
			db = strtok(NULL," ,\t\n\r\f");
		}
	} else if(!strcmp(opcode,"dw")) {
		dw = strtok(NULL," ,\t\n\r\f");
		while(dw) {
			if(!aDW(dw,loc+len)) {len = 0;break;}
			len += 2;
			dw = strtok(NULL," ,\t\n\r\f");
		}
	} else {
		arg1 = strtok(NULL,",:");
		arg2 = strtok(NULL,",");
		flag = aOpCode(opcode,arg1,arg2,loc+len,locOffset,locCS);
		if(flag) len += flag;
		else len = 0;
	}
	return len;
}

#define dSOL dasmStmt,resOperand,loc+len
#define dEmitRM len+=dModRM
#define dEmitRI	len+=dRImm
#define dEmitSL len+=dSLabel
#define dEmitOP	len+=dOP
#define dEmitPR len+=dPrefix
#define dEmitJR len+=dJRel
#define dEmitDB len+=dDB(dSOL,opcode)
#define dEmitG1 len+=dGroup1(dSOL,opcode)
#define dEmitXC len+=dXCHG(dSOL,opcode)
#define dEmitJF len+=dJFar
#define dEmitMO	len+=dMovOff
#define dEmitMI len+=dMovImm(dSOL,opcode)
#define dEmitOI len+=dOPImm
#define dEmitG2	len+=dGroup2(dSOL,opcode)
#define dEmitAA	len+=dAAX
#define dEmitG3	len+=dGroup3(dSOL,opcode)
#define dEmitG4 len+=dGroup4(dSOL)
#define dEmitG5	len+=dGroup5(dSOL)
#define isOperandES		(resOperand->flag == 0)
#define isOperandCS		(resOperand->flag == 1)
#define isOperandSS		(resOperand->flag == 2)
#define isOperandDS		(resOperand->flag == 3)
#define isOperandNul	(resOperand->flag == 4)
#define setOperandES	{resOperand->flag = 0;}
#define setOperandCS	{resOperand->flag = 1;}
#define setOperandSS	{resOperand->flag = 2;}
#define setOperandDS	{resOperand->flag = 3;}
#define setOperandNul	{resOperand->flag = 4;}
#define dGetByte(n)	{(n) = *(loc+len++);}
#define dGetWord(n)	{(n) = *(loc+len++);(n) += ((*(loc+len++))<<8);}
#define dANY(str) {strcat(dasmStmt,(str));}
#define dCOMMA {dANY(",");}
#define dS {\
		switch(modrm.reg) {\
		case 0:	dANY("ES");break;\
		case 1:	dANY("CS");break;\
		case 2:	dANY("SS");break;\
		case 3:	dANY("DS");break;\
		default:dANY("(ERROR:REG)");break;}}
#define dR(bit) {\
	switch(bit) {\
	case 8:\
		switch(modrm.reg) {\
		case 0:	dANY("AL");break;\
		case 1:	dANY("CL");break;\
		case 2:	dANY("DL");break;\
		case 3:	dANY("BL");break;\
		case 4:	dANY("AH");break;\
		case 5:	dANY("CH");break;\
		case 6:	dANY("DH");break;\
		case 7:	dANY("BH");break;\
		default:dANY("(ERROR:REG)");break;}\
		break;\
	case 16:\
		switch(modrm.reg) {\
		case 0:	dANY("AX");break;\
		case 1:	dANY("CX");break;\
		case 2:	dANY("DX");break;\
		case 3:	dANY("BX");break;\
		case 4:	dANY("SP");break;\
		case 5:	dANY("BP");break;\
		case 6:	dANY("SI");break;\
		case 7:	dANY("DI");break;\
		default:dANY("(ERROR:REG)");break;}\
		break;\
	default:dANY("(ERROR:REG)");break;}}
#define dRM(bit) {\
	switch(bit) {\
	case 0: resOperand->len = 2;break;\
	case 8:	resOperand->len = 1;break;\
	case 16:resOperand->len = 2;break;\
	default:resOperand->len = 0;break;}\
	if(modrm.mod < 3) {\
		switch(bit) {\
		case 0:	dANY("[")			break;\
		case 8:	dANY("BYTE PTR [")	break;\
		case 16:dANY("WORD PTR [")	break;\
		default:dANY("(ERROR:PTR)")	break;}}\
	switch(modrm.mod) {\
	case 0:\
		switch(modrm.rm) {\
		case 0:	dANY("BX+SI")	break;\
		case 1:	dANY("BX+DI")	break;\
		case 2:	dANY("BP+SI")	break;\
		case 3:	dANY("BP+DI")	break;\
		case 4:	dANY("SI")		break;\
		case 5:	dANY("DI")		break;\
		case 6:	dStrCat16(dasmStmt,modrm.imm);break;\
		case 7:	dANY("BX")		break;\
		default:dANY("(ERROR:R/M)");break;}\
		break;\
	case 1:\
		switch(modrm.rm) {\
		case 0:	dANY("BX+SI+")	dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 1:	dANY("BX+DI+")	dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 2:	dANY("BP+SI+")	dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 3:	dANY("BP+DI+")	dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 4:	dANY("SI+")		dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 5:	dANY("DI+")		dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 6:	dANY("BP+")		dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		case 7:	dANY("BX+")		dStrCat8(dasmStmt,(unsigned char)modrm.imm);break;\
		default:dANY("(ERROR:R/M)")	break;}\
		break;\
	case 2:\
		switch(modrm.rm) {\
		case 0:	dANY("BX+SI+")	dStrCat16(dasmStmt,modrm.imm);break;\
		case 1:	dANY("BX+DI+")	dStrCat16(dasmStmt,modrm.imm);break;\
		case 2:	dANY("BP+SI+")	dStrCat16(dasmStmt,modrm.imm);break;\
		case 3:	dANY("BP+DI+")	dStrCat16(dasmStmt,modrm.imm);break;\
		case 4:	dANY("SI+")		dStrCat16(dasmStmt,modrm.imm);break;\
		case 5:	dANY("DI+")		dStrCat16(dasmStmt,modrm.imm);break;\
		case 6:	dANY("BP+")		dStrCat16(dasmStmt,modrm.imm);break;\
		case 7:	dANY("BX+")		dStrCat16(dasmStmt,modrm.imm);break;\
		default:dANY("(ERROR:R/M)")	break;}\
		break;\
	case 3:\
		switch(bit) {\
		case 8:\
			switch(modrm.rm) {\
			case 0:	dANY("AL")	break;\
			case 1:	dANY("CL")	break;\
			case 2:	dANY("DL")	break;\
			case 3:	dANY("BL")	break;\
			case 4:	dANY("AH")	break;\
			case 5:	dANY("CH")	break;\
			case 6:	dANY("DH")	break;\
			case 7:	dANY("BH")	break;\
			default:dANY("(ERROR:R/M)")	break;}\
			break;\
		case 16:\
			switch(modrm.rm) {\
			case 0:	dANY("AX")	break;\
			case 1:	dANY("CX")	break;\
			case 2:	dANY("DX")	break;\
			case 3:	dANY("BX")	break;\
			case 4:	dANY("SP")	break;\
			case 5:	dANY("BP")	break;\
			case 6:	dANY("SI")	break;\
			case 7:	dANY("DI")	break;\
			default:dANY("(ERROR:R/M)")	break;}\
			break;\
		default:dANY("(ERROR:R/M)")	break;}\
		break;\
	default:\
		break;\
	}\
	if(modrm.mod < 3) dANY("]");}
#define dImm(bit) {\
	switch(bit) {\
	case 8:	dGetByte(imm)	dStrCat8(dasmStmt,(unsigned char)imm);break;\
	case 16:dGetWord(imm)	dStrCat16(dasmStmt,imm);break;\
	default:dANY("(ERROR:IMM)")	break;}}
#define dOff(bit) {\
	switch(bit) {\
	case 8:	dANY("BYTE PTR [")	dGetWord(imm)	dStrCat16(dasmStmt,imm);break;\
	case 16:dANY("WORD PTR [")	dGetWord(imm)	dStrCat16(dasmStmt,imm);break;\
	default:dANY("(ERROR:IMM)")	break;}\
	dANY("]");}
#define dSImm8 {\
	dGetByte(imm)\
	if(imm&0x80) {dANY("-")	dStrCat8(dasmStmt,(unsigned char)((~imm)+1));}\
	else {dANY("+")	dStrCat8(dasmStmt,(unsigned char)imm);}}
#define dGetModRM {\
	modrm.mod = *(loc+len++);\
	modrm.rm = ((modrm.mod&0x07)>>0);\
	modrm.reg = ((modrm.mod&0x38)>>3);\
	modrm.mod = ((modrm.mod&0xc0)>>6);\
	if(dispLen(modrm)) {\
			modrm.imm = *(loc+len++);\
			if(dispLen(modrm) == 2) modrm.imm += ((*(loc+len++))<<8);\
	} else modrm.imm = 0;\
	if(isOperandNul) {\
		if((modrm.mod == 0 && (modrm.rm == 2 || modrm.rm == 3)) || \
			((modrm.mod == 1 || modrm.mod == 2) && (modrm.rm == 2 || modrm.rm == 3 || modrm.rm == 6))\
			) setOperandSS\
		else if(modrm.mod != 3)setOperandDS\
	}\
	if(modrm.mod == 3) setOperandNul\
	if(!isOperandNul) {\
		resOperand->seg = 0x01;\
		resOperand->mod = modrm.mod;\
		resOperand->rm = modrm.rm;\
		resOperand->imm = modrm.imm;\
	}\
}

typedef enum {RM8_R8,R8_RM8,RM16_R16,R16_RM16,
	RM8_I8,I8_RM8,RM16_I16,I16_RM16,
	RM16_S16,S16_RM16,
	AL_I8,I8_AL,AX_I16,I16_AX,
	R16_MX,RM16_NONE,
	AX_I8,I8_AX
} InsType;
/*	write dasmStmt, resOperand;
	use locMemory, locSegment, locOffset;
	read (unsigned char *)(locMemory+(locSegment<<4)+locOffset)*/
static void dStrCat8(char *str,unsigned char n)
{
	char c,s[3];
	int i;
	s[2] = '\0';
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		s[1-i] = c;
	}
	strcat(str,s);
}
static void dStrCat16(char *str,unsigned short n)
{
	char c,s[5];
	int i;
	s[4] = '\0';
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		s[3-i] = c;
	}
	strcat(str,s);
}
static int dModRM(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op,InsType it)
{
	int len = 0;
	unsigned short imm;
	ModRM modrm;
	dANY(op);
	dANY("\t");
	dGetModRM
	/*if(isOperandNul) {
		if((modrm.mod == 0 && (modrm.rm == 2 || modrm.rm == 3)) || 
			((modrm.mod == 1 || modrm.mod == 2) && (modrm.rm == 2 || modrm.rm == 3 || modrm.rm == 6))
			) setOperandSS
		else if(modrm.mod == 3) setOperandNul
		else setOperandDS
	}
	if(!isOperandNul) {
		resOperand->seg = 0x01;
		resOperand->mod = modrm.mod;
		resOperand->rm = modrm.rm;
		resOperand->imm = modrm.imm;
	}*/
	switch(it) {
	case RM8_R8:		dRM(8)		dCOMMA	dR(8)		break;
	case R8_RM8:		dR(8)		dCOMMA	dRM(8)		break;
	case RM16_R16:		dRM(16)		dCOMMA	dR(16)		break;
	case R16_RM16:		dR(16)		dCOMMA	dRM(16)		break;
	case RM8_I8:		dRM(8)		dCOMMA	dImm(8)		break;
	case I8_RM8:		dImm(8)		dCOMMA	dRM(8)		break;
	case RM16_I16:		dRM(16)		dCOMMA	dImm(16)	break;
	case I16_RM16:		dImm(16)	dCOMMA	dRM(16)		break;
	case RM16_S16:		dRM(16)		dCOMMA	dS			break;
	case S16_RM16:		dS			dCOMMA	dRM(16)		break;
	case R16_MX:		dR(16)		dCOMMA	dRM(0)		break;
	case RM16_NONE:		dRM(0)							break;
	default:	dANY("(ERROR:INSFORMAT)")				break;
	}
	return len;
}
static int dRImm(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op,InsType it)
{
	int len = 0;
	unsigned short imm;
	setOperandNul
	dANY(op)
	dANY("\t")
	switch(it) {\
	case AL_I8:		dANY("AL")	dCOMMA	dImm(8)		break;
	case I8_AL:		dImm(8)		dCOMMA	dANY("AL")	break;
	case AX_I16:	dANY("AX")	dCOMMA	dImm(16)	break;
	case I16_AX:	dImm(16)	dCOMMA	dANY("AX")	break;
	case AX_I8:		dANY("AX")	dCOMMA	dImm(8)		break;
	case I8_AX:		dImm(8)		dCOMMA	dANY("AX")	break;
	default:		dANY("(ERROR:REGIMM)")			break;}
	return len;
}
static int dSLabel(char *dasmStmt,Operand *resOperand,unsigned char *loc,int op)
{
	int len = 0;
	resOperand->flag = op;
	resOperand->seg = 0x00;
	switch(op) {
	case 0:	dANY("ES:")	break;
	case 1:	dANY("CS:")	break;
	case 2:	dANY("SS:")	break;
	case 3:	dANY("DS:")	break;
	default:dANY("(ERROR:REG)")	break;}
	return len;
}
static int dOP(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op)
{
	int len = 0;
	setOperandNul
	dANY(op);
	return len;
}
static int dPrefix(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op)
{
	int len = 0;
	dANY(op);
	return len;
}
static int dDB(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char num)
{
	int len = 0;
	setOperandNul
	dANY("DB\t")
	dStrCat8(dasmStmt,num);
	return len;
}
static int dJRel(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op,unsigned short locOffset,int bit)
{
	int len = 0;
	unsigned short rel;
	setOperandNul
	dANY(op);
	dANY("\t")
	switch(bit) {
	case 8:
		rel = *(loc+len++);
		if(rel < 0x80) rel = locOffset+2+rel;
		else rel = locOffset+2+rel-0x100;
		break;
	case 16:
		rel = *(loc+len++);
		rel += (*(loc+len++)<<8);
		rel += locOffset+3;
		break;
	//case 1616:

	default:	dANY("(ERROR:JREL)")	break;}
	dStrCat16(dasmStmt,rel);
	return len;
}
static int dGroup1(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char op)
{	// op = 0x80,0x81,0x82,0x83
	int len = 0;
	int sign = 0;
	unsigned short imm;
	ModRM modrm;
	dGetModRM
	switch(modrm.reg) {
	case 0:	dANY("ADD")			break;
	case 1: dANY("OR") sign = 1;break;
	case 2: dANY("ADC")			break;
	case 3:	dANY("SBB")			break;
	case 4:	dANY("AND")sign = 1;break;
	case 5: dANY("SUB")			break;
	case 6: dANY("XOR")sign = 1;break;
	case 7: dANY("CMP")			break;
	default:dANY("(ERROR:INS)")	break;
	}
	dANY("\t")
	switch(op) {
	case 0x80:	dRM(8)	dCOMMA	dImm(8)		break;
	case 0x81:	dRM(16)	dCOMMA	dImm(16)	break;
	case 0x82:	dRM(8)	dCOMMA	dImm(8)		break;
	case 0x83:	dRM(16)	dCOMMA	dSImm8		break;
	default:	dANY("(ERROR:INSFORMAT)")	break;
	}
	return len;
}
static int dXCHG(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char op)
{
	int len = 0;
	ModRM modrm;
	setOperandNul
	dANY("XCHG\t")
	modrm.reg = op-0x90;
	dR(16)	dCOMMA	dANY("AX")
	return len;
}
static int dMovOff(char *dasmStmt,Operand *resOperand,unsigned char *loc,InsType it)
{
	int len = 0;
	unsigned short imm;
	if(isOperandNul) setOperandDS
	resOperand->seg = 0x01;
	resOperand->mod = 0x00;
	resOperand->rm = 0x06;
	resOperand->len = 1;
	dANY("MOV\t")
	switch(it) {\
	case AL_I8:		dANY("AL")	dCOMMA	dOff(8)		break;
	case I8_AL:		dOff(8)		dCOMMA	dANY("AL")	break;
	case AX_I16:	dANY("AX")	dCOMMA	dOff(16)	break;
	case I16_AX:	dOff(16)	dCOMMA	dANY("AX")	break;
	default:dANY("(ERROR:REGIMM)");break;}
	resOperand->imm = imm;
	return len;
}
static int dMovImm(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char op)
{
	int len = 0;
	unsigned short imm;
	ModRM modrm;
	unsigned char myop;
	setOperandNul
	if(op > 0xb7) myop = 0xb8;
	else myop = 0xb0;
	dANY("MOV\t")
	modrm.reg = op-myop;
	switch(myop) {
	case 0xb0:	dR(8)	dCOMMA	dImm(8)		break;
	case 0xb8:	dR(16)	dCOMMA	dImm(16)	break;
	default:dANY("(ERROR:MOVOP)");break;}
	return len;
}
static int dOPImm(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op,int bit)
{
	int len = 0;
	unsigned short imm;
	setOperandNul
	dANY(op)	dANY("\t")	dImm(bit)
	return len;
}
static int dGroup2(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char op)
{	// op = 0xd0,0xd1,0xd2,0xd3
	int len = 0;
	//unsigned short imm;
	ModRM modrm;
	dGetModRM
	switch(modrm.reg) {
	case 0:	dANY("ROL")	break;
	case 1: dANY("ROR")	break;
	case 2: dANY("RCL")	break;
	case 3:	dANY("RCR")	break;
	case 4:	dANY("SHL")	break;
	case 5:	dANY("SHR")	break;
	//case 6:	dANY("SAL")	break;
	case 7:	dANY("SAR")	break;
	default:dANY("(ERROR:OPD0H)")break;}
	dANY("\t")
	switch(op) {
	case 0xd0:	dRM(8)	if(modrm.reg < 4) dANY(",1")	break;
	case 0xd1:	dRM(16)	if(modrm.reg < 4) dANY(",1")	break;
	case 0xd2:	dRM(8)	dANY(",CL")	break;
	case 0xd3:	dRM(16)	dANY(",CL")	break;}
	return len;
}
static int dAAX(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op)
{
	int len = 0;
	unsigned short imm;
	setOperandNul
	dGetByte(imm)
	dANY(op)
	if(imm != 0x0a) {dANY("\t")	dStrCat8(dasmStmt,(unsigned char)imm);}
	return len;
}
static int dJFar(char *dasmStmt,Operand *resOperand,unsigned char *loc,const char *op)
{
	int len = 0;
	unsigned short seg,ptr;
	setOperandNul
	dANY(op)
	dANY("\t")
	dGetWord(ptr)
	dGetWord(seg)
	dStrCat16(dasmStmt,seg);
	dANY(":")
	dStrCat16(dasmStmt,ptr);
	return len;
}
static int dGroup3(char *dasmStmt,Operand *resOperand,unsigned char *loc,unsigned char op)
{
	int len = 0;
	unsigned short imm;
	ModRM modrm;
	dGetModRM
	switch(modrm.reg) {
	case 0:	dANY("TEST\t")	break;
	case 2:	dANY("NOT\t")	break;
	case 3:	dANY("NEG\t")	break;
	case 4:	dANY("MUL\t")	break;
	case 5:	dANY("IMUL\t")	break;
	case 6:	dANY("DIV\t")	break;
	case 7:	dANY("IDIV\t")	break;
	default:dANY("(ERROR:GROUP3)\t")	break;
	}
	switch(op) {
	case 0xf6:	dRM(8)	if(modrm.reg == 0) {dCOMMA	dImm(8)	}	break;
	case 0xf7:	dRM(16)	if(modrm.reg == 0) {dCOMMA	dImm(16)}	break;
	default:dANY("(ERROR:GROUP3OP)");break;
	}
	return len;
}
static int dGroup4(char *dasmStmt,Operand *resOperand,unsigned char *loc)
{
	int len = 0;
	ModRM modrm;
	dGetModRM
	switch(modrm.reg) {
	case 0:	dANY("INC\t")	break;
	case 1:	dANY("DEC\t")	break;
	default:dANY("(ERROR:GROUP4)\t")	break;
	}
	dRM(8)
	return len;
}
static int dGroup5(char *dasmStmt,Operand *resOperand,unsigned char *loc)
{
	int len = 0;
	ModRM modrm;
	dGetModRM
	switch(modrm.reg) {
	case 0:	dANY("INC\t")		break;
	case 1:	dANY("DEC\t")		break;
	case 2:	dANY("CALL\t")		break;
	case 3:	dANY("CALL\tFAR ")	break;
	case 4:	dANY("JMP\t")		break;
	case 5:	dANY("JMP\tFAR ")	break;
	case 6:	dANY("PUSH\t")		break;
	default:dANY("(ERROR:GROUP5)\t")	break;
	}
	dRM(0)
	return len;
}

int disassemble(char *dasmStmt,Operand *resOperand,
	const void *locMemory,
	const unsigned short locSegment,
	const unsigned short locOffset)
{
	//unsigned short imm;
	int len = 0;
	unsigned char opcode;
	unsigned char *loc = (unsigned char *)(((unsigned char *)locMemory)+(locSegment<<4)+locOffset);
	if(!isOperandES && !isOperandCS && !isOperandSS && !isOperandES) {
		setOperandNul
		resOperand->seg = 0x00;
	}
	strcpy(dasmStmt,"\0");
	dGetByte(opcode)
	switch(opcode) {
	case 0x00:	dEmitRM(dSOL,"ADD",RM8_R8);		break;
	case 0x01:	dEmitRM(dSOL,"ADD",RM16_R16);	break;
	case 0x02:	dEmitRM(dSOL,"ADD",R8_RM8);		break;
	case 0x03:	dEmitRM(dSOL,"ADD",R16_RM16);	break;
	case 0x04:	dEmitRI(dSOL,"ADD",AL_I8);		break;
	case 0x05:	dEmitRI(dSOL,"ADD",AX_I16);		break;
	case 0x06:	dEmitOP(dSOL,"PUSH\tES");		break;
	case 0x07:	dEmitOP(dSOL,"POP\tES");		break;
	case 0x08:	dEmitRM(dSOL,"OR",RM8_R8);		break;
	case 0x09:	dEmitRM(dSOL,"OR",RM16_R16);	break;
	case 0x0a:	dEmitRM(dSOL,"OR",R8_RM8);		break;
	case 0x0b:	dEmitRM(dSOL,"OR",R16_RM16);	break;
	case 0x0c:	dEmitRI(dSOL,"OR",AL_I8);		break;
	case 0x0d:	dEmitRI(dSOL,"OR",AX_I16);		break;
	case 0x0e:	dEmitOP(dSOL,"PUSH\tCS");		break;
	case 0x0f:	dEmitOP(dSOL,"POP\tCS");		break; /* IS NO LONGER 'POP CS' AFTER 8086 */
	case 0x10:	dEmitRM(dSOL,"ADC",RM8_R8);		break;
	case 0x11:	dEmitRM(dSOL,"ADC",RM16_R16);	break;
	case 0x12:	dEmitRM(dSOL,"ADC",R8_RM8);		break;
	case 0x13:	dEmitRM(dSOL,"ADC",R16_RM16);	break;
	case 0x14:	dEmitRI(dSOL,"ADC",AL_I8);		break;
	case 0x15:	dEmitRI(dSOL,"ADC",AX_I16);		break;
	case 0x16:	dEmitOP(dSOL,"PUSH\tSS");		break;
	case 0x17:	dEmitOP(dSOL,"POP\tSS");		break;
	case 0x18:	dEmitRM(dSOL,"SBB",RM8_R8);		break;
	case 0x19:	dEmitRM(dSOL,"SBB",RM16_R16);	break;
	case 0x1a:	dEmitRM(dSOL,"SBB",R8_RM8);		break;
	case 0x1b:	dEmitRM(dSOL,"SBB",R16_RM16);	break;
	case 0x1c:	dEmitRI(dSOL,"SBB",AL_I8);		break;
	case 0x1d:	dEmitRI(dSOL,"SBB",AX_I16);		break;
	case 0x1e:	dEmitOP(dSOL,"PUSH\tDS");		break;
	case 0x1f:	dEmitOP(dSOL,"POP\tDS");		break;
	case 0x20:	dEmitRM(dSOL,"AND",RM8_R8);		break;
	case 0x21:	dEmitRM(dSOL,"AND",RM16_R16);	break;
	case 0x22:	dEmitRM(dSOL,"AND",R8_RM8);		break;
	case 0x23:	dEmitRM(dSOL,"AND",R16_RM16);	break;
	case 0x24:	dEmitRI(dSOL,"AND",AL_I8);		break;
	case 0x25:	dEmitRI(dSOL,"AND",AX_I16);		break;
	case 0x26:	dEmitSL(dSOL,0);				break;
	case 0x27:	dEmitOP(dSOL,"DAA")	;			break;
	case 0x28:	dEmitRM(dSOL,"SUB",RM8_R8);		break;
	case 0x29:	dEmitRM(dSOL,"SUB",RM16_R16);	break;
	case 0x2a:	dEmitRM(dSOL,"SUB",R8_RM8);		break;
	case 0x2b:	dEmitRM(dSOL,"SUB",R16_RM16);	break;
	case 0x2c:	dEmitRI(dSOL,"SUB",AL_I8);		break;
	case 0x2d:	dEmitRI(dSOL,"SUB",AX_I16);		break;
	case 0x2e:	dEmitSL(dSOL,1);				break;
	case 0x2f:	dEmitOP(dSOL,"DAS");			break;
	case 0x30:	dEmitRM(dSOL,"XOR",RM8_R8);		break;
	case 0x31:	dEmitRM(dSOL,"XOR",RM16_R16);	break;
	case 0x32:	dEmitRM(dSOL,"XOR",R8_RM8);		break;
	case 0x33:	dEmitRM(dSOL,"XOR",R16_RM16);	break;
	case 0x34:	dEmitRI(dSOL,"XOR",AL_I8);		break;
	case 0x35:	dEmitRI(dSOL,"XOR",AX_I16);		break;
	case 0x36:	dEmitSL(dSOL,2);				break;
	case 0x37:	dEmitOP(dSOL,"AAA");			break;
	case 0x38:	dEmitRM(dSOL,"CMP",RM8_R8);		break;
	case 0x39:	dEmitRM(dSOL,"CMP",RM16_R16);	break;
	case 0x3a:	dEmitRM(dSOL,"CMP",R8_RM8);		break;
	case 0x3b:	dEmitRM(dSOL,"CMP",R16_RM16);	break;
	case 0x3c:	dEmitRI(dSOL,"CMP",AL_I8);		break;
	case 0x3d:	dEmitRI(dSOL,"CMP",AX_I16);		break;
	case 0x3e:	dEmitSL(dSOL,3);				break;
	case 0x3f:	dEmitOP(dSOL,"AAS");			break;
	case 0x40:	dEmitOP(dSOL,"INC\tAX");		break;
	case 0x41:	dEmitOP(dSOL,"INC\tCX");		break;
	case 0x42:	dEmitOP(dSOL,"INC\tDX");		break;
	case 0x43:	dEmitOP(dSOL,"INC\tBX");		break;
	case 0x44:	dEmitOP(dSOL,"INC\tSP");		break;
	case 0x45:	dEmitOP(dSOL,"INC\tBP");		break;
	case 0x46:	dEmitOP(dSOL,"INC\tSI");		break;
	case 0x47:	dEmitOP(dSOL,"INC\tDI");		break;
	case 0x48:	dEmitOP(dSOL,"DEC\tAX");		break;
	case 0x49:	dEmitOP(dSOL,"DEC\tCX");		break;
	case 0x4a:	dEmitOP(dSOL,"DEC\tDX");		break;
	case 0x4b:	dEmitOP(dSOL,"DEC\tBX");		break;
	case 0x4c:	dEmitOP(dSOL,"DEC\tSP");		break;
	case 0x4d:	dEmitOP(dSOL,"DEC\tBP");		break;
	case 0x4e:	dEmitOP(dSOL,"DEC\tSI");		break;
	case 0x4f:	dEmitOP(dSOL,"DEC\tDI");		break;
	case 0x50:	dEmitOP(dSOL,"PUSH\tAX");		break;
	case 0x51:	dEmitOP(dSOL,"PUSH\tCX");		break;
	case 0x52:	dEmitOP(dSOL,"PUSH\tDX");		break;
	case 0x53:	dEmitOP(dSOL,"PUSH\tBX");		break;
	case 0x54:	dEmitOP(dSOL,"PUSH\tSP");		break;
	case 0x55:	dEmitOP(dSOL,"PUSH\tBP");		break;
	case 0x56:	dEmitOP(dSOL,"PUSH\tSI");		break;
	case 0x57:	dEmitOP(dSOL,"PUSH\tDI");		break;
	case 0x58:	dEmitOP(dSOL,"POP\tAX");		break;
	case 0x59:	dEmitOP(dSOL,"POP\tCX");		break;
	case 0x5a:	dEmitOP(dSOL,"POP\tDX");		break;
	case 0x5b:	dEmitOP(dSOL,"POP\tBX");		break;
	case 0x5c:	dEmitOP(dSOL,"POP\tSP");		break;
	case 0x5d:	dEmitOP(dSOL,"POP\tBP");		break;
	case 0x5e:	dEmitOP(dSOL,"POP\tSI");		break;
	case 0x5f:	dEmitOP(dSOL,"POP\tDI");		break;
	case 0x60:	dEmitDB;						break;
	case 0x61:	dEmitDB;						break;
	case 0x62:	dEmitDB;						break;
	case 0x63:	dEmitDB;						break;
	case 0x64:	dEmitDB;						break;
	case 0x65:	dEmitDB;						break;
	case 0x66:	dEmitDB;						break;
	case 0x67:	dEmitDB;						break;
	case 0x68:	dEmitDB;						break;
	case 0x69:	dEmitDB;						break;
	case 0x6a:	dEmitDB;						break;
	case 0x6b:	dEmitDB;						break;
	case 0x6c:	dEmitDB;						break;
	case 0x6d:	dEmitDB;						break;
	case 0x6e:	dEmitDB;						break;
	case 0x6f:	dEmitDB;						break;
	case 0x70:	dEmitJR(dSOL,"JO",locOffset,8);	break;
	case 0x71:	dEmitJR(dSOL,"JNO",locOffset,8);break;
	case 0x72:	dEmitJR(dSOL,"JC",locOffset,8);	break;
	case 0x73:	dEmitJR(dSOL,"JNC",locOffset,8);break;
	case 0x74:	dEmitJR(dSOL,"JZ",locOffset,8);	break;
	case 0x75:	dEmitJR(dSOL,"JNZ",locOffset,8);break;
	case 0x76:	dEmitJR(dSOL,"JBE",locOffset,8);break;
	case 0x77:	dEmitJR(dSOL,"JA",locOffset,8);	break;
	case 0x78:	dEmitJR(dSOL,"JS",locOffset,8);	break;
	case 0x79:	dEmitJR(dSOL,"JNS",locOffset,8);break;
	case 0x7a:	dEmitJR(dSOL,"JP",locOffset,8);	break;
	case 0x7b:	dEmitJR(dSOL,"JNP",locOffset,8);break;
	case 0x7c:	dEmitJR(dSOL,"JL",locOffset,8);	break;
	case 0x7d:	dEmitJR(dSOL,"JNL",locOffset,8);break;
	case 0x7e:	dEmitJR(dSOL,"JLE",locOffset,8);break;
	case 0x7f:	dEmitJR(dSOL,"JG",locOffset,8);	break;
	case 0x80:	dEmitG1;						break;
	case 0x81:	dEmitG1;						break;
	case 0x82:	dEmitG1;						break;
	case 0x83:	dEmitG1;						break;
	case 0x84:	dEmitRM(dSOL,"TEST",RM8_R8);	break;
	case 0x85:	dEmitRM(dSOL,"TEST",RM16_R16);	break;
	case 0x86:	dEmitRM(dSOL,"XCHG",R8_RM8);	break;
	case 0x87:	dEmitRM(dSOL,"XCHG",R16_RM16);	break;
	case 0x88:	dEmitRM(dSOL,"MOV",RM8_R8);		break;
	case 0x89:	dEmitRM(dSOL,"MOV",RM16_R16);	break;
	case 0x8a:	dEmitRM(dSOL,"MOV",R8_RM8);		break;
	case 0x8b:	dEmitRM(dSOL,"MOV",R16_RM16);	break;
	case 0x8c:	dEmitRM(dSOL,"MOV",RM16_S16);	break;
	case 0x8d:	dEmitRM(dSOL,"LEA",R16_MX);		break;
	case 0x8e:	dEmitRM(dSOL,"MOV",S16_RM16);	break;
	case 0x8f:	dEmitRM(dSOL,"POP",RM16_NONE);	break;
	case 0x90:	dEmitOP(dSOL,"NOP");			break;
	case 0x91:	dEmitXC;						break;
	case 0x92:	dEmitXC;						break;
	case 0x93:	dEmitXC;						break;
	case 0x94:	dEmitXC;						break;
	case 0x95:	dEmitXC;						break;
	case 0x96:	dEmitXC;						break;
	case 0x97:	dEmitXC;						break;
	case 0x98:	dEmitOP(dSOL,"CBW");			break;
	case 0x99:	dEmitOP(dSOL,"CWD");			break;
	case 0x9a:	dEmitJF(dSOL,"CALL");			break;
	case 0x9b:	dEmitOP(dSOL,"WAIT");			break;
	case 0x9c:	dEmitOP(dSOL,"PUSHF");			break;
	case 0x9d:	dEmitOP(dSOL,"POPF");			break;
	case 0x9e:	dEmitOP(dSOL,"SAHF");			break;
	case 0x9f:	dEmitOP(dSOL,"LAHF");			break;
	case 0xa0:	dEmitMO(dSOL,AL_I8);			break;
	case 0xa1:	dEmitMO(dSOL,AX_I16);			break;
	case 0xa2:	dEmitMO(dSOL,I8_AL);			break;
	case 0xa3:	dEmitMO(dSOL,I16_AX);			break;
	case 0xa4:	dEmitOP(dSOL,"MOVSB");			break;
	case 0xa5:	dEmitOP(dSOL,"MOVSW");			break;
	case 0xa6:	dEmitOP(dSOL,"CMPSB");			break;
	case 0xa7:	dEmitOP(dSOL,"CMPSW");			break;
	case 0xa8:	dEmitRI(dSOL,"TEST",AL_I8);		break;
	case 0xa9:	dEmitRI(dSOL,"TEST",AX_I16);	break;
	case 0xaa:	dEmitOP(dSOL,"STOSB");			break;
	case 0xab:	dEmitOP(dSOL,"STOSW");			break;
	case 0xac:	dEmitOP(dSOL,"LODSB");			break;
	case 0xad:	dEmitOP(dSOL,"LODSW");			break;
	case 0xae:	dEmitOP(dSOL,"SCASB");			break;
	case 0xaf:	dEmitOP(dSOL,"SCASW");			break;
	case 0xb0:	dEmitMI;						break;
	case 0xb1:	dEmitMI;						break;
	case 0xb2:	dEmitMI;						break;
	case 0xb3:	dEmitMI;						break;
	case 0xb4:	dEmitMI;						break;
	case 0xb5:	dEmitMI;						break;
	case 0xb6:	dEmitMI;						break;
	case 0xb7:	dEmitMI;						break;
	case 0xb8:	dEmitMI;						break;
	case 0xb9:	dEmitMI;						break;
	case 0xba:	dEmitMI;						break;
	case 0xbb:	dEmitMI;						break;
	case 0xbc:	dEmitMI;						break;
	case 0xbd:	dEmitMI;						break;
	case 0xbe:	dEmitMI;						break;
	case 0xbf:	dEmitMI;						break;
	case 0xc0:	dEmitDB;						break;
	case 0xc1:	dEmitDB;						break;
	case 0xc2:	dEmitOI(dSOL,"RET",16);			break;
	case 0xc3:	dEmitOP(dSOL,"RET");			break;
	case 0xc4:	dEmitRM(dSOL,"LES",R16_MX);		break;
	case 0xc5:	dEmitRM(dSOL,"LDS",R16_MX);		break;
	case 0xc6:	dEmitRM(dSOL,"MOV",RM8_I8);		break;
	case 0xc7:	dEmitRM(dSOL,"MOV",RM16_I16);	break;
	case 0xc8:	dEmitDB;						break;
	case 0xc9:	dEmitDB;						break;
	case 0xca:	dEmitOI(dSOL,"RETF",16);		break;
	case 0xcb:	dEmitOP(dSOL,"RETF");			break;
	case 0xcc:	dEmitOP(dSOL,"INT\t3");			break;
	case 0xcd:	dEmitOI(dSOL,"INT",8);			break;
	case 0xce:	dEmitOP(dSOL,"INTO");			break;
	case 0xcf:	dEmitOP(dSOL,"IRET");			break;
	case 0xd0:	dEmitG2;						break;
	case 0xd1:	dEmitG2;						break;
	case 0xd2:	dEmitG2;						break;
	case 0xd3:	dEmitG2;						break;
	case 0xd4:	dEmitAA(dSOL,"AAM");			break;
	case 0xd5:	dEmitAA(dSOL,"AAD");			break;
	case 0xd6:	dEmitDB;						break;
	case 0xd7:	dEmitOP(dSOL,"XLAT");			break;
	case 0xd8:	dEmitDB;						break;
	case 0xd9:	dEmitDB;						break;
	case 0xda:	dEmitDB;						break;
	case 0xdb:	dEmitDB;						break;
	case 0xdc:	dEmitDB;						break;
	case 0xdd:	dEmitDB;						break;
	case 0xde:	dEmitDB;						break;
	case 0xdf:	dEmitDB;						break;
	case 0xe0:	dEmitJR(dSOL,"LOOPNZ",locOffset,8);break;
	case 0xe1:	dEmitJR(dSOL,"LOOPZ",locOffset,8);break;
	case 0xe2:	dEmitJR(dSOL,"LOOP",locOffset,8);break;
	case 0xe3:	dEmitJR(dSOL,"JCXZ",locOffset,8);break;
	case 0xe4:	dEmitRI(dSOL,"IN",AL_I8);		break;
	case 0xe5:	dEmitRI(dSOL,"IN",AX_I8);		break;
	case 0xe6:	dEmitRI(dSOL,"OUT",I8_AL);		break;
	case 0xe7:	dEmitRI(dSOL,"OUT",I8_AX);		break;
	case 0xe8:	dEmitJR(dSOL,"CALL",locOffset,16);break;
	case 0xe9:	dEmitJR(dSOL,"JMP",locOffset,16);break;
	case 0xea:	dEmitJF(dSOL,"JMP");			break;
	case 0xeb:	dEmitJR(dSOL,"JMP",locOffset,8);break;
	case 0xec:	dEmitOP(dSOL,"IN\tAL,DX");		break;
	case 0xed:	dEmitOP(dSOL,"IN\tAX,DX");		break;
	case 0xee:	dEmitOP(dSOL,"OUT\tDX,AL");		break;
	case 0xef:	dEmitOP(dSOL,"OUT\tDX,AX");		break;
	case 0xf0:	dEmitPR(dSOL,"LOCK");			break;
	case 0xf1:	dEmitDB;						break;
	case 0xf2:	dEmitPR(dSOL,"REPNZ");			break;
	case 0xf3:	dEmitPR(dSOL,"REPZ");			break;
	case 0xf4:	dEmitOP(dSOL,"HLT");			break;
	case 0xf5:	dEmitOP(dSOL,"CMC");			break;
	case 0xf6:	dEmitG3;						break;
	case 0xf7:	dEmitG3;						break;
	case 0xf8:	dEmitOP(dSOL,"CLC");			break;
	case 0xf9:	dEmitOP(dSOL,"STC");			break;
	case 0xfa:	dEmitOP(dSOL,"CLI");			break;
	case 0xfb:	dEmitOP(dSOL,"STI");			break;
	case 0xfc:	dEmitOP(dSOL,"CLD");			break;
	case 0xfd:	dEmitOP(dSOL,"STD");			break;
	case 0xfe:	dEmitG4;						break;
	case 0xff:	dEmitG5;						break;
	default:	dANY("(IMPOSSIBLE)")			break;}
	return len;
}