/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"

#include "vapi.h"

#include "vport.h"
#include "vram.h"
#include "vcpu.h"
#include "vpic.h"
#include "bios/qdbios.h"
#include "vcpuins.h"

static t_faddrcc table[0x100];

#define MOD ((modrm&0xc0)>>6)
#define REG ((modrm&0x38)>>3)
#define RM  ((modrm&0x07)>>0)

#define ADD_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define	 OR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define ADC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define SBB_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define AND_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SUB_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define XOR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define CMP_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define INC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define DEC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define TEST_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAM_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAD_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAA_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAS_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

t_cpuins vcpuins;
t_cpurec vcpurec;

static t_vaddrcc rimm;

#define todo static void

static void CaseError(const char *str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: %s.\n",str);
	vapiCallBackMachineStop();
}
static void CalcCF()
{
	switch(vcpuins.type) {
	case ADD8:
	case ADD16:
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,(vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2));
		break;
	case ADC8:
		MakeBit(vcpu.flags, VCPU_EFLAGS_CF, (
			(GetBit(vcpu.flags, VCPU_EFLAGS_CF) && vcpuins.opr2 == 0xff) ?
			1 : ((vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2))));
		break;
	case ADC16:
		MakeBit(vcpu.flags, VCPU_EFLAGS_CF, (
			(GetBit(vcpu.flags, VCPU_EFLAGS_CF) && vcpuins.opr2 == 0xffff) ?
			1 : ((vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2))));
		break;
	case SBB8:
		MakeBit(vcpu.flags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(GetBit(vcpu.flags, VCPU_EFLAGS_CF) && (vcpuins.opr2 == 0xff)));
		break;
	case SBB16:
		MakeBit(vcpu.flags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(GetBit(vcpu.flags, VCPU_EFLAGS_CF) && (vcpuins.opr2 == 0xffff)));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,vcpuins.opr1 < vcpuins.opr2);
		break;
	default:CaseError("CalcCF::vcpuins.type");break;}
}
static void CalcOF()
{
	switch(vcpuins.type) {
	case ADD8:
	case ADC8:
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x0080) == (vcpuins.opr2&0x0080)) && ((vcpuins.opr1&0x0080) != (vcpuins.result&0x0080)));
		break;
	case ADD16:
	case ADC16:
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x8000) == (vcpuins.opr2&0x8000)) && ((vcpuins.opr1&0x8000) != (vcpuins.result&0x8000)));
		break;
	case SBB8:
	case SUB8:
	case CMP8:
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x0080) != (vcpuins.opr2&0x0080)) && ((vcpuins.opr2&0x0080) == (vcpuins.result&0x0080)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x8000) != (vcpuins.opr2&0x8000)) && ((vcpuins.opr2&0x8000) == (vcpuins.result&0x8000)));
		break;
	default:CaseError("CalcOF::vcpuins.type");break;}
}
static void CalcAF()
{
	MakeBit(vcpu.flags,VCPU_EFLAGS_AF,((vcpuins.opr1^vcpuins.opr2)^vcpuins.result)&0x10);
}
static void CalcPF()
{
	t_nubit8 res8 = vcpuins.result & 0xff;
	t_nubitcc count = 0;
	while(res8)
	{
		res8 &= res8-1; 
		count++;
	}
	MakeBit(vcpu.flags,VCPU_EFLAGS_PF,!(count&0x01));
}
static void CalcZF()
{
	MakeBit(vcpu.flags,VCPU_EFLAGS_ZF,!vcpuins.result);
}
static void CalcSF()
{
	switch(vcpuins.bit) {
	case 8:	MakeBit(vcpu.flags,VCPU_EFLAGS_SF,!!(vcpuins.result&0x80));break;
	case 16:MakeBit(vcpu.flags,VCPU_EFLAGS_SF,!!(vcpuins.result&0x8000));break;
	default:CaseError("CalcSF::vcpuins.bit");break;}
}
static void CalcTF() {}
static void CalcIF() {}
static void CalcDF() {}

static void SetFlags(t_nubit16 flags)
{
	if(flags & VCPU_EFLAGS_CF) CalcCF();
	if(flags & VCPU_EFLAGS_PF) CalcPF();
	if(flags & VCPU_EFLAGS_AF) CalcAF();
	if(flags & VCPU_EFLAGS_ZF) CalcZF();
	if(flags & VCPU_EFLAGS_SF) CalcSF();
	if(flags & VCPU_EFLAGS_TF) CalcTF();
	if(flags & VCPU_EFLAGS_IF) CalcIF();
	if(flags & VCPU_EFLAGS_DF) CalcDF();
	if(flags & VCPU_EFLAGS_OF) CalcOF();
}
static void GetMem()
{
	/* returns vcpuins.rrm */
	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vramRealWord(vcpu.cs.selector,vcpu.ip));
	vcpu.ip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns rimm
	rimm = vramGetRealAddr(vcpu.cs.selector,vcpu.ip);
	switch(immbit) {
	case 8:		vcpu.ip += 1;break;
	case 16:	vcpu.ip += 2;break;
	case 32:	vcpu.ip += 4;break;
	default:CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns vcpuins.rrm and vcpuins.rr
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramRealByte(vcpu.cs.selector,vcpu.ip++);
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.si);break;
		case 5:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.di);break;
		case 6: disp16 = vramRealWord(vcpu.cs.selector,vcpu.ip);vcpu.ip += 2;
			    vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,disp16); break;
		case 7:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::RM");break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramRealByte(vcpu.cs.selector,vcpu.ip);vcpu.ip += 1;
		switch(RM) {
		case 0:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.si+disp8);break;
		case 1:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.di+disp8);break;
		case 2:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.si+disp8);break;
		case 3:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.di+disp8);break;
		case 4:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.si+disp8);break;
		case 5:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.di+disp8);break;
		case 6:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+disp8);break;
		case 7:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+disp8);break;
		default:CaseError("GetModRegRM::MOD1::RM");break;}
		break;
	case 2:
		disp16 = vramRealWord(vcpu.cs.selector,vcpu.ip);vcpu.ip += 2;
		switch(RM) {
		case 0:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.si+disp16);break;
		case 1:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+vcpu.di+disp16);break;
		case 2:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.si+disp16);break;
		case 3:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+vcpu.di+disp16);break;
		case 4:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.si+disp16);break;
		case 5:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.di+disp16);break;
		case 6:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverss->selector,vcpu.bp+disp16);break;
		case 7:	vcpuins.rrm = vramGetRealAddr(vcpuins.roverds->selector,vcpu.bx+disp16);break;
		default:CaseError("GetModRegRM::MOD2::RM");break;}
		break;
	case 3:
		switch(RM) {
		case 0:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.al); else vcpuins.rrm = GetRef(vcpu.ax); break;
		case 1:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.cl); else vcpuins.rrm = GetRef(vcpu.cx); break;
		case 2:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.dl); else vcpuins.rrm = GetRef(vcpu.dx); break;
		case 3:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.bl); else vcpuins.rrm = GetRef(vcpu.bx); break;
		case 4:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.ah); else vcpuins.rrm = GetRef(vcpu.sp); break;
		case 5:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.ch); else vcpuins.rrm = GetRef(vcpu.bp); break;
		case 6:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.dh); else vcpuins.rrm = GetRef(vcpu.si); break;
		case 7:	if(rmbit == 8) vcpuins.rrm = GetRef(vcpu.bh); else vcpuins.rrm = GetRef(vcpu.di); break;
		default:CaseError("GetModRegRM::MOD3::RM");break;}
		break;
	default:CaseError("GetModRegRM::MOD");break;}
	switch(regbit) {
	case 0:		vcpuins.cr = REG;					break;
	case 4:
		switch(REG) {
		case 0:	vcpuins.rr = GetRef(vcpu.es.selector);	break;
		case 1:	vcpuins.rr = GetRef(vcpu.cs.selector);	break;
		case 2:	vcpuins.rr = GetRef(vcpu.ss.selector);	break;
		case 3:	vcpuins.rr = GetRef(vcpu.ds.selector);	break;
		default:CaseError("GetModRegRM::regbit4::REG");break;}
		break;
	case 8:
		switch(REG) {
		case 0:	vcpuins.rr = GetRef(vcpu.al);	break;
		case 1:	vcpuins.rr = GetRef(vcpu.cl);	break;
		case 2:	vcpuins.rr = GetRef(vcpu.dl);	break;
		case 3:	vcpuins.rr = GetRef(vcpu.bl);	break;
		case 4:	vcpuins.rr = GetRef(vcpu.ah);	break;
		case 5:	vcpuins.rr = GetRef(vcpu.ch);	break;
		case 6:	vcpuins.rr = GetRef(vcpu.dh);	break;
		case 7:	vcpuins.rr = GetRef(vcpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::REG");break;}
		break;
	case 16:
		switch(REG) {
		case 0: vcpuins.rr = GetRef(vcpu.ax);	break;
		case 1:	vcpuins.rr = GetRef(vcpu.cx);	break;
		case 2:	vcpuins.rr = GetRef(vcpu.dx);	break;
		case 3:	vcpuins.rr = GetRef(vcpu.bx);	break;
		case 4:	vcpuins.rr = GetRef(vcpu.sp);	break;
		case 5:	vcpuins.rr = GetRef(vcpu.bp);	break;
		case 6:	vcpuins.rr = GetRef(vcpu.si);	break;
		case 7: vcpuins.rr = GetRef(vcpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramRealByte(vcpu.cs.selector,vcpu.ip++);
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rrm = vcpu.si;break;
		case 5:	vcpuins.rrm = vcpu.di;break;
		case 6:	vcpuins.rrm = vramRealWord(vcpu.cs.selector,vcpu.ip);vcpu.ip += 2;break;
		case 7:	vcpuins.rrm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::RM");break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramRealByte(vcpu.cs.selector,vcpu.ip);vcpu.ip += 1;
		switch(RM) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si+disp8;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di+disp8;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si+disp8;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di+disp8;break;
		case 4:	vcpuins.rrm = vcpu.si+disp8;break;
		case 5:	vcpuins.rrm = vcpu.di+disp8;break;
		case 6:	vcpuins.rrm = vcpu.bp+disp8;break;
		case 7:	vcpuins.rrm = vcpu.bx+disp8;break;
		default:CaseError("GetModRegRMEA::MOD1::RM");break;}
		vcpuins.rrm %= 0x10000;
		break;
	case 2:
		disp16 = vramRealWord(vcpu.cs.selector,vcpu.ip);vcpu.ip += 2;
		switch(RM) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si+disp16;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di+disp16;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si+disp16;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di+disp16;break;
		case 4:	vcpuins.rrm = vcpu.si+disp16;break;
		case 5:	vcpuins.rrm = vcpu.di+disp16;break;
		case 6:
			vcpuins.rrm = vcpu.bp+disp16;
			break;
		case 7:	vcpuins.rrm = vcpu.bx+disp16;break;
		default:CaseError("GetModRegRMEA::MOD2::RM");break;}
		vcpuins.rrm %= 0x10000;
		break;
	default:CaseError("GetModRegRMEA::MOD");break;}
	switch(REG) {
	case 0: vcpuins.rr = GetRef(vcpu.ax);	break;
	case 1:	vcpuins.rr = GetRef(vcpu.cx);	break;
	case 2:	vcpuins.rr = GetRef(vcpu.dx);	break;
	case 3:	vcpuins.rr = GetRef(vcpu.bx);	break;
	case 4:	vcpuins.rr = GetRef(vcpu.sp);	break;
	case 5:	vcpuins.rr = GetRef(vcpu.bp);	break;
	case 6:	vcpuins.rr = GetRef(vcpu.si);	break;
	case 7: vcpuins.rr = GetRef(vcpu.di);	break;
	default:CaseError("GetModRegRMEA::REG");break;}
}

/* abstract instructions */
static void INT(t_nubit8 intid);
static void ADD(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
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
}
static void OR(void *dest, void *src, t_nubit8 len)
{
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
	ClrBit(vcpu.flags, VCPU_EFLAGS_OF);
	ClrBit(vcpu.flags, VCPU_EFLAGS_CF);
	ClrBit(vcpu.flags, VCPU_EFLAGS_AF);
	SetFlags(OR_FLAG);
}
static void ADC(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF)) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("ADC::len");break;}
	SetFlags(ADC_FLAG);
}
static void SBB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SBB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF))) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("SBB::len");break;}
	SetFlags(SBB_FLAG);
}
static void AND(void *dest, void *src, t_nubit8 len)
{
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
	ClrBit(vcpu.flags,VCPU_EFLAGS_OF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
	SetFlags(AND_FLAG);
}
static void SUB(void *dest, void *src, t_nubit8 len)
{
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
}
static void XOR(void *dest, void *src, t_nubit8 len)
{
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
	ClrBit(vcpu.flags,VCPU_EFLAGS_OF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
	SetFlags(XOR_FLAG);
}
static void CMP(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nsbit8(src) & 0xff;
		vcpuins.result = ((t_nubit8)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xff;
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
		vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit16)vcpuins.opr2)&0xffff;
		break;
	default:CaseError("CMP::len");break;}
	SetFlags(CMP_FLAG);
}
static void PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		vcpu.sp -= 2;
		vramRealWord(vcpu.ss.selector,vcpu.sp) = data;
		break;
	default:CaseError("PUSH::len");break;}
}
static void POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = vramRealWord(vcpu.ss.selector,vcpu.sp);
		vcpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
static void INC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("INC::len");break;}
	SetFlags(INC_FLAG);
}
static void DEC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("DEC::len");break;}
	SetFlags(DEC_FLAG);
}
static void JCC(void *src, t_bool flagj,t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(flagj)
			vcpu.ip += d_nsbit8(src);
		break;
	default:CaseError("JCC::len");break;}
}
static void TEST(void *dest, void *src, t_nubit8 len)
{
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
	ClrBit(vcpu.flags,VCPU_EFLAGS_OF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
	SetFlags(TEST_FLAG);
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
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+(t_nubit8)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		break;
	case 16:
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		break;
	default:CaseError("ROL::len");break;}
}
static void ROR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(tempCF) d_nubit8(dest) |= 0x80;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^(!!(d_nubit8(dest)&0x40)));
		break;
	case 16:
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(tempCF) d_nubit16(dest) |= 0x8000;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^(!!(d_nubit16(dest)&0x4000)));
		break;
	default:CaseError("ROR::len");break;}
}
static void RCL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+GetBit(vcpu.flags, VCPU_EFLAGS_CF);
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		break;
	case 16:
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+GetBit(vcpu.flags, VCPU_EFLAGS_CF);
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		break;
	default:CaseError("RCL::len");break;}
}
static void RCR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(GetBit(vcpu.flags, VCPU_EFLAGS_CF)) d_nubit8(dest) |= 0x80;
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		break;
	case 16:
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(GetBit(vcpu.flags, VCPU_EFLAGS_CF)) d_nubit16(dest) |= 0x8000;
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		break;
	default:CaseError("RCR::len");break;}
}
static void SHL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			SetFlags(SHL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SHL_FLAG);
		}
		break;
	default:CaseError("SHL::len");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SHR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,!!(tempdest8&0x80));
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			SetFlags(SHR_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,!!(tempdest16&0x8000));
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SHR_FLAG);
		}
		break;
	default:CaseError("SHR::len");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SAL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			SetFlags(SAL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_EFLAGS_CF));
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SAL_FLAG);
		}
		break;
	default:CaseError("SAL::len");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SAR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= tempdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,0);
		if(count != 0) {
			vcpuins.result = d_nsbit8(dest);
			SetFlags(SAR_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= tempdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_EFLAGS_OF,0);
		if(count != 0) {
			vcpuins.result = d_nsbit16(dest);
			SetFlags(SAR_FLAG);
		}
		break;
	default:CaseError("SAR::len");break;}
	if (count) _ClrEFLAGS_AF;
}
static void STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(GetBit(vcpu.flags, VCPU_EFLAGS_DF)) {
			if (flagdi) vcpu.di--;
			if (flagsi) vcpu.si--;
		} else {
			if (flagdi) vcpu.di++;
			if (flagsi) vcpu.si++;
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(GetBit(vcpu.flags, VCPU_EFLAGS_DF)) {
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
		vramRealByte(vcpu.es.selector,vcpu.di) = vramRealByte(vcpuins.roverds->selector,vcpu.si);
		STRDIR(8,1,1);
		break;
	case 16:
		vcpuins.bit = 16;
		vramRealWord(vcpu.es.selector,vcpu.di) = vramRealWord(vcpuins.roverds->selector,vcpu.si);
		STRDIR(16,1,1);
		break;
	default:CaseError("MOVS::len");break;}
}
static void CMPS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vramRealByte(vcpuins.roverds->selector,vcpu.si);
		vcpuins.opr2 = vramRealByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vramRealWord(vcpuins.roverds->selector,vcpu.si);
		vcpuins.opr2 = vramRealWord(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("CMPS::len");break;}
}
static void STOS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vramRealByte(vcpu.es.selector,vcpu.di) = vcpu.al;
		STRDIR(8,0,1);
		break;
	case 16:
		vcpuins.bit = 16;
		vramRealWord(vcpu.es.selector,vcpu.di) = vcpu.ax;
		STRDIR(16,0,1);
		break;
	default:CaseError("STOS::len");break;}
}
static void LODS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpu.al = vramRealByte(vcpuins.roverds->selector,vcpu.si);
		STRDIR(8,1,0);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpu.ax = vramRealWord(vcpuins.roverds->selector,vcpu.si);
		STRDIR(16,1,0);
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
		vcpuins.opr2 = vramRealByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = vramRealWord(vcpu.es.selector,vcpu.di);
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
		vcpuins.bit = 8;
		d_nubit8(dest) = ~d_nubit8(dest);
		break;
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = ~d_nubit16(dest);
		break;
	default:CaseError("NOT::len");break;}
}
static void NEG(void *dest, t_nubit8 len)
{
	t_nubitcc zero = 0;
	switch(len) {
	case 8:	
		vcpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		d_nubit8(dest) = (t_nubit8)zero;
		break;
	case 16:
		vcpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		d_nubit16(dest) = (t_nubit16)zero;
		break;
	default:CaseError("NEG::len");break;}
}
static void MUL(void *src, t_nubit8 len)
{
	t_nubit32 tempresult;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpu.ax = vcpu.al * d_nubit8(src);
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,!!vcpu.ah);
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,!!vcpu.ah);
		break;
	case 16:
		vcpuins.bit = 16;
		tempresult = vcpu.ax * d_nubit16(src);
		vcpu.dx = (tempresult>>16)&0xffff;
		vcpu.ax = tempresult&0xffff;
		MakeBit(vcpu.flags,VCPU_EFLAGS_OF,!!vcpu.dx);
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,!!vcpu.dx);
		break;
	default:CaseError("MUL::len");break;}
}
static void IMUL(void *src, t_nubit8 len)
{
	t_nsbit32 tempresult;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpu.ax = (t_nsbit8)vcpu.al * d_nsbit8(src);
		if(vcpu.ax == vcpu.al) {
			ClrBit(vcpu.flags,VCPU_EFLAGS_OF);
			ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(vcpu.flags,VCPU_EFLAGS_OF);
			SetBit(vcpu.flags,VCPU_EFLAGS_CF);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempresult = (t_nsbit16)vcpu.ax * d_nsbit16(src);
		vcpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		vcpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)vcpu.ax) {
			ClrBit(vcpu.flags,VCPU_EFLAGS_OF);
			ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(vcpu.flags,VCPU_EFLAGS_OF);
			SetBit(vcpu.flags,VCPU_EFLAGS_CF);
		}
		break;
	default:CaseError("IMUL::len");break;}
}
static void DIV(void *src, t_nubit8 len)
{
	t_nubit16 tempAX = vcpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
		break;
	default:CaseError("DIV::len");break;}
}
static void IDIV(void *src, t_nubit8 len)
{
	t_nsbit16 tempAX = vcpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
		break;
	default:CaseError("IDIV::len");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&vcpu.flags,16);
	ClrBit(vcpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
	PUSH((void *)&vcpu.cs.selector,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = vramRealWord(0x0000,intid*4+0);
	vcpu.cs.selector = vramRealWord(0x0000,intid*4+2);
}

/* regular instructions */
static void UndefinedOpcode()
{
	vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
	vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
		vcpu.cs.selector, vcpu.ip, vramRealByte(vcpu.cs.selector,vcpu.ip+0),
		vramRealByte(vcpu.cs.selector,vcpu.ip+1), vramRealByte(vcpu.cs.selector,vcpu.ip+2),
		vramRealByte(vcpu.cs.selector,vcpu.ip+3), vramRealByte(vcpu.cs.selector,vcpu.ip+4));
	vapiCallBackMachineStop();
}
static void ADD_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void ADD_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void ADD_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void ADD_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void ADD_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADD((void *)&vcpu.al,(void *)rimm,8);
}
static void ADD_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADD((void *)&vcpu.ax,(void *)rimm,16);
}
static void PUSH_ES()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.es.selector,16);
}
static void POP_ES()
{
	vcpu.ip++;
	POP((void *)&vcpu.es.selector,16);
}
static void OR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void OR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void OR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void OR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void OR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)rimm,8);
}
static void OR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)rimm,16);
}
static void PUSH_CS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cs.selector,16);
}
static void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs.selector,16);
}
static void INS_0F()
{
	UndefinedOpcode();
}
static void ADC_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void ADC_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void ADC_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void ADC_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void ADC_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADC((void *)&vcpu.al,(void *)rimm,8);
}
static void ADC_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADC((void *)&vcpu.ax,(void *)rimm,16);
}
static void PUSH_SS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ss.selector,16);
}
static void POP_SS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ss.selector,16);
}
static void SBB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void SBB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void SBB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void SBB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void SBB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)rimm,8);
}
static void SBB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)rimm,16);
}
static void PUSH_DS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ds.selector,16);
}
static void POP_DS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ds.selector,16);
}
static void AND_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void AND_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void AND_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void AND_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void AND_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	AND((void *)&vcpu.al,(void *)rimm,8);
}
static void AND_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	AND((void *)&vcpu.ax,(void *)rimm,16);
}
static void ES()
{
	vcpu.ip++;
	vcpuins.roverds = &vcpu.es;
	vcpuins.roverss = &vcpu.es;
}
static void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_EFLAGS_AF)) {
		vcpu.al = newAL;
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetBit(vcpu.flags, VCPU_EFLAGS_CF) || ((newAL < oldAL) || (newAL < 0x06)));
		SetBit(vcpu.flags, VCPU_EFLAGS_AF);
	} else ClrBit(vcpu.flags, VCPU_EFLAGS_AF);
	if(((vcpu.al & 0xf0) > 0x90) || GetBit(vcpu.flags, VCPU_EFLAGS_CF)) {
		vcpu.al += 0x60;
		SetBit(vcpu.flags,VCPU_EFLAGS_CF);
	} else ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	vcpuins.bit = 8;
	vcpuins.result = (t_nubitcc)vcpu.al;
	SetFlags(DAA_FLAG);
}
static void SUB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void SUB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void SUB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void SUB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void SUB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)rimm,8);
}
static void SUB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)rimm,16);
}
static void CS()
{
	vcpu.ip++;
	vcpuins.roverds = &vcpu.cs;
	vcpuins.roverss = &vcpu.cs;
}
static void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_EFLAGS_AF)) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.flags,VCPU_EFLAGS_CF,GetBit(vcpu.flags, VCPU_EFLAGS_CF) || (oldAL < 0x06));
		SetBit(vcpu.flags,VCPU_EFLAGS_AF);
	} else ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
	if((vcpu.al > 0x9f) || GetBit(vcpu.flags, VCPU_EFLAGS_CF)) {
		vcpu.al -= 0x60;
		SetBit(vcpu.flags,VCPU_EFLAGS_CF);
	} else ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	vcpuins.bit = 8;
	vcpuins.result = (t_nubitcc)vcpu.al;
	SetFlags(DAS_FLAG);
}
static void XOR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void XOR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void XOR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void XOR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void XOR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)rimm,8);
}
static void XOR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)rimm,16);
}
static void SS()
{
	vcpu.ip++;
	vcpuins.roverds = &vcpu.ss;
	vcpuins.roverss = &vcpu.ss;
}
static void AAA()
{
	vcpu.ip++;
	if(((vcpu.al&0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_EFLAGS_AF)) {
		vcpu.al += 0x06;
		vcpu.ah += 0x01;
		SetBit(vcpu.flags,VCPU_EFLAGS_AF);
		SetBit(vcpu.flags,VCPU_EFLAGS_CF);
	} else {
		ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
		ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
	}
	vcpu.al &= 0x0f;
}
static void CMP_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void CMP_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void CMP_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void CMP_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void CMP_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)rimm,8);
}
static void CMP_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)rimm,16);
}
static void DS()
{
	vcpu.ip++;
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ds;
}
static void AAS()
{
	vcpu.ip++;
	if(((vcpu.al&0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_EFLAGS_AF)) {
		vcpu.al -= 0x06;
		vcpu.ah += 0x01;
		SetBit(vcpu.flags,VCPU_EFLAGS_AF);
		SetBit(vcpu.flags,VCPU_EFLAGS_CF);
	} else {
		ClrBit(vcpu.flags,VCPU_EFLAGS_CF);
		ClrBit(vcpu.flags,VCPU_EFLAGS_AF);
	}
	vcpu.al &= 0x0f;
}
static void INC_AX()
{
	vcpu.ip++;
	INC((void *)&vcpu.ax,16);
}
static void INC_CX()
{
	vcpu.ip++;
	INC((void *)&vcpu.cx,16);
}
static void INC_DX()
{
	vcpu.ip++;
	INC((void *)&vcpu.dx,16);
}
static void INC_BX()
{
	vcpu.ip++;
	INC((void *)&vcpu.bx,16);
}
static void INC_SP()
{
	vcpu.ip++;
	INC((void *)&vcpu.sp,16);
}
static void INC_BP()
{
	vcpu.ip++;
	INC((void *)&vcpu.bp,16);
}
static void INC_SI()
{
	vcpu.ip++;
	INC((void *)&vcpu.si,16);
}
static void INC_DI()
{
	vcpu.ip++;
	INC((void *)&vcpu.di,16);
}
static void DEC_AX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.ax,16);
}
static void DEC_CX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.cx,16);
}
static void DEC_DX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.dx,16);
}
static void DEC_BX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bx,16);
}
static void DEC_SP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.sp,16);
}
static void DEC_BP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bp,16);
}
static void DEC_SI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.si,16);
}
static void DEC_DI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.di,16);
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
	JCC((void *)rimm, GetBit(vcpu.flags, VCPU_EFLAGS_OF), 8);
}
static void JNO()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, !GetBit(vcpu.flags, VCPU_EFLAGS_OF), 8);
}
static void JC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, GetBit(vcpu.flags, VCPU_EFLAGS_CF), 8);
}
static void JNC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, !GetBit(vcpu.flags, VCPU_EFLAGS_CF), 8);
}
static void JZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, GetBit(vcpu.flags, VCPU_EFLAGS_ZF), 8);
}
static void JNZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, !GetBit(vcpu.flags, VCPU_EFLAGS_ZF), 8);
}
static void JBE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (GetBit(vcpu.flags, VCPU_EFLAGS_CF) ||
		GetBit(vcpu.flags, VCPU_EFLAGS_ZF)), 8);
}
static void JA()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (!GetBit(vcpu.flags, VCPU_EFLAGS_CF) &&
		!GetBit(vcpu.flags, VCPU_EFLAGS_ZF)), 8);
}
static void JS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, GetBit(vcpu.flags, VCPU_EFLAGS_SF), 8);
}
static void JNS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, !GetBit(vcpu.flags, VCPU_EFLAGS_SF), 8);
}
static void JP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, GetBit(vcpu.flags, VCPU_EFLAGS_PF), 8);
}
static void JNP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, !GetBit(vcpu.flags, VCPU_EFLAGS_PF), 8);
}
static void JL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (GetBit(vcpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(vcpu.flags, VCPU_EFLAGS_OF)), 8);
}
static void JNL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (GetBit(vcpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(vcpu.flags, VCPU_EFLAGS_OF)), 8);
}
static void JLE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (GetBit(vcpu.flags, VCPU_EFLAGS_ZF) ||
		(GetBit(vcpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(vcpu.flags, VCPU_EFLAGS_OF))), 8);
}
static void JG()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)rimm, (!GetBit(vcpu.flags, VCPU_EFLAGS_ZF) &&
		(GetBit(vcpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(vcpu.flags, VCPU_EFLAGS_OF))), 8);
}
static void INS_80()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch (vcpuins.cr) {
	case 0:	ADD((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 1:	OR ((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 2:	ADC((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 3:	SBB((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 4:	AND((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 5:	SUB((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 6:	XOR((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 7:	CMP((void *)vcpuins.rrm,(void *)rimm,8);break;
	default:CaseError("INS_80::vcpuins.rr");break;}
}
static void INS_81()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch (vcpuins.cr) {
	case 0:	ADD((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 1:	OR ((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 2:	ADC((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 3:	SBB((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 4:	AND((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 5:	SUB((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 6:	XOR((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 7:	CMP((void *)vcpuins.rrm,(void *)rimm,16);break;
	default:CaseError("INS_81::vcpuins.rr");break;}
}
static void INS_82()
{
	INS_80();
}
static void INS_83()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch (vcpuins.cr) {
	case 0:	ADD((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 1:	OR ((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 2:	ADC((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 3:	SBB((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 4:	AND((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 5:	SUB((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 6:	XOR((void *)vcpuins.rrm,(void *)rimm,12);break;
	case 7:	CMP((void *)vcpuins.rrm,(void *)rimm,12);break;
	default:CaseError("INS_83::vcpuins.rr");break;}
}
static void TEST_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void TEST_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void XCHG_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XCHG((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void XCHG_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XCHG((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void MOV_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
}
static void MOV_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void MOV_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
}
static void MOV_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void MOV_RM16_SEG()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
}
static void LEA_R16_M16()
{
	vcpu.ip++;
	GetModRegRMEA();
	d_nubit16(vcpuins.rr) = vcpuins.rrm & 0xffff;
}
static void MOV_SEG_RM16()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
}
static void POP_RM16()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch (vcpuins.cr) {
	case 0:
		POP((void *)vcpuins.rrm,16);
		break;
	default:CaseError("POP_RM16::vcpuins.rr");break;}
}
static void NOP()
{
	vcpu.ip++;
}
static void XCHG_CX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.cx,(void *)&vcpu.ax,16);
}
static void XCHG_DX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.dx,(void *)&vcpu.ax,16);
}
static void XCHG_BX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bx,(void *)&vcpu.ax,16);
}
static void XCHG_SP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.sp,(void *)&vcpu.ax,16);
}
static void XCHG_BP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bp,(void *)&vcpu.ax,16);
}
static void XCHG_SI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.si,(void *)&vcpu.ax,16);
}
static void XCHG_DI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.di,(void *)&vcpu.ax,16);
}
static void CBW()
{
	vcpu.ip++;
	vcpu.ax = (t_nsbit8)vcpu.al;
}
static void CWD()
{
	vcpu.ip++;
	if (vcpu.ax & 0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
}
static void CALL_PTR16_16()
{
	t_nubit16 newcs,newip;
	vcpu.ip++;
	GetImm(16);
	newip = d_nubit16(rimm);
	GetImm(16);
	newcs = d_nubit16(rimm);
	PUSH((void *)&vcpu.cs.selector,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = newip;
	vcpu.cs.selector = newcs;
}
todo WAIT()
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
}
static void LAHF()
{
	vcpu.ip++;
	vcpu.ah = d_nubit8(&vcpu.flags);
}
static void MOV_AL_M8()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.al,(void *)vcpuins.rrm,8);
}
static void MOV_AX_M16()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)vcpuins.rrm,16);
}
static void MOV_M8_AL()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rrm,(void *)&vcpu.al,8);
}
static void MOV_M16_AX()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rrm,(void *)&vcpu.ax,16);
}
static void MOVSB()
{
	vcpu.ip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) MOVS(8);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) MOVS(16);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) CMPS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			CMPS(8);
			vcpu.cx--;
			if((vcpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) || (vcpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(vcpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
}
static void CMPSW()
{
	vcpu.ip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) CMPS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			CMPS(16);
			vcpu.cx--;
			if((vcpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) || (vcpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(vcpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
}
static void TEST_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	TEST((void *)&vcpu.al,(void *)rimm,8);
}
static void TEST_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)rimm,16);
}
static void STOSB()
{
	vcpu.ip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) STOS(16);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) LODS(16);
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
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) SCAS(8);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			SCAS(8);
			vcpu.cx--;
			if((vcpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) || (vcpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(vcpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
}
static void SCASW()
{
	vcpu.ip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) SCAS(16);
	else {
		while(vcpu.cx) {
			//vcpuinsExecInt();
			SCAS(16);
			vcpu.cx--;
			if((vcpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) || (vcpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(vcpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
}
static void MOV_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.al,(void *)rimm,8);
}
static void MOV_CL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)rimm,8);
}
static void MOV_DL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)rimm,8);
}
static void MOV_BL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)rimm,8);
}
static void MOV_AH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)rimm,8);
}
static void MOV_CH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)rimm,8);
}
static void MOV_DH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)rimm,8);
}
static void MOV_BH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)rimm,8);
}
static void MOV_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)rimm,16);
}
static void MOV_CX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)rimm,16);
}
static void MOV_DX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)rimm,16);
}
static void MOV_BX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)rimm,16);
}
static void MOV_SP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)rimm,16);
}
static void MOV_BP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)rimm,16);
}
static void MOV_SI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)rimm,16);
}
static void MOV_DI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)rimm,16);
}
static void INS_C0()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)rimm,8);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)rimm,8);break;
	default:CaseError("INS_C0::vcpuins.rr");break;}
}
static void INS_C1()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)rimm,16);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)rimm,16);break;
	default:CaseError("INS_C1::vcpuins.rr");break;}
}
static void RET_I16()
{
	t_nubit16 addsp;
	vcpu.ip++;
	GetImm(16);
	addsp = d_nubit16(rimm);
	POP((void *)&vcpu.ip,16);
	vcpu.sp += addsp;
}
static void RET()
{
	vcpu.ip++;
	POP((void *)&vcpu.ip,16);
}
static void LES_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	MOV((void *)&vcpu.es.selector,(void *)(vcpuins.rrm+2),16);
}
static void LDS_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	MOV((void *)&vcpu.ds.selector,(void *)(vcpuins.rrm+2),16);
}
static void INS_C6()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch (vcpuins.cr) {
	case 0: /* MOV_RM8_I8 */
		MOV((void *)vcpuins.rrm,(void *)rimm,8);
		break;
	default: CaseError("INS_C6::cr");break;}
}
static void INS_C7()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch (vcpuins.cr) {
	case 0: /* MOV_RM16_I16 */
		MOV((void *)vcpuins.rrm,(void *)rimm,16);
		break;
	default: CaseError("INS_C7::cr");break;}
}
static void RETF_I16()
{
	t_nubit16 addsp;
	vcpu.ip++;
	GetImm(16);
	addsp = d_nubit16(rimm);
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs.selector,16);
	vcpu.sp += addsp;
}
static void RETF()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs.selector,16);
}
static void INT3()
{
	vcpu.ip++;
	INT(0x03);
}
static void INT_I8()
{
	vcpu.ip++;
	GetImm(8);
	INT(d_nubit8(rimm));
}
static void INTO()
{
	vcpu.ip++;
	if(GetBit(vcpu.flags, VCPU_EFLAGS_OF)) INT(0x04);
}
static void IRET()
{
	vcpu.ip++;
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs.selector,16);
	POP((void *)&vcpu.flags,16);
}
static void INS_D0()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,NULL,8);break;
	case 1:	ROR((void *)vcpuins.rrm,NULL,8);break;
	case 2:	RCL((void *)vcpuins.rrm,NULL,8);break;
	case 3:	RCR((void *)vcpuins.rrm,NULL,8);break;
	case 4:	SHL((void *)vcpuins.rrm,NULL,8);break;
	case 5:	SHR((void *)vcpuins.rrm,NULL,8);break;
	case 6:	SAL((void *)vcpuins.rrm,NULL,8);break;
	case 7:	SAR((void *)vcpuins.rrm,NULL,8);break;
	default:CaseError("INS_D0::vcpuins.rr");break;}
}
static void INS_D1()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,NULL,16);break;
	case 1:	ROR((void *)vcpuins.rrm,NULL,16);break;
	case 2:	RCL((void *)vcpuins.rrm,NULL,16);break;
	case 3:	RCR((void *)vcpuins.rrm,NULL,16);break;
	case 4:	SHL((void *)vcpuins.rrm,NULL,16);break;
	case 5:	SHR((void *)vcpuins.rrm,NULL,16);break;
	case 6:	SAL((void *)vcpuins.rrm,NULL,16);break;
	case 7:	SAR((void *)vcpuins.rrm,NULL,16);break;
	default:CaseError("INS_D1::vcpuins.rr");break;}
}
static void INS_D2()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	default:CaseError("INS_D2::vcpuins.rr");break;}
}
static void INS_D3()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch (vcpuins.cr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	default:CaseError("INS_D3::vcpuins.rr");break;}
}
static void AAM()
{
	t_nubit8 base,tempAL;
	vcpu.ip++;
	GetImm(8);
	base = d_nubit8(rimm);
	if(base == 0) INT(0x00);
	else {
		tempAL = vcpu.al;
		vcpu.ah = tempAL / base;
		vcpu.al = tempAL % base;
		vcpuins.bit = 0x08;
		vcpuins.result = vcpu.al & 0xff;
		SetFlags(AAM_FLAG);
	}
}
static void AAD()
{
	t_nubit8 base,tempAL,tempAH;
	vcpu.ip++;
	GetImm(8);
	base = d_nubit8(rimm);
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
}
static void XLAT()
{
	vcpu.ip++;
	vcpu.al = vramRealByte(vcpuins.roverds->selector,vcpu.bx+vcpu.al);
}
static void LOOPNZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(rimm);
	vcpu.cx--;
	if(vcpu.cx && !GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) vcpu.ip += rel8;
}
static void LOOPZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(rimm);
	vcpu.cx--;
	if(vcpu.cx && GetBit(vcpu.flags, VCPU_EFLAGS_ZF)) vcpu.ip += rel8;
}
static void LOOP()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(rimm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.ip += rel8;
}
static void JCXZ_REL8()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void*)rimm,!vcpu.cx,8);
}
static void IN_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(rimm)]);
	vcpu.al = vport.iobyte;
}
static void IN_AX_I8()
{
	vcpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(rimm)]);
	vcpu.ax = vport.ioword;
}
static void OUT_I8_AL()
{
	vcpu.ip++;
	GetImm(8);
	vport.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(rimm)]);
}
static void OUT_I8_AX()
{
	vcpu.ip++;
	GetImm(8);
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[d_nubit8(rimm)]);
}
static void CALL_REL16()
{
	t_nsbit16 rel16;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(rimm);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip += rel16;
}
static void JMP_REL16()
{
	vcpu.ip++;
	GetImm(16);
	vcpu.ip += d_nsbit16(rimm);
}
static void JMP_PTR16_16()
{
	t_nubit16 newip,newcs;
	vcpu.ip++;
	GetImm(16);
	newip = d_nubit16(rimm);
	GetImm(16);
	newcs = d_nubit16(rimm);
	vcpu.ip = newip;
	vcpu.cs.selector = newcs;
}
static void JMP_REL8()
{
	vcpu.ip++;
	GetImm(8);
	vcpu.ip += d_nsbit8(rimm);
}
static void IN_AL_DX()
{
	vcpu.ip++;
	ExecFun(vport.in[vcpu.dx]);
	vcpu.al = vport.iobyte;
}
static void IN_AX_DX()
{
	vcpu.ip++;
	ExecFun(vport.in[vcpu.dx]);
	vcpu.ax = vport.ioword;
}
static void OUT_DX_AL()
{
	vcpu.ip++;
	vport.iobyte = vcpu.al;
	ExecFun(vport.out[vcpu.dx]);
}
static void OUT_DX_AX()
{
	vcpu.ip++;
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[vcpu.dx]);
}
todo LOCK()
{
	vcpu.ip++;
	/* Not Implemented */
}
static void REPNZ()
{
	// CMPS,SCAS
	vcpu.ip++;
	vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
}
static void REP()
{	// MOVS,LODS,STOS,CMPS,SCAS
	vcpu.ip++;
	vcpuins.prefix_rep = PREFIX_REP_REPZ;
}
todo HLT()
{
	vcpu.ip++;
	/* Not Implemented */
}
static void CMC()
{
	vcpu.ip++;
	vcpu.flags ^= VCPU_EFLAGS_CF;
}
static void INS_F6()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch (vcpuins.cr) {
	case 0:	GetImm(8);
			TEST((void *)vcpuins.rrm,(void *)rimm,8);
			break;
	case 2:	NOT ((void *)vcpuins.rrm,8);	break;
	case 3:	NEG ((void *)vcpuins.rrm,8);	break;
	case 4:	MUL ((void *)vcpuins.rrm,8);	break;
	case 5:	IMUL((void *)vcpuins.rrm,8);	break;
	case 6:	DIV ((void *)vcpuins.rrm,8);	break;
	case 7:	IDIV((void *)vcpuins.rrm,8);	break;
	default:CaseError("INS_F6::vcpuins.rr");break;}
}
static void INS_F7()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch (vcpuins.cr) {
	case 0:	GetImm(16);
			TEST((void *)vcpuins.rrm,(void *)rimm,16);
			break;
	case 2:	NOT ((void *)vcpuins.rrm,16);	break;
	case 3:	NEG ((void *)vcpuins.rrm,16);	break;
	case 4:	MUL ((void *)vcpuins.rrm,16);	break;
	case 5:	IMUL((void *)vcpuins.rrm,16);	break;
	case 6:	DIV ((void *)vcpuins.rrm,16);	break;
	case 7:	IDIV((void *)vcpuins.rrm,16);	break;
	default:CaseError("INS_F7::vcpuins.rr");break;}
}
static void CLC()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_EFLAGS_CF);
}
static void STC()
{
	vcpu.ip++;
	SetBit(vcpu.flags, VCPU_EFLAGS_CF);
}
static void CLI()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_EFLAGS_IF);
}
static void STI()
{
	vcpu.ip++;
	SetBit(vcpu.flags, VCPU_EFLAGS_IF);
}
static void CLD()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_EFLAGS_DF);
}
static void STD()
{
	vcpu.ip++;
	SetBit(vcpu.flags,VCPU_EFLAGS_DF);
}
static void INS_FE()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch (vcpuins.cr) {
	case 0:	INC((void *)vcpuins.rrm,8);	break;
	case 1:	DEC((void *)vcpuins.rrm,8);	break;
	default:CaseError("INS_FE::vcpuins.rr");break;}
}
static void INS_FF()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch (vcpuins.cr) {
	case 0:	INC((void *)vcpuins.rrm,16);	break;
	case 1:	DEC((void *)vcpuins.rrm,16);	break;
	case 2:	/* CALL_RM16 */
		PUSH((void *)&vcpu.ip,16);
		vcpu.ip = d_nubit16(vcpuins.rrm);
		break;
	case 3:	/* CALL_M16_16 */
		PUSH((void *)&vcpu.cs.selector,16);
		PUSH((void *)&vcpu.ip,16);
		vcpu.ip = d_nubit16(vcpuins.rrm);
		vcpu.cs.selector = d_nubit16(vcpuins.rrm + 2);
		break;
	case 4:	/* JMP_RM16 */
		vcpu.ip = d_nubit16(vcpuins.rrm);
		break;
	case 5:	/* JMP_M16_16 */
		vcpu.ip = d_nubit16(vcpuins.rrm);
		vcpu.cs.selector = d_nubit16(vcpuins.rrm + 2);
		break;
	case 6:	/* PUSH_RM16 */
		PUSH((void *)vcpuins.rrm,16);
		break;
	default:CaseError("INS_FF::vcpuins.rr");break;}
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
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ss;
	vcpuins.prefix_rep = PREFIX_REP_NONE;
}
static void ExecIns()
{
	t_nubit8 opcode;
	vcpurec.rcpu = vcpu;
	vcpurec.linear = (vcpu.cs.selector << 4) + vcpu.ip;
	if (vcpuinsReadLinear(vcpurec.linear, (t_vaddrcc)vcpurec.opcodes, 15))
		vcpurec.oplen = 0;
	else
		vcpurec.oplen = 15;
	ClrPrefix();
	do {
		opcode = vramRealByte(vcpu.cs.selector, vcpu.ip);
		ExecFun(table[opcode]);
	} while (IsPrefix(opcode));
}
static void ExecInt()
{
	/* hardware interrupt handeler */
	if(vcpu.flagnmi) INT(0x02);
	vcpu.flagnmi = 0x00;

	if(GetBit(vcpu.flags, VCPU_EFLAGS_IF) && vpicScanINTR())
		INT(vpicGetINTR());

	if(GetBit(vcpu.flags, VCPU_EFLAGS_TF)) INT(0x01);
}

static void QDX()
{
	vcpu.ip++;
	GetImm(8);
	switch (d_nubit8(rimm)) {
	case 0x00:
	case 0xff:
		vapiPrint("\nNXVM STOP at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.ip,d_nubit8(rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe:
		vapiPrint("\nNXVM RESET at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.ip,d_nubit8(rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	default:
		qdbiosExecInt(d_nubit8(rimm));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_ZF, GetBit(_flags, VCPU_EFLAGS_ZF));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_CF, GetBit(_flags, VCPU_EFLAGS_CF));
		break;
	}
}

/* external interface */
t_bool vcpuinsLoadSreg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	rsreg->selector = selector;
	rsreg->base = (selector << 4);
	return 0;
}
t_bool vcpuinsReadLinear(t_nubit32 linear, t_vaddrcc rcode, t_nubit8 byte)
{
	t_nubit8 i;
	t_nubit32 physical = linear;
	for (i = 0;i < byte;++i)
		d_nubit8(rcode + i) = vramByte(physical + i);
	return 0;
}
t_bool vcpuinsWriteLinear(t_nubit32 linear, t_vaddrcc rcode, t_nubit8 byte)
{
	t_nubit8 i;
	t_nubit32 physical = linear;
	for (i = 0;i < byte;++i)
		vramByte(physical + i) = d_nubit8(rcode + i);
	return 0;
}
void vcpuinsInit()
{
	memset(&vcpuins, 0x00, sizeof(t_cpuins));
	table[0x00] = (t_faddrcc)ADD_RM8_R8;
	table[0x01] = (t_faddrcc)ADD_RM16_R16;
	table[0x02] = (t_faddrcc)ADD_R8_RM8;
	table[0x03] = (t_faddrcc)ADD_R16_RM16;
	table[0x04] = (t_faddrcc)ADD_AL_I8;
	table[0x05] = (t_faddrcc)ADD_AX_I16;
	table[0x06] = (t_faddrcc)PUSH_ES;
	table[0x07] = (t_faddrcc)POP_ES;
	table[0x08] = (t_faddrcc)OR_RM8_R8;
	table[0x09] = (t_faddrcc)OR_RM16_R16;
	table[0x0a] = (t_faddrcc)OR_R8_RM8;
	table[0x0b] = (t_faddrcc)OR_R16_RM16;
	table[0x0c] = (t_faddrcc)OR_AL_I8;
	table[0x0d] = (t_faddrcc)OR_AX_I16;
	table[0x0e] = (t_faddrcc)PUSH_CS;
	table[0x0f] = (t_faddrcc)POP_CS;
	//table[0x0f] = (t_faddrcc)INS_0F;
	table[0x10] = (t_faddrcc)ADC_RM8_R8;
	table[0x11] = (t_faddrcc)ADC_RM16_R16;
	table[0x12] = (t_faddrcc)ADC_R8_RM8;
	table[0x13] = (t_faddrcc)ADC_R16_RM16;
	table[0x14] = (t_faddrcc)ADC_AL_I8;
	table[0x15] = (t_faddrcc)ADC_AX_I16;
	table[0x16] = (t_faddrcc)PUSH_SS;
	table[0x17] = (t_faddrcc)POP_SS;
	table[0x18] = (t_faddrcc)SBB_RM8_R8;
	table[0x19] = (t_faddrcc)SBB_RM16_R16;
	table[0x1a] = (t_faddrcc)SBB_R8_RM8;
	table[0x1b] = (t_faddrcc)SBB_R16_RM16;
	table[0x1c] = (t_faddrcc)SBB_AL_I8;
	table[0x1d] = (t_faddrcc)SBB_AX_I16;
	table[0x1e] = (t_faddrcc)PUSH_DS;
	table[0x1f] = (t_faddrcc)POP_DS;
	table[0x20] = (t_faddrcc)AND_RM8_R8;
	table[0x21] = (t_faddrcc)AND_RM16_R16;
	table[0x22] = (t_faddrcc)AND_R8_RM8;
	table[0x23] = (t_faddrcc)AND_R16_RM16;
	table[0x24] = (t_faddrcc)AND_AL_I8;
	table[0x25] = (t_faddrcc)AND_AX_I16;
	table[0x26] = (t_faddrcc)ES;
	table[0x27] = (t_faddrcc)DAA;
	table[0x28] = (t_faddrcc)SUB_RM8_R8;
	table[0x29] = (t_faddrcc)SUB_RM16_R16;
	table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	table[0x2b] = (t_faddrcc)SUB_R16_RM16;
	table[0x2c] = (t_faddrcc)SUB_AL_I8;
	table[0x2d] = (t_faddrcc)SUB_AX_I16;
	table[0x2e] = (t_faddrcc)CS;
	table[0x2f] = (t_faddrcc)DAS;
	table[0x30] = (t_faddrcc)XOR_RM8_R8;
	table[0x31] = (t_faddrcc)XOR_RM16_R16;
	table[0x32] = (t_faddrcc)XOR_R8_RM8;
	table[0x33] = (t_faddrcc)XOR_R16_RM16;
	table[0x34] = (t_faddrcc)XOR_AL_I8;
	table[0x35] = (t_faddrcc)XOR_AX_I16;
	table[0x36] = (t_faddrcc)SS;
	table[0x37] = (t_faddrcc)AAA;
	table[0x38] = (t_faddrcc)CMP_RM8_R8;
	table[0x39] = (t_faddrcc)CMP_RM16_R16;
	table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	table[0x3b] = (t_faddrcc)CMP_R16_RM16;
	table[0x3c] = (t_faddrcc)CMP_AL_I8;
	table[0x3d] = (t_faddrcc)CMP_AX_I16;
	table[0x3e] = (t_faddrcc)DS;
	table[0x3f] = (t_faddrcc)AAS;
	table[0x40] = (t_faddrcc)INC_AX;
	table[0x41] = (t_faddrcc)INC_CX;
	table[0x42] = (t_faddrcc)INC_DX;
	table[0x43] = (t_faddrcc)INC_BX;
	table[0x44] = (t_faddrcc)INC_SP;
	table[0x45] = (t_faddrcc)INC_BP;
	table[0x46] = (t_faddrcc)INC_SI;
	table[0x47] = (t_faddrcc)INC_DI;
	table[0x48] = (t_faddrcc)DEC_AX;
	table[0x49] = (t_faddrcc)DEC_CX;
	table[0x4a] = (t_faddrcc)DEC_DX;
	table[0x4b] = (t_faddrcc)DEC_BX;
	table[0x4c] = (t_faddrcc)DEC_SP;
	table[0x4d] = (t_faddrcc)DEC_BP;
	table[0x4e] = (t_faddrcc)DEC_SI;
	table[0x4f] = (t_faddrcc)DEC_DI;
	table[0x50] = (t_faddrcc)PUSH_AX;
	table[0x51] = (t_faddrcc)PUSH_CX;
	table[0x52] = (t_faddrcc)PUSH_DX;
	table[0x53] = (t_faddrcc)PUSH_BX;
	table[0x54] = (t_faddrcc)PUSH_SP;
	table[0x55] = (t_faddrcc)PUSH_BP;
	table[0x56] = (t_faddrcc)PUSH_SI;
	table[0x57] = (t_faddrcc)PUSH_DI;
	table[0x58] = (t_faddrcc)POP_AX;
	table[0x59] = (t_faddrcc)POP_CX;
	table[0x5a] = (t_faddrcc)POP_DX;
	table[0x5b] = (t_faddrcc)POP_BX;
	table[0x5c] = (t_faddrcc)POP_SP;
	table[0x5d] = (t_faddrcc)POP_BP;
	table[0x5e] = (t_faddrcc)POP_SI;
	table[0x5f] = (t_faddrcc)POP_DI;
	table[0x60] = (t_faddrcc)UndefinedOpcode;
	table[0x61] = (t_faddrcc)UndefinedOpcode;
	table[0x62] = (t_faddrcc)UndefinedOpcode;
	table[0x63] = (t_faddrcc)UndefinedOpcode;
	table[0x64] = (t_faddrcc)UndefinedOpcode;
	table[0x65] = (t_faddrcc)UndefinedOpcode;
	table[0x66] = (t_faddrcc)UndefinedOpcode;
	table[0x67] = (t_faddrcc)UndefinedOpcode;
	table[0x68] = (t_faddrcc)UndefinedOpcode;
	table[0x69] = (t_faddrcc)UndefinedOpcode;
	table[0x6a] = (t_faddrcc)UndefinedOpcode;
	table[0x6b] = (t_faddrcc)UndefinedOpcode;
	table[0x6c] = (t_faddrcc)UndefinedOpcode;
	table[0x6d] = (t_faddrcc)UndefinedOpcode;
	table[0x6e] = (t_faddrcc)UndefinedOpcode;
	table[0x6f] = (t_faddrcc)UndefinedOpcode;
	table[0x70] = (t_faddrcc)JO;
	table[0x71] = (t_faddrcc)JNO;
	table[0x72] = (t_faddrcc)JC;
	table[0x73] = (t_faddrcc)JNC;
	table[0x74] = (t_faddrcc)JZ;
	table[0x75] = (t_faddrcc)JNZ;
	table[0x76] = (t_faddrcc)JBE;
	table[0x77] = (t_faddrcc)JA;
	table[0x78] = (t_faddrcc)JS;
	table[0x79] = (t_faddrcc)JNS;
	table[0x7a] = (t_faddrcc)JP;
	table[0x7b] = (t_faddrcc)JNP;
	table[0x7c] = (t_faddrcc)JL;
	table[0x7d] = (t_faddrcc)JNL;
	table[0x7e] = (t_faddrcc)JLE;
	table[0x7f] = (t_faddrcc)JG;
	table[0x80] = (t_faddrcc)INS_80;
	table[0x81] = (t_faddrcc)INS_81;
	table[0x82] = (t_faddrcc)INS_82;
	table[0x83] = (t_faddrcc)INS_83;
	table[0x84] = (t_faddrcc)TEST_RM8_R8;
	table[0x85] = (t_faddrcc)TEST_RM16_R16;
	table[0x86] = (t_faddrcc)XCHG_R8_RM8;
	table[0x87] = (t_faddrcc)XCHG_R16_RM16;
	table[0x88] = (t_faddrcc)MOV_RM8_R8;
	table[0x89] = (t_faddrcc)MOV_RM16_R16;
	table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	table[0x8b] = (t_faddrcc)MOV_R16_RM16;
	table[0x8c] = (t_faddrcc)MOV_RM16_SEG;
	table[0x8d] = (t_faddrcc)LEA_R16_M16;
	table[0x8e] = (t_faddrcc)MOV_SEG_RM16;
	table[0x8f] = (t_faddrcc)POP_RM16;
	table[0x90] = (t_faddrcc)NOP;
	table[0x91] = (t_faddrcc)XCHG_CX_AX;
	table[0x92] = (t_faddrcc)XCHG_DX_AX;
	table[0x93] = (t_faddrcc)XCHG_BX_AX;
	table[0x94] = (t_faddrcc)XCHG_SP_AX;
	table[0x95] = (t_faddrcc)XCHG_BP_AX;
	table[0x96] = (t_faddrcc)XCHG_SI_AX;
	table[0x97] = (t_faddrcc)XCHG_DI_AX;
	table[0x98] = (t_faddrcc)CBW;
	table[0x99] = (t_faddrcc)CWD;
	table[0x9a] = (t_faddrcc)CALL_PTR16_16;
	table[0x9b] = (t_faddrcc)WAIT;
	table[0x9c] = (t_faddrcc)PUSHF;
	table[0x9d] = (t_faddrcc)POPF;
	table[0x9e] = (t_faddrcc)SAHF;
	table[0x9f] = (t_faddrcc)LAHF;
	table[0xa0] = (t_faddrcc)MOV_AL_M8;
	table[0xa1] = (t_faddrcc)MOV_AX_M16;
	table[0xa2] = (t_faddrcc)MOV_M8_AL;
	table[0xa3] = (t_faddrcc)MOV_M16_AX;
	table[0xa4] = (t_faddrcc)MOVSB;
	table[0xa5] = (t_faddrcc)MOVSW;
	table[0xa6] = (t_faddrcc)CMPSB;
	table[0xa7] = (t_faddrcc)CMPSW;
	table[0xa8] = (t_faddrcc)TEST_AL_I8;
	table[0xa9] = (t_faddrcc)TEST_AX_I16;
	table[0xaa] = (t_faddrcc)STOSB;
	table[0xab] = (t_faddrcc)STOSW;
	table[0xac] = (t_faddrcc)LODSB;
	table[0xad] = (t_faddrcc)LODSW;
	table[0xae] = (t_faddrcc)SCASB;
	table[0xaf] = (t_faddrcc)SCASW;
	table[0xb0] = (t_faddrcc)MOV_AL_I8;
	table[0xb1] = (t_faddrcc)MOV_CL_I8;
	table[0xb2] = (t_faddrcc)MOV_DL_I8;
	table[0xb3] = (t_faddrcc)MOV_BL_I8;
	table[0xb4] = (t_faddrcc)MOV_AH_I8;
	table[0xb5] = (t_faddrcc)MOV_CH_I8;
	table[0xb6] = (t_faddrcc)MOV_DH_I8;
	table[0xb7] = (t_faddrcc)MOV_BH_I8;
	table[0xb8] = (t_faddrcc)MOV_AX_I16;
	table[0xb9] = (t_faddrcc)MOV_CX_I16;
	table[0xba] = (t_faddrcc)MOV_DX_I16;
	table[0xbb] = (t_faddrcc)MOV_BX_I16;
	table[0xbc] = (t_faddrcc)MOV_SP_I16;
	table[0xbd] = (t_faddrcc)MOV_BP_I16;
	table[0xbe] = (t_faddrcc)MOV_SI_I16;
	table[0xbf] = (t_faddrcc)MOV_DI_I16;
	table[0xc0] = (t_faddrcc)UndefinedOpcode;
	table[0xc1] = (t_faddrcc)UndefinedOpcode;
	table[0xc2] = (t_faddrcc)RET_I16;
	table[0xc3] = (t_faddrcc)RET;
	table[0xc4] = (t_faddrcc)LES_R16_M16;
	table[0xc5] = (t_faddrcc)LDS_R16_M16;
	table[0xc6] = (t_faddrcc)INS_C6;
	table[0xc7] = (t_faddrcc)INS_C7;
	table[0xc8] = (t_faddrcc)UndefinedOpcode;
	table[0xc9] = (t_faddrcc)UndefinedOpcode;
	table[0xca] = (t_faddrcc)RETF_I16;
	table[0xcb] = (t_faddrcc)RETF;
	table[0xcc] = (t_faddrcc)INT3;
	table[0xcd] = (t_faddrcc)INT_I8;
	table[0xce] = (t_faddrcc)INTO;
	table[0xcf] = (t_faddrcc)IRET;
	table[0xd0] = (t_faddrcc)INS_D0;
	table[0xd1] = (t_faddrcc)INS_D1;
	table[0xd2] = (t_faddrcc)INS_D2;
	table[0xd3] = (t_faddrcc)INS_D3;
	table[0xd4] = (t_faddrcc)AAM;
	table[0xd5] = (t_faddrcc)AAD;
	table[0xd6] = (t_faddrcc)UndefinedOpcode;
	table[0xd7] = (t_faddrcc)XLAT;
	table[0xd8] = (t_faddrcc)UndefinedOpcode;
	table[0xd9] = (t_faddrcc)UndefinedOpcode;
	//table[0xd9] = (t_faddrcc)INS_D9;
	table[0xda] = (t_faddrcc)UndefinedOpcode;
	table[0xdb] = (t_faddrcc)UndefinedOpcode;
	//table[0xdb] = (t_faddrcc)INS_DB;
	table[0xdc] = (t_faddrcc)UndefinedOpcode;
	table[0xdd] = (t_faddrcc)UndefinedOpcode;
	table[0xde] = (t_faddrcc)UndefinedOpcode;
	table[0xdf] = (t_faddrcc)UndefinedOpcode;
	table[0xe0] = (t_faddrcc)LOOPNZ;
	table[0xe1] = (t_faddrcc)LOOPZ;
	table[0xe2] = (t_faddrcc)LOOP;
	table[0xe3] = (t_faddrcc)JCXZ_REL8;
	table[0xe4] = (t_faddrcc)IN_AL_I8;
	table[0xe5] = (t_faddrcc)IN_AX_I8;
	table[0xe6] = (t_faddrcc)OUT_I8_AL;
	table[0xe7] = (t_faddrcc)OUT_I8_AX;
	table[0xe8] = (t_faddrcc)CALL_REL16;
	table[0xe9] = (t_faddrcc)JMP_REL16;
	table[0xea] = (t_faddrcc)JMP_PTR16_16;
	table[0xeb] = (t_faddrcc)JMP_REL8;
	table[0xec] = (t_faddrcc)IN_AL_DX;
	table[0xed] = (t_faddrcc)IN_AX_DX;
	table[0xee] = (t_faddrcc)OUT_DX_AL;
	table[0xef] = (t_faddrcc)OUT_DX_AX;
	table[0xf0] = (t_faddrcc)LOCK;
	table[0xf1] = (t_faddrcc)QDX;
	table[0xf2] = (t_faddrcc)REPNZ;
	table[0xf3] = (t_faddrcc)REP;
	table[0xf4] = (t_faddrcc)HLT;
	table[0xf5] = (t_faddrcc)CMC;
	table[0xf6] = (t_faddrcc)INS_F6;
	table[0xf7] = (t_faddrcc)INS_F7;
	table[0xf8] = (t_faddrcc)CLC;
	table[0xf9] = (t_faddrcc)STC;
	table[0xfa] = (t_faddrcc)CLI;
	table[0xfb] = (t_faddrcc)STI;
	table[0xfc] = (t_faddrcc)CLD;
	table[0xfd] = (t_faddrcc)STD;
	table[0xfe] = (t_faddrcc)INS_FE;
	table[0xff] = (t_faddrcc)INS_FF;
}
void vcpuinsReset()
{
	vcpuins.rrm = vcpuins.rr = rimm = (t_vaddrcc)NULL;
	vcpuins.opr1 = vcpuins.opr2 = vcpuins.result = vcpuins.bit = 0;
	ClrPrefix();
	vcpurec.svcextl = 0;
}
void vcpuinsRefresh()
{
	ExecIns();
	ExecInt();
}
void vcpuinsFinal() {}
