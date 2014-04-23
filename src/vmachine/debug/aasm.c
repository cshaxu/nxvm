/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"

#include "../vmachine.h"
#include "aasm.h"

static char* STRCAT(char *_Dest, const char *_Source)
{return strcat(_Dest, _Source);}
static char* STRCPY(char *_Dest, const char *_Source)
{return strcpy(_Dest, _Source);}
static char* STRTOK(char *_Str, const char *_Delim)
{return strtok(_Str, _Delim);}
static int STRCMP(const char *_Str1, const char *_Str2)
{return strcmp(_Str1, _Str2);}
static int SPRINTF(char *_Dest, const char *_Format, ...)
{
	int nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, _Format);
	nWrittenBytes = vsprintf(_Dest, _Format, arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}

static char pool[0x1000];
static t_string astmt, aop, aopr1, aopr2;
static t_nubit16 avcs, avip;

#define setbyte(n) (vramVarByte(avcs, avip) = (t_nubit8)(n))
#define setword(n) (vramVarWord(avcs, avip) = (t_nubit16)(n))

static void SetImm8(t_nubit8 byte)
{
	setbyte(byte);avip++;
}

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
