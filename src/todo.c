/* This file is a part of NXVM project. */

#include "stdio.h"
#include "string.h"
#include "memory.h"

#include "../vmachine/vglobal.h"

static char output[0x100];
static t_faddrcc table[0x100];

static void CaseError(const char *str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: %s.\n",str);
	vapiCallBackMachineStop();
}

static void GetMem()
{
	/* returns vcpuins.rm */
	vcpuins.rm = vramGetAddr(vcpu.overds,vramVarWord(vcpu.cs,vcpu.ip));
	vcpu.ip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns vcpuins.imm
	vcpuins.imm = vramGetAddr(vcpu.cs,vcpu.ip);
	switch(immbit) {
	case 8:		vcpu.ip += 1;break;
	case 16:	vcpu.ip += 2;break;
	case 32:	vcpu.ip += 4;break;
	default:CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns vcpuins.rm and vcpuins.r
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramVarByte(vcpu.cs,vcpu.ip++);
	vcpuins.rm = vcpuins.r = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.si);break;
		case 5:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.di);break;
		case 6: disp16 = vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;
			    vcpuins.rm = vramGetAddr(vcpu.overds,disp16); break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;}
		switch(RM) {
		case 0:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.si+disp8);break;
		case 1:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.di+disp8);break;
		case 2:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.si+disp8);break;
		case 3:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.di+disp8);break;
		case 4:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.si+disp8);break;
		case 5:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.di+disp8);break;
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+disp8);break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+disp8);break;
		default:CaseError("GetModRegRM::MOD1::RM");break;}
		bugfix(23) ;
		else {
			bugfix(3) vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);
			else vcpuins.rm += vramVarByte(vcpu.cs,vcpu.ip);
			vcpu.ip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;}
		switch(RM) {
		case 0:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.si+disp16);break;
		case 1:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.di+disp16);break;
		case 2:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.si+disp16);break;
		case 3:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.di+disp16);break;
		case 4:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.si+disp16);break;
		case 5:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.di+disp16);break;
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+disp16);break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+disp16);break;
		default:CaseError("GetModRegRM::MOD2::RM");break;}
		bugfix(23) ;
		else {vcpuins.rm += vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;}
		break;
	case 3:
		switch(RM) {
		case 0:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.al); else vcpuins.rm = (t_vaddrcc)(&vcpu.ax); break;
		case 1:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.cl); else vcpuins.rm = (t_vaddrcc)(&vcpu.cx); break;
		case 2:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.dl); else vcpuins.rm = (t_vaddrcc)(&vcpu.dx); break;
		case 3:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.bl); else vcpuins.rm = (t_vaddrcc)(&vcpu.bx); break;
		case 4:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.ah); else vcpuins.rm = (t_vaddrcc)(&vcpu.sp); break;
		case 5:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.ch); else vcpuins.rm = (t_vaddrcc)(&vcpu.bp); break;
		case 6:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.dh); else vcpuins.rm = (t_vaddrcc)(&vcpu.si); break;
		case 7:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.bh); else vcpuins.rm = (t_vaddrcc)(&vcpu.di); break;
		default:CaseError("GetModRegRM::MOD3::RM");break;}
		break;
	default:CaseError("GetModRegRM::MOD");break;}
	switch(regbit) {
	case 0:		vcpuins.r = REG;					break;
	case 4:
		switch(REG) {
		case 0:	vcpuins.r = (t_vaddrcc)(&vcpu.es);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cs);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.ss);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.ds);	break;
		default:CaseError("GetModRegRM::regbit4::REG");break;}
		break;
	case 8:
		switch(REG) {
		case 0:	vcpuins.r = (t_vaddrcc)(&vcpu.al);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cl);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dl);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bl);	break;
		case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.ah);	break;
		case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.ch);	break;
		case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.dh);	break;
		case 7:	vcpuins.r = (t_vaddrcc)(&vcpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::REG");break;}
		break;
	case 16:
		switch(REG) {
		case 0: vcpuins.r = (t_vaddrcc)(&vcpu.ax);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cx);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dx);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bx);	break;
		case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.sp);	break;
		case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.bp);	break;
		case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.si);	break;
		case 7: vcpuins.r = (t_vaddrcc)(&vcpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramVarByte(vcpu.cs,vcpu.ip++);
	vcpuins.rm = vcpuins.r = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rm = vcpu.si;break;
		case 5:	vcpuins.rm = vcpu.di;break;
		case 6:	vcpuins.rm = vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;break;
		case 7:	vcpuins.rm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;}
		switch(RM) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si+disp8;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di+disp8;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si+disp8;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di+disp8;break;
		case 4:	vcpuins.rm = vcpu.si+disp8;break;
		case 5:	vcpuins.rm = vcpu.di+disp8;break;
		case 6:	vcpuins.rm = vcpu.bp+disp8;break;
		case 7:	vcpuins.rm = vcpu.bx+disp8;break;
		default:CaseError("GetModRegRMEA::MOD1::RM");break;}
		bugfix(23) {vcpuins.rm %= 0x10000;}
		else {
			bugfix(3) vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);
			else vcpuins.rm += vramVarByte(vcpu.cs,vcpu.ip);
			vcpu.ip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;}
		switch(RM) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si+disp16;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di+disp16;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si+disp16;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di+disp16;break;
		case 4:	vcpuins.rm = vcpu.si+disp16;break;
		case 5:	vcpuins.rm = vcpu.di+disp16;break;
		case 6:
			bugfix(14) vcpuins.rm = vcpu.bp+disp16;
			else vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp);
			break;
		case 7:	vcpuins.rm = vcpu.bx+disp16;break;
		default:CaseError("GetModRegRMEA::MOD2::RM");break;}
		bugfix(23) {vcpuins.rm %= 0x10000;}
		else {vcpuins.rm += vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;}
		break;
	default:CaseError("GetModRegRMEA::MOD");break;}
	switch(REG) {
	case 0: vcpuins.r = (t_vaddrcc)(&vcpu.ax);	break;
	case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cx);	break;
	case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dx);	break;
	case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bx);	break;
	case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.sp);	break;
	case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.bp);	break;
	case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.si);	break;
	case 7: vcpuins.r = (t_vaddrcc)(&vcpu.di);	break;
	default:CaseError("GetModRegRMEA::REG");break;}
}

static void INT(t_nubit8 intid);
static void ADD(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		bugfix(6) break;
		else ;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		bugfix(22) vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		else vcpuins.opr2 = d_nsbit8(src); /* in this case opr2 could be 0xffffffff */
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("ADD::len");break;}
	SetFlags(ADD_FLAG);
#define op add
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void OR(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = OR8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("OR::len");break;}
	ClrBit(vcpu.flags, VCPU_FLAG_OF);
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
	ClrBit(vcpu.flags, VCPU_FLAG_AF);
	SetFlags(OR_FLAG);
#define op or
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void ADC(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("ADC::len");break;}
	SetFlags(ADC_FLAG);
#define op adc
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void SBB(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SBB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("SBB::len");break;}
	SetFlags(SBB_FLAG);
#define op sbb
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void AND(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = AND8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("AND::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(AND_FLAG);
#define op and
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void SUB(void *dest, void *src, t_nubit8 len)
{
//	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("SUB::len");break;}
	SetFlags(SUB_FLAG);
//#define op sub
//	aexecall
//#undef op
//	asregall
//	acheckall(AFLAGS1)
}
static void XOR(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = XOR8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("XOR::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(XOR_FLAG);
#define op xor
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void CMP(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nsbit8(src) & 0xff;
		bugfix(7) vcpuins.result = ((t_nubit8)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xff;
		else vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xffff;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit16(src) & 0xffff;
		bugfix(7) vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit16)vcpuins.opr2)&0xffff;
		else vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		break;
	default:CaseError("CMP::len");break;}
	SetFlags(CMP_FLAG);
#define op cmp
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		vcpu.sp -= 2;
		bugfix(13) vramVarWord(vcpu.ss,vcpu.sp) = data;
		else vramVarWord(vcpu.ss,vcpu.sp) = d_nubit16(src);
		break;
	default:CaseError("PUSH::len");break;}
}
static void POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = vramVarWord(vcpu.ss,vcpu.sp);
		vcpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
static void INC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op inc
		aexec_idc8;
#undef op
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op inc
		aexec_idc16;
#undef op
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("INC::len");break;}
	SetFlags(INC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static void DEC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op dec
		aexec_idc8;
#undef op
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op dec
		aexec_idc16;
#undef op
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("DEC::len");break;}
	SetFlags(DEC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static void JCC(void *src, t_bool flagj,t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(flagj)
			bugfix(5) vcpu.ip += d_nsbit8(src);
			else vcpu.ip += d_nubit8(src);
		break;
	default:CaseError("JCC::len");break;}
}
static void TEST(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = TEST8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xff;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = TEST16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xffff;
		break;
	default:CaseError("TEST::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(TEST_FLAG);
#define op test
	aexecall
#undef op
	acheckall(AFLAGS1)
}
static void XCHG(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.opr2;
		d_nubit8(src) = (t_nubit8)vcpuins.opr1;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.opr2;
		d_nubit16(src) = (t_nubit16)vcpuins.opr1;
		break;
	default:CaseError("XCHG::len");break;}
}
static void MOV(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		d_nubit8(dest) = d_nubit8(src);
		break;
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = d_nubit16(src);
		break;
	default:CaseError("MOV::len");break;}
}
static void ROL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+(t_nubit8)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
#define op rol
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
#define op rol
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void ROR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(tempCF) d_nubit8(dest) |= 0x80;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^(!!(d_nubit8(dest)&0x40)));
#define op ror
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(tempCF) d_nubit16(dest) |= 0x8000;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^(!!(d_nubit16(dest)&0x4000)));
#define op ror
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void RCL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+GetBit(vcpu.flags, VCPU_FLAG_CF);
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
#define op rcl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+GetBit(vcpu.flags, VCPU_FLAG_CF);
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
#define op rcl
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void RCR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(GetBit(vcpu.flags, VCPU_FLAG_CF)) d_nubit8(dest) |= 0x80;
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
#define op rcr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(GetBit(vcpu.flags, VCPU_FLAG_CF)) d_nubit16(dest) |= 0x8000;
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
#define op rcr
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void SHL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHL::len");break;}
	if (count) ClrAF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void SHR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,!!(tempdest8&0x80));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,!!(tempdest16&0x8000));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHR::len");break;}
	if (count) ClrAF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void SAL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAL::len");break;}
	if (count) ClrAF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void SAR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= tempdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,0);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= tempdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,0);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAR::len");break;}
	if (count) ClrAF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
	bugfix(10) {
		/* add parameters flagsi, flagdi */
	} else ;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(GetBit(vcpu.flags, VCPU_FLAG_DF)) {
			if (flagdi) vcpu.di--;
			if (flagsi) vcpu.si--;
		} else {
			if (flagdi) vcpu.di++;
			if (flagsi) vcpu.si++;
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(GetBit(vcpu.flags, VCPU_FLAG_DF)) {
			if (flagdi) vcpu.di -= 2;
			if (flagsi) vcpu.si -= 2;
		} else {
			if (flagdi) vcpu.di += 2;
			if (flagsi) vcpu.si += 2;
		}
		break;
	default:CaseError("STRDIR::len");break;}
}
static void MOVS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vramVarByte(vcpu.es,vcpu.di) = vramVarByte(vcpu.overds,vcpu.si);
		STRDIR(8,1,1);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOVSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vramVarWord(vcpu.es,vcpu.di) = vramVarWord(vcpu.overds,vcpu.si);
		STRDIR(16,1,1);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOVSW\n");
		break;
	default:CaseError("MOVS::len");break;}
	//qdcgaCheckVideoRam(vramGetAddr(vcpu.es, vcpu.di));
}
static void CMPS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vramVarByte(vcpu.overds,vcpu.si);
		vcpuins.opr2 = vramVarByte(vcpu.es,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMPSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vramVarWord(vcpu.overds,vcpu.si);
		vcpuins.opr2 = vramVarWord(vcpu.es,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMPSW\n");
		break;
	default:CaseError("CMPS::len");break;}
}
static void STOS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vramVarByte(vcpu.es,vcpu.di) = vcpu.al;
		STRDIR(8,0,1);
		/*if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
		WriteVideoRam(eCPU.di+t-0xa0000);*/
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  STOSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vramVarWord(vcpu.es,vcpu.di) = vcpu.ax;
		STRDIR(16,0,1);
		/*if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000)
		{
			for (i=0;i<tmpOpdSize;i++)
			{
				WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
			}
		}*/
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  STOSW\n");
		break;
	default:CaseError("STOS::len");break;}
}
static void LODS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpu.al = vramVarByte(vcpu.overds,vcpu.si);
		STRDIR(8,1,0);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LODSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpu.ax = vramVarWord(vcpu.overds,vcpu.si);
		STRDIR(16,1,0);
		// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LODSW\n");
		break;
	default:CaseError("LODS::len");break;}
}
static void SCAS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vcpu.al;
		vcpuins.opr2 = vramVarByte(vcpu.es,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = vramVarWord(vcpu.es,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::len");break;}
}
static void NOT(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
		vcpuins.bit = 8;
		d_nubit8(dest) = ~d_nubit8(dest);
#define op not
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		vcpuins.bit = 16;
		d_nubit16(dest) = ~d_nubit16(dest);
#define op not
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NOT::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static void NEG(void *dest, t_nubit8 len)
{
	t_nubitcc zero = 0;
	switch(len) {
	case 8:	
		async ub1 = d_nubit8(dest);
		vcpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		d_nubit8(dest) = (t_nubit8)zero;
#define op neg
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		vcpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		d_nubit16(dest) = (t_nubit16)zero;
#define op neg
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NEG::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static void MUL(void *src, t_nubit8 len)
{
	t_nubit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		vcpu.ax = vcpu.al * d_nubit8(src);
		MakeBit(vcpu.flags,VCPU_FLAG_OF,!!vcpu.ah);
		MakeBit(vcpu.flags,VCPU_FLAG_CF,!!vcpu.ah);
#define op mul
		aexec_mdx8;
#undef op
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		tempresult = vcpu.ax * d_nubit16(src);
		vcpu.dx = (tempresult>>16)&0xffff;
		vcpu.ax = tempresult&0xffff;
		MakeBit(vcpu.flags,VCPU_FLAG_OF,!!vcpu.dx);
		MakeBit(vcpu.flags,VCPU_FLAG_CF,!!vcpu.dx);
#define op mul
		aexec_mdx16;
#undef op
		break;
	default:CaseError("MUL::len");break;}
	acheck(VCPU_FLAG_OF | VCPU_FLAG_CF) vapiPrintIns(vcpu.cs,vcpu.ip,"+MUL");
}
static void IMUL(void *src, t_nubit8 len)
{
	t_nsbit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		vcpu.ax = (t_nsbit8)vcpu.al * d_nsbit8(src);
		if(vcpu.ax == vcpu.al) {
			ClrBit(vcpu.flags,VCPU_FLAG_OF);
			ClrBit(vcpu.flags,VCPU_FLAG_CF);
		} else {
			SetBit(vcpu.flags,VCPU_FLAG_OF);
			SetBit(vcpu.flags,VCPU_FLAG_CF);
		}
#define op imul
		aexec_mdx8;
#undef op
		acheck(VCPU_FLAG_OF | VCPU_FLAG_CF) vapiPrintIns(vcpu.cs,vcpu.ip,"+IMUL");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		tempresult = (t_nsbit16)vcpu.ax * d_nsbit16(src);
		vcpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		vcpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)vcpu.ax) {
			ClrBit(vcpu.flags,VCPU_FLAG_OF);
			ClrBit(vcpu.flags,VCPU_FLAG_CF);
		} else {
			SetBit(vcpu.flags,VCPU_FLAG_OF);
			SetBit(vcpu.flags,VCPU_FLAG_CF);
		}
#define op imul
		aexec_mdx16;
#undef op
		acheck(VCPU_FLAG_OF | VCPU_FLAG_CF) vapiPrintIns(vcpu.cs,vcpu.ip,"+IMUL");
		break;
	default:CaseError("IMUL::len");break;}
}
static void DIV(void *src, t_nubit8 len)
{
	t_nubit16 tempAX = vcpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
#define op div
		aexec_mdx8;
#undef op
		acheck(VCPU_FLAG_DF | VCPU_FLAG_TF) vapiPrintIns(vcpu.cs,vcpu.ip,"+DIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
#define op div
		aexec_mdx16;
#undef op
		acheck(VCPU_FLAG_DF | VCPU_FLAG_TF) vapiPrintIns(vcpu.cs,vcpu.ip,"+DIV");
		break;
	default:CaseError("DIV::len");break;}
}
static void IDIV(void *src, t_nubit8 len)
{
	t_nsbit16 tempAX = vcpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
#define op idiv
		aexec_mdx8;
#undef op
		acheck(VCPU_FLAG_DF | VCPU_FLAG_TF) vapiPrintIns(vcpu.cs,vcpu.ip,"+IDIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
#define op idiv
		aexec_mdx16;
#undef op
		acheck(VCPU_FLAG_DF | VCPU_FLAG_TF) vapiPrintIns(vcpu.cs,vcpu.ip,"+IDIV");
		break;
	default:CaseError("IDIV::len");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&vcpu.flags,16);
	ClrBit(vcpu.flags, (VCPU_FLAG_IF | VCPU_FLAG_TF));
	PUSH((void *)&vcpu.cs,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = vramVarWord(0x0000,intid*4+0);
	vcpu.cs = vramVarWord(0x0000,intid*4+2);
}

static void OpError()
{
	vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
	vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
		vcpu.cs, vcpu.ip, vramVarByte(vcpu.cs,vcpu.ip+0),
		vramVarByte(vcpu.cs,vcpu.ip+1), vramVarByte(vcpu.cs,vcpu.ip+2),
		vramVarByte(vcpu.cs,vcpu.ip+3), vramVarByte(vcpu.cs,vcpu.ip+4));
	vapiCallBackMachineStop();
}
static void ADD_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_RM8_R8\n");
}
static void ADD_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_RM16_R16\n");
}
static void ADD_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_R8_RM8\n");
}
static void ADD_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_R16_RM16\n");
}
static void ADD_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADD((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_AL_I8\n");
}
static void ADD_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADD((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_AX_I16\n");
}
static void PUSH_ES()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.es,16);
}
static void POP_ES()
{
	vcpu.ip++;
	POP((void *)&vcpu.es,16);
}
static void OR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_RM8_R8\n");
}
static void OR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_RM16_R16\n");
}
static void OR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_R8_RM8\n");
}
static void OR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_R16_RM16\n");
}
static void OR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_AL_I8\n");
}
static void OR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_AX_I16\n");
}
static void PUSH_CS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cs,16);
}
static void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs,16);
}
static void INS_0F()
{
	OpError();
}
static void ADC_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_RM8_R8\n");
}
static void ADC_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_RM16_R16\n");
}
static void ADC_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_R8_RM8\n");
}
static void ADC_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_R16_RM16\n");
}
static void ADC_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADC((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_AL_I8\n");
}
static void ADC_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADC((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_AX_I16\n");
}
static void PUSH_SS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ss,16);
}
static void POP_SS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ss,16);
}
static void SBB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_RM8_R8\n");
}
static void SBB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_RM16_R16\n");
}
static void SBB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_R8_RM8\n");
}
static void SBB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_R16_RM16\n");
}
static void SBB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_AL_I8\n");
}
static void SBB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_AX_I16\n");
}
static void PUSH_DS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ds,16);
}
static void POP_DS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ds,16);
}
static void AND_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_RM8_R8\n");
}
static void AND_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_RM16_R16\n");
}
static void AND_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_R8_RM8\n");
}
static void AND_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_R16_RM16\n");
}
static void AND_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	AND((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_AL_I8\n");
}
static void AND_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	AND((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_AX_I16\n");
}
static void ES()
{
	vcpu.ip++;
	vcpu.overds = vcpu.es;
	vcpu.overss = vcpu.es;
}
static void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.ip++;
	async;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al = newAL;
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetBit(vcpu.flags, VCPU_FLAG_CF) || ((newAL < oldAL) || (newAL < 0x06)));
		bugfix(19) SetBit(vcpu.flags, VCPU_FLAG_AF);
		else ;
	} else ClrBit(vcpu.flags, VCPU_FLAG_AF);
	if(((vcpu.al & 0xf0) > 0x90) || GetBit(vcpu.flags, VCPU_FLAG_CF)) {
		vcpu.al += 0x60;
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_CF);
	bugfix(18) {
		vcpuins.bit = 8;
		vcpuins.result = (t_nubitcc)vcpu.al;
		SetFlags(DAA_FLAG);
	} else ;
#define op daa
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"DAA");
}
static void SUB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_RM8_R8\n");
}
static void SUB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_RM16_R16\n");
}
static void SUB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_R8_RM8\n");
}
static void SUB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_R16_RM16\n");
}
static void SUB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_AL_I8\n");
}
static void SUB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_AX_I16\n");
}
static void CS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.cs;
	vcpu.overss = vcpu.cs;
}
static void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.ip++;
	async;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetBit(vcpu.flags, VCPU_FLAG_CF) || (oldAL < 0x06));
		SetBit(vcpu.flags,VCPU_FLAG_AF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_AF);
	if((vcpu.al > 0x9f) || GetBit(vcpu.flags, VCPU_FLAG_CF)) {
		vcpu.al -= 0x60;
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_CF);
	bugfix(18) {
		vcpuins.bit = 8;
		vcpuins.result = (t_nubitcc)vcpu.al;
		SetFlags(DAS_FLAG);
	} else ;
#define op das
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"DAS");
}
static void XOR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_RM8_R8\n");
}
static void XOR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_RM16_R16\n");
}
static void XOR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_R8_RM8\n");
}
static void XOR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_R16_RM16\n");
}
static void XOR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_AL_I8\n");
}
static void XOR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_AX_I16\n");
}
static void SS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.ss;
	vcpu.overss = vcpu.ss;
}
static void AAA()
{
	vcpu.ip++;
	async;
	if(((vcpu.al&0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al += 0x06;
		vcpu.ah += 0x01;
		SetBit(vcpu.flags,VCPU_FLAG_AF);
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else {
		ClrBit(vcpu.flags,VCPU_FLAG_AF);
		ClrBit(vcpu.flags,VCPU_FLAG_CF);
	}
	vcpu.al &= 0x0f;
#define op aaa
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"AAA");
}
static void CMP_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_RM8_R8\n");
}
static void CMP_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_RM16_R16\n");
}
static void CMP_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_R8_RM8\n");
}
static void CMP_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_R16_RM16\n");
}
static void CMP_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_AL_I8\n");
}
static void CMP_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_AX_I16\n");
}
static void DS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.ds;
	vcpu.overss = vcpu.ds;
}
static void AAS()
{
	vcpu.ip++;
	async;
	if(((vcpu.al&0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al -= 0x06;
		vcpu.ah += 0x01;
		SetBit(vcpu.flags,VCPU_FLAG_AF);
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else {
		ClrBit(vcpu.flags,VCPU_FLAG_CF);
		ClrBit(vcpu.flags,VCPU_FLAG_AF);
	}
	vcpu.al &= 0x0f;
#define op aas
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"AAS");
}
static void INC_AX()
{
	vcpu.ip++;
	INC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_AX\n");
}
static void INC_CX()
{
	vcpu.ip++;
	INC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_CX\n");
}
static void INC_DX()
{
	vcpu.ip++;
	INC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_DX\n");
}
static void INC_BX()
{
	vcpu.ip++;
	INC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_BX\n");
}
static void INC_SP()
{
	vcpu.ip++;
	INC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_SP\n");
}
static void INC_BP()
{
	vcpu.ip++;
	INC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_BP\n");
}
static void INC_SI()
{
	vcpu.ip++;
	INC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_SI\n");
}
static void INC_DI()
{
	vcpu.ip++;
	INC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_DI\n");
}
static void DEC_AX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_AX\n");
}
static void DEC_CX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_CX\n");
}
static void DEC_DX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_DX\n");
}
static void DEC_BX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_BX\n");
}
static void DEC_SP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_SP\n");
}
static void DEC_BP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_BP\n");
}
static void DEC_SI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_SI\n");
}
static void DEC_DI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_DI\n");
}
static void PUSH_AX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ax,16);
}
static void PUSH_CX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cx,16);
}
static void PUSH_DX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.dx,16);
}
static void PUSH_BX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bx,16);
}
static void PUSH_SP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.sp,16);
}
static void PUSH_BP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bp,16);
}
static void PUSH_SI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.si,16);
}
static void PUSH_DI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.di,16);
}
static void POP_AX()
{
	vcpu.ip++;
	POP((void *)&vcpu.ax,16);
}
static void POP_CX()
{
	vcpu.ip++;
	POP((void *)&vcpu.cx,16);
}
static void POP_DX()
{
	vcpu.ip++;
	POP((void *)&vcpu.dx,16);
}
static void POP_BX()
{
	vcpu.ip++;
	POP((void *)&vcpu.bx,16);
}
static void POP_SP()
{
	vcpu.ip++;
	POP((void *)&vcpu.sp,16);
}
static void POP_BP()
{
	vcpu.ip++;
	POP((void *)&vcpu.bp,16);
}
static void POP_SI()
{
	vcpu.ip++;
	POP((void *)&vcpu.si,16);
}
static void POP_DI()
{
	vcpu.ip++;
	POP((void *)&vcpu.di,16);
}
static void JO()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, GetBit(vcpu.flags, VCPU_FLAG_OF), 8);
}
static void JNO()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !GetBit(vcpu.flags, VCPU_FLAG_OF), 8);
}
static void JC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, GetBit(vcpu.flags, VCPU_FLAG_CF), 8);
}
static void JNC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !GetBit(vcpu.flags, VCPU_FLAG_CF), 8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNC\n");
}
static void JZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, GetBit(vcpu.flags, VCPU_FLAG_ZF), 8);
}
static void JNZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !GetBit(vcpu.flags, VCPU_FLAG_ZF), 8);
}
static void JBE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (GetBit(vcpu.flags, VCPU_FLAG_CF) ||
		GetBit(vcpu.flags, VCPU_FLAG_ZF)), 8);
}
static void JA()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (!GetBit(vcpu.flags, VCPU_FLAG_CF) &&
		!GetBit(vcpu.flags, VCPU_FLAG_ZF)), 8);
}
static void JS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, GetBit(vcpu.flags, VCPU_FLAG_SF), 8);
}
static void JNS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !GetBit(vcpu.flags, VCPU_FLAG_SF), 8);
}
static void JP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, GetBit(vcpu.flags, VCPU_FLAG_PF), 8);
}
static void JNP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !GetBit(vcpu.flags, VCPU_FLAG_PF), 8);
}
static void JL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (GetBit(vcpu.flags, VCPU_FLAG_SF) !=
		GetBit(vcpu.flags, VCPU_FLAG_OF)), 8);
}
static void JNL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (GetBit(vcpu.flags, VCPU_FLAG_SF) ==
		GetBit(vcpu.flags, VCPU_FLAG_OF)), 8);
}
static void JLE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (GetBit(vcpu.flags, VCPU_FLAG_ZF) ||
		(GetBit(vcpu.flags, VCPU_FLAG_SF) !=
		GetBit(vcpu.flags, VCPU_FLAG_OF))), 8);
}
static void JG()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (!GetBit(vcpu.flags, VCPU_FLAG_ZF) &&
		(GetBit(vcpu.flags, VCPU_FLAG_SF) ==
		GetBit(vcpu.flags, VCPU_FLAG_OF))), 8);
}
static void INS_80()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ADD((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 1:	OR ((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 2:	ADC((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 3:	SBB((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 4:	AND((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 5:	SUB((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 6:	XOR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 7:	CMP((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	default:CaseError("INS_80::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_80\n");
}
static void INS_81()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch(vcpuins.r) {
	case 0:	ADD((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 1:	OR ((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 2:	ADC((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 3:	SBB((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 4:	AND((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 5:	SUB((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 6:	XOR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 7:	CMP((void *)vcpuins.rm,(void *)vcpuins.imm,16);
	break;
	default:CaseError("INS_81::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_81\n");
}
static void INS_82()
{
	INS_80();
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_82\n");
}
static void INS_83()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ADD((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 1:	OR ((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 2:	ADC((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 3:	SBB((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 4:	AND((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 5:	SUB((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 6:	XOR((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	case 7:	CMP((void *)vcpuins.rm,(void *)vcpuins.imm,12);break;
	default:CaseError("INS_83::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_83\n");
}
static void TEST_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_RM8_R8\n");
}
static void TEST_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_RM16_R16\n");
}
static void XCHG_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_R8_RM8\n");
}
static void XCHG_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_R16_RM16\n");
}
static void MOV_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM8_R8\n");
}
static void MOV_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM16_R16\n");
}
static void MOV_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_R8_RM8\n");
}
static void MOV_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_R16_RM16\n");
}
static void MOV_RM16_SEG()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM16_SEG\n");
}
static void LEA_R16_M16()
{
	vcpu.ip++;
	GetModRegRMEA();
	d_nubit16(vcpuins.r) = vcpuins.rm & 0xffff;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LEA_R16_M16\n");
}
static void MOV_SEG_RM16()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SEG_RM16\n");
}
static void POP_RM16()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:
		bugfix(14) POP((void *)vcpuins.rm,16);
		else POP((void *)vcpuins.rm,16);
		break;
	default:CaseError("POP_RM16::vcpuins.r");break;}
}
static void NOP()
{
	vcpu.ip++;
}
static void XCHG_CX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.cx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_CX_AX\n");
}
static void XCHG_DX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.dx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_DX_AX\n");
}
static void XCHG_BX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_BX_AX\n");
}
static void XCHG_SP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.sp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_SP_AX\n");
}
static void XCHG_BP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_BP_AX\n");
}
static void XCHG_SI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.si,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_SI_AX\n");
}
static void XCHG_DI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.di,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_DI_AX\n");
}
static void CBW()
{
	vcpu.ip++;
	async;
	vcpu.ax = (t_nsbit8)vcpu.al;
#define op cbw
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"CBW");
}
static void CWD()
{
	vcpu.ip++;
	async;
	if (vcpu.ax & 0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
#define op cwd
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"CWD");
}
static void CALL_PTR16_16()
{
	t_nubit16 newcs,newip;
	async;
	vcpu.ip++;
	GetImm(16);
	newip = d_nubit16(vcpuins.imm);
	GetImm(16);
	newcs = d_nubit16(vcpuins.imm);
	aipcheck;
	PUSH((void *)&vcpu.cs,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = newip;
	vcpu.cs = newcs;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CALL_PTR16_16\n");
}
static void WAIT()
{
	vcpu.ip++;
	/* not implemented */
}
static void PUSHF()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.flags,16);
}
static void POPF()
{
	vcpu.ip++;
	POP((void *)&vcpu.flags,16);
}
static void SAHF()
{
	vcpu.ip++;
	d_nubit8(&vcpu.flags) = vcpu.ah;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SAHF\n");
}
static void LAHF()
{
	vcpu.ip++;
//	vapiPrint("1:LAHF:%4X\n",vcpu.flags);
	vcpu.ah = d_nubit8(&vcpu.flags);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LAHF\n");
}
static void MOV_AL_M8()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.al,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AL_M8\n");
}
static void MOV_AX_M16()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AX_M16\n");
}
static void MOV_M8_AL()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.al,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M8_AL\n");
}
static void MOV_M16_AX()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M16_AX\n");
}
static void MOVSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) MOVS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			MOVS(8);
			vcpu.cx--;
		}
	}
}
static void MOVSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) MOVS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			MOVS(16);
			vcpu.cx--;
		}
	}
}
static void CMPSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) CMPS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			CMPS(8);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
static void CMPSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) CMPS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			CMPS(16);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
static void TEST_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	TEST((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_AL_I8\n");
}
static void TEST_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_AX_I16\n");
}
static void STOSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) STOS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			STOS(8);
			vcpu.cx--;
		}
	}
}
static void STOSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) STOS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			STOS(16);
			vcpu.cx--;
		}
	}
}
static void LODSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) LODS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			LODS(8);
			vcpu.cx--;
		}
	}
}
static void LODSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) LODS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			LODS(16);
			vcpu.cx--;
		}
	}
}
static void SCASB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) SCAS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			SCAS(8);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
static void SCASW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) SCAS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			SCAS(16);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
static void MOV_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AL_I8\n");
}
static void MOV_CL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CL_I8\n");
}
static void MOV_DL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DL_I8\n");
}
static void MOV_BL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BL_I8\n");
}
static void MOV_AH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AH_I8\n");
}
static void MOV_CH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CH_I8\n");
}
static void MOV_DH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DH_I8\n");
}
static void MOV_BH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BH_I8\n");
}
static void MOV_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AX_I16\n");
}
static void MOV_CX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CX_I16\n");
}
static void MOV_DX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DX_I16\n");
}
static void MOV_BX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BX_I16\n");
}
static void MOV_SP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SP_I16\n");
}
static void MOV_BP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BP_I16\n");
}
static void MOV_SI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SI_I16\n");
}
static void MOV_DI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DI_I16\n");
}
static void INS_C0()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	default:CaseError("INS_C0::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_C0\n");
}
static void INS_C1()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	default:CaseError("INS_C1::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_C1\n");
}
static void RET_I16()
{
	t_nubit16 addsp;
	vcpu.ip++;
	bugfix(15) {
		GetImm(16);
		addsp = d_nubit16(vcpuins.imm);
	} else {
		GetImm(8);
		addsp = d_nubit8(vcpuins.imm);
	}
	POP((void *)&vcpu.ip,16);
	vcpu.sp += addsp;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RET_I16\n");
}
static void RET()
{
	POP((void *)&vcpu.ip,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RET\n");
}
static void LES_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.es,(void *)(vcpuins.rm+2),16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LES_R16_M16\n");
}
static void LDS_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.ds,(void *)(vcpuins.rm+2),16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LDS_R16_M16\n");
}
static void MOV_M8_I8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M8_I8\n");
}
static void MOV_M16_I16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M16_I16\n");
}
static void RETF_I16()
{
	t_nubit16 addsp;
	vcpu.ip++;
	GetImm(16);
	addsp = d_nubit16(vcpuins.imm);
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	vcpu.sp += addsp;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RETF_I16\n");
}
static void RETF()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RETF\n");
}
static void INT3()
{
	async;
	vcpu.ip++;
	aipcheck;
	INT(0x03);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INT3\n");
}
static void INT_I8()
{
	async;
	vcpu.ip++;
	GetImm(8);
	aipcheck;
	INT(d_nubit8(vcpuins.imm));
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INT_I8\n");
}
static void INTO()
{
	async;
	vcpu.ip++;
	aipcheck;
	if(GetBit(vcpu.flags, VCPU_FLAG_OF)) INT(0x04);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INTO\n");
}
static void IRET()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	POP((void *)&vcpu.flags,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IRET\n");
}
static void INS_D0()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,NULL,8);break;
	case 1:	ROR((void *)vcpuins.rm,NULL,8);break;
	case 2:	RCL((void *)vcpuins.rm,NULL,8);break;
	case 3:	RCR((void *)vcpuins.rm,NULL,8);break;
	case 4:	SHL((void *)vcpuins.rm,NULL,8);break;
	case 5:	SHR((void *)vcpuins.rm,NULL,8);break;
	case 6:	SAL((void *)vcpuins.rm,NULL,8);break;
	case 7:	SAR((void *)vcpuins.rm,NULL,8);break;
	default:CaseError("INS_D0::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D0\n");
}
static void INS_D1()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,NULL,16);break;
	case 1:	ROR((void *)vcpuins.rm,NULL,16);break;
	case 2:	RCL((void *)vcpuins.rm,NULL,16);break;
	case 3:	RCR((void *)vcpuins.rm,NULL,16);break;
	case 4:	SHL((void *)vcpuins.rm,NULL,16);break;
	case 5:	SHR((void *)vcpuins.rm,NULL,16);break;
	case 6:	SAL((void *)vcpuins.rm,NULL,16);break;
	case 7:	SAR((void *)vcpuins.rm,NULL,16);break;
	default:CaseError("INS_D1::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D1\n");
}
static void INS_D2()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	default:CaseError("INS_D2::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D2\n");
}
static void INS_D3()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	default:CaseError("INS_D3::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D3\n");
}
static void AAM()
{
	t_nubit8 base,tempAL;
	vcpu.ip++;
	GetImm(8);
	async;
	base = d_nubit8(vcpuins.imm);
	if(base == 0) INT(0x00);
	else {
		tempAL = vcpu.al;
		vcpu.ah = tempAL / base;
		vcpu.al = tempAL % base;
		vcpuins.bit = 0x08;
		vcpuins.result = vcpu.al & 0xff;
		SetFlags(AAM_FLAG);
	}
#define op aam
	aexec_cax16
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-2,"AAM");
}
static void AAD()
{
	t_nubit8 base,tempAL,tempAH;
	vcpu.ip++;
	GetImm(8);
	async;
	base = d_nubit8(vcpuins.imm);
	if(base == 0) INT(0x00);
	else {
		tempAL = vcpu.al;
		tempAH = vcpu.ah;
		vcpu.al = (tempAL+(tempAH*base)) & 0xff;
		vcpu.ah = 0x00;
		vcpuins.bit = 0x08;
		vcpuins.result = vcpu.al & 0xff;
		SetFlags(AAD_FLAG);
	}
#define op aad
	aexec_cax16
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-2,"AAD");
}
static void XLAT()
{
	vcpu.ip++;
	vcpu.al = vramVarByte(vcpu.overds,vcpu.bx+vcpu.al);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XLAT\n");
}
/*
static void INS_D9()
{// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D9\n");
}
static void INS_DB()
{// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_DB\n");
}
*/
static void LOOPNZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) vcpu.ip += rel8;
}
static void LOOPZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && GetBit(vcpu.flags, VCPU_FLAG_ZF)) vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOOPZ\n");
}
static void LOOP()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOOP\n");
}
static void JCXZ_REL8()
{
	vcpu.ip++;
	GetImm(8);
	JCC((static void*)vcpuins.imm,!vcpu.cx,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JCXZ_REL8\n");
}
static void IN_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.imm));
	ExecFun(vport.in[d_nubit8(vcpuins.imm)]);
	vcpu.al = vport.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AL_I8\n");
}
static void IN_AX_I8()
{
	vcpu.ip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.imm));
	ExecFun(vport.in[d_nubit8(vcpuins.imm)]);
	vcpu.ax = vport.ioword;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AX_I8\n");
}
static void OUT_I8_AL()
{
	vcpu.ip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.imm));
	vport.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_I8_AL\n");
}
static void OUT_I8_AX()
{
	vcpu.ip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.imm));
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_I8_AX\n");
}
static void CALL_REL16()
{
	t_nsbit16 rel16;
	async;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.imm);
	aipcheck;
	PUSH((void *)&vcpu.ip,16);
	bugfix(12) vcpu.ip += rel16;
	else vcpu.ip += d_nubit16(vcpuins.imm);
}
static void JMP_REL16()
{
	t_nsbit16 rel16;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.imm);
	bugfix(2) vcpu.ip += rel16;
	else vcpu.ip += d_nubit16(vcpuins.imm);
}
static void JMP_PTR16_16()
{
	t_nubit16 newip,newcs;
	vcpu.ip++;
	GetImm(16);
	newip = d_nubit16(vcpuins.imm);
	GetImm(16);
	newcs = d_nubit16(vcpuins.imm);
	vcpu.ip = newip;
	vcpu.cs = newcs;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JMP_PTR16_16\n");
}
static void JMP_REL8()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(vcpuins.imm);
	bugfix(9) vcpu.ip += rel8;
	else vcpu.ip += d_nubit8(vcpuins.imm);
}
static void IN_AL_DX()
{
	vcpu.ip++;
//	vapiPrint("IN: %04X\n",vcpu.dx);
	ExecFun(vport.in[vcpu.dx]);
	vcpu.al = vport.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AL_DX\n");
}
static void IN_AX_DX()
{
	vcpu.ip++;
//	vapiPrint("IN: %04X\n",vcpu.dx);
	ExecFun(vport.in[vcpu.dx]);
	vcpu.ax = vport.ioword;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AX_DX\n");
}
static void OUT_DX_AL()
{
	vcpu.ip++;
	vport.iobyte = vcpu.al;
//	vapiPrint("OUT: %04X\n",vcpu.dx);
	ExecFun(vport.out[vcpu.dx]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_DX_AL\n");
}
static void OUT_DX_AX()
{
	vcpu.ip++;
//	vapiPrint("OUT: %04X\n",vcpu.dx);
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[vcpu.dx]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_DX_AX\n");
}
static void LOCK()
{
	vcpu.ip++;
	/* Not Implemented */
}
static void REPNZ()
{
	// CMPS,SCAS
	vcpu.ip++;
	vcpuins.rep = RT_REPZNZ;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  REPNZ\n");
}
static void REP()
{	// MOVS,LODS,STOS,CMPS,SCAS
	vcpu.ip++;
	vcpuins.rep = RT_REPZ;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  REP\n");
}
static void HLT()
{
	vcpu.ip++;
	/* Not Implemented */
}
static void CMC()
{
	vcpu.ip++;
	async;
	vcpu.flags ^= VCPU_FLAG_CF;
#define op cmc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"CMC");
}
static void INS_F6()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	GetImm(8);
			TEST((void *)vcpuins.rm,(void *)vcpuins.imm,8);
			break;
	case 2:	NOT ((void *)vcpuins.rm,8);	break;
	case 3:	NEG ((void *)vcpuins.rm,8);	break;
	case 4:	MUL ((void *)vcpuins.rm,8);	break;
	case 5:	IMUL((void *)vcpuins.rm,8);	break;
	case 6:	DIV ((void *)vcpuins.rm,8);	break;
	case 7:	IDIV((void *)vcpuins.rm,8);	break;
	default:CaseError("INS_F6::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_F6\n");
}
static void INS_F7()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	GetImm(16);
			TEST((void *)vcpuins.rm,(void *)vcpuins.imm,16);
			break;
	case 2:	NOT ((void *)vcpuins.rm,16);	break;
	case 3:	NEG ((void *)vcpuins.rm,16);	break;
	case 4:	MUL ((void *)vcpuins.rm,16);	break;
	case 5:	IMUL((void *)vcpuins.rm,16);	break;
	case 6:	DIV ((void *)vcpuins.rm,16);	break;
	case 7:	IDIV((void *)vcpuins.rm,16);	break;
	default:CaseError("INS_F7::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_F7\n");
}
static void CLC()
{
	vcpu.ip++;
	async;
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
#define op clc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"CLC");
}
static void STC()
{
	vcpu.ip++;
	async;
	SetBit(vcpu.flags, VCPU_FLAG_CF);
#define op stc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"STC");
}
static void CLI()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_FLAG_IF);
}
static void STI()
{
	vcpu.ip++;
	SetBit(vcpu.flags, VCPU_FLAG_IF);
}
static void CLD()
{
	vcpu.ip++;
	async;
	ClrBit(vcpu.flags, VCPU_FLAG_DF);
#define op cld
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"CLD");
}
static void STD()
{
	vcpu.ip++;
	async;
	SetBit(vcpu.flags,VCPU_FLAG_DF);
#define op STD
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs,vcpu.ip-1,"STD");
}
static void INS_FE()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	INC((void *)vcpuins.rm,8);	break;
	case 1:	DEC((void *)vcpuins.rm,8);	break;
	default:CaseError("INS_FE::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_FE\n");
}
static void INS_FF()
{
	async;
	vcpu.ip++;
	GetModRegRM(0,16);
	aipcheck;
	switch(vcpuins.r) {
	case 0:	INC((void *)vcpuins.rm,16);	break;
	case 1:	DEC((void *)vcpuins.rm,16);	break;
	case 2:	/* CALL_RM16 */
		PUSH((void *)&vcpu.ip,16);
		vcpu.ip = d_nubit16(vcpuins.rm);
		break;
	case 3:	/* CALL_M16_16 */
		PUSH((void *)&vcpu.cs,16);
		PUSH((void *)&vcpu.ip,16);
		vcpu.ip = d_nubit16(vcpuins.rm);
		bugfix(11) vcpu.cs = d_nubit16(vcpuins.rm+2);
		else vcpu.cs = d_nubit16(vcpuins.rm+1);
		break;
	case 4:	/* JMP_RM16 */
		vcpu.ip = d_nubit16(vcpuins.rm);
		break;
	case 5:	/* JMP_M16_16 */
		vcpu.ip = d_nubit16(vcpuins.rm);
		bugfix(11) vcpu.cs = d_nubit16(vcpuins.rm+2);
		else vcpu.cs = d_nubit16(vcpuins.rm+1);
		break;
	case 6:	/* PUSH_RM16 */
		PUSH((void *)vcpuins.rm,16);
		break;
	default:CaseError("INS_FF::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_FF\n");
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
	vcpu.overds = vcpu.ds;
	vcpu.overss = vcpu.ss;
	vcpuins.rep = RT_NONE;
}
static void ExecIns()
{
	t_nubit8 opcode;
		do {
			opcode = vramVarByte(vcpu.cs, vcpu.ip);
			ExecFun(vcpuins.table[opcode]);
		} while (IsPrefix(opcode));
		ClrPrefix();
}
static void ExecInt()
{
	/* hardware interrupt handeler */
	if(vcpu.flagnmi) INT(0x02);
	vcpu.flagnmi = 0x00;

	if(GetBit(vcpu.flags, VCPU_FLAG_IF) && vpicScanINTR())
		INT(vpicGetINTR());

	if(GetBit(vcpu.flags, VCPU_FLAG_TF)) INT(0x01);
}

static void QDX()
{
	vcpu.ip++;
	GetImm(8);
	if (d_nubit8(vcpuins.imm) == 0xff) {
		vapiPrint("\nNXVM stopped at CS:%04X IP:%04X by QDX FF\n",vcpu.cs,vcpu.ip);
		vapiPrint("This happens because of the special stop instruction.\n");
		vapiCallBackMachineStop();
		return;
	}
	qdbiosExecInt(d_nubit8(vcpuins.imm));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_ZF, GetBit(_flags, VCPU_FLAG_ZF));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_CF, GetBit(_flags, VCPU_FLAG_CF));
}

void dasm(t_string stmt, t_vaddrcc base, t_nubit16 seg, t_nubit16 off)
{
	t_nubit8 op = 0x00;
	switch (op) {
	case 0x00: ADD_RM8_R8();
	break;case 0x01: ADD_RM16_R16();
	break;case 0x02: ADD_R8_RM8();
	break;case 0x03: ADD_R16_RM16();
	break;case 0x04: ADD_AL_I8();
	break;case 0x05: ADD_AX_I16();
	break;case 0x06: PUSH_ES();
	break;case 0x07: POP_ES();
	break;case 0x08: OR_RM8_R8();
	break;case 0x09: OR_RM16_R16();
	break;case 0x0a: OR_R8_RM8();
	break;case 0x0b: OR_R16_RM16();
	break;case 0x0c: OR_AL_I8();
	break;case 0x0d: OR_AX_I16();
	break;case 0x0e: PUSH_CS();
	break;case 0x0f: POP_CS();
	//break;case 0x0f: INS_0F();
	break;case 0x10: ADC_RM8_R8();
	break;case 0x11: ADC_RM16_R16();
	break;case 0x12: ADC_R8_RM8();
	break;case 0x13: ADC_R16_RM16();
	break;case 0x14: ADC_AL_I8();
	break;case 0x15: ADC_AX_I16();
	break;case 0x16: PUSH_SS();
	break;case 0x17: POP_SS();
	break;case 0x18: SBB_RM8_R8();
	break;case 0x19: SBB_RM16_R16();
	break;case 0x1a: SBB_R8_RM8();
	break;case 0x1b: SBB_R16_RM16();
	break;case 0x1c: SBB_AL_I8();
	break;case 0x1d: SBB_AX_I16();
	break;case 0x1e: PUSH_DS();
	break;case 0x1f: POP_DS();
	break;case 0x20: AND_RM8_R8();
	break;case 0x21: AND_RM16_R16();
	break;case 0x22: AND_R8_RM8();
	break;case 0x23: AND_R16_RM16();
	break;case 0x24: AND_AL_I8();
	break;case 0x25: AND_AX_I16();
	break;case 0x26: ES();
	break;case 0x27: DAA();
	break;case 0x28: SUB_RM8_R8();
	break;case 0x29: SUB_RM16_R16();
	break;case 0x2a: SUB_R8_RM8();
	break;case 0x2b: SUB_R16_RM16();
	break;case 0x2c: SUB_AL_I8();
	break;case 0x2d: SUB_AX_I16();
	break;case 0x2e: CS();
	break;case 0x2f: DAS();
	break;case 0x30: XOR_RM8_R8();
	break;case 0x31: XOR_RM16_R16();
	break;case 0x32: XOR_R8_RM8();
	break;case 0x33: XOR_R16_RM16();
	break;case 0x34: XOR_AL_I8();
	break;case 0x35: XOR_AX_I16();
	break;case 0x36: SS();
	break;case 0x37: AAA();
	break;case 0x38: CMP_RM8_R8();
	break;case 0x39: CMP_RM16_R16();
	break;case 0x3a: CMP_R8_RM8();
	break;case 0x3b: CMP_R16_RM16();
	break;case 0x3c: CMP_AL_I8();
	break;case 0x3d: CMP_AX_I16();
	break;case 0x3e: DS();
	break;case 0x3f: AAS();
	break;case 0x40: INC_AX();
	break;case 0x41: INC_CX();
	break;case 0x42: INC_DX();
	break;case 0x43: INC_BX();
	break;case 0x44: INC_SP();
	break;case 0x45: INC_BP();
	break;case 0x46: INC_SI();
	break;case 0x47: INC_DI();
	break;case 0x48: DEC_AX();
	break;case 0x49: DEC_CX();
	break;case 0x4a: DEC_DX();
	break;case 0x4b: DEC_BX();
	break;case 0x4c: DEC_SP();
	break;case 0x4d: DEC_BP();
	break;case 0x4e: DEC_SI();
	break;case 0x4f: DEC_DI();
	break;case 0x50: PUSH_AX();
	break;case 0x51: PUSH_CX();
	break;case 0x52: PUSH_DX();
	break;case 0x53: PUSH_BX();
	break;case 0x54: PUSH_SP();
	break;case 0x55: PUSH_BP();
	break;case 0x56: PUSH_SI();
	break;case 0x57: PUSH_DI();
	break;case 0x58: POP_AX();
	break;case 0x59: POP_CX();
	break;case 0x5a: POP_DX();
	break;case 0x5b: POP_BX();
	break;case 0x5c: POP_SP();
	break;case 0x5d: POP_BP();
	break;case 0x5e: POP_SI();
	break;case 0x5f: POP_DI();
	break;case 0x60: OpError();
	break;case 0x61: OpError();
	break;case 0x62: OpError();
	break;case 0x63: OpError();
	break;case 0x64: OpError();
	break;case 0x65: OpError();
	break;case 0x66: OpError();
	break;case 0x67: OpError();
	break;case 0x68: OpError();
	//break;case 0x66: OpdSize();
	//break;case 0x67: AddrSize();
	//break;case 0x68: PUSH_I16();
	break;case 0x69: OpError();
	break;case 0x6a: OpError();
	break;case 0x6b: OpError();
	break;case 0x6c: OpError();
	break;case 0x6d: OpError();
	break;case 0x6e: OpError();
	break;case 0x6f: OpError();
	break;case 0x70: JO();
	break;case 0x71: JNO();
	break;case 0x72: JC();
	break;case 0x73: JNC();
	break;case 0x74: JZ();
	break;case 0x75: JNZ();
	break;case 0x76: JBE();
	break;case 0x77: JA();
	break;case 0x78: JS();
	break;case 0x79: JNS();
	break;case 0x7a: JP();
	break;case 0x7b: JNP();
	break;case 0x7c: JL();
	break;case 0x7d: JNL();
	break;case 0x7e: JLE();
	break;case 0x7f: JG();
	break;case 0x80: INS_80();
	break;case 0x81: INS_81();
	break;case 0x82: INS_82();
	break;case 0x83: INS_83();
	break;case 0x84: TEST_RM8_R8();
	break;case 0x85: TEST_RM16_R16();
	break;case 0x86: XCHG_R8_RM8();
	break;case 0x87: XCHG_R16_RM16();
	break;case 0x88: MOV_RM8_R8();
	break;case 0x89: MOV_RM16_R16();
	break;case 0x8a: MOV_R8_RM8();
	break;case 0x8b: MOV_R16_RM16();
	break;case 0x8c: MOV_RM16_SEG();
	break;case 0x8d: LEA_R16_M16();
	break;case 0x8e: MOV_SEG_RM16();
	break;case 0x8f: POP_RM16();
	break;case 0x90: NOP();
	break;case 0x91: XCHG_CX_AX();
	break;case 0x92: XCHG_DX_AX();
	break;case 0x93: XCHG_BX_AX();
	break;case 0x94: XCHG_SP_AX();
	break;case 0x95: XCHG_BP_AX();
	break;case 0x96: XCHG_SI_AX();
	break;case 0x97: XCHG_DI_AX();
	break;case 0x98: CBW();
	break;case 0x99: CWD();
	break;case 0x9a: CALL_PTR16_16();
	break;case 0x9b: WAIT();
	break;case 0x9c: PUSHF();
	break;case 0x9d: POPF();
	break;case 0x9e: SAHF();
	break;case 0x9f: LAHF();
	break;case 0xa0: MOV_AL_M8();
	break;case 0xa1: MOV_AX_M16();
	break;case 0xa2: MOV_M8_AL();
	break;case 0xa3: MOV_M16_AX();
	break;case 0xa4: MOVSB();
	break;case 0xa5: MOVSW();
	break;case 0xa6: CMPSB();
	break;case 0xa7: CMPSW();
	break;case 0xa8: TEST_AL_I8();
	break;case 0xa9: TEST_AX_I16();
	break;case 0xaa: STOSB();
	break;case 0xab: STOSW();
	break;case 0xac: LODSB();
	break;case 0xad: LODSW();
	break;case 0xae: SCASB();
	break;case 0xaf: SCASW();
	break;case 0xb0: MOV_AL_I8();
	break;case 0xb1: MOV_CL_I8();
	break;case 0xb2: MOV_DL_I8();
	break;case 0xb3: MOV_BL_I8();
	break;case 0xb4: MOV_AH_I8();
	break;case 0xb5: MOV_CH_I8();
	break;case 0xb6: MOV_DH_I8();
	break;case 0xb7: MOV_BH_I8();
	break;case 0xb8: MOV_AX_I16();
	break;case 0xb9: MOV_CX_I16();
	break;case 0xba: MOV_DX_I16();
	break;case 0xbb: MOV_BX_I16();
	break;case 0xbc: MOV_SP_I16();
	break;case 0xbd: MOV_BP_I16();
	break;case 0xbe: MOV_SI_I16();
	break;case 0xbf: MOV_DI_I16();
	break;case 0xc0: OpError();
	break;case 0xc1: OpError();
	break;case 0xc2: RET_I16();
	break;case 0xc3: RET();
	break;case 0xc4: LES_R16_M16();
	break;case 0xc5: LDS_R16_M16();
	break;case 0xc6: MOV_M8_I8();
	break;case 0xc7: MOV_M16_I16();
	break;case 0xc8: OpError();
	break;case 0xc9: OpError();
	break;case 0xca: RETF_I16();
	break;case 0xcb: RETF();
	break;case 0xcc: INT3();
	break;case 0xcd: INT_I8();
	break;case 0xce: INTO();
	break;case 0xcf: IRET();
	break;case 0xd0: INS_D0();
	break;case 0xd1: INS_D1();
	break;case 0xd2: INS_D2();
	break;case 0xd3: INS_D3();
	break;case 0xd4: AAM();
	break;case 0xd5: AAD();
	break;case 0xd6: OpError();
	break;case 0xd7: XLAT();
	break;case 0xd8: OpError();
	break;case 0xd9: OpError();
	//break;case 0xd9: INS_D9();
	break;case 0xda: OpError();
	break;case 0xdb: OpError();
	//break;case 0xdb: INS_DB();
	break;case 0xdc: OpError();
	break;case 0xdd: OpError();
	break;case 0xde: OpError();
	break;case 0xdf: OpError();
	break;case 0xe0: LOOPNZ();
	break;case 0xe1: LOOPZ();
	break;case 0xe2: LOOP();
	break;case 0xe3: JCXZ_REL8();
	break;case 0xe4: IN_AL_I8();
	break;case 0xe5: IN_AX_I8();
	break;case 0xe6: OUT_I8_AL();
	break;case 0xe7: OUT_I8_AX();
	break;case 0xe8: CALL_REL16();
	break;case 0xe9: JMP_REL16();
	break;case 0xea: JMP_PTR16_16();
	break;case 0xeb: JMP_REL8();
	break;case 0xec: IN_AL_DX();
	break;case 0xed: IN_AX_DX();
	break;case 0xee: OUT_DX_AL();
	break;case 0xef: OUT_DX_AX();
	break;case 0xf0: LOCK();
	break;case 0xf1: QDX();
	break;case 0xf2: REPNZ();
	break;case 0xf3: REP();
	break;case 0xf4: HLT();
	break;case 0xf5: CMC();
	break;case 0xf6: INS_F6();
	break;case 0xf7: INS_F7();
	break;case 0xf8: CLC();
	break;case 0xf9: STC();
	break;case 0xfa: CLI();
	break;case 0xfb: STI();
	break;case 0xfc: CLD();
	break;case 0xfd: STD();
	break;case 0xfe: INS_FE();
	break;case 0xff: INS_FF();
	}
}