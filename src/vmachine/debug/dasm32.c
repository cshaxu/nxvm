/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "../vcpuins.h"
#include "../vapi.h"
#include "dasm32.h"

typedef char t_dasm_str[0x0100];

static t_vaddrcc   drcode;
static t_dasm_str  dstmt;
static t_dasm_str  dop, dopr, drm, dr, dimm, dmovsreg, doverds, doverss, dimmoff8, dimmoff16, dimmsign;
static t_bool      flagmem, prefix_oprsize, prefix_addrsize;
static t_nubit8    cr;
static t_nubit64   cimm;
static t_nubit8    iop;

/* instruction dispatch */
static t_faddrcc dtable[0x100], dtable_0f[0x100];

/* stack pointer size */
#define _GetStackSize   (vcpu.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.cs.seg.exec.defsize ^ prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.cs.seg.exec.defsize ^ prefix_addrsize) ? 4 : 2)

#define _bb
#define _be
#define _cb
#define _ce
#define _chr(n) (n)
#define _chk(n) (n)
#define _chb(n) (n)
#define _impossible_
#define _impossible_r_
#define _comment_
#define _newins_

static void SPRINTFSI(char *str, t_nubit32 imm, t_nubit8 byte)
{
	char sign;
	t_nubit8 i8u;
	t_nubit16 i16u;
	t_nubit32 i32u;
	i8u = GetMax8(imm);
	i16u = GetMax16(imm);
	i32u = GetMax32(imm);
	switch (byte) {
	case 1:
		if (GetMSB8(imm)) {
			sign = '-';
		    i8u = ((~i8u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%02X", sign, i8u);
		break;
	case 2:
		if (GetMSB16(imm)) {
			sign = '-';
		    i16u = ((~i16u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%04X", sign, i16u);
		break;
	case 4:
		if (GetMSB32(imm)) {
			sign = '-';
		    i32u = ((~i32u) + 0x01);
		} else {
			sign = '+';
		}
		SPRINTF(str, "%c%08X", sign, i32u);
		break;
	default:_impossible_;break;
	}
}

/* kernel decoding function */
static t_bool _kdf_check_prefix(t_nubit8 opcode)
{
	switch (opcode) {
	case 0xf0:
	case 0xf2:
	case 0xf3:
	case 0x2e:
	case 0x36:
	case 0x3e:
	case 0x26:
		return 1;
		break;
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		i386(opcode) return 1;
		else return 0;
		break;
	default:
		return 0;
		break;
	}
	return 0x00;
}

static void _kdf_skip(t_nubit8 byte)
{
	_cb("_kdf_skip");
	_chk(iop += byte);
	_ce;
}
static t_nubit64 _kdf_code(t_nubit8 byte)
{
	t_nubitcc i;
	t_nubit64 result = 0;
	_cb("_kdf_code");
	for (i = 0;i < byte;++i)
		d_nubit8(GetRef(result) + i) = d_nubit8(drcode + iop + i);
	_chr(_kdf_skip(byte));
	_ce;
	return result;
}
static void _kdf_modrm(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	t_nsbit8 disp8;
	t_nubit16 disp16;
	t_nubit32 disp32;
	t_dasm_str dsibindex, dptr, drm2;
	t_nubit8 modrm, sib;
	t_nsbit8 sign;
	t_nubit8 disp8u;
	_cb("_kdf_modrm");
	_chk(modrm = (t_nubit8)_kdf_code(1));
	flagmem = 1;
	drm[0] = dr[0] = dsibindex[0] = 0;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		switch (_GetModRM_MOD(modrm)) {
		case 0: _bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[BX+SI]", doverds);break;
			case 1: SPRINTF(drm2, "%s:[BX+DI]", doverds);break;
			case 2: SPRINTF(drm2, "%s:[BP+SI]", doverss);break;
			case 3: SPRINTF(drm2, "%s:[BP+DI]", doverss);break;
			case 4: SPRINTF(drm2, "%s:[SI]",    doverds);break;
			case 5: SPRINTF(drm2, "%s:[DI]",    doverds);break;
			case 6: _bb("ModRM_RM(6)");
				_chk(disp16 = (t_nubit16)_kdf_code(2));
				SPRINTF(drm2, "%s:[%04X]", doverds, disp16);
				_be;break;
			case 7: SPRINTF(drm2, "%s:[BX]", doverds);break;
			default:_impossible_;break;}
			
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_kdf_code(1));
			sign = (disp8 & 0x80) ? '-' : '+';
			disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[BX+SI%c%02X]", doverds, sign, disp8);break;
			case 1: SPRINTF(drm2, "%s:[BX+DI%c%02X]", doverds, sign, disp8);break;
			case 2: SPRINTF(drm2, "%s:[BP+SI%c%02X]", doverss, sign, disp8);break;
			case 3: SPRINTF(drm2, "%s:[BP+DI%c%02X]", doverss, sign, disp8);break;
			case 4: SPRINTF(drm2, "%s:[SI%c%02X]",    doverds, sign, disp8);break;
			case 5: SPRINTF(drm2, "%s:[DI%c%02X]",    doverds, sign, disp8);break;
			case 6: SPRINTF(drm2, "%s:[BP%c%02X]",    doverss, sign, disp8);break;
			case 7: SPRINTF(drm2, "%s:[BX%c%02x]",    doverds, sign, disp8);break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp16 = (t_nubit16)_kdf_code(2));
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[BX+SI+%04X]", doverds, disp16);break;
			case 1: SPRINTF(drm2, "%s:[BX+DI+%04X]", doverds, disp16);break;
			case 2: SPRINTF(drm2, "%s:[BP+SI+%04X]", doverss, disp16);break;
			case 3: SPRINTF(drm2, "%s:[BP+DI+%04X]", doverss, disp16);break;
			case 4: SPRINTF(drm2, "%s:[SI+%04X]",    doverds, disp16);break;
			case 5: SPRINTF(drm2, "%s:[DI+%04X]",    doverds, disp16);break;
			case 6: SPRINTF(drm2, "%s:[BP+%04X]",    doverss, disp16);break;
			case 7: SPRINTF(drm2, "%s:[BX+%04x]",    doverds, disp16);break;
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
			_chk(sib = (t_nubit8)_kdf_code(1));
			switch (_GetSIB_Index(sib)) {
			case 0: SPRINTF(dsibindex, "+EAX*%d", (1 << _GetSIB_SS(sib)));break;
			case 1: SPRINTF(dsibindex, "+ECX*%d", (1 << _GetSIB_SS(sib)));break;
			case 2: SPRINTF(dsibindex, "+EDX*%d", (1 << _GetSIB_SS(sib)));break;
			case 3: SPRINTF(dsibindex, "+EBX*%d", (1 << _GetSIB_SS(sib)));break;
			case 4: break;
			case 5: SPRINTF(dsibindex, "+EBP*%d", (1 << _GetSIB_SS(sib)));break;
			case 6: SPRINTF(dsibindex, "+ESI*%d", (1 << _GetSIB_SS(sib)));break;
			case 7: SPRINTF(dsibindex, "+EDI*%d", (1 << _GetSIB_SS(sib)));break;
			default:_impossible_;break;}
		}
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			_bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[EAX]", doverds);break;
			case 1: SPRINTF(drm2, "%s:[ECX]", doverds);break;
			case 2: SPRINTF(drm2, "%s:[EDX]", doverds);break;
			case 3: SPRINTF(drm2, "%s:[EBX]", doverds);break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm2, "%s:[EAX%s]", doverds, dsibindex);break;
				case 1: SPRINTF(drm2, "%s:[ECX%s]", doverds, dsibindex);break;
				case 2: SPRINTF(drm2, "%s:[EDX%s]", doverds, dsibindex);break;
				case 3: SPRINTF(drm2, "%s:[EBX%s]", doverds, dsibindex);break;
				case 4: SPRINTF(drm2, "%s:[ESP%s]", doverss, dsibindex);break;
				case 5: _bb("SIB_Base(5)");
					_chk(disp32 = (t_nubit32)_kdf_code(4));
					SPRINTF(drm2, "%s:[%08X%s]", doverds, disp32, dsibindex);
					_be;break;
				case 6: SPRINTF(drm2, "%s:[ESI%s]", doverds, dsibindex);break;
				case 7: SPRINTF(drm2, "%s:[EDI%s]", doverds, dsibindex);break;
				default:_impossible_;break;}
				_be;break;
			case 5: _bb("ModRM_RM(5)");
				_chk(disp32 = (t_nubit32)_kdf_code(4));
				SPRINTF(drm2, "%s:[%08X]", doverds, disp32);
				_be;break;
			case 6: SPRINTF(drm2, "%s:[ESI]", doverds);break;
			case 7: SPRINTF(drm2, "%s:[EDI]", doverds);break;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_kdf_code(1));
			sign = (disp8 & 0x80) ? '-' : '+';
			disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[EAX%c%02X]", doverds, sign, disp8u);break;
			case 1: SPRINTF(drm2, "%s:[ECX%c%02X]", doverds, sign, disp8u);break;
			case 2: SPRINTF(drm2, "%s:[EDX%c%02X]", doverds, sign, disp8u);break;
			case 3: SPRINTF(drm2, "%s:[EBX%c%02X]", doverds, sign, disp8u);break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm2, "%s:[EAX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 1: SPRINTF(drm2, "%s:[ECX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 2: SPRINTF(drm2, "%s:[EDX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 3: SPRINTF(drm2, "%s:[EBX%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 4: SPRINTF(drm2, "%s:[ESP%s%c%02X]", doverss, dsibindex, sign, disp8u);break;
				case 5: SPRINTF(drm2, "%s:[EBP%s%c%02X]", doverss, dsibindex, sign, disp8u);break;
				case 6: SPRINTF(drm2, "%s:[ESI%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				case 7: SPRINTF(drm2, "%s:[EDI%s%c%02X]", doverds, dsibindex, sign, disp8u);break;
				default:_impossible_;break;}
				_be;break;
			case 5: SPRINTF(drm2, "%s:[EBP%c%02X]", doverss, sign, disp8u);break;
			case 6: SPRINTF(drm2, "%s:[ESI%c%02X]", doverds, sign, disp8u);break;
			case 7: SPRINTF(drm2, "%s:[EDI%c%02X]", doverds, sign, disp8u);break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp32 = (t_nubit32)_kdf_code(4));
			switch (_GetModRM_RM(modrm)) {
			case 0: SPRINTF(drm2, "%s:[EAX+%08X]", doverds, disp32);break;
			case 1: SPRINTF(drm2, "%s:[ECX+%08X]", doverds, disp32);break;
			case 2: SPRINTF(drm2, "%s:[EDX+%08X]", doverds, disp32);break;
			case 3: SPRINTF(drm2, "%s:[EBX+%08X]", doverds, disp32);break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: SPRINTF(drm2, "%s:[EAX%s+%08X]", doverds, dsibindex, disp32);break;
				case 1: SPRINTF(drm2, "%s:[ECX%s+%08X]", doverds, dsibindex, disp32);break;
				case 2: SPRINTF(drm2, "%s:[EDX%s+%08X]", doverds, dsibindex, disp32);break;
				case 3: SPRINTF(drm2, "%s:[EBX%s+%08X]", doverds, dsibindex, disp32);break;
				case 4: SPRINTF(drm2, "%s:[ESP%s+%08X]", doverss, dsibindex, disp32);break;
				case 5: SPRINTF(drm2, "%s:[EBP%s+%08X]", doverss, dsibindex, disp32);break;
				case 6: SPRINTF(drm2, "%s:[ESI%s+%08X]", doverds, dsibindex, disp32);break;
				case 7: SPRINTF(drm2, "%s:[EDI%s+%08X]", doverds, dsibindex, disp32);break;
				default:_impossible_;break;}
				_be;break;
			case 5: SPRINTF(drm2, "%s:[EBP+%08X]", doverss, disp32);break;
			case 6: SPRINTF(drm2, "%s:[ESI+%08X]", doverds, disp32);break;
			case 7: SPRINTF(drm2, "%s:[EDI+%08X]", doverds, disp32);break;
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
	} else {
		switch (rmbyte) {
		case 1: SPRINTF(dptr, "BYTE PTR ");break;
		case 2: SPRINTF(dptr, "WORD PTR ");break;
		case 4: SPRINTF(dptr, "DWORD PTR ");break;
		default:_impossible_;break;
		}
		SPRINTF(drm, "%s%s", dptr, drm2);
	}
	switch (regbyte) {
	case 0:
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
static void _d_skip(t_nubit8 byte)
{
	_cb("_d_skip");
	_chk(_kdf_skip(byte));
	_ce;
}
static t_nubit64 _d_code(t_nubit8 byte)
{
	t_nubit64 result;
	_cb("_d_code");
	_chr(result = _kdf_code(byte));
	_ce;
	return result;
}
static void _d_imm(t_nubit8 byte)
{
	_cb("_d_imm");
	_chk(cimm = _d_code(byte));
	_ce;
}
static void _d_moffs(t_nubit8 byte)
{
	t_nubit32 offset;
	_cb("_d_moffs");
	flagmem = 1;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		_chk(offset = GetMax16(_d_code(2)));
		SPRINTF(drm, "%s:[%04X]", doverds, GetMax16(offset));
		_be;break;
	case 4: _bb("AddressSize(4)");
		_chk(offset = GetMax32(_d_code(4)));
		SPRINTF(drm, "%s:[%08X]", doverds, GetMax32(offset));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_modrm_sreg(t_nubit8 rmbyte)
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
		SPRINTF(dmovsreg, "<ERROR>");
		_be;break;
	}
	_ce;
}
static void _d_modrm_ea(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	t_dasm_str drm2;
	_cb("_d_modrm_ea");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!flagmem) {
		_bb("flagmem(0)");
		SPRINTF(drm, "<ERROR>");
		_be;
	} else {
		STRCPY(drm2, drm);
		SPRINTF(drm, "%s", (drm2 + 4));
		drm[strlen(drm) - 1] = 0;
	}
	_ce;
}
static void _d_modrm(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	_cb("_d_modrm");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!flagmem && vcpu.flaglock) {
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	t_nubit8 oldiop;
	t_nubit8 opcode;
	_cb("INS_0F");
	_adv;
	oldiop = iop;
	_chk(opcode = GetMax8(_d_code(1)));
	iop = oldiop;
	_chk(ExecFun(dtable_0f[opcode]));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	SPRINTF(dopr, "AL,%02X", GetMax8(cimm));
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
		SPRINTF(dopr, "AX,%04X", GetMax16(cimm));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(4));
		SPRINTF(dopr, "EAX,%08X", GetMax32(cimm));
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
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "AX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EAX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ECX()
{
	_cb("INC_ECX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "CX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ECX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EDX()
{
	_cb("INC_EDX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EBX()
{
	_cb("INC_EBX");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ESP()
{
	_cb("INC_ESP");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EBP()
{
	_cb("INC_EBP");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_ESI()
{
	_cb("INC_ESI");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INC_EDI()
{
	_cb("INC_EDI");
	_adv;
	SPRINTF(dop, "INC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EAX()
{
	_cb("DEC_EAX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "AX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EAX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ECX()
{
	_cb("DEC_ECX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "CX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ECX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EDX()
{
	_cb("DEC_EDX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EBX()
{
	_cb("DEC_EBX");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ESP()
{
	_cb("DEC_ESP");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EBP()
{
	_cb("DEC_EBP");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_ESI()
{
	_cb("DEC_ESI");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void DEC_EDI()
{
	_cb("DEC_EDI");
	_adv;
	SPRINTF(dop, "DEC");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EAX()
{
	_cb("PUSH_EAX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "AX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EAX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ECX()
{
	_cb("PUSH_ECX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "CX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ECX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EDX()
{
	_cb("PUSH_EDX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EBX()
{
	_cb("PUSH_EBX");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ESP()
{
	_cb("PUSH_ESP");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EBP()
{
	_cb("PUSH_EBP");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_ESI()
{
	_cb("PUSH_ESI");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_EDI()
{
	_cb("PUSH_EDI");
	_adv;
	SPRINTF(dop, "PUSH");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void POP_EAX()
{
	_cb("POP_EAX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "AX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EAX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ECX()
{
	_cb("POP_ECX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "CX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ECX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EDX()
{
	_cb("POP_EDX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EBX()
{
	_cb("POP_EBX");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BX");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBX");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ESP()
{
	_cb("POP_ESP");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EBP()
{
	_cb("POP_EBP");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "BP");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EBP");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_ESI()
{
	_cb("POP_ESI");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "SI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "ESI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void POP_EDI()
{
	_cb("POP_EDI");
	_adv;
	SPRINTF(dop, "POP");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		SPRINTF(dopr, "DI");break;
		_be;break;
	case 4: _bb("OperandSize(4)");
		SPRINTF(dopr, "EDI");break;
		_be;break;
	default:_impossible_;break;}
	_ce;
}

static void PUSHA()
{
	_cb("PUSHA");
	_adv;
	SPRINTF(dop, "PUSHA");
	_ce;
}
static void POPA()
{
	_cb("POPA");
	_adv;
	SPRINTF(dop, "POPA");
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
	case 2: SPRINTF(dopr, "%04X", GetMax16(cimm));break;
	case 4: SPRINTF(dopr, "%08X", GetMax32(cimm));break;
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
	case 2: SPRINTF(dopr, "%s,%s,%04X", dr, drm, GetMax16(cimm));break;
	case 4: SPRINTF(dopr, "%s,%s,%08X", dr, drm, GetMax32(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void PUSH_I8()
{
	_cb("PUSH_I8");
	_adv;
	SPRINTF(dop, "PUSH");
	_chk(_d_imm(1));
	SPRINTF(dopr, "%02X", GetMax8(cimm));
	_ce;
}
static void IMUL_R32_RM32_I8()
{
	_cb("IMUL_R32_RM32_I8");
	_adv;
	SPRINTF(dop, "IMUL");
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_d_imm(1));
	SPRINTF(dopr, "%s,%s,%04X", dr, drm, GetMax8(cimm));
	_ce;
}
static void INSB()
{
	_cb("INSB");
	_adv;
	SPRINTF(dop, "INSB");
	_ce;
}
static void INSW()
{
	_cb("INSW");
	_adv;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		SPRINTF(dop, "INSW");
		_be;break;
	case 4: _bb("AddressSize(4)");
		SPRINTF(dop, "INSW");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void OUTSB()
{
	_cb("OUTSB");
	_adv;
	SPRINTF(dop, "OUTSB");
	_ce;
}
static void OUTSW()
{
	_cb("OUTSW");
	_adv;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		SPRINTF(dop, "OUTSW");
		_be;break;
	case 4: _bb("AddressSize(4)");
		SPRINTF(dop, "OUT	");
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void JO_REL8()
{
	_cb("JO_REL8");
	_adv;
	SPRINTF(dop, "JO");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNO_REL8()
{
	_cb("JNO_REL8");
	_adv;
	SPRINTF(dop, "JNO");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JC_REL8()
{
	_cb("JC_REL8");
	_adv;
	SPRINTF(dop, "JC");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNC_REL8()
{
	_cb("JNC_REL8");
	_adv;
	SPRINTF(dop, "JNC");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JZ_REL8()
{
	_cb("JZ_REL8");
	_adv;
	SPRINTF(dop, "JZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNZ_REL8()
{
	_cb("JNZ_REL8");
	_adv;
	SPRINTF(dop, "JNZ");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNA_REL8()
{
	_cb("JNA_REL8");
	_adv;
	SPRINTF(dop, "JNA");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JA_REL8()
{
	_cb("JA_REL8");
	_adv;
	SPRINTF(dop, "JA");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JS_REL8()
{
	_cb("JS_REL8");
	_adv;
	SPRINTF(dop, "JS");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNS_REL8()
{
	_cb("JNS_REL8");
	_adv;
	SPRINTF(dop, "JNS");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JP_REL8()
{
	_cb("JP_REL8");
	_adv;
	SPRINTF(dop, "JP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNP_REL8()
{
	_cb("JNP_REL8");
	_adv;
	SPRINTF(dop, "JNP");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JL_REL8()
{
	_cb("JL_REL8");
	_adv;
	SPRINTF(dop, "JL");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNL_REL8()
{
	_cb("JNL_REL8");
	_adv;
	SPRINTF(dop, "JNL");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JNG_REL8()
{
	_cb("JNG_REL8");
	_adv;
	SPRINTF(dop, "JNG");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
	_ce;
}
static void JG_REL8()
{
	_cb("JG_REL8");
	_adv;
	SPRINTF(dop, "JG");
	_chk(_d_imm(1));
	SPRINTFSI(dopr, GetMax8(cimm), 1);
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
	SPRINTF(dopr, "%s,%02X", drm, GetMax8(cimm));
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
	case 2: SPRINTF(dopr, "%s,%04X", drm, GetMax16(cimm));break;
	case 4: SPRINTF(dopr, "%s,%08X", drm, GetMax32(cimm));break;
	default:_impossible_;break;}
	_ce;
}
static void INS_83()
{
	_cb("INS_83");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	_chk(_d_imm(1));
	switch (vcpuins.cr) {
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
	SPRINTF(dopr, "%s,%02X", drm, GetMax8(cimm));
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
	SPRINTF(dopr, "%s,%s", dr, drm)
	_ce;
}
static void MOV_RM16_SREG()
{
	_cb("MOV_RM16_SREG");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_sreg(2));
	SPRINTF(dopr, "%s,%s", drm, dr)
	_ce;
}
static void LEA_R16_M16()
{
	_cb("LEA_R16_M16");
	_adv;
	SPRINTF(dop, "LEA");
	_chk(_d_modrm_ea(_GetOperandSize, _GetOperandSize));
	SPRINTF(dopr, "%s,%s", dr, drm);
	_ce;
}
static void MOV_SREG_RM16()
{
	_cb("MOV_SREG_RM16");
	_adv;
	SPRINTF(dop, "MOV");
	_chk(_d_modrm_sreg(2));
	SPRINTF(dopr, "%s,%s", dr, drm)
	_ce;
}
static void INS_8F()
{
	_cb("INS_8F");
	_adv;
	_chk(_d_modrm(0, _GetOperandSize));
	switch (vcpuins.cr) {
	case 0: /* POP_RM32 */
		_bb("POP_RM32");
		SPRINTF(dop, "POP");
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
	t_nubit16 newcs;
	t_nubit32 neweip;
	_cb("CALL_PTR16_32");
	_adv;
	SPRINTF(dop, "CALL");
	switch (_GetOperandSize) {
	case 2: _bb("OperandSize(2)");
		_chk(_d_imm(4));
		neweip = GetMax16(vcpuins.cimm);
		newcs = GetMax16(vcpuins.cimm >> 16);
		SPRINTF(dopr, "%04X:%04X", newcs, GetMax16(neweip));
		_chk(_e_call_far(newcs, neweip, 2));
		_be;break;
	case 4: _bb("OperandSize(4)");
		_chk(_d_imm(8));
		neweip = GetMax32(vcpuins.cimm);
		newcs = GetMax16(vcpuins.cimm >> 32);
		SPRINTF(dopr, "%04X:%08X", newcs, GetMax32(neweip));
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
	SPRINTF(dop, "PUSHF");
	_ce;
}
static void POPF()
{
	_cb("POPF");
	_adv;
	SPRINTF(dop, "POPF");
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
	i386(0xa0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_moffs(1));
	_chk(vcpuins.result = _m_read_rm(1));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void MOV_EAX_MOFFS32()
{
	_cb("MOV_EAX_MOFFS32");
	i386(0xa1) {
		_adv;
		_chk(_d_moffs(_GetOperandSize));
		_chk(vcpuins.result = _m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.ax = GetMax16(vcpuins.result);break;
		case 4: vcpu.eax = GetMax32(vcpuins.result);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_moffs(2));
		_chk(vcpuins.result = _m_read_rm(2));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void MOV_MOFFS8_AL()
{
	_cb("MOV_MOFFS8_AL");
	i386(0xa2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_moffs(1));
	vcpuins.result = vcpu.al;
	_chk(_m_write_rm(vcpuins.result, 1));
	_ce;
}
static void MOV_MOFFS32_EAX()
{
	_cb("MOV_MOFFS32_EAX");
	i386(0xa3) {
		_adv;
		_chk(_d_moffs(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpuins.result = vcpu.ax;break;
		case 4: vcpuins.result = vcpu.eax;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_moffs(2);
		vcpuins.result = vcpu.ax;
		_m_write_rm(vcpuins.result, 2);
	}
	_ce;
}
static void MOVSB()
{
	_cb("MOVSB");
	i386(0xa4) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_movs(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_movs(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_movs(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_movs(1);
		else {
			if (vcpu.cx) {
				_m_movs(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void MOVSW()
{
	_cb("MOVSW");
	i386(0xa5) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_movs(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_movs(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_movs(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_movs(2);
		else {
			if (vcpu.cx) {
				_m_movs(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void CMPSB()
{
	_cb("CMPSB");
	i386(0xa6) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_cmps(8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_cmps(8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_cmps(8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_cmps(8);
		else {
			if (vcpu.cx) {
				_a_cmps(8);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void CMPSW()
{
	_cb("CMPSW");
	i386(0xa7) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_cmps(_GetOperandSize * 8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_cmps(_GetOperandSize * 8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_cmps(_GetOperandSize * 8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_cmps(16);
		else {
			if (vcpu.cx) {
				_a_cmps(16);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void TEST_AL_I8()
{
	_cb("TEST_AL_I8");
	i386(0xa8) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_test(vcpu.al, vcpuins.cimm, 8));
	_ce;
}
static void TEST_EAX_I32()
{
	_cb("TEST_EAX_I32");
	i386(0xa9) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_test(vcpu.ax, vcpuins.cimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_test(vcpu.eax, vcpuins.cimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_test(vcpu.ax, vcpuins.cimm, 16));
	}
	_ce;
}
static void STOSB()
{
	_cb("STOSB");
	i386(0xaa) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_stos(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_stos(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_stos(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_stos(1);
		else {
			if (vcpu.cx) {
				_m_stos(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void STOSW()
{
	_cb("STOSW");
	i386(0xab) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_stos(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_stos(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_stos(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_stos(2);
		else {
			if (vcpu.cx) {
				_m_stos(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void LODSB()
{
	_cb("LODSB");
	i386(0xac) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_lods(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_lods(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_lods(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_lods(1);
		else {
			if (vcpu.cx) {
				_m_lods(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void LODSW()
{
	_cb("LODSW");
	i386(0xad) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_lods(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_lods(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_lods(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_lods(2);
		else {
			if (vcpu.cx) {
				_m_lods(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void SCASB()
{
	_cb("SCASB");
	i386(0xae) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_scas(8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_scas(8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_scas(8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_scas(8);
		else {
			if (vcpu.cx) {
				_a_scas(8);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void SCASW()
{
	_cb("SCASW");
	i386(0xaf) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_scas(_GetOperandSize * 8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_scas(_GetOperandSize * 8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_scas(_GetOperandSize * 8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_scas(16);
		else {
			if (vcpu.cx) {
				_a_scas(16);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
static void MOV_AL_I8()
{
	_cb("MOV_AL_I8");
	i386(0xb0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.al = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_CL_I8()
{
	_cb("MOV_CL_I8");
	i386(0xb1) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.cl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_DL_I8()
{
	_cb("MOV_DL_I8");
	i386(0xb2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.dl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_BL_I8()
{
	_cb("MOV_BL_I8");
	i386(0xb3) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.bl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_AH_I8()
{
	_cb("MOV_AH_I8");
	i386(0xb4) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.ah = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_CH_I8()
{
	_cb("MOV_CH_I8");
	i386(0xb5) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.ch = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_DH_I8()
{
	_cb("MOV_DH_I8");
	i386(0xb6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.dh = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_BH_I8()
{
	_cb("MOV_BH_I8");
	i386(0xb7) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.bh = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_EAX_I32()
{
	_cb("MOV_EAX_I32");
	i386(0xb8) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.ax = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.eax = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.ax = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ECX_I32()
{
	_cb("MOV_ECX_I32");
	i386(0xb9) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.cx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ecx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.cx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EDX_I32()
{
	_cb("MOV_EDX_I32");
	i386(0xba) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.dx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.edx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.dx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EBX_I32()
{
	_cb("MOV_EBX_I32");
	i386(0xbb) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.bx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ebx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.bx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ESP_I32()
{
	_cb("MOV_ESP_I32");
	i386(0xbc) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.sp = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.esp = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.sp = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EBP_I32()
{
	_cb("MOV_EBP_I32");
	i386(0xbd) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.bp = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ebp = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.bp = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ESI_I32()
{
	_cb("MOV_ESI_I32");
	i386(0xbe) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.si = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.esi = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.si = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EDI_I32()
{
	_cb("MOV_EDI_I32");
	i386(0xbf) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.di = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.edi = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.di = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void INS_C0()
{
	_cb("INS_C0");
	_newins_;
	i386(0xc0) {
		_adv;
		_chk(_d_modrm(0, 1));
		_chk(vcpuins.crm = _m_read_rm(1));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM8_I8 */
			_bb("ROL_RM8_I8");
			_chk(_a_rol(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 1: /* ROR_RM8_I8 */
			_bb("ROR_RM8_I8");
			_chk(_a_ror(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 2: /* RCL_RM8_I8 */
			_bb("RCL_RM8_I8");
			_chk(_a_rcl(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 3: /* RCR_RM8_I8 */
			_bb("RCR_RM8_I8");
			_chk(_a_rcr(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 4: /* SHL_RM8_I8 */
			_bb("SHL_RM8_I8");
			_chk(_a_shl(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 5: /* SHR_RM8_I8 */
			_bb("SHR_RM8_I8");
			_chk(_a_shr(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM8_I8 */
			_bb("SAR_RM8_I8");
			_chk(_a_sar(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, 1));
	} else
		UndefinedOpcode();
	_ce;
}
static void INS_C1()
{
	_cb("INS_C1");
	_newins_;
	i386(0xc1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32_I8 */
			_bb("ROL_RM32_I8");
			_chk(_a_rol(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32_I8 */
			_bb("ROR_RM32_I8");
			_chk(_a_ror(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32_I8 */
			_bb("RCL_RM32_I8");
			_chk(_a_rcl(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32_I8 */
			_bb("RCR_RM32_I8");
			_chk(_a_rcr(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32_I8 */
			_bb("SHL_RM32_I8");
			_chk(_a_shl(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32_I8 */
			_bb("SHR_RM32_I8");
			_chk(_a_shr(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32_I8 */
			_bb("SAR_RM32_I8");
			_chk(_a_sar(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	} else 
		UndefinedOpcode();
	_ce;
}
static void RET_I16()
{
	_cb("RET_I16");
	i386(0xc2) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_near(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_ret_near(GetMax16(vcpuins.cimm), 2);
	}
	_ce;
}
static void RET()
{
	_cb("RET");
	i386(0xc3) {
		_adv;
		_chk(_e_ret_near(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_ret_near(0, 2));
	}
	_ce;
}
static void LES_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LES_R32_M16_32");
	i386(0xc4) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: offset = GetMax16(vcpuins.crm);break;
		case 4: offset = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		vcpuins.mrm.offset += _GetOperandSize;
		_chk(vcpuins.crm = _m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.es, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 4);
		_chk(vcpuins.crm = _m_read_rm(2));
		offset = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(vcpuins.crm = _m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.es, vcpuins.rr, selector, offset, 2));
	}
	_ce;
}
static void LDS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LDS_R32_M16_32");
	i386(0xc5) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: offset = GetMax16(vcpuins.crm);break;
		case 4: offset = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		vcpuins.mrm.offset += _GetOperandSize;
		_chk(vcpuins.crm = _m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.ds, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 4);
		_chk(vcpuins.crm = _m_read_rm(2));
		offset = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(vcpuins.crm = _m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.ds, vcpuins.rr, selector, offset, 2));
	}
	_ce;
}
static void INS_C6()
{
	_cb("INS_C6");
	i386(0xc6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	switch (vcpuins.cr) {
	case 0: /* MOV_RM8_I8 */
		_bb("MOV_RM8_I8");
		_chk(_d_imm(1));
		_chk(_m_write_rm(vcpuins.cimm, 1));
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
	i386(0xc7) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM32_I32 */
			_bb("MOV_RM32_I32");
			_chk(_d_imm(_GetOperandSize));
			_chk(_m_write_rm(vcpuins.cimm, _GetOperandSize));
			_be;break;
		case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM16_I16 */
			_bb("MOV_RM16_I16");
			_chk(_d_imm(2));
			_chk(_m_write_rm(vcpuins.cimm, 2));
			_be;break;
		case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void ENTER()
{
	t_nubit32 data = 0x00000000;
	t_nubit32 temp = 0x00000000;
	t_nubit32 i = 0x00000000;
	t_nubit16 size = 0x0000;
	t_nubit8 level = 0x00;
	_cb("ENTER");
	_newins_;
	i386(0xc8) {
		_adv;
		_chk(_d_imm(2));
		size = (t_nubit16)vcpuins.cimm;
		_chk(_d_imm(1));
		level = (t_nubit8)vcpuins.cimm;
		level %= 32;
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			_chk(_e_push(vcpu.bp, 2));
			_comment("ENTER: fetch sp after pushing bp");
			temp = vcpu.sp;
			_be;break;
		case 4: _bb("StackSize(4)");
			_chk(_e_push(vcpu.ebp, 4));
			_comment("ENTER: fetch esp after pushing ebp");
			temp = vcpu.esp;
			_be;break;
		default:_impossible_;break;}
		if (level) {
			_bb("level(!0)");
			_comment("ENTER: this instruction is too inefficient and error-prone.");
			for (i = 0;i < level;++i) {
				_bb("for");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 2;
						_chk(data = (t_nubit16)_s_read_ss(vcpu.bp, 2));
						_chk(_e_push(data, 2));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 2;
						_chk(data = (t_nubit16)_s_read_ss(vcpu.ebp, 2));
						_chk(_e_push(data, 2));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				case 4: _bb("OperandSize(4)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 4;
						_chk(data = (t_nubit32)_s_read_ss(vcpu.bp, 4));
						_chk(_e_push(data, 4));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 4;
						_chk(data = (t_nubit32)_s_read_ss(vcpu.ebp, 4));
						_chk(_e_push(data, 4));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			switch (_GetOperandSize) {
			case 2: _bb("OperandSize(2)");
				_chk(_e_push(temp, 2));
				_be;break;
			case 4: _bb("OperandSize(4)");
				_chk(_e_push(temp, 4));
				_be;break;
			default:_impossible_;break;}
			_be;
		}
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			vcpu.bp = GetMax16(temp);
			vcpu.sp = vcpu.bp - size;
			_be;break;
		case 4: _bb("StackSize(4)");
			vcpu.ebp = GetMax32(temp);
			vcpu.esp = vcpu.ebp - size;
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void LEAVE()
{
	_cb("LEAVE");
	_newins_;
	i386(0xc9) {
		if (!_IsProtected && vcpu.ebp > 0x0000ffff) {
			_bb("Protected(0),ebp(>0000ffff)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_m_test_logical(&vcpu.ss, vcpu.ebp, _GetOperandSize, 1));
		switch (_GetStackSize) {
		case 2: vcpu.sp = vcpu.bp;break;
		case 4: vcpu.esp = vcpu.ebp;break;
		default:_impossible_;break;}
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.bp = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.ebp = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void RETF_I16()
{
	_cb("RETF_I16");
	i386(0xca) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_far(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_ret_far(GetMax16(vcpuins.cimm), 2);
	}
	_ce;
}
static void RETF()
{
	_cb("RETF");
	i386(0xcb) {
		_adv;
		_chk(_e_ret_far(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_ret_far(0, 2));
	}
	_ce;
}
static void INT3()
{
	_cb("INT3");
	_newins_;
	i386(0xcc) {
		_adv;
		_chk(_e_int3(_GetOperandSize));
	} else {
		vcpu.ip++;
		_e_int_n(0x03, _GetOperandSize);
	}
	_ce;
}
static void INT_I8()
{
	_cb("INT_I8");
	i386(0xcc) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_int_n((t_nubit8)vcpuins.cimm, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_e_int_n((t_nubit8)vcpuins.cimm, 2);
	}
	_ce;
}
static void INTO()
{
	_cb("INTO");
	_newins_;
	i386(0xce) {
		_adv;
		_chk(_e_into(_GetOperandSize));
	} else {
		vcpu.ip++;
		if(_GetEFLAGS_OF) _e_int_n(0x04, _GetOperandSize);
	}
	_ce;
}
static void IRET()
{
	_cb("IRET");
	i386(0xcf) {
		_adv;
		_chk(_e_iret(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_iret(2));
	}
	_ce;
}
static void INS_D0()
{
	_cb("INS_D0");
	i386(0xd0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(vcpuins.crm = _m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* ROL_RM8 */
		_bb("ROL_RM8");
		_chk(_a_rol(vcpuins.crm, 1, 8));
		_be;break;
	case 1: /* ROR_RM8 */
		_bb("ROR_RM8");
		_chk(_a_ror(vcpuins.crm, 1, 8));
		_be;break;
	case 2: /* RCL_RM8 */
		_bb("RCL_RM8");
		_chk(_a_rcl(vcpuins.crm, 1, 8));
		_be;break;
	case 3: /* RCR_RM8 */
		_bb("RCR_RM8");
		_chk(_a_rcr(vcpuins.crm, 1, 8));
		_be;break;
	case 4: /* SHL_RM8 */
		_bb("SHL_RM8");
		_chk(_a_shl(vcpuins.crm, 1, 8));
		_be;break;
	case 5: /* SHR_RM8 */
		_bb("SHR_RM8");
		_chk(_a_shr(vcpuins.crm, 1, 8));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("vcpuins.cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8 */
		_bb("SAR_RM8");
		_chk(_a_sar(vcpuins.crm, 1, 8));
		_be;break;
	default:_impossible_;break;}
	_chk(_m_write_rm(vcpuins.result, 1));
	_ce;
}
static void INS_D1()
{
	_cb("INS_D1");
	i386(0xd1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32 */
			_bb("ROL_RM32");
			_chk(_a_rol(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32 */
			_bb("ROR_RM32");
			_chk(_a_ror(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32 */
			_bb("RCL_RM32");
			_chk(_a_rcl(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32 */
			_bb("RCR_RM32");
			_chk(_a_rcr(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32 */
			_bb("SHL_RM32");
			_chk(_a_shl(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32 */
			_bb("SHR_RM32");
			_chk(_a_shr(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32 */
			_bb("SAR_RM32");
			_chk(_a_sar(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(vcpuins.crm = _m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16 */
			_bb("ROL_RM16");
			_chk(_a_rol(vcpuins.crm, 1, 16));
			_be;break;
		case 1: /* ROR_RM16 */
			_bb("ROR_RM16");
			_chk(_a_ror(vcpuins.crm, 1, 16));
			_be;break;
		case 2: /* RCL_RM16 */
			_bb("RCL_RM16");
			_chk(_a_rcl(vcpuins.crm, 1, 16));
			_be;break;
		case 3: /* RCR_RM16 */
			_bb("RCR_RM16");
			_chk(_a_rcr(vcpuins.crm, 1, 16));
			_be;break;
		case 4: /* SHL_RM16 */
			_bb("SHL_RM16");
			_chk(_a_shl(vcpuins.crm, 1, 16));
			_be;break;
		case 5: /* SHR_RM16 */
			_bb("SHR_RM16");
			_chk(_a_shr(vcpuins.crm, 1, 16));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16 */
			_bb("SAR_RM16");
			_chk(_a_sar(vcpuins.crm, 1, 16));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, 2));
	}
	_ce;
}
static void INS_D2()
{
	_cb("INS_D2");
	i386(0xd2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(vcpuins.crm = _m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* ROL_RM8_CL */
		_bb("ROL_RM8_CL");
		_chk(_a_rol(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 1: /* ROR_RM8_CL */
		_bb("ROR_RM8_CL");
		_chk(_a_ror(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 2: /* RCL_RM8_CL */
		_bb("RCL_RM8_CL");
		_chk(_a_rcl(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 3: /* RCR_RM8_CL */
		_bb("RCR_RM8_CL");
		_chk(_a_rcr(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 4: /* SHL_RM8_CL */
		_bb("SHL_RM8_CL");
		_chk(_a_shl(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 5: /* SHR_RM8_CL */
		_bb("SHR_RM8_CL");
		_chk(_a_shr(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("vcpuins.cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8_CL */
		_bb("SAR_RM8_CL");
		_chk(_a_sar(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	default:_impossible_;break;}
	_chk(_m_write_rm(vcpuins.result, 1));
	_ce;
}
static void INS_D3()
{
	_cb("INS_D3");
	i386(0xd3) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32_CL */
			_bb("ROL_RM32_CL");
			_chk(_a_rol(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32_CL */
			_bb("ROR_RM32_CL");
			_chk(_a_ror(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32_CL */
			_bb("RCL_RM32_CL");
			_chk(_a_rcl(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32_CL */
			_bb("RCR_RM32_CL");
			_chk(_a_rcr(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32_CL */
			_bb("SHL_RM32_CL");
			_chk(_a_shl(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32_CL */
			_bb("SHR_RM32_CL");
			_chk(_a_shr(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32_CL */
			_bb("SAR_RM32_CL");
			_chk(_a_sar(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(vcpuins.crm = _m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16_CL */
			_bb("ROL_RM16_CL");
			_chk(_a_rol(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 1: /* ROR_RM16_CL */
			_bb("ROR_RM16_CL");
			_chk(_a_ror(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 2: /* RCL_RM16_CL */
			_bb("RCL_RM16_CL");
			_chk(_a_rcl(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 3: /* RCR_RM16_CL */
			_bb("RCR_RM16_CL");
			_chk(_a_rcr(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 4: /* SHL_RM16_CL */
			_bb("SHL_RM16_CL");
			_chk(_a_shl(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 5: /* SHR_RM16_CL */
			_bb("SHR_RM16_CL");
			_chk(_a_shr(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16_CL */
			_bb("SAR_RM16_CL");
			_chk(_a_sar(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		default:_impossible_;break;}
		_chk(_m_write_rm(vcpuins.result, 2));
	}
	_ce;
}
static void AAM()
{
	t_nubit8 base;
	_cb("AAM");
	i386(0xd4) {
		_adv;
		_chk(_d_imm(1));
	} else {
		vcpu.ip++;
		_d_imm(1);
	}
	base = GetMax8(vcpuins.cimm);
	vcpu.ah = vcpu.al / base;
	vcpu.al = vcpu.al % base;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAM_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
static void AAD()
{
	t_nubit8 base;
	_cb("AAD");
	i386(0xd5) {
		_adv;
		_chk(_d_imm(1));
	} else {
		vcpu.ip++;
		_d_imm(1);
	}
	base = GetMax8(vcpuins.cimm);
	vcpu.al = GetMax8(vcpu.al + (vcpu.ah * base));
	vcpu.ah = 0x00;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAD_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
static void XLAT()
{
	_cb("XLAT");
	i386(0xd7) {
		_adv;
		switch (_GetAddressSize) {
		case 2: _bb("AddressSize(2)");
			_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), 1));
			_be;break;
		case 4: _bb("AddressSize(4)");
			_newins_;
			_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.ebx + vcpu.al), 1));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), 1));
	}
	_ce;
}
static void LOOPNZ()
{
	_cb("LOOPNZ");
	i386(0xe0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, !_GetEFLAGS_ZF));
	_ce;
}
static void LOOPZ()
{
	_cb("LOOPZ");
	i386(0xe1) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, _GetEFLAGS_ZF));
	_ce;
}
static void LOOP()
{
	_cb("LOOP");
	i386(0xe2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, 1));
	_ce;
}
static void JCXZ_REL8()
{
	t_nubit32 cecx = 0x00000000;
	_cb("JCXZ_REL8");
	i386(0xe3) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetAddressSize) {
		case 2: cecx = vcpu.cx;break;
		case 4: cecx = vcpu.ecx;break;
		default:_impossible_;break;}
		_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !cecx));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_e_jcc(GetMax32(vcpuins.cimm), 1, !vcpu.cx);
	}
	_ce;
}
static void IN_AL_I8()
{
	_cb("IN_AL_I8");
	i386(0xe4) {
		_adv;
		_chk(_d_imm(1));
		_chk(vcpu.al = (t_nubit8)_p_input(GetMax8(vcpuins.cimm), 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.al = (t_nubit8)_p_input(GetMax8(vcpuins.cimm), 1);
	}
	_ce;
}
static void IN_EAX_I8()
{
	_cb("IN_EAX_I8");
	_newins_;
	i386(0xe5) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.ax = (t_nubit16)_p_input(GetMax8(vcpuins.cimm), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.eax = (t_nubit32)_p_input(GetMax8(vcpuins.cimm), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.ax = (t_nubit16)_p_input(GetMax8(vcpuins.cimm), 2);
	}
	_ce;
}
static void OUT_I8_AL()
{
	_cb("OUT_I8_AL");
	i386(0xe6) {
		_adv;
		_chk(_d_imm(1));
		_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.al, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_p_output(GetMax8(vcpuins.cimm), vcpu.al, 1);
	}
	_ce;

}
static void OUT_I8_EAX()
{
	_cb("OUT_I8_EAX");
	_newins_;
	i386(0xe7) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(1);
		_p_output(GetMax8(vcpuins.cimm), vcpu.ax, 2);
	}
	_ce;
}
static void CALL_REL32()
{
	_cb("CALL_REL32");
	i386(0xe8) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_e_call_near(GetMax16(vcpu.ip + (t_nsbit16)vcpuins.cimm), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_e_call_near(GetMax32(vcpu.eip + (t_nsbit32)vcpuins.cimm), 4));
			_be;break;
		default: _bb("OperandSize");
			_chk(_SetExcept_CE(_GetOperandSize));
			_be;break;
		}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_e_call_near(GetMax16(vcpu.ip + (t_nsbit16)vcpuins.cimm), 2));
	}
	_ce;
}
static void JMP_REL32()
{
	_cb("JMP_REL32");
	i386(0xe9) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, 1));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_jcc(GetMax32(vcpuins.cimm), 2, 1);
	}
	_ce;
}
static void JMP_PTR16_32()
{
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	_cb("JMP_PTR16_32");
	i386(0xea) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			neweip = (t_nubit16)vcpuins.cimm;
			_chk(_d_imm(2));
			newcs = (t_nubit16)vcpuins.cimm;
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			neweip = (t_nubit32)vcpuins.cimm;
			_chk(_d_imm(2));
			newcs = (t_nubit16)vcpuins.cimm;
			_be;break;
		default:_impossible_;break;}
		_chk(_e_jmp_far(newcs, neweip, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		neweip = (t_nubit16)vcpuins.cimm;
		_chk(_d_imm(2));
		newcs = (t_nubit16)vcpuins.cimm;
		_chk(_e_jmp_far(newcs, neweip, 2));
	}
	_ce;
}
static void JMP_REL8()
{
	_cb("JMP_REL8");
	i386(0xeb) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, 1));
	_ce;
}
static void IN_AL_DX()
{
	_cb("IN_AL_DX");
	i386(0xec) {
		_adv;
		_chk(vcpu.al = (t_nubit8)_p_input(vcpu.dx, 1));
	} else {
		vcpu.ip++;
		vcpu.al = (t_nubit8)_p_input(vcpu.dx, 1);
	}
	_ce;
}
static void IN_EAX_DX()
{
	_cb("IN_EAX_DX");
	i386(0xed) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.ax = (t_nubit16)_p_input(vcpu.dx, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.eax = (t_nubit32)_p_input(vcpu.dx, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpu.ax = (t_nubit16)_p_input(vcpu.dx, 2);
	}
	_ce;
}
static void OUT_DX_AL()
{
	_cb("OUT_DX_AL");
	i386(0xee) {
		_adv;
		_chk(_p_output(vcpu.dx, vcpu.al, 1));
	} else {
		vcpu.ip++;
		_p_output(vcpu.dx, vcpu.al, 1);
	}
	_ce;
}
static void OUT_DX_EAX()
{
	_cb("OUT_DX_EAX");
	_newins_;
	i386(0xef) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(vcpu.dx, vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(vcpu.dx, vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_p_output(vcpu.dx, vcpu.ax, 2);
	}
	_ce;
}
static void PREFIX_LOCK()
{
	t_nubit8 opcode = 0x00;
	t_nubit8 modrm = 0x00;
	t_nubit8 opcode_0f = 0x00;
	t_nubit32 ceip = vcpu.eip;
	_cb("PREFIX_LOCK");
	_newins_;
	i386(0xf0) {
		_adv;
		do {
			_chk(opcode = (t_nubit8)_s_read_cs(ceip, 1));
			ceip++;
		} while (_kdf_check_prefix(opcode));
		switch (opcode) {
		case 0x00: case 0x01: /* ADD */
		case 0x08: case 0x09: /* OR */
		case 0x10: case 0x11: /* ADC */
		case 0x18: case 0x19: /* SBB */
		case 0x20: case 0x21: /* AND */
		case 0x28: case 0x29: /* SUB */
		case 0x30: case 0x31: /* XOR */
		case 0x86: case 0x87: /* XCHG */
		case 0xf6: case 0xf7: /* NOT, NEG */
		case 0xfe: case 0xff: /* DEC, INC */
			vcpu.flaglock = 1;
			break;
		case 0x80: case 0x81: case 0x83:
			_bb("opcode(0x80/0x81/0x83)");
			_chk(modrm = (t_nubit8)_s_read_cs(ceip, 1));
			if (_GetModRM_REG(modrm) != 7)
				vcpu.flaglock = 1;
			else {
				_bb("ModRM_REG(7)");
				_chk(_SetExcept_UD(0));
				_be;
			}
			_be;break;
		case 0x0f: _bb("opcode(0f)");
			_chk(opcode_0f = (t_nubit8)_s_read_cs(ceip, 1));
			switch (opcode_0f) {
			case 0xa3: /* BT */
			case 0xab: /* BTS */
			case 0xb3: /* BTR */
			case 0xbb: /* BTC */
			case 0xba:
				vcpu.flaglock = 1;
				break;
			default: _bb("opcode_0f");
				_chk(_SetExcept_UD(0));
				_be;break;
			}
			_be;break;
		default: _bb("opcode");
			_chk(_SetExcept_UD(0));
			_be;break;
		}
	} else _adv;
	_ce;
}
static void PREFIX_REPNZ()
{
	_cb("PREFIX_REPNZ");
	i386(0xf2) {
		_adv;
		vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
	} else {
		vcpu.ip++;
		vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
	}
	_ce;
}
static void PREFIX_REPZ()
{
	_cb("PREFIX_REPZ");
	i386(0xf3) {
		_adv;
		vcpuins.prefix_rep = PREFIX_REP_REPZ;
	} else {
		vcpu.ip++;
		vcpuins.prefix_rep = PREFIX_REP_REPZ;
	}
	_ce;
}
static void HLT()
{
	_cb("HLT");
	_newins_;
	if (_GetCR0_PE && _GetCPL) {
		_bb("CR0_PE(1),CPL(!0)");
		_SetExcept_GP(0);
		_be;
	}
	_adv;
	vcpu.flaghalt = 1;
	_ce;
}
static void CMC()
{
	_cb("CMC");
	i386(0xf5) {
		_adv;
		vcpu.eflags ^= VCPU_EFLAGS_CF;
	} else {
		vcpu.ip++;
		vcpu.eflags ^= VCPU_EFLAGS_CF;
	}
	_ce;
}
static void INS_F6()
{
	_cb("INS_F6");
	i386(0xf6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(vcpuins.crm = _m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* TEST_RM8_I8 */
		_bb("TEST_RM8_I8");
		_chk(_d_imm(1));
		_chk(_a_test(vcpuins.crm, vcpuins.cimm, 8));
		_be;break;
	case 1: /* UndefinedOpcode */
		_bb("ModRM_REG(1)");
		_chk(UndefinedOpcode());
		_be;break;
	case 2: /* NOT_RM8 */
		_bb("NOT_RM8");
		_chk(_a_not(vcpuins.crm, 8));
		_chk(_m_write_rm(vcpuins.result, 1));
		_be;break;
	case 3: /* NEG_RM8 */
		_bb("NEG_RM8");
		_chk(_a_neg(vcpuins.crm, 8));
		_chk(_m_write_rm(vcpuins.result, 1));
		_be;break;
	case 4: /* MUL_RM8 */
		_bb("MUL_RM8");
		_chk(_a_mul(vcpuins.crm, 8));
		_be;break;
	case 5: /* IMUL_RM8 */
		_bb("IMUL_RM8");
		_chk(_a_imul(vcpuins.crm, 8));
		_be;break;
	case 6: /* DIV_RM8 */
		_bb("DIV_RM8");
		_chk(_a_div(vcpuins.crm, 8));
		_be;break;
	case 7: /* IDIV_RM8 */
		_bb("IDIV_RM8");
		_newins_;
		_chk(_a_idiv(vcpuins.crm, 8));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_F7()
{
	_cb("INS_F7");
	i386(0xf7) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* TEST_RM32_I32 */
			_bb("TEST_RM32_I32");
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_test(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM16 */
			_bb("NOT_RM16");
			_chk(_a_not(vcpuins.crm, _GetOperandSize * 8));
			_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
			_be;break;
		case 3: /* NEG_RM16 */
			_bb("NEG_RM16");
			_chk(_a_neg(vcpuins.crm, _GetOperandSize * 8));
			_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
			_be;break;
		case 4: /* MUL_RM16 */
			_bb("MUL_RM16");
			_chk(_a_mul(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 5: /* IMUL_RM16 */
			_bb("IMUL_RM16");
			_chk(_a_imul(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 6: /* DIV_RM16 */
			_bb("DIV_RM16");
			_chk(_a_div(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 7: /* IDIV_RM16 */
			_bb("IDIV_RM16");
			_chk(_a_idiv(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(vcpuins.crm = _m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* TEST_RM16_I16 */
			_bb("TEST_RM16_I16");
			_chk(_d_imm(2));
			_chk(_a_test(vcpuins.crm, vcpuins.cimm, 16));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM16 */
			_bb("NOT_RM16");
			_chk(_a_not(vcpuins.crm, 16));
			_chk(_m_write_rm(vcpuins.result, 2));
			_be;break;
		case 3: /* NEG_RM16 */
			_bb("NEG_RM16");
			_chk(_a_neg(vcpuins.crm, 16));
			_chk(_m_write_rm(vcpuins.result, 2));
			_be;break;
		case 4: /* MUL_RM16 */
			_bb("MUL_RM16");
			_chk(_a_mul(vcpuins.crm, 16));
			_be;break;
		case 5: /* IMUL_RM16 */
			_bb("IMUL_RM16");
			_chk(_a_imul(vcpuins.crm, 16));
			_be;break;
		case 6: /* DIV_RM16 */
			_bb("DIV_RM16");
			_chk(_a_div(vcpuins.crm, 16));
			_be;break;
		case 7: /* IDIV_RM16 */
			_bb("IDIV_RM16");
			_chk(_a_idiv(vcpuins.crm, 16));
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void CLC()
{
	_cb("CLC");
	i386(0xf8) {
		_adv;
		_ClrEFLAGS_CF;
	} else {
		vcpu.ip++;
		_ClrEFLAGS_CF;
	}
	_ce;
}
static void STC()
{
	_cb("STC");
	i386(0xf9) {
		_adv;
		_SetEFLAGS_CF;
	} else {
		vcpu.ip++;
		_SetEFLAGS_CF;
	}
	_ce;
}
static void CLI()
{
	_cb("CLI");
	i386(0xfa) {
		_adv;
		if (!_GetCR0_PE)
			_ClrEFLAGS_IF;
		else {
			_bb("CR0_PE(1)");
			_newins_;
			if (!_GetEFLAGS_VM) {
				_bb("EFLAGS_VM(0)");
				if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
					_ClrEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			} else {
				_bb("EFLAGS_VM(1)");
				if (_GetEFLAGS_IOPL == 3)
					_ClrEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		}
	} else {
		vcpu.ip++;
		_ClrEFLAGS_IF;
	}
	_ce;
}
static void STI()
{
	_cb("STI");
	i386(0xfb) {
		_adv;
		if (!_GetCR0_PE)
			_SetEFLAGS_IF;
		else {
			_bb("CR0_PE(1)");
			_newins_;
			if (!_GetEFLAGS_VM) {
				_bb("EFLAGS_VM(0)");
				if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
					_SetEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			} else {
				_bb("EFLAGS_VM(1)");
				if (_GetEFLAGS_IOPL == 3)
					_SetEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		}
		vcpuins.flagmaskint = 1;
	} else {
		vcpu.ip++;
		_SetEFLAGS_IF;
	}
	_ce;
}
static void CLD()
{
	_cb("CLD");
	i386(0xfc) {
		_adv;
		_ClrEFLAGS_DF;
	} else {
		vcpu.ip++;
		_ClrEFLAGS_DF;
	}
	_ce;
}
static void STD()
{
	_cb("CLD");
	i386(0xfd) {
		_adv;
		_SetEFLAGS_DF;
	} else {
		vcpu.ip++;
		_SetEFLAGS_DF;
	}
	_ce;
}
static void INS_FE()
{
	_cb("INS_FE");
	i386(0xfe) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(vcpuins.crm = _m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* INC_RM8 */
		_bb("INC_RM8");
		_chk(_a_inc(vcpuins.crm, 8));
		_chk(_m_write_rm(vcpuins.result, 1));
		_be;break;
	case 1: /* DEC_RM8 */
		_bb("DEC_RM8");
		_chk(_a_dec(vcpuins.crm, 8));
		_chk(_m_write_rm(vcpuins.result, 1));
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
	t_nubit8 modrm;
	t_nubit16 newcs;
	t_nubit32 neweip;
	_cb("INS_FF");
	i386(0xff) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* INC_RM32 */
			_bb("INC_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			_chk(_a_inc(vcpuins.crm, _GetOperandSize * 8));
			_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
			_be;break;
		case 1: /* DEC_RM32 */
			_bb("DEC_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			_chk(_a_dec(vcpuins.crm, _GetOperandSize * 8));
			_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
			_be;break;
		case 2: /* CALL_RM32 */
			_bb("CALL_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			neweip = GetMax32(vcpuins.crm);
			_chk(_e_call_near(neweip, _GetOperandSize));
			_be;break;
		case 3: /* CALL_M16_32 */
			_bb("CALL_M16_32");
			_chk(_d_modrm(0, _GetOperandSize + 2));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			neweip = (t_nubit32)vcpuins.crm;
			vcpuins.mrm.offset += _GetOperandSize;
			_chk(vcpuins.crm = _m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_call_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 4: /* JMP_RM32 */
			_bb("JMP_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			neweip = GetMax32(vcpuins.crm);
			_chk(_e_jmp_near(neweip, _GetOperandSize));
			_be;break;
		case 5: /* JMP_M16_32 */
			_bb("JMP_M16_32");
			_chk(_d_modrm(0, _GetOperandSize + 2));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			neweip = (t_nubit32)vcpuins.crm;
			vcpuins.mrm.offset += _GetOperandSize;
			_chk(vcpuins.crm = _m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_jmp_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 6: /* PUSH_RM32 */
			_bb("PUSH_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
			_chk(_e_push(vcpuins.crm, _GetOperandSize));
			_be;break;
		case 7: /* UndefinedOpcode */
			_bb("ModRM_REG(7)");
			_chk(UndefinedOpcode());
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* INC_RM16 */
			_bb("INC_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			_chk(_a_inc(vcpuins.crm, 16));
			_chk(_m_write_rm(vcpuins.result, 2));
			_be;break;
		case 1: /* DEC_RM16 */
			_bb("DEC_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			_chk(_a_dec(vcpuins.crm, 16));
			_chk(_m_write_rm(vcpuins.result, 2));
			_be;break;
		case 2: /* CALL_RM16 */
			_bb("CALL_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			neweip = GetMax16(vcpuins.crm);
			_chk(_e_call_near(neweip, 2));
			_be;break;
		case 3: /* CALL_M16_16 */
			_bb("CALL_M16_16");
			_chk(_d_modrm(0, 4));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(vcpuins.crm = _m_read_rm(2));
			neweip = (t_nubit16)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(vcpuins.crm = _m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_call_far(newcs, neweip, 2));
			_be;break;
		case 4: /* JMP_RM16 */
			_bb("JMP_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			neweip = GetMax16(vcpuins.crm);
			_chk(_e_jmp_near(neweip, 2));
			_be;break;
		case 5: /* JMP_M16_16 */
			_bb("JMP_M16_16");
			_chk(_d_modrm(0, 4));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(vcpuins.crm = _m_read_rm(2));
			neweip = (t_nubit16)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(vcpuins.crm = _m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_jmp_far(newcs, neweip, 2));
			_be;break;
		case 6: /* PUSH_RM16 */
			_bb("PUSH_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			_chk(_e_push(vcpuins.crm, 2));
			_be;break;
		case 7: /* UndefinedOpcode */
			_bb("ModRM_REG(7)");
			_chk(UndefinedOpcode());
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}

static void _d_modrm_creg()
{
	_cb("_d_modrm_creg");
	_chk(_kdf_modrm(0, 4));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.cr0; break;
	case 1: _bb("vcpuins.cr(1)");_chk(_SetExcept_UD(0));_be;break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.cr2; break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.cr3; break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: _bb("vcpuins.cr(6)");_chk(_SetExcept_UD(0));_be;break;
	case 7: _bb("vcpuins.cr(7)");_chk(_SetExcept_UD(0));_be;break;
	default:_impossible_;break;}
	_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, 4));
	_ce;
}
static void _d_modrm_dreg()
{
	_cb("_d_modrm_dreg");
	_chk(_kdf_modrm(0, 4));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.dr0; break;
	case 1: vcpuins.rr = (t_vaddrcc)&vcpu.dr1; break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.dr2; break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.dr3; break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.dr6; break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.dr7; break;
	default:_impossible_;break;}
	_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, 4));
	_ce;
}
static void _d_modrm_treg()
{
	_cb("_d_modrm_treg");
	_chk(_kdf_modrm(0, 4));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: _bb("vcpuins.cr(0)");_chk(_SetExcept_UD(0));_be;break;
	case 1: _bb("vcpuins.cr(1)");_chk(_SetExcept_UD(0));_be;break;
	case 2: _bb("vcpuins.cr(2)");_chk(_SetExcept_UD(0));_be;break;
	case 3: _bb("vcpuins.cr(3)");_chk(_SetExcept_UD(0));_be;break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.tr6; break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.tr7; break;
	default:_impossible_;break;}
	_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, 4));
	_ce;
}
static void _d_bit_rmimm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	/* xrm = actual destination, cimm = (1 << bitoffset) */
	t_nsbit16 bitoff16 = 0;
	t_nsbit32 bitoff32 = 0;
	t_nubit32 bitoperand = 0;
	_cb("_d_bit_rmimm");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!regbyte) _chk(_d_imm(1));
	switch (rmbyte) {
	case 2: _bb("rmbyte(2)");
		if (vcpuins.flagmem && regbyte) {
			/* valid for btcc_m16_r16 */
			_bb("flagmem(1),regbyte(1)");
			bitoff16 = (t_nsbit16)vcpuins.cr;
			if (bitoff16 >= 0)
				vcpuins.mrm.offset += 2 * (bitoff16 / 16);
			else
				vcpuins.mrm.offset += 2 * ((bitoff16 - 15) / 16);
			bitoperand = ((t_nubit16)bitoff16) % 16;
			_be;
		} else if (regbyte) {
			bitoperand = vcpuins.cr % 16;
		} else {
			bitoperand = (GetMax16(vcpuins.cimm) % 16);
		}
		_chk(vcpuins.cimm = GetMax16((1 << bitoperand)));
		_be;break;
	case 4: _bb("rmbyte(4)");
		if (vcpuins.flagmem && regbyte) {
			_bb("flagmem(1),regbyte(1)");
			bitoff32 = (t_nsbit32)vcpuins.cr;
			if (bitoff32 >= 0)
				vcpuins.mrm.offset += 4 * (bitoff32 / 32);
			else
				vcpuins.mrm.offset += 4 * ((bitoff32 - 31) / 32);
			bitoperand = ((t_nubit32)bitoff32) % 32;
			_be;
		} else if (regbyte) {
			bitoperand = vcpuins.cr % 32;
		} else {
			bitoperand = (GetMax32(vcpuins.cimm) % 32);
		}
		_chk(vcpuins.cimm = GetMax32((1 << bitoperand)));
		_be;break;
	default: _bb("rmbyte");
		_chk(_SetExcept_CE(rmbyte));
		_be;break;
	}
	_ce;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc \
do { \
	switch (bit) { \
	case 16: vcpuins.opr1 = GetMax16(cdest);break; \
	case 32: vcpuins.opr1 = GetMax32(cdest);break; \
	default: _bb("bit"); \
		_chk(_SetExcept_CE(bit)); \
		_be;break; \
	} \
	vcpuins.opr2 = bitoperand; \
	vcpuins.result = vcpuins.opr1; \
	MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(vcpuins.opr1, vcpuins.opr2)); \
} while (0)

static void _a_bscc(t_nubit64 csrc, t_nubit8 bit, t_bool forward)
{
	t_nubit32 temp;
	_cb("_a_bscc");
	if (forward) temp = 0;
	else temp = bit - 1;
	switch (bit) {
	case 16: _bb("bit(16)");
		vcpuins.opr1 = GetMax16(csrc);
		if (!vcpuins.opr1)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(vcpuins.opr1, GetMax64(1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			vcpuins.result = GetMax16(temp);
		}
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.opr1 = GetMax32(csrc);
		if (!vcpuins.opr1)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(vcpuins.opr1, GetMax64(1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			vcpuins.result = GetMax32(temp);
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_bt(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bt");
	_kac_btcc;
	_ce;
}
static void _a_btc(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btc");
	_kac_btcc;
	MakeBit(vcpuins.result, vcpuins.opr2, !_GetEFLAGS_CF);
	_ce;
}
static void _a_btr(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btr");
	_kac_btcc;
	ClrBit(vcpuins.result, vcpuins.opr2);
	_ce;
}
static void _a_bts(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bts");
	_kac_btcc;
	SetBit(vcpuins.result, vcpuins.opr2);
	_ce;
}

static void _a_imul2(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	t_nubit64 temp;
	_cb("_a_imul2");
	switch (bit) {
	case 16: _bb("bit(16+16)");
		_newins_;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16((t_nsbit16)cdest);
		vcpuins.opr2 = GetMax16((t_nsbit16)csrc);
		temp = GetMax32((t_nsbit16)vcpuins.opr1 * (t_nsbit16)vcpuins.opr2);
		vcpuins.result = GetMax16(temp);
		if (GetMax32(temp) != GetMax32((t_nsbit16)vcpuins.result)) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 32: _bb("bit(32+32");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32((t_nsbit32)cdest);
		vcpuins.opr2 = GetMax32((t_nsbit32)csrc);
		temp = GetMax64((t_nsbit32)vcpuins.opr1 * (t_nsbit32)vcpuins.opr2);
		vcpuins.result = GetMax32(temp);
		if (GetMax64(temp) != GetMax64((t_nsbit32)vcpuins.result)) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_shld(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf;
	t_bool flagbit;
	t_nsbit32 i;
	_cb("_a_shld");
	count &= 0x1f;
	if (!count) {
		_ce;
		return;
	}
	if (count > bit) {
		/* bad parameters */
		/* dest is undefined */
		/* cf, of, sf, zf, af, pf are undefined */
		vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
			VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
	} else {
		switch (bit) {
		case 16: _bb("bit(16)");
			vcpuins.bit = 16;
			vcpuins.opr1 = GetMax16(cdest);
			vcpuins.opr2 = GetMax16(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB16(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i - count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i - count + bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		case 32: _bb("bit(32)");
			vcpuins.bit = 32;
			vcpuins.opr1 = GetMax32(cdest);
			vcpuins.opr2 = GetMax32(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB32(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i - count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i - count + bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		default: _bb("bit");
			_chk(_SetExcept_CE(bit));
			_be;break;
		}
		_chk(_kaf_set_flags(SHLD_FLAG));
		vcpuins.udf |= VCPU_EFLAGS_AF;
	}
	_ce;
}
static void _a_shrd(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf;
	t_bool flagbit;
	t_nsbit32 i;
	_cb("_a_shrd");
	count &= 0x1f;
	if (!count) {
		_ce;
		return;
	}
	if (count > bit) {
		/* bad parameters */
		/* dest is undefined */
		/* cf, of, sf, zf, af, pf are undefined */
		vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
			VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
	} else {
		switch (bit) {
		case 16: _bb("bit(16)");
			vcpuins.bit = 16;
			vcpuins.opr1 = GetMax16(cdest);
			vcpuins.opr2 = GetMax16(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB16(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i + count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i + count - bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		case 32: _bb("bit(32)");
			vcpuins.bit = 32;
			vcpuins.opr1 = GetMax32(cdest);
			vcpuins.opr2 = GetMax32(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB32(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i + count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i + count - bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		default: _bb("bit");
			_chk(_SetExcept_CE(bit));
			_be;break;
		}
		_chk(_kaf_set_flags(SHRD_FLAG));
		vcpuins.udf |= VCPU_EFLAGS_AF;
	}
	_ce;
}

static void _m_setcc_rm(t_bool condition)
{
	_cb("_m_setcc_rm");
	vcpuins.result = GetMax8(!!condition);
	_chk(_m_write_rm(vcpuins.result, 1));
	_ce;
}

static void INS_0F_00()
{
	t_nubit8 modrm;
	t_nubit64 descriptor;
	_cb("INS_0F_00");
	_adv;
	if (_IsProtected) {
		_bb("Protected");
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* SLDT_RM16 */
			_bb("SLDT_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
			_chk(_m_write_rm(GetMax16(vcpu.ldtr.selector), vcpuins.flagmem ? 2 : _GetOperandSize));
			_be;break;
		case 1: /* STR_RM16 */
			_bb("STR_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
			_chk(_m_write_rm(GetMax16(vcpu.tr.selector), vcpuins.flagmem ? 2 : _GetOperandSize));
			_be;break;
		case 2: /* LLDT_RM16 */
			_bb("LLDT_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			_chk(_s_load_ldtr(GetMax16(vcpuins.crm)));
			_be;break;
		case 3: /* LTR_RM16 */
			_bb("LTR_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			_chk(_s_load_tr(GetMax16(vcpuins.crm)));
			_be;break;
		case 4: /* VERR_RM16 */
			_bb("VERR_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_bb("selector(valid)");
				_chk(descriptor = _s_read_xdt(GetMax16(vcpuins.crm)));
				if (_IsDescSys(descriptor) ||
						(!_IsDescCodeConform(descriptor) &&
							(_GetCPL > _GetDesc_DPL(descriptor) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(descriptor)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescData(descriptor) || _IsDescCodeReadable(descriptor)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
				_be;
			}
			_be;break;
		case 5: /* VERW_RM16 */
			_bb("VERW_RM16");
			_chk(_d_modrm(0, 2));
			_chk(vcpuins.crm = _m_read_rm(2));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_bb("selector(valid)");
				_chk(descriptor = _s_read_xdt(GetMax16(vcpuins.crm)));
				if (_IsDescSys(descriptor) ||
						(!_IsDescCodeConform(descriptor) &&
							(_GetCPL > _GetDesc_DPL(descriptor) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(descriptor)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescDataWridtable(descriptor)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
				_be;
			}
			_be;break;
		case 6: _bb("ModRM_REG(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
		_be;
	} else {
		_bb("!Protected");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
static void INS_0F_01()
{
	t_nubit8 modrm;
	t_nubit16 limit;
	t_nubit32 base;
	_cb("INS_0F_01");
	_adv;
	_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
	switch (_GetModRM_REG(modrm)) {
	case 0: /* SGDT_M32_16 */
		_bb("SGDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_write_rm(vcpu.gdtr.limit, 2));
		vcpuins.mrm.offset += 2;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_m_write_rm(GetMax24(vcpu.gdtr.base), 4));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_m_write_rm(GetMax32(vcpu.gdtr.base), 4));
			_be;break;
		default:_impossible_;break;}
		_be;break;
	case 1: /* SIDT_M32_16 */
		_bb("SIDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_write_rm(vcpu.gdtr.limit, 2));
		vcpuins.mrm.offset += 2;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_m_write_rm(GetMax24(vcpu.idtr.base), 4));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_m_write_rm(GetMax32(vcpu.idtr.base), 4));
			_be;break;
		default:_impossible_;break;}
		_be;break;
	case 2: /* LGDT_M32_16 */
		_bb("LGDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(vcpuins.crm = _m_read_rm(2));
		limit = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(vcpuins.crm = _m_read_rm(4));
		switch (_GetOperandSize) {
		case 2: base = GetMax24(vcpuins.crm);break;
		case 4: base = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		_comment("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
			vcpurec.linear, base, limit);
		_chk(_s_load_gdtr(base, limit, _GetOperandSize));
		_be;break;
	case 3: /* LIDT_M32_16 */
		_bb("LIDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(vcpuins.crm = _m_read_rm(2));
		limit = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(vcpuins.crm = _m_read_rm(4));
		switch (_GetOperandSize) {
		case 2: base = GetMax24(vcpuins.crm);break;
		case 4: base = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		_comment("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
			vcpurec.linear, base, limit);
		_chk(_s_load_idtr(base, limit, _GetOperandSize));
		_be;break;
	case 4: /* SMSW_RM16 */
		_bb("SMSW_RM16");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
		if (_GetOperandSize == 4 && !vcpuins.flagmem)
			_chk(_m_write_rm(GetMax16(vcpu.cr0), 4));
		else
			_chk(_m_write_rm(GetMax16(vcpu.cr0), 2));
		_be;break;
	case 5: _bb("ModRM_REG(5)");_chk(UndefinedOpcode());_be;break;
	case 6: /* LMSW_RM16 */
		_bb("LMSW_RM16");
		_chk(_d_modrm(0, 2));
		_chk(vcpuins.crm = _m_read_rm(2));
		_chk(_s_load_cr0_msw(GetMax16(vcpuins.crm)));
		_be;break;
	case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void LAR_R32_RM32()
{
	t_nubit16 selector;
	t_nubit64 descriptor;
	_cb("LAR_R32_RM32");
	_newins_;
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(vcpuins.crm = GetMax16(_m_read_rm(2)));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_bb("selector(valid)");
			_chk(descriptor = _s_read_xdt(selector));
			if (_IsDescUser(descriptor)) {
				if (_IsDescCodeConform(descriptor))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(descriptor)) {
				case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
				case VCPU_DESC_SYS_TYPE_LDT:
				case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
				case VCPU_DESC_SYS_TYPE_CALLGATE_16:
				case VCPU_DESC_SYS_TYPE_TASKGATE:
				case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
				case VCPU_DESC_SYS_TYPE_CALLGATE_32:
					_SetEFLAGS_ZF;
					break;
				default:
					_ClrEFLAGS_ZF;
				}
			}
			if (_GetEFLAGS_ZF) {
				_bb("EFLAGS_ZF");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_m_write_ref(vcpuins.rr, (GetMax16(descriptor >> 32) & 0xff00), 2));
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_m_write_ref(vcpuins.rr, (GetMax32(descriptor >> 32) & 0x00ffff00), 4));
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
static void LSL_R32_RM32()
{
	t_nubit16 selector;
	t_nubit32 limit;
	t_nubit64 descriptor;
	_cb("LSL_R32_RM32");
	_newins_;
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(vcpuins.crm = GetMax16(_m_read_rm(2)));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_bb("selector(valid)");
			_chk(descriptor = _s_read_xdt(selector));
			if (_IsDescUser(descriptor)) {
				if (_IsDescCodeConform(descriptor))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(descriptor)) {
				case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
				case VCPU_DESC_SYS_TYPE_LDT:
				case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
				case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
					_SetEFLAGS_ZF;
					break;
				default:
					_ClrEFLAGS_ZF;
				}
			}
			if (_GetEFLAGS_ZF) {
				_bb("EFLAGS_ZF");
				limit = _IsDescSegGranularLarge(descriptor) ?
					((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : _GetDescSeg_Limit(descriptor);
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_m_write_ref(vcpuins.rr, GetMax16(limit), 2));
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_m_write_ref(vcpuins.rr, GetMax32(limit), 4));
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
static void CLTS()
{
	_cb("CLTS");
	_newins_;
	_adv;
	if (!_GetCR0_PE) {
		_ClrCR0_TS;
	} else {
		_bb("CR0_PE(1)");
		if (_GetCPL > 0)
			_chk(_SetExcept_GP(0));
		else
			_ClrCR0_TS;
		_be;
	}
	_ce;
}
static void WBINVD()
{
	_cb("WBINVD");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void MOV_R32_CR()
{
	_cb("MOV_R32_CR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_creg());
	_chk(_m_write_ref(vcpuins.rrm, vcpuins.cr, 4));
	_ce;
}
static void MOV_R32_DR()
{
	_cb("MOV_R32_DR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_dreg());
	_chk(_m_write_ref(vcpuins.rrm, vcpuins.cr, 4));
	_ce;
}
static void MOV_CR_R32()
{
	_cb("MOV_CR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_creg());
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, 4));
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr0) _comment("MOV_CR_R32: executed at L%08X, CR0=%08X\n", vcpurec.linear, vcpu.cr0);
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr2) _comment("MOV_CR_R32: executed at L%08X, CR2=%08X\n", vcpurec.linear, vcpu.cr2);
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr3) _comment("MOV_CR_R32: executed at L%08X, CR3=%08X\n", vcpurec.linear, vcpu.cr3);
	_ce;
}
static void MOV_DR_R32()
{
	_cb("MOV_DR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_dreg());
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, 4));
	_ce;
}
static void MOV_R32_TR()
{
	_cb("MOV_R32_TR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_treg());
	_chk(_m_write_ref(vcpuins.rrm, vcpuins.cr, 4));
	_ce;
}
static void MOV_TR_R32()
{
	_cb("MOV_TR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_treg());
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, 4));
	_ce;
}
static void WRMSR()
{
	_cb("WRMSR");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void RDMSR()
{
	_cb("RDMSR");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void JO_REL32()
{
	_cb("JO_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_OF));
	_ce;
}
static void JNO_REL32()
{
	_cb("JNO_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_OF));
	_ce;
}
static void JC_REL32()
{
	_cb("JC_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_CF));
	_ce;
}
static void JNC_REL32()
{
	_cb("JNC_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_CF));
	_ce;
}
static void JZ_REL32()
{
	_cb("JZ_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_ZF));
	_ce;
}
static void JNZ_REL32()
{
	_cb("JNZ_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
	_ce;
}
static void JNA_REL32()
{
	_cb("JNA_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JA_REL32()
{
	_cb("JA_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JS_REL32()
{
	_cb("JS_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_SF));
	_ce;
}
static void JNS_REL32()
{
	_cb("JNS_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_SF));
	_ce;
}
static void JP_REL32()
{
	_cb("JP_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_PF));
	_ce;
}
static void JNP_REL32()
{
	_cb("JNP_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_PF));
	_ce;
}
static void JL_REL32()
{
	_cb("JL_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
static void JNL_REL32()
{
	_cb("JNL_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
static void JNG_REL32()
{
	_cb("JNG_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
			(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	_ce;
}
static void JG_REL32()
{
	_cb("JG_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	_ce;
}
static void SETO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_OF));
	_ce;
}
static void SETNO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_OF));
	_ce;
}
static void SETC_RM8()
{
	_cb("SETC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_CF));
	_ce;
}
static void SETNC_RM8()
{
	_cb("SETNC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_CF));
	_ce;
}
static void SETZ_RM8()
{
	_cb("SETZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_ZF));
	_ce;
}
static void SETNZ_RM8()
{
	_cb("SETNZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_ZF));
	_ce;
}
static void SETNA_RM8()
{
	_cb("SETNA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_CF || _GetEFLAGS_ZF));
	_ce;
}
static void SETA_RM8()
{
	_cb("SETA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void SETS_RM8()
{
	_cb("SETS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF));
	_ce;
}
static void SETNS_RM8()
{
	_cb("SETNS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_SF));
	_ce;
}
static void SETP_RM8()
{
	_cb("SETP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_PF));
	_ce;
}
static void SETNP_RM8()
{
	_cb("SETNP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_PF));
	_ce;
}
static void SETL_RM8()
{
	_cb("SETL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF != _GetEFLAGS_OF));
	_ce;
}
static void SETNL_RM8()
{
	_cb("SETNL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF == _GetEFLAGS_OF));
	_ce;
}
static void SETNG_RM8()
{
	_cb("SETNG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
static void SETG_RM8()
{
	_cb("SETG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
static void PUSH_FS()
{
	_cb("PUSH_FS");
	_newins_;
	_adv;
	_chk(_e_push(vcpu.fs.selector, _GetOperandSize));
	_ce;
}
static void POP_FS()
{
	t_nubit16 sel;
	_cb("POP_FS");
	_newins_;
	_adv;
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_fs(sel));
	_ce;
}
static void CPUID()
{
	_cb("CPUID");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void BT_RM32_R32()
{
	_cb("BT_RM32_R32");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 0));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_bt((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
static void SHLD_RM32_R32_I8()
{
	_cb("SHLD_RM32_R32_I8");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_d_imm(1));
	_chk(_a_shld(vcpuins.crm, vcpuins.cr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void SHLD_RM32_R32_CL()
{
	_cb("SHLD_RM32_R32_CL");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_shld(vcpuins.crm, vcpuins.cr, vcpu.cl, _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void PUSH_GS()
{
	_cb("PUSH_GS");
	_newins_;
	_adv;
	_chk(_e_push(vcpu.gs.selector, _GetOperandSize));
	_ce;
}
static void POP_GS()
{
	t_nubit16 sel;
	_cb("POP_GS");
	_newins_;
	_adv;
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_gs(sel));
	_ce;
}
static void RSM()
{
	_cb("RSM");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void BTS_RM32_R32()
{
	_cb("BTS_RM32_R32");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_bts((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void SHRD_RM32_R32_I8()
{
	_cb("SHRD_RM32_R32_I8");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_d_imm(1));
	_chk(_a_shrd(vcpuins.crm, vcpuins.cr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void SHRD_RM32_R32_CL()
{
	_cb("SHRD_RM32_R32_CL");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_shrd(vcpuins.crm, vcpuins.cr, vcpu.cl, _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void IMUL_R32_RM32()
{
	_cb("IMUL_R32_RM32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_imul2(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
	_chk(_m_write_ref(vcpuins.rr, vcpuins.result, _GetOperandSize));
	_ce;
}
static void LSS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LSS_R32_M16_32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(vcpuins.crm = _m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.ss, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void BTR_RM32_R32()
{
	_cb("BTR_RM32_R32");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_btr((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void LFS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LFS_R32_M16_32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(vcpuins.crm = _m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.fs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void LGS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LGS_R32_M16_32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(vcpuins.crm = _m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.gs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void INS_0F_BA()
{
	t_bool write = 0;
	t_nubit8 modrm = 0x00;
	_cb("INS_0F_BA");
	_newins_;
	_adv;
	_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
	if (_GetModRM_REG(modrm) == 4) write = 0;
	else write = 1;
	_chk(_d_bit_rmimm(0, _GetOperandSize, write));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	switch (vcpuins.cr) {
	case 0: _bb("cr(0)");_chk(UndefinedOpcode());_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: /* BT_RM32_I8 */
		_bb("BT_RM32_I8");
		_chk(_a_bt((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	case 5: /* BTS_RM32_I8 */
		_bb("BTS_RM32_I8");
		_chk(_a_bts((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
		_be;break;
	case 6: /* BTR_RM32_I8 */
		_bb("BTR_RM32_I8");
		_chk(_a_btr((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
		_be;break;
	case 7: /* BTC_RM32_I8 */
		_bb("BTC_RM32_I8");
		_chk(_a_btc((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void BTC_RM32_R32()
{
	_cb("BTC_RM32_R32");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_btc((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_chk(_m_write_rm(vcpuins.result, _GetOperandSize));
	_ce;
}
static void BSF_R32_RM32()
{
	_cb("BSF_R32_RM32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_bscc(vcpuins.crm, _GetOperandSize * 8, 1));
	if (!_GetEFLAGS_ZF) {
		_bb("EFLAGS_ZF(0)");
		_chk(_m_write_ref(vcpuins.rr, vcpuins.result, _GetOperandSize));
		_be;
	}
	_ce;
}
static void BSR_R32_RM32()
{
	_cb("BSR_R32_RM32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(vcpuins.crm = _m_read_rm(_GetOperandSize));
	_chk(_a_bscc(vcpuins.crm, _GetOperandSize * 8, 0));
	if (!_GetEFLAGS_ZF) {
		_bb("EFLAGS_ZF(0)");
		_chk(_m_write_ref(vcpuins.rr, vcpuins.result, _GetOperandSize));
		_be;
	}
	_ce;
}
static void MOVZX_R32_RM8()
{
	_cb("MOVZX_R32_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1));
	_chk(vcpuins.crm = (t_nubit8)_m_read_rm(1));
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, _GetOperandSize));
	_ce;
}
static void MOVZX_R32_RM16()
{
	_cb("MOVZX_R32_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(4, 2));
	_chk(vcpuins.crm = (t_nubit16)_m_read_rm(2));
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, 4));
	_ce;
}
static void MOVSX_R32_RM8()
{
	_cb("MOVSX_R32_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1));
	_chk(vcpuins.crm = (t_nsbit8)_m_read_rm(1));
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, _GetOperandSize));
	_ce;
}
static void MOVSX_R32_RM16()
{
	_cb("MOVSX_R32_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(4, 2));
	_chk(vcpuins.crm = (t_nsbit16)_m_read_rm(2));
	_chk(_m_write_ref(vcpuins.rr, vcpuins.crm, 4));
	_ce;
}


static void RecInit()
{
	vcpurec.msize = 0;
	vcpurec.rcpu = vcpu;
	vcpurec.linear = _kma_linear_logical(&vcpu.cs, vcpu.eip, 1, 0x00, 0, 1);
	if (vcpuins.flagwe && vcpuins.welin == vcpurec.linear) {
		vapiPrint("L%08x: EXECUTED\n", vcpurec.linear);
		vapiCallBackDebugPrintRegs(1);
	}
	if (vcpuins.except) vcpurec.linear = 0xcccccccc;
#ifndef VGLOBAL_BOCHS
	vcpurec.opcode = _kma_read_logical(&vcpu.cs, vcpu.eip, 0x08, 0, 1);
	if (vcpuins.except) vcpurec.opcode = 0xcccccccccccccccc;
	if (vcpu.esp + 7 > vcpu.ss.limit)
		vcpurec.stack = _kma_read_logical(&vcpu.ss, vcpu.esp, vcpu.ss.limit - vcpu.esp + 1, 0, 1);
	else
		vcpurec.stack = _kma_read_logical(&vcpu.ss, vcpu.esp, 0x08, 0, 1);
	if (vcpuins.except) vcpurec.stack = 0xcccccccccccccccc;
#endif
}
static void RecFinal()
{}
static void ExecInit()
{
	dop[0] = 0;
	dopr[0] = 0;
	SPRINTF(doverds, "DS");
	SPRINTF(doverss, "SS");
	prefix_oprsize = 0x00;
	prefix_addrsize = 0x00;
	flagmem = 0;

	vcpuins.prefix_rep = PREFIX_REP_NONE;
	vcpuins.prefix_lock = 0x00;
}
static void ExecFinal()
{}
static void ExecIns()
{
	static t_bool y = 0;
	t_nubit8 opcode;
	RecInit();
	ExecInit();
	do {
		_cb("ExecIns");
		_chb(opcode = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		_chb(ExecFun(dtable[opcode]));
		_chb(_s_test_eip());
		_chb(_s_test_esp());
		_ce;
	} while (_kdf_check_prefix(opcode));
	if (!y && _GetCR0_PE && _GetEFLAGS_VM) {
		_comment("enter v86!\n");
		y = 1;
	}
	ExecFinal();
	RecFinal();
} 
static void ExecInt()
{
#ifndef VGLOBAL_BOCHS
	t_nubit8 intr = 0x00;
	/* hardware interrupt handeler */
	if (vcpuins.flagmaskint)
		return;
	if(!vcpu.flagmasknmi && vcpu.flagnmi) {
		vcpu.flaghalt = 0;
		vcpu.flagnmi = 0;
		ExecInit();
		_e_intr_n(0x02, _GetOperandSize);
		ExecFinal();
	}
	if (_GetEFLAGS_IF && vpicScanINTR()) {
		vcpu.flaghalt = 0;
		intr = vpicGetINTR();
		/*printf("vint = %x\n", intr);*/
		ExecInit();
		_e_intr_n(intr, _GetOperandSize);
		ExecFinal();
	}
	if (_GetEFLAGS_TF) {
		vcpu.flaghalt = 0;
		ExecInit();
		_e_intr_n(0x01, _GetOperandSize);
		ExecFinal();
	}
#endif
}

static void QDX()
{
#ifndef VGLOBAL_BOCHS
	t_nubit32 eflags;
	_cb("QDX");
	_adv;
	_chk(_d_imm(1));
	switch (GetMax8(vcpuins.cimm)) {
	case 0x00:
	case 0xff: /* STOP */
		vapiPrint("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe: /* RESET */
		vapiPrint("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	case 0x02:
		vcpurec.svcextl++;
		break;
	case 0x03:
		vcpurec.svcextl--;
		break;
	default: /* QDINT */
		_bb("QDINT");
		qdbiosExecInt(GetMax8(vcpuins.cimm));
		if (vcpuins.cimm < 0x80) {
			_chk(eflags = (t_nubit32)_s_read_ss(vcpu.sp + 4, 4));
			MakeBit(eflags, VCPU_EFLAGS_ZF, _GetEFLAGS_ZF);
			MakeBit(eflags, VCPU_EFLAGS_CF, _GetEFLAGS_CF);
			_chk(_s_write_ss(vcpu.sp + 4, eflags, 4));
		}
		_be;break;
	}
	_ce;
#endif
}

void dasm(t_string stmt, t_vaddrcc rcode)
{
	dtable[0x00] = (t_faddrcc)ADD_RM8_R8;
	dtable[0x01] = (t_faddrcc)ADD_RM32_R32;
	dtable[0x02] = (t_faddrcc)ADD_R8_RM8;
	dtable[0x03] = (t_faddrcc)ADD_R32_RM32;
	dtable[0x04] = (t_faddrcc)ADD_AL_I8;
	dtable[0x05] = (t_faddrcc)ADD_EAX_I32;
	dtable[0x06] = (t_faddrcc)PUSH_ES;
	dtable[0x07] = (t_faddrcc)POP_ES;
	dtable[0x08] = (t_faddrcc)OR_RM8_R8;
	dtable[0x09] = (t_faddrcc)OR_RM32_R32;
	dtable[0x0a] = (t_faddrcc)OR_R8_RM8;
	dtable[0x0b] = (t_faddrcc)OR_R32_RM32;
	dtable[0x0c] = (t_faddrcc)OR_AL_I8;
	dtable[0x0d] = (t_faddrcc)OR_EAX_I32;
	dtable[0x0e] = (t_faddrcc)PUSH_CS;
	dtable[0x0f] = (t_faddrcc)INS_0F;//
	dtable[0x10] = (t_faddrcc)ADC_RM8_R8;
	dtable[0x11] = (t_faddrcc)ADC_RM32_R32;
	dtable[0x12] = (t_faddrcc)ADC_R8_RM8;
	dtable[0x13] = (t_faddrcc)ADC_R32_RM32;
	dtable[0x14] = (t_faddrcc)ADC_AL_I8;
	dtable[0x15] = (t_faddrcc)ADC_EAX_I32;
	dtable[0x16] = (t_faddrcc)PUSH_SS;
	dtable[0x17] = (t_faddrcc)POP_SS;
	dtable[0x18] = (t_faddrcc)SBB_RM8_R8;
	dtable[0x19] = (t_faddrcc)SBB_RM32_R32;
	dtable[0x1a] = (t_faddrcc)SBB_R8_RM8;
	dtable[0x1b] = (t_faddrcc)SBB_R32_RM32;
	dtable[0x1c] = (t_faddrcc)SBB_AL_I8;
	dtable[0x1d] = (t_faddrcc)SBB_EAX_I32;
	dtable[0x1e] = (t_faddrcc)PUSH_DS;
	dtable[0x1f] = (t_faddrcc)POP_DS;
	dtable[0x20] = (t_faddrcc)AND_RM8_R8;
	dtable[0x21] = (t_faddrcc)AND_RM32_R32;
	dtable[0x22] = (t_faddrcc)AND_R8_RM8;
	dtable[0x23] = (t_faddrcc)AND_R32_RM32;
	dtable[0x24] = (t_faddrcc)AND_AL_I8;
	dtable[0x25] = (t_faddrcc)AND_EAX_I32;
	dtable[0x26] = (t_faddrcc)PREFIX_ES;
	dtable[0x27] = (t_faddrcc)DAA;
	dtable[0x28] = (t_faddrcc)SUB_RM8_R8;
	dtable[0x29] = (t_faddrcc)SUB_RM32_R32;
	dtable[0x2a] = (t_faddrcc)SUB_R8_RM8;
	dtable[0x2b] = (t_faddrcc)SUB_R32_RM32;
	dtable[0x2c] = (t_faddrcc)SUB_AL_I8;
	dtable[0x2d] = (t_faddrcc)SUB_EAX_I32;
	dtable[0x2e] = (t_faddrcc)PREFIX_CS;
	dtable[0x2f] = (t_faddrcc)DAS;
	dtable[0x30] = (t_faddrcc)XOR_RM8_R8;
	dtable[0x31] = (t_faddrcc)XOR_RM32_R32;
	dtable[0x32] = (t_faddrcc)XOR_R8_RM8;
	dtable[0x33] = (t_faddrcc)XOR_R32_RM32;
	dtable[0x34] = (t_faddrcc)XOR_AL_I8;
	dtable[0x35] = (t_faddrcc)XOR_EAX_I32;
	dtable[0x36] = (t_faddrcc)PREFIX_SS;
	dtable[0x37] = (t_faddrcc)AAA;
	dtable[0x38] = (t_faddrcc)CMP_RM8_R8;
	dtable[0x39] = (t_faddrcc)CMP_RM32_R32;
	dtable[0x3a] = (t_faddrcc)CMP_R8_RM8;
	dtable[0x3b] = (t_faddrcc)CMP_R32_RM32;
	dtable[0x3c] = (t_faddrcc)CMP_AL_I8;
	dtable[0x3d] = (t_faddrcc)CMP_EAX_I32;
	dtable[0x3e] = (t_faddrcc)PREFIX_DS;
	dtable[0x3f] = (t_faddrcc)AAS;
	dtable[0x40] = (t_faddrcc)INC_EAX;
	dtable[0x41] = (t_faddrcc)INC_ECX;
	dtable[0x42] = (t_faddrcc)INC_EDX;
	dtable[0x43] = (t_faddrcc)INC_EBX;
	dtable[0x44] = (t_faddrcc)INC_ESP;
	dtable[0x45] = (t_faddrcc)INC_EBP;
	dtable[0x46] = (t_faddrcc)INC_ESI;
	dtable[0x47] = (t_faddrcc)INC_EDI;
	dtable[0x48] = (t_faddrcc)DEC_EAX;
	dtable[0x49] = (t_faddrcc)DEC_ECX;
	dtable[0x4a] = (t_faddrcc)DEC_EDX;
	dtable[0x4b] = (t_faddrcc)DEC_EBX;
	dtable[0x4c] = (t_faddrcc)DEC_ESP;
	dtable[0x4d] = (t_faddrcc)DEC_EBP;
	dtable[0x4e] = (t_faddrcc)DEC_ESI;
	dtable[0x4f] = (t_faddrcc)DEC_EDI;
	dtable[0x50] = (t_faddrcc)PUSH_EAX;
	dtable[0x51] = (t_faddrcc)PUSH_ECX;
	dtable[0x52] = (t_faddrcc)PUSH_EDX;
	dtable[0x53] = (t_faddrcc)PUSH_EBX;
	dtable[0x54] = (t_faddrcc)PUSH_ESP;
	dtable[0x55] = (t_faddrcc)PUSH_EBP;
	dtable[0x56] = (t_faddrcc)PUSH_ESI;
	dtable[0x57] = (t_faddrcc)PUSH_EDI;
	dtable[0x58] = (t_faddrcc)POP_EAX;
	dtable[0x59] = (t_faddrcc)POP_ECX;
	dtable[0x5a] = (t_faddrcc)POP_EDX;
	dtable[0x5b] = (t_faddrcc)POP_EBX;
	dtable[0x5c] = (t_faddrcc)POP_ESP;
	dtable[0x5d] = (t_faddrcc)POP_EBP;
	dtable[0x5e] = (t_faddrcc)POP_ESI;
	dtable[0x5f] = (t_faddrcc)POP_EDI;
	dtable[0x60] = (t_faddrcc)PUSHA;//
	dtable[0x61] = (t_faddrcc)POPA;//
	dtable[0x62] = (t_faddrcc)BOUND_R16_M16_16;//
	dtable[0x63] = (t_faddrcc)ARPL_RM16_R16;//
	dtable[0x64] = (t_faddrcc)PREFIX_FS;//
	dtable[0x65] = (t_faddrcc)PREFIX_GS;//
	dtable[0x66] = (t_faddrcc)PREFIX_OprSize;//
	dtable[0x67] = (t_faddrcc)PREFIX_AddrSize;//
	dtable[0x68] = (t_faddrcc)PUSH_I32;//
	dtable[0x69] = (t_faddrcc)IMUL_R32_RM32_I32;//
	dtable[0x6a] = (t_faddrcc)PUSH_I8;//
	dtable[0x6b] = (t_faddrcc)IMUL_R32_RM32_I8;//
	dtable[0x6c] = (t_faddrcc)INSB;//
	dtable[0x6d] = (t_faddrcc)INSW;//
	dtable[0x6e] = (t_faddrcc)OUTSB;
	dtable[0x6f] = (t_faddrcc)OUTSW;
	dtable[0x70] = (t_faddrcc)JO_REL8;
	dtable[0x71] = (t_faddrcc)JNO_REL8;
	dtable[0x72] = (t_faddrcc)JC_REL8;
	dtable[0x73] = (t_faddrcc)JNC_REL8;
	dtable[0x74] = (t_faddrcc)JZ_REL8;
	dtable[0x75] = (t_faddrcc)JNZ_REL8;
	dtable[0x76] = (t_faddrcc)JNA_REL8;
	dtable[0x77] = (t_faddrcc)JA_REL8;
	dtable[0x78] = (t_faddrcc)JS_REL8;
	dtable[0x79] = (t_faddrcc)JNS_REL8;
	dtable[0x7a] = (t_faddrcc)JP_REL8;
	dtable[0x7b] = (t_faddrcc)JNP_REL8;
	dtable[0x7c] = (t_faddrcc)JL_REL8;
	dtable[0x7d] = (t_faddrcc)JNL_REL8;
	dtable[0x7e] = (t_faddrcc)JNG_REL8;
	dtable[0x7f] = (t_faddrcc)JG_REL8;
	dtable[0x80] = (t_faddrcc)INS_80;
	dtable[0x81] = (t_faddrcc)INS_81;
	dtable[0x82] = (t_faddrcc)UndefinedOpcode;
	dtable[0x83] = (t_faddrcc)INS_83;
	dtable[0x84] = (t_faddrcc)TEST_RM8_R8;
	dtable[0x85] = (t_faddrcc)TEST_RM32_R32;
	dtable[0x86] = (t_faddrcc)XCHG_RM8_R8;
	dtable[0x87] = (t_faddrcc)XCHG_RM32_R32;
	dtable[0x88] = (t_faddrcc)MOV_RM8_R8;
	dtable[0x89] = (t_faddrcc)MOV_RM32_R32;
	dtable[0x8a] = (t_faddrcc)MOV_R8_RM8;
	dtable[0x8b] = (t_faddrcc)MOV_R32_RM32;
	dtable[0x8c] = (t_faddrcc)MOV_RM16_SREG;
	dtable[0x8d] = (t_faddrcc)LEA_R16_M16;
	dtable[0x8e] = (t_faddrcc)MOV_SREG_RM16;
	dtable[0x8f] = (t_faddrcc)INS_8F;
	dtable[0x90] = (t_faddrcc)NOP;
	dtable[0x91] = (t_faddrcc)XCHG_ECX_EAX;
	dtable[0x92] = (t_faddrcc)XCHG_EDX_EAX;
	dtable[0x93] = (t_faddrcc)XCHG_EBX_EAX;
	dtable[0x94] = (t_faddrcc)XCHG_ESP_EAX;
	dtable[0x95] = (t_faddrcc)XCHG_EBP_EAX;
	dtable[0x96] = (t_faddrcc)XCHG_ESI_EAX;
	dtable[0x97] = (t_faddrcc)XCHG_EDI_EAX;
	dtable[0x98] = (t_faddrcc)CBW;
	dtable[0x99] = (t_faddrcc)CWD;
	dtable[0x9a] = (t_faddrcc)CALL_PTR16_32;
	dtable[0x9b] = (t_faddrcc)WAIT;
	dtable[0x9c] = (t_faddrcc)PUSHF;
	dtable[0x9d] = (t_faddrcc)POPF;
	dtable[0x9e] = (t_faddrcc)SAHF;
	dtable[0x9f] = (t_faddrcc)LAHF;
	dtable[0xa0] = (t_faddrcc)MOV_AL_MOFFS8;
	dtable[0xa1] = (t_faddrcc)MOV_EAX_MOFFS32;
	dtable[0xa2] = (t_faddrcc)MOV_MOFFS8_AL;
	dtable[0xa3] = (t_faddrcc)MOV_MOFFS32_EAX;
	dtable[0xa4] = (t_faddrcc)MOVSB;
	dtable[0xa5] = (t_faddrcc)MOVSW;
	dtable[0xa6] = (t_faddrcc)CMPSB;
	dtable[0xa7] = (t_faddrcc)CMPSW;
	dtable[0xa8] = (t_faddrcc)TEST_AL_I8;
	dtable[0xa9] = (t_faddrcc)TEST_EAX_I32;
	dtable[0xaa] = (t_faddrcc)STOSB;
	dtable[0xab] = (t_faddrcc)STOSW;
	dtable[0xac] = (t_faddrcc)LODSB;
	dtable[0xad] = (t_faddrcc)LODSW;
	dtable[0xae] = (t_faddrcc)SCASB;
	dtable[0xaf] = (t_faddrcc)SCASW;
	dtable[0xb0] = (t_faddrcc)MOV_AL_I8;
	dtable[0xb1] = (t_faddrcc)MOV_CL_I8;
	dtable[0xb2] = (t_faddrcc)MOV_DL_I8;
	dtable[0xb3] = (t_faddrcc)MOV_BL_I8;
	dtable[0xb4] = (t_faddrcc)MOV_AH_I8;
	dtable[0xb5] = (t_faddrcc)MOV_CH_I8;
	dtable[0xb6] = (t_faddrcc)MOV_DH_I8;
	dtable[0xb7] = (t_faddrcc)MOV_BH_I8;
	dtable[0xb8] = (t_faddrcc)MOV_EAX_I32;
	dtable[0xb9] = (t_faddrcc)MOV_ECX_I32;
	dtable[0xba] = (t_faddrcc)MOV_EDX_I32;
	dtable[0xbb] = (t_faddrcc)MOV_EBX_I32;
	dtable[0xbc] = (t_faddrcc)MOV_ESP_I32;
	dtable[0xbd] = (t_faddrcc)MOV_EBP_I32;
	dtable[0xbe] = (t_faddrcc)MOV_ESI_I32;
	dtable[0xbf] = (t_faddrcc)MOV_EDI_I32;
	dtable[0xc0] = (t_faddrcc)INS_C0;//
	dtable[0xc1] = (t_faddrcc)INS_C1;//
	dtable[0xc2] = (t_faddrcc)RET_I16;
	dtable[0xc3] = (t_faddrcc)RET;
	dtable[0xc4] = (t_faddrcc)LES_R32_M16_32;
	dtable[0xc5] = (t_faddrcc)LDS_R32_M16_32;
	dtable[0xc6] = (t_faddrcc)INS_C6;
	dtable[0xc7] = (t_faddrcc)INS_C7;
	dtable[0xc8] = (t_faddrcc)ENTER;//
	dtable[0xc9] = (t_faddrcc)LEAVE;//
	dtable[0xca] = (t_faddrcc)RETF_I16;
	dtable[0xcb] = (t_faddrcc)RETF;
	dtable[0xcc] = (t_faddrcc)INT3;
	dtable[0xcd] = (t_faddrcc)INT_I8;
	dtable[0xce] = (t_faddrcc)INTO;
	dtable[0xcf] = (t_faddrcc)IRET;
	dtable[0xd0] = (t_faddrcc)INS_D0;
	dtable[0xd1] = (t_faddrcc)INS_D1;
	dtable[0xd2] = (t_faddrcc)INS_D2;
	dtable[0xd3] = (t_faddrcc)INS_D3;
	dtable[0xd4] = (t_faddrcc)AAM;
	dtable[0xd5] = (t_faddrcc)AAD;
	dtable[0xd6] = (t_faddrcc)UndefinedOpcode;
	dtable[0xd7] = (t_faddrcc)XLAT;
	dtable[0xd8] = (t_faddrcc)UndefinedOpcode;
	dtable[0xd9] = (t_faddrcc)UndefinedOpcode;
	//dtable[0xd9] = (t_faddrcc)INS_D9;
	dtable[0xda] = (t_faddrcc)UndefinedOpcode;
	dtable[0xdb] = (t_faddrcc)UndefinedOpcode;
	//dtable[0xdb] = (t_faddrcc)INS_DB;
	dtable[0xdc] = (t_faddrcc)UndefinedOpcode;
	dtable[0xdd] = (t_faddrcc)UndefinedOpcode;
	dtable[0xde] = (t_faddrcc)UndefinedOpcode;
	dtable[0xdf] = (t_faddrcc)UndefinedOpcode;
	dtable[0xe0] = (t_faddrcc)LOOPNZ;
	dtable[0xe1] = (t_faddrcc)LOOPZ;
	dtable[0xe2] = (t_faddrcc)LOOP;
	dtable[0xe3] = (t_faddrcc)JCXZ_REL8;
	dtable[0xe4] = (t_faddrcc)IN_AL_I8;
	dtable[0xe5] = (t_faddrcc)IN_EAX_I8;
	dtable[0xe6] = (t_faddrcc)OUT_I8_AL;
	dtable[0xe7] = (t_faddrcc)OUT_I8_EAX;
	dtable[0xe8] = (t_faddrcc)CALL_REL32;
	dtable[0xe9] = (t_faddrcc)JMP_REL32;
	dtable[0xea] = (t_faddrcc)JMP_PTR16_32;
	dtable[0xeb] = (t_faddrcc)JMP_REL8;
	dtable[0xec] = (t_faddrcc)IN_AL_DX;
	dtable[0xed] = (t_faddrcc)IN_EAX_DX;
	dtable[0xee] = (t_faddrcc)OUT_DX_AL;
	dtable[0xef] = (t_faddrcc)OUT_DX_EAX;
	dtable[0xf0] = (t_faddrcc)PREFIX_LOCK;
	dtable[0xf1] = (t_faddrcc)QDX;
	dtable[0xf2] = (t_faddrcc)PREFIX_REPNZ;
	dtable[0xf3] = (t_faddrcc)PREFIX_REPZ;
	dtable[0xf4] = (t_faddrcc)HLT;
	dtable[0xf5] = (t_faddrcc)CMC;
	dtable[0xf6] = (t_faddrcc)INS_F6;
	dtable[0xf7] = (t_faddrcc)INS_F7;
	dtable[0xf8] = (t_faddrcc)CLC;
	dtable[0xf9] = (t_faddrcc)STC;
	dtable[0xfa] = (t_faddrcc)CLI;
	dtable[0xfb] = (t_faddrcc)STI;
	dtable[0xfc] = (t_faddrcc)CLD;
	dtable[0xfd] = (t_faddrcc)STD;
	dtable[0xfe] = (t_faddrcc)INS_FE;
	dtable[0xff] = (t_faddrcc)INS_FF;
	dtable_0f[0x00] = (t_faddrcc)INS_0F_00;
	dtable_0f[0x01] = (t_faddrcc)INS_0F_01;
	dtable_0f[0x02] = (t_faddrcc)LAR_R32_RM32;
	dtable_0f[0x03] = (t_faddrcc)LSL_R32_RM32;
	dtable_0f[0x04] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x05] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x06] = (t_faddrcc)CLTS;
	dtable_0f[0x07] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x08] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x09] = (t_faddrcc)WBINVD;
	dtable_0f[0x0a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x0b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x0c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x0d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x0e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x0f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x10] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x11] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x12] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x13] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x14] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x15] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x16] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x17] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x18] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x19] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x1f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x20] = (t_faddrcc)MOV_R32_CR;
	dtable_0f[0x21] = (t_faddrcc)MOV_R32_DR;
	dtable_0f[0x22] = (t_faddrcc)MOV_CR_R32;
	dtable_0f[0x23] = (t_faddrcc)MOV_DR_R32;
	dtable_0f[0x24] = (t_faddrcc)MOV_R32_TR;
	dtable_0f[0x25] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x26] = (t_faddrcc)MOV_TR_R32;
	dtable_0f[0x27] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x28] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x29] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x2f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x30] = (t_faddrcc)WRMSR;
	dtable_0f[0x31] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x32] = (t_faddrcc)RDMSR;
	dtable_0f[0x33] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x34] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x35] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x36] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x37] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x38] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x39] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x3f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x40] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x41] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x42] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x43] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x44] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x45] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x46] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x47] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x48] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x49] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x4f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x50] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x51] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x52] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x53] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x54] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x55] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x56] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x57] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x58] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x59] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x5f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x60] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x61] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x62] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x63] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x64] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x65] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x66] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x67] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x68] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x69] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x6f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x70] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x71] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x72] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x73] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x74] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x75] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x76] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x77] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x78] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x79] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7a] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7b] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7c] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7d] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7e] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x7f] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0x80] = (t_faddrcc)JO_REL32;
	dtable_0f[0x81] = (t_faddrcc)JNO_REL32;
	dtable_0f[0x82] = (t_faddrcc)JC_REL32;
	dtable_0f[0x83] = (t_faddrcc)JNC_REL32;
	dtable_0f[0x84] = (t_faddrcc)JZ_REL32;
	dtable_0f[0x85] = (t_faddrcc)JNZ_REL32;
	dtable_0f[0x86] = (t_faddrcc)JNA_REL32;
	dtable_0f[0x87] = (t_faddrcc)JA_REL32;
	dtable_0f[0x88] = (t_faddrcc)JS_REL32;
	dtable_0f[0x89] = (t_faddrcc)JNS_REL32;
	dtable_0f[0x8a] = (t_faddrcc)JP_REL32;
	dtable_0f[0x8b] = (t_faddrcc)JNP_REL32;
	dtable_0f[0x8c] = (t_faddrcc)JL_REL32;
	dtable_0f[0x8d] = (t_faddrcc)JNL_REL32;
	dtable_0f[0x8e] = (t_faddrcc)JNG_REL32;
	dtable_0f[0x8f] = (t_faddrcc)JG_REL32;
	dtable_0f[0x90] = (t_faddrcc)SETO_RM8;
	dtable_0f[0x91] = (t_faddrcc)SETNO_RM8;
	dtable_0f[0x92] = (t_faddrcc)SETC_RM8;
	dtable_0f[0x93] = (t_faddrcc)SETNC_RM8;
	dtable_0f[0x94] = (t_faddrcc)SETZ_RM8;
	dtable_0f[0x95] = (t_faddrcc)SETNZ_RM8;
	dtable_0f[0x96] = (t_faddrcc)SETNA_RM8;
	dtable_0f[0x97] = (t_faddrcc)SETA_RM8;
	dtable_0f[0x98] = (t_faddrcc)SETS_RM8;
	dtable_0f[0x99] = (t_faddrcc)SETNS_RM8;
	dtable_0f[0x9a] = (t_faddrcc)SETP_RM8;
	dtable_0f[0x9b] = (t_faddrcc)SETNP_RM8;
	dtable_0f[0x9c] = (t_faddrcc)SETL_RM8;
	dtable_0f[0x9d] = (t_faddrcc)SETNL_RM8;
	dtable_0f[0x9e] = (t_faddrcc)SETNG_RM8;
	dtable_0f[0x9f] = (t_faddrcc)SETG_RM8;
	dtable_0f[0xa0] = (t_faddrcc)PUSH_FS;
	dtable_0f[0xa1] = (t_faddrcc)POP_FS;
	dtable_0f[0xa2] = (t_faddrcc)CPUID;
	dtable_0f[0xa3] = (t_faddrcc)BT_RM32_R32;
	dtable_0f[0xa4] = (t_faddrcc)SHLD_RM32_R32_I8;
	dtable_0f[0xa5] = (t_faddrcc)SHLD_RM32_R32_CL;
	dtable_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xa8] = (t_faddrcc)PUSH_GS;
	dtable_0f[0xa9] = (t_faddrcc)POP_GS;
	dtable_0f[0xaa] = (t_faddrcc)RSM;
	dtable_0f[0xab] = (t_faddrcc)BTS_RM32_R32;
	dtable_0f[0xac] = (t_faddrcc)SHRD_RM32_R32_I8;
	dtable_0f[0xad] = (t_faddrcc)SHRD_RM32_R32_CL;
	dtable_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xaf] = (t_faddrcc)IMUL_R32_RM32;
	dtable_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xb2] = (t_faddrcc)LSS_R32_M16_32;
	dtable_0f[0xb3] = (t_faddrcc)BTR_RM32_R32;
	dtable_0f[0xb4] = (t_faddrcc)LFS_R32_M16_32;
	dtable_0f[0xb5] = (t_faddrcc)LGS_R32_M16_32;
	dtable_0f[0xb6] = (t_faddrcc)MOVZX_R32_RM8;
	dtable_0f[0xb7] = (t_faddrcc)MOVZX_R32_RM16;
	dtable_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xba] = (t_faddrcc)INS_0F_BA;
	dtable_0f[0xbb] = (t_faddrcc)BTC_RM32_R32;
	dtable_0f[0xbc] = (t_faddrcc)BSF_R32_RM32;
	dtable_0f[0xbd] = (t_faddrcc)BSR_R32_RM32;
	dtable_0f[0xbe] = (t_faddrcc)MOVSX_R32_RM8;
	dtable_0f[0xbf] = (t_faddrcc)MOVSX_R32_RM16;
	dtable_0f[0xc0] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc1] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc2] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc3] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc4] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc5] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc6] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc7] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc8] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xc9] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xca] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xcb] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xcc] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xcd] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xce] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xcf] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd0] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd1] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd2] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd3] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd4] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd5] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd6] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd7] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd8] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xd9] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xda] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xdb] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xdc] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xdd] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xde] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xdf] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe0] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe1] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe2] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe3] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe4] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe5] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe6] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe7] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe8] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xe9] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xea] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xeb] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xec] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xed] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xee] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xef] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf0] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf1] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf2] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf3] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf4] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf5] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf6] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf7] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf8] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xf9] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xfa] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xfb] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xfc] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xfd] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xfe] = (t_faddrcc)UndefinedOpcode;
	dtable_0f[0xff] = (t_faddrcc)UndefinedOpcode;
}
