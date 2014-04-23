#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "memory.h"

#include "../vglobal.h"
#include "../vmachine.h"

typedef char t_dasm_strs[0x20];
typedef enum {PF_NONE, PF_CS, PF_DS, PF_SS, PF_ES, PF_REPZ, PF_REPNZ} t_dasm_prefix;

static t_dasm_strs  dstmt, dbin, dop, dopr, dtip, drm, dr, dimm, dds, dss;
static t_nubit8     rid;
static t_nubit16    dods, doss;
static t_nubitcc    len;
static t_vaddrcc    dvrm, dvr;
static t_nubit16    dvcs, dvip;
static t_dasm_prefix dprefix = PF_NONE;

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
	t_nubit8 modrm = vramVarByte(dvcs, dvip++);
	t_nsbit8 sign;
	t_nubit8 disp8u;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0: sprintf(drm, "[BX+SI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si), vramVarByte(dods,vcpu.bx+vcpu.si));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si), vramVarWord(dods,vcpu.bx+vcpu.si));
			break;
		case 1: sprintf(drm, "[BX+DI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di), vramVarByte(dods,vcpu.bx+vcpu.di));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di), vramVarWord(dods,vcpu.bx+vcpu.di));
			break;
		case 2: sprintf(drm, "[BP+SI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si), vramVarByte(doss,vcpu.bp+vcpu.si));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si), vramVarWord(doss,vcpu.bp+vcpu.si));
			break;
		case 3: sprintf(drm, "[BP+DI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di), vramVarByte(doss,vcpu.bp+vcpu.di));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di), vramVarWord(doss,vcpu.bp+vcpu.di));
			break;
		case 4: sprintf(drm, "[SI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si), vramVarByte(dods,vcpu.si));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si), vramVarWord(dods,vcpu.si));
			break;
		case 5: sprintf(drm, "[DI]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di), vramVarByte(dods,vcpu.di));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di), vramVarWord(dods,vcpu.di));
			break;
		case 6: disp16 = vramVarWord(dvcs, dvip); dvip += 2;
			    sprintf(drm, "[%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(disp16), vramVarByte(dods,disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(disp16), vramVarWord(dods,disp16));
			break;
		case 7: sprintf(drm, "[BX]");
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx), vramVarByte(dods,vcpu.bx));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx), vramVarWord(dods,vcpu.bx));
			break;
		default:sprintf(drm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramVarByte(dvcs, dvip);dvip += 1;
		sign = (disp8 & 0x80) ? '-' : '+';
		disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
		switch(RM) {
		case 0: sprintf(drm, "[BX+SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp8), vramVarByte(dods,vcpu.bx+vcpu.si+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp8), vramVarWord(dods,vcpu.bx+vcpu.si+disp8));
			break;
		case 1: sprintf(drm, "[BX+DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp8), vramVarByte(dods,vcpu.bx+vcpu.di+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp8), vramVarWord(dods,vcpu.bx+vcpu.di+disp8));
			break;
		case 2: sprintf(drm, "[BP+SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp8), vramVarByte(doss,vcpu.bp+vcpu.si+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp8), vramVarWord(doss,vcpu.bp+vcpu.si+disp8));
			break;
		case 3: sprintf(drm, "[BP+DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp8), vramVarByte(doss,vcpu.bp+vcpu.di+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp8), vramVarWord(doss,vcpu.bp+vcpu.di+disp8));
			break;
		case 4: sprintf(drm, "[SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si+disp8), vramVarByte(dods,vcpu.si+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si+disp8), vramVarWord(dods,vcpu.si+disp8));
			break;
		case 5: sprintf(drm, "[DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di+disp8), vramVarByte(dods,vcpu.di+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di+disp8), vramVarWord(dods,vcpu.di+disp8));
			break;
		case 6: sprintf(drm, "[BP%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+disp8), vramVarByte(doss,vcpu.bp+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+disp8), vramVarWord(doss,vcpu.bp+disp8));
			break;
		case 7: sprintf(drm, "[BX%c%02X]", sign, disp8u);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+disp8), vramVarByte(dods,vcpu.bx+disp8));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+disp8), vramVarWord(dods,vcpu.bx+disp8));
			break;
		default:sprintf(drm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 2:
		disp16 = vramVarWord(dvcs,dvip);dvip += 2;
		switch(RM) {
		case 0: sprintf(drm, "[BX+SI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp16), vramVarByte(dods,vcpu.bx+vcpu.si+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp16), vramVarWord(dods,vcpu.bx+vcpu.si+disp16));
			break;
		case 1: sprintf(drm, "[BX+DI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp16), vramVarByte(dods,vcpu.bx+vcpu.di+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp16), vramVarWord(dods,vcpu.bx+vcpu.di+disp16));
			break;
		case 2: sprintf(drm, "[BP+SI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp16), vramVarByte(doss,vcpu.bp+vcpu.si+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp16), vramVarWord(doss,vcpu.bp+vcpu.si+disp16));
			break;
		case 3: sprintf(drm, "[BP+DI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp16), vramVarByte(doss,vcpu.bp+vcpu.di+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp16), vramVarWord(doss,vcpu.bp+vcpu.di+disp16));
			break;
		case 4: sprintf(drm, "[SI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si+disp16), vramVarByte(dods,vcpu.si+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si+disp16), vramVarWord(dods,vcpu.si+disp16));
			break;
		case 5: sprintf(drm, "[DI+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di+disp16), vramVarByte(dods,vcpu.di+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di+disp16), vramVarWord(dods,vcpu.di+disp16));
			break;
		case 6: sprintf(drm, "[BP+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+disp16), vramVarByte(doss,vcpu.bp+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+disp16), vramVarWord(doss,vcpu.bp+disp16));
			break;
		case 7: sprintf(drm, "[BX+%04X]", disp16);
			if (rmbit == 8)       sprintf(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+disp16), vramVarByte(dods,vcpu.bx+disp16));
			else if (rmbit == 16) sprintf(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+disp16), vramVarWord(dods,vcpu.bx+disp16));
			break;
		default:sprintf(drm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	case 3:
		switch(RM) {
		case 0: if(rmbit == 8) sprintf(drm, "AL"); else sprintf(drm, "AX"); break;
		case 1: if(rmbit == 8) sprintf(drm, "CL"); else sprintf(drm, "CX"); break;
		case 2: if(rmbit == 8) sprintf(drm, "DL"); else sprintf(drm, "DX"); break;
		case 3: if(rmbit == 8) sprintf(drm, "BL"); else sprintf(drm, "BX"); break;
		case 4: if(rmbit == 8) sprintf(drm, "AH"); else sprintf(drm, "SP"); break;
		case 5: if(rmbit == 8) sprintf(drm, "CH"); else sprintf(drm, "BP"); break;
		case 6: if(rmbit == 8) sprintf(drm, "DH"); else sprintf(drm, "SI"); break;
		case 7: if(rmbit == 8) sprintf(drm, "BH"); else sprintf(drm, "DI"); break;
		default:sprintf(drm, "<ERROR:MOD(%02X),RM(%02X)>", MOD, RM);break;}
		break;
	default:sprintf(drm, "<ERROR:MOD(%02X)>", MOD);break;}
	switch(regbit) {
	case 0: rid = REG;break;
	case 4:
		switch(REG) {
		case 0: sprintf(dr, "ES");break;
		case 1: sprintf(dr, "CS");break;
		case 2: sprintf(dr, "SS");break;
		case 3: sprintf(dr, "DS");break;
		default:sprintf(dr, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	case 8:
		switch(REG) {
		case 0: sprintf(dr, "AL");break;
		case 1: sprintf(dr, "CL");break;
		case 2: sprintf(dr, "DL");break;
		case 3: sprintf(dr, "BL");break;
		case 4: sprintf(dr, "AH");break;
		case 5: sprintf(dr, "CH");break;
		case 6: sprintf(dr, "DH");break;
		case 7: sprintf(dr, "BH");break;
		default:sprintf(dr, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	case 16:
		switch(REG) {
		case 0: sprintf(dr, "AX");break;
		case 1: sprintf(dr, "CX");break;
		case 2: sprintf(dr, "DX");break;
		case 3: sprintf(dr, "BX");break;
		case 4: sprintf(dr, "SP");break;
		case 5: sprintf(dr, "BP");break;
		case 6: sprintf(dr, "SI");break;
		case 7: sprintf(dr, "DI");break;
		default:sprintf(dr, "<ERROR:BIT(%02X),REG(%02X)>", regbit, REG);break;}
		break;
	default:sprintf(dr, "<ERROR:BIT(%02X)>", regbit);break;}
}

static void ADD_RM8_R8()
{
	dvip++;
	GetModRegRM(8,8);
	sprintf(dop, "ADD");
	sprintf(dopr, "%s,%s", drm, dr);
}
static void ES()
{
	dvip++;
	sprintf(dop, "ES:");
	sprintf(dds, "ES");
	sprintf(dss, "ES");
	dods = vcpu.es;
	doss = vcpu.es;
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
static void ClrPrefix()
{
	sprintf(dds, "DS");
	sprintf(dss, "SS");
	dods = vcpu.ds;
	doss = vcpu.ss;
}

t_nubitcc dasm(t_string stmt, t_nubit16 seg, t_nubit16 off)
{
	t_nubitcc i,l;
	t_nubit8 opcode;
	dvcs  = seg;
	dvip  = off;
	stmt[0] = 0;
	ClrPrefix();
	do {
		dop[0]  = 0;
		dopr[0] = 0;
		dbin[0] = 0;
		dtip[0] = 0;
		opcode = vramVarByte(dvcs, dvip);
		switch (opcode) {
		case 0x00:  ADD_RM8_R8(); break;
		case 0x26:  ES();         break;
		default:                  break;
		}
		l = (dvip - off);
		len += l;
		for (i = 0;i < l;++i) sprintf(dbin, "%s%02X", dbin, vramVarByte(seg, off+i));
		if (dtip[0])
			sprintf(dstmt, "%04X:%04X\t%s\t%s\t%s\t;%s\n", seg, off, dbin, dop, dopr, dtip);
		else
			sprintf(dstmt, "%04X:%04X\t%s\t%s\t%s\n", seg, off, dbin, dop, dopr);
		off = dvip;
		STRCAT(stmt, dstmt);
	} while (IsPrefix(opcode));
	return len;
}
