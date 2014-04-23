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

static t_nubitcc flgoperand1,flgoperand2,flgresult,flglen;
static enum {
	ADD8,ADD16,
	OR8,OR16,
	ADC8,ADC16,
	SBB8,SBB16
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

t_bool GetCF() {return !!(vcpu.flags & CF);}
t_bool GetPF() {return !!(vcpu.flags & PF);}
t_bool GetAF() {return !!(vcpu.flags & AF);}
t_bool GetZF() {return !!(vcpu.flags & ZF);}
t_bool GetSF() {return !!(vcpu.flags & SF);}
t_bool GetTF() {return !!(vcpu.flags & TF);}
t_bool GetIF() {return !!(vcpu.flags & IF);}
t_bool GetDF() {return !!(vcpu.flags & DF);}
t_bool GetOF() {return !!(vcpu.flags & OF);}

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
		SetOF(((flgoperand1&0x0080) != (flgoperand2&0x0080)) && ((flgoperand2&0x0080) == (flgresult&0x0080)));
		break;
	case SBB16:
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
static void GetImm(t_nubitcc immbit)
{
	imm = memoryBase+SHL4(vcpu.cs)+(vcpu.ip);
	switch(immbit) {
	case 8:		vcpu.ip += 1;break;
	case 16:	vcpu.ip += 2;break;
	case 32:	vcpu.ip += 4;break;
	default:	break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	t_nubit8 modrm = vmemoryGetByte(vcpu.cs,vcpu.ip++);
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.si;break;
		case 1:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.di;break;
		case 2:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.si;break;
		case 3:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.di;break;
		case 4:	rm = memoryBase+SHL4(insDS)+vcpu.si;break;
		case 5:	rm = memoryBase+SHL4(insDS)+vcpu.di;break;
		case 6:	rm = memoryBase+SHL4(insDS)+vmemoryGetWord(vcpu.cs,vcpu.ip);vcpu.ip += 2;break;
		case 7:	rm = memoryBase+SHL4(insDS)+vcpu.bx;break;
		default:break;}
		break;
	case 1:
		switch(RM) {
		case 0:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.si;break;
		case 1:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.di;break;
		case 2:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.si;break;
		case 3:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.di;break;
		case 4:	rm = memoryBase+SHL4(insDS)+vcpu.si;break;
		case 5:	rm = memoryBase+SHL4(insDS)+vcpu.di;break;
		case 6:	rm = memoryBase+SHL4(insSS)+vcpu.bp;break;
		case 7:	rm = memoryBase+SHL4(insDS)+vcpu.bx;break;
		default:break;}
		rm += vmemoryGetByte(vcpu.cs,vcpu.ip);vcpu.ip += 1;
		break;
	case 2:
		switch(RM) {
		case 0:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.si;break;
		case 1:	rm = memoryBase+SHL4(insDS)+vcpu.bx+vcpu.di;break;
		case 2:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.si;break;
		case 3:	rm = memoryBase+SHL4(insSS)+vcpu.bp+vcpu.di;break;
		case 4:	rm = memoryBase+SHL4(insDS)+vcpu.si;break;
		case 5:	rm = memoryBase+SHL4(insDS)+vcpu.di;break;
		case 6:	rm = memoryBase+SHL4(insSS)+vcpu.bp;break;
		case 7:	rm = memoryBase+SHL4(insDS)+vcpu.bx;break;
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
	switch(REG) {
	case 0:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.al); else r = (t_vaddrcc)(&vcpu.ax); break;
	case 1:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.cl); else r = (t_vaddrcc)(&vcpu.cx); break;
	case 2:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.dl); else r = (t_vaddrcc)(&vcpu.dx); break;
	case 3:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.bl); else r = (t_vaddrcc)(&vcpu.bx); break;
	case 4:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.ah); else r = (t_vaddrcc)(&vcpu.sp); break;
	case 5:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.ch); else r = (t_vaddrcc)(&vcpu.bp); break;
	case 6:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.dh); else r = (t_vaddrcc)(&vcpu.si); break;
	case 7:	if(regbit == 8) r = (t_vaddrcc)(&vcpu.bh); else r = (t_vaddrcc)(&vcpu.di); break;
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
static void PUSH(void *src, t_nubitcc len)
{
	switch(len) {
	case 16:
		vcpu.sp -= 2;
		*(t_nubit16 *)(memoryBase+SHL4(vcpu.ss)+vcpu.sp) = *(t_nubit16 *)src;
		break;
	default:break;}
}
static void POP(void *dest, t_nubitcc len)
{
	switch(len) {
	case 16:
		*(t_nubit16 *)dest = *(t_nubit16 *)(memoryBase+SHL4(vcpu.ss)+vcpu.sp);
		vcpu.sp += 2;
		break;
	default:break;}
}
static void OR(void *dest, void *src, t_nubitcc len)
{
	switch(len) {
	case 8:
		flglen = 8;
		flginstype = OR8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1|flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = OR16;
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
		flginstype = OR8;
		flgoperand1 = *(t_nubit8 *)dest;
		flgoperand2 = *(t_nubit8 *)src;
		flgresult = (flgoperand1&flgoperand2)&0xff;
		*(t_nubit8 *)dest = flgresult;
		break;
	case 16:
		flglen = 16;
		flginstype = OR16;
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
{nvmprint("IO_NOP\n");}
void ADD_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)rm,(void *)r,8);
	nvmprint("ADD_RM8_R8\n");
}
void ADD_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)rm,(void *)r,16);
	nvmprint("ADD_RM16_R16\n");
}
void ADD_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)r,(void *)rm,8);
	nvmprint("ADD_R8_RM8\n");
}
void ADD_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)r,(void *)rm,16);
	nvmprint("ADD_R16_RM16\n");
}
void ADD_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADD((void *)&vcpu.al,(void *)imm,8);
	nvmprint("ADD_AL_I8\n");
}
void ADD_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADD((void *)&vcpu.ax,(void *)imm,16);
	nvmprint("ADD_AX_I16\n");
}
void PUSH_ES()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.es,16);
	nvmprint("PUSH_ES\n");
}
void POP_ES()
{
	vcpu.ip++;
	POP((void *)&vcpu.es,16);
	nvmprint("POP_ES\n");
}
void OR_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)rm,(void *)r,8);
	nvmprint("OR_RM8_R8\n");
}
void OR_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)rm,(void *)r,16);
	nvmprint("OR_RM16_R16\n");
}
void OR_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	OR((void *)r,(void *)rm,8);
	nvmprint("OR_R8_RM8\n");
}
void OR_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	OR((void *)r,(void *)rm,16);
	nvmprint("OR_R16_RM16\n");
}
void OR_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)imm,8);
	nvmprint("OR_AL_I8\n");
}
void OR_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)imm,16);
	nvmprint("OR_AX_I16\n");
}
void PUSH_CS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.cs,16);
	
	nvmprint("PUSH_CS\n");
}
void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs,16);
	
	nvmprint("POP_CS\n");
}
/*void INS_0F()
{nvmprint("INS_0F\n");}*/
void ADC_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)rm,(void *)r,8);
	nvmprint("ADC_RM8_R8\n");
}
void ADC_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)rm,(void *)r,16);
	nvmprint("ADC_RM16_R16\n");
}
void ADC_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)r,(void *)rm,8);
	nvmprint("ADC_R8_RM8\n");
}
void ADC_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)r,(void *)rm,16);
	nvmprint("ADC_R16_RM16\n");
}
void ADC_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	ADC((void *)&vcpu.al,(void *)imm,8);
	nvmprint("ADC_AL_I8\n");
}
void ADC_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	ADC((void *)&vcpu.ax,(void *)imm,16);
	nvmprint("ADC_AX_I16\n");
}
void PUSH_SS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ss,16);
	nvmprint("PUSH_SS\n");
}
void POP_SS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ss,16);
	nvmprint("POP_SS\n");
}
void SBB_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)rm,(void *)r,8);
	nvmprint("SBB_RM8_R8\n");
}
void SBB_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)rm,(void *)r,16);
	nvmprint("SBB_RM16_R16\n");
}
void SBB_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)r,(void *)rm,8);
	nvmprint("SBB_R8_RM8\n");
}
void SBB_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)r,(void *)rm,16);
	nvmprint("SBB_R16_RM16\n");
}
void SBB_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)imm,8);
	nvmprint("SBB_AL_I8\n");
}
void SBB_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)imm,16);
	nvmprint("SBB_AX_I16\n");
}
void PUSH_DS()
{
	vcpu.ip++;
	PUSH((void *)&vcpu.ds,16);
	nvmprint("PUSH_DS\n");
}
void POP_DS()
{
	vcpu.ip++;
	POP((void *)&vcpu.ds,16);
	nvmprint("POP_DS\n");
}
void AND_RM8_R8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)rm,(void *)r,8);
	nvmprint("AND_RM8_R8\n");
}
void AND_RM16_R16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)rm,(void *)r,16);
	nvmprint("AND_RM16_R16\n");
}
void AND_R8_RM8()
{
	vcpu.ip++;
	GetModRegRM(8,8);
	AND((void *)r,(void *)rm,8);
	nvmprint("AND_R8_RM8\n");
}
void AND_R16_RM16()
{
	vcpu.ip++;
	GetModRegRM(16,16);
	AND((void *)r,(void *)rm,16);
	nvmprint("AND_R16_RM16\n");
}
void AND_AL_I8()
{
	vcpu.ip++;
	GetImm(8);
	AND((void *)&vcpu.al,(void *)imm,8);
	nvmprint("AND_AL_I8\n");
}
void AND_AX_I16()
{
	vcpu.ip++;
	GetImm(16);
	AND((void *)&vcpu.ax,(void *)imm,16);
	nvmprint("AND_AX_I16\n");
}
void ES()
{
	insDS = vcpu.es;
	insSS = vcpu.es;
	nvmprint("ES\n");
}
void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	if(((vcpu.al & 0x0f) > 0x09) || GetAF()) {
		vcpu.al = newAL;
		SetCF(GetCF() | ((newAL < oldAL) || (newAL < 0x06)));
	} else SetAF(0);
	if(((vcpu.al & 0xf0) > 0x90) || GetCF()) {
		vcpu.al += 0x60;
		SetCF(1);
	} else SetCF(0);
	nvmprint("DAA\n");
}
void SUB_RM8_R8()
{nvmprint("SUB_RM8_R8\n");}
void SUB_RM16_R16()
{nvmprint("SUB_RM16_R16\n");}
void SUB_R8_RM8()
{nvmprint("SUB_R8_RM8\n");}
void SUB_R16_RM16()
{nvmprint("SUB_R16_RM16\n");}
void SUB_AL_I8()
{nvmprint("SUB_AL_I8\n");}
void SUB_AX_I16()
{nvmprint("SUB_AX_I16\n");}
void CS()
{nvmprint("CS\n");}
void DAS()
{nvmprint("DAS\n");}
void XOR_RM8_R8()
{nvmprint("XOR_RM8_R8\n");}
void XOR_RM16_R16()
{nvmprint("XOR_RM16_R16\n");}
void XOR_R8_RM8()
{nvmprint("XOR_R8_RM8\n");}
void XOR_R16_RM16()
{nvmprint("XOR_R16_RM16\n");}
void XOR_AL_I8()
{nvmprint("XOR_AL_I8\n");}
void XOR_AX_I16()
{nvmprint("XOR_AX_I16\n");}
void SS()
{nvmprint("SS\n");}
void AAA()
{nvmprint("AAA\n");}
void CMP_RM8_R8()
{nvmprint("CMP_RM8_R8\n");}
void CMP_RM16_R16()
{nvmprint("CMP_RM16_R16\n");}
void CMP_R8_RM8()
{nvmprint("CMP_R8_RM8\n");}
void CMP_R16_RM16()
{nvmprint("CMP_R16_RM16\n");}
void CMP_AL_I8()
{nvmprint("CMP_AL_I8\n");}
void CMP_AX_I16()
{nvmprint("CMP_AX_I16\n");}
void DS()
{nvmprint("DS\n");}
void AAS()
{nvmprint("AAS\n");}
void INC_AX()
{nvmprint("INC_AX\n");}
void INC_CX()
{nvmprint("INC_CX\n");}
void INC_DX()
{nvmprint("INC_DX\n");}
void INC_BX()
{nvmprint("INC_BX\n");}
void INC_SP()
{nvmprint("INC_SP\n");}
void INC_BP()
{nvmprint("INC_BP\n");}
void INC_SI()
{nvmprint("INC_SI\n");}
void INC_DI()
{nvmprint("INC_DI\n");}
void DEC_AX()
{nvmprint("DEC_AX\n");}
void DEC_CX()
{nvmprint("DEC_CX\n");}
void DEC_DX()
{nvmprint("DEC_DX\n");}
void DEC_BX()
{nvmprint("DEC_BX\n");}
void DEC_SP()
{nvmprint("DEC_SP\n");}
void DEC_BP()
{nvmprint("DEC_BP\n");}
void DEC_SI()
{nvmprint("DEC_SI\n");}
void DEC_DI()
{nvmprint("DEC_DI\n");}
void PUSH_AX()
{nvmprint("PUSH_AX\n");}
void PUSH_CX()
{nvmprint("PUSH_CX\n");}
void PUSH_DX()
{nvmprint("PUSH_DX\n");}
void PUSH_BX()
{nvmprint("PUSH_BX\n");}
void PUSH_SP()
{nvmprint("PUSH_SP\n");}
void PUSH_BP()
{nvmprint("PUSH_BP\n");}
void PUSH_SI()
{nvmprint("PUSH_SI\n");}
void PUSH_DI()
{nvmprint("PUSH_DI\n");}
void POP_AX()
{nvmprint("POP_AX\n");}
void POP_CX()
{nvmprint("POP_CX\n");}
void POP_DX()
{nvmprint("POP_DX\n");}
void POP_BX()
{nvmprint("POP_BX\n");}
void POP_SP()
{nvmprint("POP_SP\n");}
void POP_BP()
{nvmprint("POP_BP\n");}
void POP_SI()
{nvmprint("POP_SI\n");}
void POP_DI()
{nvmprint("POP_DI\n");}
/*void OpdSize()
{nvmprint("OpdSize\n");}
void AddrSize()
{nvmprint("AddrSize\n");}
void PUSH_I16()
{nvmprint("PUSH_I16\n");}*/
void JO()
{nvmprint("JO\n");}
void JNO()
{nvmprint("JNO\n");}
void JC()
{nvmprint("JC\n");}
void JNC()
{nvmprint("JNC\n");}
void JZ()
{nvmprint("JZ\n");}
void JNZ()
{nvmprint("JNZ\n");}
void JBE()
{nvmprint("JBE\n");}
void JA()
{nvmprint("JA\n");}
void JS()
{nvmprint("JS\n");}
void JNS()
{nvmprint("JNS\n");}
void JP()
{nvmprint("JP\n");}
void JNP()
{nvmprint("JNP\n");}
void JL()
{nvmprint("JL\n");}
void JNL()
{nvmprint("JNL\n");}
void JLE()
{nvmprint("JLE\n");}
void JG()
{nvmprint("JG\n");}
void INS_80()
{nvmprint("INS_80\n");}
void INS_81()
{nvmprint("INS_81\n");}
void INS_82()
{nvmprint("INS_82\n");}
void INS_83()
{nvmprint("INS_83\n");}
void TEST_RM8_M8()
{nvmprint("TEST_RM8_M8\n");}
void TEST_RM16_M16()
{nvmprint("TEST_RM16_M16\n");}
void XCHG_R8_RM8()
{nvmprint("XCHG_R8_RM8\n");}
void XCHG_R16_RM16()
{nvmprint("XCHG_R16_RM16\n");}
void MOV_RM8_R8()
{nvmprint("MOV_RM8_R8\n");}
void MOV_RM16_R16()
{nvmprint("MOV_RM16_R16\n");}
void MOV_R8_RM8()
{nvmprint("MOV_R8_RM8\n");}
void MOV_R16_RM16()
{nvmprint("MOV_R16_RM16\n");}
void MOV_RM_SEG()
{nvmprint("MOV_RM_SEG\n");}
void LEA_R16_M16()
{nvmprint("LEA_R16_M16\n");}
void MOV_SEG_RM()
{nvmprint("MOV_SEG_RM\n");}
void POP_RM16()
{nvmprint("POP_RM16\n");}
void NOP()
{nvmprint("NOP\n");}
void XCHG_CX_AX()
{nvmprint("XCHG_CX_AX\n");}
void XCHG_DX_AX()
{nvmprint("XCHG_DX_AX\n");}
void XCHG_BX_AX()
{nvmprint("XCHG_BX_AX\n");}
void XCHG_SP_AX()
{nvmprint("XCHG_SP_AX\n");}
void XCHG_BP_AX()
{nvmprint("XCHG_BP_AX\n");}
void XCHG_SI_AX()
{nvmprint("XCHG_SI_AX\n");}
void XCHG_DI_AX()
{nvmprint("XCHG_DI_AX\n");}
void CBW()
{nvmprint("CBW\n");}
void CWD()
{nvmprint("CWD\n");}
void CALL_FAR()
{nvmprint("CALL_FAR\n");}
void WAIT()
{nvmprint("WAIT\n");}
void PUSHF()
{nvmprint("PUSHF\n");}
void POPF()
{nvmprint("POPF\n");}
void SAHF()
{nvmprint("SAHF\n");}
void LAHF()
{nvmprint("LAHF\n");}
void MOV_AL_M8()
{nvmprint("MOV_AL_M8\n");}
void MOV_AX_M16()
{nvmprint("MOV_AX_M16\n");}
void MOV_M8_AL()
{nvmprint("MOV_M8_AL\n");}
void MOV_M16_AX()
{nvmprint("MOV_M16_AX\n");}
void MOVSB()
{nvmprint("MOVSB\n");}
void MOVSW()
{nvmprint("MOVSW\n");}
void CMPSB()
{nvmprint("CMPSB\n");}
void CMPSW()
{nvmprint("CMPSW\n");}
void TEST_AL_I8()
{nvmprint("TEST_AL_I8\n");}
void TEST_AX_I16()
{nvmprint("TEST_AX_I16\n");}
void STOSB()
{nvmprint("STOSB\n");}
void STOSW()
{nvmprint("STOSW\n");}
void LODSB()
{nvmprint("LODSB\n");}
void LODSW()
{nvmprint("LODSW\n");}
void SCASB()
{nvmprint("SCASB\n");}
void SCASW()
{nvmprint("SCASW\n");}
void MOV_AL_I8()
{nvmprint("MOV_AL_I8\n");}
void MOV_CL_I8()
{nvmprint("MOV_CL_I8\n");}
void MOV_DL_I8()
{nvmprint("MOV_DL_I8\n");}
void MOV_BL_I8()
{nvmprint("MOV_BL_I8\n");}
void MOV_AH_I8()
{nvmprint("MOV_AH_I8\n");}
void MOV_CH_I8()
{nvmprint("MOV_CH_I8\n");}
void MOV_DH_I8()
{nvmprint("MOV_DH_I8\n");}
void MOV_BH_I8()
{nvmprint("MOV_BH_I8\n");}
void MOV_AX_I16()
{nvmprint("MOV_AX_I16\n");}
void MOV_CX_I16()
{nvmprint("MOV_CX_I16\n");}
void MOV_DX_I16()
{nvmprint("MOV_DX_I16\n");}
void MOV_BX_I16()
{nvmprint("MOV_BX_I16\n");}
void MOV_SP_I16()
{nvmprint("MOV_SP_I16\n");}
void MOV_BP_I16()
{nvmprint("MOV_BP_I16\n");}
void MOV_SI_I16()
{nvmprint("MOV_SI_I16\n");}
void MOV_DI_I16()
{nvmprint("MOV_DI_I16\n");}
void INS_C0()
{nvmprint("INS_C0\n");}
void INS_C1()
{nvmprint("INS_C1\n");}
void RET_I8()
{nvmprint("RET_I8\n");}
void RET_NEAR()
{nvmprint("RET_NEAR\n");}
void LES_R16_M16()
{nvmprint("LES_R16_M16\n");}
void LDS_R16_M16()
{nvmprint("LDS_R16_M16\n");}
void MOV_M8_I8()
{nvmprint("MOV_M8_I8\n");}
void MOV_M16_I16()
{nvmprint("MOV_M16_I16\n");}
void RET_I16()
{nvmprint("RET_I16\n");}
void RET_FAR()
{nvmprint("RET_FAR\n");}
void INT3()
{nvmprint("INT3\n");}
void INT_I8()
{nvmprint("INT_I8\n");}
void INTO()
{nvmprint("INTO\n");}
void IRET()
{nvmprint("IRET\n");}
void INS_D0()
{nvmprint("INS_D0\n");}
void INS_D1()
{nvmprint("INS_D1\n");}
void INS_D2()
{nvmprint("INS_D2\n");}
void INS_D3()
{nvmprint("INS_D3\n");}
void AAM()
{nvmprint("AAM\n");}
void AAD()
{nvmprint("AAD\n");}
void XLAT()
{nvmprint("XLAT\n");}
void INS_D9()
{nvmprint("INS_D9\n");}
void INS_DB()
{nvmprint("INS_DB\n");}
void LOOPNE()
{nvmprint("LOOPNE\n");}
void LOOPE()
{nvmprint("LOOPE\n");}
void LOOP_NEAR()
{nvmprint("LOOP_NEAR\n");}
void JCXZ_NEAR()
{nvmprint("JCXZ_NEAR\n");}
void IN_AL_N()
{nvmprint("IN_AL_N\n");}
void IN_AX_N()
{nvmprint("IN_AX_N\n");}
void OUT_N_AL()
{nvmprint("OUT_N_AL\n");}
void OUT_N_AX()
{nvmprint("OUT_N_AX\n");}
void CALL_NEAR()
{nvmprint("CALL_NEAR\n");}
void JMP_NEAR_LABEL()
{nvmprint("JMP_NEAR_LABEL\n");}
void JMP_FAR_LABEL()
{nvmprint("JMP_FAR_LABEL\n");}
void JMP_NEAR()
{nvmprint("JMP_NEAR\n");}
void IN_AL_DX()
{nvmprint("IN_AL_DX\n");}
void IN_AX_DX()
{nvmprint("IN_AX_DX\n");}
void OUT_DX_AL()
{nvmprint("OUT_DX_AL\n");}
void OUT_DX_AX()
{nvmprint("OUT_DX_AX\n");}
void LOCK()
{nvmprint("LOCK\n");}
void REPNE()
{nvmprint("REPNE\n");}
void REP()
{nvmprint("REP\n");}
void HLT()
{nvmprint("HLT\n");}
void CMC()
{nvmprint("CMC\n");}
void INS_F6()
{nvmprint("INS_F6\n");}
void INS_F7()
{nvmprint("INS_F7\n");}
void CLC()
{nvmprint("CLC\n");}
void STC()
{nvmprint("STC\n");}
void CLI()
{nvmprint("CLI\n");}
void STI()
{nvmprint("STI\n");}
void CLD()
{nvmprint("CLD\n");}
void STD()
{nvmprint("STD\n");}
void INS_FE()
{nvmprint("INS_FE\n");}
void INS_FF()
{nvmprint("INS_FF\n");}

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
	InsTable[0x84] = TEST_RM8_M8;
	InsTable[0x85] = TEST_RM16_M16;
	InsTable[0x86] = XCHG_R8_RM8;
	InsTable[0x87] = XCHG_R16_RM16;
	InsTable[0x88] = MOV_RM8_R8;
	InsTable[0x89] = MOV_RM16_R16;
	InsTable[0x8a] = MOV_R8_RM8;
	InsTable[0x8b] = MOV_R16_RM16;
	InsTable[0x8c] = MOV_RM_SEG;
	InsTable[0x8d] = LEA_R16_M16;
	InsTable[0x8e] = MOV_SEG_RM;
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
	InsTable[0xc0] = INS_C0;
	InsTable[0xc1] = INS_C1;
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