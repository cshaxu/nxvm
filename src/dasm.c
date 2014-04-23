#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"

static char output[0x100];
static char rm[0x10];
static char r[0x10];
static char imm[0x10];
static int len;
static int rid;
static t_nubit16 dcs, dip;

static char* STRCAT(char *_Dest, const char *_Source)
{return strcat(_Dest, _Source);}
static char* STRCPY(char *_Dest, const char *_Source)
{return strcpy(_Dest, _Source);}
static char* STRTOK(char *_Str, const char *_Delim)
{return strtok(_Str, _Delim);}
static int STRCMP(const char *_Str1, const char *_Str2)
{return strcmp(_Str1, _Str2);}

#define MOD ((modrm&0xc0)>>6)
#define REG ((modrm&0x38)>>3)
#define RM  ((modrm&0x07)>>0)

static void GetModRegRM(t_nubit8 regbit,t_nubit8 rmbit)
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = memByte(cs, ip++);
	char sign;
	t_nubit8 disp8u;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0: sprintf(rm, "[BX+SI]");break;
		case 1: sprintf(rm, "[BX+DI]");break;
		case 2: sprintf(rm, "[BP+SI]");break;
		case 3: sprintf(rm, "[BP+DI]");break;
		case 4: sprintf(rm, "[SI]");break;
		case 5: sprintf(rm, "[DI]");break;
		case 6: disp16 = memWord(cs, ip);ip += 2;
			    sprintf(rm, "[%04X]", disp16); break;
		case 7: sprintf(rm, "[BX]");break;
		default:sprintf(rm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 1:
		disp8 = (t_nsbit8)memByte(cs, ip);ip += 1;
		sign = (disp8 & 0x80) ? '-' : '+';
		disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
		switch(RM) {
		case 0: sprintf(rm, "[BX+SI%c%02X]", sign, disp8u);break;
		case 1: sprintf(rm, "[BX+DI%c%02X]", sign, disp8u);break;
		case 2: sprintf(rm, "[BP+SI%c%02X]", sign, disp8u);break;
		case 3: sprintf(rm, "[BP+DI%c%02X]", sign, disp8u);break;
		case 4: sprintf(rm, "[SI%c%02X]",    sign, disp8u);break;
		case 5: sprintf(rm, "[DI%c%02X]",    sign, disp8u);break;
		case 6: sprintf(rm, "[BP%c%02X]",    sign, disp8u);break;
		case 7: sprintf(rm, "[BX%c%02X]",    sign, disp8u);break;
		default:sprintf(rm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 2:
		disp16 = memWord(cs,ip);ip += 2;
		switch(RM) {
		case 0: sprintf(rm, "[BX+SI+%04X]", disp16);break;
		case 1: sprintf(rm, "[BX+DI+%04X]", disp16);break;
		case 2: sprintf(rm, "[BP+SI+%04X]", disp16);break;
		case 3: sprintf(rm, "[BP+DI+%04X]", disp16);break;
		case 4: sprintf(rm, "[SI+%04X]",    disp16);break;
		case 5: sprintf(rm, "[DI+%04X]",    disp16);break;
		case 6: sprintf(rm, "[BP+%04X]",    disp16);break;
		case 7: sprintf(rm, "[BX+%04X]",    disp16);break;
		default:sprintf(rm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 3:
		switch(RM) {
		case 0: if(rmbit == 8) sprintf(rm, "AL"); else sprintf(rm, "AX"); break;
		case 1: if(rmbit == 8) sprintf(rm, "CL"); else sprintf(rm, "CX"); break;
		case 2: if(rmbit == 8) sprintf(rm, "DL"); else sprintf(rm, "DX"); break;
		case 3: if(rmbit == 8) sprintf(rm, "BL"); else sprintf(rm, "BX"); break;
		case 4: if(rmbit == 8) sprintf(rm, "AH"); else sprintf(rm, "SP"); break;
		case 5: if(rmbit == 8) sprintf(rm, "CH"); else sprintf(rm, "BP"); break;
		case 6: if(rmbit == 8) sprintf(rm, "DH"); else sprintf(rm, "SI"); break;
		case 7: if(rmbit == 8) sprintf(rm, "BH"); else sprintf(rm, "DI"); break;
		default:sprintf(rm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	default:sprintf(rm, "<ERROR:MOD(%02X)>", MOD);break;}
	switch(regbit) {
	case 0: rid = REG;break;
	case 4:
		switch(REG) {
		case 0: sprintf(r, "ES");break;
		case 1: sprintf(r, "CS");break;
		case 2: sprintf(r, "SS");break;
		case 3: sprintf(r, "DS");break;
		default:sprintf(r, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	case 8:
		switch(REG) {
		case 0:	sprintf(r, "AL");break;
		case 1:	sprintf(r, "CL");break;
		case 2:	sprintf(r, "DL");break;
		case 3:	sprintf(r, "BL");break;
		case 4:	sprintf(r, "AH");break;
		case 5:	sprintf(r, "CH");break;
		case 6:	sprintf(r, "DH");break;
		case 7:	sprintf(r, "BH");break;
		default:sprintf(r, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	case 16:
		switch(REG) {
		case 0: sprintf(r, "AX");break;
		case 1:	sprintf(r, "CX");break;
		case 2:	sprintf(r, "DX");break;
		case 3:	sprintf(r, "BX");break;
		case 4:	sprintf(r, "SP");break;
		case 5:	sprintf(r, "BP");break;
		case 6:	sprintf(r, "SI");break;
		case 7: sprintf(r, "DI");break;
		default:sprintf(r, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	default:sprintf(r, "<ERROR:BIT(%02X)>", regbit);break;}
}

static void ADD_RM8_R8()
{
	ip++;
	GetModRegRM(8,8);
	sprintf(output, "%04X:%04X;ADD;%s,%s;");
}

int dasm(const char *stmt, t_nubit16 seg, t_nubit16 off)
{
	t_nubit8 opcode;
	ram = mem;
	cs  = seg;
	ip  = off;
	output[0] = 0;
	rm[0] = r[0] = imm[0] = 0;
	len = 0;
	switch (opcode) {
	case 0x00:  ADD_RM8_R8(); break;
	default:                  break;
	}
	STRCPY(stmt, output);
	return len;
}
