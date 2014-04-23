/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"

#include "vapi.h"

#include "vmachine.h"
#include "vcpuins.h"

#ifdef VCPUINS_DEBUG
#include "qdbios.h"
#include "qdvga.h"
/* TODO: INT() is modified for the INT test. Please correct it finally! */
#endif

#define MOD ((modrm&0xc0)>>6)
#define REG ((modrm&0x38)>>3)
#define RM  ((modrm&0x07)>>0)

#define ADD_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_CF | VCPU_FLAG_PF)
#define	 OR_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define ADC_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_CF | VCPU_FLAG_PF)
#define SBB_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_CF | VCPU_FLAG_PF)
#define AND_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define SUB_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_CF | VCPU_FLAG_PF)
#define XOR_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define CMP_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_CF | VCPU_FLAG_PF)
#define INC_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_PF)
#define DEC_FLAG  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_PF)
#define TEST_FLAG (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define SHL_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define SHR_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define SAL_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define SAR_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define AAM_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)
#define AAD_FLAG  (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF)

t_cpuins vcpuins;

static void CaseError(const char *str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: %s.\n",str);
	vmachineStop();
}
static void CalcCF()
{
	switch(vcpuins.type) {
	case ADD8:
	case ADD16:
		MakeBit(vcpu.flags,VCPU_FLAG_CF,(vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2));
		break;
	case ADC8:
	case ADC16:
/* vcpuins bug fix #1
		MakeBit(vcpu.flags,VCPU_FLAG_CF,vcpuins.result <= vcpuins.opr1);*/
		MakeBit(vcpu.flags,VCPU_FLAG_CF,vcpuins.result < vcpuins.opr1);
		break;
	case SBB8:
		MakeBit(vcpu.flags,VCPU_FLAG_CF,(vcpuins.opr1 < vcpuins.result) || (vcpuins.opr2 == 0xff));
		break;
	case SBB16:
		MakeBit(vcpu.flags,VCPU_FLAG_CF,(vcpuins.opr1 < vcpuins.result) || (vcpuins.opr2 == 0xffff));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		MakeBit(vcpu.flags,VCPU_FLAG_CF,vcpuins.opr1 < vcpuins.opr2);
		break;
	default:CaseError("CalcCF::vcpuins.type");break;}
}
static void CalcOF()
{
	switch(vcpuins.type) {
	case ADD8:
	case ADC8:
		MakeBit(vcpu.flags,VCPU_FLAG_OF,((vcpuins.opr1&0x0080) == (vcpuins.opr2&0x0080)) && ((vcpuins.opr1&0x0080) != (vcpuins.result&0x0080)));
		break;
	case ADD16:
	case ADC16:
		MakeBit(vcpu.flags,VCPU_FLAG_OF,((vcpuins.opr1&0x8000) == (vcpuins.opr2&0x8000)) && ((vcpuins.opr1&0x8000) != (vcpuins.result&0x8000)));
		break;
	case SBB8:
	case SUB8:
	case CMP8:
		MakeBit(vcpu.flags,VCPU_FLAG_OF,((vcpuins.opr1&0x0080) != (vcpuins.opr2&0x0080)) && ((vcpuins.opr2&0x0080) == (vcpuins.result&0x0080)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		MakeBit(vcpu.flags,VCPU_FLAG_OF,((vcpuins.opr1&0x8000) != (vcpuins.opr2&0x8000)) && ((vcpuins.opr2&0x8000) == (vcpuins.result&0x8000)));
		break;
	default:CaseError("CalcOF::vcpuins.type");break;}
}
static void CalcAF()
{
	MakeBit(vcpu.flags,VCPU_FLAG_AF,((vcpuins.opr1^vcpuins.opr2)^vcpuins.result)&0x10);
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
	MakeBit(vcpu.flags,VCPU_FLAG_PF,!(count&0x01));
}
static void CalcZF()
{
	MakeBit(vcpu.flags,VCPU_FLAG_ZF,!vcpuins.result);
}
static void CalcSF()
{
	switch(vcpuins.bit) {
	case 8:	MakeBit(vcpu.flags,VCPU_FLAG_SF,!!(vcpuins.result&0x80));break;
	case 16:MakeBit(vcpu.flags,VCPU_FLAG_SF,!!(vcpuins.result&0x8000));break;
	default:CaseError("CalcSF::vcpuins.bit");break;}
}
static void CalcTF() {}
static void CalcIF() {}
static void CalcDF() {}

static void SetFlags(t_nubit16 flags)
{
	if(flags & VCPU_FLAG_CF) CalcCF();
	if(flags & VCPU_FLAG_PF) CalcPF();
	if(flags & VCPU_FLAG_AF) CalcAF();
	if(flags & VCPU_FLAG_ZF) CalcZF();
	if(flags & VCPU_FLAG_SF) CalcSF();
	if(flags & VCPU_FLAG_TF) CalcTF();
	if(flags & VCPU_FLAG_IF) CalcIF();
	if(flags & VCPU_FLAG_DF) CalcDF();
	if(flags & VCPU_FLAG_OF) CalcOF();
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
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overds,vramVarWord(vcpu.cs,vcpu.ip));vcpu.ip += 2;break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::RM");break;}
		break;
	case 1:
		switch(RM) {
		case 0:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.si);break;
		case 5:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.di);break;
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp);break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD1::RM");break;}
/* vcpuins bug fix #3
		vcpuins.rm += vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;*/
		vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;
		break;
	case 2:
		switch(RM) {
		case 0:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.si);break;
		case 5:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.di);break;
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp);break;
		case 7:	vcpuins.rm = vramGetAddr(vcpu.overds,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD2::RM");break;}
		vcpuins.rm += vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;
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
	// returns vcpuins.rm and vcpuins.r
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
		switch(RM) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rm = vcpu.si;break;
		case 5:	vcpuins.rm = vcpu.di;break;
		case 6:	vcpuins.rm = vcpu.bp;break;
		case 7:	vcpuins.rm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD1::RM");break;}
/* vcpuins bug fix #p
		vcpuins.rm += vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;*/
		vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;
		break;
	case 2:
		switch(RM) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rm = vcpu.si;break;
		case 5:	vcpuins.rm = vcpu.di;break;
/* vcpuins bug fix #14
		case 6:	vcpuins.rm = vramGetAddr(vcpu.overss,vcpu.bp);break;*/
		case 6:	vcpuins.rm = vcpu.bp;break;
		case 7:	vcpuins.rm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD2::RM");break;}
		vcpuins.rm += vramVarWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;
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
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
/* vcpuins bug fix #6 */
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("OR::len");break;}
	ClrBit(vcpu.flags, VCPU_FLAG_OF);
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
	ClrBit(vcpu.flags, VCPU_FLAG_AF);
	SetFlags(OR_FLAG);
}
static void ADC(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF))&0xffff;
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)))&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)))&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+GetBit(vcpu.flags, VCPU_FLAG_CF)))&0xffff;
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("AND::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(AND_FLAG);
}
static void SUB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nsbit8(src);
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("XOR::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(XOR_FLAG);
}
static void CMP(void *op1, void *op2, t_nubit8 len)
{
/* vcpuins debug: TODO: still need to check */
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = d_nubit8(op1) & 0xff;
		vcpuins.opr2 = d_nsbit8(op2) & 0xff;
/* vcpuins bug fix #7
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;*//*s->u*/
		vcpuins.result = ((t_nubit8)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xff;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(op1) & 0xffff;
		vcpuins.opr2 = d_nsbit8(op2) & 0xffff;
		vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xffff;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(op1) & 0xffff;
		vcpuins.opr2 = d_nsbit16(op2) & 0xffff;
/* vcpuins bug fix #7
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;*/
		vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit16)vcpuins.opr2)&0xffff;
		break;
	default:CaseError("CMP::len");break;}
	SetFlags(CMP_FLAG);
}
static void PUSH(void *src, t_nubit8 len)
{
/* vcpuins bug fix #13 */
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		vcpu.sp -= 2;
//		vapiPrint("V: PUSH %04X to %08X\n",data,((_ss<<4)+_sp));
		vramVarWord(vcpu.ss,vcpu.sp) = data;
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
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest);
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest);
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
/* vcpuins bug fix #5
			vcpu.ip += d_nubit8(src);*/
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
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xff;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = TEST16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xffff;
		break;
	default:CaseError("TEST::len");break;}
	ClrBit(vcpu.flags,VCPU_FLAG_OF);
	ClrBit(vcpu.flags,VCPU_FLAG_CF);
	ClrBit(vcpu.flags,VCPU_FLAG_AF);
	SetFlags(TEST_FLAG);
}
static void XCHG(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.opr1 = d_nubit8(dest);
		vcpuins.opr2 = d_nubit8(src);
		d_nubit8(dest) = (t_nubit8)vcpuins.opr2;
		d_nubit8(src) = (t_nubit8)vcpuins.opr1;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.opr1 = d_nubit16(dest);
		vcpuins.opr2 = d_nubit16(src);
		d_nubit16(dest) = (t_nubit16)vcpuins.opr2;
		d_nubit16(src) = (t_nubit16)vcpuins.opr1;
		break;
	default:CaseError("XCHG::len");break;}
}
static void MOV(void *dest, void *src, t_nubit8 len)
{
/*	t_vaddrcc pd  = (t_vaddrcc)(dest);
	t_vaddrcc ps  = (t_vaddrcc)(src);
	t_vaddrcc pal = (t_vaddrcc)(&vcpu.al);
	t_vaddrcc pah = (t_vaddrcc)(&vcpu.ah);
	t_vaddrcc pbl = (t_vaddrcc)(&vcpu.bl);
	t_vaddrcc pbh = (t_vaddrcc)(&vcpu.bh);
	t_vaddrcc pcl = (t_vaddrcc)(&vcpu.cl);
	t_vaddrcc pch = (t_vaddrcc)(&vcpu.ch);
	t_vaddrcc pdl = (t_vaddrcc)(&vcpu.dl);
	t_vaddrcc pdh = (t_vaddrcc)(&vcpu.dh);
	t_vaddrcc pcs = (t_vaddrcc)(&vcpu.cs);
	t_vaddrcc pds = (t_vaddrcc)(&vcpu.ds);
	t_vaddrcc pes = (t_vaddrcc)(&vcpu.es);
	t_vaddrcc pss = (t_vaddrcc)(&vcpu.ss);
	t_vaddrcc psp = (t_vaddrcc)(&vcpu.sp);
	t_vaddrcc pbp = (t_vaddrcc)(&vcpu.bp);
	t_vaddrcc psi = (t_vaddrcc)(&vcpu.si);
	t_vaddrcc pdi = (t_vaddrcc)(&vcpu.di);*/
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
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		break;
	case 16:
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
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
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^(!!(d_nubit8(dest)&0x40)));
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
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^(!!(d_nubit16(dest)&0x4000)));
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
			d_nubit8(dest) = (d_nubit8(dest)<<1)+GetBit(vcpu.flags, VCPU_FLAG_CF);
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		break;
	case 16:
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+GetBit(vcpu.flags, VCPU_FLAG_CF);
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
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
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(GetBit(vcpu.flags, VCPU_FLAG_CF)) d_nubit8(dest) |= 0x80;
			MakeBit(vcpu.flags,VCPU_FLAG_CF,tempCF);
			tempcount--;
		}
		break;
	case 16:
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			SetFlags(SHL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SHL_FLAG);
		}
		break;
	default:CaseError("SHL::len");break;}
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,!!(tempdest8&0x80));
/* vcpuins bug fix #8
		else if(count != 0) {*/
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,!!(tempdest16&0x8000));
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SHR_FLAG);
		}
		break;
	default:CaseError("SHR::len");break;}
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit8(dest), 8)^GetBit(vcpu.flags, VCPU_FLAG_CF));
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			SetFlags(SAL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,GetMSB(d_nubit16(dest), 16)^GetBit(vcpu.flags, VCPU_FLAG_CF));
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			SetFlags(SAL_FLAG);
		}
		break;
	default:CaseError("SAL::len");break;}
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= tempdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,0);
/* vcpuins bug fix #8
		else if(count != 0) {*/
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
			MakeBit(vcpu.flags,VCPU_FLAG_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= tempdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.flags,VCPU_FLAG_OF,0);
/* vcpuins bug fix #8
		else if(count != 0) {*/
		if(count != 0) {
			vcpuins.result = d_nsbit16(dest);
			SetFlags(SAR_FLAG);
		}
		break;
	default:CaseError("SAR::len");break;}
}
static void STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
/* vcpuins bug fix #10: add parameters flagsi, flagdi*/
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
	//qdvgaCheckVideoRam(vramGetAddr(vcpu.es, vcpu.di));
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
	case 8:	vcpuins.bit = 8; d_nubit8(dest) = ~d_nubit8(dest); break;
	case 16:vcpuins.bit = 16;d_nubit16(dest) = ~d_nubit16(dest);break;
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
		MakeBit(vcpu.flags,VCPU_FLAG_OF,!!vcpu.ah);
		MakeBit(vcpu.flags,VCPU_FLAG_CF,!!vcpu.ah);
		break;
	case 16:
		vcpuins.bit = 16;
		tempresult = vcpu.ax * d_nubit16(src);
		vcpu.dx = (tempresult>>16)&0xffff;
		vcpu.ax = tempresult&0xffff;
		MakeBit(vcpu.flags,VCPU_FLAG_OF,!!vcpu.dx);
		MakeBit(vcpu.flags,VCPU_FLAG_CF,!!vcpu.dx);
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
			ClrBit(vcpu.flags,VCPU_FLAG_OF);
			ClrBit(vcpu.flags,VCPU_FLAG_CF);
		} else {
			SetBit(vcpu.flags,VCPU_FLAG_OF);
			SetBit(vcpu.flags,VCPU_FLAG_CF);
		}
		break;
	case 16:
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
#ifdef VCPUINS_DEBUG
	if (qdbiosExecInt(intid)) return;
#endif
	PUSH((void *)&vcpu.flags,16);
	ClrBit(vcpu.flags, (VCPU_FLAG_IF | VCPU_FLAG_TF));
	PUSH((void *)&vcpu.cs,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = vramVarWord(0x0000,intid*4+0);
	vcpu.cs = vramVarWord(0x0000,intid*4+2);
}

void OpError()
{
	vapiPrint("The NXVM CPU has encountered an illegal instruction.\nCS:");
	vapiPrintWord(vcpu.cs);
	vapiPrint(" IP:");
	vapiPrintWord(vcpu.ip);
	vapiPrint(" OP:");
	vapiPrintByte(vramVarByte(vcpu.cs,vcpu.ip+0));
	vapiPrint(" ");
	vapiPrintByte(vramVarByte(vcpu.cs,vcpu.ip+1));
	vapiPrint(" ");
	vapiPrintByte(vramVarByte(vcpu.cs,vcpu.ip+2));
	vapiPrint(" ");
	vapiPrintByte(vramVarByte(vcpu.cs,vcpu.ip+3));
	vapiPrint(" ");
	vapiPrintByte(vramVarByte(vcpu.cs,vcpu.ip+4));
	vapiPrint("\n");
	vmachineStop();
}
void IO_NOP()
{
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IO_NOP\n");
}
void ADD_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_RM8_R8\n");
}
void ADD_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_RM16_R16\n");
}
void ADD_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_R8_RM8\n");
}
void ADD_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_R16_RM16\n");
}
void ADD_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADD((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_AL_I8\n");
}
void ADD_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADD((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADD_AX_I16\n");
}
void PUSH_ES()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.es,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_ES\n");
}
void POP_ES()
{
	vcpu.ip++;
	POP((void *)&vcpu.es,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_ES\n");
}
void OR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_RM8_R8\n");
}
void OR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_RM16_R16\n");
}
void OR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_R8_RM8\n");
}
void OR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_R16_RM16\n");
}
void OR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_AL_I8\n");
}
void OR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OR_AX_I16\n");
}
void PUSH_CS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cs,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_CS\n");
}
void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs,16);
	
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_CS\n");
}
/*void INS_0F()
{// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_0F\n");}*/
void ADC_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_RM8_R8\n");
}
void ADC_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_RM16_R16\n");
}
void ADC_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_R8_RM8\n");
}
void ADC_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_R16_RM16\n");
}
void ADC_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADC((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_AL_I8\n");
}
void ADC_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADC((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ADC_AX_I16\n");
}
void PUSH_SS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ss,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_SS\n");
}
void POP_SS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ss,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_SS\n");
}
void SBB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_RM8_R8\n");
}
void SBB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_RM16_R16\n");
}
void SBB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_R8_RM8\n");
}
void SBB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_R16_RM16\n");
}
void SBB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_AL_I8\n");
}
void SBB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SBB_AX_I16\n");
}
void PUSH_DS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ds,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_DS\n");
}
void POP_DS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ds,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_DS\n");
}
void AND_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_RM8_R8\n");
}
void AND_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_RM16_R16\n");
}
void AND_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_R8_RM8\n");
}
void AND_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_R16_RM16\n");
}
void AND_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	AND((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_AL_I8\n");
}
void AND_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	AND((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AND_AX_I16\n");
}
void ES()
{
	vcpu.ip++;
	vcpu.overds = vcpu.es;
	vcpu.overss = vcpu.es;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  ES:\n");
}
void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al = newAL;
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetBit(vcpu.flags, VCPU_FLAG_CF) || ((newAL < oldAL) || (newAL < 0x06)));
	} else ClrBit(vcpu.flags, VCPU_FLAG_AF);
	if(((vcpu.al & 0xf0) > 0x90) || GetBit(vcpu.flags, VCPU_FLAG_CF)) {
		vcpu.al += 0x60;
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_CF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DAA\n");
}
void SUB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_RM8_R8\n");
}
void SUB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_RM16_R16\n");
}
void SUB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_R8_RM8\n");
}
void SUB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_R16_RM16\n");
}
void SUB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_AL_I8\n");
}
void SUB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SUB_AX_I16\n");
}
void CS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.cs;
	vcpu.overss = vcpu.cs;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CS:\n");
}
void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetBit(vcpu.flags, VCPU_FLAG_AF)) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.flags,VCPU_FLAG_CF,GetBit(vcpu.flags, VCPU_FLAG_CF) || (oldAL < 0x06));
		SetBit(vcpu.flags,VCPU_FLAG_AF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_AF);
	if((vcpu.al > 0x9f) || GetBit(vcpu.flags, VCPU_FLAG_CF)) {
		vcpu.al -= 0x60;
		SetBit(vcpu.flags,VCPU_FLAG_CF);
	} else ClrBit(vcpu.flags,VCPU_FLAG_CF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DAS\n");
}
void XOR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_RM8_R8\n");
}
void XOR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_RM16_R16\n");
}
void XOR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_R8_RM8\n");
}
void XOR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_R16_RM16\n");
}
void XOR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_AL_I8\n");
}
void XOR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XOR_AX_I16\n");
}
void SS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.ss;
	vcpu.overss = vcpu.ss;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SS:\n");
}
void AAA()
{
	vcpu.ip++;
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
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AAA\n");
}
void CMP_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_RM8_R8\n");
}
void CMP_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_RM16_R16\n");
}
void CMP_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_R8_RM8\n");
}
void CMP_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_R16_RM16\n");
}
void CMP_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_AL_I8\n");
}
void CMP_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMP_AX_I16\n");
}
void DS()
{
	vcpu.ip++;
	vcpu.overds = vcpu.ds;
	vcpu.overss = vcpu.ds;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DS:\n");
}
void AAS()
{
	vcpu.ip++;
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
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AAS\n");
}
void INC_AX()
{
	vcpu.ip++;
	INC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_AX\n");
}
void INC_CX()
{
	vcpu.ip++;
	INC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_CX\n");
}
void INC_DX()
{
	vcpu.ip++;
	INC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_DX\n");
}
void INC_BX()
{
	vcpu.ip++;
	INC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_BX\n");
}
void INC_SP()
{
	vcpu.ip++;
	INC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_SP\n");
}
void INC_BP()
{
	vcpu.ip++;
	INC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_BP\n");
}
void INC_SI()
{
	vcpu.ip++;
	INC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_SI\n");
}
void INC_DI()
{
	vcpu.ip++;
	INC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INC_DI\n");
}
void DEC_AX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_AX\n");
}
void DEC_CX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_CX\n");
}
void DEC_DX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_DX\n");
}
void DEC_BX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_BX\n");
}
void DEC_SP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_SP\n");
}
void DEC_BP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_BP\n");
}
void DEC_SI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_SI\n");
}
void DEC_DI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  DEC_DI\n");
}
void PUSH_AX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_AX\n");
}
void PUSH_CX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_CX\n");
}
void PUSH_DX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_DX\n");
}
void PUSH_BX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_BX\n");
}
void PUSH_SP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_SP\n");
}
void PUSH_BP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_BP\n");
}
void PUSH_SI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_SI\n");
}
void PUSH_DI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSH_DI\n");
}
void POP_AX()
{
	vcpu.ip++;
	POP((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_AX\n");
}
void POP_CX()
{
	vcpu.ip++;
	POP((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_CX\n");
}
void POP_DX()
{
	vcpu.ip++;
	POP((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_DX\n");
}
void POP_BX()
{
	vcpu.ip++;
	POP((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_BX\n");
}
void POP_SP()
{
	vcpu.ip++;
	POP((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_SP\n");
}
void POP_BP()
{
	vcpu.ip++;
	POP((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_BP\n");
}
void POP_SI()
{
	vcpu.ip++;
	POP((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_SI\n");
}
void POP_DI()
{
	vcpu.ip++;
	POP((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_DI\n");
}
void JO()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,GetBit(vcpu.flags, VCPU_FLAG_OF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JO\n");
}
void JNO()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,!GetBit(vcpu.flags, VCPU_FLAG_OF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNO\n");
}
void JC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,GetBit(vcpu.flags, VCPU_FLAG_CF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JC\n");
}
void JNC()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,!GetBit(vcpu.flags, VCPU_FLAG_CF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNC\n");
}
void JZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,GetBit(vcpu.flags, VCPU_FLAG_ZF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JZ\n");
}
void JNZ()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,!GetBit(vcpu.flags, VCPU_FLAG_ZF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNZ\n");
}
void JBE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,(GetBit(vcpu.flags, VCPU_FLAG_CF) || GetBit(vcpu.flags, VCPU_FLAG_ZF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JBE\n");
}
void JA()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,(!GetBit(vcpu.flags, VCPU_FLAG_CF) && !GetBit(vcpu.flags, VCPU_FLAG_ZF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JA\n");
}
void JS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,GetBit(vcpu.flags, VCPU_FLAG_SF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JS\n");
}
void JNS()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,!GetBit(vcpu.flags, VCPU_FLAG_SF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNS\n");
}
void JP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,GetBit(vcpu.flags, VCPU_FLAG_PF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JP\n");
}
void JNP()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,!GetBit(vcpu.flags, VCPU_FLAG_PF),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNP\n");
}
void JL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,(GetBit(vcpu.flags, VCPU_FLAG_SF) != GetBit(vcpu.flags, VCPU_FLAG_OF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JL\n");
}
void JNL()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,(GetBit(vcpu.flags, VCPU_FLAG_SF) == GetBit(vcpu.flags, VCPU_FLAG_OF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JNL\n");
}
void JLE()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,((GetBit(vcpu.flags, VCPU_FLAG_SF) != GetBit(vcpu.flags, VCPU_FLAG_OF)) || GetBit(vcpu.flags, VCPU_FLAG_ZF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JLE\n");
}
void JG()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void *)vcpuins.imm,((GetBit(vcpu.flags, VCPU_FLAG_SF) == GetBit(vcpu.flags, VCPU_FLAG_OF)) && !GetBit(vcpu.flags, VCPU_FLAG_ZF)),8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JG\n");
}
void INS_80()
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
void INS_81()
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
void INS_82()
{
	INS_80();
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_82\n");
}
void INS_83()
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
void TEST_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_RM8_R8\n");
}
void TEST_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_RM16_R16\n");
}
void XCHG_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_R8_RM8\n");
}
void XCHG_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_R16_RM16\n");
}
void MOV_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM8_R8\n");
}
void MOV_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM16_R16\n");
}
void MOV_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_R8_RM8\n");
}
void MOV_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_R16_RM16\n");
}
void MOV_RM16_SEG()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_RM16_SEG\n");
}
void LEA_R16_M16()
{
	vcpu.ip++;
	GetModRegRMEA();
	d_nubit16(vcpuins.r) = vcpuins.rm & 0xffff;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LEA_R16_M16\n");
}
void MOV_SEG_RM16()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SEG_RM16\n");
}
void POP_RM16()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
/* vcpuins bug fix #4
		case 0:	POP((void *)vcpuins.rm,16);*/
	case 0:	POP((void *)vcpuins.rm,16);break;
	default:CaseError("POP_RM16::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POP_RM16\n");
}
void NOP()
{
	vcpu.ip++;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  NOP\n");
}
void XCHG_CX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.cx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_CX_AX\n");
}
void XCHG_DX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.dx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_DX_AX\n");
}
void XCHG_BX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_BX_AX\n");
}
void XCHG_SP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.sp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_SP_AX\n");
}
void XCHG_BP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_BP_AX\n");
}
void XCHG_SI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.si,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_SI_AX\n");
}
void XCHG_DI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.di,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XCHG_DI_AX\n");
}
void CBW()
{
	vcpu.ip++;
	vcpu.ax = (t_nsbit8)vcpu.al;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CBW\n");
}
void CWD()
{
	vcpu.ip++;
	if(vcpu.ax&0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CWD\n");
}
void CALL_PTR16_16()
{
	t_nubit16 newcs,newip;
	vcpu.ip++;
	GetImm(16);
	newip = d_nubit16(vcpuins.imm);
	GetImm(16);
	newcs = d_nubit16(vcpuins.imm);
	PUSH((void *)&vcpu.cs,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = newip;
	vcpu.cs = newcs;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CALL_PTR16_16\n");
}
void WAIT()
{
	vcpu.ip++;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  WAIT\n");
}
void PUSHF()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.flags,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  PUSHF\n");
}
void POPF()
{
	vcpu.ip++;
	POP((void *)&vcpu.flags,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  POPF\n");
}
void SAHF()
{
	vcpu.ip++;
	d_nubit8(&vcpu.flags) = vcpu.ah;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  SAHF\n");
}
void LAHF()
{
	vcpu.ip++;
//	vapiPrint("1:LAHF:%4X\n",vcpu.flags);
	vcpu.ah = d_nubit8(&vcpu.flags);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LAHF\n");
}
void MOV_AL_M8()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.al,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AL_M8\n");
}
void MOV_AX_M16()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AX_M16\n");
}
void MOV_M8_AL()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.al,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M8_AL\n");
}
void MOV_M16_AX()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M16_AX\n");
}
void MOVSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) MOVS(8);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			MOVS(8);
			vcpu.cx--;
		}
	}
}
void MOVSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) MOVS(16);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			MOVS(16);
			vcpu.cx--;
		}
	}
}
void CMPSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) CMPS(8);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			CMPS(8);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
void CMPSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) CMPS(16);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			CMPS(16);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
void TEST_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	TEST((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_AL_I8\n");
}
void TEST_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  TEST_AX_I16\n");
}
void STOSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) STOS(8);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			STOS(8);
			vcpu.cx--;
		}
	}
}
void STOSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) STOS(16);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			STOS(16);
			vcpu.cx--;
		}
	}
}
void LODSB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) LODS(8);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			LODS(8);
			vcpu.cx--;
		}
	}
}
void LODSW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) LODS(16);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			LODS(16);
			vcpu.cx--;
		}
	}
}
void SCASB()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) SCAS(8);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			SCAS(8);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
void SCASW()
{
	vcpu.ip++;
	if(vcpuins.rep == RT_NONE) SCAS(16);
	else {
		while(vcpu.cx) {
			vcpuinsExecInt();
			SCAS(16);
			vcpu.cx--;
			if((vcpuins.rep == RT_REPZ && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) || (vcpuins.rep == RT_REPZNZ && GetBit(vcpu.flags, VCPU_FLAG_ZF))) break;
		}
	}
}
void MOV_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AL_I8\n");
}
void MOV_CL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CL_I8\n");
}
void MOV_DL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DL_I8\n");
}
void MOV_BL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BL_I8\n");
}
void MOV_AH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AH_I8\n");
}
void MOV_CH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CH_I8\n");
}
void MOV_DH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DH_I8\n");
}
void MOV_BH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BH_I8\n");
}
void MOV_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_AX_I16\n");
}
void MOV_CX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_CX_I16\n");
}
void MOV_DX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DX_I16\n");
}
void MOV_BX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BX_I16\n");
}
void MOV_SP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SP_I16\n");
}
void MOV_BP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_BP_I16\n");
}
void MOV_SI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_SI_I16\n");
}
void MOV_DI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_DI_I16\n");
}
void INS_C0()
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
void INS_C1()
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
void RET_I16()
{
/* vcpuins bug fix #15 */
	t_nubit16 addsp;
	vcpu.ip++;
	GetImm(16);
	addsp = d_nubit16(vcpuins.imm);
	POP((void *)&vcpu.ip,16);
	vcpu.sp += addsp;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RET_I16\n");
}
void RET()
{
	POP((void *)&vcpu.ip,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RET\n");
}
void LES_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.es,(void *)(vcpuins.rm+2),16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LES_R16_M16\n");
}
void LDS_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.ds,(void *)(vcpuins.rm+2),16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LDS_R16_M16\n");
}
void MOV_M8_I8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M8_I8\n");
}
void MOV_M16_I16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  MOV_M16_I16\n");
}
void RETF_I16()
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
void RETF()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  RETF\n");
}
void INT3()
{
	vcpu.ip++;
	INT(0x03);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INT3\n");
}
void INT_I8()
{
	vcpu.ip++;
	GetImm(8);
	INT(d_nubit8(vcpuins.imm));
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INT_I8\n");
}
void INTO()
{
	vcpu.ip++;
	if(GetBit(vcpu.flags, VCPU_FLAG_OF)) INT(0x04);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INTO\n");
}
void IRET()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	POP((void *)&vcpu.flags,16);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IRET\n");
}
void INS_D0()
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
void INS_D1()
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
void INS_D2()
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
void INS_D3()
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
void AAM()
{
	t_nubit8 base,tempAL;
	vcpu.ip++;
	GetImm(8);
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
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AAM\n");
}
void AAD()
{
	t_nubit8 base,tempAL,tempAH;
	vcpu.ip++;
	GetImm(8);
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
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  AAD\n");
}
void XLAT()
{
	vcpu.ip++;
	vcpu.al = vramVarByte(vcpu.overds,vcpu.bx+vcpu.al);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  XLAT\n");
}
/*
void INS_D9()
{// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_D9\n");
}
void INS_DB()
{// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_DB\n");
}
*/
void LOOPNZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
/* vcpuins bug fix #12 */
	rel8 = d_nsbit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && !GetBit(vcpu.flags, VCPU_FLAG_ZF)) vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOOPNZ\n");
}
void LOOPZ()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
/* vcpuins bug fix #12 */
	rel8 = d_nsbit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && GetBit(vcpu.flags, VCPU_FLAG_ZF)) vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOOPZ\n");
}
void LOOP()
{
	t_nsbit8 rel8;
/* vcpuins bug fix #2
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.ip += rel8;*/
	vcpu.ip++;
	GetImm(8);
/* vcpuins bug fix #12 */
	rel8 = d_nsbit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOOP\n");
}
void JCXZ_REL8()
{
	vcpu.ip++;
	GetImm(8);
	JCC((void*)vcpuins.imm,!vcpu.cx,8);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JCXZ_REL8\n");
}
void IN_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(vcpuins.imm)]);
	vcpu.al = vcpu.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AL_I8\n");
}
void IN_AX_I8()
{
	vcpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(vcpuins.imm)+0]);
	vcpu.al = vcpu.iobyte;
	ExecFun(vport.in[d_nubit8(vcpuins.imm)+1]);
	vcpu.ah = vcpu.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AX_I8\n");
}
void OUT_I8_AL()
{
	vcpu.ip++;
	GetImm(8);
	vcpu.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_I8_AL\n");
}
void OUT_I8_AX()
{
	vcpu.ip++;
	GetImm(8);
	vcpu.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)+0]);
	vcpu.iobyte = vcpu.ah;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)+1]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_I8_AX\n");
}
void CALL_REL16()
{
	t_nsbit16 rel16;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.imm);
	PUSH((void *)&vcpu.ip,16);
/* vcpuins bug fix #12
	vcpu.ip += d_nubit16(vcpuins.imm);*/
	vcpu.ip += rel16;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CALL_REL16\n");
}
void JMP_REL16()
{
	t_nsbit16 rel16;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.imm);
/* vcpuins bug fix #p
	vcpu.ip += d_nubit16(vcpuins.imm);*/
	vcpu.ip += rel16;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JMP_REL16\n");
}
void JMP_PTR16_16()
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
void JMP_REL8()
{
	t_nsbit8 rel8;
	vcpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(vcpuins.imm);
/* vcpuins bug fix #9
	vcpu.ip += d_nubit8(vcpuins.imm);*/
	vcpu.ip += rel8;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  JMP_REL8\n");
}
void IN_AL_DX()
{
	vcpu.ip++;
	ExecFun(vport.in[vcpu.dx]);
	vcpu.al = vcpu.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AL_DX\n");
}
void IN_AX_DX()
{
	vcpu.ip++;
	ExecFun(vport.in[vcpu.dx+0]);
	vcpu.al = vcpu.iobyte;
	ExecFun(vport.in[vcpu.dx+1]);
	vcpu.ah = vcpu.iobyte;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  IN_AX_DX\n");
}
void OUT_DX_AL()
{
	vcpu.ip++;
	vcpu.iobyte = vcpu.al;
	ExecFun(vport.out[vcpu.dx]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_DX_AL\n");
}
void OUT_DX_AX()
{
	vcpu.ip++;
	/* TODO: this maybe a bug: GetImm(8); */
	vcpu.iobyte = vcpu.al;
	ExecFun(vport.out[vcpu.dx+0]);
	vcpu.iobyte = vcpu.ah;
	ExecFun(vport.out[vcpu.dx+1]);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  OUT_DX_AX\n");
}
void LOCK()
{
	vcpu.ip++;
	/* Not Implemented */
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  LOCK\n");
}
void REPNZ()
{
	// CMPS,SCAS
	vcpu.ip++;
	vcpuins.rep = RT_REPZNZ;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  REPNZ\n");
}
void REP()
{	// MOVS,LODS,STOS,CMPS,SCAS
	vcpu.ip++;
	vcpuins.rep = RT_REPZ;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  REP\n");
}
void HLT()
{
	vcpu.ip++;
	/* Not Implemented */
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  HLT\n");
}
void CMC()
{
	vcpu.ip++;
	vcpu.flags ^= VCPU_FLAG_CF;
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CMC\n");
}
void INS_F6()
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
void INS_F7()
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
void CLC()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CLC\n");
}
void STC()
{
	vcpu.ip++;
	SetBit(vcpu.flags, VCPU_FLAG_CF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  STC\n");
}
void CLI()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_FLAG_IF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CLI\n");
}
void STI()
{
	vcpu.ip++;
	SetBit(vcpu.flags, VCPU_FLAG_IF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  STI\n");
}
void CLD()
{
	vcpu.ip++;
	ClrBit(vcpu.flags, VCPU_FLAG_DF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  CLD\n");
}
void STD()
{
	vcpu.ip++;
	SetBit(vcpu.flags,VCPU_FLAG_DF);
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  STD\n");
}
void INS_FE()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	INC((void *)vcpuins.rm,8);	break;
	case 1:	DEC((void *)vcpuins.rm,8);	break;
	default:CaseError("INS_FE::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs,vcpu.ip);vapiPrint("  INS_FE\n");
}
void INS_FF()
{
	vcpu.ip++;
	GetModRegRM(0,16);
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
/* vcpuins bug fix #11
		vcpu.cs = _nubit16(vcpuins.rm+1);*/
		vcpu.cs = d_nubit16(vcpuins.rm+2);
		break;
	case 4:	/* JMP_RM16 */
		vcpu.ip = d_nubit16(vcpuins.rm);
		break;
	case 5:	/* JMP_M16_16 */
		vcpu.ip = d_nubit16(vcpuins.rm);
/* vcpuins bug fix #11
		vcpu.cs = d_nubit16(vcpuins.rm+1);*/
		vcpu.cs = d_nubit16(vcpuins.rm+2);
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

void vcpuinsExecIns()
{
	t_nubit8 opcode = vramVarByte(vcpu.cs, vcpu.ip);
/* vcpuins bug fix #16
	Note: in this bug, if an interrupt generated between
	prefix and operation, the prefix may not be deployed
	ExecFun(vcpuins.table[opcode]);
	if (!IsPrefix(opcode)) ClrPrefix();*/
/* vcpuins bug fix #17
	Note: in this case, if we use two prefixes, the second prefix
	will be discarded incorrectly
	if (IsPrefix(opcode)) ExecFun(vcpuins.table[opcode]);
	opcode = vramVarByte(vcpu.cs, vcpu.ip);
	ExecFun(vcpuins.table[opcode]);
	ClrPrefix();*/
	ExecFun(vcpuins.table[opcode]);
	if (!IsPrefix(opcode)) ClrPrefix();
	else vcpuinsExecIns();
}
void vcpuinsExecInt()
{
	/* hardware interrupt handeler */
	t_nubit8 intr;
	if(vcpu.flagnmi) {
		INT(0x02);
	}
	vcpu.flagnmi = 0x00;
	if(GetBit(vcpu.flags, VCPU_FLAG_IF) && vpicScanINTR()) {
		intr = vpicGetINTR();
		INT(intr);
	}
	if(GetBit(vcpu.flags, VCPU_FLAG_TF)) {
		INT(0x01);
	}
}

void vcpuinsInit()
{
	memset(&vcpuins, 0x00, sizeof(t_cpuins));
	ClrPrefix();
	vcpuins.table[0x00] = (t_faddrcc)ADD_RM8_R8;
	vcpuins.table[0x01] = (t_faddrcc)ADD_RM16_R16;
	vcpuins.table[0x02] = (t_faddrcc)ADD_R8_RM8;
	vcpuins.table[0x03] = (t_faddrcc)ADD_R16_RM16;
	vcpuins.table[0x04] = (t_faddrcc)ADD_AL_I8;
	vcpuins.table[0x05] = (t_faddrcc)ADD_AX_I16;
	vcpuins.table[0x06] = (t_faddrcc)PUSH_ES;
	vcpuins.table[0x07] = (t_faddrcc)POP_ES;
	vcpuins.table[0x08] = (t_faddrcc)OR_RM8_R8;
	vcpuins.table[0x09] = (t_faddrcc)OR_RM16_R16;
	vcpuins.table[0x0a] = (t_faddrcc)OR_R8_RM8;
	vcpuins.table[0x0b] = (t_faddrcc)OR_R16_RM16;
	vcpuins.table[0x0c] = (t_faddrcc)OR_AL_I8;
	vcpuins.table[0x0d] = (t_faddrcc)OR_AX_I16;
	vcpuins.table[0x0e] = (t_faddrcc)PUSH_CS;
	vcpuins.table[0x0f] = (t_faddrcc)POP_CS;
	//vcpuins.table[0x0f] = (t_faddrcc)INS_0F;
	vcpuins.table[0x10] = (t_faddrcc)ADC_RM8_R8;
	vcpuins.table[0x11] = (t_faddrcc)ADC_RM16_R16;
	vcpuins.table[0x12] = (t_faddrcc)ADC_R8_RM8;
	vcpuins.table[0x13] = (t_faddrcc)ADC_R16_RM16;
	vcpuins.table[0x14] = (t_faddrcc)ADC_AL_I8;
	vcpuins.table[0x15] = (t_faddrcc)ADC_AX_I16;
	vcpuins.table[0x16] = (t_faddrcc)PUSH_SS;
	vcpuins.table[0x17] = (t_faddrcc)POP_SS;
	vcpuins.table[0x18] = (t_faddrcc)SBB_RM8_R8;
	vcpuins.table[0x19] = (t_faddrcc)SBB_RM16_R16;
	vcpuins.table[0x1a] = (t_faddrcc)SBB_R8_RM8;
	vcpuins.table[0x1b] = (t_faddrcc)SBB_R16_RM16;
	vcpuins.table[0x1c] = (t_faddrcc)SBB_AL_I8;
	vcpuins.table[0x1d] = (t_faddrcc)SBB_AX_I16;
	vcpuins.table[0x1e] = (t_faddrcc)PUSH_DS;
	vcpuins.table[0x1f] = (t_faddrcc)POP_DS;
	vcpuins.table[0x20] = (t_faddrcc)AND_RM8_R8;
	vcpuins.table[0x21] = (t_faddrcc)AND_RM16_R16;
	vcpuins.table[0x22] = (t_faddrcc)AND_R8_RM8;
	vcpuins.table[0x23] = (t_faddrcc)AND_R16_RM16;
	vcpuins.table[0x24] = (t_faddrcc)AND_AL_I8;
	vcpuins.table[0x25] = (t_faddrcc)AND_AX_I16;
	vcpuins.table[0x26] = (t_faddrcc)ES;
	vcpuins.table[0x27] = (t_faddrcc)DAA;
	vcpuins.table[0x28] = (t_faddrcc)SUB_RM8_R8;
	vcpuins.table[0x29] = (t_faddrcc)SUB_RM16_R16;
	vcpuins.table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	vcpuins.table[0x2b] = (t_faddrcc)SUB_R16_RM16;
	vcpuins.table[0x2c] = (t_faddrcc)SUB_AL_I8;
	vcpuins.table[0x2d] = (t_faddrcc)SUB_AX_I16;
	vcpuins.table[0x2e] = (t_faddrcc)CS;
	vcpuins.table[0x2f] = (t_faddrcc)DAS;
	vcpuins.table[0x30] = (t_faddrcc)XOR_RM8_R8;
	vcpuins.table[0x31] = (t_faddrcc)XOR_RM16_R16;
	vcpuins.table[0x32] = (t_faddrcc)XOR_R8_RM8;
	vcpuins.table[0x33] = (t_faddrcc)XOR_R16_RM16;
	vcpuins.table[0x34] = (t_faddrcc)XOR_AL_I8;
	vcpuins.table[0x35] = (t_faddrcc)XOR_AX_I16;
	vcpuins.table[0x36] = (t_faddrcc)SS;
	vcpuins.table[0x37] = (t_faddrcc)AAA;
	vcpuins.table[0x38] = (t_faddrcc)CMP_RM8_R8;
	vcpuins.table[0x39] = (t_faddrcc)CMP_RM16_R16;
	vcpuins.table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	vcpuins.table[0x3b] = (t_faddrcc)CMP_R16_RM16;
	vcpuins.table[0x3c] = (t_faddrcc)CMP_AL_I8;
	vcpuins.table[0x3d] = (t_faddrcc)CMP_AX_I16;
	vcpuins.table[0x3e] = (t_faddrcc)DS;
	vcpuins.table[0x3f] = (t_faddrcc)AAS;
	vcpuins.table[0x40] = (t_faddrcc)INC_AX;
	vcpuins.table[0x41] = (t_faddrcc)INC_CX;
	vcpuins.table[0x42] = (t_faddrcc)INC_DX;
	vcpuins.table[0x43] = (t_faddrcc)INC_BX;
	vcpuins.table[0x44] = (t_faddrcc)INC_SP;
	vcpuins.table[0x45] = (t_faddrcc)INC_BP;
	vcpuins.table[0x46] = (t_faddrcc)INC_SI;
	vcpuins.table[0x47] = (t_faddrcc)INC_DI;
	vcpuins.table[0x48] = (t_faddrcc)DEC_AX;
	vcpuins.table[0x49] = (t_faddrcc)DEC_CX;
	vcpuins.table[0x4a] = (t_faddrcc)DEC_DX;
	vcpuins.table[0x4b] = (t_faddrcc)DEC_BX;
	vcpuins.table[0x4c] = (t_faddrcc)DEC_SP;
	vcpuins.table[0x4d] = (t_faddrcc)DEC_BP;
	vcpuins.table[0x4e] = (t_faddrcc)DEC_SI;
	vcpuins.table[0x4f] = (t_faddrcc)DEC_DI;
	vcpuins.table[0x50] = (t_faddrcc)PUSH_AX;
	vcpuins.table[0x51] = (t_faddrcc)PUSH_CX;
	vcpuins.table[0x52] = (t_faddrcc)PUSH_DX;
	vcpuins.table[0x53] = (t_faddrcc)PUSH_BX;
	vcpuins.table[0x54] = (t_faddrcc)PUSH_SP;
	vcpuins.table[0x55] = (t_faddrcc)PUSH_BP;
	vcpuins.table[0x56] = (t_faddrcc)PUSH_SI;
	vcpuins.table[0x57] = (t_faddrcc)PUSH_DI;
	vcpuins.table[0x58] = (t_faddrcc)POP_AX;
	vcpuins.table[0x59] = (t_faddrcc)POP_CX;
	vcpuins.table[0x5a] = (t_faddrcc)POP_DX;
	vcpuins.table[0x5b] = (t_faddrcc)POP_BX;
	vcpuins.table[0x5c] = (t_faddrcc)POP_SP;
	vcpuins.table[0x5d] = (t_faddrcc)POP_BP;
	vcpuins.table[0x5e] = (t_faddrcc)POP_SI;
	vcpuins.table[0x5f] = (t_faddrcc)POP_DI;
	vcpuins.table[0x60] = (t_faddrcc)OpError;
	vcpuins.table[0x61] = (t_faddrcc)OpError;
	vcpuins.table[0x62] = (t_faddrcc)OpError;
	vcpuins.table[0x63] = (t_faddrcc)OpError;
	vcpuins.table[0x64] = (t_faddrcc)OpError;
	vcpuins.table[0x65] = (t_faddrcc)OpError;
	vcpuins.table[0x66] = (t_faddrcc)OpError;
	vcpuins.table[0x67] = (t_faddrcc)OpError;
	vcpuins.table[0x68] = (t_faddrcc)OpError;
	//vcpuins.table[0x66] = (t_faddrcc)OpdSize;
	//vcpuins.table[0x67] = (t_faddrcc)AddrSize;
	//vcpuins.table[0x68] = (t_faddrcc)PUSH_I16;
	vcpuins.table[0x69] = (t_faddrcc)OpError;
	vcpuins.table[0x6a] = (t_faddrcc)OpError;
	vcpuins.table[0x6b] = (t_faddrcc)OpError;
	vcpuins.table[0x6c] = (t_faddrcc)OpError;
	vcpuins.table[0x6d] = (t_faddrcc)OpError;
	vcpuins.table[0x6e] = (t_faddrcc)OpError;
	vcpuins.table[0x6f] = (t_faddrcc)OpError;
	vcpuins.table[0x70] = (t_faddrcc)JO;
	vcpuins.table[0x71] = (t_faddrcc)JNO;
	vcpuins.table[0x72] = (t_faddrcc)JC;
	vcpuins.table[0x73] = (t_faddrcc)JNC;
	vcpuins.table[0x74] = (t_faddrcc)JZ;
	vcpuins.table[0x75] = (t_faddrcc)JNZ;
	vcpuins.table[0x76] = (t_faddrcc)JBE;
	vcpuins.table[0x77] = (t_faddrcc)JA;
	vcpuins.table[0x78] = (t_faddrcc)JS;
	vcpuins.table[0x79] = (t_faddrcc)JNS;
	vcpuins.table[0x7a] = (t_faddrcc)JP;
	vcpuins.table[0x7b] = (t_faddrcc)JNP;
	vcpuins.table[0x7c] = (t_faddrcc)JL;
	vcpuins.table[0x7d] = (t_faddrcc)JNL;
	vcpuins.table[0x7e] = (t_faddrcc)JLE;
	vcpuins.table[0x7f] = (t_faddrcc)JG;
	vcpuins.table[0x80] = (t_faddrcc)INS_80;
	vcpuins.table[0x81] = (t_faddrcc)INS_81;
	vcpuins.table[0x82] = (t_faddrcc)INS_82;
	vcpuins.table[0x83] = (t_faddrcc)INS_83;
	vcpuins.table[0x84] = (t_faddrcc)TEST_RM8_R8;
	vcpuins.table[0x85] = (t_faddrcc)TEST_RM16_R16;
	vcpuins.table[0x86] = (t_faddrcc)XCHG_R8_RM8;
	vcpuins.table[0x87] = (t_faddrcc)XCHG_R16_RM16;
	vcpuins.table[0x88] = (t_faddrcc)MOV_RM8_R8;
	vcpuins.table[0x89] = (t_faddrcc)MOV_RM16_R16;
	vcpuins.table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	vcpuins.table[0x8b] = (t_faddrcc)MOV_R16_RM16;
	vcpuins.table[0x8c] = (t_faddrcc)MOV_RM16_SEG;
	vcpuins.table[0x8d] = (t_faddrcc)LEA_R16_M16;
	vcpuins.table[0x8e] = (t_faddrcc)MOV_SEG_RM16;
	vcpuins.table[0x8f] = (t_faddrcc)POP_RM16;
	vcpuins.table[0x90] = (t_faddrcc)NOP;
	vcpuins.table[0x91] = (t_faddrcc)XCHG_CX_AX;
	vcpuins.table[0x92] = (t_faddrcc)XCHG_DX_AX;
	vcpuins.table[0x93] = (t_faddrcc)XCHG_BX_AX;
	vcpuins.table[0x94] = (t_faddrcc)XCHG_SP_AX;
	vcpuins.table[0x95] = (t_faddrcc)XCHG_BP_AX;
	vcpuins.table[0x96] = (t_faddrcc)XCHG_SI_AX;
	vcpuins.table[0x97] = (t_faddrcc)XCHG_DI_AX;
	vcpuins.table[0x98] = (t_faddrcc)CBW;
	vcpuins.table[0x99] = (t_faddrcc)CWD;
	vcpuins.table[0x9a] = (t_faddrcc)CALL_PTR16_16;
	vcpuins.table[0x9b] = (t_faddrcc)WAIT;
	vcpuins.table[0x9c] = (t_faddrcc)PUSHF;
	vcpuins.table[0x9d] = (t_faddrcc)POPF;
	vcpuins.table[0x9e] = (t_faddrcc)SAHF;
	vcpuins.table[0x9f] = (t_faddrcc)LAHF;
	vcpuins.table[0xa0] = (t_faddrcc)MOV_AL_M8;
	vcpuins.table[0xa1] = (t_faddrcc)MOV_AX_M16;
	vcpuins.table[0xa2] = (t_faddrcc)MOV_M8_AL;
	vcpuins.table[0xa3] = (t_faddrcc)MOV_M16_AX;
	vcpuins.table[0xa4] = (t_faddrcc)MOVSB;
	vcpuins.table[0xa5] = (t_faddrcc)MOVSW;
	vcpuins.table[0xa6] = (t_faddrcc)CMPSB;
	vcpuins.table[0xa7] = (t_faddrcc)CMPSW;
	vcpuins.table[0xa8] = (t_faddrcc)TEST_AL_I8;
	vcpuins.table[0xa9] = (t_faddrcc)TEST_AX_I16;
	vcpuins.table[0xaa] = (t_faddrcc)STOSB;
	vcpuins.table[0xab] = (t_faddrcc)STOSW;
	vcpuins.table[0xac] = (t_faddrcc)LODSB;
	vcpuins.table[0xad] = (t_faddrcc)LODSW;
	vcpuins.table[0xae] = (t_faddrcc)SCASB;
	vcpuins.table[0xaf] = (t_faddrcc)SCASW;
	vcpuins.table[0xb0] = (t_faddrcc)MOV_AL_I8;
	vcpuins.table[0xb1] = (t_faddrcc)MOV_CL_I8;
	vcpuins.table[0xb2] = (t_faddrcc)MOV_DL_I8;
	vcpuins.table[0xb3] = (t_faddrcc)MOV_BL_I8;
	vcpuins.table[0xb4] = (t_faddrcc)MOV_AH_I8;
	vcpuins.table[0xb5] = (t_faddrcc)MOV_CH_I8;
	vcpuins.table[0xb6] = (t_faddrcc)MOV_DH_I8;
	vcpuins.table[0xb7] = (t_faddrcc)MOV_BH_I8;
	vcpuins.table[0xb8] = (t_faddrcc)MOV_AX_I16;
	vcpuins.table[0xb9] = (t_faddrcc)MOV_CX_I16;
	vcpuins.table[0xba] = (t_faddrcc)MOV_DX_I16;
	vcpuins.table[0xbb] = (t_faddrcc)MOV_BX_I16;
	vcpuins.table[0xbc] = (t_faddrcc)MOV_SP_I16;
	vcpuins.table[0xbd] = (t_faddrcc)MOV_BP_I16;
	vcpuins.table[0xbe] = (t_faddrcc)MOV_SI_I16;
	vcpuins.table[0xbf] = (t_faddrcc)MOV_DI_I16;
	vcpuins.table[0xc0] = (t_faddrcc)OpError;
	vcpuins.table[0xc1] = (t_faddrcc)OpError;
	vcpuins.table[0xc2] = (t_faddrcc)RET_I16;
	vcpuins.table[0xc3] = (t_faddrcc)RET;
	vcpuins.table[0xc4] = (t_faddrcc)LES_R16_M16;
	vcpuins.table[0xc5] = (t_faddrcc)LDS_R16_M16;
	vcpuins.table[0xc6] = (t_faddrcc)MOV_M8_I8;
	vcpuins.table[0xc7] = (t_faddrcc)MOV_M16_I16;
	vcpuins.table[0xc8] = (t_faddrcc)OpError;
	vcpuins.table[0xc9] = (t_faddrcc)OpError;
	vcpuins.table[0xca] = (t_faddrcc)RETF_I16;
	vcpuins.table[0xcb] = (t_faddrcc)RETF;
	vcpuins.table[0xcc] = (t_faddrcc)INT3;
	vcpuins.table[0xcd] = (t_faddrcc)INT_I8;
	vcpuins.table[0xce] = (t_faddrcc)INTO;
	vcpuins.table[0xcf] = (t_faddrcc)IRET;
	vcpuins.table[0xd0] = (t_faddrcc)INS_D0;
	vcpuins.table[0xd1] = (t_faddrcc)INS_D1;
	vcpuins.table[0xd2] = (t_faddrcc)INS_D2;
	vcpuins.table[0xd3] = (t_faddrcc)INS_D3;
	vcpuins.table[0xd4] = (t_faddrcc)AAM;
	vcpuins.table[0xd5] = (t_faddrcc)AAD;
	vcpuins.table[0xd6] = (t_faddrcc)OpError;
	vcpuins.table[0xd7] = (t_faddrcc)XLAT;
	vcpuins.table[0xd8] = (t_faddrcc)OpError;
	vcpuins.table[0xd9] = (t_faddrcc)OpError;
	//vcpuins.table[0xd9] = (t_faddrcc)INS_D9;
	vcpuins.table[0xda] = (t_faddrcc)OpError;
	vcpuins.table[0xdb] = (t_faddrcc)OpError;
	//vcpuins.table[0xdb] = (t_faddrcc)INS_DB;
	vcpuins.table[0xdc] = (t_faddrcc)OpError;
	vcpuins.table[0xdd] = (t_faddrcc)OpError;
	vcpuins.table[0xde] = (t_faddrcc)OpError;
	vcpuins.table[0xdf] = (t_faddrcc)OpError;
	vcpuins.table[0xe0] = (t_faddrcc)LOOPNZ;
	vcpuins.table[0xe1] = (t_faddrcc)LOOPZ;
	vcpuins.table[0xe2] = (t_faddrcc)LOOP;
	vcpuins.table[0xe3] = (t_faddrcc)JCXZ_REL8;
	vcpuins.table[0xe4] = (t_faddrcc)IN_AL_I8;
	vcpuins.table[0xe5] = (t_faddrcc)IN_AX_I8;
	vcpuins.table[0xe6] = (t_faddrcc)OUT_I8_AL;
	vcpuins.table[0xe7] = (t_faddrcc)OUT_I8_AX;
	vcpuins.table[0xe8] = (t_faddrcc)CALL_REL16;
	vcpuins.table[0xe9] = (t_faddrcc)JMP_REL16;
	vcpuins.table[0xea] = (t_faddrcc)JMP_PTR16_16;
	vcpuins.table[0xeb] = (t_faddrcc)JMP_REL8;
	vcpuins.table[0xec] = (t_faddrcc)IN_AL_DX;
	vcpuins.table[0xed] = (t_faddrcc)IN_AX_DX;
	vcpuins.table[0xee] = (t_faddrcc)OUT_DX_AL;
	vcpuins.table[0xef] = (t_faddrcc)OUT_DX_AX;
	vcpuins.table[0xf0] = (t_faddrcc)LOCK;
	vcpuins.table[0xf1] = (t_faddrcc)OpError;
	vcpuins.table[0xf2] = (t_faddrcc)REPNZ;
	vcpuins.table[0xf3] = (t_faddrcc)REP;
	vcpuins.table[0xf4] = (t_faddrcc)HLT;
	vcpuins.table[0xf5] = (t_faddrcc)CMC;
	vcpuins.table[0xf6] = (t_faddrcc)INS_F6;
	vcpuins.table[0xf7] = (t_faddrcc)INS_F7;
	vcpuins.table[0xf8] = (t_faddrcc)CLC;
	vcpuins.table[0xf9] = (t_faddrcc)STC;
	vcpuins.table[0xfa] = (t_faddrcc)CLI;
	vcpuins.table[0xfb] = (t_faddrcc)STI;
	vcpuins.table[0xfc] = (t_faddrcc)CLD;
	vcpuins.table[0xfd] = (t_faddrcc)STD;
	vcpuins.table[0xfe] = (t_faddrcc)INS_FE;
	vcpuins.table[0xff] = (t_faddrcc)INS_FF;
}
void vcpuinsFinal() {}