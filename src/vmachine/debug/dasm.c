/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "../vapi.h"
#include "../vmachine.h"
#include "debug.h"
#include "dasm.h"

typedef char t_dasm_strl[0x0200];
typedef char t_dasm_strs[0x0020];

static t_dasm_strl  dstmt;
static t_dasm_strs  dbin, dop, dopr, dtip, drm, dr, dimm, dds, dss, dimmoff8, dimmoff16, dimmsign;
static t_bool       ismem;
static t_nubit8     rid;
static t_nubit16    dods, doss;
static t_vaddrcc    dvrm, dvr;
static t_nubit32    diptr; /* global physical address */

#define _GetModRM_MOD(modrm) ((modrm&0xc0)>>6)
#define _GetModRM_REG(modrm) ((modrm&0x38)>>3)
#define _GetModRM_RM(modrm)  ((modrm&0x07)>>0)

static void GetMem(t_nubitcc membit)
{
	switch (membit) {
	case 8: SPRINTF(drm, "[%04X]", vramWord(diptr));
		    SPRINTF(dtip, "%s:%04X=%02X", dds, vramWord(diptr), vramRealByte(dods,vramWord(diptr)));
			break;
	case 16:SPRINTF(drm, "[%04X]", vramWord(diptr));
		    SPRINTF(dtip, "%s:%04X=%04X", dds, vramWord(diptr), vramRealWord(dods,vramWord(diptr)));
			break;
	default:SPRINTF(drm, "<ERROR:MEMBIT(%02X)>",membit);break;}
	diptr += 2;
}
static void GetImm(t_nubitcc immbit)
{
	t_nsbit8 imm8 = (t_nsbit8)vramByte(diptr);
	t_nsbit16 imm16 = (t_nsbit16)vramWord(diptr);
	t_nsbit8 sign8 = (imm8 & 0x80) ? '-' : '+';
	t_nubit8 immu8 = (imm8 & 0x80) ? ((~imm8) + 0x01) : imm8;
	dimmoff8[0] = 0;
	dimmoff16[0] = 0;
	switch(immbit) {
	case 8:  SPRINTF(dimm, "%02X", vramByte(diptr)); diptr += 1;
		     SPRINTF(dimmoff8,  "%04X", (t_nubit16)(diptr+imm8));
		     SPRINTF(dimmsign,  "%c%02X", sign8, immu8);              break;
	case 16: SPRINTF(dimm, "%04X", vramWord(diptr)); diptr += 2;
		     SPRINTF(dimmoff16, "%04X", (t_nubit16)(diptr+imm16));                break;
	case 32: SPRINTF(dimm, "%08X", vramDWord(diptr));diptr += 4;break;
	default:SPRINTF(dimm, "<ERROR:IMMBIT(%02X)>",immbit);break;}
}
static void GetModRegRM(t_nubit8 regbit,t_nubit8 rmbit)
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramByte(diptr++);
	t_nsbit8 sign;
	t_nubit8 disp8u;
	ismem = 0x01;
	switch(_GetModRM_MOD(modrm)) {
	case 0:
		switch(_GetModRM_RM(modrm)) {
		case 0: SPRINTF(drm, "[BX+SI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si), vramRealByte(dods,vcpu.bx+vcpu.si));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si), vramRealWord(dods,vcpu.bx+vcpu.si));
			break;
		case 1: SPRINTF(drm, "[BX+DI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di), vramRealByte(dods,vcpu.bx+vcpu.di));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di), vramRealWord(dods,vcpu.bx+vcpu.di));
			break;
		case 2: SPRINTF(drm, "[BP+SI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si), vramRealByte(doss,vcpu.bp+vcpu.si));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si), vramRealWord(doss,vcpu.bp+vcpu.si));
			break;
		case 3: SPRINTF(drm, "[BP+DI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di), vramRealByte(doss,vcpu.bp+vcpu.di));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di), vramRealWord(doss,vcpu.bp+vcpu.di));
			break;
		case 4: SPRINTF(drm, "[SI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si), vramRealByte(dods,vcpu.si));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si), vramRealWord(dods,vcpu.si));
			break;
		case 5: SPRINTF(drm, "[DI]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di), vramRealByte(dods,vcpu.di));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di), vramRealWord(dods,vcpu.di));
			break;
		case 6: disp16 = vramWord(diptr); diptr += 2;
			    SPRINTF(drm, "[%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(disp16), vramRealByte(dods,disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(disp16), vramRealWord(dods,disp16));
			break;
		case 7: SPRINTF(drm, "[BX]");
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx), vramRealByte(dods,vcpu.bx));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx), vramRealWord(dods,vcpu.bx));
			break;
		default:SPRINTF(drm, "<ERROR:ModRM_MOD(%02X),ModRM_RM(%02X)>", _GetModRM_MOD(modrm), _GetModRM_RM(modrm));break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramByte(diptr);diptr += 1;
		sign = (disp8 & 0x80) ? '-' : '+';
		disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
		switch(_GetModRM_RM(modrm)) {
		case 0: SPRINTF(drm, "[BX+SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp8), vramRealByte(dods,vcpu.bx+vcpu.si+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp8), vramRealWord(dods,vcpu.bx+vcpu.si+disp8));
			break;
		case 1: SPRINTF(drm, "[BX+DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp8), vramRealByte(dods,vcpu.bx+vcpu.di+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp8), vramRealWord(dods,vcpu.bx+vcpu.di+disp8));
			break;
		case 2: SPRINTF(drm, "[BP+SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp8), vramRealByte(doss,vcpu.bp+vcpu.si+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp8), vramRealWord(doss,vcpu.bp+vcpu.si+disp8));
			break;
		case 3: SPRINTF(drm, "[BP+DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp8), vramRealByte(doss,vcpu.bp+vcpu.di+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp8), vramRealWord(doss,vcpu.bp+vcpu.di+disp8));
			break;
		case 4: SPRINTF(drm, "[SI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si+disp8), vramRealByte(dods,vcpu.si+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si+disp8), vramRealWord(dods,vcpu.si+disp8));
			break;
		case 5: SPRINTF(drm, "[DI%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di+disp8), vramRealByte(dods,vcpu.di+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di+disp8), vramRealWord(dods,vcpu.di+disp8));
			break;
		case 6: SPRINTF(drm, "[BP%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+disp8), vramRealByte(doss,vcpu.bp+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+disp8), vramRealWord(doss,vcpu.bp+disp8));
			break;
		case 7: SPRINTF(drm, "[BX%c%02X]", sign, disp8u);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+disp8), vramRealByte(dods,vcpu.bx+disp8));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+disp8), vramRealWord(dods,vcpu.bx+disp8));
			break;
		default:SPRINTF(drm, "<ERROR:ModRM_MOD(%02X),ModRM_RM(%02X)>", _GetModRM_MOD(modrm), _GetModRM_RM(modrm));break;}
		break;
	case 2:
		disp16 = vramWord(diptr);diptr += 2;
		switch(_GetModRM_RM(modrm)) {
		case 0: SPRINTF(drm, "[BX+SI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp16), vramRealByte(dods,vcpu.bx+vcpu.si+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.si+disp16), vramRealWord(dods,vcpu.bx+vcpu.si+disp16));
			break;
		case 1: SPRINTF(drm, "[BX+DI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp16), vramRealByte(dods,vcpu.bx+vcpu.di+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+vcpu.di+disp16), vramRealWord(dods,vcpu.bx+vcpu.di+disp16));
			break;
		case 2: SPRINTF(drm, "[BP+SI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp16), vramRealByte(doss,vcpu.bp+vcpu.si+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.si+disp16), vramRealWord(doss,vcpu.bp+vcpu.si+disp16));
			break;
		case 3: SPRINTF(drm, "[BP+DI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp16), vramRealByte(doss,vcpu.bp+vcpu.di+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+vcpu.di+disp16), vramRealWord(doss,vcpu.bp+vcpu.di+disp16));
			break;
		case 4: SPRINTF(drm, "[SI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.si+disp16), vramRealByte(dods,vcpu.si+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.si+disp16), vramRealWord(dods,vcpu.si+disp16));
			break;
		case 5: SPRINTF(drm, "[DI+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.di+disp16), vramRealByte(dods,vcpu.di+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.di+disp16), vramRealWord(dods,vcpu.di+disp16));
			break;
		case 6: SPRINTF(drm, "[BP+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dss, (t_nubit16)(vcpu.bp+disp16), vramRealByte(doss,vcpu.bp+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dss, (t_nubit16)(vcpu.bp+disp16), vramRealWord(doss,vcpu.bp+disp16));
			break;
		case 7: SPRINTF(drm, "[BX+%04X]", disp16);
			if (rmbit == 8)       SPRINTF(dtip, "%s:%04X=%02X", dds, (t_nubit16)(vcpu.bx+disp16), vramRealByte(dods,vcpu.bx+disp16));
			else if (rmbit == 16) SPRINTF(dtip, "%s:%04X=%04X", dds, (t_nubit16)(vcpu.bx+disp16), vramRealWord(dods,vcpu.bx+disp16));
			break;
		default:SPRINTF(drm, "<ERROR:ModRM_MOD(%02X),ModRM_RM(%02X)>", _GetModRM_MOD(modrm), _GetModRM_RM(modrm));break;}
		break;
	case 3:
		ismem = 0x00;
		switch(_GetModRM_RM(modrm)) {
		case 0: if(rmbit == 8) SPRINTF(drm, "AL"); else SPRINTF(drm, "AX");break;
		case 1: if(rmbit == 8) SPRINTF(drm, "CL"); else SPRINTF(drm, "CX");break;
		case 2: if(rmbit == 8) SPRINTF(drm, "DL"); else SPRINTF(drm, "DX");break;
		case 3: if(rmbit == 8) SPRINTF(drm, "BL"); else SPRINTF(drm, "BX");break;
		case 4: if(rmbit == 8) SPRINTF(drm, "AH"); else SPRINTF(drm, "SP");break;
		case 5: if(rmbit == 8) SPRINTF(drm, "CH"); else SPRINTF(drm, "BP");break;
		case 6: if(rmbit == 8) SPRINTF(drm, "DH"); else SPRINTF(drm, "SI");break;
		case 7: if(rmbit == 8) SPRINTF(drm, "BH"); else SPRINTF(drm, "DI");break;
		default:SPRINTF(drm, "<ERROR:ModRM_MOD(%02X),ModRM_RM(%02X)>", _GetModRM_MOD(modrm), _GetModRM_RM(modrm));break;}
		break;
	default:SPRINTF(drm, "<ERROR:ModRM_MOD(modrm)(%02X)>", _GetModRM_MOD(modrm));break;}
	switch(regbit) {
	case 0: rid = _GetModRM_REG(modrm);break;
	case 4:
		switch(_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "ES");break;
		case 1: SPRINTF(dr, "CS");break;
		case 2: SPRINTF(dr, "SS");break;
		case 3: SPRINTF(dr, "DS");break;
		default:SPRINTF(dr, "<ERROR:REGBIT(%02X),ModRM_REG(%02X)>", regbit, _GetModRM_REG(modrm));break;}
		break;
	case 8:
		switch(_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "AL");break;
		case 1: SPRINTF(dr, "CL");break;
		case 2: SPRINTF(dr, "DL");break;
		case 3: SPRINTF(dr, "BL");break;
		case 4: SPRINTF(dr, "AH");break;
		case 5: SPRINTF(dr, "CH");break;
		case 6: SPRINTF(dr, "DH");break;
		case 7: SPRINTF(dr, "BH");break;
		default:SPRINTF(dr, "<ERROR:REGBIT(%02X),ModRM_REG(%02X)>", regbit, _GetModRM_REG(modrm));break;}
		break;
	case 16:
		switch(_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "AX");break;
		case 1: SPRINTF(dr, "CX");break;
		case 2: SPRINTF(dr, "DX");break;
		case 3: SPRINTF(dr, "BX");break;
		case 4: SPRINTF(dr, "SP");break;
		case 5: SPRINTF(dr, "BP");break;
		case 6: SPRINTF(dr, "SI");break;
		case 7: SPRINTF(dr, "DI");break;
		default:SPRINTF(dr, "<ERROR:REGBIT(%02X),ModRM_REG(%02X)>", regbit, _GetModRM_REG(modrm));break;}
		break;
	default:SPRINTF(dr, "<ERROR:REGBIT(%02X)>", regbit);break;}
}

static void UndefinedOpcode()
{
	t_nubit8 opcode = vramByte(diptr);
	diptr++;
	SPRINTF(dop, "DB");
	SPRINTF(dopr, "%02X", opcode);
}
static void ADD_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void ADD_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void ADD_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void ADD_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void ADD_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void ADD_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void PUSH_ES()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "ES");
}
static void POP_ES()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "ES");
}
static void OR_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void OR_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void OR_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void OR_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void OR_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void OR_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void PUSH_CS()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "CS");
}
static void POP_CS()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "CS");
}
static void ADC_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void ADC_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void ADC_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void ADC_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void ADC_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void ADC_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void PUSH_SS()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "SS");
}
static void POP_SS()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "SS");
}
static void SBB_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void SBB_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void SBB_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void SBB_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void SBB_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void SBB_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void PUSH_DS()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "DS");
}
static void POP_DS()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "DS");
}
static void AND_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void AND_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void AND_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void AND_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void AND_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void AND_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void ES()
{
	diptr++;
	SPRINTF(dop, "ES:");
	SPRINTF(dds, "ES");
	SPRINTF(dss, "ES");
	dods = _es;
	doss = _es;
}
static void DAA()
{
	diptr++;
	SPRINTF(dop, "DAA");
}
static void SUB_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void SUB_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void SUB_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void SUB_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void SUB_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void SUB_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void CS()
{
	diptr++;
	SPRINTF(dop, "CS:");
	SPRINTF(dds, "CS");
	SPRINTF(dss, "CS");
	dods = _cs;
	doss = _cs;
}
static void DAS()
{
	diptr++;
	SPRINTF(dop, "DAS");
}
static void XOR_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void XOR_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void XOR_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void XOR_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void XOR_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void XOR_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void SS()
{
	diptr++;
	SPRINTF(dop, "SS:");
	SPRINTF(dds, "SS");
	SPRINTF(dss, "SS");
	dods = _ss;
	doss = _ss;
}
static void AAA()
{
	diptr++;
	SPRINTF(dop, "AAA");
}
static void CMP_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void CMP_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void CMP_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void CMP_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void CMP_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void CMP_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void DS()
{
	diptr++;
	SPRINTF(dop, "DS:");
	SPRINTF(dds, "DS");
	SPRINTF(dss, "DS");
	dods = _ds;
	doss = _ds;
}
static void AAS()
{
	diptr++;
	SPRINTF(dop, "AAS");
}
static void INC_AX()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "AX");
}
static void INC_CX()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "CX");
}
static void INC_DX()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "DX");
}
static void INC_BX()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "BX");
}
static void INC_SP()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "SP");
}
static void INC_BP()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "BP");
}
static void INC_SI()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "SI");
}
static void INC_DI()
{
	diptr++;
	SPRINTF(dop, "INC");
	SPRINTF(dopr, "DI");
}
static void DEC_AX()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "AX");
}
static void DEC_CX()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "CX");
}
static void DEC_DX()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "DX");
}
static void DEC_BX()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "BX");
}
static void DEC_SP()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "SP");
}
static void DEC_BP()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "BP");
}
static void DEC_SI()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "SI");
}
static void DEC_DI()
{
	diptr++;
	SPRINTF(dop, "DEC");
	SPRINTF(dopr, "DI");
}
static void PUSH_AX()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "AX");
}
static void PUSH_CX()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "CX");
}
static void PUSH_DX()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "DX");
}
static void PUSH_BX()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "BX");
}
static void PUSH_SP()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "SP");
}
static void PUSH_BP()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "BP");
}
static void PUSH_SI()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "SI");
}
static void PUSH_DI()
{
	diptr++;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "DI");
}
static void POP_AX()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "AX");
}
static void POP_CX()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "CX");
}
static void POP_DX()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "DX");
}
static void POP_BX()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "BX");
}
static void POP_SP()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "SP");
}
static void POP_BP()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "BP");
}
static void POP_SI()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "SI");
}
static void POP_DI()
{
	diptr++;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "DI");
}
static void PREFIX_OprSize()
{
	diptr++;
	SPRINTF(dop, "OPR+");
}
static void PREFIX_AddrSize()
{
	diptr++;
	SPRINTF(dop, "ADR+");
}
static void JO()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JO");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNO()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNO");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JC()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JC");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNC()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNC");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JZ()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JZ");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNZ()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNZ");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JBE()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JBE");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JA()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JA");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JS()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JS");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNS()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNS");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JP()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JP");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNP()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNP");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JL()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JL");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JNL()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JNL");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JLE()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JLE");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JG()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JG");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void INS_80()
{
	diptr++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(rid) {
	case 0: SPRINTF(dop, "ADD");break;
	case 1: SPRINTF(dop, "OR");break;
	case 2: SPRINTF(dop, "ADC");break;
	case 3: SPRINTF(dop, "SBB");break;
	case 4: SPRINTF(dop, "AND");break;
	case 5: SPRINTF(dop, "SUB");break;
	case 6: SPRINTF(dop, "XOR");break;
	case 7: SPRINTF(dop, "CMP");break;
	default: SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);return;}
	if (ismem) SPRINTF(dopr, "BYTE PTR %s,%s", drm, dimm);
	else       SPRINTF(dopr, "%s,%s", drm, dimm);
}
static void INS_81()
{
	diptr++;
	GetModRegRM(0,16);
	GetImm(16);
	switch(rid) {
	case 0: SPRINTF(dop, "ADD");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 1: SPRINTF(dop, "OR");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 2: SPRINTF(dop, "ADC");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 3: SPRINTF(dop, "SBB");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 4: SPRINTF(dop, "AND");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 5: SPRINTF(dop, "SUB");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 6: SPRINTF(dop, "XOR");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	case 7: SPRINTF(dop, "CMP");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		break;
	default:
		SPRINTF(dop, "INS_81");
		SPRINTF(dopr, "<ERROR:REGID(%02X)>", rid);
		break;
	}
}
static void INS_82()
{
	INS_80();
}
static void INS_83()
{
	diptr++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(rid) {
	case 0: SPRINTF(dop, "ADD");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 1: SPRINTF(dop, "OR");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 2: SPRINTF(dop, "ADC");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 3: SPRINTF(dop, "SBB");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 4: SPRINTF(dop, "AND");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 5: SPRINTF(dop, "SUB");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 6: SPRINTF(dop, "XOR");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	case 7: SPRINTF(dop, "CMP");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimmsign);
		else       SPRINTF(dopr, "%s,%s", drm, dimmsign);
		break;
	default:
		SPRINTF(dop, "INS_81");
		SPRINTF(dopr, "<ERROR:REGID(%02X)>", rid);
		break;
	}
}
static void TEST_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "TEST");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void TEST_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "TEST");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void XCHG_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "%s,%s",dr,drm);
}
static void XCHG_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "%s,%s",dr,drm);
}
static void MOV_RM8_R8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void MOV_RM16_R16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void MOV_R8_RM8()
{
	diptr++;
	GetModRegRM(8,8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void MOV_R16_RM16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void MOV_RM16_SREG()
{
	diptr++;
	GetModRegRM(4,16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", drm, dr);
}
static void LEA_R16_M16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "LEA");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void MOV_SREG_RM16()
{
	diptr++;
	GetModRegRM(4,16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void POP_RM16()
{
	diptr++;
	GetModRegRM(0,16);
	SPRINTF(dop, "POP");
	switch(rid) {
	case 0: SPRINTF(dopr, "%s", drm);break;
	default:SPRINTF(dopr, "<ERROR:REGID(%02X)>", rid);break;}
}
static void NOP()
{
	diptr++;
	SPRINTF(dop, "NOP");
}
static void XCHG_CX_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "CX,AX");
}
static void XCHG_DX_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "DX,AX");
}
static void XCHG_BX_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "BX,AX");
}
static void XCHG_SP_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "SP,AX");
}
static void XCHG_BP_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "BP,AX");
}
static void XCHG_SI_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "SI,AX");
}
static void XCHG_DI_AX()
{
	diptr++;
	SPRINTF(dop, "XCHG");
	SPRINTF(dopr, "DI,AX");
}
static void CBW()
{
	diptr++;
	SPRINTF(dop, "CBW");
}
static void CWD()
{
	diptr++;
	SPRINTF(dop, "CWD");
}
static void CALL_PTR16_16()
{
	diptr++;
	SPRINTF(dop, "CALL");
	GetImm(16);
	SPRINTF(dopr, ":%s", dimm);
	GetImm(16);
	STRCAT(dimm, dopr);
	STRCPY(dopr, dimm);
}
static void WAIT()
{
	diptr++;
	SPRINTF(dop, "WAIT");
}
static void PUSHF()
{
	diptr++;
	SPRINTF(dop, "PUSHF");
}
static void POPF()
{
	diptr++;
	SPRINTF(dop, "POPF");
}
static void SAHF()
{
	diptr++;
	SPRINTF(dop, "SAHF");
}
static void LAHF()
{
	diptr++;
	SPRINTF(dop, "LAHF");
}
static void MOV_AL_M8()
{
	diptr++;
	GetMem(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "AL,%s", drm);
}
static void MOV_AX_M16()
{
	diptr++;
	GetMem(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "AX,%s", drm);
}
static void MOV_M8_AL()
{
	diptr++;
	GetMem(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,AL", drm);
}
static void MOV_M16_AX()
{
	diptr++;
	GetMem(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "%s,AX", drm);
}
static void MOVSB()
{
	diptr++;
	SPRINTF(dop, "MOVSB");
}
static void MOVSW()
{
	diptr++;
	SPRINTF(dop, "MOVSW");
}
static void CMPSB()
{
	diptr++;
	SPRINTF(dop, "CMPSB");
}
static void CMPSW()
{
	diptr++;
	SPRINTF(dop, "CMPSW");
}
static void TEST_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "TEST");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void TEST_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "TEST");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void STOSB()
{
	diptr++;
	SPRINTF(dop, "STOSB");
}
static void STOSW()
{
	diptr++;
	SPRINTF(dop, "STOSW");
}
static void LODSB()
{
	diptr++;
	SPRINTF(dop, "LODSB");
}
static void LODSW()
{
	diptr++;
	SPRINTF(dop, "LODSW");
}
static void SCASB()
{
	diptr++;
	SPRINTF(dop, "SCASB");
}
static void SCASW()
{
	diptr++;
	SPRINTF(dop, "SCASW");
}
static void MOV_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void MOV_CL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "CL,%s", dimm);
}
static void MOV_DL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "DL,%s", dimm);
}
static void MOV_BL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "BL,%s", dimm);
}
static void MOV_AH_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "AH,%s", dimm);
}
static void MOV_CH_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "CH,%s", dimm);
}
static void MOV_DH_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "DH,%s", dimm);
}
static void MOV_BH_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "BH,%s", dimm);
}
static void MOV_AX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void MOV_CX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "CX,%s", dimm);
}
static void MOV_DX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "DX,%s", dimm);
}
static void MOV_BX_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "BX,%s", dimm);
}
static void MOV_SP_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "SP,%s", dimm);
}
static void MOV_BP_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "BP,%s", dimm);
}
static void MOV_SI_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "SI,%s", dimm);
}
static void MOV_DI_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "DI,%s", dimm);
}
static void RET_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "RET");
	SPRINTF(dopr, "%s", dimm);
}
static void RET()
{
	diptr++;
	SPRINTF(dop, "RET");
}
static void LES_R16_M16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "LES");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void LDS_R16_M16()
{
	diptr++;
	GetModRegRM(16,16);
	SPRINTF(dop, "LDS");
	SPRINTF(dopr, "%s,%s", dr, drm);
}
static void MOV_M8_I8()
{
	diptr++;
	GetModRegRM(8,8);
	GetImm(8);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "BYTE PTR %s,%s", drm, dimm);
}
static void MOV_M16_I16()
{
	diptr++;
	GetModRegRM(16,16);
	GetImm(16);
	SPRINTF(dop, "MOV");
	SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
}
static void RETF_I16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "RETF");
	SPRINTF(dopr, "%s", dimm);
}
static void RETF()
{
	diptr++;
	SPRINTF(dop, "RETF");
}
static void INT3()
{
	diptr++;
	SPRINTF(dop, "INT");
	SPRINTF(dopr, "03");
}
static void INT_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "INT");
	SPRINTF(dopr, "%s", dimm);
}
static void INTO()
{
	diptr++;
	SPRINTF(dop, "INT");
	SPRINTF(dopr, "04");
}
static void IRET()
{
	diptr++;
	SPRINTF(dop, "IRET");
}
static void INS_D0()
{
	diptr++;
	GetModRegRM(0,8);
	switch(rid) {
	case 0: SPRINTF(dop, "ROL");break;
	case 1: SPRINTF(dop, "ROR");break;
	case 2: SPRINTF(dop, "RCL");break;
	case 3: SPRINTF(dop, "RCR");break;
	case 4: SPRINTF(dop, "SHL");break;
	case 5: SPRINTF(dop, "SHR");break;
	case 6: SPRINTF(dop, "SAL");break;
	case 7: SPRINTF(dop, "SAR");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);return;}
	if (ismem) SPRINTF(dopr, "BYTE PTR %s,1", drm);
	else       SPRINTF(dopr, "%s,1", drm);
}
static void INS_D1()
{
	diptr++;
	GetModRegRM(0,16);
	switch(rid) {
	case 0: SPRINTF(dop, "ROL");break;
	case 1: SPRINTF(dop, "ROR");break;
	case 2: SPRINTF(dop, "RCL");break;
	case 3: SPRINTF(dop, "RCR");break;
	case 4: SPRINTF(dop, "SHL");break;
	case 5: SPRINTF(dop, "SHR");break;
	case 6: SPRINTF(dop, "SAL");break;
	case 7: SPRINTF(dop, "SAR");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);return;}
	if (ismem) SPRINTF(dopr, "WORD PTR %s,1", drm);
	else       SPRINTF(dopr, "%s,1", drm);
}
static void INS_D2()
{
	diptr++;
	GetModRegRM(0,8);
	switch(rid) {
	case 0: SPRINTF(dop, "ROL");break;
	case 1: SPRINTF(dop, "ROR");break;
	case 2: SPRINTF(dop, "RCL");break;
	case 3: SPRINTF(dop, "RCR");break;
	case 4: SPRINTF(dop, "SHL");break;
	case 5: SPRINTF(dop, "SHR");break;
	case 6: SPRINTF(dop, "SAL");break;
	case 7: SPRINTF(dop, "SAR");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);return;}
	if (ismem) SPRINTF(dopr, "BYTE PTR %s,CL", drm);
	else       SPRINTF(dopr, "%s,CL", drm);
}
static void INS_D3()
{
	diptr++;
	GetModRegRM(0,16);
	switch(rid) {
	case 0: SPRINTF(dop, "ROL");break;
	case 1: SPRINTF(dop, "ROR");break;
	case 2: SPRINTF(dop, "RCL");break;
	case 3: SPRINTF(dop, "RCR");break;
	case 4: SPRINTF(dop, "SHL");break;
	case 5: SPRINTF(dop, "SHR");break;
	case 6: SPRINTF(dop, "SAL");break;
	case 7: SPRINTF(dop, "SAR");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);return;}
	if (ismem) SPRINTF(dopr, "WORD PTR %s,CL", drm);
	else       SPRINTF(dopr, "%s,CL", drm);
}
static void AAM()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "AAM");
	if (strcmp(dimm, "0A")) SPRINTF(dopr, "%s", dimm);
}
static void AAD()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "AAD");
	if (strcmp(dimm, "0A")) SPRINTF(dopr, "%s", dimm);
}
static void XLAT()
{
	diptr++;
	SPRINTF(dop, "XLAT");
}
static void LOOPNZ()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "LOOPNZ");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void LOOPZ()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "LOOPZ");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void LOOP()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "LOOP");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void JCXZ_REL8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JCXZ");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void IN_AL_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "IN");
	SPRINTF(dopr, "AL,%s", dimm);
}
static void IN_AX_I8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "IN");
	SPRINTF(dopr, "AX,%s", dimm);
}
static void OUT_I8_AL()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "OUT");
	SPRINTF(dopr, "%s,AL", dimm);
}
static void OUT_I8_AX()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "OUT");
	SPRINTF(dopr, "%s,AX", dimm);
}
static void CALL_REL16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "CALL");
	SPRINTF(dopr, "%s", dimmoff16);
}
static void JMP_REL16()
{
	diptr++;
	GetImm(16);
	SPRINTF(dop, "JMP");
	SPRINTF(dopr, "%s", dimmoff16);
}
static void JMP_PTR16_16()
{
	diptr++;
	SPRINTF(dop, "JMP");
	GetImm(16);
	SPRINTF(dopr, ":%s", dimm);
	GetImm(16);
	STRCAT(dimm, dopr);
	STRCPY(dopr, dimm);
}
static void JMP_REL8()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "JMP");
	SPRINTF(dopr, "%s", dimmoff8);
}
static void IN_AL_DX()
{
	diptr++;
	SPRINTF(dop, "IN");
	SPRINTF(dopr, "AL,DX");
}
static void IN_AX_DX()
{
	diptr++;
	SPRINTF(dop, "IN");
	SPRINTF(dopr, "AX,DX");
}
static void OUT_DX_AL()
{
	diptr++;
	SPRINTF(dop, "OUT");
	SPRINTF(dopr, "DX,AL");
}
static void OUT_DX_AX()
{
	diptr++;
	SPRINTF(dop, "OUT");
	SPRINTF(dopr, "DX,AX");
}
static void LOCK()
{
	diptr++;
	SPRINTF(dop, "LOCK");
}
static void QDX()
{
	diptr++;
	GetImm(8);
	SPRINTF(dop, "QDX");
	SPRINTF(dopr, "%s", dimm);
}
static void REPNZ()
{
	diptr++;
	SPRINTF(dop, "REPNZ");
}
static void REP()
{
	diptr++;
	SPRINTF(dop, "REP");
}
static void HLT()
{
	diptr++;
	SPRINTF(dop, "HLT");
}
static void CMC()
{
	diptr++;
	SPRINTF(dop, "CMC");
}
static void INS_F6()
{
	diptr++;
	GetModRegRM(0,8);
	switch(rid) {
	case 0:	GetImm(8);
		SPRINTF(dop, "TEST");
		if (ismem) SPRINTF(dopr, "BYTE PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		return;
	case 2: SPRINTF(dop, "NOT");break;
	case 3:	SPRINTF(dop, "NEG");break;
	case 4:	SPRINTF(dop, "MUL");break;
	case 5:	SPRINTF(dop, "IMUL");break;
	case 6:	SPRINTF(dop, "DIV");break;
	case 7:	SPRINTF(dop, "IDIV");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>",rid);return;}
	if (ismem) SPRINTF(dopr, "BYTE PTR %s", drm);
	else       SPRINTF(dopr, "%s", drm);
}
static void INS_F7()
{
	diptr++;
	GetModRegRM(0,16);
	switch(rid) {
	case 0:	GetImm(16);
		SPRINTF(dop, "TEST");
		if (ismem) SPRINTF(dopr, "WORD PTR %s,%s", drm, dimm);
		else       SPRINTF(dopr, "%s,%s", drm, dimm);
		return;
	case 2: SPRINTF(dop, "NOT");break;
	case 3:	SPRINTF(dop, "NEG");break;
	case 4:	SPRINTF(dop, "MUL");break;
	case 5:	SPRINTF(dop, "IMUL");break;
	case 6:	SPRINTF(dop, "DIV");break;
	case 7:	SPRINTF(dop, "IDIV");break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>",rid);return;}
	if (ismem) SPRINTF(dopr, "WORD PTR %s", drm);
	else       SPRINTF(dopr, "%s", drm);
}
static void CLC()
{
	diptr++;
	SPRINTF(dop, "CLC");
}
static void STC()
{
	diptr++;
	SPRINTF(dop, "STC");
}
static void CLI()
{
	diptr++;
	SPRINTF(dop, "CLI");
}
static void STI()
{
	diptr++;
	SPRINTF(dop, "STI");
}
static void CLD()
{
	diptr++;
	SPRINTF(dop, "CLD");
}
static void STD()
{
	diptr++;
	SPRINTF(dop, "STD");
}
static void INS_FE()
{
	diptr++;
	GetModRegRM(0,8);
	switch(rid) {
	case 0:	SPRINTF(dop, "INC");
		if (ismem) SPRINTF(dopr, "BYTE PTR %s", drm);
		else       SPRINTF(dopr, "%s", drm);
		break;
	case 1:	SPRINTF(dop, "DEC");
		if (ismem) SPRINTF(dopr, "BYTE PTR %s", drm);
		else       SPRINTF(dopr, "%s", drm);
		break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);break;}
}
static void INS_FF()
{
	diptr++;
	GetModRegRM(0,16);
	switch(rid) {
	case 0:	SPRINTF(dop, "INC");
		if (ismem) SPRINTF(dopr, "WORD PTR %s", drm);
		else       SPRINTF(dopr, "%s", drm);
		break;
	case 1:	SPRINTF(dop, "DEC");
		if (ismem) SPRINTF(dopr, "WORD PTR %s", drm);
		else       SPRINTF(dopr, "%s", drm);
		break;
	case 2:	SPRINTF(dop, "CALL");
		SPRINTF(dopr, "%s", drm);
		break;
	case 3:	SPRINTF(dop, "CALL");
		SPRINTF(dopr, "FAR %s", drm);
		break;
	case 4:	SPRINTF(dop, "JMP"); /* JMP_RM16 */
		SPRINTF(dopr, "%s", drm);
		break;
	case 5: SPRINTF(dop, "JMP"); /* JMP_M16_16 */
		SPRINTF(dopr, "FAR %s", drm);
		break;
	case 6: SPRINTF(dop, "PUSH"); /* PUSH_RM16 */
		SPRINTF(dopr, "%s", drm);
		break;
	default:SPRINTF(dop, "<ERROR:REGID(%02X)>", rid);break;}
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65:
	case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
static void ClrPrefix()
{
	SPRINTF(dds, "DS");
	SPRINTF(dss, "SS");
	dods = _ds;
	doss = _ss;
}

static void exec(t_nubit8 opcode)
{
	switch (opcode) {
	case 0x00: ADD_RM8_R8();   break;
	case 0x01: ADD_RM16_R16(); break;
	case 0x02: ADD_R8_RM8();   break;
	case 0x03: ADD_R16_RM16(); break;
	case 0x04: ADD_AL_I8();    break;
	case 0x05: ADD_AX_I16();   break;
	case 0x06: PUSH_ES();      break;
	case 0x07: POP_ES();       break;
	case 0x08: OR_RM8_R8();    break;
	case 0x09: OR_RM16_R16();  break;
	case 0x0a: OR_R8_RM8();    break;
	case 0x0b: OR_R16_RM16();  break;
	case 0x0c: OR_AL_I8();     break;
	case 0x0d: OR_AX_I16();    break;
	case 0x0e: PUSH_CS();      break;
	case 0x0f: POP_CS();       break; //case 0x0f: INS_0F();     break;
	case 0x10: ADC_RM8_R8();   break;
	case 0x11: ADC_RM16_R16(); break;
	case 0x12: ADC_R8_RM8();   break;
	case 0x13: ADC_R16_RM16(); break;
	case 0x14: ADC_AL_I8();    break;
	case 0x15: ADC_AX_I16();   break;
	case 0x16: PUSH_SS();      break;
	case 0x17: POP_SS();       break;
	case 0x18: SBB_RM8_R8();   break;
	case 0x19: SBB_RM16_R16(); break;
	case 0x1a: SBB_R8_RM8();   break;
	case 0x1b: SBB_R16_RM16(); break;
	case 0x1c: SBB_AL_I8();    break;
	case 0x1d: SBB_AX_I16();   break;
	case 0x1e: PUSH_DS();      break;
	case 0x1f: POP_DS();       break;
	case 0x20: AND_RM8_R8();   break;
	case 0x21: AND_RM16_R16(); break;
	case 0x22: AND_R8_RM8();   break;
	case 0x23: AND_R16_RM16(); break;
	case 0x24: AND_AL_I8();    break;
	case 0x25: AND_AX_I16();   break;
	case 0x26: ES();           break;
	case 0x27: DAA();          break;
	case 0x28: SUB_RM8_R8();   break;
	case 0x29: SUB_RM16_R16(); break;
	case 0x2a: SUB_R8_RM8();   break;
	case 0x2b: SUB_R16_RM16(); break;
	case 0x2c: SUB_AL_I8();    break;
	case 0x2d: SUB_AX_I16();   break;
	case 0x2e: CS();           break;
	case 0x2f: DAS();          break;
	case 0x30: XOR_RM8_R8();   break;
	case 0x31: XOR_RM16_R16(); break;
	case 0x32: XOR_R8_RM8();   break;
	case 0x33: XOR_R16_RM16(); break;
	case 0x34: XOR_AL_I8();    break;
	case 0x35: XOR_AX_I16();   break;
	case 0x36: SS();           break;
	case 0x37: AAA();          break;
	case 0x38: CMP_RM8_R8();   break;
	case 0x39: CMP_RM16_R16(); break;
	case 0x3a: CMP_R8_RM8();   break;
	case 0x3b: CMP_R16_RM16(); break;
	case 0x3c: CMP_AL_I8();    break;
	case 0x3d: CMP_AX_I16();   break;
	case 0x3e: DS();           break;
	case 0x3f: AAS();          break;
	case 0x40: INC_AX();       break;
	case 0x41: INC_CX();       break;
	case 0x42: INC_DX();       break;
	case 0x43: INC_BX();       break;
	case 0x44: INC_SP();       break;
	case 0x45: INC_BP();       break;
	case 0x46: INC_SI();       break;
	case 0x47: INC_DI();       break;
	case 0x48: DEC_AX();       break;
	case 0x49: DEC_CX();       break;
	case 0x4a: DEC_DX();       break;
	case 0x4b: DEC_BX();       break;
	case 0x4c: DEC_SP();       break;
	case 0x4d: DEC_BP();       break;
	case 0x4e: DEC_SI();       break;
	case 0x4f: DEC_DI();       break;
	case 0x50: PUSH_AX();      break;
	case 0x51: PUSH_CX();      break;
	case 0x52: PUSH_DX();      break;
	case 0x53: PUSH_BX();      break;
	case 0x54: PUSH_SP();      break;
	case 0x55: PUSH_BP();      break;
	case 0x56: PUSH_SI();      break;
	case 0x57: PUSH_DI();      break;
	case 0x58: POP_AX();       break;
	case 0x59: POP_CX();       break;
	case 0x5a: POP_DX();       break;
	case 0x5b: POP_BX();       break;
	case 0x5c: POP_SP();       break;
	case 0x5d: POP_BP();       break;
	case 0x5e: POP_SI();       break;
	case 0x5f: POP_DI();       break;
	case 0x60: UndefinedOpcode(); break;
	case 0x61: UndefinedOpcode(); break;
	case 0x62: UndefinedOpcode(); break;
	case 0x63: UndefinedOpcode(); break;
	case 0x64: UndefinedOpcode(); break;
	case 0x65: UndefinedOpcode(); break;
	case 0x66: PREFIX_OprSize();  break;
	case 0x67: PREFIX_AddrSize(); break;
	case 0x68: UndefinedOpcode(); break;
	case 0x69: UndefinedOpcode(); break;
	case 0x6a: UndefinedOpcode(); break;
	case 0x6b: UndefinedOpcode(); break;
	case 0x6c: UndefinedOpcode(); break;
	case 0x6d: UndefinedOpcode(); break;
	case 0x6e: UndefinedOpcode(); break;
	case 0x6f: UndefinedOpcode(); break;
	case 0x70: JO();           break;
	case 0x71: JNO();          break;
	case 0x72: JC();           break;
	case 0x73: JNC();          break;
	case 0x74: JZ();           break;
	case 0x75: JNZ();          break;
	case 0x76: JBE();          break;
	case 0x77: JA();           break;
	case 0x78: JS();           break;
	case 0x79: JNS();          break;
	case 0x7a: JP();           break;
	case 0x7b: JNP();          break;
	case 0x7c: JL();           break;
	case 0x7d: JNL();          break;
	case 0x7e: JLE();          break;
	case 0x7f: JG();           break;
	case 0x80: INS_80();       break;
	case 0x81: INS_81();       break;
	case 0x82: INS_82();       break;
	case 0x83: INS_83();       break;
	case 0x84: TEST_RM8_R8();  break;
	case 0x85: TEST_RM16_R16();break;
	case 0x86: XCHG_R8_RM8();  break;
	case 0x87: XCHG_R16_RM16();break;
	case 0x88: MOV_RM8_R8();   break;
	case 0x89: MOV_RM16_R16(); break;
	case 0x8a: MOV_R8_RM8();   break;
	case 0x8b: MOV_R16_RM16(); break;
	case 0x8c: MOV_RM16_SREG();break;
	case 0x8d: LEA_R16_M16();  break;
	case 0x8e: MOV_SREG_RM16();break;
	case 0x8f: POP_RM16();     break;
	case 0x90: NOP();          break;
	case 0x91: XCHG_CX_AX();   break;
	case 0x92: XCHG_DX_AX();   break;
	case 0x93: XCHG_BX_AX();   break;
	case 0x94: XCHG_SP_AX();   break;
	case 0x95: XCHG_BP_AX();   break;
	case 0x96: XCHG_SI_AX();   break;
	case 0x97: XCHG_DI_AX();   break;
	case 0x98: CBW();          break;
	case 0x99: CWD();          break;
	case 0x9a: CALL_PTR16_16();break;
	case 0x9b: WAIT();         break;
	case 0x9c: PUSHF();        break;
	case 0x9d: POPF();         break;
	case 0x9e: SAHF();         break;
	case 0x9f: LAHF();         break;
	case 0xa0: MOV_AL_M8();    break;
	case 0xa1: MOV_AX_M16();   break;
	case 0xa2: MOV_M8_AL();    break;
	case 0xa3: MOV_M16_AX();   break;
	case 0xa4: MOVSB();        break;
	case 0xa5: MOVSW();        break;
	case 0xa6: CMPSB();        break;
	case 0xa7: CMPSW();        break;
	case 0xa8: TEST_AL_I8();   break;
	case 0xa9: TEST_AX_I16();  break;
	case 0xaa: STOSB();        break;
	case 0xab: STOSW();        break;
	case 0xac: LODSB();        break;
	case 0xad: LODSW();        break;
	case 0xae: SCASB();        break;
	case 0xaf: SCASW();        break;
	case 0xb0: MOV_AL_I8();    break;
	case 0xb1: MOV_CL_I8();    break;
	case 0xb2: MOV_DL_I8();    break;
	case 0xb3: MOV_BL_I8();    break;
	case 0xb4: MOV_AH_I8();    break;
	case 0xb5: MOV_CH_I8();    break;
	case 0xb6: MOV_DH_I8();    break;
	case 0xb7: MOV_BH_I8();    break;
	case 0xb8: MOV_AX_I16();   break;
	case 0xb9: MOV_CX_I16();   break;
	case 0xba: MOV_DX_I16();   break;
	case 0xbb: MOV_BX_I16();   break;
	case 0xbc: MOV_SP_I16();   break;
	case 0xbd: MOV_BP_I16();   break;
	case 0xbe: MOV_SI_I16();   break;
	case 0xbf: MOV_DI_I16();   break;
	case 0xc0: UndefinedOpcode(); break;
	case 0xc1: UndefinedOpcode(); break;
	case 0xc2: RET_I16();      break;
	case 0xc3: RET();          break;
	case 0xc4: LES_R16_M16();  break;
	case 0xc5: LDS_R16_M16();  break;
	case 0xc6: MOV_M8_I8();    break;
	case 0xc7: MOV_M16_I16();  break;
	case 0xc8: UndefinedOpcode(); break;
	case 0xc9: UndefinedOpcode(); break;
	case 0xca: RETF_I16();     break;
	case 0xcb: RETF();         break;
	case 0xcc: INT3();         break;
	case 0xcd: INT_I8();       break;
	case 0xce: INTO();         break;
	case 0xcf: IRET();         break;
	case 0xd0: INS_D0();       break;
	case 0xd1: INS_D1();       break;
	case 0xd2: INS_D2();       break;
	case 0xd3: INS_D3();       break;
	case 0xd4: AAM();          break;
	case 0xd5: AAD();          break;
	case 0xd6: UndefinedOpcode(); break;
	case 0xd7: XLAT();         break;
	case 0xd8: UndefinedOpcode(); break;
	case 0xd9: UndefinedOpcode(); break; //case 0xd9: INS_D9();     break;
	case 0xda: UndefinedOpcode(); break;
	case 0xdb: UndefinedOpcode(); break; //case 0xdb: INS_UndefinedOpcode();     break;
	case 0xdc: UndefinedOpcode(); break;
	case 0xdd: UndefinedOpcode(); break;
	case 0xde: UndefinedOpcode(); break;
	case 0xdf: UndefinedOpcode(); break;
	case 0xe0: LOOPNZ();       break;
	case 0xe1: LOOPZ();        break;
	case 0xe2: LOOP();         break;
	case 0xe3: JCXZ_REL8();    break;
	case 0xe4: IN_AL_I8();     break;
	case 0xe5: IN_AX_I8();     break;
	case 0xe6: OUT_I8_AL();    break;
	case 0xe7: OUT_I8_AX();    break;
	case 0xe8: CALL_REL16();   break;
	case 0xe9: JMP_REL16();    break;
	case 0xea: JMP_PTR16_16(); break;
	case 0xeb: JMP_REL8();     break;
	case 0xec: IN_AL_DX();     break;
	case 0xed: IN_AX_DX();     break;
	case 0xee: OUT_DX_AL();    break;
	case 0xef: OUT_DX_AX();    break;
	case 0xf0: LOCK();         break;
	case 0xf1: QDX();          break;
	case 0xf2: REPNZ();        break;
	case 0xf3: REP();          break;
	case 0xf4: HLT();          break;
	case 0xf5: CMC();          break;
	case 0xf6: INS_F6();       break;
	case 0xf7: INS_F7();       break;
	case 0xf8: CLC();          break;
	case 0xf9: STC();          break;
	case 0xfa: CLI();          break;
	case 0xfb: STI();          break;
	case 0xfc: CLD();          break;
	case 0xfd: STD();          break;
	case 0xfe: INS_FE();       break;
	case 0xff: INS_FF();       break;
	default:   UndefinedOpcode(); break;
	}
}

t_nubitcc dasmx(t_string stmt, t_nubit32 linear, t_nubit8 flagout)
{
	t_nubitcc i,l,len;
	t_nubit8 opcode;
	t_nubit32 physical = GetMax32(_dbgm_addr_physical_linear(linear));
	diptr = physical;
	len     = 0;
	stmt[0] = 0;
	ClrPrefix();
	do {
		dop[0]       = 0;
		dopr[0]      = 0;
		dbin[0]      = 0;
		dtip[0]      = 0;
		drm[0]       = 0;
		dr[0]        = 0;
		dimm[0]      = 0;
		dimmoff8[0]  = 0;
		dimmoff16[0] = 0;
		dimmsign[0]  = 0;
		opcode = vramByte(diptr);
		exec(opcode);
		l = diptr - physical;
		len += l;
		if (flagout) {
			for (i = 0;i < l;++i) SPRINTF(dbin, "%s%02X", dbin, vramByte(physical + i));
			SPRINTF(dstmt, "L%08X %s", linear, dbin);
			for (i = strlen(dstmt);i < 24;++i) STRCAT(dstmt, " ");
		} else dstmt[0] = 0;
		STRCAT(dstmt, dop);
		if (flagout) {
			for (i = strlen(dstmt);i < 32;++i) STRCAT(dstmt, " ");
		} else STRCAT(dstmt, "\t");
		STRCAT(dstmt, dopr);
		if (flagout == 2 && dtip[0]) {
			for (i = strlen(dstmt);i < 64;++i) STRCAT(dstmt, " ");
			STRCAT(dstmt, dtip);
		}
		STRCAT(dstmt, "\n");
		STRCAT(stmt, dstmt);
		linear += l;
		physical = diptr;
	} while (IsPrefix(opcode));
	return len;
}
t_nubitcc dasm(t_string stmt, t_nubit16 seg, t_nubit16 off, t_nubit8 flagout)
{
	t_nubitcc i,l,len;
	t_nubit8 opcode;
	t_nubit32 physical = (seg << 4) + off;
	diptr = physical;
	len     = 0;
	stmt[0] = 0;
	ClrPrefix();
	do {
		dop[0]       = 0;
		dopr[0]      = 0;
		dbin[0]      = 0;
		dtip[0]      = 0;
		drm[0]       = 0;
		dr[0]        = 0;
		dimm[0]      = 0;
		dimmoff8[0]  = 0;
		dimmoff16[0] = 0;
		dimmsign[0]  = 0;
		opcode = vramByte(diptr);
		exec(opcode);
		l = diptr - physical;
		len += l;
		if (flagout) {
			for (i = 0;i < l;++i) SPRINTF(dbin, "%s%02X", dbin, vramByte(physical + i));
			SPRINTF(dstmt, "%04X:%04X %s", seg, off, dbin);
			for (i = strlen(dstmt);i < 24;++i) STRCAT(dstmt, " ");
		} else dstmt[0] = 0;
		STRCAT(dstmt, dop);
		if (flagout) {
			for (i = strlen(dstmt);i < 32;++i) STRCAT(dstmt, " ");
		} else STRCAT(dstmt, "\t");
		STRCAT(dstmt, dopr);
		if (flagout == 2 && dtip[0]) {
			for (i = strlen(dstmt);i < 64;++i) STRCAT(dstmt, " ");
			STRCAT(dstmt, dtip);
		}
		STRCAT(dstmt, "\n");
		STRCAT(stmt, dstmt);
		off += l;
		physical = diptr;
	} while (IsPrefix(opcode));
	return len;
}
