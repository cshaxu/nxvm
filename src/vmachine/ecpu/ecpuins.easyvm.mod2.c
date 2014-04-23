//////////////////////////////////////////////////////////////////////////
// 名称：Instruction.cpp
// 功能：模拟8086指令集
// 日期：2008年4月20日
// 作者：梁一信
//////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "../vglobal.h"
#include "../vpic.h"
#include "../vapi.h"
#include "../bios/qdbios.h"
#include "../vcpu.h"
#include "../vcpuins.h"
#include "../vram.h"
#include "ecpuapi.h"
#include "ecpuins.h"
#include "ecpu.h"

static t_vaddrcc Ins0FTable[0x100];
static t_nubit32 evIP;

#define ECPUINS_FLAGS_MASKED (VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_VM)

#define eIMS (vramAddr(evIP))
#define toe8 (TranslateOpcExt(0,(char **)&r8,(char **)&rm8))
#define toe16 (TranslateOpcExt(1,(char **)&r16,(char **)&rm16))
#define toe32 (TranslateOpcExt(1,(char **)&r32,(char **)&rm32))

const t_nubit16 Glbffff=0xffff;		//当寻址超过0xfffff的时候，返回的是一个可以令程序Reset的地址
t_nubit16 GlbZero=0x0;			//有些寻址用到两个偏移寄存器；有些寻址只用到一个偏移寄存器，另外一个指向这里。

t_nubit16 *rm16,*r16;			//解释寻址字节的时候用
t_nubit32 *rm32,*r32;
t_nubit8 *rm8,*r8;				//
t_nubit16 t161,t162;			//可以随便使用的
t_nubit32 t321,t322;				//
t_nubit8 t81,t82;				//

t_nubit32 t,t2,t3;				//

t_nubit16 OperandSize=2;			//Operand Size，由描述符里的D位和OpdSize前缀共同决定，初始值为2
t_nubit16 AddressSize=2;			//Address Size，由描述符里的D位和AddrSize前缀共同决定，初始值为2

t_cpuins ecpuins;

#define SAME static void
#define DONE static void
#define VOID static void

VOID SyncCSIP()
{
	t_vaddrcc tevip = evIP - (ecpu.cs.selector << 4);
	ecpu.cs.selector += tevip / 0x10000;
	ecpu.ip  = tevip % 0x10000;
}
VOID SyncEVIP()
{
	evIP = (ecpu.cs.selector << 4) + ecpu.ip;
}
VOID PrintFlags(t_nubit16 flags)
{
	if(flags & VCPU_EFLAGS_OF) vapiPrint("OV ");
	else                     vapiPrint("NV ");
	if(flags & VCPU_EFLAGS_DF) vapiPrint("DN ");
	else                     vapiPrint("UP ");
	if(flags & VCPU_EFLAGS_IF) vapiPrint("EI ");
	else                     vapiPrint("DI ");
	if(flags & VCPU_EFLAGS_SF) vapiPrint("NG ");
	else                     vapiPrint("PL ");
	if(flags & VCPU_EFLAGS_ZF) vapiPrint("ZR ");
	else                     vapiPrint("NZ ");
	if(flags & VCPU_EFLAGS_AF) vapiPrint("AC ");
	else                     vapiPrint("NA ");
	if(flags & VCPU_EFLAGS_PF) vapiPrint("PE ");
	else                     vapiPrint("PO ");
	if(flags & VCPU_EFLAGS_CF) vapiPrint("CY ");
	else                     vapiPrint("NC ");
	vapiPrint("\n");
}

static void ecpuinsExecIns();

VOID LongCallNewIP(char OffsetByte)
{
	t_nubit32 tcs=ecpu.cs.selector;
	t_nubit32 tevIP=evIP;
	tcs<<=4;
	tevIP+=OffsetByte;
	tevIP-=tcs;
	ecpu.cs.selector+=tevIP/0x10000;
	ecpu.ip=tevIP%0x10000;
}
VOID SegOffInc(t_nubit16 *seg, t_nubit16 *off)
{
		(*off)++;
}
VOID SegOffDec(t_nubit16 *seg, t_nubit16 *off)
{
		(*off)--;
}
t_nubit8 GetM8_16(t_nubit16 off)
{
	if (off+(t=ecpu.overds,t<<4)<=0xfffff)
		return *(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+vram.base);
	else
		return 0xff;
}
t_nubit8 GetM8_32(t_nubit32 off)
{
	return *(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+vram.base);
}
t_nubit16 GetM16_16(t_nubit16 off)
{
	if (off+(t=ecpu.overds,t<<4)<=0xfffff)
		return d_nubit16(off+(t=ecpu.overds,t<<4)+vram.base);
	else
		return 0xffff;
}
t_nubit16 GetM16_32(t_nubit32 off)
{
	return d_nubit16(off+(t=ecpu.overds,t<<4)+vram.base);
}
t_nubit32 GetM32_16(t_nubit16 off)
{
	if (off+(t=ecpu.overds,t<<4)<=0xfffff)
		return *(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+vram.base);
	else
		return 0xffffffff;
}
t_nubit32 GetM32_32(t_nubit32 off)
{
	return *(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+vram.base);
}
VOID SetM8(t_nubit16 off, t_nubit8 val)
{
	if (off+(t=ecpu.overds,t<<4))
		*(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+vram.base)=val;

}
VOID SetM16(t_nubit16 off, t_nubit16 val)
{
	if (off+(t=ecpu.overds,t<<4))
		d_nubit16(off+(t=ecpu.overds,t<<4)+vram.base)=val;
}
VOID SetM32(t_nubit16 off, t_nubit32 val)
{
	if (off+(t=ecpu.overds,t<<4))
		*(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+vram.base)=val;
}
VOID *FindRegAddr(t_bool w,char reg)
{
	switch(reg) {
	case 0:
		return &(ecpu.al);
		break;
	case 1:
		return &(ecpu.cl);
		break;
	case 2:
		return &(ecpu.dl);
		break;
	case 3:
		return &(ecpu.bl);
		break;
	case 4:
		if (w)
			return &(ecpu.sp);
		else
			return &(ecpu.ah);
		break;
	case 5:
		if (w)
			return &(ecpu.bp);
		else
			return &(ecpu.ch);
		break;
	case 6:
		if (w)
			return &(ecpu.si);
		else
			return &(ecpu.dh);
		break;
	case 7:
		if (w)
			return &(ecpu.di);
		else
			return &(ecpu.bh);
		break;
	default:
		return 0;
	}
}
VOID *FindSegAddr(t_bool w,char reg)
{
	reg&=0x03;
	switch(reg) {
	case 0:
		return &(ecpu.es.selector);
		break;
	case 1:
		return &(ecpu.cs.selector);
		break;
	case 2:
		return &(ecpu.ss.selector);
		break;
	case 3:
		return &(ecpu.ds.selector);
		break;
	default:
		return 0;
	}
}
t_nubit32 FindRemAddr(char rem , t_nubit16 **off1, t_nubit16 **off2)
{
	t_nubit32 ret;
	t_nubit32 tds, tes, tss;
	tds=ecpu.overds;
	tds<<=4;
	tes=ecpu.es.selector;
	tes<<=4;
	tss=ecpu.overss;
	tss<<=4;
	if (AddressSize == 2) {
		switch(rem) {
		case 0:
			*off1=&ecpu.bx;
			*off2=&ecpu.si;
			ret=vram.base+(t_nubit16)(ecpu.bx+ecpu.si)+(tds);
			break;
		case 1:
			*off1=&ecpu.bx;
			*off2=&ecpu.di;
			ret=vram.base+(t_nubit16)(ecpu.bx+ecpu.di)+(tds);
			break;
		case 2:
			*off1=&ecpu.bp;
			*off2=&ecpu.si;
			ret=vram.base+(t_nubit16)(ecpu.bp+ecpu.si)+(tss);
			break;
		case 3:
			*off1=&ecpu.bp;
			*off2=&ecpu.di;
			ret=vram.base+(t_nubit16)(ecpu.bp+ecpu.di)+(tss);
			break;
		case 4:
			*off1=&ecpu.si;
			*off2=&GlbZero;
			ret=vram.base+ecpu.si+(tds);
			break;
		case 5:
			*off1=&ecpu.di;
			*off2=&GlbZero;
			ret=vram.base+ecpu.di+(tds);
			break;
		case 6:
			*off1=&ecpu.bp;
			*off2=&GlbZero;
			ret=vram.base+ecpu.bp+(tss);
			break;
		case 7:
			*off1=&ecpu.bx;
			*off2=&GlbZero;
			ret=vram.base+ecpu.bx+(tds);
			break;
		default:
			return 0;
		}
	}
	else {
		*off2=&GlbZero;
		switch(rem) {
		case 0:
			*off1=&ecpu.ax;
			ret=vram.base+(t_nubit16)ecpu.eax+(tds);
			break;
		case 1:
			*off1=&ecpu.cx;
			ret=vram.base+(t_nubit16)ecpu.ecx+(tds);
			break;
		case 2:
			*off1=&ecpu.dx;
			ret=vram.base+(t_nubit16)ecpu.edx+(tss);
			break;
		case 3:
			*off1=&ecpu.bx;
			ret=vram.base+(t_nubit16)ecpu.ebx+(tss);
			break;
		case 4:
			__asm nop		//SIB followed
			break;
		case 5:
			*off1=&ecpu.bp;
			ret=vram.base+ecpu.ebp+(tds);
			break;
		case 6:
			*off1=&ecpu.si;
			ret=vram.base+ecpu.esi+(tss);
			break;
		case 7:
			*off1=&ecpu.di;
			ret=vram.base+ecpu.edi+(tds);
			break;
		default:
			return 0;
		}
	}
	if (ret-vram.base<=0xfffff)
		return ret;
	else
		return (t_nubit32)&Glbffff;
}
VOID TranslateOpcExt(t_bool w,char** rg,char** rm)
{
	t_nubit16 *off1,*off2;
	t_nubit32 tds=ecpu.overds;
	t_nubit8 mod, reg, rem;
	tds<<=4;
	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	reg=d_nsbit8(vramAddr(evIP)) & 0x38;
	reg>>=3;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;

	*rg=(char *)FindRegAddr(w,reg);

	switch(mod) {
	case 0:
		if (rem==6 && AddressSize==2) {
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=vram.base;
			evIP++;
		}
		else if (rem==5 && AddressSize==4) {
			evIP++;
			*rm=(char *)((*(t_nubit32 *)eIMS) + tds);
			*rm+=vram.base;
			evIP+=3;
		}
		else
			*rm=(char *)FindRemAddr(rem,&off1,&off2);
		break;
	case 1:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;
		if (AddressSize==2)
			*rm+=(*off1+*off2+d_nsbit8(vramAddr(evIP)))-(*off1+*off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nsbit8(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;
		if (AddressSize==2)
			*rm+=(t_nubit16)(*off1+*off2+d_nubit16(vramAddr(evIP)))-(*off1+*off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
		else
			*rm+=(t_nubit16)(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nubit16(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
		evIP++;
		break;
	case 3:
		*rm=(char *)FindRegAddr(w,rem);
		break;
	default:
		;
	}
	evIP++;
}
VOID TranslateOpcExtSeg(t_bool w,char** rg,char** rm)
{
	t_nubit16 *off1,*off2;
	t_nubit32 tds;
	t_nubit8 mod,reg,rem;
	w=0x01;				//与Seg有关的操作，w只有是1
	tds=ecpu.overds;
	tds<<=4;

	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	reg=d_nsbit8(vramAddr(evIP)) & 0x38;
	reg>>=3;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;

	*rg=(char *)FindSegAddr(w,reg);

	switch(mod) {
	case 0:
		if (rem==6 && AddressSize==2) {
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=vram.base;
			evIP++;
		}
		else if (rem==5 && AddressSize==4) {
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=vram.base;
			evIP+=3;
		}
		else
			*rm=(char *)FindRemAddr(rem,&off1,&off2);
		break;
	case 1:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;
		if (AddressSize==2)
			*rm+=(*off1+*off2+d_nsbit8(vramAddr(evIP)))-(*off1+*off2);		//对偏移寄存器溢出的处理
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nsbit8(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;
		if (AddressSize==2)
			*rm+=(t_nubit16)(*off1+*off2+d_nubit16(vramAddr(evIP)))-(*off1+*off2);
		else
			*rm+=(t_nubit16)(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nubit16(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);
		evIP++;
		break;
	case 3:
		*rm=(char *)FindRegAddr(w,rem);
		break;
	default:
		;
	}
	evIP++;
}
t_bool Bit(void*BitBase, int BitOffset)
{
	t_nubit32 tmp=(t_nubit32)BitBase;
	tmp+=BitOffset/8;
	BitBase=(void*)tmp;
	return (*(t_nubit8 *)BitBase>>(BitOffset%8))&1;
}
// 读到的字节未编码指令
static void UndefinedOpcode()
{
	t_nubit8 *pc=(t_nubit8 *)vramAddr(evIP)-1;
	vapiPrint("An unkown instruction [ %2x %2x %2x %2x %2x %2x ] was read at [ %4xh:%4xh ], easyVM only support 8086 instruction set in this version. easyVM will be terminated.",*(pc),*(pc+1),*(pc+2),*(pc+3),*(pc+4),*(pc+5),ecpu.cs.selector,ecpu.ip);
	vapiCallBackMachineStop();
}

t_nubit8 ub1,ub2,ub3;
t_nubit16 uw1,uw2,uw3;
t_nubit32 udw1,udw2,udw3;

#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
#define im(addr) 0x00
#else
#define im(addr) vramIsAddrInMem(addr)
#endif

#define bugfix(n) if(1)

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

static void CaseError(const char *str)
{
	vapiPrint("The NXVM ECPU has encountered an internal case error: %s.\n",str);
	vapiCallBackMachineStop();
}
static void CalcCF()
{
	switch(ecpuins.type) {
	case ADD8:
	case ADD16:
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,(ecpuins.result < ecpuins.opr1) || (ecpuins.result < ecpuins.opr2));
		break;
	case ADC8:
		bugfix(21)
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (
				(GetBit(ecpu.flags, VCPU_EFLAGS_CF) && ecpuins.opr2 == 0xff) ?
				1 : ((ecpuins.result < ecpuins.opr1) || (ecpuins.result < ecpuins.opr2))));
		else
			bugfix(1) MakeBit(ecpu.flags,VCPU_EFLAGS_CF,ecpuins.result < ecpuins.opr1);
			else MakeBit(ecpu.flags,VCPU_EFLAGS_CF,ecpuins.result <= ecpuins.opr1);
		break;
	case ADC16:
		bugfix(21)
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (
				(GetBit(ecpu.flags, VCPU_EFLAGS_CF) && ecpuins.opr2 == 0xffff) ?
				1 : ((ecpuins.result < ecpuins.opr1) || (ecpuins.result < ecpuins.opr2))));
		else
			bugfix(1) MakeBit(ecpu.flags,VCPU_EFLAGS_CF,ecpuins.result < ecpuins.opr1);
			else MakeBit(ecpu.flags,VCPU_EFLAGS_CF,ecpuins.result <= ecpuins.opr1);
		break;
	case SBB8:
		bugfix(20)
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (ecpuins.opr1 < ecpuins.result) ||
				(GetBit(ecpu.flags, VCPU_EFLAGS_CF) && (ecpuins.opr2 == 0xff)));
		else
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (ecpuins.opr1 < ecpuins.result) ||
				(ecpuins.opr2 == 0xff));
		break;
	case SBB16:
		bugfix(20)
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (ecpuins.opr1 < ecpuins.result) ||
				(GetBit(ecpu.flags, VCPU_EFLAGS_CF) && (ecpuins.opr2 == 0xffff)));
		else
			MakeBit(ecpu.flags, VCPU_EFLAGS_CF, (ecpuins.opr1 < ecpuins.result) ||
				(ecpuins.opr2 == 0xffff));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,ecpuins.opr1 < ecpuins.opr2);
		break;
	default:CaseError("CalcCF::ecpuins.type");break;}
}
static void CalcOF()
{
	switch(ecpuins.type) {
	case ADD8:
	case ADC8:
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,((ecpuins.opr1&0x0080) == (ecpuins.opr2&0x0080)) && ((ecpuins.opr1&0x0080) != (ecpuins.result&0x0080)));
		break;
	case ADD16:
	case ADC16:
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,((ecpuins.opr1&0x8000) == (ecpuins.opr2&0x8000)) && ((ecpuins.opr1&0x8000) != (ecpuins.result&0x8000)));
		break;
	case SBB8:
	case SUB8:
	case CMP8:
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,((ecpuins.opr1&0x0080) != (ecpuins.opr2&0x0080)) && ((ecpuins.opr2&0x0080) == (ecpuins.result&0x0080)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,((ecpuins.opr1&0x8000) != (ecpuins.opr2&0x8000)) && ((ecpuins.opr2&0x8000) == (ecpuins.result&0x8000)));
		break;
	default:CaseError("CalcOF::ecpuins.type");break;}
}
static void CalcAF()
{
	MakeBit(ecpu.flags,VCPU_EFLAGS_AF,((ecpuins.opr1^ecpuins.opr2)^ecpuins.result)&0x10);
}
static void CalcPF()
{
	t_nubit8 res8 = ecpuins.result & 0xff;
	t_nubitcc count = 0;
	while(res8) {
		res8 &= res8-1;
		count++;
	}
	MakeBit(ecpu.flags,VCPU_EFLAGS_PF,!(count&0x01));
}
static void CalcZF()
{
	MakeBit(ecpu.flags,VCPU_EFLAGS_ZF,!ecpuins.result);
}
static void CalcSF()
{
	switch(ecpuins.bit) {
	case 8:MakeBit(ecpu.flags,VCPU_EFLAGS_SF,!!(ecpuins.result&0x80));break;
	case 16:MakeBit(ecpu.flags,VCPU_EFLAGS_SF,!!(ecpuins.result&0x8000));break;
	default:CaseError("CalcSF::ecpuins.bit");break;}
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
	/* returns ecpuins.rrm */
	ecpuins.rrm = vramGetRealAddr(ecpu.overds,vramRealWord(ecpu.cs.selector,ecpu.ip));
	ecpu.ip += 2;
}
static void GetImm(t_nubit8 immbit)
{
	// returns ecpuins.rimm
	ecpuins.rimm = vramGetRealAddr(ecpu.cs.selector,ecpu.ip);
	switch(immbit) {
	case 8:ecpu.ip += 1;break;
	case 16:ecpu.ip += 2;break;
	case 32:ecpu.ip += 4;break;
	default:CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubit8 regbit,t_nubit8 rmbit)
{
	// returns ecpuins.rrm and ecpuins.rr
	t_nubit8 modrm = vramRealByte(ecpu.cs.selector,ecpu.ip++);
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	ecpuins.flagmem = 1;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.si);break;
		case 1:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.di);break;
		case 2:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.si);break;
		case 3:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.di);break;
		case 4:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.si);break;
		case 5:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.di);break;
		case 6:ecpuins.rrm = vramGetRealAddr(ecpu.overds,vramRealWord(ecpu.cs.selector,ecpu.ip));ecpu.ip += 2;break;
		case 7:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::RM");break;}
		break;
	case 1:
		switch(RM) {
		case 0:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.si);break;
		case 1:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.di);break;
		case 2:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.si);break;
		case 3:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.di);break;
		case 4:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.si);break;
		case 5:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.di);break;
		case 6:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp);break;
		case 7:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx);break;
		default:CaseError("GetModRegRM::MOD1::RM");break;}
		bugfix(3) {
			ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		} else {
			ecpuins.rrm += vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		}
		break;
	case 2:
		switch(RM) {
		case 0:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.si);break;
		case 1:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.di);break;
		case 2:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.si);break;
		case 3:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.di);break;
		case 4:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.si);break;
		case 5:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.di);break;
		case 6:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp);break;
		case 7:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx);break;
		default:CaseError("GetModRegRM::MOD2::RM");break;}
		ecpuins.rrm += (t_nubit16)vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;
		break;
	case 3:
		ecpuins.flagmem = 0;
		switch(RM) {
		case 0:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.al); else ecpuins.rrm = (t_vaddrcc)(&ecpu.ax); break;
		case 1:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.cl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.cx); break;
		case 2:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.dl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.dx); break;
		case 3:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.bl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.bx); break;
		case 4:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.ah); else ecpuins.rrm = (t_vaddrcc)(&ecpu.sp); break;
		case 5:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.ch); else ecpuins.rrm = (t_vaddrcc)(&ecpu.bp); break;
		case 6:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.dh); else ecpuins.rrm = (t_vaddrcc)(&ecpu.si); break;
		case 7:if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.bh); else ecpuins.rrm = (t_vaddrcc)(&ecpu.di); break;
		default:CaseError("GetModRegRM::MOD3::RM");break;}
		break;
	default:CaseError("GetModRegRM::MOD");break;}
	switch(regbit) {
	case 0:ecpuins.rr = REG;					break;
	case 4:
		switch(REG) {
		case 0:ecpuins.rr = (t_vaddrcc)(&ecpu.es.selector);	break;
		case 1:ecpuins.rr = (t_vaddrcc)(&ecpu.cs.selector);	break;
		case 2:ecpuins.rr = (t_vaddrcc)(&ecpu.ss.selector);	break;
		case 3:ecpuins.rr = (t_vaddrcc)(&ecpu.ds.selector);	break;
		default:CaseError("GetModRegRM::regbit4::REG");break;}
		break;
	case 8:
		switch(REG) {
		case 0:ecpuins.rr = (t_vaddrcc)(&ecpu.al);	break;
		case 1:ecpuins.rr = (t_vaddrcc)(&ecpu.cl);	break;
		case 2:ecpuins.rr = (t_vaddrcc)(&ecpu.dl);	break;
		case 3:ecpuins.rr = (t_vaddrcc)(&ecpu.bl);	break;
		case 4:ecpuins.rr = (t_vaddrcc)(&ecpu.ah);	break;
		case 5:ecpuins.rr = (t_vaddrcc)(&ecpu.ch);	break;
		case 6:ecpuins.rr = (t_vaddrcc)(&ecpu.dh);	break;
		case 7:ecpuins.rr = (t_vaddrcc)(&ecpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::REG");break;}
		break;
	case 16:
		switch(REG) {
		case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.ax);	break;
		case 1:ecpuins.rr = (t_vaddrcc)(&ecpu.cx);	break;
		case 2:ecpuins.rr = (t_vaddrcc)(&ecpu.dx);	break;
		case 3:ecpuins.rr = (t_vaddrcc)(&ecpu.bx);	break;
		case 4:ecpuins.rr = (t_vaddrcc)(&ecpu.sp);	break;
		case 5:ecpuins.rr = (t_vaddrcc)(&ecpu.bp);	break;
		case 6:ecpuins.rr = (t_vaddrcc)(&ecpu.si);	break;
		case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nubit8 modrm = vramRealByte(ecpu.cs.selector,ecpu.ip++);
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:ecpuins.rrm = ecpu.bx+ecpu.si;break;
		case 1:ecpuins.rrm = ecpu.bx+ecpu.di;break;
		case 2:ecpuins.rrm = ecpu.bp+ecpu.si;break;
		case 3:ecpuins.rrm = ecpu.bp+ecpu.di;break;
		case 4:ecpuins.rrm = ecpu.si;break;
		case 5:ecpuins.rrm = ecpu.di;break;
		case 6:ecpuins.rrm = vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;break;
		case 7:ecpuins.rrm = ecpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::RM");break;}
		break;
	case 1:
		switch(RM) {
		case 0:ecpuins.rrm = ecpu.bx+ecpu.si;break;
		case 1:ecpuins.rrm = ecpu.bx+ecpu.di;break;
		case 2:ecpuins.rrm = ecpu.bp+ecpu.si;break;
		case 3:ecpuins.rrm = ecpu.bp+ecpu.di;break;
		case 4:ecpuins.rrm = ecpu.si;break;
		case 5:ecpuins.rrm = ecpu.di;break;
		case 6:ecpuins.rrm = ecpu.bp;break;
		case 7:ecpuins.rrm = ecpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD1::RM");break;}
		bugfix(3) {
			ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		} else {
			ecpuins.rrm += vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		}
		break;
	case 2:
		switch(RM) {
		case 0:ecpuins.rrm = ecpu.bx+ecpu.si;break;
		case 1:ecpuins.rrm = ecpu.bx+ecpu.di;break;
		case 2:ecpuins.rrm = ecpu.bp+ecpu.si;break;
		case 3:ecpuins.rrm = ecpu.bp+ecpu.di;break;
		case 4:ecpuins.rrm = ecpu.si;break;
		case 5:ecpuins.rrm = ecpu.di;break;
		case 6:
			bugfix(14) ecpuins.rrm = ecpu.bp;
			else ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp);
			break;
		case 7:ecpuins.rrm = ecpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD2::RM");break;}
		ecpuins.rrm += vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;
		break;
	default:CaseError("GetModRegRMEA::MOD");break;}
	switch(REG) {
	case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.ax);	break;
	case 1:ecpuins.rr = (t_vaddrcc)(&ecpu.cx);	break;
	case 2:ecpuins.rr = (t_vaddrcc)(&ecpu.dx);	break;
	case 3:ecpuins.rr = (t_vaddrcc)(&ecpu.bx);	break;
	case 4:ecpuins.rr = (t_vaddrcc)(&ecpu.sp);	break;
	case 5:ecpuins.rr = (t_vaddrcc)(&ecpu.bp);	break;
	case 6:ecpuins.rr = (t_vaddrcc)(&ecpu.si);	break;
	case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.di);	break;
	default:CaseError("GetModRegRMEA::REG");break;}
}

DONE ADD(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = ADD8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		bugfix(22) ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		else ecpuins.opr2 = d_nsbit8(src); /* in this case opr2 could be 0xffffffff */
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("ADD::len");break;}
	SetFlags(ADD_FLAG);
}
DONE OR (void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = OR8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = OR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = OR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("OR::len");break;}
	ClrBit(ecpu.flags, VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags, VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags, VCPU_EFLAGS_AF);
	SetFlags(OR_FLAG);
}
DONE ADC(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = ADC8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = ADC16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = ADC16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("ADC::len");break;}
	SetFlags(ADC_FLAG);
}
DONE SBB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = SBB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = SBB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = SBB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("SBB::len");break;}
	SetFlags(SBB_FLAG);
}
DONE SUB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = SUB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("SUB::len");break;}
	SetFlags(SUB_FLAG);
}
DONE AND(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = AND8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = AND16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = AND16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("AND::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(AND_FLAG);
}
DONE XOR(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = XOR8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = XOR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = XOR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("XOR::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(XOR_FLAG);
}
DONE CMP(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nsbit8(src) & 0xff;
		ecpuins.result = ((t_nubit8)ecpuins.opr1-(t_nsbit8)ecpuins.opr2)&0xff;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = ((t_nubit16)ecpuins.opr1-(t_nsbit8)ecpuins.opr2)&0xffff;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit16(src) & 0xffff;
		bugfix(7) ecpuins.result = ((t_nubit16)ecpuins.opr1-(t_nsbit16)ecpuins.opr2)&0xffff;
		else ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		break;
	default:CaseError("_CMP::len");break;}
	SetFlags(CMP_FLAG);
}
DONE STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		if(GetBit(ecpu.flags, VCPU_EFLAGS_DF)) {
			if (flagdi) ecpu.di--;
			if (flagsi) ecpu.si--;
		} else {
			if (flagdi) ecpu.di++;
			if (flagsi) ecpu.si++;
		}
		break;
	case 16:
		ecpuins.bit = 16;
		if(GetBit(ecpu.flags, VCPU_EFLAGS_DF)) {
			if (flagdi) ecpu.di -= 2;
			if (flagsi) ecpu.si -= 2;
		} else {
			if (flagdi) ecpu.di += 2;
			if (flagsi) ecpu.si += 2;
		}
		break;
	default:CaseError("STRDIR::len");break;}
}
DONE MOVS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		vramRealByte(ecpu.es.selector,ecpu.di) = vramRealByte(ecpu.overds,ecpu.si);
		STRDIR(8,1,1);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  MOVSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es.selector,ecpu.di) = vramRealWord(ecpu.overds,ecpu.si);
		STRDIR(16,1,1);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  MOVSW\n");
		break;
	default:CaseError("MOVS::len");break;}
	//qdcgaCheckVideoRam(vramGetRealAddr(ecpu.es.selector, ecpu.di));
}
DONE CMPS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = vramRealByte(ecpu.overds,ecpu.si);
		ecpuins.opr2 = vramRealByte(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  CMPSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = vramRealWord(ecpu.overds,ecpu.si);
		ecpuins.opr2 = vramRealWord(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  CMPSW\n");
		break;
	default:CaseError("_CMPS::len");break;}
}
DONE STOS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		vramRealByte(ecpu.es.selector,ecpu.di) = ecpu.al;
		STRDIR(8,0,1);
		/*if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
		WriteVideoRam(eCPU.di+t-0xa0000);*/
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  STOSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es.selector,ecpu.di) = ecpu.ax;
		STRDIR(16,0,1);
		/*if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000) {
			for (i=0;i<OperandSize;i++) {
				WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
			}
		}*/
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  STOSW\n");
		break;
	default:CaseError("STOS::len");break;}
}
DONE LODS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpu.al = vramRealByte(ecpu.overds,ecpu.si);
		STRDIR(8,1,0);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  LODSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		ecpu.ax = vramRealWord(ecpu.overds,ecpu.si);
		STRDIR(16,1,0);
		// _vapiPrintAddr(ecpu.cs.selector,ecpu.ip);vapiPrint("  LODSW\n");
		break;
	default:CaseError("LODS::len");break;}
}
DONE SCAS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = ecpu.al;
		ecpuins.opr2 = vramRealByte(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = ecpu.ax;
		ecpuins.opr2 = vramRealWord(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::len");break;}
}
DONE PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		ecpu.sp -= 2;
		bugfix(13) vramRealWord(ecpu.ss.selector,ecpu.sp) = data;
		else vramRealWord(ecpu.ss.selector,ecpu.sp) = d_nubit16(src);
		break;
	default:CaseError("PUSH::len");break;}
}
DONE POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		d_nubit16(dest) = vramRealWord(ecpu.ss.selector,ecpu.sp);
		ecpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
DONE INT(t_nubit8 intid)
{
	PUSH((void *)&ecpu.flags,16);
	ClrBit(ecpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
	PUSH((void *)&ecpu.cs.selector,16);
	PUSH((void *)&ecpu.ip,16);
	ecpu.ip = vramRealWord(0x0000,intid*4+0);
	ecpu.cs.selector = vramRealWord(0x0000,intid*4+2);
	evIP = (ecpu.cs.selector << 4) + ecpu.ip;
}
DONE TEST(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = TEST8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1 & ecpuins.opr2)&0xff;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = TEST16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1 & ecpuins.opr2)&0xffff;
		break;
	default:CaseError("TEST::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(TEST_FLAG);
}
DONE NOT(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = ~d_nubit8(dest);
		break;
	case 16:
		ecpuins.bit = 16;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = ~d_nubit16(dest);
		break;
	default:CaseError("NOT::len");break;}
}
DONE NEG(void *dest, t_nubit8 len)
{
	t_nubitcc zero = 0;
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)zero;
		break;
	case 16:
		ecpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)zero;
		break;
	default:CaseError("NEG::len");break;}
}
DONE MUL(void *src, t_nubit8 len)
{
	t_nubit32 tempresult;
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpu.ax = ecpu.al * d_nubit8(src);
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!ecpu.ah);
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,!!ecpu.ah);
		break;
	case 16:
		ecpuins.bit = 16;
		tempresult = ecpu.ax * d_nubit16(src);
		ecpu.dx = (tempresult>>16)&0xffff;
		ecpu.ax = tempresult&0xffff;
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!ecpu.dx);
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,!!ecpu.dx);
		break;
	default:CaseError("MUL::len");break;}
}
DONE IMUL(void *src, t_nubit8 len)
{
	t_nsbit32 tempresult;
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpu.ax = (t_nsbit8)ecpu.al * d_nsbit8(src);
		if(ecpu.ax == ecpu.al) {
			ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
			ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(ecpu.flags,VCPU_EFLAGS_OF);
			SetBit(ecpu.flags,VCPU_EFLAGS_CF);
		}
		break;
	case 16:
		ecpuins.bit = 16;
		tempresult = (t_nsbit16)ecpu.ax * d_nsbit16(src);
		ecpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		ecpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)ecpu.ax) {
			ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
			ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(ecpu.flags,VCPU_EFLAGS_OF);
			SetBit(ecpu.flags,VCPU_EFLAGS_CF);
		}
		break;
	default:CaseError("IMUL::len");break;}
}
DONE DIV(void *src, t_nubit8 len)
{
	t_nubit16 tempAX = ecpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)ecpu.dx)<<16)+ecpu.ax;
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			ecpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			ecpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
		break;
	case 16:
		ecpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			ecpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			ecpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
		break;
	default:CaseError("DIV::len");break;}
}
DONE IDIV(void *src, t_nubit8 len)
{
	t_nsbit16 tempAX = ecpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)ecpu.dx)<<16)+ecpu.ax;
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			ecpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			ecpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
		break;
	case 16:
		ecpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			ecpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			ecpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
		break;
	default:CaseError("IDIV::len");break;}
}

DONE INC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = ADD8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = 0x01;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = 0x0001;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("INC::len");break;}
	SetFlags(INC_FLAG);
}
DONE DEC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = SUB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = 0x01;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = 0x0001;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("DEC::len");break;}
	SetFlags(DEC_FLAG);
}

VOID XCHG(void *dest, void *src, int Len)
{
	switch(Len) {
	case 1:
		t81 = d_nubit8(dest);
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = d_nubit8(src);
		if (!im((t_vaddrcc)src))
			d_nubit8(src) = t81;
		break;
	case 2:
		t161=d_nubit16(dest);
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = d_nubit16(src);
		if (!im((t_vaddrcc)src))
			d_nubit16(src) = t161;
		break;
	case 4:
		t321=d_nubit32(dest);
		if (!im((t_vaddrcc)dest))
			d_nubit32(dest) = d_nubit32(src);
		if (!im((t_vaddrcc)src))
			d_nubit32(src) = t321;
		break;
	}
}

SAME MOV(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		d_nubit8(dest) = d_nubit8(src);
		break;
	case 16:
		ecpuins.bit = 16;
		d_nubit16(dest) = d_nubit16(src);
		break;
	default:CaseError("MOV::len");break;}
}
VOID _MOV(void *dest, void *src, int bit)
{
	switch(bit) {
	case 1:
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest) = d_nubit8(src);
		break;
	case 2:
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest) = d_nubit16(src);
		break;
	case 4:
		if (!im((t_vaddrcc)dest))
			d_nubit32(dest) = d_nubit32(src);
		break;
	}
}
VOID SHL(void *dest, t_nubit8 mb, int Len)
{
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	t_nubit8 tSHLrm8;
	t_nubit16 tSHLrm16;
	t_nubit32 tSHLrm32;
	switch(Len) {
	case 1:
		tSHLrm8=d_nubit8(dest);
		__asm {
			mov al,tSHLrm8
			mov cl,mb
			push ecpu.eflags
			popfd
			shl al,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm8,al
		}
		if (!im((t_vaddrcc)dest))
			d_nubit8(dest)=tSHLrm8;
		break;
	case 2:
		tSHLrm16=d_nubit16(dest);
		__asm {
			mov ax,tSHLrm16
			mov cl,mb
			push ecpu.eflags
			popfd
			shl ax,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm16,ax
		}
		if (!im((t_vaddrcc)dest))
			d_nubit16(dest)=tSHLrm16;
		break;
	case 4:
		tSHLrm32=d_nubit32(dest);
		__asm {
			mov eax,tSHLrm32
			mov cl,mb
			push ecpu.eflags
			popfd
			shl eax,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm32,eax
		}
		if (!im((t_vaddrcc)dest))
			d_nubit32(dest)=tSHLrm8;
		break;
	}
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
}
VOID Jcc(int Len)
{
	switch(Len) {
	case 1: evIP += d_nsbit8(eIMS);break;
	case 2: evIP += d_nsbit16(eIMS);break;
	case 4: evIP += d_nsbit32(eIMS);break;
	default: break;
	}
}
VOID JCC(char code, t_bool J)
{
	if (d_nsbit8(eIMS-1)==code) {
		if (J) {
			Jcc(1);
		}
		evIP++;
	}
	else {
		if (J) {
			Jcc(OperandSize);
		}
		evIP+=OperandSize;
	}
}
VOID JMP_NEAR();

#define ci1 if(1) {SyncCSIP();ecpu.ip--;} else
#define ci2 SyncEVIP()

// 0x00
SAME ADD_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)ecpuins.rrm, (void *)ecpuins.rr, 8);
	ci2;
}
SAME ADD_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)ecpuins.rrm, (void *)ecpuins.rr, 16);
	ci2;
}
SAME ADD_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)ecpuins.rr, (void *)ecpuins.rrm, 8);
	ci2;
}
SAME ADD_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)ecpuins.rr, (void *)ecpuins.rrm, 16);
	ci2;
}
SAME ADD_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	ADD((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME ADD_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	ADD((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
	ci2;
}
SAME PUSH_ES()
{
	ci1;
	ecpu.ip++;
	PUSH(&ecpu.es.selector,16);
	ci2;
}
SAME POP_ES()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.es.selector,16);
	ci2;
}
SAME OR_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	OR((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME OR_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	OR((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME OR_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	OR((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME OR_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	OR((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME OR_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	OR((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME OR_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	OR((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME PUSH_CS()
{
	ci1;
	ecpu.ip++;
	PUSH(&ecpu.cs.selector,16);
	ci2;
}
// 0x10
SAME ADC_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME ADC_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME ADC_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME ADC_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME ADC_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	ADC((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME ADC_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	ADC((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME PUSH_SS()
{
	ci1;
	ecpu.ip++;
	PUSH(&ecpu.ss.selector,16);
	ci2;
}
SAME POP_SS()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.ss.selector,16);
	ci2;
}
SAME SBB_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME SBB_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME SBB_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME SBB_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME SBB_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	SBB((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME SBB_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	SBB((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME PUSH_DS()
{
	ci1;
	ecpu.ip++;
	PUSH(&ecpu.ds.selector,16);
	ci2;
}
SAME POP_DS()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.ds.selector,16);
	ci2;
}
// 0x20
SAME AND_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	AND((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME AND_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	AND((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME AND_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	AND((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME AND_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	AND((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME AND_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	AND((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME AND_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	AND((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME ES()
{
	ci1;
	ecpu.ip++;
	ecpu.overds=ecpu.es.selector;
	ecpu.overss=ecpu.es.selector;
	ci2;
}
SAME DAA()
{
	t_nubit8 oldAL = ecpu.al;
	t_nubit8 newAL = ecpu.al + 0x06;
	ci1;
	ecpu.ip++;
	if(((ecpu.al & 0x0f) > 0x09) || GetBit(ecpu.flags, VCPU_EFLAGS_AF)) {
		ecpu.al = newAL;
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetBit(ecpu.flags, VCPU_EFLAGS_CF) || ((newAL < oldAL) || (newAL < 0x06)));
		bugfix(19) SetBit(ecpu.flags, VCPU_EFLAGS_AF);
		else ;
	} else ClrBit(ecpu.flags, VCPU_EFLAGS_AF);
	if(((ecpu.al & 0xf0) > 0x90) || GetBit(ecpu.flags, VCPU_EFLAGS_CF)) {
		ecpu.al += 0x60;
		SetBit(ecpu.flags,VCPU_EFLAGS_CF);
	} else ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ecpuins.bit = 8;
	ecpuins.result = (t_nubitcc)ecpu.al;
	SetFlags(DAA_FLAG);
	ci2;
}
SAME SUB_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME SUB_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME SUB_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME SUB_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME SUB_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	SUB((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME SUB_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	SUB((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME CS()
{
	ci1;
	ecpu.ip++;
	ecpu.overds=ecpu.cs.selector;
	ecpu.overss=ecpu.cs.selector;
	ci2;
}
SAME DAS()
{
	t_nubit8 oldAL = ecpu.al;
	ci1;
	ecpu.ip++;
	if(((ecpu.al & 0x0f) > 0x09) || GetBit(ecpu.flags, VCPU_EFLAGS_AF)) {
		ecpu.al -= 0x06;
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetBit(ecpu.flags, VCPU_EFLAGS_CF) || (oldAL < 0x06));
		SetBit(ecpu.flags,VCPU_EFLAGS_AF);
	} else ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	if((ecpu.al > 0x9f) || GetBit(ecpu.flags, VCPU_EFLAGS_CF)) {
		ecpu.al -= 0x60;
		SetBit(ecpu.flags,VCPU_EFLAGS_CF);
	} else ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	bugfix(18) {
		ecpuins.bit = 8;
		ecpuins.result = (t_nubitcc)ecpu.al;
		SetFlags(DAS_FLAG);
	} else ;
	ci2;
}
// 0x30
SAME XOR_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME XOR_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME XOR_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME XOR_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME XOR_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	XOR((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME XOR_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	XOR((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME SS()
{
	ci1;
	ecpu.ip++;
	ecpu.overds=ecpu.ss.selector;
	ecpu.overss=ecpu.ss.selector;
	ci2;
}
SAME AAA()
{
	ci1;
	ecpu.ip++;
	if(((ecpu.al&0x0f) > 0x09) || GetBit(ecpu.flags, VCPU_EFLAGS_AF)) {
		ecpu.al += 0x06;
		ecpu.ah += 0x01;
		SetBit(ecpu.flags,VCPU_EFLAGS_AF);
		SetBit(ecpu.flags,VCPU_EFLAGS_CF);
	} else {
		ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
		ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	}
	ecpu.al &= 0x0f;
	ci2;
}
SAME CMP_RM8_R8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME CMP_RM16_R16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
SAME CMP_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
	ci2;
}
SAME CMP_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
SAME CMP_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	CMP((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME CMP_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	CMP((void *)&ecpu.al,(void *)ecpuins.rimm,16);
	ci2;
}
SAME DS()
{
	ci1;
	ecpu.ip++;
	ecpu.overds=ecpu.ds.selector;
	ecpu.overss=ecpu.ds.selector;
	ci2;
}
SAME AAS()
{
	ci1;
	ecpu.ip++;
	if(((ecpu.al&0x0f) > 0x09) || GetBit(ecpu.flags, VCPU_EFLAGS_AF)) {
		ecpu.al -= 0x06;
		ecpu.ah += 0x01;
		SetBit(ecpu.flags,VCPU_EFLAGS_AF);
		SetBit(ecpu.flags,VCPU_EFLAGS_CF);
	} else {
		ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	}
	ecpu.al &= 0x0f;
	ci2;
}
// 0x40
SAME INC_AX()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.ax,16);
	ci2;
}
SAME INC_CX()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.cx,16);
	ci2;
}
SAME INC_DX()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.dx,16);
	ci2;
}
SAME INC_BX()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.bx,16);
	ci2;
}
SAME INC_SP()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.sp,16);
	ci2;
}
SAME INC_BP()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.bp,16);
	ci2;
}
SAME INC_SI()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.si,16);
	ci2;
}
SAME INC_DI()
{
	ci1;
	ecpu.ip++;
	INC((void *)&ecpu.di,16);
	ci2;
}
SAME DEC_AX()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.ax,16);
	ci2;
}
SAME DEC_CX()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.cx,16);
	ci2;
}
SAME DEC_DX()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.dx,16);
	ci2;
}
SAME DEC_BX()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.bx,16);
	ci2;
}
SAME DEC_SP()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.sp,16);
	ci2;
}
SAME DEC_BP()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.bp,16);
	ci2;
}
SAME DEC_SI()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.si,16);
	ci2;
}
SAME DEC_DI()
{
	ci1;
	ecpu.ip++;
	DEC((void *)&ecpu.di,16);
	ci2;
}
// 0x50
SAME PUSH_AX()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.ax,16);
	ci2;
}
SAME PUSH_CX()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.cx,16);
	ci2;
}
SAME PUSH_DX()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.dx,16);
	ci2;
}
SAME PUSH_BX()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.bx,16);
	ci2;
}
SAME PUSH_SP()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.sp,16);
	ci2;
}
SAME PUSH_BP()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.bp,16);
	ci2;
}
SAME PUSH_SI()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.si,16);
	ci2;
}
SAME PUSH_DI()
{
	ci1;
	ecpu.ip++;
	PUSH((void *)&ecpu.di,16);
	ci2;
}
SAME POP_AX()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.ax,16);
	ci2;
}
SAME POP_CX()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.cx,16);
	ci2;
}
SAME POP_DX()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.dx,16);
	ci2;
}
SAME POP_BX()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.bx,16);
	ci2;
}
SAME POP_SP()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.sp,16);
	ci2;
}
SAME POP_BP()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.bp,16);
	ci2;
}
SAME POP_SI()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.si,16);
	ci2;
}
SAME POP_DI()
{
	ci1;
	ecpu.ip++;
	POP((void *)&ecpu.di,16);
	ci2;
}
// 0x60
VOID ARPL()
{
	toe16;
	if (((*rm16)&3)<((*r16)&3)) {
		ecpu.eflags|=ZF;
		if (!im((t_vaddrcc)rm16)) {
			(*rm16) &= 0xfc;
			(*rm16) |= (*r16)&3;
		}
	}
	else
		ecpu.eflags&=~ZF;
}
VOID OpdSize()
{
	OperandSize=6-OperandSize;
	ecpuinsExecIns();
	OperandSize=6-OperandSize;
}
VOID AddrSize()
{
	AddressSize=6-AddressSize;
	ecpuinsExecIns();
	AddressSize=6-AddressSize;
}
VOID PUSH_I16()
{
	PUSH((void*)eIMS, 16);
}
// 0x70
VOID JO()
{
	JCC(0x70,ecpu.eflags & OF);
}
VOID JNO()
{
	JCC(0x71,!(ecpu.flags & OF));
}
VOID JC()
{
	JCC(0x72,ecpu.flags & CF);
}
VOID JNC()
{
	JCC(0x73,!(ecpu.flags & CF));
}
VOID JZ()
{
	JCC(0x74,ecpu.flags & ZF);
}
VOID JNZ()
{
	JCC(0x75,!(ecpu.flags & ZF));
}
VOID JBE()
{
	JCC(0x76,ecpu.flags & CF || ecpu.flags & ZF);
}
VOID JA()
{
	JCC(0x77,!(ecpu.flags & CF) && !(ecpu.flags & ZF));
}
VOID JS()
{
	JCC(0x78,ecpu.flags & SF);
}
VOID JNS()
{
	JCC(0x79,!(ecpu.flags & SF));
}
VOID JP()
{
	JCC(0x7A,ecpu.flags & PF);
}
VOID JNP()
{
	JCC(0x7B,!(ecpu.flags & PF));
}
VOID JL()
{
	JCC(0x7C,((ecpu.flags & SF) != (ecpu.flags & OF)));
}
VOID JNL()
{
	JCC(0x7D,((ecpu.flags & SF) == (ecpu.flags & OF)));
}
VOID JLE()
{
	JCC(0x7E,((ecpu.flags & SF) != (ecpu.flags & OF)) || (ecpu.flags & ZF));
}
VOID JG()
{
	JCC(0x7F,((ecpu.flags & SF) == (ecpu.flags & OF)) && !(ecpu.flags & ZF));
}
// 0x80
VOID INS_80()	//这里是以80开头的指令的集。
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	default:CaseError("INS_80::ecpuins.rr");break;}
	ci2;
}
VOID INS_81()	//这里是以81开头的指令的集。
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	default:CaseError("INS_81::ecpuins.r");break;}
	ci2;
}
VOID INS_83()	//这里是以83开头的指令的集。
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	default:CaseError("INS_83::ecpuins.r");break;}
	ci2;
}
SAME TEST_RM8_M8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
	ci2;
}
SAME TEST_RM16_M16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
	ci2;
}
VOID XCHG_R8_RM8()
{
	toe8;
	XCHG(r8,rm8,1);
}
VOID XCHG_R16_RM16()
{
	toe16;
	XCHG(r16,rm16,OperandSize);
}
VOID MOV_RM8_R8()
{
	toe8;
	_MOV(rm8,r8,1);
}
VOID MOV_RM16_R16()
{
	toe16;
	_MOV(rm16,r16,OperandSize);
}
SAME MOV_R8_RM8()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(8,8);
	d_nubit8(ecpuins.rr) = d_nubit8(ecpuins.rrm);
	ci2;
}
VOID MOV_R16_RM16()
{
	toe16;
	_MOV(r16,rm16,OperandSize);
}
VOID MOV_RM_SEG()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*r16=*rm16;
	_MOV(r16,rm16,2);
}
VOID LEA_R16_M16()
{
	t_nubit8 mod, rem;
	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;
	toe16;
	switch(rem) {
	case 0:
	case 1:
	case 4:
	case 5:
	case 7:
		*r16=(t_nubit16)((t_nubit32)rm16-vram.base-(t=ecpu.overds,t<<4));
		break;
	case 2:
	case 3:
		*r16=(t_nubit16)((t_nubit32)rm16-vram.base-(t=ecpu.overss,t<<4));
		break;
	case 6:
		if (mod==0)
			*r16=(t_nubit16)((t_nubit32)rm16-vram.base-(t=ecpu.overds,t<<4));
		else
			*r16=(t_nubit16)((t_nubit32)rm16-vram.base-(t=ecpu.overss,t<<4));
		break;
	default:
		return ;
	}
}
VOID MOV_SEG_RM()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*rm16=*r16;
	_MOV(rm16,r16,2);
}
VOID POP_RM16()
{
	toe16;
 	POP((void*)rm16, 16);
}
// 0x90
VOID NOP()
{
	__asm nop
}
VOID XCHG_CX_AX()
{
	XCHG(&ecpu.cx,&ecpu.ax,OperandSize);
}
VOID XCHG_DX_AX()
{
	XCHG(&ecpu.dx,&ecpu.ax,OperandSize);
}
VOID XCHG_BX_AX()
{
	XCHG(&ecpu.bx,&ecpu.ax,OperandSize);
}
VOID XCHG_SP_AX()
{
	XCHG(&ecpu.sp,&ecpu.ax,OperandSize);
}
VOID XCHG_BP_AX()
{
	XCHG(&ecpu.bp,&ecpu.ax,OperandSize);
}
VOID XCHG_SI_AX()
{
	XCHG(&ecpu.si,&ecpu.ax,OperandSize);
}
VOID XCHG_DI_AX()
{
	XCHG(&ecpu.di,&ecpu.ax,OperandSize);
}
VOID CBW()
{
	switch(OperandSize) {
	case 2:
		ecpu.ax=(char)ecpu.al;
		break;
	case 4:
		ecpu.eax=(short)ecpu.ax;
		break;
	}
}
VOID CWD()
{
	switch(OperandSize) {
	case 2:
		if (ecpu.ax & 0x8000)
			ecpu.dx=0xffff;
		else
			ecpu.dx=0;
		break;
	case 4:
		if (ecpu.eax & 0x80000000)
			ecpu.edx=0xffffffff;
		else
			ecpu.edx=0;
		break;
	}
}
VOID CALL_FAR()
{
	LongCallNewIP(4);			//这个指令后带4个字节的数据

	ecpu.sp-=2;
	d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp))=ecpu.cs.selector;
	ecpu.sp-=2;
	d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp))=ecpu.ip;
	ecpu.ip=d_nubit16(eIMS);
	evIP+=2;
	ecpu.cs.selector=d_nubit16(eIMS);
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID WAIT()
{
	__asm nop
}
VOID PUSHF()
{
	PUSH(&ecpu.flags, 16);
}
VOID POPF()
{
	t_nubit32 newflags;
	POP(&newflags, 16);
	ecpu.flags = (ecpu.flags & VCPU_EFLAGS_RESERVED) | (newflags & ~VCPU_EFLAGS_RESERVED);

}
VOID SAHF()
{
	*(t_nubit8 *)&ecpu.flags=ecpu.ah;
}
VOID LAHF()
{
	ecpu.ah=*(t_nubit8 *)&ecpu.flags;
}
// 0xA0
VOID MOV_AL_M8()
{
	t81=GetM8_16(d_nubit16(eIMS));
	_MOV(&ecpu.al,&t81,1);
	evIP+=2;
}
VOID MOV_AX_M16()
{
	switch(OperandSize) {
	case 2:
		t161=GetM16_16(d_nubit16(eIMS));
		_MOV(&ecpu.ax,&t161,OperandSize);
		break;
	case 4:
		t321=GetM32_16(d_nubit16(eIMS));
		_MOV(&ecpu.eax,&t321,OperandSize);
		break;
	}
	evIP+=AddressSize;
}
VOID MOV_M8_AL()
{
	SetM8(d_nubit16(eIMS),ecpu.al);
	evIP+=2;
}
VOID MOV_M16_AX()
{
	switch(OperandSize) {
	case 2:
		SetM16(d_nubit16(eIMS),ecpu.ax);
		break;
	case 4:
		SetM32(d_nubit16(eIMS),ecpu.eax);
		break;
	}
	evIP+=AddressSize;
}

SAME MOVSB()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(8);
	else {
		if (ecpu.cx) {
			MOVS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
}
SAME MOVSW()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(16);
	else {
		if (ecpu.cx) {
			MOVS(16);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
}
SAME CMPSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) CMPS(8);
	else {
		if (ecpu.cx) {
			CMPS(8);
			ecpu.cx--;
		}
		if (ecpu.cx &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
				ecpuins.flaginsloop = 1;
	}
	ci2;
}
SAME CMPSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) CMPS(16);
	else {
		if (ecpu.cx) {
			CMPS(16);
			ecpu.cx--;
		}
		if (ecpu.cx &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
				ecpuins.flaginsloop = 1;
	}
	ci2;
}
SAME TEST_AL_I8()
{
	ci1;
	ecpu.ip++;
	GetImm(8);
	TEST((void *)&ecpu.al,(void *)ecpuins.rimm,8);
	ci2;
}
SAME TEST_AX_I16()
{
	ci1;
	ecpu.ip++;
	GetImm(16);
	TEST((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
	ci2;
}

SAME STOSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
	else {
		if (ecpu.cx) {
			STOS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
	ci2;
}
SAME STOSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(16);
	else {
		if (ecpu.cx) {
			STOS(16);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
	ci2;
}
SAME LODSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
	else {
		if (ecpu.cx) {
			LODS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
	ci2;
}
SAME LODSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(16);
	else {
		if (ecpu.cx) {
			LODS(16);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
	ci2;
}
SAME SCASB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) SCAS(8);
	else {
		if (ecpu.cx) {
			SCAS(8);
			ecpu.cx--;
		}
		if (ecpu.cx &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
				ecpuins.flaginsloop = 0x01;
	}
	ci2;
}
SAME SCASW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) SCAS(16);
	else {
		if (ecpu.cx) {
			SCAS(16);
			ecpu.cx--;
		}
		if (ecpu.cx &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
			!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
				ecpuins.flaginsloop = 0x01;
	}
	ci2;
}

VOID MOV_AL_I8()
{
	_MOV(&ecpu.al,(void*)eIMS,1);
	evIP++;
}
VOID MOV_CL_I8()
{
	_MOV(&ecpu.cl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_DL_I8()
{
	_MOV(&ecpu.dl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_BL_I8()
{
	_MOV(&ecpu.bl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_AH_I8()
{
	_MOV(&ecpu.ah,(void*)eIMS,1);
	evIP++;
}
VOID MOV_CH_I8()
{
	_MOV(&ecpu.ch,(void*)eIMS,1);
	evIP++;
}
VOID MOV_DH_I8()
{
	_MOV(&ecpu.dh,(void*)eIMS,1);
	evIP++;
}
VOID MOV_BH_I8()
{
	_MOV(&ecpu.bh,(void*)eIMS,1);
	evIP++;
}
VOID MOV_AX_I16()
{
	_MOV(&ecpu.ax,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_CX_I16()
{
	_MOV(&ecpu.cx,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_DX_I16()
{
	_MOV(&ecpu.dx,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_BX_I16()
{
	_MOV(&ecpu.bx,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_SP_I16()
{
	_MOV(&ecpu.sp,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_BP_I16()
{
	_MOV(&ecpu.bp,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_SI_I16()
{
	_MOV(&ecpu.si,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID MOV_DI_I16()
{
	_MOV(&ecpu.di,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
// 0xC0
VOID INS_C0()
{
	t_nubit8 t,teIMS;
	t_nubit16 teIMS16;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce>>=3;
	toe8;
	t=*rm8;
	teIMS=d_nubit8(eIMS);
	teIMS16 = teIMS;
	__asm push teIMS16
	__asm push ecpu.flags
	switch(oce) {
	case 0:
		__asm popf				//因为switch语句会改变eflags的值，所以不能把这句提到switch之外。
		__asm pop ecx			//因为push teIMS的时候压了4个字节，所以这里也要用pop ecx弹4个字节
		__asm rol t,cl
		break;
	case 1:
		__asm popf
		__asm pop ecx
		__asm ror t,cl
		break;
	case 2:
		__asm popf
		__asm pop ecx
		__asm rcl t,cl
		break;
	case 3:
		__asm popf
		__asm pop ecx
		__asm rcr t,cl
		break;
	case 4:
		__asm popf
		__asm pop ecx
		__asm shl t,cl
		break;
	case 5:
		__asm popf
		__asm pop ecx
		__asm shr t,cl
		break;
	case 6:
		UndefinedOpcode();
		break;
	case 7:
		__asm popf
		__asm pop ecx
		__asm sar t,cl
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
	if (!im((t_vaddrcc)rm8))
		*rm8=t;
	evIP++;
}
VOID INS_C1()
{
	t_nubit16 t, teIMS16;
	t_nubit32 t2;
	t_nubit8 teIMS;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce>>=3;
	switch (OperandSize) {
	case 2:
		toe16;
		teIMS=d_nubit8(eIMS);
		t=*rm16;
		teIMS16 = teIMS;
		__asm push teIMS16
		__asm push ecpu.eflags
		switch(oce) {
		case 0:
			__asm popfd
			__asm pop ecx
			__asm rol t,cl
			break;
		case 1:
			__asm popfd
			__asm pop ecx
			__asm ror t,cl
			break;
		case 2:
			__asm popfd
			__asm pop ecx
			__asm rcl t,cl
			break;
		case 3:
			__asm popfd
			__asm pop ecx
			__asm rcr t,cl
			break;
		case 4:
			__asm popfd
			__asm pop ecx
			__asm shl t,cl
			break;
		case 5:
			__asm popfd
			__asm pop ecx
			__asm shr t,cl
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm popfd
			__asm pop ecx
			__asm sar t,cl
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		if (!im((t_vaddrcc)rm16)) *rm16=t;
		break;
	case 4:
		toe32;
		teIMS=d_nubit8(eIMS);
		teIMS16 = teIMS;
		t2=*rm32;
		__asm push teIMS16
		__asm push ecpu.eflags
		switch(oce) {
		case 0:
			__asm popfd
			__asm pop ecx
			__asm rol t2,cl
			break;
		case 1:
			__asm popfd
			__asm pop ecx
			__asm ror t2,cl
			break;
		case 2:
			__asm popfd
			__asm pop ecx
			__asm rcl t2,cl
			break;
		case 3:
			__asm popfd
			__asm pop ecx
			__asm rcr t2,cl
			break;
		case 4:
			__asm popfd
			__asm pop ecx
			__asm shl t2,cl
			break;
		case 5:
			__asm popfd
			__asm pop ecx
			__asm shr t2,cl
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm popfd
			__asm pop ecx
			__asm sar t2,cl
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		if (!im((t_vaddrcc)rm32)) *rm32=t2;
		break;
	}
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
	evIP++;
}
VOID SHL_RM8_I8()
{
	t_nubit8 teIMS;
	toe8;
	teIMS=d_nubit8(eIMS);
	SHL(rm8,teIMS,1);
	evIP++;
}
VOID SHL_RM16_I8()
{
	t_nubit8 teIMS;
	toe16;
	teIMS=d_nubit8(eIMS);
	SHL(rm16,teIMS,OperandSize);
	evIP++;
}
VOID RET_I8()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID RET_NEAR()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID LES_R16_M16()
{
	switch(OperandSize) {
	case 2:
		toe16;
		*r16=*rm16;
		ecpu.es.selector=d_nubit16(rm16+1);
		break;
	case 4:
		toe32;
		*r32=*rm32;
		ecpu.es.selector=d_nubit16(rm32+1);
		break;
	}
}
VOID LDS_R16_M16()
{
	switch(OperandSize) {
	case 2:
		toe16;
		*r16=*rm16;
		ecpu.ds.selector=d_nubit16(rm16+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	case 4:
		toe32;
		*r32=*rm32;
		ecpu.ds.selector=d_nubit16(rm32+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	}
}
VOID MOV_M8_I8()
{
	toe8;
	_MOV(rm8,(void*)eIMS,1);
	evIP++;
}
VOID MOV_M16_I16()
{
	toe16;
	_MOV(rm16,(void*)eIMS,OperandSize);
	evIP+=OperandSize;
}
VOID RET_I16()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.cs.selector=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID RET_FAR()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.cs.selector=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
SAME INT3() {INT(0x03);}
SAME INT_I8()
{
	t_nubit8 id;
	LongCallNewIP(1);
	id = d_nubit8(eIMS);
	INT(id);
}
SAME INTO() {if (GetBit(ecpu.flags, VCPU_EFLAGS_OF)) INT(0x04);}
VOID IRET()					//在实模式下，iret和ret far是一样的，这里可以直接调用RET_FAR()的，不过为了以后扩展着想就不这样做。
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.cs.selector=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	ecpu.flags=d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp));
	ecpu.sp+=2;
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
// 0xD0
VOID INS_D0()
{
	t_nubit8 t;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm push ecpu.flags;
	switch(oce) {
	case 0:
		__asm popf				//因为switch语句会改变eflags的值，所以不能把这句提到switch之外。
		__asm rol t,1
		break;
	case 1:
		__asm popf
		__asm ror t,1
		break;
	case 2:
		__asm popf
		__asm rcl t,1
		break;
	case 3:
		__asm popf
		__asm rcr t,1
		break;
	case 4:
		__asm popf
		__asm shl t,1
		break;
	case 5:
		__asm popf
		__asm shr t,1
		break;
	case 6:
		UndefinedOpcode();
		break;
	case 7:
		__asm popf
		__asm sar t,1
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
	if (!im((t_vaddrcc)rm8)) *rm8=t;
}
VOID INS_D1()
{
	t_nubit16 t;
	t_nubit32 t2;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce>>=3;
	switch (OperandSize) {
	case 2:
		toe16;
		t=*rm16;
		__asm push ecpu.eflags;
		switch(oce) {
		case 0:
			__asm popfd
			__asm rol t,1
			break;
		case 1:
			__asm popfd
			__asm ror t,1
			break;
		case 2:
			__asm popfd
			__asm rcl t,1
			break;
		case 3:
			__asm popfd
			__asm rcr t,1
			break;
		case 4:
			__asm popfd
			__asm shl t,1
			break;
		case 5:
			__asm popfd
			__asm shr t,1
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm popfd
			__asm sar t,1
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		if (!im((t_vaddrcc)rm16)) *rm16=t;
		break;
	case 4:
		toe32;
		t2=*rm32;
		__asm push ecpu.eflags;
		switch(oce) {
		case 0:
			__asm popfd
			__asm rol t2,1
			break;
		case 1:
			__asm popfd
			__asm ror t2,1
			break;
		case 2:
			__asm popfd
			__asm rcl t2,1
			break;
		case 3:
			__asm popfd
			__asm rcr t2,1
			break;
		case 4:
			__asm popfd
			__asm shl t2,1
			break;
		case 5:
			__asm popfd
			__asm shr t2,1
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm popfd
			__asm sar t2,1
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		if (!im((t_vaddrcc)rm32)) *rm32=t2;
		break;
	}
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
}
VOID INS_D2()
{
	t_nubit8 t;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm mov cl,ecpu.cl;
	__asm push ecpu.flags;
	switch(oce) {
	case 0:
		__asm popf
		__asm rol t,cl
		break;
	case 1:
		__asm popf
		__asm ror t,cl
		break;
	case 2:
		__asm popf
		__asm rcl t,cl
		break;
	case 3:
		__asm popf
		__asm rcr t,cl
		break;
	case 4:
		__asm popf
		__asm shl t,cl
		break;
	case 5:
		__asm popf
		__asm shr t,cl
		break;
	case 6:
		UndefinedOpcode();
		break;
	case 7:
		__asm popf
		__asm sar t,cl
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
	if (!im((t_vaddrcc)rm8)) *rm8=t;
}
VOID INS_D3()
{
	char oce;
	t_nubit16 t;
	t_nubit32 t2;
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	switch(OperandSize) {
	case 2:
		toe16;
		t=*rm16;
		switch(oce) {
		case 0:
			__asm mov cl,ecpu.cl;				//switch/case有可能要用到cl，所以不能把这两行提取出去
			__asm push ecpu.flags;
			__asm popf
			__asm rol t,cl
			break;
		case 1:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm ror t,cl
			break;
		case 2:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm rcl t,cl
			break;
		case 3:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm rcr t,cl
			break;
		case 4:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm shl t,cl
			break;
		case 5:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm shr t,cl
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.flags;
			__asm popf
			__asm sar t,cl
			break;
		}
		__asm pushf
		__asm pop ecpu.flags
		if (!im((t_vaddrcc)rm16)) *rm16=t;
		break;
	case 4:
		toe32;
		t2=*rm32;
		switch(oce) {
		case 0:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm rol t2,cl
			break;
		case 1:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm ror t2,cl
			break;
		case 2:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm rcl t2,cl
			break;
		case 3:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm rcr t2,cl
			break;
		case 4:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm shl t2,cl
			break;
		case 5:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm shr t2,cl
			break;
		case 6:
			UndefinedOpcode();
			break;
		case 7:
			__asm mov cl,ecpu.cl;
			__asm push ecpu.eflags;
			__asm popfd
			__asm sar t2,cl
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		if (!im((t_vaddrcc)rm32)) *rm32=t2;
		break;
	}
	ecpu.flags = (ecpu.flags & ~ECPUINS_FLAGS_MASKED) | flagsave;
}
VOID AAM()
{
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	if ((d_nubit8(eIMS))==0)
		INT(0x00);
	else
	__asm {
		mov ax,ecpu.ax
		push ecpu.flags
		popf
		aam
		pushf
		pop ecpu.flags
		mov ecpu.ax,ax
	}
	evIP++;			//aam的编码是D4 0A
}
VOID AAD()
{
	t_nubit16 flagsave = ecpu.flags & ECPUINS_FLAGS_MASKED;
	__asm {
		mov ax,ecpu.ax
		push ecpu.flags
		popf
		aad
		pushf
		pop ecpu.flags
		mov ecpu.ax,ax
	}
	evIP++;			//aad的编码是D5 0A
}
VOID ESC_9()
{
	__asm nop;
}
VOID XLAT()
{
	ecpu.al=GetM8_16((t_nubit16)(ecpu.bx+ecpu.al));
}
VOID ESC_0() {INT(0x07);}
VOID ESC_1() {toe16;INT(0x07);}
VOID ESC_2() {INT(0x07);}
VOID ESC_3() {INT(0x07);evIP++;}
VOID ESC_4() {INT(0x07);}
VOID ESC_5() {INT(0x07);}
VOID ESC_6() {INT(0x07);}
VOID ESC_7() {INT(0x07);}
// 0xE0
VOID LOOPNE()
{
	ecpu.cx--;
	if (ecpu.cx && !(ecpu.flags &ZF))
		JMP_NEAR();
	else
		evIP++;
}
VOID LOOPE()
{
	ecpu.cx--;
	if (ecpu.cx && (ecpu.flags &ZF))
		JMP_NEAR();
	else
		evIP++;
}
VOID LOOP_NEAR()
{
	ecpu.cx--;
	if (ecpu.cx)
		JMP_NEAR();
	else
		evIP++;
}
VOID JCXZ_NEAR()
{
	if (!ecpu.cx)
		JMP_NEAR();
	else
		evIP++;
}
SAME IN_AL_N()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	ExecFun(vport.in[d_nubit8(eIMS)]);
	ecpu.al = vport.iobyte;
#else
	ecpu.flagignore = 0x01;
#endif
	evIP++;
}
SAME IN_AX_N()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	ExecFun(vport.in[d_nubit8(eIMS)]);
	ecpu.ax = vport.ioword;
#else
	ecpu.flagignore = 0x01;
#endif
	evIP++;
}
SAME OUT_N_AL()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[d_nubit8(eIMS)]);
#else
	ecpu.flagignore = 0x01;
#endif
	evIP++;
}
SAME OUT_N_AX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	vport.ioword = ecpu.ax;
	ExecFun(vport.out[d_nubit8(eIMS)]);
#else
	ecpu.flagignore = 0x01;
#endif
	evIP++;
}
VOID CALL_NEAR()
{
	LongCallNewIP(2);

	ecpu.sp-=2;										//段内CALL，CS不压栈
	d_nubit16(vramGetRealAddr(ecpu.ss.selector, ecpu.sp))=ecpu.ip;
	ecpu.ip+=(d_nubit16(eIMS));
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID JMP_NEAR_LABEL()	//立即数是两字节的
{
	LongCallNewIP(2);
	ecpu.ip+=(d_nubit16(eIMS));
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID JMP_FAR_LABEL()
{
	LongCallNewIP(4);
	ecpu.ip=d_nubit16(eIMS);
	evIP+=2;
	ecpu.cs.selector=d_nubit16(eIMS);
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
VOID JMP_NEAR()			//立即数是一字节的
{
	LongCallNewIP(1);
	ecpu.ip+=(d_nsbit8(eIMS));
	evIP=((t=ecpu.cs.selector,t<<4))+ecpu.ip;
}
SAME IN_AL_DX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	ExecFun(vport.in[ecpu.dx]);
	ecpu.al = vport.iobyte;
#else
	ecpu.flagignore = 0x01;
#endif
}
SAME IN_AX_DX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	ExecFun(vport.in[ecpu.dx]);
	ecpu.ax = vport.ioword;
#else
	ecpu.flagignore = 0x01;
#endif
}
SAME OUT_DX_AL()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[ecpu.dx]);
#else
	ecpu.flagignore = 0x01;
#endif
}
SAME OUT_DX_AX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	vport.ioword = ecpu.ax;
	ExecFun(vport.out[ecpu.dx]);
#else
	ecpu.flagignore = 0x01;
#endif
}

// 0xF0
SAME LOCK() {}
SAME REPNE()
{
	// _CMPS,SCAS
	ci1;
	ecpu.ip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZNZ;
	ci2;
}
SAME REP()
{
	ci1;
	ecpu.ip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZ;
	ci2;
}
VOID HLT() {ecpu.ip++;}
VOID CMC() {ecpu.flags^=CF;}
SAME INS_F6()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	GetImm(8);
			TEST((void *)ecpuins.rrm, (void *)ecpuins.rimm, 8);
			break;
	case 2:	NOT ((void *)ecpuins.rrm,8);	break;
	case 3:	NEG ((void *)ecpuins.rrm,8);	break;
	case 4:	MUL ((void *)ecpuins.rrm,8);	break;
	case 5:	IMUL((void *)ecpuins.rrm,8);	break;
	case 6:	DIV ((void *)ecpuins.rrm,8);	break;
	case 7:	IDIV((void *)ecpuins.rrm,8);	break;
	default:CaseError("INS_F6::ecpuins.rr");break;}
	ci2;
}
SAME INS_F7()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:	GetImm(16);
			TEST((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);
			break;
	case 2:	NOT ((void *)ecpuins.rrm,16);	break;
	case 3:	NEG ((void *)ecpuins.rrm,16);	break;
	case 4:	MUL ((void *)ecpuins.rrm,16);	break;
	case 5:	IMUL((void *)ecpuins.rrm,16);	break;
	case 6:	DIV ((void *)ecpuins.rrm,16);	break;
	case 7:	IDIV((void *)ecpuins.rrm,16);	break;
	default:CaseError("INS_F7::ecpuins.rr");break;}
	ci2;
}
VOID CLC()
{
	ecpu.flags &= ~CF;
}
VOID STC()
{
	ecpu.flags |= CF;
}
VOID CLI()
{
	ecpu.flags &= ~IF;
}
VOID STI()
{
	ecpu.flags |= IF;
	ecpuins.flagmaskint = 1;
}
VOID CLD()
{
	ecpu.flags &= ~DF;
}
VOID STD()
{
	ecpu.flags |= DF;
}
SAME INS_FE()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	INC((void *)ecpuins.rrm,8);	break;
	case 1:	DEC((void *)ecpuins.rrm,8);	break;
	default:CaseError("INS_FE::ecpuins.rr");break;}
	ci2;
}
SAME INS_FF()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:	INC((void *)ecpuins.rrm,16);	break;
	case 1:	DEC((void *)ecpuins.rrm,16);	break;
	case 2:	/* CALL_RM16 */
		PUSH((void *)&ecpu.ip,16);
		ecpu.ip = d_nubit16(ecpuins.rrm);
		break;
	case 3:	/* CALL_M16_16 */
		PUSH((void *)&ecpu.cs.selector,16);
		PUSH((void *)&ecpu.ip,16);
		ecpu.ip = d_nubit16(ecpuins.rrm);
		ecpu.cs.selector = d_nubit16(ecpuins.rrm+2);
		break;
	case 4:	/* JMP_RM16 */
		ecpu.ip = d_nubit16(ecpuins.rrm);
		break;
	case 5:	/* JMP_M16_16 */
		ecpu.ip = d_nubit16(ecpuins.rrm);
		ecpu.cs.selector = d_nubit16(ecpuins.rrm+2);
		break;
	case 6:	/* PUSH_RM16 */
		PUSH((void *)ecpuins.rrm,16);
		break;
	default:CaseError("INS_FF::ecpuins.rr");break;}
	ci2;
}

VOID INS_0F()
{
	t_nubit8 OpC=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit32 tcs,InsFucAddr;
	evIP++;
	InsFucAddr=Ins0FTable[OpC];
	__asm call InsFucAddr;
	tcs=ecpu.cs.selector;
	ecpu.ip=(evIP - (tcs << 4)) % 0x10000;
	ecpu.overds=ecpu.ds.selector;
	ecpu.overss=ecpu.ss.selector;
}

// 下面这部分是0F开头的指令
VOID ADDPS()
{
	t_nubit8 a=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push ecpu.eflags;
	__asm popf
	for (i=0;i<4;i++) {
		ecpu.xmm[a].fp[i]+=ecpu.xmm[b].fp[i];
	}
	__asm pushf
	__asm pop ecpu.eflags;
	evIP++;
}
VOID ADDSS()
{
	t_nubit8 a=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit8 b=a&0x7;
	a>>=4;a&=0x7;
	__asm push ecpu.eflags;
	__asm popf
	ecpu.xmm[a].fp[3]+=ecpu.xmm[b].fp[3];
	__asm pushf
	__asm pop ecpu.eflags;
	evIP++;
}
VOID ANDNPS()
{
	t_nubit8 a=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push ecpu.eflags
	__asm popf
	for (i=0;i<4;i++) {
		ecpu.xmm[a].ip[i]=~ecpu.xmm[a].ip[i];
		ecpu.xmm[a].ip[i]&=ecpu.xmm[b].ip[i];
	}
	__asm pushf
	__asm pop ecpu.eflags
	evIP++;
}
VOID ANDPS()
{
	t_nubit8 a=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push ecpu.eflags;
	__asm popf
	for (i=0;i<4;i++) {
		ecpu.xmm[a].ip[i]&=ecpu.xmm[b].ip[i];
	}
	__asm pushf
	__asm pop ecpu.eflags;
	evIP++;
}
VOID BSF()
{
	int temp;
	switch(OperandSize) {
	case 2:
		toe16;
		if (*rm16==0) {
			ecpu.eflags|=ZF;
		}
		else {
			ecpu.eflags&=~ZF;
			temp=0;
			while (!Bit(rm16,temp)) {
				temp++;
				*r16=temp;
			}
		}
		break;
	case 4:
		toe32;
		if (*rm32==0) {
			ecpu.eflags|=ZF;
		}
		else {
			ecpu.eflags&=~ZF;
			temp=0;
			while (!Bit(rm32,temp)) {
				temp++;
				*r32=temp;
			}
		}
		break;
	}
}
VOID BSR()
{
	int temp;
	switch(OperandSize) {
	case 2:
		toe16;
		if (*rm16==0) {
			ecpu.eflags|=ZF;
		}
		else {
			ecpu.eflags&=~ZF;
			temp=OperandSize-1;
			while (!Bit(rm16,temp)) {
				temp--;
				*r16=temp;
			}
		}
		break;
	case 4:
		toe32;
		if (*rm32==0) {
			ecpu.eflags|=ZF;
		}
		else {
			ecpu.eflags&=~ZF;
			temp=OperandSize-1;
			while (!Bit(rm32,temp)) {
				temp--;
				*r32=temp;
			}
		}
		break;
	}
}
VOID BSWAP()
{
	t_nubit32 temp;
	t_nubit32 *desc;
	desc=(t_nubit32 *)FindRegAddr(1,*(t_nubit8 *)(eIMS-1));
	temp=*desc;
	*desc=((temp&0xff)<<24)+((temp&0xff00)<<8)+((temp&0xff0000)>>8)+((temp&0xff000000)>>24);
}
VOID CPUID()
{
	// do nothing
	__asm nop
}
VOID FINIT()
{
	ecpu.FpuCR=0x037f;
	ecpu.FpuSR=0;
	ecpu.FpuTR=0xffff;
	ecpu.Fpudp.seg=0;
	ecpu.Fpudp.off=0;
	ecpu.Fpuip.seg=0;
	ecpu.Fpuip.off=0;
	ecpu.FpuOpcode=0;
}
VOID INS_D9()
{
	char oce,mod,rem;
	t_nubit8 OpC=*(t_nubit8 *)(vramAddr(evIP));
	//evIP++;
	if (OpC<0xc0) {
		oce=OpC & 0x38;
		oce>>=3;
		mod=OpC & 0xc0;
		mod>>=6;
		mod&=0x3;
		rem=OpC & 0x07;
		toe16;
		switch(oce) {
		case 7:
			if (!im((t_vaddrcc)rm16)) *rm16=ecpu.FpuSR;
			break;
		default:
			UndefinedOpcode();
			break;
		}
	}
}
VOID INS_DB()
{
	t_nubit8 OpC=*(t_nubit8 *)(vramAddr(evIP));
	evIP++;
	switch(OpC) {
	case 0xe3:
		FINIT();
		break;
	default:
		UndefinedOpcode();
	}

}
VOID MOVZX_RM8()
{
	toe8;
	t321=*rm8;
	_MOV(r16,&t321,OperandSize);
}
VOID MOVZX_RM16()
{
	toe16;
	t321=*rm16;
	_MOV(r16,&t321,OperandSize);
}
VOID POP_FS()
{
	POP(&ecpu.fs, 16);
}
VOID INS_0F01()
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe16;
	switch(oce) {
	case 0:
		UndefinedOpcode();
		break;
	case 1:
		UndefinedOpcode();
		break;
	case 2:
		UndefinedOpcode();
		break;
	case 3:
		UndefinedOpcode();
		break;
	case 4:
		if (!im((t_vaddrcc)rm16)) *rm16=(t_nubit16)ecpu.CR[0];
		break;
	case 5:
		UndefinedOpcode();
		break;
	case 6:
		UndefinedOpcode();
		break;
	case 7:
		UndefinedOpcode();
		break;
	}
}
// SPECIAL
VOID QDX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	qdbiosExecInt(d_nubit8(eIMS));
	MakeBit(vramRealWord(ecpu.ss.selector,ecpu.sp + 4), VCPU_EFLAGS_ZF, GetBit(ecpu.eflags, VCPU_EFLAGS_ZF));
	MakeBit(vramRealWord(ecpu.ss.selector,ecpu.sp + 4), VCPU_EFLAGS_CF, GetBit(ecpu.eflags, VCPU_EFLAGS_CF));
#else
	ecpu.flagignore = 0x01;
#endif
	evIP++;
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
				return 0x01;break;
	default:return 0x00;break;
	}
}
static void ClrPrefix()
{
	ecpu.overds = ecpu.ds.selector;
	ecpu.overss = ecpu.ss.selector;
	ecpuins.prefix_rep = PREFIX_REP_NONE;
}

static void ecpuinsExecIns()
{
	t_nubit8 opcode;
	//vapiPrint("bf\n");ecpuapiPrintRegs();
	ecpuins.oldcs.selector = ecpu.cs.selector;
	ecpuins.oldss.selector = ecpu.ss.selector;
	ecpuins.oldeip = ecpu.eip;
	ecpuins.oldesp = ecpu.esp;
	ecpuins.flaginsloop = 0;
	ecpuins.flagmaskint = 0;
	SyncEVIP();

	do {
		opcode = vramByte(evIP);
		evIP++;
		ExecFun(ecpuins.table[opcode]);
		ecpu.ip = (evIP - (ecpu.cs.selector << 4)) % 0x10000;
	} while (IsPrefix(opcode));
	//vapiPrint("af\n");ecpuapiPrintRegs();
	ClrPrefix();

	if (ecpuins.flaginsloop) {
		ecpu.cs.selector = ecpuins.oldcs.selector;
		ecpu.eip = ecpuins.oldeip;
		SyncEVIP();
	}
}
static void ecpuinsExecInt()
{
	/* hardware interrupt handeler */
	t_nubit8 intr;
	if (ecpuins.flagmaskint)
		return;
	if(ecpu.flagnmi) {
		INT(0x02);
	}
	ecpu.flagnmi = 0x00;

	if(GetBit(ecpu.flags, VCPU_EFLAGS_IF) && ecpuapiScanINTR()) {
		intr = ecpuapiGetINTR();
		//printf("eint = %x\n", intr);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
		INT(intr);
#else
		ecpu.flagignore = 1;
#endif
	}
	if(GetBit(ecpu.flags, VCPU_EFLAGS_TF)) {
		INT(0x01);
	}
}

void ecpuinsInit()
{
	ecpuins.table[0x00]=(t_faddrcc)ADD_RM8_R8;
	ecpuins.table[0x01]=(t_faddrcc)ADD_RM16_R16;
	ecpuins.table[0x02]=(t_faddrcc)ADD_R8_RM8;
	ecpuins.table[0x03]=(t_faddrcc)ADD_R16_RM16;
	ecpuins.table[0x04]=(t_faddrcc)ADD_AL_I8;
	ecpuins.table[0x05]=(t_faddrcc)ADD_AX_I16;
	ecpuins.table[0x06]=(t_faddrcc)PUSH_ES;
	ecpuins.table[0x07]=(t_faddrcc)POP_ES;
	ecpuins.table[0x08]=(t_faddrcc)OR_RM8_R8;
	ecpuins.table[0x09]=(t_faddrcc)OR_RM16_R16;
	ecpuins.table[0x0A]=(t_faddrcc)OR_R8_RM8;
	ecpuins.table[0x0B]=(t_faddrcc)OR_R16_RM16;
	ecpuins.table[0x0C]=(t_faddrcc)OR_AL_I8;
	ecpuins.table[0x0D]=(t_faddrcc)OR_AX_I16;
	ecpuins.table[0x0E]=(t_faddrcc)PUSH_CS;
	ecpuins.table[0x0F]=(t_faddrcc)INS_0F;
	ecpuins.table[0x10]=(t_faddrcc)ADC_RM8_R8;
	ecpuins.table[0x11]=(t_faddrcc)ADC_RM16_R16;
	ecpuins.table[0x12]=(t_faddrcc)ADC_R8_RM8;
	ecpuins.table[0x13]=(t_faddrcc)ADC_R16_RM16;
	ecpuins.table[0x14]=(t_faddrcc)ADC_AL_I8;
	ecpuins.table[0x15]=(t_faddrcc)ADC_AX_I16;
	ecpuins.table[0x16]=(t_faddrcc)PUSH_SS;
	ecpuins.table[0x17]=(t_faddrcc)POP_SS;
	ecpuins.table[0x18]=(t_faddrcc)SBB_RM8_R8;
	ecpuins.table[0x19]=(t_faddrcc)SBB_RM16_R16;
	ecpuins.table[0x1A]=(t_faddrcc)SBB_R8_RM8;
	ecpuins.table[0x1B]=(t_faddrcc)SBB_R16_RM16;
	ecpuins.table[0x1C]=(t_faddrcc)SBB_AL_I8;
	ecpuins.table[0x1D]=(t_faddrcc)SBB_AX_I16;
	ecpuins.table[0x1E]=(t_faddrcc)PUSH_DS;
	ecpuins.table[0x1F]=(t_faddrcc)POP_DS;
	ecpuins.table[0x20]=(t_faddrcc)AND_RM8_R8;
	ecpuins.table[0x21]=(t_faddrcc)AND_RM16_R16;
	ecpuins.table[0x22]=(t_faddrcc)AND_R8_RM8;
	ecpuins.table[0x23]=(t_faddrcc)AND_R16_RM16;
	ecpuins.table[0x24]=(t_faddrcc)AND_AL_I8;
	ecpuins.table[0x25]=(t_faddrcc)AND_AX_I16;
	ecpuins.table[0x26]=(t_faddrcc)ES;
	ecpuins.table[0x27]=(t_faddrcc)DAA;
	ecpuins.table[0x28]=(t_faddrcc)SUB_RM8_R8;
	ecpuins.table[0x29]=(t_faddrcc)SUB_RM16_R16;
	ecpuins.table[0x2A]=(t_faddrcc)SUB_R8_RM8;
	ecpuins.table[0x2B]=(t_faddrcc)SUB_R16_RM16;
	ecpuins.table[0x2C]=(t_faddrcc)SUB_AL_I8;
	ecpuins.table[0x2D]=(t_faddrcc)SUB_AX_I16;
	ecpuins.table[0x2E]=(t_faddrcc)CS;
	ecpuins.table[0x2F]=(t_faddrcc)DAS;
	ecpuins.table[0x30]=(t_faddrcc)XOR_RM8_R8;
	ecpuins.table[0x31]=(t_faddrcc)XOR_RM16_R16;
	ecpuins.table[0x32]=(t_faddrcc)XOR_R8_RM8;
	ecpuins.table[0x33]=(t_faddrcc)XOR_R16_RM16;
	ecpuins.table[0x34]=(t_faddrcc)XOR_AL_I8;
	ecpuins.table[0x35]=(t_faddrcc)XOR_AX_I16;
	ecpuins.table[0x36]=(t_faddrcc)SS;
	ecpuins.table[0x37]=(t_faddrcc)AAA;
	ecpuins.table[0x38]=(t_faddrcc)CMP_RM8_R8;
	ecpuins.table[0x39]=(t_faddrcc)CMP_RM16_R16;
	ecpuins.table[0x3A]=(t_faddrcc)CMP_R8_RM8;
	ecpuins.table[0x3B]=(t_faddrcc)CMP_R16_RM16;
	ecpuins.table[0x3C]=(t_faddrcc)CMP_AL_I8;
	ecpuins.table[0x3D]=(t_faddrcc)CMP_AX_I16;
	ecpuins.table[0x3E]=(t_faddrcc)DS;
	ecpuins.table[0x3F]=(t_faddrcc)AAS;
	ecpuins.table[0x40]=(t_faddrcc)INC_AX;
	ecpuins.table[0x41]=(t_faddrcc)INC_CX;
	ecpuins.table[0x42]=(t_faddrcc)INC_DX;
	ecpuins.table[0x43]=(t_faddrcc)INC_BX;
	ecpuins.table[0x44]=(t_faddrcc)INC_SP;
	ecpuins.table[0x45]=(t_faddrcc)INC_BP;
	ecpuins.table[0x46]=(t_faddrcc)INC_SI;
	ecpuins.table[0x47]=(t_faddrcc)INC_DI;
	ecpuins.table[0x48]=(t_faddrcc)DEC_AX;
	ecpuins.table[0x49]=(t_faddrcc)DEC_CX;
	ecpuins.table[0x4A]=(t_faddrcc)DEC_DX;
	ecpuins.table[0x4B]=(t_faddrcc)DEC_BX;
	ecpuins.table[0x4C]=(t_faddrcc)DEC_SP;
	ecpuins.table[0x4D]=(t_faddrcc)DEC_BP;
	ecpuins.table[0x4E]=(t_faddrcc)DEC_SI;
	ecpuins.table[0x4F]=(t_faddrcc)DEC_DI;
	ecpuins.table[0x50]=(t_faddrcc)PUSH_AX;
	ecpuins.table[0x51]=(t_faddrcc)PUSH_CX;
	ecpuins.table[0x52]=(t_faddrcc)PUSH_DX;
	ecpuins.table[0x53]=(t_faddrcc)PUSH_BX;
	ecpuins.table[0x54]=(t_faddrcc)PUSH_SP;
	ecpuins.table[0x55]=(t_faddrcc)PUSH_BP;
	ecpuins.table[0x56]=(t_faddrcc)PUSH_SI;
	ecpuins.table[0x57]=(t_faddrcc)PUSH_DI;
	ecpuins.table[0x58]=(t_faddrcc)POP_AX;
	ecpuins.table[0x59]=(t_faddrcc)POP_CX;
	ecpuins.table[0x5A]=(t_faddrcc)POP_DX;
	ecpuins.table[0x5B]=(t_faddrcc)POP_BX;
	ecpuins.table[0x5C]=(t_faddrcc)POP_SP;
	ecpuins.table[0x5D]=(t_faddrcc)POP_BP;
	ecpuins.table[0x5E]=(t_faddrcc)POP_SI;
	ecpuins.table[0x5F]=(t_faddrcc)POP_DI;
	ecpuins.table[0x60]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x61]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x62]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x63]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x64]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x65]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x66]=(t_faddrcc)OpdSize;
	ecpuins.table[0x67]=(t_faddrcc)AddrSize;
	ecpuins.table[0x68]=(t_faddrcc)PUSH_I16;
	ecpuins.table[0x69]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6A]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6B]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6C]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6D]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6E]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6F]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x70]=(t_faddrcc)JO;
	ecpuins.table[0x71]=(t_faddrcc)JNO;
	ecpuins.table[0x72]=(t_faddrcc)JC;
	ecpuins.table[0x73]=(t_faddrcc)JNC;
	ecpuins.table[0x74]=(t_faddrcc)JZ;
	ecpuins.table[0x75]=(t_faddrcc)JNZ;
	ecpuins.table[0x76]=(t_faddrcc)JBE;
	ecpuins.table[0x77]=(t_faddrcc)JA;
	ecpuins.table[0x78]=(t_faddrcc)JS;
	ecpuins.table[0x79]=(t_faddrcc)JNS;
	ecpuins.table[0x7A]=(t_faddrcc)JP;
	ecpuins.table[0x7B]=(t_faddrcc)JNP;
	ecpuins.table[0x7C]=(t_faddrcc)JL;
	ecpuins.table[0x7D]=(t_faddrcc)JNL;
	ecpuins.table[0x7E]=(t_faddrcc)JLE;
	ecpuins.table[0x7F]=(t_faddrcc)JG;
	ecpuins.table[0x80]=(t_faddrcc)INS_80;
	ecpuins.table[0x81]=(t_faddrcc)INS_81;
	ecpuins.table[0x82]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x83]=(t_faddrcc)INS_83;
	ecpuins.table[0x84]=(t_faddrcc)TEST_RM8_M8;
	ecpuins.table[0x85]=(t_faddrcc)TEST_RM16_M16;
	ecpuins.table[0x86]=(t_faddrcc)XCHG_R8_RM8;
	ecpuins.table[0x87]=(t_faddrcc)XCHG_R16_RM16;
	ecpuins.table[0x88]=(t_faddrcc)MOV_RM8_R8;
	ecpuins.table[0x89]=(t_faddrcc)MOV_RM16_R16;
	ecpuins.table[0x8A]=(t_faddrcc)MOV_R8_RM8;
	ecpuins.table[0x8B]=(t_faddrcc)MOV_R16_RM16;
	ecpuins.table[0x8C]=(t_faddrcc)MOV_RM_SEG;
	ecpuins.table[0x8D]=(t_faddrcc)LEA_R16_M16;
	ecpuins.table[0x8E]=(t_faddrcc)MOV_SEG_RM;
	ecpuins.table[0x8F]=(t_faddrcc)POP_RM16;
	ecpuins.table[0x90]=(t_faddrcc)NOP;
	ecpuins.table[0x91]=(t_faddrcc)XCHG_CX_AX;
	ecpuins.table[0x92]=(t_faddrcc)XCHG_DX_AX;
	ecpuins.table[0x93]=(t_faddrcc)XCHG_BX_AX;
	ecpuins.table[0x94]=(t_faddrcc)XCHG_SP_AX;
	ecpuins.table[0x95]=(t_faddrcc)XCHG_BP_AX;
	ecpuins.table[0x96]=(t_faddrcc)XCHG_SI_AX;
	ecpuins.table[0x97]=(t_faddrcc)XCHG_DI_AX;
	ecpuins.table[0x98]=(t_faddrcc)CBW;
	ecpuins.table[0x99]=(t_faddrcc)CWD;
	ecpuins.table[0x9A]=(t_faddrcc)CALL_FAR;
	ecpuins.table[0x9B]=(t_faddrcc)WAIT;
	ecpuins.table[0x9C]=(t_faddrcc)PUSHF;
	ecpuins.table[0x9D]=(t_faddrcc)POPF;
	ecpuins.table[0x9E]=(t_faddrcc)SAHF;
	ecpuins.table[0x9F]=(t_faddrcc)LAHF;
	ecpuins.table[0xA0]=(t_faddrcc)MOV_AL_M8;
	ecpuins.table[0xA1]=(t_faddrcc)MOV_AX_M16;
	ecpuins.table[0xA2]=(t_faddrcc)MOV_M8_AL;
	ecpuins.table[0xA3]=(t_faddrcc)MOV_M16_AX;
	ecpuins.table[0xA4]=(t_faddrcc)MOVSB;
	ecpuins.table[0xA5]=(t_faddrcc)MOVSW;
	ecpuins.table[0xA6]=(t_faddrcc)CMPSB;
	ecpuins.table[0xA7]=(t_faddrcc)CMPSW;
	ecpuins.table[0xA8]=(t_faddrcc)TEST_AL_I8;
	ecpuins.table[0xA9]=(t_faddrcc)TEST_AX_I16;
	ecpuins.table[0xAA]=(t_faddrcc)STOSB;
	ecpuins.table[0xAB]=(t_faddrcc)STOSW;
	ecpuins.table[0xAC]=(t_faddrcc)LODSB;
	ecpuins.table[0xAD]=(t_faddrcc)LODSW;
	ecpuins.table[0xAE]=(t_faddrcc)SCASB;
	ecpuins.table[0xAF]=(t_faddrcc)SCASW;
	ecpuins.table[0xB0]=(t_faddrcc)MOV_AL_I8;
	ecpuins.table[0xB1]=(t_faddrcc)MOV_CL_I8;
	ecpuins.table[0xB2]=(t_faddrcc)MOV_DL_I8;
	ecpuins.table[0xB3]=(t_faddrcc)MOV_BL_I8;
	ecpuins.table[0xB4]=(t_faddrcc)MOV_AH_I8;
	ecpuins.table[0xB5]=(t_faddrcc)MOV_CH_I8;
	ecpuins.table[0xB6]=(t_faddrcc)MOV_DH_I8;
	ecpuins.table[0xB7]=(t_faddrcc)MOV_BH_I8;
	ecpuins.table[0xB8]=(t_faddrcc)MOV_AX_I16;
	ecpuins.table[0xB9]=(t_faddrcc)MOV_CX_I16;
	ecpuins.table[0xBA]=(t_faddrcc)MOV_DX_I16;
	ecpuins.table[0xBB]=(t_faddrcc)MOV_BX_I16;
	ecpuins.table[0xBC]=(t_faddrcc)MOV_SP_I16;
	ecpuins.table[0xBD]=(t_faddrcc)MOV_BP_I16;
	ecpuins.table[0xBE]=(t_faddrcc)MOV_SI_I16;
	ecpuins.table[0xBF]=(t_faddrcc)MOV_DI_I16;
	ecpuins.table[0xC0]=(t_faddrcc)INS_C0;
	ecpuins.table[0xC1]=(t_faddrcc)INS_C1;
	ecpuins.table[0xC2]=(t_faddrcc)RET_I8;
	ecpuins.table[0xC3]=(t_faddrcc)RET_NEAR;
	ecpuins.table[0xC4]=(t_faddrcc)LES_R16_M16;
	ecpuins.table[0xC5]=(t_faddrcc)LDS_R16_M16;
	ecpuins.table[0xC6]=(t_faddrcc)MOV_M8_I8;
	ecpuins.table[0xC7]=(t_faddrcc)MOV_M16_I16;
	ecpuins.table[0xC8]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xC9]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xCA]=(t_faddrcc)RET_I16;
	ecpuins.table[0xCB]=(t_faddrcc)RET_FAR;
	ecpuins.table[0xCC]=(t_faddrcc)INT3;
	ecpuins.table[0xCD]=(t_faddrcc)INT_I8;
	ecpuins.table[0xCE]=(t_faddrcc)INTO;
	ecpuins.table[0xCF]=(t_faddrcc)IRET;
	ecpuins.table[0xD0]=(t_faddrcc)INS_D0;
	ecpuins.table[0xD1]=(t_faddrcc)INS_D1;
	ecpuins.table[0xD2]=(t_faddrcc)INS_D2;
	ecpuins.table[0xD3]=(t_faddrcc)INS_D3;
	ecpuins.table[0xD4]=(t_faddrcc)AAM;
	ecpuins.table[0xD5]=(t_faddrcc)AAD;
	ecpuins.table[0xD6]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xD7]=(t_faddrcc)XLAT;
	ecpuins.table[0xD8]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xD9]=(t_faddrcc)INS_D9;
	ecpuins.table[0xDA]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xDB]=(t_faddrcc)INS_DB;
	ecpuins.table[0xDC]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xDD]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xDE]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xDF]=(t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xE0]=(t_faddrcc)LOOPNE;
	ecpuins.table[0xE1]=(t_faddrcc)LOOPE;
	ecpuins.table[0xE2]=(t_faddrcc)LOOP_NEAR;
	ecpuins.table[0xE3]=(t_faddrcc)JCXZ_NEAR;
	ecpuins.table[0xE4]=(t_faddrcc)IN_AL_N;
	ecpuins.table[0xE5]=(t_faddrcc)IN_AX_N;
	ecpuins.table[0xE6]=(t_faddrcc)OUT_N_AL;
	ecpuins.table[0xE7]=(t_faddrcc)OUT_N_AX;
	ecpuins.table[0xE8]=(t_faddrcc)CALL_NEAR;
	ecpuins.table[0xE9]=(t_faddrcc)JMP_NEAR_LABEL;
	ecpuins.table[0xEA]=(t_faddrcc)JMP_FAR_LABEL;
	ecpuins.table[0xEB]=(t_faddrcc)JMP_NEAR;
	ecpuins.table[0xEC]=(t_faddrcc)IN_AL_DX;
	ecpuins.table[0xED]=(t_faddrcc)IN_AX_DX;
	ecpuins.table[0xEE]=(t_faddrcc)OUT_DX_AL;
	ecpuins.table[0xEF]=(t_faddrcc)OUT_DX_AX;
	ecpuins.table[0xF0]=(t_faddrcc)LOCK;
	ecpuins.table[0xF1]=(t_faddrcc)QDX;
	ecpuins.table[0xF2]=(t_faddrcc)REPNE;
	ecpuins.table[0xF3]=(t_faddrcc)REP;
	ecpuins.table[0xF4]=(t_faddrcc)HLT;
	ecpuins.table[0xF5]=(t_faddrcc)CMC;
	ecpuins.table[0xF6]=(t_faddrcc)INS_F6;
	ecpuins.table[0xF7]=(t_faddrcc)INS_F7;
	ecpuins.table[0xF8]=(t_faddrcc)CLC;
	ecpuins.table[0xF9]=(t_faddrcc)STC;
	ecpuins.table[0xFA]=(t_faddrcc)CLI;
	ecpuins.table[0xFB]=(t_faddrcc)STI;
	ecpuins.table[0xFC]=(t_faddrcc)CLD;
	ecpuins.table[0xFD]=(t_faddrcc)STD;
	ecpuins.table[0xFE]=(t_faddrcc)INS_FE;
	ecpuins.table[0xFF]=(t_faddrcc)INS_FF;
	Ins0FTable[0x00]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x01]=(t_faddrcc)INS_0F01;
	Ins0FTable[0x02]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x03]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x04]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x05]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x06]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x07]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x08]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x09]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x0F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x10]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x11]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x12]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x13]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x14]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x15]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x16]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x17]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x18]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x19]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x1F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x20]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x21]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x22]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x23]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x24]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x25]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x26]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x27]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x28]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x29]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x2F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x30]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x31]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x32]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x33]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x34]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x35]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x36]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x37]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x38]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x39]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x3F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x40]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x41]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x42]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x43]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x44]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x45]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x46]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x47]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x48]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x49]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x4F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x50]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x51]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x52]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x53]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x54]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x55]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x56]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x57]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x58]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x59]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x5F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x60]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x61]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x62]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x63]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x64]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x65]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x66]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x67]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x68]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x69]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x6F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x70]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x71]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x72]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x73]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x74]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x75]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x76]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x77]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x78]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x79]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x7F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x80]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x81]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x82]=(t_faddrcc)JC;
	Ins0FTable[0x83]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x84]=(t_faddrcc)JZ;
	Ins0FTable[0x85]=(t_faddrcc)JNZ;
	Ins0FTable[0x86]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x87]=(t_faddrcc)JA;
	Ins0FTable[0x88]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x89]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x8F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x90]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x91]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x92]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x93]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x94]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x95]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x96]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x97]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x98]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x99]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9A]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9B]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9C]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9D]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9E]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0x9F]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA1]=(t_faddrcc)POP_FS;
	Ins0FTable[0xA2]=(t_faddrcc)CPUID;
	Ins0FTable[0xA3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA6]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA7]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xA9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAD]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xAF]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB1]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB2]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB6]=(t_faddrcc)MOVZX_RM8;
	Ins0FTable[0xB7]=(t_faddrcc)MOVZX_RM16;
	Ins0FTable[0xB8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xB9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBD]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xBF]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC1]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC2]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC6]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC7]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xC9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCD]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xCF]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD1]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD2]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD6]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD7]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xD9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDD]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xDF]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE1]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE2]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE6]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE7]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xE9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xEA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xEB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xEC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xED]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xEE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xEF]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF0]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF1]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF2]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF3]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF4]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF5]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF6]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF7]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF8]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xF9]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFA]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFB]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFC]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFD]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFE]=(t_faddrcc)UndefinedOpcode;
	Ins0FTable[0xFF]=(t_faddrcc)UndefinedOpcode;
}
void ecpuinsRefresh()
{
	ecpuinsExecIns();
	ecpuinsExecInt();
}
void ecpuinsFinal() {}
