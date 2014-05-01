/* This file is a part of NXVM project. */

#include "../utils.h"

#include "dasm32.h"

/* DEBUGGING OPTIONS ******************************************************* */
#define DASM_TRACE 0
/* ************************************************************************* */

#if DASM_TRACE == 1

static t_utils_trace_call trace;

#define _cb(s) utilsTraceCallBegin(&trace, s)
#define _ce    utilsTraceCallEnd(&trace)
#define _bb(s) utilsTraceBlockBegin(&trace, s)
#define _be    utilsTraceBlockEnd(&trace)
#define _chb(n) if (1) {(n);if (flagerror) {trace.flagerror = 1;utilsTraceFinal(&trace);break;   }} while (0)
#define _chk(n) do     {(n);if (flagerror) {trace.flagerror = 1;utilsTraceFinal(&trace);return;  }} while (0)
#define _chr(n) do     {(n);if (flagerror) {trace.flagerror = 1;utilsTraceFinal(&trace);return 0;}} while (0)
#else
#define _cb(s)
#define _ce
#define _be
#define _bb(s)
#define _chb(n) if (1) {(n);if (flagerror) break;   } while (0)
#define _chk(n) do     {(n);if (flagerror) return;  } while (0)
#define _chr(n) do     {(n);if (flagerror) return 0;} while (0)
#endif

#define _impossible_   _chk(flagerror = 1)
#define _impossible_r_ _chr(flagerror = 1)

typedef unsigned char t_dasm_prefix;

static unsigned char defsize; /* default size passed in */
static unsigned char flagerror;
static unsigned char *drcode;
static char dstmt[0x100];
static char dop[0x100], dopr[0x100], drm[0x100], dr[0x100], dimm[0x100];
static char dmovsreg[0x100], doverds[0x100], doverss[0x100];
static char dimmoff8[0x100], dimmoff16[0x100], dimmsign[0x100];
static unsigned char flagmem, flaglock;
static t_dasm_prefix prefix_oprsize, prefix_addrsize;
static unsigned char cr;
static uint64_t cimm;
static unsigned char iop;

/* instruction dispatch */
static void (*dtable[0x100])(),(*dtable_0f[0x100])();

/* stack pointer size (unused) */
/* #define _GetStackSize   (vcpu.ss.seg.data.big ? 4 : 2) */
/* operand size */
#define _GetOperandSize ((defsize ^ prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((defsize ^ prefix_addrsize) ? 4 : 2)
/* get modrm and sib bits */
#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)      (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib)   (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)    (((sib) & 0x07) >> 0)

#define _comment_
#define _newins_

static void SPRINTFSI(char *str, unsigned int imm, unsigned char byte)
{
	char sign;
	unsigned char i8u;
	unsigned short i16u;
	unsigned int i32u;
	_cb("SPRINTFSI");
	i8u = (unsigned char)(imm);
	i16u = (unsigned short)(imm);
	i32u = (unsigned int)(imm);
	switch (byte) {
	case 1:
		if ((unsigned char)(imm & 0x80)) {
			sign = '-';
		    i8u = ((~i8u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%02X", sign, i8u);
		break;
	case 2:
		if ((unsigned short)(imm & 0x8000)) {
			sign = '-';
		    i16u = ((~i16u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%04X", sign, i16u);
		break;
	case 4:
		if ((unsigned int)(imm & 0x80000000)) {
			sign = '-';
		    i32u = ((~i32u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%08X", sign, i32u);
		break;
	default:_impossible_;break;}
	_ce;
}

/* kernel decoding function */
static unsigned char _kdf_check_prefix(unsigned char opcode)
{
	_cb("_kdf_check_prefix");
	switch (opcode) {
	case 0xf0:
	case 0xf2:
	case 0xf3:
	case 0x2e:
	case 0x36:
	case 0x3e:
	case 0x26:
		_ce;
		return 1;
		break;
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		_ce;
		return 1;
		break;
	default:
		_ce;
		return 0;
		break;
	}
	_ce;
	return 0;
}

static void _kdf_skip(unsigned char byte)
{
	_cb("_kdf_skip");
	_chk(iop += byte);
	_ce;
}
static void _kdf_code(unsigned char *rdata, unsigned char byte)
{
	unsigned char i;
	_cb("_kdf_code");
	for (i = 0;i < byte;++i)
		*(rdata + i) = *(drcode + iop + i);
	_chk(_kdf_skip(byte));
	_ce;
}
static void _kdf_modrm(unsigned char regbyte, unsigned char rmbyte)
{
	char disp8;
	unsigned short disp16;
	unsigned int disp32;
	char dsibindex[0x100], dptr[0x100];
	unsigned char modrm, sib;
	char sign;
	unsigned char disp8u;
	_cb("_kdf_modrm");
	_chk(_kdf_code(&modrm, 1));
	flagmem = 1;
	drm[0] = dr[0] = dsibindex[0] = 0;
	switch (rmbyte) {
	case 1: SPRINTF(dptr, "BYTE PTR ");break;
	case 2: SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dptr, "DWORD PTR ");break;
	default:dptr[0] = 0;break;}
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		switch (_GetModRM_MOD(modrm)) {
		case 0: _bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[BX+SI]", doverds);break;
			case 1: SPRINTF(drm, "%s:[BX+DI]", doverds);break;
			case 2: SPRINTF(drm, "%s:[BP+SI]", doverss);break;
			case 3: SPRINTF(drm, "%s:[BP+DI]", doverss);break;
			case 4: SPRINTF(drm, "%s:[SI]",    doverds);break;
			case 5: SPRINTF(drm, "%s:[DI]",    doverds);break;
			case 6: _bb("ModRM_RM(6)");
				_chk(_kdf_code((unsigned char *)(&disp16), 2));
				SPRINTF(drm, "%s:[%04X]", doverds, disp16);
				_be;break;
			case 7: SPRINTF(drm, "%s:[BX]", doverds);break;
			default:_impossible_;break;}
			
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(_kdf_code((unsigned char *)(&disp8), 1));
			sign = (disp8 & 0x80) ? '-' : '+';
			disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[BX+SI%c%02X]", doverds, sign, disp8u);break;
			case 1: SPRINTF(drm, "%s:[BX+DI%c%02X]", doverds, sign, disp8u);break;
			case 2: SPRINTF(drm, "%s:[BP+SI%c%02X]", doverss, sign, disp8u);break;
			case 3: SPRINTF(drm, "%s:[BP+DI%c%02X]", doverss, sign, disp8u);break;
			case 4: SPRINTF(drm, "%s:[SI%c%02X]",    doverds, sign, disp8u);break;
			case 5: SPRINTF(drm, "%s:[DI%c%02X]",    doverds, sign, disp8u);break;
			case 6: SPRINTF(drm, "%s:[BP%c%02X]",    doverss, sign, disp8u);break;
			case 7: SPRINTF(drm, "%s:[BX%c%02X]",    doverds, sign, disp8u);break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(_kdf_code((unsigned char *)(&disp16), 2));
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[BX+SI+%04X]", doverds, disp16);break;
			case 1: SPRINTF(drm, "%s:[BX+DI+%04X]", doverds, disp16);break;
			case 2: SPRINTF(drm, "%s:[BP+SI+%04X]", doverss, disp16);break;
			case 3: SPRINTF(drm, "%s:[BP+DI+%04X]", doverss, disp16);break;
			case 4: SPRINTF(drm, "%s:[SI+%04X]",    doverds, disp16);break;
			case 5: SPRINTF(drm, "%s:[DI+%04X]",    doverds, disp16);break;
			case 6: SPRINTF(drm, "%s:[BP+%04X]",    doverss, disp16);break;
			case 7: SPRINTF(drm, "%s:[BX+%04X]",    doverds, disp16);break;
			default:_impossible_;break;}
			_be;break;
		case 3:
			break;
		default:
			_impossible_;
			break;
		}
		_be;break;
	case 4: _bb("AddressSize(4)");
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			_bb("ModRM_MOD(!3),ModRM_RM(4)");
			_chk(_kdf_code((unsigned char *)(&sib), 1));
			switch (_GetSIB_Index(sib)) {
			case 0: SPRINTF(dsibindex, "+EAX*%02X", (1 << _GetSIB_SS(sib)));break;
			case 1: SPRINTF(dsibindex, "+ECX*%02X", (1 << _GetSIB_SS(sib)));break;
			case 2: SPRINTF(dsibindex, "+EDX*%02X", (1 << _GetSIB_SS(sib)));break;
			case 3: SPRINTF(dsibindex, "+EBX*%02X", (1 << _GetSIB_SS(sib)));break;
			case 4: break;
			case 5: SPRINTF(dsibindex, "+EBP*%02X", (1 << _GetSIB_SS(sib)));break;
			case 6: SPRINTF(dsibindex, "+ESI*%02X", (1 << _GetSIB_SS(sib)));break;
			case 7: SPRINTF(dsibindex, "+EDI*%02X", (1 << _GetSIB_SS(sib)));break;
			default:_impossible_;break;}
		}
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			_bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[EAX]", doverds);break;
			case 1: SPRINTF(drm, "%s:[ECX]", doverds);break;
			case 2: SPRINTF(drm, "%s:[EDX]", doverds);break;
			case 3: SPRINTF(drm, "%s:[EBX]", doverds);break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm, "%s:[EAX%s]", doverds, dsibindex);break;
				case 1: SPRINTF(drm, "%s:[ECX%s]", doverds, dsibindex);break;
				case 2: SPRINTF(drm, "%s:[EDX%s]", doverds, dsibindex);break;
				case 3: SPRINTF(drm, "%s:[EBX%s]", doverds, dsibindex);break;
				case 4: SPRINTF(drm, "%s:[ESP%s]", doverss, dsibindex);break;
				case 5: _bb("SIB_Base(5)");
					_chk(_kdf_code((unsigned char *)(&disp32), 4));
					SPRINTF(drm, "%s:[%08X%s]", doverds, disp32, dsibindex);
					_be;break;
				case 6: SPRINTF(drm, "%s:[ESI%s]", doverds, dsibindex);break;
				case 7: SPRINTF(drm, "%s:[EDI%s]", doverds, dsibindex);break;
				default:_impossible_;break;}
				_be;break;
			case 5: _bb("ModRM_RM(5)");
				_chk(_kdf_code((unsigned char *)(&disp32), 4));
				SPRINTF(drm, "%s:[%08X]", doverds, disp32);
				_be;break;
			case 6: SPRINTF(drm, "%s:[ESI]", doverds);break;
			case 7: SPRINTF(drm, "%s:[EDI]", doverds);break;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(_kdf_code((unsigned char *)(&disp8), 1));
			sign = (disp8 & 0x80) ? '-' : '+';
			disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[EAX%c%02X]", doverds, sign, disp8u);break;
			case 1: SPRINTF(drm, "%s:[ECX%c%02X]", doverds, sign, disp8u);break;
			case 2: SPRINTF(drm, "%s:[EDX%c%02X]", doverds, sign, disp8u);break;
			case 3: SPRINTF(drm, "%s:[EBX%c%02X]", doverds, sign, disp8u);break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm, "%s:[EAX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 1: SPRINTF(drm, "%s:[ECX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 2: SPRINTF(drm, "%s:[EDX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 3: SPRINTF(drm, "%s:[EBX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 4: SPRINTF(drm, "%s:[ESP%s%c%02X]", doverss, dsibindex, sign, disp8u);break;
				case 5: SPRINTF(drm, "%s:[EBP%s%c%02X]", doverss, dsibindex, sign, disp8u);break;
				case 6: SPRINTF(drm, "%s:[ESI%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 7: SPRINTF(drm, "%s:[EDI%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				default:_impossible_;break;}
				_be;break;
			case 5: SPRINTF(drm, "%s:[EBP%c%02X]", doverss, sign, disp8u);break;
			case 6: SPRINTF(drm, "%s:[ESI%c%02X]", doverds, sign, disp8u);break;
			case 7: SPRINTF(drm, "%s:[EDI%c%02X]", doverds, sign, disp8u);break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(_kdf_code((unsigned char *)(&disp32), 4));
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "%s:[EAX+%08X]", doverds, disp32);break;
			case 1: SPRINTF(drm, "%s:[ECX+%08X]", doverds, disp32);break;
			case 2: SPRINTF(drm, "%s:[EDX+%08X]", doverds, disp32);break;
			case 3: SPRINTF(drm, "%s:[EBX+%08X]", doverds, disp32);break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm, "%s:[EAX%s+%08X]", doverds, dsibindex, disp32);break;
				case 1: SPRINTF(drm, "%s:[ECX%s+%08X]", doverds, dsibindex, disp32);break;
				case 2: SPRINTF(drm, "%s:[EDX%s+%08X]", doverds, dsibindex, disp32);break;
				case 3: SPRINTF(drm, "%s:[EBX%s+%08X]", doverds, dsibindex, disp32);break;
				case 4: SPRINTF(drm, "%s:[ESP%s+%08X]", doverss, dsibindex, disp32);break;
				case 5: SPRINTF(drm, "%s:[EBP%s+%08X]", doverss, dsibindex, disp32);break;
				case 6: SPRINTF(drm, "%s:[ESI%s+%08X]", doverds, dsibindex, disp32);break;
				case 7: SPRINTF(drm, "%s:[EDI%s+%08X]", doverds, dsibindex, disp32);break;
				default:_impossible_;break;}
				_be;break;
			case 5: SPRINTF(drm, "%s:[EBP+%08X]", doverss, disp32);break;
			case 6: SPRINTF(drm, "%s:[ESI+%08X]", doverds, disp32);break;
			case 7: SPRINTF(drm, "%s:[EDI+%08X]", doverds, disp32);break;
			default:_impossible_;break;}
			_be;break;
		case 3:
			break;
		default:
			_impossible_;
			break;
		}
		_be;break;
	default:
		_impossible_;
		break;
	}
	if (_GetModRM_MOD(modrm) == 3) {
		_bb("ModRM_MOD(3)");
		flagmem = 0;
		switch (rmbyte) {
		case 1:
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "AL");break;
			case 1: SPRINTF(drm, "CL");break;
			case 2: SPRINTF(drm, "DL");break;
			case 3: SPRINTF(drm, "BL");break;
			case 4: SPRINTF(drm, "AH");break;
			case 5: SPRINTF(drm, "CH");break;
			case 6: SPRINTF(drm, "DH");break;
			case 7: SPRINTF(drm, "BH");break;
			default:_impossible_;break;}
			break;
		case 2:
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "AX");break;
			case 1: SPRINTF(drm, "CX");break;
			case 2: SPRINTF(drm, "DX");break;
			case 3: SPRINTF(drm, "BX");break;
			case 4: SPRINTF(drm, "SP");break;
			case 5: SPRINTF(drm, "BP");break;
			case 6: SPRINTF(drm, "SI");break;
			case 7: SPRINTF(drm, "DI");break;
			default:_impossible_;break;}
			break;
		case 4:
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm, "EAX");break;
			case 1: SPRINTF(drm, "ECX");break;
			case 2: SPRINTF(drm, "EDX");break;
			case 3: SPRINTF(drm, "EBX");break;
			case 4: SPRINTF(drm, "ESP");break;
			case 5: SPRINTF(drm, "EBP");break;
			case 6: SPRINTF(drm, "ESI");break;
			case 7: SPRINTF(drm, "EDI");break;
			default:_impossible_;break;}
			break;
		default:_impossible_;break;}
		_be;
	}
	switch (regbyte) {
	case 0:
		if (flagmem) {
			STRCAT(dptr, drm);
			STRCPY(drm, dptr);
		}
	case 9:
		/* reg is operation or segment */
		cr = _GetModRM_REG(modrm);
		break;
	case 1:
		switch (_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "AL");break;
		case 1: SPRINTF(dr, "CL");break;
		case 2: SPRINTF(dr, "DL");break;
		case 3: SPRINTF(dr, "BL");break;
		case 4: SPRINTF(dr, "AH");break;
		case 5: SPRINTF(dr, "CH");break;
		case 6: SPRINTF(dr, "DH");break;
		case 7: SPRINTF(dr, "BH");break;
		default:_impossible_;break;}
		break;
	case 2:
		switch (_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "AX");break;
		case 1: SPRINTF(dr, "CX");break;
		case 2: SPRINTF(dr, "DX");break;
		case 3: SPRINTF(dr, "BX");break;
		case 4: SPRINTF(dr, "SP");break;
		case 5: SPRINTF(dr, "BP");break;
		case 6: SPRINTF(dr, "SI");break;
		case 7: SPRINTF(dr, "DI");break;
		default:_impossible_;break;}
		break;
	case 4:
		switch (_GetModRM_REG(modrm)) {
		case 0: SPRINTF(dr, "EAX");break;
		case 1: SPRINTF(dr, "ECX");break;
		case 2: SPRINTF(dr, "EDX");break;
		case 3: SPRINTF(dr, "EBX");break;
		case 4: SPRINTF(dr, "ESP");break;
		case 5: SPRINTF(dr, "EBP");break;
		case 6: SPRINTF(dr, "ESI");break;
		case 7: SPRINTF(dr, "EDI");break;
		default:_impossible_;break;}
		break;
	default: _impossible_;break;
	}
	_ce;
}
static void _d_skip(unsigned char byte)
{
	_cb("_d_skip");
	_chk(_kdf_skip(byte));
	_ce;
}
static void _d_code(unsigned char *rdata, unsigned char byte)
{
	_cb("_d_code");
	_chk(_kdf_code(rdata, byte));
	_ce;
}
static void _d_imm(unsigned char byte)
{
	_cb("_d_imm");
	cimm = 0;
	_chk(_d_code((unsigned char *)(&cimm), byte));
	_ce;
}
static void _d_moffs(unsigned char byte)
{
	unsigned int offset = 0;
	_cb("_d_moffs");
	flagmem = 1;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		_chk(_d_code((unsigned char *)(&offset), 2));
		SPRINTF(drm, "%s:[%04X]", doverds, (unsigned short)(offset));
		_be;break;
	case 4: _bb("AddressSize(4)");
		_chk(_d_code((unsigned char *)(&offset), 4));
		SPRINTF(drm, "%s:[%08X]", doverds, (unsigned int)(offset));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_modrm_sreg(unsigned char rmbyte)
{
	_cb("_d_modrm_sreg");
	_chk(_kdf_modrm(0, rmbyte));
	switch (cr) {
	case 0: SPRINTF(dr, "ES");break;
	case 1: SPRINTF(dr, "CS");break;
	case 2: SPRINTF(dr, "SS");break;
	case 3: SPRINTF(dr, "DS");break;
	case 4: SPRINTF(dr, "FS");break;
	case 5: SPRINTF(dr, "GS");break;
	default: _bb("cr");
		SPRINTF(dr, "<ERROR>");
		_be;break;
	}
	_ce;
}
static void _d_modrm(unsigned char regbyte, unsigned char rmbyte)
{
	_cb("_d_modrm");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!flagmem && flaglock) {
		_bb("flagmem(0),flaglock(1)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	_ce;
}

#define _adv _chk(_d_skip(1))

static void UndefinedOpcode()
{
	_cb("UndefinedOpcode");
	SPRINTF(dop, "<ERROR>");
	_ce;
}
static void ADD_RM8_R8()
{
	_cb("ADD_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void ADD_RM32_R32()
{
	_cb("ADD_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void ADD_R8_RM8()
{
	_cb("ADD_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void ADD_R32_RM32()
{
	_cb("ADD_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void ADD_AL_I8()
{
	_cb("ADD_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "ADD");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void ADD_EAX_I32()
{
	_cb("ADD_EAX_I32");
	_adv;
	SPRINTF(dop, "ADD");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ES()
{
	_cb("PUSH_ES");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "ES");
	_ce;
}
static void POP_ES()
{
	_cb("POP_ES");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "ES");
	_ce;
}
static void OR_RM8_R8()
{
	_cb("OR_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void OR_RM32_R32()
{
	_cb("OR_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void OR_R8_RM8()
{
	_cb("OR_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void OR_R32_RM32()
{
	_cb("OR_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void OR_AL_I8()
{
	_cb("OR_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "OR");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void OR_EAX_I32()
{
	_cb("OR_EAX_I32");
	_adv;
	SPRINTF(dop, "OR");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void PUSH_CS()
{
	_cb("PUSH_CS");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "CS");
	_ce;
}
static void POP_CS()
{
	_cb("POP_CS");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "CS");
	_ce;
}
static void INS_0F()
{
	unsigned char oldiop;
	unsigned char opcode;
	_cb("INS_0F");
	_adv;
	oldiop = iop;
	_chk(_d_code((unsigned char *)(&opcode), 1));
	iop = oldiop;
    _chk((*(dtable_0f[opcode]))());
	_ce;
}
static void ADC_RM8_R8()
{
	_cb("ADC_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void ADC_RM32_R32()
{
	_cb("ADC_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void ADC_R8_RM8()
{
	_cb("ADC_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void ADC_R32_RM32()
{
	_cb("ADC_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void ADC_AL_I8()
{
	_cb("ADC_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "ADC");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void ADC_EAX_I32()
{
	_cb("ADC_EAX_I32");
	_adv;
	SPRINTF(dop, "ADC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void PUSH_SS()
{
	_cb("PUSH_SS");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "SS");
	_ce;
}
static void POP_SS()
{
	_cb("POP_SS");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "SS");
	_ce;
}
static void SBB_RM8_R8()
{
	_cb("SBB_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SBB_RM32_R32()
{
	_cb("SBB_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SBB_R8_RM8()
{
	_cb("SBB_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void SBB_R32_RM32()
{
	_cb("SBB_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void SBB_AL_I8()
{
	_cb("SBB_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "SBB");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void SBB_EAX_I32()
{
	_cb("SBB_EAX_I32");
	_adv;
	SPRINTF(dop, "SBB");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void PUSH_DS()
{
	_cb("PUSH_DS");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "DS");
	_ce;
}
static void POP_DS()
{
	_cb("POP_DS");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "DS");
	_ce;
}
static void AND_RM8_R8()
{
	_cb("AND_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void AND_RM32_R32()
{
	_cb("AND_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void AND_R8_RM8()
{
	_cb("AND_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void AND_R32_RM32()
{
	_cb("AND_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void AND_AL_I8()
{
	_cb("AND_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "AND");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void AND_EAX_I32()
{
	_cb("AND_EAX_I32");
	_adv;
	SPRINTF(dop, "AND");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PREFIX_ES()
{
	_cb("PREFIX_ES");
	_adv;
	SPRINTF(doverds, "ES");
	SPRINTF(doverss, "ES");
	_ce;
}
static void DAA()
{
	_cb("DAA");
	_adv;
	SPRINTF(dop, "DAA");
	_ce;
}
static void SUB_RM8_R8()
{
	_cb("SUB_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SUB_RM32_R32()
{
	_cb("SUB_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SUB_R8_RM8()
{
	_cb("SUB_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void SUB_R32_RM32()
{
	_cb("SUB_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void SUB_AL_I8()
{
	_cb("SUB_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "SUB");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void SUB_EAX_I32()
{
	_cb("SUB_EAX_I32");
	_adv;
	SPRINTF(dop, "SUB");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PREFIX_CS()
{
	_cb("PREFIX_CS");
	_adv;
	SPRINTF(doverds, "CS");
	SPRINTF(doverss, "CS");
	_ce;
}
static void DAS()
{
	_cb("DAS");
	_adv;
	SPRINTF(dop, "DAS");
	_ce;
}
static void XOR_RM8_R8()
{
	_cb("XOR_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void XOR_RM32_R32()
{
	_cb("XOR_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void XOR_R8_RM8()
{
	_cb("XOR_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void XOR_R32_RM32()
{
	_cb("XOR_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void XOR_AL_I8()
{
	_cb("XOR_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "XOR");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void XOR_EAX_I32()
{
	_cb("XOR_EAX_I32");
	_adv;
	SPRINTF(dop, "XOR");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PREFIX_SS()
{
	_cb("PREFIX_SS");
	_adv;
	SPRINTF(doverds, "SS");
	SPRINTF(doverss, "SS");
	_ce;
}
static void AAA()
{
	_cb("AAA");
	_adv;
	SPRINTF(dop, "AAA");
	_ce;
}
static void CMP_RM8_R8()
{
	_cb("CMP_RM8_R8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void CMP_RM32_R32()
{
	_cb("CMP_RM32_R32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void CMP_R8_RM8()
{
	_cb("CMP_R8_RM8");
	_adv;
	_chk(_d_modrm(1, 1));
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void CMP_R32_RM32()
{
	_cb("CMP_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void CMP_AL_I8()
{
	_cb("CMP_AL_I8");
	_adv;
	_chk(_d_imm(1));
	SPRINTF(dop, "CMP");
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void CMP_EAX_I32()
{
	_cb("CMP_EAX_I32");
	_adv;
	SPRINTF(dop, "CMP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PREFIX_DS()
{
	_cb("PREFIX_DS");
	_adv;
	SPRINTF(doverds, "DS");
	SPRINTF(doverss, "DS");
	_ce;
}
static void AAS()
{
	_cb("AAS");
	_adv;
	SPRINTF(dop, "AAS");
	_ce;
}
static void INC_EAX()
{
	_cb("INC_EAX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX");break;
	case 4: SPRINTF(dopr, "EAX");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ECX()
{
	_cb("INC_ECX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "CX");break;
	case 4: SPRINTF(dopr, "ECX");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EDX()
{
	_cb("INC_EDX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX");break;
	case 4: SPRINTF(dopr, "EDX");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EBX()
{
	_cb("INC_EBX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BX");break;
	case 4: SPRINTF(dopr, "EBX");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ESP()
{
	_cb("INC_ESP");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SP");break;
	case 4: SPRINTF(dopr, "ESP");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EBP()
{
	_cb("INC_EBP");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BP");break;
	case 4: SPRINTF(dopr, "EBP");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ESI()
{
	_cb("INC_ESI");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SI");break;
	case 4: SPRINTF(dopr, "ESI");break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EDI()
{
	_cb("INC_EDI");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DI");break;
	case 4: SPRINTF(dopr, "EDI");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EAX()
{
	_cb("DEC_EAX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX");break;
	case 4: SPRINTF(dopr, "EAX");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ECX()
{
	_cb("DEC_ECX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "CX");break;
	case 4: SPRINTF(dopr, "ECX");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EDX()
{
	_cb("DEC_EDX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX");break;
	case 4: SPRINTF(dopr, "EDX");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EBX()
{
	_cb("DEC_EBX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BX");break;
	case 4: SPRINTF(dopr, "EBX");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ESP()
{
	_cb("DEC_ESP");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SP");break;
	case 4: SPRINTF(dopr, "ESP");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EBP()
{
	_cb("DEC_EBP");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BP");break;
	case 4: SPRINTF(dopr, "EBP");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ESI()
{
	_cb("DEC_ESI");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SI");break;
	case 4: SPRINTF(dopr, "ESI");break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EDI()
{
	_cb("DEC_EDI");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DI");break;
	case 4: SPRINTF(dopr, "EDI");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EAX()
{
	_cb("PUSH_EAX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX");break;
	case 4: SPRINTF(dopr, "EAX");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ECX()
{
	_cb("PUSH_ECX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "CX");break;
	case 4: SPRINTF(dopr, "ECX");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EDX()
{
	_cb("PUSH_EDX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX");break;
	case 4: SPRINTF(dopr, "EDX");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EBX()
{
	_cb("PUSH_EBX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BX");break;
	case 4: SPRINTF(dopr, "EBX");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ESP()
{
	_cb("PUSH_ESP");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SP");break;
	case 4: SPRINTF(dopr, "ESP");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EBP()
{
	_cb("PUSH_EBP");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BP");break;
	case 4: SPRINTF(dopr, "EBP");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ESI()
{
	_cb("PUSH_ESI");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SI");break;
	case 4: SPRINTF(dopr, "ESI");break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EDI()
{
	_cb("PUSH_EDI");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DI");break;
	case 4: SPRINTF(dopr, "EDI");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EAX()
{
	_cb("POP_EAX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX");break;
	case 4: SPRINTF(dopr, "EAX");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ECX()
{
	_cb("POP_ECX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "CX");break;
	case 4: SPRINTF(dopr, "ECX");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EDX()
{
	_cb("POP_EDX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX");break;
	case 4: SPRINTF(dopr, "EDX");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EBX()
{
	_cb("POP_EBX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BX");break;
	case 4: SPRINTF(dopr, "EBX");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ESP()
{
	_cb("POP_ESP");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SP");break;
	case 4: SPRINTF(dopr, "ESP");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EBP()
{
	_cb("POP_EBP");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BP");break;
	case 4: SPRINTF(dopr, "EBP");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ESI()
{
	_cb("POP_ESI");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SI");break;
	case 4: SPRINTF(dopr, "ESI");break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EDI()
{
	_cb("POP_EDI");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DI");break;
	case 4: SPRINTF(dopr, "EDI");break;
	default:_impossible_;break;}
	_ce;
}


static void PUSHA()
{
	_cb("PUSHA");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "PUSHA");break;
	case 4: SPRINTF(dop, "PUSHAD");break;
	default:_impossible_;break;}
	_ce;
}
static void POPA()
{
	_cb("POPA");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "POPA");break;
	case 4: SPRINTF(dop, "POPAD");break;
	default:_impossible_;break;}
	_ce;
}
static void BOUND_R16_M16_16()
{
	_cb("BOUND_R16_M16_16");
	_adv;
	SPRINTF(dop, "BOUND");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize * 2));
	if (!flagmem) {
		SPRINTF(dopr, "<ERROR>");
	} else {
		SPRINTF(dopr, "%s,%s", dr, drm);
	}
	_ce;
}
static void ARPL_RM16_R16()
{
	_cb("ARPL_RM16_R16");
	_adv;
	SPRINTF(dop, "ARPL");
	_chk(_d_modrm(2, 2));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void PREFIX_FS()
{
	_cb("PREFIX_FS");
	_adv;
	SPRINTF(doverds, "FS");
	SPRINTF(doverss, "FS");
	_ce;
}
static void PREFIX_GS()
{
	_cb("PREFIX_GS");
	_adv;
	SPRINTF(doverds, "GS");
	SPRINTF(doverss, "GS");
	_ce;
}
static void PREFIX_OprSize()
{
	_cb("PREFIX_OprSize");
	_adv;
	prefix_oprsize = 0x01;
	_ce;
}
static void PREFIX_AddrSize()
{
	_cb("PREFIX_AddrSize");
	_adv;
	prefix_addrsize = 0x01;
	_ce;
}
static void PUSH_I32()
{
	_cb("PUSH_I32");
	_adv;
	SPRINTF(dop, "PUSH");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void IMUL_R32_RM32_I32()
{
	_cb("IMUL_R32_RM32_I32");
	_adv;
	SPRINTF(dop, "IMUL");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "%s,%s,%04X", dr, drm, (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "%s,%s,%08X", dr, drm, (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_I8()
{
	_cb("PUSH_I8");
	_adv;
	SPRINTF(dop, "PUSH");
	_chk(_d_imm(1));
	SPRINTF(dopr, "%02X", (unsigned char)(cimm));
	_ce;
}
static void IMUL_R32_RM32_I8()
{
	_cb("IMUL_R32_RM32_I8");
	_adv;
	SPRINTF(dop, "IMUL");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_d_imm(1));
	SPRINTF(dopr, "%s,%s,%02X", dr, drm, (unsigned char)(cimm));
	_ce;
}
static void INSB()
{
	char dptr[0x100];
	_cb("INSB");
	_adv;
	SPRINTF(dop, "INSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI],DX");break;
	case 4: SPRINTF(dopr, "ES:[EDI],DX");break;
	default:_impossible_;break;}
	_ce;
}
static void INSW()
{
	char dptr[0x100];
	_cb("INSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "INSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "INSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI],DX");break;
	case 4: SPRINTF(dopr, "ES:[EDI],DX");break;
	default:_impossible_;break;}
	_ce;
}
static void OUTSB()
{
	char dptr[0x100];
	_cb("OUTSB");
	_adv;
	SPRINTF(dop, "OUTSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "DX,%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "DX,%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void OUTSW()
{
	char dptr[0x100];
	_cb("OUTSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "OUTSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "OUTSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "DX,%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "DX,%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void JO_REL8()
{
	_cb("JO_REL8");
	_adv;
	SPRINTF(dop, "JO");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNO_REL8()
{
	_cb("JNO_REL8");
	_adv;
	SPRINTF(dop, "JNO");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JC_REL8()
{
	_cb("JC_REL8");
	_adv;
	SPRINTF(dop, "JC");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNC_REL8()
{
	_cb("JNC_REL8");
	_adv;
	SPRINTF(dop, "JNC");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JZ_REL8()
{
	_cb("JZ_REL8");
	_adv;
	SPRINTF(dop, "JZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNZ_REL8()
{
	_cb("JNZ_REL8");
	_adv;
	SPRINTF(dop, "JNZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNA_REL8()
{
	_cb("JNA_REL8");
	_adv;
	SPRINTF(dop, "JNA");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JA_REL8()
{
	_cb("JA_REL8");
	_adv;
	SPRINTF(dop, "JA");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JS_REL8()
{
	_cb("JS_REL8");
	_adv;
	SPRINTF(dop, "JS");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNS_REL8()
{
	_cb("JNS_REL8");
	_adv;
	SPRINTF(dop, "JNS");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JP_REL8()
{
	_cb("JP_REL8");
	_adv;
	SPRINTF(dop, "JP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNP_REL8()
{
	_cb("JNP_REL8");
	_adv;
	SPRINTF(dop, "JNP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JL_REL8()
{
	_cb("JL_REL8");
	_adv;
	SPRINTF(dop, "JL");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNL_REL8()
{
	_cb("JNL_REL8");
	_adv;
	SPRINTF(dop, "JNL");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JNG_REL8()
{
	_cb("JNG_REL8");
	_adv;
	SPRINTF(dop, "JNG");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JG_REL8()
{
	_cb("JG_REL8");
	_adv;
	SPRINTF(dop, "JG");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void INS_80()
{
	_cb("INS_80");
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_d_imm(1));
	switch (cr) {
	case 0: /* ADD_RM8_I8 */
		_bb("ADD_RM8_I8");
		SPRINTF(dop, "ADD");
		_be;break;
	case 1: /* OR_RM8_I8 */
		_bb("OR_RM8_I8");
		SPRINTF(dop, "OR");
		_be;break;
	case 2: /* ADC_RM8_I8 */
		_bb("ADC_RM8_I8");
		SPRINTF(dop, "ADC");
		_be;break;
	case 3: /* SBB_RM8_I8 */
		_bb("SBB_RM8_I8");
		SPRINTF(dop, "SBB");
		_be;break;
	case 4: /* AND_RM8_I8 */
		_bb("AND_RM8_I8");
		SPRINTF(dop, "AND");
		_be;break;
	case 5: /* SUB_RM8_I8 */
		_bb("SUB_RM8_I8");
		SPRINTF(dop, "SUB");
		_be;break;
	case 6: /* XOR_RM8_I8 */
		_bb("XOR_RM8_I8");
		SPRINTF(dop, "XOR");
		_be;break;
	case 7: /* CMP_RM8_I8 */
		_bb("CMP_RM8_I8");
		SPRINTF(dop, "CMP");
		_be;break;
	default:_impossible_;break;}
	SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
	_ce;
}
static void INS_81()
{
	_cb("INS_81");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	_chk(_d_imm(_GetOperandSize));
	switch (cr) {
	case 0: /* ADD_RM32_I32 */
		_bb("ADD_RM32_I32");
		SPRINTF(dop, "ADD");
		_be;break;
	case 1: /* OR_RM32_I32 */
		_bb("OR_RM32_I32");
		SPRINTF(dop, "OR");
		_be;break;
	case 2: /* ADC_RM32_I32 */
		_bb("ADC_RM32_I32");
		SPRINTF(dop, "ADC");
		_be;break;
	case 3: /* SBB_RM32_I32 */
		_bb("SBB_RM32_I32");
		SPRINTF(dop, "SBB");
		_be;break;
	case 4: /* AND_RM32_I32 */
		_bb("AND_RM32_I32");
		SPRINTF(dop, "AND");
		_be;break;
	case 5: /* SUB_RM32_I32 */
		_bb("SUB_RM32_I32");
		SPRINTF(dop, "SUB");
		_be;break;
	case 6: /* XOR_RM32_I32 */
		_bb("XOR_RM32_I32");
		SPRINTF(dop, "XOR");
		_be;break;
	case 7: /* CMP_RM32_I32 */
		_bb("CMP_RM32_I32");
		SPRINTF(dop, "CMP");
		_be;break;
	default:_impossible_;break;}
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "%s,%04X", drm, (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "%s,%08X", drm, (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void INS_83()
{
	char dsimm[0x100];
	_cb("INS_83");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	_chk(_d_imm(1));
	switch (cr) {
	case 0: /* ADD_RM32_I8 */
		_bb("ADD_RM32_I8");
		SPRINTF(dop, "ADD");
		_be;break;
	case 1: /* OR_RM32_I8 */
		_bb("OR_RM32_I8");
		SPRINTF(dop, "OR");
		_be;break;
	case 2: /* ADC_RM32_I8 */
		_bb("ADC_RM32_I8");
		SPRINTF(dop, "ADC");
		_be;break;
	case 3: /* SBB_RM32_I8 */
		_bb("SBB_RM32_I8");
		SPRINTF(dop, "SBB");
		_be;break;
	case 4: /* AND_RM32_I8 */
		_bb("AND_RM32_I8");
		SPRINTF(dop, "AND");
		_be;break;
	case 5: /* SUB_RM32_I8 */
		_bb("SUB_RM32_I8");
		SPRINTF(dop, "SUB");
		_be;break;
	case 6: /* XOR_RM32_I8 */
		_bb("XOR_RM32_I8");
		SPRINTF(dop, "XOR");
		_be;break;
	case 7: /* CMP_RM32_I8 */
		_bb("CMP_RM32_I8");
		SPRINTF(dop, "CMP");
		_be;break;
	default:_impossible_;break;}
	SPRINTFSI(dsimm, (unsigned char)(cimm), 1);
	SPRINTF(dopr, "%s,%s", drm, dsimm);
	_ce;
}
static void TEST_RM8_R8()
{
	_cb("TEST_RM8_R8");
	_adv;
	SPRINTF(dop, "TEST");
	_chk(_d_modrm(1, 1));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void TEST_RM32_R32()
{
	_cb("TEST_RM32_R32");
	_adv;
	SPRINTF(dop, "TEST");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void XCHG_RM8_R8()
{
	_cb("XCHG_RM8_R8");
	_adv;
	SPRINTF(dop, "XCHG");
	_chk(_d_modrm(1, 1));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void XCHG_RM32_R32()
{
	_cb("XCHG_RM32_R32");
	_adv;
	SPRINTF(dop, "XCHG");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_RM8_R8()
{
	_cb("MOV_RM8_R8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm(1, 1));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_RM32_R32()
{
	_cb("MOV_RM32_R32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_R8_RM8()
{
	_cb("MOV_R8_RM8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm(1, 1));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_R32_RM32()
{
	_cb("MOV_R32_RM32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_RM16_SREG()
{
	_cb("MOV_RM16_SREG");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_sreg(2));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void LEA_R32_M32()
{
	_cb("LEA_R32_M32");
	_adv;
	SPRINTF(dop, "LEA");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_SREG_RM16()
{
	_cb("MOV_SREG_RM16");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_sreg(2));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void INS_8F()
{
	_cb("INS_8F");
	_adv;
	_chk(_d_modrm(9, _GetOperandSize));
	switch (cr) {
	case 0: /* POP_RM32 */
		_bb("POP_RM32");
		switch (_GetOperandSize) {
		case 2: SPRINTF(dop, "POP");break;
		case 4: SPRINTF(dop, "POPD");break;
		default:_impossible_;break;}
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void NOP()
{
	_cb("NOP");
	_adv;
	SPRINTF(dop, "NOP");
	_ce;
}
static void XCHG_ECX_EAX()
{
	_cb("XCHG_ECX_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "CX,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ECX,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_EDX_EAX()
{
	_cb("XCHG_EDX_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DX,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDX,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_EBX_EAX()
{
	_cb("XCHG_EBX_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BX,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBX,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_ESP_EAX()
{
	_cb("XCHG_ESP_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SP,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESP,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_EBP_EAX()
{
	_cb("XCHG_EBP_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BP,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBP,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_ESI_EAX()
{
	_cb("XCHG_ESI_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SI,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESI,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void XCHG_EDI_EAX()
{
	_cb("XCHG_EDI_EAX");
	_adv;
	SPRINTF(dop, "XCHG");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DI,AX");
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDI,EAX");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void CBW()
{
	_cb("CBW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "CBW");break;
	case 4: SPRINTF(dop, "CWDE");break;
	default:_impossible_;break;}
	_ce;
}
static void CWD()
{
	_cb("CWD");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "CWD");break;
	case 4: SPRINTF(dop, "CDQ");break;
	default:_impossible_;break;}
	_ce;
}
static void CALL_PTR16_32()
{
	unsigned short newcs;
	unsigned int neweip;
	_cb("CALL_PTR16_32");
	_adv;
	SPRINTF(dop, "CALL");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(4));
		neweip = (unsigned short)(cimm);
		newcs = (unsigned short)(cimm >> 16);
		SPRINTF(dopr, "%04X:%04X", newcs, (unsigned short)(neweip));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(8));
		neweip = (unsigned int)(cimm);
		newcs = (unsigned short)(cimm >> 32);
		SPRINTF(dopr, "%04X:%08X", newcs, (unsigned int)(neweip));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void WAIT()
{
	_cb("WAIT");
	_adv;
	SPRINTF(dop, "WAIT");
	_ce;
}
static void PUSHF()
{
	_cb("PUSHF");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "PUSHF");break;
	case 4: SPRINTF(dop, "PUSHFD");break;
	default:_impossible_;break;}
	_ce;
}
static void POPF()
{
	_cb("POPF");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "POPF");break;
	case 4: SPRINTF(dop, "POPFD");break;
	default:_impossible_;break;}
	_ce;
}
static void SAHF()
{
	_cb("SAHF");
	_adv;
	SPRINTF(dop, "SAHF");
	_ce;
}
static void LAHF()
{
	_cb("LAHF");
	_adv;
	SPRINTF(dop, "LAHF");
	_ce;
}
static void MOV_AL_MOFFS8()
{
	_cb("MOV_AL_MOFFS8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_moffs(1));
	SPRINTF(dopr, "AL,%s", drm);
	_ce;
}
static void MOV_EAX_MOFFS32()
{
	_cb("MOV_EAX_MOFFS32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_moffs(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX,%s", drm);break;
	case 4: SPRINTF(dopr, "EAX,%s", drm);break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_MOFFS8_AL()
{
	_cb("MOV_MOFFS8_AL");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_moffs(1));
	SPRINTF(dopr, "%s,AL", drm);
	_ce;
}
static void MOV_MOFFS32_EAX()
{
	_cb("MOV_MOFFS32_EAX");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_moffs(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "%s,AX", drm);break;
	case 4: SPRINTF(dopr, "%s,EAX", drm);break;
	default:_impossible_;break;}
	_ce;
}
static void MOVSB()
{
	char dptr[0x100];
	_cb("MOVS");
	_adv;
	SPRINTF(dop, "MOVSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void MOVSW()
{
	char dptr[0x100];
	_cb("MOVSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "MOVSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "MOVSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void CMPSB()
{
	char dptr[0x100];
	_cb("CMPSB");
	_adv;
	SPRINTF(dop, "CMPSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);break;
	case 4: SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void CMPSW()
{
	char dptr[0x100];
	_cb("CMPSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "CMPSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "CMPSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);break;
	case 4: SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void TEST_AL_I8()
{
	_cb("TEST_AL_I8");
	_adv;
	SPRINTF(dop, "TEST");
	_chk(_d_imm(1));
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void TEST_EAX_I32()
{
	_cb("TEST_EAX_I32");
	_adv;
	SPRINTF(dop, "TEST");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void STOSB()
{
	char dptr[0x100];
	_cb("STOSB");
	_adv;
	SPRINTF(dop, "STOSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI]");break;
	case 4: SPRINTF(dopr, "ES:[EDI]");break;
	default:_impossible_;break;}
	_ce;
}
static void STOSW()
{
	char dptr[0x100];
	_cb("STOSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "STOSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "STOSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI]");break;
	case 4: SPRINTF(dopr, "ES:[EDI]");break;
	default:_impossible_;break;}
	_ce;
}
static void LODSB()
{
	char dptr[0x100];
	_cb("LODSB");
	_adv;
	SPRINTF(dop, "LODSB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void LODSW()
{
	char dptr[0x100];
	_cb("LODSW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "LODSW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "LODSD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "%s:[SI]", doverds);break;
	case 4: SPRINTF(dopr, "%s:[ESI]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void SCASB()
{
	char dptr[0x100];
	_cb("SCASB");
	_adv;
	SPRINTF(dop, "SCASB");
	SPRINTF(dptr, "BYTE PTR ");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI]");break;
	case 4: SPRINTF(dopr, "ES:[EDI]");break;
	default:_impossible_;break;}
	_ce;
}
static void SCASW()
{
	char dptr[0x100];
	_cb("SCASW");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "SCASW");SPRINTF(dptr, "WORD PTR ");break;
	case 4: SPRINTF(dop, "SCASD");SPRINTF(dptr, "DWORD PTR ");break;
	default:_impossible_;break;}
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "ES:[DI]");break;
	case 4: SPRINTF(dopr, "ES:[EDI]");break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_AL_I8()
{
	_cb("MOV_AL_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_CL_I8()
{
	_cb("MOV_CL_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "CL,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_DL_I8()
{
	_cb("MOV_DL_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "DL,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_BL_I8()
{
	_cb("MOV_BL_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "BL,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_AH_I8()
{
	_cb("MOV_AH_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "AH,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_CH_I8()
{
	_cb("MOV_CH_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "CH,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_DH_I8()
{
	_cb("MOV_DH_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "DH,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_BH_I8()
{
	_cb("MOV_BH_I8");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(1));
	SPRINTF(dopr, "BH,%02X", (unsigned char)(cimm));
	_ce;
}
static void MOV_EAX_I32()
{
	_cb("MOV_EAX_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EAX,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_ECX_I32()
{
	_cb("MOV_ECX_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "CX,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "ECX,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_EDX_I32()
{
	_cb("MOV_EDX_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EDX,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_EBX_I32()
{
	_cb("MOV_EBX_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BX,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EBX,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_ESP_I32()
{
	_cb("MOV_ESP_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SP,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "ESP,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_EBP_I32()
{
	_cb("MOV_EBP_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "BP,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EBP,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_ESI_I32()
{
	_cb("MOV_ESI_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "SI,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "ESI,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void MOV_EDI_I32()
{
	_cb("MOV_EDI_I32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DI,%04X", (unsigned short)(cimm));break;
	case 4: SPRINTF(dopr, "EDI,%08X", (unsigned int)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void INS_C0()
{
	_cb("INS_C0");
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_d_imm(1));
	switch (cr) {
	case 0: /* ROL_RM8_I8 */
		_bb("ROL_RM8_I8");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 1: /* ROR_RM8_I8 */
		_bb("ROR_RM8_I8");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 2: /* RCL_RM8_I8 */
		_bb("RCL_RM8_I8");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 3: /* RCR_RM8_I8 */
		_bb("RCR_RM8_I8");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 4: /* SHL_RM8_I8 */
		_bb("SHL_RM8_I8");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 5: /* SHR_RM8_I8 */
		_bb("SHR_RM8_I8");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8_I8 */
		_bb("SAR_RM8_I8");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_C1()
{
	_cb("INS_C1");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	_chk(_d_imm(1));
	switch (cr) {
	case 0: /* ROL_RM32_I8 */
		_bb("ROL_RM32_I8");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 1: /* ROR_RM32_I8 */
		_bb("ROR_RM32_I8");
		SPRINTF(dop, "ROR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 2: /* RCL_RM32_I8 */
		_bb("RCL_RM32_I8");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 3: /* RCR_RM32_I8 */
		_bb("RCR_RM32_I8");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 4: /* SHL_RM32_I8 */
		_bb("SHL_RM32_I8");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 5: /* SHR_RM32_I8 */
		_bb("SHR_RM32_I8");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM32_I8 */
		_bb("SAR_RM32_I8");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void RET_I16()
{
	_cb("RET_I16");
	_adv;
	SPRINTF(dop, "RET");
	_chk(_d_imm(2));
	SPRINTF(dopr, "%04X", (unsigned short)(cimm));
	_ce;
}
static void RET()
{
	_cb("RET");
	_adv;
	SPRINTF(dop, "RET");
	_ce;
}
static void LES_R32_M16_32()
{
	_cb("LES_R32_M16_32");
	_adv;
	SPRINTF(dop, "LES");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void LDS_R32_M16_32()
{
	_cb("LDS_R32_M16_32");
	_adv;
	SPRINTF(dop, "LDS");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void INS_C6()
{
	_cb("INS_C6");
	_adv;
	_chk(_d_modrm(0, 1));
	switch (cr) {
	case 0: /* MOV_RM8_I8 */
		_bb("MOV_RM8_I8");
		SPRINTF(dop, "MOV");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_C7()
{
	_cb("INS_C7");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (cr) {
	case 0: /* MOV_RM32_I32 */
		_bb("MOV_RM32_I32");
		SPRINTF(dop, "MOV");
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "%s,%04X", drm, (unsigned short)(cimm));break;
		case 4: SPRINTF(dopr, "%s,%08X", drm, (unsigned int)(cimm));break;
		default:_impossible_;break;}
		_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void ENTER()
{
	char dframesize[0x100], dnestlevel[0x100];
	_cb("ENTER");
	_adv;
	SPRINTF(dop, "ENTER");
	_chk(_d_imm(2));
	SPRINTF(dframesize, "%04X", (unsigned short)(cimm));
	_chk(_d_imm(1));
	SPRINTF(dnestlevel, "%02X", (unsigned char)(cimm));
	SPRINTF(dopr, "%s,%s", dframesize, dnestlevel);
	_ce;
}
static void LEAVE()
{
	_cb("LEAVE");
	_adv;
	SPRINTF(dop, "LEAVE");
	_ce;
}
static void RETF_I16()
{
	_cb("RETF_I16");
	_adv;
	SPRINTF(dop, "RETF");
	_chk(_d_imm(2));
	SPRINTF(dopr, "%04X", (unsigned short)(cimm));
	_ce;
}
static void RETF()
{
	_cb("RETF");
	_adv;
	SPRINTF(dop, "RETF");
	_ce;
}
static void INT3()
{
	_cb("INT3");
	_adv;
	SPRINTF(dop, "INT3");
	_ce;
}
static void INT_I8()
{
	_cb("INT_I8");
	_adv;
	SPRINTF(dop, "INT");
	_chk(_d_imm(1));
	SPRINTF(dopr, "%02X", (unsigned char)(cimm));
	_ce;
}
static void INTO()
{
	_cb("INTO");
	_adv;
	SPRINTF(dop, "INTO");
	_ce;
}
static void IRET()
{
	_cb("IRET");
	_adv;
	switch (_GetOperandSize) {
	case 2: SPRINTF(dop, "IRET");break;
	case 4: SPRINTF(dop, "IRETD");break;
	default:_impossible_;break;}
	_ce;
}
static void INS_D0()
{
	_cb("INS_D0");
	_adv;
	_chk(_d_modrm(0, 1));
	switch (cr) {
	case 0: /* ROL_RM8 */
		_bb("ROL_RM8");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 1: /* ROR_RM8 */
		_bb("ROR_RM8");
		SPRINTF(dop, "ROR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 2: /* RCL_RM8 */
		_bb("RCL_RM8");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 3: /* RCR_RM8 */
		_bb("RCR_RM8");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 4: /* SHL_RM8 */
		_bb("SHL_RM8");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 5: /* SHR_RM8 */
		_bb("SHR_RM8");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8 */
		_bb("SAR_RM8");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_D1()
{
	_cb("INS_D1");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (cr) {
	case 0: /* ROL_RM32 */
		_bb("ROL_RM32");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 1: /* ROR_RM32 */
		_bb("ROR_RM32");
		SPRINTF(dop, "ROR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 2: /* RCL_RM32 */
		_bb("RCL_RM32");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 3: /* RCR_RM32 */
		_bb("RCR_RM32");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 4: /* SHL_RM32 */
		_bb("SHL_RM32");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 5: /* SHR_RM32 */
		_bb("SHR_RM32");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM32 */
		_bb("SAR_RM32");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,01", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_D2()
{
	_cb("INS_D2");
	_adv;
	_chk(_d_modrm(0, 1));
	switch (cr) {
	case 0: /* ROL_RM8_CL */
		_bb("ROL_RM8_CL");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 1: /* ROR_RM8_CL */
		_bb("ROR_RM8_CL");
		SPRINTF(dop, "ROR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 2: /* RCL_RM8_CL */
		_bb("RCL_RM8_CL");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 3: /* RCR_RM8_CL */
		_bb("RCR_RM8_CL");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 4: /* SHL_RM8_CL */
		_bb("SHL_RM8_CL");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 5: /* SHR_RM8_CL */
		_bb("SHR_RM8_CL");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8_CL */
		_bb("SAR_RM8_CL");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_D3()
{
	_cb("INS_D3");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (cr) {
	case 0: /* ROL_RM32_CL */
		_bb("ROL_RM32_CL");
		SPRINTF(dop, "ROL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 1: /* ROR_RM32_CL */
		_bb("ROR_RM32_CL");
		SPRINTF(dop, "ROR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 2: /* RCL_RM32_CL */
		_bb("RCL_RM32_CL");
		SPRINTF(dop, "RCL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 3: /* RCR_RM32_CL */
		_bb("RCR_RM32_CL");
		SPRINTF(dop, "RCR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 4: /* SHL_RM32_CL */
		_bb("SHL_RM32_CL");
		SPRINTF(dop, "SHL");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 5: /* SHR_RM32_CL */
		_bb("SHR_RM32_CL");
		SPRINTF(dop, "SHR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM32_CL */
		_bb("SAR_RM32_CL");
		SPRINTF(dop, "SAR");
		SPRINTF(dopr, "%s,CL", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void AAM()
{
	_cb("AAM");
	_adv;
	SPRINTF(dop, "AAM");
	_chk(_d_imm(1));
	if ((unsigned char)(cimm) != 0x0a) SPRINTF(dopr, "%02X", (unsigned char)(cimm));
	_ce;
}
static void AAD()
{
	_cb("AAD");
	_adv;
	SPRINTF(dop, "AAD");
	_chk(_d_imm(1));
	if ((unsigned char)(cimm) != 0x0a) SPRINTF(dopr, "%02X", (unsigned char)(cimm));
	_ce;
}
static void XLAT()
{
	_cb("XLAT");
	_adv;
	SPRINTF(dop, "XLATB");
	switch (_GetAddressSize) {
	case 2: SPRINTF(dopr, "%s:[BX+AL]", doverds);break;
	case 4: SPRINTF(dopr, "%s:[EBX+AL]", doverds);break;
	default:_impossible_;break;}
	_ce;
}
static void LOOPNZ_REL8()
{
	_cb("LOOPNZ_REL8");
	_adv;
	SPRINTF(dop, "LOOPNZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void LOOPZ_REL8()
{
	_cb("LOOPZ_REL8");
	_adv;
	SPRINTF(dop, "LOOPZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void LOOP_REL8()
{
	_cb("LOOP_REL8");
	_adv;
	SPRINTF(dop, "LOOP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void JCXZ_REL8()
{
	_cb("JCXZ_REL8");
	_adv;
	SPRINTF(dop, "JCXZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void IN_AL_I8()
{
	_cb("IN_AL_I8");
	_adv;
	SPRINTF(dop, "IN");
	_chk(_d_imm(1));
	SPRINTF(dopr, "AL,%02X", (unsigned char)(cimm));
	_ce;
}
static void IN_EAX_I8()
{
	_cb("IN_EAX_I8");
	_adv;
	SPRINTF(dop, "IN");
	_chk(_d_imm(1));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX,%02X", (unsigned char)(cimm));break;
	case 4: SPRINTF(dopr, "EAX,%02X", (unsigned char)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void OUT_I8_AL()
{
	_cb("OUT_I8_AL");
	_adv;
	SPRINTF(dop, "OUT");
	_chk(_d_imm(1));
	SPRINTF(dopr, "%02X,AL", (unsigned char)(cimm));
	_ce;
}
static void OUT_I8_EAX()
{
	_cb("OUT_I8_EAX");
	_adv;
	SPRINTF(dop, "OUT");
	_chk(_d_imm(1));
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "%02X,AX", (unsigned char)(cimm));break;
	case 4: SPRINTF(dopr, "%02X,EAX", (unsigned char)(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void CALL_REL32()
{
	_cb("CALL_REL32");
	_adv;
	SPRINTF(dop, "CALL");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JMP_REL32()
{
	_cb("JMP_REL32");
	_adv;
	SPRINTF(dop, "JMP");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JMP_PTR16_32()
{
	unsigned short newcs;
	unsigned int neweip;
	_cb("JMP_PTR16_32");
	_adv;
	SPRINTF(dop, "JMP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(2));
		neweip = (unsigned short)(cimm);
		_chk(_d_imm(2));
		newcs = (unsigned short)(cimm);
		SPRINTF(dopr, "%04X:%04X", newcs, (unsigned short)(neweip));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_newins_;
		_chk(_d_imm(4));
		neweip = (unsigned int)(cimm);
		_chk(_d_imm(2));
		newcs = (unsigned short)(cimm);
		SPRINTF(dopr, "%04X:%08X", newcs, (unsigned int)(neweip));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void JMP_REL8()
{
	_cb("JMP_REL8");
	_adv;
	SPRINTF(dop, "JMP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, (unsigned char)(cimm), 1);
	_ce;
}
static void IN_AL_DX()
{
	_cb("IN_AL_DX");
	_adv;
	SPRINTF(dop, "IN");
	SPRINTF(dopr, "AL,DX");
	_ce;
}
static void IN_EAX_DX()
{
	_cb("IN_EAX_DX");
	_adv;
	SPRINTF(dop, "IN");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "AX,DX");break;
	case 4: SPRINTF(dopr, "EAX,DX");break;
	default:_impossible_;break;}
	_ce;
}
static void OUT_DX_AL()
{
	_cb("OUT_DX_AL");
	_adv;
	SPRINTF(dop, "OUT");
	SPRINTF(dopr, "DX,AL");
	_ce;
}
static void OUT_DX_EAX()
{
	_cb("OUT_DX_EAX");
	_adv;
	SPRINTF(dop, "OUT");
	switch (_GetOperandSize) {
	case 2: SPRINTF(dopr, "DX,AX");break;
	case 4: SPRINTF(dopr, "DX,EAX");break;
	default:_impossible_;break;}
	_ce;
}
static void PREFIX_LOCK()
{
	_cb("PREFIX_LOCK");
	_adv;
	SPRINTF(dop, "LOCK:");
	_ce;
}
static void PREFIX_REPNZ()
{
	_cb("PREFIX_REPNZ");
	_adv;
	SPRINTF(dop, "REPNZ:");
	_ce;
}
static void PREFIX_REPZ()
{
	_cb("PREFIX_REPZ");
	_adv;
	SPRINTF(dop, "REPZ:");
	_ce;
}
static void HLT()
{
	_cb("HLT");
	_adv;
	SPRINTF(dop, "HLT");
	_ce;
}
static void CMC()
{
	_cb("CMC");
	_adv;
	SPRINTF(dop, "CMC");
	_ce;
}
static void INS_F6()
{
	_cb("INS_F6");
	_adv;
	_chk(_d_modrm(0, 1));
	switch (cr) {
	case 0: /* TEST_RM8_I8 */
		_bb("TEST_RM8_I8");
		SPRINTF(dop, "TEST");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 1: /* UndefinedOpcode */
		_bb("ModRM_REG(1)");
		_chk(UndefinedOpcode());
		_be;break;
	case 2: /* NOT_RM8 */
		_bb("NOT_RM8");
		SPRINTF(dop, "NOT");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 3: /* NEG_RM8 */
		_bb("NEG_RM8");
		SPRINTF(dop, "NEG");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 4: /* MUL_RM8 */
		_bb("MUL_RM8");
		SPRINTF(dop, "MUL");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 5: /* IMUL_RM8 */
		_bb("IMUL_RM8");
		SPRINTF(dop, "IMUL");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 6: /* DIV_RM8 */
		_bb("DIV_RM8");
		SPRINTF(dop, "DIV");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 7: /* IDIV_RM8 */
		_bb("IDIV_RM8");
		SPRINTF(dop, "IDIV");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_F7()
{
	_cb("INS_F7");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (cr) {
	case 0: /* TEST_RM32_I32 */
		_bb("TEST_RM32_I32");
		SPRINTF(dop, "TEST");
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "%s,%04X", drm, (unsigned short)(cimm));break;
		case 4: SPRINTF(dopr, "%s,%08X", drm, (unsigned int)(cimm));break;
		default:_impossible_;break;}
		_be;break;
	case 1: /* UndefinedOpcode */
		_bb("ModRM_REG(1)");
		_chk(UndefinedOpcode());
		_be;break;
	case 2: /* NOT_RM32 */
		_bb("NOT_RM32");
		SPRINTF(dop, "NOT");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 3: /* NEG_RM32 */
		_bb("NEG_RM16");
		SPRINTF(dop, "NEG");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 4: /* MUL_RM32 */
		_bb("MUL_RM32");
		SPRINTF(dop, "MUL");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 5: /* IMUL_RM32 */
		_bb("IMUL_RM32");
		SPRINTF(dop, "IMUL");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 6: /* DIV_RM32 */
		_bb("DIV_RM32");
		SPRINTF(dop, "DIV");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 7: /* IDIV_RM32 */
		_bb("IDIV_RM32");
		SPRINTF(dop, "IDIV");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void CLC()
{
	_cb("CLC");
	_adv;
	SPRINTF(dop, "CLC");
	_ce;
}
static void STC()
{
	_cb("STC");
	_adv;
	SPRINTF(dop, "STC");
	_ce;
}
static void CLI()
{
	_cb("CLI");
	_adv;
	SPRINTF(dop, "CLI");
	_ce;
}
static void STI()
{
	_cb("STI");
	_adv;
	SPRINTF(dop, "STI");
	_ce;
}
static void CLD()
{
	_cb("CLD");
	_adv;
	SPRINTF(dop, "CLD");
	_ce;
}
static void STD()
{
	_cb("STD");
	_adv;
	SPRINTF(dop, "STD");
	_ce;
}
static void INS_FE()
{
	_cb("INS_FE");
	_adv;
	_chk(_d_modrm(0, 1));
	switch (cr) {
	case 0: /* INC_RM8 */
		_bb("INC_RM8");
		SPRINTF(dop, "INC");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 1: /* DEC_RM8 */
		_bb("DEC_RM8");
		SPRINTF(dop, "DEC");
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_FF()
{
	char dptr[0x100];
	unsigned char oldiop;
	unsigned char modrm;
	_cb("INS_FF");
	_adv;
	oldiop = iop;
	_chk(_d_code((unsigned char *)(&modrm), 1));
	iop = oldiop;
	switch (_GetModRM_REG(modrm)) {
	case 0: /* INC_RM32 */
		_bb("INC_RM32");
		SPRINTF(dop, "INC");
		_chk(_d_modrm(0, _GetOperandSize));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 1: /* DEC_RM32 */
		_bb("DEC_RM32");
		SPRINTF(dop, "DEC");
		_chk(_d_modrm(0, _GetOperandSize));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 2: /* CALL_RM32 */
		_bb("CALL_RM32");
		SPRINTF(dop, "CALL");
		_chk(_d_modrm(0, _GetOperandSize));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 3: /* CALL_M16_32 */
		_bb("CALL_M16_32");
		SPRINTF(dop, "CALL");
		_chk(_d_modrm(9, _GetOperandSize + 2));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dptr, "WORD PTR ");break;
		case 4: SPRINTF(dptr, "DWORD PTR ");break;
		default:_impossible_;break;}
		SPRINTF(dopr, "FAR %s%s", dptr, drm);
		_be;break;
	case 4: /* JMP_RM32 */
		_bb("JMP_RM32");
		SPRINTF(dop, "JMP");
		_chk(_d_modrm(0, _GetOperandSize));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 5: /* JMP_M16_32 */
		_bb("JMP_M16_32");
		SPRINTF(dop, "JMP");
		_chk(_d_modrm(9, _GetOperandSize + 2));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dptr, "WORD PTR ");break;
		case 4: SPRINTF(dptr, "DWORD PTR ");break;
		default:_impossible_;break;}
		SPRINTF(dopr, "FAR %s%s", dptr, drm);
		_be;break;
	case 6: /* PUSH_RM32 */
		_bb("PUSH_RM32");
		SPRINTF(dop, "PUSH");
		_chk(_d_modrm(0, _GetOperandSize));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 7: /* UndefinedOpcode */
		_bb("ModRM_REG(7)");
		_chk(UndefinedOpcode());
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void _d_modrm_creg()
{
	_cb("_d_modrm_creg");
	_chk(_kdf_modrm(9, 4));
	if (flagmem) {
		_bb("flagmem(1)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	switch (cr) {
	case 0: SPRINTF(dr, "CR0");break;
	case 2: SPRINTF(dr, "CR2");break;
	case 3: SPRINTF(dr, "CR3");break;;
	default:SPRINTF(dr, "<ERROR>");break;}
	_ce;
}
static void _d_modrm_dreg()
{
	_cb("_d_modrm_dreg");
	_chk(_kdf_modrm(9, 4));
	if (flagmem) {
		_bb("flagmem(1)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	switch (cr) {
	case 0: SPRINTF(dr, "DR0");break;
	case 1: SPRINTF(dr, "DR1");break;
	case 2: SPRINTF(dr, "DR2");break;
	case 3: SPRINTF(dr, "DR3");break;
	case 6: SPRINTF(dr, "DR6");break;
	case 7: SPRINTF(dr, "DR7");break;
	default:SPRINTF(dr, "<ERROR>");break;}
	_ce;
}
static void _d_modrm_treg()
{
	_cb("_d_modrm_treg");
	_chk(_kdf_modrm(9, 4));
	if (flagmem) {
		_bb("flagmem(1)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	switch (cr) {
	case 6: SPRINTF(dr, "TR6");break;
	case 7: SPRINTF(dr, "TR7");break;
	default:SPRINTF(dr, "<ERROR>");break;}
	_ce;
}

static void INS_0F_00()
{
	unsigned char modrm, oldiop;
	_cb("INS_0F_00");
	_adv;
	oldiop = iop;
	_chk(_d_code((unsigned char *)(&modrm), 1));
	iop = oldiop;
	switch (_GetModRM_REG(modrm)) {
	case 0: /* SLDT_RM16 */
		_bb("SLDT_RM16");
		SPRINTF(dop, "SLDT");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 1: /* STR_RM16 */
		_bb("STR_RM16");
		SPRINTF(dop, "STR");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 2: /* LLDT_RM16 */
		_bb("LLDT_RM16");
		SPRINTF(dop, "LLDT");
		_chk(_d_modrm(0, 2));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 3: /* LTR_RM16 */
		_bb("LTR_RM16");
		SPRINTF(dop, "LTR");
		_chk(_d_modrm(0, 2));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 4: /* VERR_RM16 */
		_bb("VERR_RM16");
		SPRINTF(dop, "VERR");
		_chk(_d_modrm(0, 2));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 5: /* VERW_RM16 */
		_bb("VERW_RM16");
		SPRINTF(dop, "VERW");
		_chk(_d_modrm(0, 2));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 6: _bb("ModRM_REG(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_0F_01()
{
	unsigned char modrm, oldiop;
	_cb("INS_0F_01");
	_adv;
	oldiop = iop;
	_chk(_d_code((unsigned char *)(&modrm), 1));
	iop = oldiop;
	switch (_GetModRM_REG(modrm)) {
	case 0: /* SGDT_M32_16 */
		_bb("SGDT_M32_16");
		SPRINTF(dop, "SGDT");
		_chk(_d_modrm(0, 6));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "WORD PTR %s", drm);break;
		case 4: SPRINTF(dopr, "DWORD PTR %s", drm);break;
		default:_impossible_;break;}
		_be;break;
	case 1: /* SIDT_M32_16 */
		_bb("SIDT_M32_16");
		SPRINTF(dop, "SIDT");
		_chk(_d_modrm(0, 6));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "WORD PTR %s", drm);break;
		case 4: SPRINTF(dopr, "DWORD PTR %s", drm);break;
		default:_impossible_;break;}
		_be;break;
	case 2: /* LGDT_M32_16 */
		_bb("LGDT_M32_16");
		SPRINTF(dop, "LGDT");
		_chk(_d_modrm(0, 6));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "WORD PTR %s", drm);break;
		case 4: SPRINTF(dopr, "DWORD PTR %s", drm);break;
		default:_impossible_;break;}
		_be;break;
	case 3: /* LIDT_M32_16 */
		_bb("LIDT_M32_16");
		SPRINTF(dop, "LIDT");
		_chk(_d_modrm(0, 6));
		if (!flagmem) {
			_bb("flagmem(0)");
			SPRINTF(drm, "<ERROR>");
			_be;
		}
		switch (_GetOperandSize) {
		case 2: SPRINTF(dopr, "WORD PTR %s", drm);break;
		case 4: SPRINTF(dopr, "DWORD PTR %s", drm);break;
		default:_impossible_;break;}
		_be;break;
	case 4: /* SMSW_RM16 */
		_bb("SMSW_RM16");
		SPRINTF(dop, "SMSW");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 5: _bb("ModRM_REG(5)");_chk(UndefinedOpcode());_be;break;
	case 6: /* LMSW_RM16 */
		_bb("LMSW_RM16");
		SPRINTF(dop, "LMSW");
		_chk(_d_modrm(0, 2));
		SPRINTF(dopr, "%s", drm);
		_be;break;
	case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void LAR_R32_RM32()
{
	_cb("LAR_R32_RM32");
	_adv;
	SPRINTF(dop, "LAR");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void LSL_R32_RM32()
{
	_cb("LSL_R32_RM32");
	_adv;
	SPRINTF(dop, "LSL");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void CLTS()
{
	_cb("CLTS");
	_adv;
	SPRINTF(dop, "CLTS");
	_ce;
}
static void WBINVD() {}
static void MOV_R32_CR()
{
	_cb("MOV_R32_CR");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_creg());
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_R32_DR()
{
	_cb("MOV_R32_DR");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_dreg());
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_CR_R32()
{
	_cb("MOV_CR_R32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_creg());
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_DR_R32()
{
	_cb("MOV_DR_R32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_dreg());
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_R32_TR()
{
	_cb("MOV_R32_TR");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_treg());
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void MOV_TR_R32()
{
	_cb("MOV_TR_R32");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_treg());
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void WRMSR() {}
static void RDMSR() {}
static void JO_REL32()
{
	_cb("JO_REL32");
	_adv;
	SPRINTF(dop, "JO");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNO_REL32()
{
	_cb("JNO_REL32");
	_adv;
	SPRINTF(dop, "JNO");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JC_REL32()
{
	_cb("JC_REL32");
	_adv;
	SPRINTF(dop, "JC");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNC_REL32()
{
	_cb("JNC_REL32");
	_adv;
	SPRINTF(dop, "JNC");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JZ_REL32()
{
	_cb("JZ_REL32");
	_adv;
	SPRINTF(dop, "JZ");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNZ_REL32()
{
	_cb("JNZ_REL32");
	_adv;
	SPRINTF(dop, "JNZ");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNA_REL32()
{
	_cb("JNA_REL32");
	_adv;
	SPRINTF(dop, "JNA");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JA_REL32()
{
	_cb("JA_REL32");
	_adv;
	SPRINTF(dop, "JA");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JS_REL32()
{
	_cb("JS_REL32");
	_adv;
	SPRINTF(dop, "JS");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNS_REL32()
{
	_cb("JNS_REL32");
	_adv;
	SPRINTF(dop, "JNS");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JP_REL32()
{
	_cb("JP_REL32");
	_adv;
	SPRINTF(dop, "JP");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNP_REL32()
{
	_cb("JNP_REL32");
	_adv;
	SPRINTF(dop, "JNP");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JL_REL32()
{
	_cb("JL_REL32");
	_adv;
	SPRINTF(dop, "JL");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNL_REL32()
{
	_cb("JNL_REL32");
	_adv;
	SPRINTF(dop, "JNL");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JNG_REL32()
{
	_cb("JNG_REL32");
	_adv;
	SPRINTF(dop, "JNG");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void JG_REL32()
{
	_cb("JG_REL32");
	_adv;
	SPRINTF(dop, "JG");
	_chk(_d_imm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: SPRINTFSI(dopr, (unsigned short)(cimm), 2);break;
	case 4: SPRINTFSI(dopr, (unsigned int)(cimm), 4);break;
	default:_impossible_;break;}
	_ce;
}
static void SETO_RM8()
{
	_cb("SETO_RM8");
	_adv;
	SPRINTF(dop, "SETO");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNO_RM8()
{
	_cb("SETO_RM8");
	_adv;
	SPRINTF(dop, "SETNO");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETC_RM8()
{
	_cb("SETC_RM8");
	_adv;
	SPRINTF(dop, "SETC");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNC_RM8()
{
	_cb("SETNC_RM8");
	_adv;
	SPRINTF(dop, "SETNC");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETZ_RM8()
{
	_cb("SETZ_RM8");
	_adv;
	SPRINTF(dop, "SETZ");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNZ_RM8()
{
	_cb("SETNZ_RM8");
	_adv;
	SPRINTF(dop, "SETNZ");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNA_RM8()
{
	_cb("SETNA_RM8");
	_adv;
	SPRINTF(dop, "SETNA");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETA_RM8()
{
	_cb("SETA_RM8");
	_adv;
	SPRINTF(dop, "SETA");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETS_RM8()
{
	_cb("SETS_RM8");
	_adv;
	SPRINTF(dop, "SETS");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNS_RM8()
{
	_cb("SETNS_RM8");
	_adv;
	SPRINTF(dop, "SETNS");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETP_RM8()
{
	_cb("SETP_RM8");
	_adv;
	SPRINTF(dop, "SETP");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNP_RM8()
{
	_cb("SETNP_RM8");
	_adv;
	SPRINTF(dop, "SETNP");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETL_RM8()
{
	_cb("SETL_RM8");
	_adv;
	SPRINTF(dop, "SETL");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNL_RM8()
{
	_cb("SETNL_RM8");
	_adv;
	SPRINTF(dop, "SETNL");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETNG_RM8()
{
	_cb("SETNG_RM8");
	_adv;
	SPRINTF(dop, "SETNG");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void SETG_RM8()
{
	_cb("SETG_RM8");
	_adv;
	SPRINTF(dop, "SETG");
	_chk(_d_modrm(0, 1));
	SPRINTF(dopr, "%s", drm);
	_ce;
}
static void PUSH_FS()
{
	_cb("PUSH_FS");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "FS");
	_ce;
}
static void POP_FS()
{
	_cb("POP_FS");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "FS");
	_ce;
}
static void CPUID() {}
static void BT_RM32_R32()
{
	_cb("BT_RM32_R32");
	_adv;
	SPRINTF(dop, "BT");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SHLD_RM32_R32_I8()
{
	_cb("SHLD_RM32_R32_I8");
	_adv;
	SPRINTF(dop, "SHLD");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_d_imm(1));
	SPRINTF(dopr, "%s,%s,%02X", drm, dr, (unsigned char)(cimm));
	_ce;
}
static void SHLD_RM32_R32_CL()
{
	_cb("SHLD_RM32_R32_CL");
	_adv;
	SPRINTF(dop, "SHLD");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s,CL", drm, dr);
	_ce;
}
static void PUSH_GS()
{
	_cb("PUSH_GS");
	_adv;
	SPRINTF(dop, "PUSH");
	SPRINTF(dopr, "GS");
	_ce;
}
static void POP_GS()
{
	_cb("POP_GS");
	_adv;
	SPRINTF(dop, "POP");
	SPRINTF(dopr, "GS");
	_ce;
}
static void RSM() {}
static void BTS_RM32_R32()
{
	_cb("BTS_RM32_R32");
	_adv;
	SPRINTF(dop, "BTS");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void SHRD_RM32_R32_I8()
{
	_cb("SHRD_RM32_R32_I8");
	_adv;
	SPRINTF(dop, "SHRD");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_d_imm(1));
	SPRINTF(dopr, "%s,%s,%02X", drm, dr, (unsigned char)(cimm));
	_ce;
}
static void SHRD_RM32_R32_CL()
{
	_cb("SHRD_RM32_R32_CL");
	_adv;
	SPRINTF(dop, "SHRD");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s,CL", drm, dr);
	_ce;
}
static void IMUL_R32_RM32()
{
	_cb("IMUL_R32_RM32");
	_adv;
	SPRINTF(dop, "IMUL");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void LSS_R32_M16_32()
{
	_cb("LSS_R32_M16_32");
	_adv;
	SPRINTF(dop, "LSS");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void BTR_RM32_R32()
{
	_cb("BTR_RM32_R32");
	_adv;
	SPRINTF(dop, "BTR");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void LFS_R32_M16_32()
{
	_cb("LFS_R32_M16_32");
	_adv;
	SPRINTF(dop, "LFS");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void LGS_R32_M16_32()
{
	_cb("LGS_R32_M16_32");
	_adv;
	SPRINTF(dop, "LGS");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	}
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOVZX_R32_RM8()
{
	char dptr[0x100];
	_cb("MOVZX_R32_RM8");
	_adv;
	SPRINTF(dop, "MOVZX");
	_chk(_d_modrm(_GetOperandSize, 1));
	if (flagmem) SPRINTF(dptr, "BYTE PTR ");
	else dptr[0] = 0;
	SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
	_ce;
}
static void MOVZX_R32_RM16()
{
	char dptr[0x100];
	_cb("MOVZX_R32_RM16");
	_adv;
	SPRINTF(dop, "MOVZX");
	_chk(_d_modrm(4, 2));
	if (flagmem) SPRINTF(dptr, "WORD PTR ");
	else dptr[0] = 0;
	SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
	_ce;
}
static void INS_0F_BA()
{
	unsigned char modrm, oldiop;
	_cb("INS_0F_BA");
	_adv;
	oldiop = iop;
	_chk(_d_code((unsigned char *)(&modrm), 1));
	iop = oldiop;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (cr) {
	case 0: _bb("cr(0)");_chk(UndefinedOpcode());_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: /* BT_RM32_I8 */
		_bb("BT_RM32_I8");
		SPRINTF(dop, "BT");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 5: /* BTS_RM32_I8 */
		_bb("BTS_RM32_I8");
		SPRINTF(dop, "BTS");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 6: /* BTR_RM32_I8 */
		_bb("BTR_RM32_I8");
		SPRINTF(dop, "BTR");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	case 7: /* BTC_RM32_I8 */
		_bb("BTC_RM32_I8");
		SPRINTF(dop, "BTC");
		_chk(_d_imm(1));
		SPRINTF(dopr, "%s,%02X", drm, (unsigned char)(cimm));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void BTC_RM32_R32()
{
	_cb("BTC_RM32_R32");
	_adv;
	SPRINTF(dop, "BTC");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", drm, dr);
	_ce;
}
static void BSF_R32_RM32()
{
	_cb("BSF_R32_RM32");
	_adv;
	SPRINTF(dop, "BSF");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void BSR_R32_RM32()
{
	_cb("BSR_R32_RM32");
	_adv;
	SPRINTF(dop, "BSR");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOVSX_R32_RM8()
{
	char dptr[0x100];
	_cb("MOVSX_R32_RM8");
	_adv;
	SPRINTF(dop, "MOVSX");
	_chk(_d_modrm(_GetOperandSize, 1));
	if (flagmem) SPRINTF(dptr, "BYTE PTR ");
	else dptr[0] = 0;
	SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
	_ce;
}
static void MOVSX_R32_RM16()
{
	char dptr[0x100];
	_cb("MOVSX_R32_RM16");
	_adv;
	SPRINTF(dop, "MOVSX");
	_chk(_d_modrm(4, 2));
	if (flagmem) SPRINTF(dptr, "WORD PTR ");
	else dptr[0] = 0;
	SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
	_ce;
}
static void QDX()
{
	_cb("QDX");
	_adv;
	SPRINTF(dop, "QDX");
	_chk(_d_imm(1));
	SPRINTF(dopr, "%02X", (unsigned char)(cimm));
	_ce;
}

static unsigned char flaginit = 0;

unsigned char dasm32(char *stmt, unsigned char *rcode, unsigned char flag32) {
	int i;
	unsigned char opcode, oldiop;
#if DASM_TRACE == 1
	utilsTraceInit(&trace);
#endif
	if (!flaginit) {
		dtable[0x00] = ADD_RM8_R8;
		dtable[0x01] = ADD_RM32_R32;
		dtable[0x02] = ADD_R8_RM8;
		dtable[0x03] = ADD_R32_RM32;
		dtable[0x04] = ADD_AL_I8;
		dtable[0x05] = ADD_EAX_I32;
		dtable[0x06] = PUSH_ES;
		dtable[0x07] = POP_ES;
		dtable[0x08] = OR_RM8_R8;
		dtable[0x09] = OR_RM32_R32;
		dtable[0x0a] = OR_R8_RM8;
		dtable[0x0b] = OR_R32_RM32;
		dtable[0x0c] = OR_AL_I8;
		dtable[0x0d] = OR_EAX_I32;
		dtable[0x0e] = PUSH_CS;
		dtable[0x0f] = INS_0F;//
		dtable[0x10] = ADC_RM8_R8;
		dtable[0x11] = ADC_RM32_R32;
		dtable[0x12] = ADC_R8_RM8;
		dtable[0x13] = ADC_R32_RM32;
		dtable[0x14] = ADC_AL_I8;
		dtable[0x15] = ADC_EAX_I32;
		dtable[0x16] = PUSH_SS;
		dtable[0x17] = POP_SS;
		dtable[0x18] = SBB_RM8_R8;
		dtable[0x19] = SBB_RM32_R32;
		dtable[0x1a] = SBB_R8_RM8;
		dtable[0x1b] = SBB_R32_RM32;
		dtable[0x1c] = SBB_AL_I8;
		dtable[0x1d] = SBB_EAX_I32;
		dtable[0x1e] = PUSH_DS;
		dtable[0x1f] = POP_DS;
		dtable[0x20] = AND_RM8_R8;
		dtable[0x21] = AND_RM32_R32;
		dtable[0x22] = AND_R8_RM8;
		dtable[0x23] = AND_R32_RM32;
		dtable[0x24] = AND_AL_I8;
		dtable[0x25] = AND_EAX_I32;
		dtable[0x26] = PREFIX_ES;
		dtable[0x27] = DAA;
		dtable[0x28] = SUB_RM8_R8;
		dtable[0x29] = SUB_RM32_R32;
		dtable[0x2a] = SUB_R8_RM8;
		dtable[0x2b] = SUB_R32_RM32;
		dtable[0x2c] = SUB_AL_I8;
		dtable[0x2d] = SUB_EAX_I32;
		dtable[0x2e] = PREFIX_CS;
		dtable[0x2f] = DAS;
		dtable[0x30] = XOR_RM8_R8;
		dtable[0x31] = XOR_RM32_R32;
		dtable[0x32] = XOR_R8_RM8;
		dtable[0x33] = XOR_R32_RM32;
		dtable[0x34] = XOR_AL_I8;
		dtable[0x35] = XOR_EAX_I32;
		dtable[0x36] = PREFIX_SS;
		dtable[0x37] = AAA;
		dtable[0x38] = CMP_RM8_R8;
		dtable[0x39] = CMP_RM32_R32;
		dtable[0x3a] = CMP_R8_RM8;
		dtable[0x3b] = CMP_R32_RM32;
		dtable[0x3c] = CMP_AL_I8;
		dtable[0x3d] = CMP_EAX_I32;
		dtable[0x3e] = PREFIX_DS;
		dtable[0x3f] = AAS;
		dtable[0x40] = INC_EAX;
		dtable[0x41] = INC_ECX;
		dtable[0x42] = INC_EDX;
		dtable[0x43] = INC_EBX;
		dtable[0x44] = INC_ESP;
		dtable[0x45] = INC_EBP;
		dtable[0x46] = INC_ESI;
		dtable[0x47] = INC_EDI;
		dtable[0x48] = DEC_EAX;
		dtable[0x49] = DEC_ECX;
		dtable[0x4a] = DEC_EDX;
		dtable[0x4b] = DEC_EBX;
		dtable[0x4c] = DEC_ESP;
		dtable[0x4d] = DEC_EBP;
		dtable[0x4e] = DEC_ESI;
		dtable[0x4f] = DEC_EDI;
		dtable[0x50] = PUSH_EAX;
		dtable[0x51] = PUSH_ECX;
		dtable[0x52] = PUSH_EDX;
		dtable[0x53] = PUSH_EBX;
		dtable[0x54] = PUSH_ESP;
		dtable[0x55] = PUSH_EBP;
		dtable[0x56] = PUSH_ESI;
		dtable[0x57] = PUSH_EDI;
		dtable[0x58] = POP_EAX;
		dtable[0x59] = POP_ECX;
		dtable[0x5a] = POP_EDX;
		dtable[0x5b] = POP_EBX;
		dtable[0x5c] = POP_ESP;
		dtable[0x5d] = POP_EBP;
		dtable[0x5e] = POP_ESI;
		dtable[0x5f] = POP_EDI;
		dtable[0x60] = PUSHA;//
		dtable[0x61] = POPA;//
		dtable[0x62] = BOUND_R16_M16_16;//
		dtable[0x63] = ARPL_RM16_R16;//
		dtable[0x64] = PREFIX_FS;//
		dtable[0x65] = PREFIX_GS;//
		dtable[0x66] = PREFIX_OprSize;//
		dtable[0x67] = PREFIX_AddrSize;//
		dtable[0x68] = PUSH_I32;//
		dtable[0x69] = IMUL_R32_RM32_I32;//
		dtable[0x6a] = PUSH_I8;//
		dtable[0x6b] = IMUL_R32_RM32_I8;//
		dtable[0x6c] = INSB;//
		dtable[0x6d] = INSW;//
		dtable[0x6e] = OUTSB;
		dtable[0x6f] = OUTSW;
		dtable[0x70] = JO_REL8;
		dtable[0x71] = JNO_REL8;
		dtable[0x72] = JC_REL8;
		dtable[0x73] = JNC_REL8;
		dtable[0x74] = JZ_REL8;
		dtable[0x75] = JNZ_REL8;
		dtable[0x76] = JNA_REL8;
		dtable[0x77] = JA_REL8;
		dtable[0x78] = JS_REL8;
		dtable[0x79] = JNS_REL8;
		dtable[0x7a] = JP_REL8;
		dtable[0x7b] = JNP_REL8;
		dtable[0x7c] = JL_REL8;
		dtable[0x7d] = JNL_REL8;
		dtable[0x7e] = JNG_REL8;
		dtable[0x7f] = JG_REL8;
		dtable[0x80] = INS_80;
		dtable[0x81] = INS_81;
		dtable[0x82] = UndefinedOpcode;
		dtable[0x83] = INS_83;
		dtable[0x84] = TEST_RM8_R8;
		dtable[0x85] = TEST_RM32_R32;
		dtable[0x86] = XCHG_RM8_R8;
		dtable[0x87] = XCHG_RM32_R32;
		dtable[0x88] = MOV_RM8_R8;
		dtable[0x89] = MOV_RM32_R32;
		dtable[0x8a] = MOV_R8_RM8;
		dtable[0x8b] = MOV_R32_RM32;
		dtable[0x8c] = MOV_RM16_SREG;
		dtable[0x8d] = LEA_R32_M32;
		dtable[0x8e] = MOV_SREG_RM16;
		dtable[0x8f] = INS_8F;
		dtable[0x90] = NOP;
		dtable[0x91] = XCHG_ECX_EAX;
		dtable[0x92] = XCHG_EDX_EAX;
		dtable[0x93] = XCHG_EBX_EAX;
		dtable[0x94] = XCHG_ESP_EAX;
		dtable[0x95] = XCHG_EBP_EAX;
		dtable[0x96] = XCHG_ESI_EAX;
		dtable[0x97] = XCHG_EDI_EAX;
		dtable[0x98] = CBW;
		dtable[0x99] = CWD;
		dtable[0x9a] = CALL_PTR16_32;
		dtable[0x9b] = WAIT;
		dtable[0x9c] = PUSHF;
		dtable[0x9d] = POPF;
		dtable[0x9e] = SAHF;
		dtable[0x9f] = LAHF;
		dtable[0xa0] = MOV_AL_MOFFS8;
		dtable[0xa1] = MOV_EAX_MOFFS32;
		dtable[0xa2] = MOV_MOFFS8_AL;
		dtable[0xa3] = MOV_MOFFS32_EAX;
		dtable[0xa4] = MOVSB;
		dtable[0xa5] = MOVSW;
		dtable[0xa6] = CMPSB;
		dtable[0xa7] = CMPSW;
		dtable[0xa8] = TEST_AL_I8;
		dtable[0xa9] = TEST_EAX_I32;
		dtable[0xaa] = STOSB;
		dtable[0xab] = STOSW;
		dtable[0xac] = LODSB;
		dtable[0xad] = LODSW;
		dtable[0xae] = SCASB;
		dtable[0xaf] = SCASW;
		dtable[0xb0] = MOV_AL_I8;
		dtable[0xb1] = MOV_CL_I8;
		dtable[0xb2] = MOV_DL_I8;
		dtable[0xb3] = MOV_BL_I8;
		dtable[0xb4] = MOV_AH_I8;
		dtable[0xb5] = MOV_CH_I8;
		dtable[0xb6] = MOV_DH_I8;
		dtable[0xb7] = MOV_BH_I8;
		dtable[0xb8] = MOV_EAX_I32;
		dtable[0xb9] = MOV_ECX_I32;
		dtable[0xba] = MOV_EDX_I32;
		dtable[0xbb] = MOV_EBX_I32;
		dtable[0xbc] = MOV_ESP_I32;
		dtable[0xbd] = MOV_EBP_I32;
		dtable[0xbe] = MOV_ESI_I32;
		dtable[0xbf] = MOV_EDI_I32;
		dtable[0xc0] = INS_C0;//
		dtable[0xc1] = INS_C1;//
		dtable[0xc2] = RET_I16;
		dtable[0xc3] = RET;
		dtable[0xc4] = LES_R32_M16_32;
		dtable[0xc5] = LDS_R32_M16_32;
		dtable[0xc6] = INS_C6;
		dtable[0xc7] = INS_C7;
		dtable[0xc8] = ENTER;//
		dtable[0xc9] = LEAVE;//
		dtable[0xca] = RETF_I16;
		dtable[0xcb] = RETF;
		dtable[0xcc] = INT3;
		dtable[0xcd] = INT_I8;
		dtable[0xce] = INTO;
		dtable[0xcf] = IRET;
		dtable[0xd0] = INS_D0;
		dtable[0xd1] = INS_D1;
		dtable[0xd2] = INS_D2;
		dtable[0xd3] = INS_D3;
		dtable[0xd4] = AAM;
		dtable[0xd5] = AAD;
		dtable[0xd6] = UndefinedOpcode;
		dtable[0xd7] = XLAT;
		dtable[0xd8] = UndefinedOpcode;
		dtable[0xd9] = UndefinedOpcode;
		dtable[0xda] = UndefinedOpcode;
		dtable[0xdb] = UndefinedOpcode;
		dtable[0xdc] = UndefinedOpcode;
		dtable[0xdd] = UndefinedOpcode;
		dtable[0xde] = UndefinedOpcode;
		dtable[0xdf] = UndefinedOpcode;
		dtable[0xe0] = LOOPNZ_REL8;
		dtable[0xe1] = LOOPZ_REL8;
		dtable[0xe2] = LOOP_REL8;
		dtable[0xe3] = JCXZ_REL8;
		dtable[0xe4] = IN_AL_I8;
		dtable[0xe5] = IN_EAX_I8;
		dtable[0xe6] = OUT_I8_AL;
		dtable[0xe7] = OUT_I8_EAX;
		dtable[0xe8] = CALL_REL32;
		dtable[0xe9] = JMP_REL32;
		dtable[0xea] = JMP_PTR16_32;
		dtable[0xeb] = JMP_REL8;
		dtable[0xec] = IN_AL_DX;
		dtable[0xed] = IN_EAX_DX;
		dtable[0xee] = OUT_DX_AL;
		dtable[0xef] = OUT_DX_EAX;
		dtable[0xf0] = PREFIX_LOCK;
		dtable[0xf1] = QDX;
		dtable[0xf2] = PREFIX_REPNZ;
		dtable[0xf3] = PREFIX_REPZ;
		dtable[0xf4] = HLT;
		dtable[0xf5] = CMC;
		dtable[0xf6] = INS_F6;
		dtable[0xf7] = INS_F7;
		dtable[0xf8] = CLC;
		dtable[0xf9] = STC;
		dtable[0xfa] = CLI;
		dtable[0xfb] = STI;
		dtable[0xfc] = CLD;
		dtable[0xfd] = STD;
		dtable[0xfe] = INS_FE;
		dtable[0xff] = INS_FF;
		dtable_0f[0x00] = INS_0F_00;
		dtable_0f[0x01] = INS_0F_01;
		dtable_0f[0x02] = LAR_R32_RM32;
		dtable_0f[0x03] = LSL_R32_RM32;
		dtable_0f[0x04] = UndefinedOpcode;
		dtable_0f[0x05] = UndefinedOpcode;
		dtable_0f[0x06] = CLTS;
		dtable_0f[0x07] = UndefinedOpcode;
		dtable_0f[0x08] = UndefinedOpcode;
		dtable_0f[0x09] = WBINVD;
		dtable_0f[0x0a] = UndefinedOpcode;
		dtable_0f[0x0b] = UndefinedOpcode;
		dtable_0f[0x0c] = UndefinedOpcode;
		dtable_0f[0x0d] = UndefinedOpcode;
		dtable_0f[0x0e] = UndefinedOpcode;
		dtable_0f[0x0f] = UndefinedOpcode;
		dtable_0f[0x10] = UndefinedOpcode;
		dtable_0f[0x11] = UndefinedOpcode;
		dtable_0f[0x12] = UndefinedOpcode;
		dtable_0f[0x13] = UndefinedOpcode;
		dtable_0f[0x14] = UndefinedOpcode;
		dtable_0f[0x15] = UndefinedOpcode;
		dtable_0f[0x16] = UndefinedOpcode;
		dtable_0f[0x17] = UndefinedOpcode;
		dtable_0f[0x18] = UndefinedOpcode;
		dtable_0f[0x19] = UndefinedOpcode;
		dtable_0f[0x1a] = UndefinedOpcode;
		dtable_0f[0x1b] = UndefinedOpcode;
		dtable_0f[0x1c] = UndefinedOpcode;
		dtable_0f[0x1d] = UndefinedOpcode;
		dtable_0f[0x1e] = UndefinedOpcode;
		dtable_0f[0x1f] = UndefinedOpcode;
		dtable_0f[0x20] = MOV_R32_CR;
		dtable_0f[0x21] = MOV_R32_DR;
		dtable_0f[0x22] = MOV_CR_R32;
		dtable_0f[0x23] = MOV_DR_R32;
		dtable_0f[0x24] = MOV_R32_TR;
		dtable_0f[0x25] = UndefinedOpcode;
		dtable_0f[0x26] = MOV_TR_R32;
		dtable_0f[0x27] = UndefinedOpcode;
		dtable_0f[0x28] = UndefinedOpcode;
		dtable_0f[0x29] = UndefinedOpcode;
		dtable_0f[0x2a] = UndefinedOpcode;
		dtable_0f[0x2b] = UndefinedOpcode;
		dtable_0f[0x2c] = UndefinedOpcode;
		dtable_0f[0x2d] = UndefinedOpcode;
		dtable_0f[0x2e] = UndefinedOpcode;
		dtable_0f[0x2f] = UndefinedOpcode;
		dtable_0f[0x30] = WRMSR;
		dtable_0f[0x31] = UndefinedOpcode;
		dtable_0f[0x32] = RDMSR;
		dtable_0f[0x33] = UndefinedOpcode;
		dtable_0f[0x34] = UndefinedOpcode;
		dtable_0f[0x35] = UndefinedOpcode;
		dtable_0f[0x36] = UndefinedOpcode;
		dtable_0f[0x37] = UndefinedOpcode;
		dtable_0f[0x38] = UndefinedOpcode;
		dtable_0f[0x39] = UndefinedOpcode;
		dtable_0f[0x3a] = UndefinedOpcode;
		dtable_0f[0x3b] = UndefinedOpcode;
		dtable_0f[0x3c] = UndefinedOpcode;
		dtable_0f[0x3d] = UndefinedOpcode;
		dtable_0f[0x3e] = UndefinedOpcode;
		dtable_0f[0x3f] = UndefinedOpcode;
		dtable_0f[0x40] = UndefinedOpcode;
		dtable_0f[0x41] = UndefinedOpcode;
		dtable_0f[0x42] = UndefinedOpcode;
		dtable_0f[0x43] = UndefinedOpcode;
		dtable_0f[0x44] = UndefinedOpcode;
		dtable_0f[0x45] = UndefinedOpcode;
		dtable_0f[0x46] = UndefinedOpcode;
		dtable_0f[0x47] = UndefinedOpcode;
		dtable_0f[0x48] = UndefinedOpcode;
		dtable_0f[0x49] = UndefinedOpcode;
		dtable_0f[0x4a] = UndefinedOpcode;
		dtable_0f[0x4b] = UndefinedOpcode;
		dtable_0f[0x4c] = UndefinedOpcode;
		dtable_0f[0x4d] = UndefinedOpcode;
		dtable_0f[0x4e] = UndefinedOpcode;
		dtable_0f[0x4f] = UndefinedOpcode;
		dtable_0f[0x50] = UndefinedOpcode;
		dtable_0f[0x51] = UndefinedOpcode;
		dtable_0f[0x52] = UndefinedOpcode;
		dtable_0f[0x53] = UndefinedOpcode;
		dtable_0f[0x54] = UndefinedOpcode;
		dtable_0f[0x55] = UndefinedOpcode;
		dtable_0f[0x56] = UndefinedOpcode;
		dtable_0f[0x57] = UndefinedOpcode;
		dtable_0f[0x58] = UndefinedOpcode;
		dtable_0f[0x59] = UndefinedOpcode;
		dtable_0f[0x5a] = UndefinedOpcode;
		dtable_0f[0x5b] = UndefinedOpcode;
		dtable_0f[0x5c] = UndefinedOpcode;
		dtable_0f[0x5d] = UndefinedOpcode;
		dtable_0f[0x5e] = UndefinedOpcode;
		dtable_0f[0x5f] = UndefinedOpcode;
		dtable_0f[0x60] = UndefinedOpcode;
		dtable_0f[0x61] = UndefinedOpcode;
		dtable_0f[0x62] = UndefinedOpcode;
		dtable_0f[0x63] = UndefinedOpcode;
		dtable_0f[0x64] = UndefinedOpcode;
		dtable_0f[0x65] = UndefinedOpcode;
		dtable_0f[0x66] = UndefinedOpcode;
		dtable_0f[0x67] = UndefinedOpcode;
		dtable_0f[0x68] = UndefinedOpcode;
		dtable_0f[0x69] = UndefinedOpcode;
		dtable_0f[0x6a] = UndefinedOpcode;
		dtable_0f[0x6b] = UndefinedOpcode;
		dtable_0f[0x6c] = UndefinedOpcode;
		dtable_0f[0x6d] = UndefinedOpcode;
		dtable_0f[0x6e] = UndefinedOpcode;
		dtable_0f[0x6f] = UndefinedOpcode;
		dtable_0f[0x70] = UndefinedOpcode;
		dtable_0f[0x71] = UndefinedOpcode;
		dtable_0f[0x72] = UndefinedOpcode;
		dtable_0f[0x73] = UndefinedOpcode;
		dtable_0f[0x74] = UndefinedOpcode;
		dtable_0f[0x75] = UndefinedOpcode;
		dtable_0f[0x76] = UndefinedOpcode;
		dtable_0f[0x77] = UndefinedOpcode;
		dtable_0f[0x78] = UndefinedOpcode;
		dtable_0f[0x79] = UndefinedOpcode;
		dtable_0f[0x7a] = UndefinedOpcode;
		dtable_0f[0x7b] = UndefinedOpcode;
		dtable_0f[0x7c] = UndefinedOpcode;
		dtable_0f[0x7d] = UndefinedOpcode;
		dtable_0f[0x7e] = UndefinedOpcode;
		dtable_0f[0x7f] = UndefinedOpcode;
		dtable_0f[0x80] = JO_REL32;
		dtable_0f[0x81] = JNO_REL32;
		dtable_0f[0x82] = JC_REL32;
		dtable_0f[0x83] = JNC_REL32;
		dtable_0f[0x84] = JZ_REL32;
		dtable_0f[0x85] = JNZ_REL32;
		dtable_0f[0x86] = JNA_REL32;
		dtable_0f[0x87] = JA_REL32;
		dtable_0f[0x88] = JS_REL32;
		dtable_0f[0x89] = JNS_REL32;
		dtable_0f[0x8a] = JP_REL32;
		dtable_0f[0x8b] = JNP_REL32;
		dtable_0f[0x8c] = JL_REL32;
		dtable_0f[0x8d] = JNL_REL32;
		dtable_0f[0x8e] = JNG_REL32;
		dtable_0f[0x8f] = JG_REL32;
		dtable_0f[0x90] = SETO_RM8;
		dtable_0f[0x91] = SETNO_RM8;
		dtable_0f[0x92] = SETC_RM8;
		dtable_0f[0x93] = SETNC_RM8;
		dtable_0f[0x94] = SETZ_RM8;
		dtable_0f[0x95] = SETNZ_RM8;
		dtable_0f[0x96] = SETNA_RM8;
		dtable_0f[0x97] = SETA_RM8;
		dtable_0f[0x98] = SETS_RM8;
		dtable_0f[0x99] = SETNS_RM8;
		dtable_0f[0x9a] = SETP_RM8;
		dtable_0f[0x9b] = SETNP_RM8;
		dtable_0f[0x9c] = SETL_RM8;
		dtable_0f[0x9d] = SETNL_RM8;
		dtable_0f[0x9e] = SETNG_RM8;
		dtable_0f[0x9f] = SETG_RM8;
		dtable_0f[0xa0] = PUSH_FS;
		dtable_0f[0xa1] = POP_FS;
		dtable_0f[0xa2] = CPUID;
		dtable_0f[0xa3] = BT_RM32_R32;
		dtable_0f[0xa4] = SHLD_RM32_R32_I8;
		dtable_0f[0xa5] = SHLD_RM32_R32_CL;
		dtable_0f[0xa6] = UndefinedOpcode;
		dtable_0f[0xa7] = UndefinedOpcode;
		dtable_0f[0xa8] = PUSH_GS;
		dtable_0f[0xa9] = POP_GS;
		dtable_0f[0xaa] = RSM;
		dtable_0f[0xab] = BTS_RM32_R32;
		dtable_0f[0xac] = SHRD_RM32_R32_I8;
		dtable_0f[0xad] = SHRD_RM32_R32_CL;
		dtable_0f[0xae] = UndefinedOpcode;
		dtable_0f[0xaf] = IMUL_R32_RM32;
		dtable_0f[0xb0] = UndefinedOpcode;
		dtable_0f[0xb1] = UndefinedOpcode;
		dtable_0f[0xb2] = LSS_R32_M16_32;
		dtable_0f[0xb3] = BTR_RM32_R32;
		dtable_0f[0xb4] = LFS_R32_M16_32;
		dtable_0f[0xb5] = LGS_R32_M16_32;
		dtable_0f[0xb6] = MOVZX_R32_RM8;
		dtable_0f[0xb7] = MOVZX_R32_RM16;
		dtable_0f[0xb8] = UndefinedOpcode;
		dtable_0f[0xb9] = UndefinedOpcode;
		dtable_0f[0xba] = INS_0F_BA;
		dtable_0f[0xbb] = BTC_RM32_R32;
		dtable_0f[0xbc] = BSF_R32_RM32;
		dtable_0f[0xbd] = BSR_R32_RM32;
		dtable_0f[0xbe] = MOVSX_R32_RM8;
		dtable_0f[0xbf] = MOVSX_R32_RM16;
		dtable_0f[0xc0] = UndefinedOpcode;
		dtable_0f[0xc1] = UndefinedOpcode;
		dtable_0f[0xc2] = UndefinedOpcode;
		dtable_0f[0xc3] = UndefinedOpcode;
		dtable_0f[0xc4] = UndefinedOpcode;
		dtable_0f[0xc5] = UndefinedOpcode;
		dtable_0f[0xc6] = UndefinedOpcode;
		dtable_0f[0xc7] = UndefinedOpcode;
		dtable_0f[0xc8] = UndefinedOpcode;
		dtable_0f[0xc9] = UndefinedOpcode;
		dtable_0f[0xca] = UndefinedOpcode;
		dtable_0f[0xcb] = UndefinedOpcode;
		dtable_0f[0xcc] = UndefinedOpcode;
		dtable_0f[0xcd] = UndefinedOpcode;
		dtable_0f[0xce] = UndefinedOpcode;
		dtable_0f[0xcf] = UndefinedOpcode;
		dtable_0f[0xd0] = UndefinedOpcode;
		dtable_0f[0xd1] = UndefinedOpcode;
		dtable_0f[0xd2] = UndefinedOpcode;
		dtable_0f[0xd3] = UndefinedOpcode;
		dtable_0f[0xd4] = UndefinedOpcode;
		dtable_0f[0xd5] = UndefinedOpcode;
		dtable_0f[0xd6] = UndefinedOpcode;
		dtable_0f[0xd7] = UndefinedOpcode;
		dtable_0f[0xd8] = UndefinedOpcode;
		dtable_0f[0xd9] = UndefinedOpcode;
		dtable_0f[0xda] = UndefinedOpcode;
		dtable_0f[0xdb] = UndefinedOpcode;
		dtable_0f[0xdc] = UndefinedOpcode;
		dtable_0f[0xdd] = UndefinedOpcode;
		dtable_0f[0xde] = UndefinedOpcode;
		dtable_0f[0xdf] = UndefinedOpcode;
		dtable_0f[0xe0] = UndefinedOpcode;
		dtable_0f[0xe1] = UndefinedOpcode;
		dtable_0f[0xe2] = UndefinedOpcode;
		dtable_0f[0xe3] = UndefinedOpcode;
		dtable_0f[0xe4] = UndefinedOpcode;
		dtable_0f[0xe5] = UndefinedOpcode;
		dtable_0f[0xe6] = UndefinedOpcode;
		dtable_0f[0xe7] = UndefinedOpcode;
		dtable_0f[0xe8] = UndefinedOpcode;
		dtable_0f[0xe9] = UndefinedOpcode;
		dtable_0f[0xea] = UndefinedOpcode;
		dtable_0f[0xeb] = UndefinedOpcode;
		dtable_0f[0xec] = UndefinedOpcode;
		dtable_0f[0xed] = UndefinedOpcode;
		dtable_0f[0xee] = UndefinedOpcode;
		dtable_0f[0xef] = UndefinedOpcode;
		dtable_0f[0xf0] = UndefinedOpcode;
		dtable_0f[0xf1] = UndefinedOpcode;
		dtable_0f[0xf2] = UndefinedOpcode;
		dtable_0f[0xf3] = UndefinedOpcode;
		dtable_0f[0xf4] = UndefinedOpcode;
		dtable_0f[0xf5] = UndefinedOpcode;
		dtable_0f[0xf6] = UndefinedOpcode;
		dtable_0f[0xf7] = UndefinedOpcode;
		dtable_0f[0xf8] = UndefinedOpcode;
		dtable_0f[0xf9] = UndefinedOpcode;
		dtable_0f[0xfa] = UndefinedOpcode;
		dtable_0f[0xfb] = UndefinedOpcode;
		dtable_0f[0xfc] = UndefinedOpcode;
		dtable_0f[0xfd] = UndefinedOpcode;
		dtable_0f[0xfe] = UndefinedOpcode;
		dtable_0f[0xff] = UndefinedOpcode;
		flaginit = 1;
	}

	stmt[0] = 0;
	drcode = rcode;
	defsize = flag32;

	iop = 0;

	flagmem = 0;
	flaglock = 0;
	prefix_oprsize = 0;
	prefix_addrsize = 0;

	SPRINTF(doverds, "DS");
	SPRINTF(doverss, "SS");

	do {
		_cb("dasm32");
		dop[0] = 0;
		dopr[0] = 0;
		dstmt[0] = 0;
		oldiop = iop;
		_chb(_d_code((unsigned char *)(&opcode), 1));
		iop = oldiop;
		_chb((*(dtable[opcode]))());
		if (strlen(dop)) {
			STRCAT(dop, " ");
			STRCPY(dstmt, dop);
			for (i = (int) strlen(dop);i < 8;++i) STRCAT(dstmt, " ");
			STRCAT(dstmt, dopr);
			STRCAT(stmt, dstmt);
		}
		_ce;
	} while (_kdf_check_prefix(opcode));
#if DASM_TRACE == 1
	if (trace.cid || trace.flagerror) {
		PRINTF("dasm32: bad machine code.\n");
	}
	utilsTraceFinal(&trace);
#endif
	return iop;
}
