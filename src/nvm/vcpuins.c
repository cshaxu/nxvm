/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "vcpu.h"
#include "vmemory.h"
#include "vcpuins.h"
#include "system/vapi.h"

#define MOD	((modrm&0xc0)>>6)
#define REG	((modrm&0x38)>>3)
#define RM	((modrm&0x07)>>0)

#define ADD_FLAG (OF | SF | ZF | AF | CF | PF)
#define OR_FLAG (SF | ZF | PF)
#define ADC_FLAG (OF | SF | ZF | AF | CF | PF)
#define SBB_FLAG (OF | SF | ZF | AF | CF | PF)
#define AND_FLAG (SF | ZF | PF)
#define SUB_FLAG (OF | SF | ZF | AF | CF | PF)
#define XOR_FLAG (SF | ZF | PF)
#define CMP_FLAG (OF | SF | ZF | AF | CF | PF)
#define INC_FLAG (OF | SF | ZF | AF | PF)
#define DEC_FLAG (OF | SF | ZF | AF | PF)
#define TEST_FLAG (SF | ZF | PF)

static t_nubitcc flgoperand1,flgoperand2,flgresult,flglen;
static enum {
	ADD8,ADD16,
	//OR8,OR16,
	ADC8,ADC16,
	SBB8,SBB16,
	//AND8,AND16,
	SUB8,SUB16,
	//XOR8,XOR16,
	CMP8,CMP16
	//TEST8,TEST16
} flginstype;

t_faddrcc InTable[0x10000];	
t_faddrcc OutTable[0x10000];
t_faddrcc InsTable[0x100];

static t_nubit16 insDS;
static t_nubit16 insSS;
static t_vaddrcc rm,r,imm;

void SetCF(t_bool flg)
{
	if(flg) vcpu.flags |= CF;
	else vcpu.flags &= ~CF;
}
void SetPF(t_bool flg)
{
	if(flg) vcpu.flags |= PF;
	else vcpu.flags &= ~PF;
}
void SetAF(t_bool flg)
{
	if(flg) vcpu.flags |= AF;
	else vcpu.flags &= ~AF;
}
void SetZF(t_bool flg)
{
	if(flg) vcpu.flags |= ZF;
	else vcpu.flags &= ~ZF;
}
void SetSF(t_bool flg)
{
	if(flg) vcpu.flags |= SF;
	else vcpu.flags &= ~SF;
}
void SetTF(t_bool flg)
{
	if(flg) vcpu.flags |= TF;
	else vcpu.flags &= ~TF;
}
void SetIF(t_bool flg)
{
	if(flg) vcpu.flags |= IF;
	else vcpu.flags &= ~IF;
}
void SetDF(t_bool flg)
{
	if(flg) vcpu.flags |= DF;
	else vcpu.flags &= ~DF;
}
void SetOF(t_bool flg)
{
	if(flg) vcpu.flags |= OF;
	else vcpu.flags &= ~OF;
}

t_nubit16 GetCF() {return (vcpu.flags&CF);}
t_nubit16 GetPF() {return (vcpu.flags&PF);}
t_nubit16 GetAF() {return (vcpu.flags&AF);}
t_nubit16 GetZF() {return (vcpu.flags&ZF);}
t_nubit16 GetSF() {return (vcpu.flags&SF);}
t_nubit16 GetTF() {return (vcpu.flags&TF);}
t_nubit16 GetIF() {return (vcpu.flags&IF);}
t_nubit16 GetDF() {return (vcpu.flags&DF);}
t_nubit16 GetOF() {return (vcpu.flags&OF);}

void CalcCF()
{
	switch(flginstype) {
	case ADD8:
	case ADD16:
		SetCF((flgresult < flgoperand1) || (flgresult < flgoperand2));
		break;
	case ADC8:
	case ADC16:
		SetCF(flgresult <= flgoperand1);
		break;
	case SBB8:
		SetCF((flgoperand1 < flgresult) || (flgoperand2 == 0xff));
		break;
	case SBB16:
		SetCF((flgoperand1 < flgresult) || (flgoperand2 == 0xffff));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		SetCF(flgoperand1 < flgoperand2);
		break;
	default:break;}
}
void CalcOF()
{
	switch(flginstype) {
	case ADD8:
	case ADC8:
		SetOF(((flgoperand1&0x0080) == (flgoperand2&0x0080)) && ((flgoperand1&0x0080) != (flgresult&0x0080)));
		break;
	case ADD16:
	case ADC16:
		SetOF(((flgoperand1&0x8000) == (flgoperand2&0x8000)) && ((flgoperand1&0x8000) != (flgresult&0x8000)));
		break;
	case SBB8:
	case SUB8:
	case CMP8:
		SetOF(((flgoperand1&0x0080) != (flgoperand2&0x0080)) && ((flgoperand2&0x0080) == (flgresult&0x0080)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		SetOF(((flgoperand1&0x8000) != (flgoperand2&0x8000)) && ((flgoperand2&0x8000) == (flgresult&0x8000)));
		break;
	default:break;}
}

void CalcPF()
{
	t_nubit8 res8 = flgresult & 0xff;
	t_nubitcc count = 0;
	while(res8)
	{
		res8 &= res8-1; 
		count++;
	}
	SetPF(!(count%2));
}
void CalcAF()
{
	SetAF(((flgoperand1^flgoperand2)^flgresult)&0x10);
}
void CalcZF()
{
	SetZF(!flgresult);
}
void CalcSF()
{
	switch(flglen) {
	case 8:	SetSF(flgresult&0x0080);break;
	case 16:SetSF(flgresult&0x8000);break;
	default:break;}
}
void CalcTF() {}
void CalcIF() {}
void CalcDF() {}

static void SetFlags(t_nubit16 flags)
{
	if(flags & CF) CalcCF();
	if(flags & PF) CalcPF();
	if(flags & AF) CalcAF();
	if(flags & ZF) CalcZF();
	if(flags & SF) CalcSF();
	if(flags & TF) CalcTF();
	if(flags & IF) CalcIF();
	if(flags & DF) CalcDF();
	if(flags & OF) CalcOF();
}
static void GetMem()
{
	// returns rm
	rm = vmemoryGetAddress(insDS,vmemoryGetWord(vcpu.cs,vcpu.ip));
	vcpu.ip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns imm
	imm = vmemoryGetAddress(vcpu.cs,vcpu.ip);
	switch(immbit) {
	case 8:		vcpu.ip += 1;break;
	case 16:	vcpu.ip += 2;break;
	case 32:	vcpu.ip += 4;break;
	default:break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns rm and r
	t_nubit8 modrm = vmemoryGetByte(vcpu.cs,vcpu.ip++);
	rm = r = NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.si);break;
		case 1:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.di);break;
		case 2:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.si);break;
		case 3:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.di);break;
		case 4:	rm = vmemoryGetAddress(insDS,vcpu.si);break;
		case 5:	rm = vmemoryGetAddress(insDS,vcpu.di);break;
		case 6:	rm = vmemoryGetAddress(insDS,vmemoryGetWord(vcpu.cs,vcpu.ip));vcpu.ip += 2;break;
		case 7:	rm = vmemoryGetAddress(insDS,vcpu.bx);break;
		default:break;}
		break;
	case 1:
		switch(RM) {
		case 0:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.si);break;
		case 1:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.di);break;
		case 2:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.si);break;
		case 3:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.di);break;
		case 4:	rm = vmemoryGetAddress(insDS,vcpu.si);break;
		case 5:	rm = vmemoryGetAddress(insDS,vcpu.di);break;
		case 6:	rm = vmemoryGetAddress(insSS,vcpu.bp);break;
		case 7:	rm = vmemoryGetAddress(insDS,vcpu.bx);break;
		default:break;}
		rm += vmemoryGetByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;
		break;
	case 2:
		switch(RM) {
		case 0:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.si);break;
		case 1:	rm = vmemoryGetAddress(insDS,vcpu.bx+vcpu.di);break;
		case 2:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.si);break;
		case 3:	rm = vmemoryGetAddress(insSS,vcpu.bp+vcpu.di);break;
		case 4:	rm = vmemoryGetAddress(insDS,vcpu.si);break;
		case 5:	rm = vmemoryGetAddress(insDS,vcpu.di);break;
		case 6:	rm = vmemoryGetAddress(insSS,vcpu.bp);break;
		case 7:	rm = vmemoryGetAddress(insDS,vcpu.bx);break;
		default:break;}
		rm += vmemoryGetWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;
		break;
	case 3:
		switch(RM) {
		case 0:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.al); else rm = (t_vaddrcc)(&vcpu.ax); break;
		case 1:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.cl); else rm = (t_vaddrcc)(&vcpu.cx); break;
		case 2:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.dl); else rm = (t_vaddrcc)(&vcpu.dx); break;
		case 3:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.bl); else rm = (t_vaddrcc)(&vcpu.bx); break;
		case 4:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.ah); else rm = (t_vaddrcc)(&vcpu.sp); break;
		case 5:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.ch); else rm = (t_vaddrcc)(&vcpu.bp); break;
		case 6:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.dh); else rm = (t_vaddrcc)(&vcpu.si); break;
		case 7:	if(rmbit == 8) rm = (t_vaddrcc)(&vcpu.bh); else rm = (t_vaddrcc)(&vcpu.di); break;
		default:break;}
		break;
	default:break;}
	switch(regbit) {
	case 0:		r = REG;					break;
	case 4:
		switch(REG) {
		case 0:	r = (t_vaddrcc)(&vcpu.es);	break;
		case 1:	r = (t_vaddrcc)(&vcpu.cs);	break;
		case 2:	r = (t_vaddrcc)(&vcpu.ss);	break;
		case 3:	r = (t_vaddrcc)(&vcpu.ds);	break;
		default:break;}
		break;
	case 8:
		switch(REG) {
		case 0:	r = (t_vaddrcc)(&vcpu.al);	break;
		case 1:	r = (t_vaddrcc)(&vcpu.cl);	break;
		case 2:	r = (t_vaddrcc)(&vcpu.dl);	break;
		case 3:	r = (t_vaddrcc)(&vcpu.bl);	break;
		case 4:	r = (t_vaddrcc)(&vcpu.ah);	break;
		case 5:	r = (t_vaddrcc)(&vcpu.ch);	break;
		case 6:	r = (t_vaddrcc)(&vcpu.dh);	break;
		case 7:	r = (t_vaddrcc)(&vcpu.bh);	break;
		default:break;}
		break;
	case 16:
		switch(REG) {
		case 0: r = (t_vaddrcc)(&vcpu.ax);	break;
		case 1:	r = (t_vaddrcc)(&vcpu.cx);	break;
		case 2:	r = (t_vaddrcc)(&vcpu.dx);	break;
		case 3:	r = (t_vaddrcc)(&vcpu.bx);	break;
		case 4:	r = (t_vaddrcc)(&vcpu.sp);	break;
		case 5:	r = (t_vaddrcc)(&vcpu.bp);	break;
		case 6:	r = (t_vaddrcc)(&vcpu.si);	break;
		case 7: r = (t_vaddrcc)(&vcpu.di);	break;
		default:break;}
		break;
	default:break;}
	if(regbit) {

	} else r = REG;
}
static void GetModRegRMEA()
{
	// returns rm and r
	t_nubit8 modrm = vmemoryGetByte(vcpu.cs,vcpu.ip++);
	rm = r = NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	rm = vcpu.bx+vcpu.si;break;
		case 1:	rm = vcpu.bx+vcpu.di;break;
		case 2:	rm = vcpu.bp+vcpu.si;break;
		case 3:	rm = vcpu.bp+vcpu.di;break;
		case 4:	rm = vcpu.si;break;
		case 5:	rm = vcpu.di;break;
		case 6:	rm = vmemoryGetWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;break;
		case 7:	rm = vcpu.bx;break;
		default:break;}
		break;
	case 1:
		switch(RM) {
		case 0:	rm = vcpu.bx+vcpu.si;break;
		case 1:	rm = vcpu.bx+vcpu.di;break;
		case 2:	rm = vcpu.bp+vcpu.si;break;
		case 3:	rm = vcpu.bp+vcpu.di;break;
		case 4:	rm = vcpu.si;break;
		case 5:	rm = vcpu.di;break;
		case 6:	rm = vcpu.bp;break;
		case 7:	rm = vcpu.bx;break;
		default:break;}
		rm += vmemoryGetByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;
		break;
	case 2:
		switch(RM) {
		case 0:	rm = vcpu.bx+vcpu.si;break;
		case 1:	rm = vcpu.bx+vcpu.di;break;
		case 2:	rm = vcpu.bp+vcpu.si;break;
		case 3:	rm = vcpu.bp+vcpu.di;break;
		case 4:	rm = vcpu.si;break;
		case 5:	rm = vcpu.di;break;
		case 6:	rm = vmemoryGetAddress(insSS,vcpu.bp);break;
		case 7:	rm = vcpu.bx;break;
		default:break;}
		rm += vmemoryGetWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;
		break;
	default:break;}
	switch(REG) {
	case 0: r = (t_vaddrcc)(&vcpu.ax);	break;
	case 1:	r = (t_vaddrcc)(&vcpu.cx);	break;
	case 2:	r = (t_vaddrcc)(&vcpu.dx);	break;
	case 3:	r = (t_vaddrcc)(&vcpu.bx);	break;
	case 4:	r = (t_vaddrcc)(&vcpu.sp);	break;
	case 5:	r = (t_vaddrcc)(&vcpu.bp);	break;
	case 6:	r = (t_vaddrcc)(&vcpu.si);	break;
	case 7: r = (t_vaddrcc)(&vcpu.di);	break;
	default:break;}
}

static void ADD(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = ADD8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1+flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
	case 12:
		flglen = 16;
		flginstype = ADD16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1+flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = ADD16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1+flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(ADD_FLAG);
}
static void OR(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		//flginstype = OR8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1|flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		//flginstype = OR16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1|flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		//flginstype = OR16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1|flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetOF(0);
	SetCF(0);
	SetAF(0);
	SetFlags(OR_FLAG);
}
static void ADC(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = ADC8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1+flgoperand2+GetCF())&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		flginstype = ADC16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1+flgoperand2+GetCF())&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = ADC16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1+flgoperand2+GetCF())&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(ADC_FLAG);
}
static void SBB(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = SBB8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1-(flgoperand2+GetCF()))&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		flginstype = SBB16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1-(flgoperand2+GetCF()))&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = SBB16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1-(flgoperand2+GetCF()))&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(SBB_FLAG);
}
static void AND(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		//flginstype = AND8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		//flginstype = AND16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		//flginstype = AND16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetOF(0);
	SetCF(0);
	SetAF(0);
	SetFlags(AND_FLAG);
}
static void SUB(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = SUB8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1-flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		flginstype = SUB16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1-flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = SUB16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1-flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(SUB_FLAG);
}
static void XOR(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		//flginstype = XOR8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1^flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 12:
		flglen = 16;
		//flginstype = XOR16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nsbit8 *)src;
		flgresult = (flgoperand1^flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		//flginstype = XOR16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1^flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetOF(0);
	SetCF(0);
	SetAF(0);
	SetFlags(XOR_FLAG);
}
static void CMP(void *op1, void *op2, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = CMP8;
		flgoperand1 = *(t_nubit8 *)op1;
		flgoperand2 = *(t_nubit8 *)op2;
		flgresult = (flgoperand1-flgoperand2)&0xff;
		break;
	case 12:
		flglen = 16;
		flginstype = CMP16;
		flgoperand1 = *(t_nubit16 *)op1;
		flgoperand2 = *(t_nsbit8 *)op2;
		flgresult = (flgoperand1-flgoperand2)&0xffff;
		break;
	case 16:
		flglen = 16;
		flginstype = CMP16;
		flgoperand1 = *(t_nubit16 *)op1;
		flgoperand2 = *(t_nubit16 *)op2;
		flgresult = (flgoperand1-flgoperand2)&0xffff;
		break;
	default:break;}
	SetFlags(CMP_FLAG);
}
static void PUSH(void *src, t_nubitcc len)
{
	switch(len) {
	case 16:
		vcpu.sp -= 2;
		vmemorySetWord(vcpu.ss,vcpu.sp,*(t_nubit16 *)src);
		break;
	default:break;}
}
static void POP(void *dest, t_nubitcc len)
{
	switch(len) {
	case 16:
		*(t_nubit16 *)dest = vmemoryGetWord(vcpu.ss,vcpu.sp);
		vcpu.sp += 2;
		break;
	default:break;}
}
static void INC(void *dest, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = ADD8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = 0x01;
		flgresult = (flgoperand1+flgoperand2)&0x00ff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = ADD16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = 0x01;
		flgresult = (flgoperand1+flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(INC_FLAG);
}
static void DEC(void *dest, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = SUB8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = 0x01;
		flgresult = (flgoperand1-flgoperand2)&0x00ff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = SUB16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = 0x01;
		flgresult = (flgoperand1-flgoperand2)&0xffff;
		*(t_nubit16 *)dest = flgresult;
		break;
	default:break;}
	SetFlags(DEC_FLAG);
}
static void JCC(t_bool jflag,t_nubitcc len)
{
	switch(len) {
	case 8:
		if(jflag)
			vcpu.ip += vmemoryGetByte(vcpu.cs,vcpu.ip);
		vcpu.ip++;
		break;
	default:break;}
}
static void TEST(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		//flginstype = TEST8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xff;
		break;
	case 16:
		flglen = 16;
		//flginstype = TEST16;
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xffff;
		break;
	default:break;}
	SetOF(0);
	SetCF(0);
	SetAF(0);
	SetFlags(TEST_FLAG);
}
static void XCHG(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		*(t_nubit8 *)dest = flgoperand2;
		*(t_nubit8 *)src = flgoperand1;
		break;
	case 16:
		flgoperand1 = *(t_nubit16 *)dest;
		flgoperand2 = *(t_nubit16 *)src;
		*(t_nubit16 *)dest = flgoperand2;
		*(t_nubit16 *)src = flgoperand1;
		break;
	default:break;}
}
static void MOV(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		*(t_nubit8 *)dest = *(t_nubit8 *)src;
		break;
	case 16:
		*(t_nubit16 *)dest = *(t_nubit16 *)src;
		break;
	default:break;}
}

void OpError()
{
	nvmprint("The NVM CPU has encountered an illegal instruction.\nCS:");
	nvmprintword(vcpu.cs);
	nvmprint(" IP:");
	nvmprintword(vcpu.ip);
	nvmprint(" OP:");
	nvmprintbyte(vmemoryGetByte(vcpu.cs,vcpu.ip+0));
	nvmprint(" ");
	nvmprintbyte(vmemoryGetByte(vcpu.cs,vcpu.ip+1));
	nvmprint(" ");
	nvmprintbyte(vmemoryGetByte(vcpu.cs,vcpu.ip+2));
	nvmprint(" ");
	nvmprintbyte(vmemoryGetByte(vcpu.cs,vcpu.ip+3));
	nvmprint(" ");
	nvmprintbyte(vmemoryGetByte(vcpu.cs,vcpu.ip+4));
	nvmprint("\n");
	cpuTermFlag = 1;
}
void IO_NOP()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IO_NOP\n");}
void ADD_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_RM8_R8\n");
}
void ADD_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_RM16_R16\n");
}
void ADD_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_R8_RM8\n");
}
void ADD_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_R16_RM16\n");
}
void ADD_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADD((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_AL_I8\n");
}
void ADD_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADD((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADD_AX_I16\n");
}
void PUSH_ES()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.es,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_ES\n");
}
void POP_ES()
{
	vcpu.ip++;
	POP((void *)&vcpu.es,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_ES\n");
}
void OR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_RM8_R8\n");
}
void OR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_RM16_R16\n");
}
void OR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_R8_RM8\n");
}
void OR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_R16_RM16\n");
}
void OR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_AL_I8\n");
}
void OR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OR_AX_I16\n");
}
void PUSH_CS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cs,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_CS\n");
}
void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs,16);
	
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_CS\n");
}
/*void INS_0F()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_0F\n");}*/
void ADC_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_RM8_R8\n");
}
void ADC_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_RM16_R16\n");
}
void ADC_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_R8_RM8\n");
}
void ADC_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_R16_RM16\n");
}
void ADC_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADC((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_AL_I8\n");
}
void ADC_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADC((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ADC_AX_I16\n");
}
void PUSH_SS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ss,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_SS\n");
}
void POP_SS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ss,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_SS\n");
}
void SBB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_RM8_R8\n");
}
void SBB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_RM16_R16\n");
}
void SBB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_R8_RM8\n");
}
void SBB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_R16_RM16\n");
}
void SBB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_AL_I8\n");
}
void SBB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SBB_AX_I16\n");
}
void PUSH_DS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ds,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_DS\n");
}
void POP_DS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ds,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_DS\n");
}
void AND_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_RM8_R8\n");
}
void AND_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_RM16_R16\n");
}
void AND_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_R8_RM8\n");
}
void AND_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_R16_RM16\n");
}
void AND_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	AND((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_AL_I8\n");
}
void AND_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	AND((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AND_AX_I16\n");
}
void ES()
{
	vcpu.ip++;
	insDS = vcpu.es;
	insSS = vcpu.es;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  ES:\n");
}
void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetAF()) {
		vcpu.al = newAL;
		SetCF(GetCF() | ((newAL < oldAL) || (newAL < 0x06)));
	} else SetAF(0);
	if(((vcpu.al & 0xf0) > 0x90) || GetCF()) {
		vcpu.al += 0x60;
		SetCF(1);
	} else SetCF(0);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DAA\n");
}
void SUB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_RM8_R8\n");
}
void SUB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_RM16_R16\n");
}
void SUB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_R8_RM8\n");
}
void SUB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_R16_RM16\n");
}
void SUB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_AL_I8\n");
}
void SUB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SUB_AX_I16\n");
}
void CS()
{
	vcpu.ip++;
	insDS = vcpu.cs;
	insSS = vcpu.cs;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CS:\n");
}
void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || GetAF()) {
		vcpu.al -= 0x06;
		SetCF(GetCF() | (oldAL < 0x06));
		SetAF(1);
	} else SetAF(0);
	if((vcpu.al > 0x9f) || GetCF()) {
		vcpu.al -= 0x60;
		SetCF(1);
	} else SetCF(0);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DAS\n");
}
void XOR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_RM8_R8\n");
}
void XOR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_RM16_R16\n");
}
void XOR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_R8_RM8\n");
}
void XOR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_R16_RM16\n");
}
void XOR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_AL_I8\n");
}
void XOR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XOR_AX_I16\n");
}
void SS()
{
	vcpu.ip++;
	insDS = vcpu.ss;
	insSS = vcpu.ss;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SS:\n");
}
void AAA()
{
	vcpu.ip++;
	if(((vcpu.al&0x0f) > 0x09) || GetAF()) {
		vcpu.al += 0x06;
		vcpu.ah += 0x01;
		SetAF(1);
		SetCF(1);
	} else {
		SetAF(0);
		SetCF(0);
	}
	vcpu.al &= 0x0f;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AAA\n");
}
void CMP_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_RM8_R8\n");
}
void CMP_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_RM16_R16\n");
}
void CMP_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_R8_RM8\n");
}
void CMP_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_R16_RM16\n");
}
void CMP_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_AL_I8\n");
}
void CMP_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMP_AX_I16\n");
}
void DS()
{
	vcpu.ip++;
	insDS = vcpu.ds;
	insSS = vcpu.ds;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DS:\n");
}
void AAS()
{
	vcpu.ip++;
	if(((vcpu.al&0x0f) > 0x09) || GetAF()) {
		vcpu.al -= 0x06;
		vcpu.ah += 0x01;
		SetAF(1);
		SetCF(1);
	} else {
		SetCF(0);
		SetAF(0);
	}
	vcpu.al &= 0x0f;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AAS\n");
}
void INC_AX()
{
	vcpu.ip++;
	INC((void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_AX\n");
}
void INC_CX()
{
	vcpu.ip++;
	INC((void *)&vcpu.cx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_CX\n");
}
void INC_DX()
{
	vcpu.ip++;
	INC((void *)&vcpu.dx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_DX\n");
}
void INC_BX()
{
	vcpu.ip++;
	INC((void *)&vcpu.bx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_BX\n");
}
void INC_SP()
{
	vcpu.ip++;
	INC((void *)&vcpu.sp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_SP\n");
}
void INC_BP()
{
	vcpu.ip++;
	INC((void *)&vcpu.bp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_BP\n");
}
void INC_SI()
{
	vcpu.ip++;
	INC((void *)&vcpu.si,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_SI\n");
}
void INC_DI()
{
	vcpu.ip++;
	INC((void *)&vcpu.di,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INC_DI\n");
}
void DEC_AX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_AX\n");
}
void DEC_CX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.cx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_CX\n");
}
void DEC_DX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.dx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_DX\n");
}
void DEC_BX()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_BX\n");
}
void DEC_SP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.sp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_SP\n");
}
void DEC_BP()
{
	vcpu.ip++;
	DEC((void *)&vcpu.bp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_BP\n");
}
void DEC_SI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.si,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_SI\n");
}
void DEC_DI()
{
	vcpu.ip++;
	DEC((void *)&vcpu.di,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  DEC_DI\n");
}
void PUSH_AX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_AX\n");
}
void PUSH_CX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_CX\n");
}
void PUSH_DX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.dx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_DX\n");
}
void PUSH_BX()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_BX\n");
}
void PUSH_SP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.sp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_SP\n");
}
void PUSH_BP()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.bp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_BP\n");
}
void PUSH_SI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.si,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_SI\n");
}
void PUSH_DI()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.di,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_DI\n");
}
void POP_AX()
{
	vcpu.ip++;
	POP((void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_AX\n");
}
void POP_CX()
{
	vcpu.ip++;
	POP((void *)&vcpu.cx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_CX\n");
}
void POP_DX()
{
	vcpu.ip++;
	POP((void *)&vcpu.dx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_DX\n");
}
void POP_BX()
{
	vcpu.ip++;
	POP((void *)&vcpu.bx,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_BX\n");
}
void POP_SP()
{
	vcpu.ip++;
	POP((void *)&vcpu.sp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_SP\n");
}
void POP_BP()
{
	vcpu.ip++;
	POP((void *)&vcpu.bp,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_BP\n");
}
void POP_SI()
{
	vcpu.ip++;
	POP((void *)&vcpu.si,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_SI\n");
}
void POP_DI()
{
	vcpu.ip++;
	POP((void *)&vcpu.di,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_DI\n");
}
/*void OpdSize()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OpdSize\n");}
void AddrSize()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AddrSize\n");}
void PUSH_I16()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSH_I16\n");}*/
void JO()
{
	vcpu.ip++;
	JCC(GetOF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JO\n");
}
void JNO()
{
	vcpu.ip++;
	JCC(!GetOF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNO\n");
}
void JC()
{
	vcpu.ip++;
	JCC(GetCF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JC\n");
}
void JNC()
{
	vcpu.ip++;
	JCC(!GetCF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNC\n");
}
void JZ()
{
	vcpu.ip++;
	JCC(GetZF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JZ\n");
}
void JNZ()
{
	vcpu.ip++;
	JCC(!GetZF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNZ\n");
}
void JBE()
{
	vcpu.ip++;
	JCC((GetCF() || GetZF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JBE\n");
}
void JA()
{
	vcpu.ip++;
	JCC((!GetCF() && !GetZF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JA\n");
}
void JS()
{
	vcpu.ip++;
	JCC(GetSF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JS\n");
}
void JNS()
{
	vcpu.ip++;
	JCC(!GetSF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNS\n");
}
void JP()
{
	vcpu.ip++;
	JCC(GetPF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JP\n");
}
void JNP()
{
	vcpu.ip++;
	JCC(!GetPF(),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNP\n");
}
void JL()
{
	vcpu.ip++;
	JCC((GetSF() != GetOF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JL\n");
}
void JNL()
{
	vcpu.ip++;
	JCC((GetSF() == GetOF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JNL\n");
}
void JLE()
{
	vcpu.ip++;
	JCC(((GetSF() != GetOF()) || GetZF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JLE\n");}
void JG()
{
	vcpu.ip++;
	JCC(((GetSF() == GetOF()) && !GetZF()),8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JG\n");}
void INS_80()
{
	vcpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(r) {
	case 0:	ADD((void *)rm,(void *)imm,8);break;
	case 1:	OR ((void *)rm,(void *)imm,8);break;
	case 2:	ADC((void *)rm,(void *)imm,8);break;
	case 3:	SBB((void *)rm,(void *)imm,8);break;
	case 4:	AND((void *)rm,(void *)imm,8);break;
	case 5:	SUB((void *)rm,(void *)imm,8);break;
	case 6:	XOR((void *)rm,(void *)imm,8);break;
	case 7:	CMP((void *)rm,(void *)imm,8);break;
	default:break;}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_80\n");
}
void INS_81()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch(r) {
	case 0:	ADD((void *)rm,(void *)imm,16);break;
	case 1:	OR ((void *)rm,(void *)imm,16);break;
	case 2:	ADC((void *)rm,(void *)imm,16);break;
	case 3:	SBB((void *)rm,(void *)imm,16);break;
	case 4:	AND((void *)rm,(void *)imm,16);break;
	case 5:	SUB((void *)rm,(void *)imm,16);break;
	case 6:	XOR((void *)rm,(void *)imm,16);break;
	case 7:	CMP((void *)rm,(void *)imm,16);break;
	default:break;}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_81\n");
}
void INS_82()
{
	INS_80();
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_82\n");
}
void INS_83()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(r) {
	case 0:	ADD((void *)rm,(void *)imm,12);break;
	case 1:	OR ((void *)rm,(void *)imm,12);break;
	case 2:	ADC((void *)rm,(void *)imm,12);break;
	case 3:	SBB((void *)rm,(void *)imm,12);break;
	case 4:	AND((void *)rm,(void *)imm,12);break;
	case 5:	SUB((void *)rm,(void *)imm,12);break;
	case 6:	XOR((void *)rm,(void *)imm,12);break;
	case 7:	CMP((void *)rm,(void *)imm,12);break;
	default:break;}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_83\n");
}
void TEST_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  TEST_RM8_R8\n");
}
void TEST_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  TEST_RM16_R16\n");
}
void XCHG_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	XCHG((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_R8_RM8\n");
}
void XCHG_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	XCHG((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_R16_RM16\n");
}
void MOV_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)rm,(void *)r,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_RM8_R8\n");
}
void MOV_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_RM16_R16\n");
}
void MOV_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)r,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_R8_RM8\n");
}
void MOV_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_R16_RM16\n");
}
void MOV_RM16_SEG()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)rm,(void *)r,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_RM16_SEG\n");
}
void LEA_R16_M16()
{
	vcpu.ip++;
	GetModRegRMEA();
	*(t_nubit16 *)r = rm&0xffff;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LEA_R16_M16\n");
}
void MOV_SEG_RM16()
{
	vcpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)r,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_SEG_RM16\n");
}
void POP_RM16()
{
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(r) {
	case 0:	POP((void *)rm,16);
	default:break;}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POP_RM16\n");
}
void NOP()
{
	vcpu.ip++;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  NOP\n");
}
void XCHG_CX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.cx,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_CX_AX\n");
}
void XCHG_DX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.dx,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_DX_AX\n");
}
void XCHG_BX_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bx,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_BX_AX\n");
}
void XCHG_SP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.sp,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_SP_AX\n");
}
void XCHG_BP_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.bp,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_BP_AX\n");
}
void XCHG_SI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.si,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_SI_AX\n");
}
void XCHG_DI_AX()
{
	vcpu.ip++;
	XCHG((void *)&vcpu.di,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XCHG_DI_AX\n");
}
void CBW()
{
	vcpu.ip++;
	vcpu.ax = (t_nsbit8)vcpu.al;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CBW\n");}
void CWD()
{
	vcpu.ip++;
	if(vcpu.ax&0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CWD\n");}
void CALL_FAR()
{
	t_nubit16 tvcs = vcpu.cs;
	t_nubit16 tvip = ++vcpu.ip;
	vcpu.ip += 4;
	PUSH((void *)&vcpu.cs,16);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip = vmemoryGetWord(tvcs,tvip);
	tvip += 2;
	vcpu.cs = vmemoryGetWord(tvcs,tvip);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CALL_FAR\n");
}
void WAIT()
{
	vcpu.ip++;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  WAIT\n");
}
void PUSHF()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.flags,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  PUSHF\n");
}
void POPF()
{
	vcpu.ip++;
	POP((void *)&vcpu.flags,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  POPF\n");}
void SAHF()
{
	vcpu.ip++;
	*(t_nubit8 *)&vcpu.flags = vcpu.ah;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SAHF\n");
}
void LAHF()
{
	vcpu.ip++;
	vcpu.ah = *(t_nubit8 *)&vcpu.flags;
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LAHF\n");
}
void MOV_AL_M8()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.al,(void *)rm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_AL_M8\n");
}
void MOV_AX_M16()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)rm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_AX_M16\n");
}
void MOV_M8_AL()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)rm,(void *)&vcpu.al,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_M8_AL\n");
}
void MOV_M16_AX()
{
	vcpu.ip++;
	GetMem();
	MOV((void *)rm,(void *)&vcpu.ax,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_M16_AX\n");}
void MOVSB()
{
	vcpu.ip++;
	vmemorySetByte(vcpu.es,vcpu.di,vmemoryGetByte(insDS,vcpu.si));
	if(GetDF()) {
		vcpu.di--;
		vcpu.si--;
	} else {
		vcpu.di++;
		vcpu.si++;
	}
	/*if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
		WriteVideoRam(eCPU.di+t-0xa0000);*/
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOVSB\n");
}
void MOVSW()
{
	vcpu.ip++;
	vmemorySetWord(vcpu.es,vcpu.di,vmemoryGetWord(insDS,vcpu.si));
	if(GetDF()) {
		vcpu.di -= 2;
		vcpu.si -= 2;
	} else {
		vcpu.di += 2;
		vcpu.si += 2;
	}
	/*if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000)
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
		}
	}*/
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOVSW\n");
}
void CMPSB()
{
	vcpu.ip++;
	flglen = 8;
	flginstype = CMP8;
	flgoperand1 = vmemoryGetByte(insDS,vcpu.si);
	flgoperand2 = vmemoryGetByte(vcpu.es,vcpu.di);
	flgresult = (flgoperand1-flgoperand2)&0xff;
	if(GetDF()) {
		vcpu.di--;
		vcpu.si--;
	} else {
		vcpu.di++;
		vcpu.si++;
	}
	SetFlags(CMP_FLAG);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMPSB\n");
}
void CMPSW()
{
	vcpu.ip++;
	flglen = 16;
	flginstype = CMP16;
	flgoperand1 = vmemoryGetWord(insDS,vcpu.si);
	flgoperand2 = vmemoryGetWord(vcpu.es,vcpu.di);
	flgresult = (flgoperand1-flgoperand2)&0xffff;
	if(GetDF()) {
		vcpu.di -= 2;
		vcpu.si -= 2;
	} else {
		vcpu.di += 2;
		vcpu.si += 2;
	}
	SetFlags(CMP_FLAG);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMPSW\n");
}
void TEST_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	TEST((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  TEST_AL_I8\n");
}
void TEST_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  TEST_AX_I16\n");
}
void STOSB()
{
	vcpu.ip++;
	vmemorySetByte(vcpu.es,vcpu.di,vcpu.al);
	if(GetDF()) {
		vcpu.di--;
	} else {
		vcpu.di++;
	}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  STOSB\n");
}
void STOSW()
{
	vcpu.ip++;
	vmemorySetWord(vcpu.es,vcpu.di,vcpu.ax);
	if(GetDF()) {
		vcpu.di -= 2;
	} else {
		vcpu.di += 2;
	}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  STOSW\n");
}
void LODSB()
{
	vcpu.ip++;
	vcpu.al = vmemoryGetByte(insDS,vcpu.si);
	if(GetDF()) {
		vcpu.si--;
	} else {
		vcpu.si++;
	}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LODSB\n");}
void LODSW()
{
	vcpu.ip++;
	vcpu.ax = vmemoryGetWord(insDS,vcpu.si);
	if(GetDF()) {
		vcpu.si -= 2;
	} else {
		vcpu.si += 2;
	}
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LODSW\n");}
void SCASB()
{
	vcpu.ip++;
	flglen = 8;
	flginstype = CMP8;
	flgoperand1 = vcpu.al;
	flgoperand2 = vmemoryGetByte(vcpu.es,vcpu.di);
	flgresult = (flgoperand1-flgoperand2)&0x00ff;
	if(GetDF()) {
		vcpu.di--;
	} else {
		vcpu.di++;
	}
	SetFlags(CMP_FLAG);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SCASB\n");}
void SCASW()
{
	vcpu.ip++;
	flglen = 16;
	flginstype = CMP16;
	flgoperand1 = vcpu.ax;
	flgoperand2 = vmemoryGetByte(vcpu.es,vcpu.di);
	flgresult = (flgoperand1-flgoperand2)&0xffff;
	if(GetDF()) {
		vcpu.di -= 2;
	} else {
		vcpu.di += 2;
	}
	SetFlags(CMP_FLAG);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  SCASW\n");
}
void MOV_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.al,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_AL_I8\n");
}
void MOV_CL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_CL_I8\n");
}
void MOV_DL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_DL_I8\n");
}
void MOV_BL_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_BL_I8\n");
}
void MOV_AH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_AH_I8\n");
}
void MOV_CH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_CH_I8\n");
}
void MOV_DH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_DH_I8\n");
}
void MOV_BH_I8()
{
	vcpu.ip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_BH_I8\n");}
void MOV_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_AX_I16\n");
}
void MOV_CX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_CX_I16\n");
}
void MOV_DX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_DX_I16\n");
}
void MOV_BX_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_BX_I16\n");
}
void MOV_SP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_SP_I16\n");
}
void MOV_BP_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_BP_I16\n");
}
void MOV_SI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_SI_I16\n");
}
void MOV_DI_I16()
{
	vcpu.ip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_DI_I16\n");
}
/*
void INS_C0()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_C0\n");}
void INS_C1()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_C1\n");}
*/
void RET_I8()
{
	t_nubit16 tvip = ++vcpu.ip;
	POP((void *)&vcpu.ip,16);
	vcpu.sp += vmemoryGetByte(vcpu.cs,tvip);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  RET_I8\n");
}
void RET_NEAR()
{
	POP((void *)&vcpu.ip,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  RET_NEAR\n");
}
void LES_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)r,(void *)rm,16);
	MOV((void *)&vcpu.es,(void *)(rm+2),16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LES_R16_M16\n");
}
void LDS_R16_M16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)r,(void *)rm,16);
	MOV((void *)&vcpu.ds,(void *)(rm+2),16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LDS_R16_M16\n");
}
void MOV_M8_I8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)rm,(void *)imm,8);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_M8_I8\n");
}
void MOV_M16_I16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)rm,(void *)imm,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  MOV_M16_I16\n");
}
void RET_I16()
{
	t_nubit16 tvcs = vcpu.cs;
	t_nubit16 tvip = ++vcpu.ip;
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	vcpu.sp += vmemoryGetWord(tvcs,tvip);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  RET_I16\n");
}
void RET_FAR()
{
	POP((void *)&vcpu.ip,16);
	POP((void *)&vcpu.cs,16);
	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  RET_FAR\n");
}

// TODO
void INT3()
{

	nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INT3\n");
}
void INT_I8()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INT_I8\n");}
void INTO()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INTO\n");}
void IRET()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IRET\n");}
void INS_D0()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_D0\n");}
void INS_D1()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_D1\n");}
void INS_D2()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_D2\n");}
void INS_D3()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_D3\n");}
void AAM()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AAM\n");}
void AAD()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  AAD\n");}
void XLAT()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  XLAT\n");}
void INS_D9()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_D9\n");}
void INS_DB()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_DB\n");}
void LOOPNE()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LOOPNE\n");}
void LOOPE()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LOOPE\n");}
void LOOP_NEAR()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LOOP_NEAR\n");}
void JCXZ_NEAR()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JCXZ_NEAR\n");}
void IN_AL_N()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IN_AL_N\n");}
void IN_AX_N()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IN_AX_N\n");}
void OUT_N_AL()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OUT_N_AL\n");}
void OUT_N_AX()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OUT_N_AX\n");}
void CALL_NEAR()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CALL_NEAR\n");}
void JMP_NEAR_LABEL()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JMP_NEAR_LABEL\n");}
void JMP_FAR_LABEL()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JMP_FAR_LABEL\n");}
void JMP_NEAR()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  JMP_NEAR\n");}
void IN_AL_DX()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IN_AL_DX\n");}
void IN_AX_DX()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  IN_AX_DX\n");}
void OUT_DX_AL()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OUT_DX_AL\n");}
void OUT_DX_AX()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  OUT_DX_AX\n");}
void LOCK()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  LOCK\n");}
void REPNE()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  REPNE\n");}
void REP()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  REP\n");}
void HLT()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  HLT\n");}
void CMC()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CMC\n");}
void INS_F6()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_F6\n");}
void INS_F7()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_F7\n");}
void CLC()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CLC\n");}
void STC()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  STC\n");}
void CLI()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CLI\n");}
void STI()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  STI\n");}
void CLD()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  CLD\n");}
void STD()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  STD\n");}
void INS_FE()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_FE\n");}
void INS_FF()
{nvmprintword(vcpu.cs);nvmprint(":");nvmprintword(vcpu.ip);nvmprint("  INS_FF\n");}

t_bool vcpuinsIsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 1;break;
	default:	return 0;break;
	}
}
void vcpuinsSB()
{
	insDS = vcpu.ds;
	insSS = vcpu.ss;
}

void CPUInsInit()
{
	int i;
	vcpuinsSB();
	for(i = 0;i < 0x10000;++i) {
		InTable[i] = IO_NOP;
		OutTable[i] = IO_NOP;
	}
	InsTable[0x00] = ADD_RM8_R8;
	InsTable[0x01] = ADD_RM16_R16;
	InsTable[0x02] = ADD_R8_RM8;
	InsTable[0x03] = ADD_R16_RM16;
	InsTable[0x04] = ADD_AL_I8;
	InsTable[0x05] = ADD_AX_I16;
	InsTable[0x06] = PUSH_ES;
	InsTable[0x07] = POP_ES;
	InsTable[0x08] = OR_RM8_R8;
	InsTable[0x09] = OR_RM16_R16;
	InsTable[0x0a] = OR_R8_RM8;
	InsTable[0x0b] = OR_R16_RM16;
	InsTable[0x0c] = OR_AL_I8;
	InsTable[0x0d] = OR_AX_I16;
	InsTable[0x0e] = PUSH_CS;
	InsTable[0x0f] = POP_CS;
	//InsTable[0x0f] = INS_0F;
	InsTable[0x10] = ADC_RM8_R8;
	InsTable[0x11] = ADC_RM16_R16;
	InsTable[0x12] = ADC_R8_RM8;
	InsTable[0x13] = ADC_R16_RM16;
	InsTable[0x14] = ADC_AL_I8;
	InsTable[0x15] = ADC_AX_I16;
	InsTable[0x16] = PUSH_SS;
	InsTable[0x17] = POP_SS;
	InsTable[0x18] = SBB_RM8_R8;
	InsTable[0x19] = SBB_RM16_R16;
	InsTable[0x1a] = SBB_R8_RM8;
	InsTable[0x1b] = SBB_R16_RM16;
	InsTable[0x1c] = SBB_AL_I8;
	InsTable[0x1d] = SBB_AX_I16;
	InsTable[0x1e] = PUSH_DS;
	InsTable[0x1f] = POP_DS;
	InsTable[0x20] = AND_RM8_R8;
	InsTable[0x21] = AND_RM16_R16;
	InsTable[0x22] = AND_R8_RM8;
	InsTable[0x23] = AND_R16_RM16;
	InsTable[0x24] = AND_AL_I8;
	InsTable[0x25] = AND_AX_I16;
	InsTable[0x26] = ES;
	InsTable[0x27] = DAA;
	InsTable[0x28] = SUB_RM8_R8;
	InsTable[0x29] = SUB_RM16_R16;
	InsTable[0x2a] = SUB_R8_RM8;
	InsTable[0x2b] = SUB_R16_RM16;
	InsTable[0x2c] = SUB_AL_I8;
	InsTable[0x2d] = SUB_AX_I16;
	InsTable[0x2e] = CS;
	InsTable[0x2f] = DAS;
	InsTable[0x30] = XOR_RM8_R8;
	InsTable[0x31] = XOR_RM16_R16;
	InsTable[0x32] = XOR_R8_RM8;
	InsTable[0x33] = XOR_R16_RM16;
	InsTable[0x34] = XOR_AL_I8;
	InsTable[0x35] = XOR_AX_I16;
	InsTable[0x36] = SS;
	InsTable[0x37] = AAA;
	InsTable[0x38] = CMP_RM8_R8;
	InsTable[0x39] = CMP_RM16_R16;
	InsTable[0x3a] = CMP_R8_RM8;
	InsTable[0x3b] = CMP_R16_RM16;
	InsTable[0x3c] = CMP_AL_I8;
	InsTable[0x3d] = CMP_AX_I16;
	InsTable[0x3e] = DS;
	InsTable[0x3f] = AAS;
	InsTable[0x40] = INC_AX;
	InsTable[0x41] = INC_CX;
	InsTable[0x42] = INC_DX;
	InsTable[0x43] = INC_BX;
	InsTable[0x44] = INC_SP;
	InsTable[0x45] = INC_BP;
	InsTable[0x46] = INC_SI;
	InsTable[0x47] = INC_DI;
	InsTable[0x48] = DEC_AX;
	InsTable[0x49] = DEC_CX;
	InsTable[0x4a] = DEC_DX;
	InsTable[0x4b] = DEC_BX;
	InsTable[0x4c] = DEC_SP;
	InsTable[0x4d] = DEC_BP;
	InsTable[0x4e] = DEC_SI;
	InsTable[0x4f] = DEC_DI;
	InsTable[0x50] = PUSH_AX;
	InsTable[0x51] = PUSH_CX;
	InsTable[0x52] = PUSH_DX;
	InsTable[0x53] = PUSH_BX;
	InsTable[0x54] = PUSH_SP;
	InsTable[0x55] = PUSH_BP;
	InsTable[0x56] = PUSH_SI;
	InsTable[0x57] = PUSH_DI;
	InsTable[0x58] = POP_AX;
	InsTable[0x59] = POP_CX;
	InsTable[0x5a] = POP_DX;
	InsTable[0x5b] = POP_BX;
	InsTable[0x5c] = POP_SP;
	InsTable[0x5d] = POP_BP;
	InsTable[0x5e] = POP_SI;
	InsTable[0x5f] = POP_DI;
	InsTable[0x60] = OpError;
	InsTable[0x61] = OpError;
	InsTable[0x62] = OpError;
	InsTable[0x63] = OpError;
	InsTable[0x64] = OpError;
	InsTable[0x65] = OpError;
	InsTable[0x66] = OpError;
	InsTable[0x67] = OpError;
	InsTable[0x68] = OpError;
	//InsTable[0x66] = OpdSize;
	//InsTable[0x67] = AddrSize;
	//InsTable[0x68] = PUSH_I16;
	InsTable[0x69] = OpError;
	InsTable[0x6a] = OpError;
	InsTable[0x6b] = OpError;
	InsTable[0x6c] = OpError;
	InsTable[0x6d] = OpError;
	InsTable[0x6e] = OpError;
	InsTable[0x6f] = OpError;
	InsTable[0x70] = JO;
	InsTable[0x71] = JNO;
	InsTable[0x72] = JC;
	InsTable[0x73] = JNC;
	InsTable[0x74] = JZ;
	InsTable[0x75] = JNZ;
	InsTable[0x76] = JBE;
	InsTable[0x77] = JA;
	InsTable[0x78] = JS;
	InsTable[0x79] = JNS;
	InsTable[0x7a] = JP;
	InsTable[0x7b] = JNP;
	InsTable[0x7c] = JL;
	InsTable[0x7d] = JNL;
	InsTable[0x7e] = JLE;
	InsTable[0x7f] = JG;
	InsTable[0x80] = INS_80;
	InsTable[0x81] = INS_81;
	InsTable[0x82] = INS_82;
	InsTable[0x83] = INS_83;
	InsTable[0x84] = TEST_RM8_R8;
	InsTable[0x85] = TEST_RM16_R16;
	InsTable[0x86] = XCHG_R8_RM8;
	InsTable[0x87] = XCHG_R16_RM16;
	InsTable[0x88] = MOV_RM8_R8;
	InsTable[0x89] = MOV_RM16_R16;
	InsTable[0x8a] = MOV_R8_RM8;
	InsTable[0x8b] = MOV_R16_RM16;
	InsTable[0x8c] = MOV_RM16_SEG;
	InsTable[0x8d] = LEA_R16_M16;
	InsTable[0x8e] = MOV_SEG_RM16;
	InsTable[0x8f] = POP_RM16;
	InsTable[0x90] = NOP;
	InsTable[0x91] = XCHG_CX_AX;
	InsTable[0x92] = XCHG_DX_AX;
	InsTable[0x93] = XCHG_BX_AX;
	InsTable[0x94] = XCHG_SP_AX;
	InsTable[0x95] = XCHG_BP_AX;
	InsTable[0x96] = XCHG_SI_AX;
	InsTable[0x97] = XCHG_DI_AX;
	InsTable[0x98] = CBW;
	InsTable[0x99] = CWD;
	InsTable[0x9a] = CALL_FAR;
	InsTable[0x9b] = WAIT;
	InsTable[0x9c] = PUSHF;
	InsTable[0x9d] = POPF;
	InsTable[0x9e] = SAHF;
	InsTable[0x9f] = LAHF;
	InsTable[0xa0] = MOV_AL_M8;
	InsTable[0xa1] = MOV_AX_M16;
	InsTable[0xa2] = MOV_M8_AL;
	InsTable[0xa3] = MOV_M16_AX;
	InsTable[0xa4] = MOVSB;
	InsTable[0xa5] = MOVSW;
	InsTable[0xa6] = CMPSB;
	InsTable[0xa7] = CMPSW;
	InsTable[0xa8] = TEST_AL_I8;
	InsTable[0xa9] = TEST_AX_I16;
	InsTable[0xaa] = STOSB;
	InsTable[0xab] = STOSW;
	InsTable[0xac] = LODSB;
	InsTable[0xad] = LODSW;
	InsTable[0xae] = SCASB;
	InsTable[0xaf] = SCASW;
	InsTable[0xb0] = MOV_AL_I8;
	InsTable[0xb1] = MOV_CL_I8;
	InsTable[0xb2] = MOV_DL_I8;
	InsTable[0xb3] = MOV_BL_I8;
	InsTable[0xb4] = MOV_AH_I8;
	InsTable[0xb5] = MOV_CH_I8;
	InsTable[0xb6] = MOV_DH_I8;
	InsTable[0xb7] = MOV_BH_I8;
	InsTable[0xb8] = MOV_AX_I16;
	InsTable[0xb9] = MOV_CX_I16;
	InsTable[0xba] = MOV_DX_I16;
	InsTable[0xbb] = MOV_BX_I16;
	InsTable[0xbc] = MOV_SP_I16;
	InsTable[0xbd] = MOV_BP_I16;
	InsTable[0xbe] = MOV_SI_I16;
	InsTable[0xbf] = MOV_DI_I16;
	InsTable[0xc0] = OpError;
	InsTable[0xc1] = OpError;
	InsTable[0xc2] = RET_I8;
	InsTable[0xc3] = RET_NEAR;
	InsTable[0xc4] = LES_R16_M16;
	InsTable[0xc5] = LDS_R16_M16;
	InsTable[0xc6] = MOV_M8_I8;
	InsTable[0xc7] = MOV_M16_I16;
	InsTable[0xc8] = OpError;
	InsTable[0xc9] = OpError;
	InsTable[0xca] = RET_I16;
	InsTable[0xcb] = RET_FAR;
	InsTable[0xcc] = INT3;
	InsTable[0xcd] = INT_I8;
	InsTable[0xce] = INTO;
	InsTable[0xcf] = IRET;
	InsTable[0xd0] = INS_D0;
	InsTable[0xd1] = INS_D1;
	InsTable[0xd2] = INS_D2;
	InsTable[0xd3] = INS_D3;
	InsTable[0xd4] = AAM;
	InsTable[0xd5] = AAD;
	InsTable[0xd6] = OpError;
	InsTable[0xd7] = XLAT;
	InsTable[0xd8] = OpError;
	InsTable[0xd9] = INS_D9;
	InsTable[0xda] = OpError;
	InsTable[0xdb] = INS_DB;
	InsTable[0xdc] = OpError;
	InsTable[0xdd] = OpError;
	InsTable[0xde] = OpError;
	InsTable[0xdf] = OpError;
	InsTable[0xe0] = LOOPNE;
	InsTable[0xe1] = LOOPE;
	InsTable[0xe2] = LOOP_NEAR;
	InsTable[0xe3] = JCXZ_NEAR;
	InsTable[0xe4] = IN_AL_N;
	InsTable[0xe5] = IN_AX_N;
	InsTable[0xe6] = OUT_N_AL;
	InsTable[0xe7] = OUT_N_AX;
	InsTable[0xe8] = CALL_NEAR;
	InsTable[0xe9] = JMP_NEAR_LABEL;
	InsTable[0xea] = JMP_FAR_LABEL;
	InsTable[0xeb] = JMP_NEAR;
	InsTable[0xec] = IN_AL_DX;
	InsTable[0xed] = IN_AX_DX;
	InsTable[0xee] = OUT_DX_AL;
	InsTable[0xef] = OUT_DX_AX;
	InsTable[0xf0] = LOCK;
	InsTable[0xf1] = OpError;
	InsTable[0xf2] = REPNE;
	InsTable[0xf3] = REP;
	InsTable[0xf4] = HLT;
	InsTable[0xf5] = CMC;
	InsTable[0xf6] = INS_F6;
	InsTable[0xf7] = INS_F7;
	InsTable[0xf8] = CLC;
	InsTable[0xf9] = STC;
	InsTable[0xfa] = CLI;
	InsTable[0xfb] = STI;
	InsTable[0xfc] = CLD;
	InsTable[0xfd] = STD;
	InsTable[0xfe] = INS_FE;
	InsTable[0xff] = INS_FF;
}
void CPUInsTerm()
{}