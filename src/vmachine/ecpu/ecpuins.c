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
#include "../VCPUINS.H"
#include "ecpuins.h"
#include "ecpu.h"

t_vaddrcc Ins0FTable[0x100];

#define SAME static void

#define eIMS (vramAddr(evIP))
#define toe8 (TranslateOpcExt(0,(char **)&r8,(char **)&rm8))
#define toe16 (TranslateOpcExt(1,(char **)&r16,(char **)&rm16))
#define toe32 (TranslateOpcExt(1,(char **)&r32,(char **)&rm32))

const t_nubit16 Glbffff=0xffff;		//当寻址超过0xfffff的时候，返回的是一个可以令程序Reset的地址
t_nubit16 GlbZero=0x0;			//有些寻址用到两个偏移寄存器；有些寻址只用到一个偏移寄存器，另外一个指向这里。

t_nubit32 evIP;			//evIP读指令时的指针

t_nubit16 *rm16,*r16;			//解释寻址字节的时候用
t_nubit32 *rm32,*r32;
t_nubit8 *rm8,*r8;				//
t_nubit16 t161,t162;			//可以随便使用的
t_nubit32 t321,t322;				//
t_nubit8 t81,t82;				//

t_nubit32 t,t2,t3;				//

t_nubit16 tmpOpdSize=2;			//Operand Size，由描述符里的D位和OpdSize前缀共同决定，初始值为2
t_nubit16 tmpAddrSize=2;			//Address Size，由描述符里的D位和AddrSize前缀共同决定，初始值为2

t_cpuins ecpuins;

#define VOID static void
VOID SyncCSIP()
{
	t_vaddrcc tevip = evIP - (ecpu.cs << 4);
	ecpu.cs += tevip / 0x10000;
	ecpu.ip  = tevip % 0x10000;
}
VOID SyncEVIP()
{
	evIP = (ecpu.cs << 4) + ecpu.ip;
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
	t_nubit32 tcs=ecpu.cs;
	t_nubit32 tevIP=evIP;
	tcs<<=4;
	tevIP+=OffsetByte;
	tevIP-=tcs;
	ecpu.cs+=tevIP/0x10000;
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
		return *(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+MemoryStart);
	else
		return 0xff;
}
t_nubit8 GetM8_32(t_nubit32 off)
{
	return *(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+MemoryStart);
}
t_nubit16 GetM16_16(t_nubit16 off)
{
	if (off+(t=ecpu.overds,t<<4)<=0xfffff)
		return d_nubit16(off+(t=ecpu.overds,t<<4)+MemoryStart);
	else
		return 0xffff;
}
t_nubit16 GetM16_32(t_nubit32 off)
{	
	return d_nubit16(off+(t=ecpu.overds,t<<4)+MemoryStart);
}
t_nubit32 GetM32_16(t_nubit16 off)
{
	if (off+(t=ecpu.overds,t<<4)<=0xfffff)
		return *(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+MemoryStart);
	else
		return 0xffffffff;
}
t_nubit32 GetM32_32(t_nubit32 off)
{
	return *(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+MemoryStart);
}
VOID SetM8(t_nubit16 off, t_nubit8 val)
{
	if (off+(t=ecpu.overds,t<<4))
		*(t_nubit8 *)(off+(t=ecpu.overds,t<<4)+MemoryStart)=val;

}
VOID SetM16(t_nubit16 off, t_nubit16 val)
{
	if (off+(t=ecpu.overds,t<<4))
		d_nubit16(off+(t=ecpu.overds,t<<4)+MemoryStart)=val;
}
VOID SetM32(t_nubit16 off, t_nubit32 val)
{
	if (off+(t=ecpu.overds,t<<4))
		*(t_nubit32 *)(off+(t=ecpu.overds,t<<4)+MemoryStart)=val;
}
VOID *FindRegAddr(t_bool w,char reg)
{
	switch(reg)
	{
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
	switch(reg)
	{
	case 0:
		return &(ecpu.es);
		break;
	case 1:
		return &(ecpu.cs);
		break;
	case 2:
		return &(ecpu.ss);
		break;
	case 3:
		return &(ecpu.ds);
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
	tes=ecpu.es;
	tes<<=4;
	tss=ecpu.overss;
	tss<<=4;
	if (tmpAddrSize==2)
	{
		switch(rem)
		{
		case 0:
			*off1=&ecpu.bx;
			*off2=&ecpu.si;
			ret=MemoryStart+(t_nubit16)(ecpu.bx+ecpu.si)+(tds);
			break;
		case 1:
			*off1=&ecpu.bx;
			*off2=&ecpu.di;
			ret=MemoryStart+(t_nubit16)(ecpu.bx+ecpu.di)+(tds);
			break;
		case 2:
			*off1=&ecpu.bp;
			*off2=&ecpu.si;
			ret=MemoryStart+(t_nubit16)(ecpu.bp+ecpu.si)+(tss);
			break;
		case 3:
			*off1=&ecpu.bp;
			*off2=&ecpu.di;
			ret=MemoryStart+(t_nubit16)(ecpu.bp+ecpu.di)+(tss);
			break;
		case 4:
			*off1=&ecpu.si;	
			*off2=&GlbZero;
			ret=MemoryStart+ecpu.si+(tds);
			break;
		case 5:
			*off1=&ecpu.di;
			*off2=&GlbZero;
			ret=MemoryStart+ecpu.di+(tds);
			break;
		case 6:
			*off1=&ecpu.bp;
			*off2=&GlbZero;
			ret=MemoryStart+ecpu.bp+(tss);
			break;
		case 7:
			*off1=&ecpu.bx;
			*off2=&GlbZero;
			ret=MemoryStart+ecpu.bx+(tds);
			break;
		default:
			return 0;
		}
	}
	else
	{
		*off2=&GlbZero;
		switch(rem)
		{
		case 0:
			*off1=&ecpu.ax;
			ret=MemoryStart+(t_nubit16)ecpu.eax+(tds);
			break;
		case 1:
			*off1=&ecpu.cx;
			ret=MemoryStart+(t_nubit16)ecpu.ecx+(tds);
			break;
		case 2:
			*off1=&ecpu.dx;
			ret=MemoryStart+(t_nubit16)ecpu.edx+(tss);
			break;
		case 3:
			*off1=&ecpu.bx;
			ret=MemoryStart+(t_nubit16)ecpu.ebx+(tss);
			break;
		case 4:
			__asm nop		//SIB followed
			break;
		case 5:
			*off1=&ecpu.bp;
			ret=MemoryStart+ecpu.ebp+(tds);
			break;
		case 6:
			*off1=&ecpu.si;
			ret=MemoryStart+ecpu.esi+(tss);
			break;
		case 7:
			*off1=&ecpu.di;
			ret=MemoryStart+ecpu.edi+(tds);
			break;
		default:
			return 0;
		}
	}
	if (ret-MemoryStart<=0xfffff)
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

	switch(mod)
	{
	case 0:
		if (rem==6 && tmpAddrSize==2)
		{
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=MemoryStart;
			evIP++;
		}
		else if (rem==5 && tmpAddrSize==4)
		{
			evIP++;
			*rm=(char *)((*(t_nubit32 *)eIMS) + tds);
			*rm+=MemoryStart;
			evIP+=3;
		} 
		else
			*rm=(char *)FindRemAddr(rem,&off1,&off2);
		break;
	case 1:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;		
		if (tmpAddrSize==2)
			*rm+=(*off1+*off2+d_nsbit8(vramAddr(evIP)))-(*off1+*off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nsbit8(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;		
		if (tmpAddrSize==2)
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

	switch(mod)
	{
	case 0:
		if (rem==6 && tmpAddrSize==2)
		{
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=MemoryStart;
			evIP++;
		}
		else if (rem==5 && tmpAddrSize==4)
		{
			evIP++;
			*rm=(char *)((d_nubit16(eIMS)) + tds);
			*rm+=MemoryStart;
			evIP+=3;
		} 
		else
			*rm=(char *)FindRemAddr(rem,&off1,&off2);
		break;
	case 1:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;		
		if (tmpAddrSize==2)
			*rm+=(*off1+*off2+d_nsbit8(vramAddr(evIP)))-(*off1+*off2);		//对偏移寄存器溢出的处理
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+d_nsbit8(vramAddr(evIP)))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;	
		if (tmpAddrSize==2)
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
VOID OpcError()
{
	t_nubit8 *pc=(t_nubit8 *)vramAddr(evIP)-1;
	vapiPrint("An unkown instruction [ %2x %2x %2x %2x %2x %2x ] was read at [ %4xh:%4xh ], easyVM only support 8086 instruction set in this version. easyVM will be terminated.",*(pc),*(pc+1),*(pc+2),*(pc+3),*(pc+4),*(pc+5),ecpu.cs,ecpu.ip);
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
	while(res8)
	{
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
	ecpuins.rrm = vramGetRealAddr(ecpu.overds,vramRealWord(ecpu.cs,ecpu.ip));
	ecpu.ip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns ecpuins.rimm
	ecpuins.rimm = vramGetRealAddr(ecpu.cs,ecpu.ip);
	switch(immbit) {
	case 8:ecpu.ip += 1;break;
	case 16:ecpu.ip += 2;break;
	case 32:ecpu.ip += 4;break;
	default:CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns ecpuins.rrm and ecpuins.rr
	t_nubit8 modrm = vramRealByte(ecpu.cs,ecpu.ip++);
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.si);break;
		case 1:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.bx+ecpu.di);break;
		case 2:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.si);break;
		case 3:ecpuins.rrm = vramGetRealAddr(ecpu.overss,ecpu.bp+ecpu.di);break;
		case 4:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.si);break;
		case 5:ecpuins.rrm = vramGetRealAddr(ecpu.overds,ecpu.di);break;
		case 6:ecpuins.rrm = vramGetRealAddr(ecpu.overds,vramRealWord(ecpu.cs,ecpu.ip));ecpu.ip += 2;break;
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
			ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs,ecpu.ip);
			ecpu.ip += 1;
		} else {
			ecpuins.rrm += vramRealByte(ecpu.cs,ecpu.ip);
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
		ecpuins.rrm += (t_nsbit16)vramRealWord(ecpu.cs,ecpu.ip);ecpu.ip += 2;
		break;
	case 3:
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
		case 0:ecpuins.rr = (t_vaddrcc)(&ecpu.es);	break;
		case 1:ecpuins.rr = (t_vaddrcc)(&ecpu.cs);	break;
		case 2:ecpuins.rr = (t_vaddrcc)(&ecpu.ss);	break;
		case 3:ecpuins.rr = (t_vaddrcc)(&ecpu.ds);	break;
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
	t_nubit8 modrm = vramRealByte(ecpu.cs,ecpu.ip++);
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
		case 6:ecpuins.rrm = vramRealWord(ecpu.cs,ecpu.ip);ecpu.ip += 2;break;
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
			ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs,ecpu.ip);
			ecpu.ip += 1;
		} else {
			ecpuins.rrm += vramRealByte(ecpu.cs,ecpu.ip);
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
		ecpuins.rrm += vramRealWord(ecpu.cs,ecpu.ip);ecpu.ip += 2;
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

static void ADD(void *dest, void *src, int len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = ADD8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		bugfix(6) break;
		else ;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		bugfix(22) ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		else ecpuins.opr2 = d_nsbit8(src); /* in this case opr2 could be 0xffffffff */
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("ADD::len");break;}
	SetFlags(ADD_FLAG);
}
static void OR(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = OR8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = OR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = OR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1|ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("OR::len");break;}
	ClrBit(ecpu.flags, VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags, VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags, VCPU_EFLAGS_AF);
	SetFlags(OR_FLAG);
}
static void ADC(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = ADC8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = ADC16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = ADC16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("ADC::len");break;}
	SetFlags(ADC_FLAG);
}
static void SBB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = SBB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = SBB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = SBB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-(ecpuins.opr2+GetBit(ecpu.flags, VCPU_EFLAGS_CF))) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("SBB::len");break;}
	SetFlags(SBB_FLAG);
}
static void SUB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = SUB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("SUB::len");break;}
	SetFlags(SUB_FLAG);
}
static void AND(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = AND8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = AND16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = AND16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("AND::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(AND_FLAG);
}
static void XOR(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = XOR8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 12:
		ecpuins.bit = 16;
		//ecpuins.type = XOR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nsbit8(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = XOR16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1^ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("XOR::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(XOR_FLAG);
}
static void CMP(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nsbit8(src) & 0xff;
		bugfix(7) ecpuins.result = ((t_nubit8)ecpuins.opr1-(t_nsbit8)ecpuins.opr2)&0xff;
		else ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
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
static void STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
	bugfix(10) {
		/* add parameters flagsi, flagdi */
	} else ;
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
static void MOVS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		vramRealByte(ecpu.es,ecpu.di) = vramRealByte(ecpu.overds,ecpu.si);
		STRDIR(8,1,1);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  MOVSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es,ecpu.di) = vramRealWord(ecpu.overds,ecpu.si);
		STRDIR(16,1,1);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  MOVSW\n");
		break;
	default:CaseError("MOVS::len");break;}
	//qdcgaCheckVideoRam(vramGetRealAddr(ecpu.es, ecpu.di));
}
static void CMPS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = vramRealByte(ecpu.overds,ecpu.si);
		ecpuins.opr2 = vramRealByte(ecpu.es,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  _CMPSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = vramRealWord(ecpu.overds,ecpu.si);
		ecpuins.opr2 = vramRealWord(ecpu.es,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  _CMPSW\n");
		break;
	default:CaseError("_CMPS::len");break;}
}
static void STOS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		vramRealByte(ecpu.es,ecpu.di) = ecpu.al;
		STRDIR(8,0,1);
		/*if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
		WriteVideoRam(eCPU.di+t-0xa0000);*/
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  STOSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es,ecpu.di) = ecpu.ax;
		STRDIR(16,0,1);
		/*if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000)
		{
			for (i=0;i<tmpOpdSize;i++)
			{
				WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
			}
		}*/
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  STOSW\n");
		break;
	default:CaseError("STOS::len");break;}
}
static void LODS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpu.al = vramRealByte(ecpu.overds,ecpu.si);
		STRDIR(8,1,0);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  LODSB\n");
		break;
	case 16:
		ecpuins.bit = 16;
		ecpu.ax = vramRealWord(ecpu.overds,ecpu.si);
		STRDIR(16,1,0);
		// _vapiPrintAddr(ecpu.cs,ecpu.ip);vapiPrint("  LODSW\n");
		break;
	default:CaseError("LODS::len");break;}
}
static void SCAS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = ecpu.al;
		ecpuins.opr2 = vramRealByte(ecpu.es,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = ecpu.ax;
		ecpuins.opr2 = vramRealWord(ecpu.es,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::len");break;}
}
static void PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		ecpu.sp -= 2;
		bugfix(13) vramRealWord(ecpu.ss,ecpu.sp) = data;
		else vramRealWord(ecpu.ss,ecpu.sp) = d_nubit16(src);
		break;
	default:CaseError("PUSH::len");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&ecpu.flags,16);
	ClrBit(ecpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
	PUSH((void *)&ecpu.cs,16);
	PUSH((void *)&ecpu.ip,16);
	ecpu.ip = vramRealWord(0x0000,intid*4+0);
	ecpu.cs = vramRealWord(0x0000,intid*4+2);
	evIP = (ecpu.cs << 4) + ecpu.ip;
}
static void POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		d_nubit16(dest) = vramRealWord(ecpu.ss,ecpu.sp);
		ecpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
VOID _ADD(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len) {
	case 1:
		toe8;
		ub1 = d_nubit8(*Des);
		ub2 = d_nubit8(*Src);
		__asm {
			pushfd
			push eax
			mov al, ub1
			push ecpu.flags
			popf
			add al, ub2
			pushf
			pop ecpu.flags
			mov ub3, al
			pop eax
			popfd
		}
		if (!im((t_vaddrcc)*Des)) d_nubit8(*Des) = ub3;
		break;
	case 2:
		toe16;
		uw1 = d_nubit16(*Des);
		uw2 = d_nubit16(*Src);
		__asm {
			pushfd
			push eax
			mov ax, uw1
			push ecpu.flags
			popf
			add ax, uw2
			pushf
			pop ecpu.flags
			mov uw3, ax
			pop eax
			popfd
		}
		if (!im((t_vaddrcc)*Des)) d_nubit16(*Des) = uw3;
		break;
	case 4:
		toe32;
		udw1 = d_nubit32(*Des);
		udw2 = d_nubit32(*Src);
		__asm {
			pushfd
			push eax
			mov eax, udw1
			push ecpu.flags
			popf
			add eax, udw2
			pushf
			pop ecpu.flags
			mov udw3, eax
			pop eax
			popfd
		}
		if (!im((t_vaddrcc)*Des)) d_nubit32(*Des) = udw3;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _PUSH(void*Src,int Len)
{
	switch(Len)
	{
	case 2:
		ecpu.sp-=2;
		d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=d_nubit16(Src);
		break;
	case 4:
		ecpu.sp-=4;
		d_nubit32(vramGetRealAddr(ecpu.ss, ecpu.sp))=d_nubit32(Src);
		break;
	}
}
VOID _POP(void*Des, int Len)
{
	switch(Len)
	{
	case 2:
		d_nubit16(Des)=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
		ecpu.sp+=2;
		break;
	case 4:
		d_nubit32(Des)=d_nubit32(vramGetRealAddr(ecpu.ss, ecpu.sp));
		ecpu.sp+=4;
		break;
	}
}
VOID _OR(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)|=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)|=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)|=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _ADC(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des);
		t82=d_nubit8(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov al,t81
		__asm adc al,t82	
		__asm mov t81,al
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit8(*Des)=t81;
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des);
		t162=d_nubit16(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov ax,t161
		__asm adc ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit16(*Des)=t161;
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des);
		t322=d_nubit32(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov eax,t321
		__asm adc eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit32(*Des)=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _SBB(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des);
		t82=d_nubit8(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit8(*Des)=t81;
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des);
		t162=d_nubit16(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov ax,t161
		__asm sbb ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit16(*Des)=t161;
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des);
		t322=d_nubit32(*Src);
		__asm push ecpu.eflags
		__asm popfd
		__asm mov eax,t321
		__asm sbb eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit32(*Des)=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _AND(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)&=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)&=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)&=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _SUB(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)-=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)-=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)-=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _XOR(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)^=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)^=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)^=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID _CMP(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	t_nubit8 opr11,opr12;
	t_nubit16 opr21,opr22;
	t_nubit32 opr41,opr42;
	switch(Len) {
	case 1:
		toe8;
		opr11 = d_nubit8(*Des);
		opr12 = d_nubit8(*Src);
		__asm pushfd
		__asm push eax
		__asm push ecpu.flags				//¼Ó²Ù×÷Ö»ÐÞ¸ÄÄ³Ð©Î»£¬Èç¹ûÖ±½Ó¾Ípop ecpu.flags»á°ÑÕû¸öecpu.flags¶¼ÐÞ¸Äµô¡£
		__asm popf
		__asm mov al, opr11
		__asm cmp al, opr12
		__asm pushf
		__asm pop ecpu.flags
		__asm pop eax
		__asm popfd
		break;
	case 2:
		toe16;
		opr21 = d_nubit16(*Des);
		opr22 = d_nubit16(*Src);
		__asm pushfd
		__asm push eax
		__asm push ecpu.flags				//¼Ó²Ù×÷Ö»ÐÞ¸ÄÄ³Ð©Î»£¬Èç¹ûÖ±½Ó¾Ípop ecpu.flags»á°ÑÕû¸öecpu.flags¶¼ÐÞ¸Äµô¡£
		__asm popf
		__asm mov ax, opr21
		__asm cmp ax, opr22
		__asm pushf
		__asm pop ecpu.flags
		__asm pop eax
		__asm popfd
		break;
	case 4:
		toe32;
		opr41 = d_nubit32(*Des);
		opr42 = d_nubit32(*Src);
		__asm pushfd
		__asm push eax
		__asm push ecpu.eflags				//¼Ó²Ù×÷Ö»ÐÞ¸ÄÄ³Ð©Î»£¬Èç¹ûÖ±½Ó¾Ípop ecpu.flags»á°ÑÕû¸öecpu.flags¶¼ÐÞ¸Äµô¡£
		__asm popfd
		__asm mov eax, opr41
		__asm cmp eax, opr42
		__asm pushfd
		__asm pop ecpu.eflags
		__asm pop eax
		__asm popfd
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID INC(void*Des, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 2:
		t161=d_nubit16(Des);
		__asm push ecpu.eflags
		__asm popfd
		__asm inc t161
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit16(Des)=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		__asm push ecpu.eflags
		__asm popfd
		__asm inc t321
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit32(Des)=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID DEC(void*Des, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 2:
		t161=d_nubit16(Des);
		__asm push ecpu.eflags
		__asm popfd
		__asm dec t161
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit16(Des)=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		__asm push ecpu.eflags
		__asm popfd
		__asm dec t321
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit32(Des)=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID TEST(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des),t82=d_nubit8(*Src);
		__asm mov al,t82
		__asm push ecpu.eflags
		__asm popfd
		__asm test t81,al			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop ecpu.eflags
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des),t162=d_nubit16(*Src);
		__asm mov ax,t162
		__asm push ecpu.eflags
		__asm popfd
		__asm test t161,ax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop ecpu.eflags
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des),t322=d_nubit32(*Src);
		__asm mov eax,t322
		__asm push ecpu.eflags
		__asm popfd
		__asm test t321,eax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop ecpu.eflags
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID XCHG(void*Des, void*Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		t81=d_nubit8(Des);
		d_nubit8(Des)=d_nubit8(Src);
		d_nubit8(Src)=t81;
		break;
	case 2:
		t161=d_nubit16(Des);
		d_nubit16(Des)=d_nubit16(Src);
		d_nubit16(Src)=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		d_nubit32(Des)=d_nubit32(Src);
		d_nubit32(Src)=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID MOV(void*Des, void*Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	switch(Len)
	{
	case 1:
		d_nubit8(Des)=d_nubit8(Src);
		break;
	case 2:
		d_nubit16(Des)=d_nubit16(Src);
		break;
	case 4:
		d_nubit32(Des)=d_nubit32(Src);
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
}
VOID SHL(void*Des, t_nubit8 mb, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	t_nubit8 tSHLrm8;
	t_nubit16 tSHLrm16;
	t_nubit32 tSHLrm32;
	switch(Len)
	{
	case 1:
		tSHLrm8=d_nubit8(Des);
		__asm
		{		
			mov al,tSHLrm8
			mov cl,mb
			push ecpu.eflags
			popfd
			shl al,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm8,al
		}
		d_nubit8(Des)=tSHLrm8;
		break;
	case 2:
		tSHLrm16=d_nubit16(Des);		
		__asm
		{		
			mov ax,tSHLrm16
			mov cl,mb
			push ecpu.eflags
			popfd
			shl ax,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm16,ax
		}
		d_nubit16(Des)=tSHLrm16;
		break;
	case 4:
		tSHLrm32=d_nubit32(Des);		
		__asm
		{		
			mov eax,tSHLrm32
			mov cl,mb
			push ecpu.eflags
			popfd
			shl eax,cl
			pushfd
			pop ecpu.eflags
			mov tSHLrm32,eax
		}
		d_nubit32(Des)=tSHLrm8;
		break;
	}
	MakeBit(ecpu.flags, VCPU_EFLAGS_IF, intf);
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
	if (d_nsbit8(eIMS-1)==code)
	{
		if (J)
		{
			Jcc(1);
		}
		evIP++;
	}
	else
	{
		if (J)
		{			
			Jcc(tmpOpdSize);
		}
		evIP+=tmpOpdSize;
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
	PUSH(&ecpu.es,16);
	ci2;
}
SAME POP_ES()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.es,16);
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
	PUSH(&ecpu.cs,16);
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
	PUSH(&ecpu.ss,16);
	ci2;
}
SAME POP_SS()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.ss,16);
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
	PUSH(&ecpu.ds,16);
	ci2;
}
SAME POP_DS()
{
	ci1;
	ecpu.ip++;
	POP(&ecpu.ds,16);
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
	ecpu.overds=ecpu.es;
	ecpu.overss=ecpu.es;
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
	ecpu.overds=ecpu.cs;
	ecpu.overss=ecpu.cs;
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
	ecpu.overds=ecpu.ss;
	ecpu.overss=ecpu.ss;
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
static void CMP_R16_RM16()
{/*
	t_vaddrcc x,y;
	t_nubit16 opr21,opr22;
	t_nubit16 uw1,uw2,f = ecpu.flags;*/
	ci1;
	ecpu.ip++;/*
	x = evIP;
	y = ecpu.ip + (ecpu.cs<<4);
	if (x != y) vapiPrint("diff loc1: ev:%x,cs:%x\n",x,y);*/
	GetModRegRM(16,16);/*
	uw1 = d_nubit16(ecpuins.rr);
	uw2 = d_nubit16(ecpuins.rrm);
	toe16;
	opr21 = d_nubit16(r16);
	opr22 = d_nubit16(rm16);
	
	if (ecpuins.rr != (t_vaddrcc)r16 || ecpuins.rrm != (t_vaddrcc)rm16 ||
		opr21 != uw1 || opr22 != uw2) {
		vapiPrint("diff: r=%x,%x; rm=%x,%x; o1=%x,%x; o2=%x,%x\n",
			r16,ecpuins.rr,(t_vaddrcc)rm16-vram.base,ecpuins.rrm-vram.base,opr21,uw1,opr22,uw2);
		vapiCallBackMachineStop();
		ci2;
		return;
	}*/

	/*__asm pushfd
	__asm push eax
	__asm push ecpu.flags
	__asm popf
	__asm mov ax, opr21
	__asm cmp ax, opr22
	__asm pushf
	__asm pop ecpu.flags
	__asm pop eax
	__asm popfd
	__asm {
		pushfd
		push eax
		mov ax, uw1
		push f
		popf
		cmp ax, uw2
		pushf
		pop f
		pop eax
		popfd
	}*/
//	d_nubit16(ecpuins.rr) = uw3;
//	_CMP((void **)&r16,(void **)&rm16,2);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
/*	if (d_nubit16(ecpuins.rr) != uw3 || (f & ~VCPU_EFLAGS_IF) != (ecpu.flags & ~VCPU_EFLAGS_IF)) {
		vapiPrint("uw1=%x,uw2=%x,uw3=%x,rm=%x,r=%x;f=%x,flags=%x\n",
			uw1,uw2,uw3,d_nubit16(ecpuins.rrm),d_nubit16(ecpuins.rr),f,ecpu.flags);
		PrintFlags(f);
		PrintFlags(ecpu.flags);
		vapiCallBackMachineStop();
	}*/
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
/*SAME CMP_R16_RM16()
{
	ci1;
	ecpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	ci2;
}
*/SAME CMP_AL_I8()
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
VOID DS()
{
	ecpu.overds=ecpu.ds;
	ecpu.overss=ecpu.ds;
}
VOID AAS()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	__asm
	{
		mov ax,ecpu.ax
		push ecpu.flags
		popf
		AAS
		pushf
		pop ecpu.flags
		mov ecpu.ax,ax
	}
}
// 0x40
VOID INC_AX()
{
	INC(&ecpu.ax,tmpOpdSize);
}
VOID INC_CX()
{
	INC(&ecpu.cx,tmpOpdSize);
}
VOID INC_DX()
{
	INC(&ecpu.dx,tmpOpdSize);
}
VOID INC_BX()
{
	INC(&ecpu.bx,tmpOpdSize);
}
VOID INC_SP()
{
	INC(&ecpu.sp,tmpOpdSize);
}
VOID INC_BP()
{
	INC(&ecpu.bp,tmpOpdSize);
}
VOID INC_SI()
{
	INC(&ecpu.si,tmpOpdSize);
}
VOID INC_DI()
{
	INC(&ecpu.di,tmpOpdSize);
}
VOID DEC_AX()
{
	DEC(&ecpu.ax,tmpOpdSize);
}
VOID DEC_CX()
{
	DEC(&ecpu.cx,tmpOpdSize);
}
VOID DEC_DX()
{
	DEC(&ecpu.dx,tmpOpdSize);
}
VOID DEC_BX()
{
	DEC(&ecpu.bx,tmpOpdSize);
}
VOID DEC_SP()
{
	DEC(&ecpu.sp,tmpOpdSize);
}
VOID DEC_BP()
{
	DEC(&ecpu.bp,tmpOpdSize);
}
VOID DEC_SI()
{
	DEC(&ecpu.si,tmpOpdSize);
}
VOID DEC_DI()
{
	DEC(&ecpu.di,tmpOpdSize);
}
// 0x50
VOID PUSH_AX()
{
	_PUSH(&ecpu.ax,tmpOpdSize);
}
VOID PUSH_CX()
{
	_PUSH(&ecpu.cx,tmpOpdSize);
}
VOID PUSH_DX()
{
	_PUSH(&ecpu.dx,tmpOpdSize);
}
VOID PUSH_BX()
{
	_PUSH(&ecpu.bx,tmpOpdSize);
}
VOID PUSH_SP()
{
	t_nubit16 tsp=ecpu.sp;
	_PUSH(&tsp,tmpOpdSize);			//不能PUSH修改之后的值
}
VOID PUSH_BP()
{
	_PUSH(&ecpu.bp,tmpOpdSize);
}
VOID PUSH_SI()
{
	_PUSH(&ecpu.si,tmpOpdSize);
}
VOID PUSH_DI()
{
	_PUSH(&ecpu.di,tmpOpdSize);
}
VOID POP_AX()
{
	_POP(&ecpu.ax,tmpOpdSize);
}
VOID POP_CX()
{
	_POP(&ecpu.cx,tmpOpdSize);
}
VOID POP_DX()
{
	_POP(&ecpu.dx,tmpOpdSize);
}
VOID POP_BX()
{
	_POP(&ecpu.bx,tmpOpdSize);
}
VOID POP_SP()
{
	_POP(&ecpu.sp,tmpOpdSize);
	ecpu.sp-=tmpOpdSize;				//_POP()里是先赋值后加的，所以这里要减回去
}
VOID POP_BP()
{
	_POP(&ecpu.bp,tmpOpdSize);
}
VOID POP_SI()
{
	_POP(&ecpu.si,tmpOpdSize);
}
VOID POP_DI()
{
	_POP(&ecpu.di,tmpOpdSize);
}
// 0x60
VOID ARPL()
{
	toe16;
	if (((*rm16)&3)<((*r16)&3))
	{
		ecpu.eflags|=ZF;
		(*rm16)&=0xfc;
		(*rm16)|=(*r16)&3;
	}
	else
		ecpu.eflags&=~ZF;
}
VOID OpdSize()
{
	tmpOpdSize=6-tmpOpdSize;
	ecpuinsExecIns();
	tmpOpdSize=6-tmpOpdSize;
}
VOID AddrSize()
{
	tmpAddrSize=6-tmpAddrSize;
	ecpuinsExecIns();
	tmpAddrSize=6-tmpAddrSize;
}
VOID PUSH_I16()
{	
	_PUSH((void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
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
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe8;
	switch(oce) {
	case 0:
		__asm push ecpu.flags
		__asm popf
		*rm8+=d_nsbit8(eIMS);
		break;
	case 1:
		__asm push ecpu.flags
		__asm popf
		*rm8|=d_nsbit8(eIMS);
		break;
	case 2:
		t81=*rm8;
		t82=d_nsbit8(eIMS);
		__asm push ecpu.flags
		__asm popf
		//*rm8+=(d_nsbit8(eIMS)+((ecpu.flags & CF)!=0));	//在VC6里，逻辑值“真”＝1
		__asm mov al,t81
		__asm adc al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 3:
		t81=*rm8;
		t82=d_nsbit8(eIMS);
		__asm push ecpu.flags
		__asm popf
		//*rm8-=(d_nsbit8(eIMS)+((ecpu.flags & CF)!=0));			
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 4:
		__asm push ecpu.flags
		__asm popf
		*rm8&=d_nsbit8(eIMS);
		break;
	case 5:
		__asm push ecpu.flags
		__asm popf
		*rm8-=d_nsbit8(eIMS);
		break;
	case 6:
		__asm push ecpu.flags
		__asm popf
		*rm8^=d_nsbit8(eIMS);
		break;
	case 7:
		t81 = *rm8;
		t82 = d_nsbit8(eIMS);
		__asm push ecpu.flags
		__asm popf
		__asm mov al, t81
		__asm cmp al, t82
//		*rm8-=d_nsbit8(eIMS);
		__asm pushf
		__asm pop ecpu.flags
		evIP++;
		return;
	}
	__asm pushf
	__asm pop ecpu.flags
	//if (oce==7)
	//	*rm8+=d_nsbit8(eIMS);
	evIP++;
}
VOID INS_81()	//这里是以81开头的指令的集。
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit32 tevIP, teIMS;
	oce>>=3;
	// 这里先保存IP，再toe16，再eIMS，然后又恢复IP
	// 这样做是因为，这条指令跟在Opcode后面的就是寻址，然后才是eIMS立即数。但是下面8个操作里面就有toe16，所以又要把IP恢复到toe16之前。
	// 其实下面8个操作里面是不应该有toe16的，不过已经写了很多，改起来太麻烦，就不改了。
	tevIP=evIP;
	toe16;
	teIMS=eIMS;
	evIP=tevIP;
	switch(oce)
	{
	case 0:
		_ADD((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 1:
		_OR((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 2:
		_ADC((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 3:
		_SBB((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 4:
		_AND((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 5:
		_SUB((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 6:
		_XOR((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 7:
		_CMP((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	}
	evIP+=tmpOpdSize;
}
VOID INS_82()	//这里是以82开头的指令的集。
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit16 tfg;
	oce>>=3;
	toe8;
	tfg=d_nsbit8(eIMS);			//这个强制转换就是按符号扩展完成的
// 	if (tfg & 0x0080)		//符号扩展
// 		tfg+=0xff00;
	switch(oce)
	{
	case 0:
		__asm push ecpu.flags
		__asm popf
		*rm8+=tfg;
		break;
	case 1:
		__asm push ecpu.flags
		__asm popf
		*rm8|=tfg;
		break;
	case 2:
		t81=*rm8;
		t82=(t_nubit8)tfg;
		__asm push ecpu.flags
		__asm popf
		//*rm8+=(tfg+((ecpu.flags & CF)!=0));	//在VC6里，逻辑值“真”＝1
		__asm mov al,t81
		__asm adc al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 3:
		t81=*rm8;
		t82=(t_nubit8)tfg;
		__asm push ecpu.flags
		__asm popf
		//*rm8-=(tfg+((ecpu.flags & CF)!=0));			
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 4:
		__asm push ecpu.flags
		__asm popf
		*rm8&=tfg;			
		break;
	case 5:
		__asm push ecpu.flags
		__asm popf
		*rm8-=tfg;			
		break;
	case 6:
		__asm push ecpu.flags
		__asm popf
		*rm8^=tfg;			
		break;
	case 7:
		__asm push ecpu.flags
		__asm popf
		*rm8-=tfg;					
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	if (oce==7)
		*rm8+=tfg;
	evIP++;
}
VOID INS_83()	//这里是以83开头的指令的集。
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	t_nubit32 tevIP;
	t_nubit16 tfg;
	t_nubit32 ptfg;
	oce>>=3;
	tevIP=evIP;
	toe16;
	tfg=d_nsbit8(eIMS);			//这个强制转换本身就是符号扩展的
	ptfg=(t_nubit32)&tfg;
	evIP=tevIP;
	switch(oce)
	{
	case 0:
		_ADD((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 1:
		_OR((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 2:
		_ADC((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 3:
		_SBB((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 4:
		_AND((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 5:
		_SUB((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 6:
		_XOR((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 7:
		_CMP((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	}
	evIP++;
}
VOID TEST_RM8_M8()
{
	TEST((void**)&rm8,(void**)&r8,1);
}
VOID TEST_RM16_M16()
{
	TEST((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID XCHG_R8_RM8()
{
	toe8;
	XCHG(r8,rm8,1);
}
VOID XCHG_R16_RM16()
{
	toe16;
	XCHG(r16,rm16,tmpOpdSize);
}
VOID MOV_RM8_R8()
{
	toe8;
	MOV(rm8,r8,1);
}
VOID MOV_RM16_R16()
{
	toe16;
	MOV(rm16,r16,tmpOpdSize);
}
VOID MOV_R8_RM8()
{
	toe8;
	MOV(r8,rm8,1);
}
VOID MOV_R16_RM16()
{
	toe16;
	MOV(r16,rm16,tmpOpdSize);
}
VOID MOV_RM_SEG()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*r16=*rm16;
	MOV(r16,rm16,2);
}
VOID LEA_R16_M16()
{
	t_nubit8 mod, rem;
	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;
	toe16;
	switch(rem)
	{
	case 0:
	case 1:
	case 4:
	case 5:
	case 7:
		*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=ecpu.overds,t<<4));
		break;
	case 2:
	case 3:
		*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=ecpu.overss,t<<4));
		break;
	case 6:
		if (mod==0)		
			*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=ecpu.overds,t<<4));
		else
			*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=ecpu.overss,t<<4));
		break;
	default:
		return ;
	}	
}
VOID MOV_SEG_RM()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*rm16=*r16;
	MOV(rm16,r16,2);
}
VOID POP_RM16()
{
	toe16;
 	_POP((void*)rm16,tmpOpdSize);
}
// 0x90
VOID NOP()
{
	__asm nop
}
VOID XCHG_CX_AX()
{
	XCHG(&ecpu.cx,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_DX_AX()
{
	XCHG(&ecpu.dx,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_BX_AX()
{
	XCHG(&ecpu.bx,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_SP_AX()
{
	XCHG(&ecpu.sp,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_BP_AX()
{
	XCHG(&ecpu.bp,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_SI_AX()
{
	XCHG(&ecpu.si,&ecpu.ax,tmpOpdSize);
}
VOID XCHG_DI_AX()
{
	XCHG(&ecpu.di,&ecpu.ax,tmpOpdSize);
}
VOID CBW()
{
	switch(tmpOpdSize)
	{
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
	switch(tmpOpdSize)
	{
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
	d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.cs;
	ecpu.sp-=2;
	d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
	ecpu.ip=d_nubit16(eIMS);
	evIP+=2;
	ecpu.cs=d_nubit16(eIMS);
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID WAIT()
{
	__asm nop
}
VOID PUSHF()
{
	_PUSH(&ecpu.flags,tmpOpdSize);
}
VOID POPF()
{
	_POP(&ecpu.flags,tmpOpdSize);
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
	MOV(&ecpu.al,&t81,1);	
	evIP+=2;
}
VOID MOV_AX_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		t161=GetM16_16(d_nubit16(eIMS));
		MOV(&ecpu.ax,&t161,tmpOpdSize);
		break;
	case 4:
		t321=GetM32_16(d_nubit16(eIMS));
		MOV(&ecpu.eax,&t321,tmpOpdSize);
		break;
	}	
	evIP+=tmpAddrSize;
}
VOID MOV_M8_AL()
{
	SetM8(d_nubit16(eIMS),ecpu.al);
	evIP+=2;
}
VOID MOV_M16_AX()
{
	switch(tmpOpdSize)
	{
	case 2:
		SetM16(d_nubit16(eIMS),ecpu.ax);
		break;
	case 4:
		SetM32(d_nubit16(eIMS),ecpu.eax);
		break;
	}	
	evIP+=tmpAddrSize;
}

SAME MOVSB()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			MOVS(8);
			ecpu.cx--;
		}
	}
}
SAME MOVSW()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			MOVS(16);
			ecpu.cx--;
		}
	}
}
SAME _CMPSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) CMPS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			CMPS(8);
			ecpu.cx--;
			if((ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) || (ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
	ci2;
}
SAME _CMPSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) CMPS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			CMPS(16);
			ecpu.cx--;
			if((ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) || (ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
	ci2;
}
VOID TEST_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)eIMS;
	TEST((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID TEST_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)eIMS;
	TEST((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}

SAME STOSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			STOS(8);
			ecpu.cx--;
		}
	}
	ci2;
}
SAME STOSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			STOS(16);
			ecpu.cx--;
		}
	}
	ci2;
}
SAME LODSB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			LODS(8);
			ecpu.cx--;
		}
	}
	ci2;
}
SAME LODSW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			LODS(16);
			ecpu.cx--;
		}
	}
	ci2;
}
SAME SCASB()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) SCAS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			SCAS(8);
			ecpu.cx--;
			if((ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) || (ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
	ci2;
}
SAME SCASW()
{
	ci1;
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) SCAS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			SCAS(16);
			ecpu.cx--;
			if((ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) || (ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF))) break;
		}
	}
	ci2;
}

VOID MOV_AL_I8()
{
	MOV(&ecpu.al,(void*)eIMS,1);
	evIP++;
}
VOID MOV_CL_I8()
{
	MOV(&ecpu.cl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_DL_I8()
{
	MOV(&ecpu.dl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_BL_I8()
{
	MOV(&ecpu.bl,(void*)eIMS,1);
	evIP++;
}
VOID MOV_AH_I8()
{
	MOV(&ecpu.ah,(void*)eIMS,1);
	evIP++;
}
VOID MOV_CH_I8()
{
	MOV(&ecpu.ch,(void*)eIMS,1);
	evIP++;
}
VOID MOV_DH_I8()
{
	MOV(&ecpu.dh,(void*)eIMS,1);
	evIP++;
}
VOID MOV_BH_I8()
{
	MOV(&ecpu.bh,(void*)eIMS,1);
	evIP++;
}
VOID MOV_AX_I16()
{
	MOV(&ecpu.ax,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_CX_I16()
{
	MOV(&ecpu.cx,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_DX_I16()
{
	MOV(&ecpu.dx,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_BX_I16()
{
	MOV(&ecpu.bx,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_SP_I16()
{
	MOV(&ecpu.sp,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_BP_I16()
{
	MOV(&ecpu.bp,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_SI_I16()
{
	MOV(&ecpu.si,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID MOV_DI_I16()
{
	MOV(&ecpu.di,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
// 0xC0
VOID INS_C0()
{
	t_nubit8 t,teIMS;
	t_nubit16 teIMS16;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	teIMS=d_nubit8(eIMS);
	teIMS16 = teIMS;
	__asm push teIMS16
	__asm push ecpu.flags
	switch(oce)
	{
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
		OpcError();
		break;
	case 7:
		__asm popf
		__asm pop ecx
		__asm sar t,cl
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	*rm8=t;
	evIP++;
}
VOID INS_C1()
{
	t_nubit16 t, teIMS16;
	t_nubit32 t2;
	t_nubit8 teIMS;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	switch (tmpOpdSize)
	{
	case 2:
		toe16;
		teIMS=d_nubit8(eIMS);
		t=*rm16;
		teIMS16 = teIMS;
		__asm push teIMS16
		__asm push ecpu.eflags
		switch(oce)
		{
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
			OpcError();
			break;
		case 7:
			__asm popfd
			__asm pop ecx
			__asm sar t,cl
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		*rm16=t;
		break;
	case 4:
		toe32;
		teIMS=d_nubit8(eIMS);
		teIMS16 = teIMS;
		t2=*rm32;
		__asm push teIMS16
		__asm push ecpu.eflags
		switch(oce)
		{
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
			OpcError();
			break;
		case 7:
			__asm popfd
			__asm pop ecx
			__asm sar t2,cl
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		*rm32=t2;
		break;
	}
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
	SHL(rm16,teIMS,tmpOpdSize);
	evIP++;
}
VOID RET_I8()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;	
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID RET_NEAR()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID LES_R16_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		*r16=*rm16;
		ecpu.es=d_nubit16(rm16+1);
		break;
	case 4:
		toe32;
		*r32=*rm32;
		ecpu.es=d_nubit16(rm32+1);
		break;
	}
}
VOID LDS_R16_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		toe16;	
		*r16=*rm16;
		ecpu.ds=d_nubit16(rm16+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	case 4:
		toe32;	
		*r32=*rm32;
		ecpu.ds=d_nubit16(rm32+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	}
}
VOID MOV_M8_I8()
{
	toe8;	
	MOV(rm8,(void*)eIMS,1);
	evIP++;
}
VOID MOV_M16_I16()
{
	toe16;	
	MOV(rm16,(void*)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
VOID RET_I16()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;	
	ecpu.cs=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;	
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID RET_FAR()
{
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	ecpu.cs=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
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
	ecpu.ip=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	ecpu.cs=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	ecpu.flags=d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp));
	ecpu.sp+=2;		
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
}
// 0xD0
VOID INS_D0()
{
	t_nubit8 t;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm push ecpu.flags;
	switch(oce)
	{
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
		OpcError();
		break;
	case 7:
		__asm popf
		__asm sar t,1
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	*rm8=t;
}
VOID INS_D1()
{
	t_nubit16 t;
	t_nubit32 t2;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	switch (tmpOpdSize)
	{
	case 2:
		toe16;
		t=*rm16;
		__asm push ecpu.eflags;
		switch(oce)
		{
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
			OpcError();
			break;
		case 7:
			__asm popfd
			__asm sar t,1
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		*rm16=t;
		break;
	case 4:
		toe32;
		t2=*rm32;
		__asm push ecpu.eflags;
		switch(oce)
		{
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
			OpcError();
			break;
		case 7:
			__asm popfd
			__asm sar t2,1
			break;
		}
		__asm pushfd
		__asm pop ecpu.eflags
		*rm32=t2;
		break;
	}
}
VOID INS_D2()
{
	t_nubit8 t;
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm mov cl,ecpu.cl;
	__asm push ecpu.flags;
	switch(oce)
	{
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
		OpcError();
		break;
	case 7:
		__asm popf
		__asm sar t,cl
		break;
	}
	__asm pushf
	__asm pop ecpu.flags
	*rm8=t;
}
VOID INS_D3()
{
	char oce;
	t_nubit16 t;
	t_nubit32 t2;
	oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		t=*rm16;
		switch(oce)
		{
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
			OpcError();
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
		*rm16=t;
		break;	
	case 4:
		toe32;
		t2=*rm32;
		switch(oce)
		{
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
			OpcError();
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
		*rm32=t2;
		break;
	}
}
VOID AAM()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	if ((d_nubit8(eIMS))==0)
		INT(0x00);
	else
	__asm
	{
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
	t_bool intf = GetBit(ecpu.flags, VCPU_EFLAGS_IF);
	__asm
	{
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
	d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
	ecpu.ip+=(d_nubit16(eIMS));
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID JMP_NEAR_LABEL()	//立即数是两字节的
{
	LongCallNewIP(2);
	ecpu.ip+=(d_nubit16(eIMS));
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID JMP_FAR_LABEL()
{
	LongCallNewIP(4);
	ecpu.ip=d_nubit16(eIMS);
	evIP+=2;
	ecpu.cs=d_nubit16(eIMS);
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID JMP_NEAR()			//立即数是一字节的
{	
	LongCallNewIP(1);
	ecpu.ip+=(d_nsbit8(eIMS));
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
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
VOID INS_F6()
{
	char oce,trm8,tc;
	oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe8;
	trm8=*rm8;
	tc=d_nsbit8(eIMS);
	switch(oce) {
	case 0:
		__asm push ecpu.flags
		__asm mov al,tc
		__asm popf
		__asm test trm8,al
		__asm pushf
		__asm pop ecpu.flags
		evIP++;
		break;
	case 1:
		OpcError();
		break;
	case 2:
		__asm push ecpu.flags
		__asm popf
		__asm not trm8
		__asm pushf
		__asm pop ecpu.flags
		*rm8=trm8;
		break;
	case 3:
		__asm push ecpu.flags
		__asm popf
		__asm neg trm8
		__asm pushf
		__asm pop ecpu.flags
		*rm8=trm8;
		break;
	case 4:
		__asm {
			pushfd
			push eax
			mov al,ecpu.al
			push ecpu.flags
			popf
			mul trm8
			pushf
			pop ecpu.flags
			mov ecpu.ax,ax
			pop eax
			popfd
		}
		break;
	case 5:
		__asm {
			pushfd
			push eax
			mov al,ecpu.al
			push ecpu.flags
			popf
			imul trm8
			pushf
			pop ecpu.flags
			mov ecpu.ax,ax
			pop eax
			popfd
		}
		break;
	case 6:
		if (trm8)
			__asm {
				pushfd
				push eax
				mov ax, ecpu.ax
				push ecpu.flags
				popf	
				div trm8
				pushf
				pop ecpu.flags
				mov ecpu.ax, ax
				pop eax
				popfd
			}
		else INT(0x00);
		break;
	case 7:
		if (trm8)
			__asm {
				pushfd
				push eax
				mov ax,ecpu.ax
				push ecpu.flags
				popf
				idiv trm8	
				pushf
				pop ecpu.flags
				mov ecpu.ax,ax
				pop eax
				popfd
			}
		else INT(0x00);
		break;
	}
}
VOID INS_F7()
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	short tc;
	short trm16;
	int tc2;
	int trm32;
	oce>>=3;
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		trm16=*rm16;
		tc=d_nubit16(eIMS);
		__asm push ecpu.flags
		switch(oce)
		{
		case 0:
			__asm mov ax,tc
			__asm popf
			__asm test trm16,ax
			break;
		case 1:
			OpcError();
			break;
		case 2:
			__asm popf
			__asm not trm16	
			*rm16=trm16;			//这两个都是要改变trm16的值的
			break;
		case 3:
			__asm popf
			__asm neg trm16	
			*rm16=trm16;
			break;
		case 4:
			__asm mov ax,ecpu.ax
			__asm popf
			__asm mul trm16	
			__asm mov ecpu.ax,ax
			__asm mov ecpu.dx,dx
			break;
		case 5:
			__asm mov ax,ecpu.ax
			__asm popf
			__asm imul trm16	
			__asm mov ecpu.ax,ax
			__asm mov ecpu.dx,dx
			break;
		case 6:
			if (trm16!=0)			//这里只针对8086的
			{		
				__asm mov ax,ecpu.ax
				__asm mov dx,ecpu.dx
				__asm popf
				__asm div trm16	
				__asm mov ecpu.ax,ax
				__asm mov ecpu.dx,dx
			}
			else
			{
				INT(0x00);
			}
			break;
		case 7:
			if (trm16!=0)
			{		
				__asm mov ax,ecpu.ax
				__asm mov dx,ecpu.dx
				__asm popf
				__asm idiv trm16	
				__asm mov ecpu.ax,ax
				__asm mov ecpu.dx,dx
			}
			else
			{
				INT(0x00);
			}
			break;
		}
		__asm pushf				//这里一定要尽快把Flags搞出来，以免运算完后被修改
		__asm pop ecpu.flags
		//*rm16=trm16;				//这里不能这样写，因为有时候那个rm16正好就是div的结果，一改回原来的trm16，结果就被覆盖了
		//evIP++;
		if (oce==0)
			evIP+=tmpOpdSize;
		break;
	case 4:
		toe32;
		trm32=*rm32;
		tc2=*(t_nubit32 *)eIMS;
		__asm push ecpu.eflags
		switch(oce)
		{
		case 0:
			__asm mov eax,tc2
			__asm popfd
			__asm test trm32,eax
			break;
		case 1:
			OpcError();
			break;
		case 2:
			__asm popfd
			__asm not trm32	
			*rm32=trm32;			//这两个都是要改变trm16的值的
			break;
		case 3:
			__asm popfd
			__asm neg trm32	
			*rm32=trm32;
			break;
		case 4:
			__asm mov eax,ecpu.eax
			__asm popfd
			__asm mul trm32	
			__asm mov ecpu.eax,eax
			__asm mov ecpu.edx,edx
			break;
		case 5:
			__asm mov eax,ecpu.eax
			__asm popfd
			__asm imul trm32	
			__asm mov ecpu.eax,eax
			__asm mov ecpu.edx,edx
			break;
		case 6:
			if (trm32!=0)			//这里只针对8086的
			{
				__asm mov eax,ecpu.eax
				__asm mov edx,ecpu.edx
				__asm popfd
				__asm div trm32	
				__asm mov ecpu.eax,eax
				__asm mov ecpu.edx,edx
			}
			else
			{
				INT(0x00);
			}
			break;
		case 7:
			if (trm32!=0)
			{
				__asm mov eax,ecpu.eax
				__asm mov edx,ecpu.edx
				__asm popfd
				__asm idiv trm32	
				__asm mov ecpu.eax,eax
				__asm mov ecpu.edx,edx
			}
			else
			{
				INT(0x00);
			}
			break;
		}
		__asm pushfd				//这里一定要尽快把Flags搞出来，以免运算完后被修改
		__asm pop ecpu.eflags
		if (oce==0)
			evIP+=tmpOpdSize;
		break;
	}
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
}
VOID CLD()
{
	ecpu.flags &= ~DF;
}
VOID STD()
{
	ecpu.flags |= DF;
}
VOID INS_FE()
{	t_nubit8 trm8;
	char oce,mod,rem;
	oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	mod&=0x3;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;
	if (oce>=2 && oce<=5)
	{	if (mod==0 && rem!=6)
			LongCallNewIP(1);
		else if (mod==1)
			LongCallNewIP(2);
		else if (mod==2 || mod==0 && rem==6)
			LongCallNewIP(3);
		else if (mod==3)
			LongCallNewIP(1);
	}
	toe8;
	trm8=*rm8;
	//__asm push ecpu.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
	switch(oce)
	{
	case 0:
		__asm push ecpu.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
		__asm popf
		//(*rm8)++;	
		__asm inc trm8				//++会被编译成add，和inc所修改的标志位不一样
		__asm pushf
		__asm pop ecpu.flags
		*rm8=trm8;
		break;
	case 1:
		__asm push ecpu.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
		__asm popf
		//(*rm8)--;
		__asm dec trm8
		__asm pushf
		__asm pop ecpu.flags
		*rm8=trm8;
		break;
	case 2:
		//__asm popf
		ecpu.sp-=2;
		d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
		JMP_NEAR_LABEL();		
		break;
	case 3:
		//__asm popf
		ecpu.sp-=2;
		d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
		ecpu.ip=*rm8;
		ecpu.cs=*(rm8+1);
		evIP+=2;		
		evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
		break;
	case 4:
		//__asm popf
		evIP+=(*rm8+3);
		break;
	case 5:
		//__asm popf
		ecpu.ip=*rm8;
		evIP+=2;	
		ecpu.cs=*(rm8+1);
		evIP+=2;
		evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
		break;
	case 6:
		//__asm popf
		ecpu.sp-=2;
		d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=(t_nubit16)*rm8;
		break;
	case 7:
		OpcError();					
		break;
	}
	//evIP++;
}
VOID INS_FF()
{
	t_nubit16 trm16;
	t_nubit32 trm32;
	char oce,mod,rem;
	oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	mod=d_nsbit8(vramAddr(evIP)) & 0xc0;
	mod>>=6;
	mod&=0x3;
	rem=d_nsbit8(vramAddr(evIP)) & 0x07;
	if (oce>=2 && oce<=5)
	{	
		if (mod==0 && rem!=6)
			LongCallNewIP(1);
		else if (mod==1)
			LongCallNewIP(2);
		else if (mod==2 || mod==0 && rem==6)
			LongCallNewIP(3);
		else if (mod==3)
			LongCallNewIP(1);
	}
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		trm16=*rm16;
		break;
	case 4:
		toe32;
		trm32=*rm32;
		break;
	}	
	switch(oce)
	{
	case 0:
		switch (tmpOpdSize)
		{
		case 2:
			__asm push ecpu.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popf
			//(*rm16)++;	
			__asm inc trm16
			__asm pushf
			__asm pop ecpu.flags
			*rm16=trm16;
			break;
		case 4:
			__asm push ecpu.eflags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popfd
			//(*rm16)++;	
			__asm inc trm32
			__asm pushfd
			__asm pop ecpu.eflags
			*rm32=trm32;
			break;		
		}
		break;		
	case 1:
		switch (tmpOpdSize)
		{
		case 2:
			__asm push ecpu.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popf
			//(*rm16)--;
			__asm dec trm16
			__asm pushf
			__asm pop ecpu.flags
			*rm16=trm16;
			break;
		case 4:
			__asm push ecpu.eflags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popfd
			//(*rm16)--;
			__asm dec trm32
			__asm pushfd
			__asm pop ecpu.eflags
			*rm32=trm32;
			break;
		}
		break;
	case 2:
		switch (tmpOpdSize)
		{
		case 2:
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
			ecpu.ip=*rm16;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		case 4:
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
			ecpu.ip=*rm32;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		}
		break;
	case 3:
		switch (tmpOpdSize)
		{
		case 2:
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.cs;
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
			ecpu.ip=*rm16;
			ecpu.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		case 4:
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.cs;
			ecpu.sp-=2;
			d_nubit16(vramGetRealAddr(ecpu.ss, ecpu.sp))=ecpu.ip;
			ecpu.ip=*rm32;
			ecpu.cs=*(rm32+1);
			evIP+=tmpOpdSize;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		}
		break;
	case 4:
		switch (tmpOpdSize)
		{
		case 2:
			ecpu.ip=(*rm16);
			evIP=(t=ecpu.cs,t<<=4)+ecpu.ip;
			break;
		case 4:
			ecpu.ip=(*rm32);
			evIP=(t=ecpu.cs,t<<=4)+ecpu.ip;
			break;
		}
		break;
	case 5:
		switch (tmpOpdSize)
		{
		case 2:
			ecpu.ip=*rm16;
			evIP+=2;		
			ecpu.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
			break;
		case 4:
			ecpu.ip=*rm32;
			evIP+=tmpOpdSize;		
			ecpu.cs=*(rm32+1);
			evIP+=tmpOpdSize;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
			break;
		}
		break;
	case 6:
		switch (tmpOpdSize)
		{
		case 2:
			_PUSH(rm16,tmpOpdSize);
			break;
		case 4:
			_PUSH(rm32,tmpOpdSize);
			break;
		}		
		break;
	case 7:
		OpcError();					
		break;
	}

	//evIP++;
}
VOID INS_0F()
{
	t_nubit8 OpC=*(t_nubit8 *)(vramAddr(evIP));
	t_nubit32 tcs,InsFucAddr;
	evIP++;
	InsFucAddr=Ins0FTable[OpC];
	__asm call InsFucAddr;
	tcs=ecpu.cs;
	ecpu.ip=(evIP - (tcs << 4)) % 0x10000;
	ecpu.overds=ecpu.ds;
	ecpu.overss=ecpu.ss;
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
	for (i=0;i<4;i++)
	{
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
	for (i=0;i<4;i++)
	{
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
	for (i=0;i<4;i++)
	{
		ecpu.xmm[a].ip[i]&=ecpu.xmm[b].ip[i];
	}
	__asm pushf
	__asm pop ecpu.eflags;
	evIP++;	
}
VOID BSF()
{
	int temp;
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		if (*rm16==0)
		{
			ecpu.eflags|=ZF;
		}
		else
		{
			ecpu.eflags&=~ZF;
			temp=0;
			while (!Bit(rm16,temp))
			{
				temp++;
				*r16=temp;
			}
		}
		break;
	case 4:
		toe32;
		if (*rm32==0)
		{
			ecpu.eflags|=ZF;
		}
		else
		{
			ecpu.eflags&=~ZF;
			temp=0;
			while (!Bit(rm32,temp))
			{
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
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		if (*rm16==0)
		{
			ecpu.eflags|=ZF;
		}
		else
		{
			ecpu.eflags&=~ZF;
			temp=tmpOpdSize-1;
			while (!Bit(rm16,temp))
			{
				temp--;
				*r16=temp;
			}
		}
		break;
	case 4:
		toe32;
		if (*rm32==0)
		{
			ecpu.eflags|=ZF;
		}
		else
		{
			ecpu.eflags&=~ZF;
			temp=tmpOpdSize-1;
			while (!Bit(rm32,temp))
			{
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
	if (OpC<0xc0)
	{
		oce=OpC & 0x38;
		oce>>=3;
		mod=OpC & 0xc0;
		mod>>=6;
		mod&=0x3;
		rem=OpC & 0x07;
		toe16;
		switch(oce)
		{
		case 7:
			*rm16=ecpu.FpuSR;
			break;
		default:
			OpcError();
			break;
		}
	}
}
VOID INS_DB()
{
	t_nubit8 OpC=*(t_nubit8 *)(vramAddr(evIP));
	evIP++;
	switch(OpC)
	{
	case 0xe3:
		FINIT();
		break;
	default:
		OpcError();
	}
		
}
VOID MOVZX_RM8()
{
	toe8;
	t321=*rm8;
	MOV(r16,&t321,tmpOpdSize);	
}
VOID MOVZX_RM16()
{
	toe16;
	t321=*rm16;
	MOV(r16,&t321,tmpOpdSize);	
}
VOID POP_FS()
{
	_POP(&ecpu.fs,2);
}
VOID INS_0F01()
{
	char oce=d_nsbit8(vramAddr(evIP)) & 0x38;
	oce>>=3;
	toe16;
	switch(oce)
	{
	case 0:
		OpcError();
		break;
	case 1:
		OpcError();
		break;
	case 2:
		OpcError();
		break;
	case 3:
		OpcError();
		break;
	case 4:
		*rm16=(t_nubit16)ecpu.CR[0];
		break;
	case 5:
		OpcError();
		break;
	case 6:
		OpcError();
		break;
	case 7:
		OpcError();
		break;
	}
}
// SPECIAL
VOID QDX()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	qdbiosExecInt(d_nubit8(eIMS));
	MakeBit(vramRealWord(ecpu.ss,ecpu.sp + 4), VCPU_EFLAGS_ZF, GetBit(ecpu.eflags, VCPU_EFLAGS_ZF));
	MakeBit(vramRealWord(ecpu.ss,ecpu.sp + 4), VCPU_EFLAGS_CF, GetBit(ecpu.eflags, VCPU_EFLAGS_CF));
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
	ecpu.overds = ecpu.ds;
	ecpu.overss = ecpu.ss;
	ecpuins.prefix_rep = PREFIX_REP_NONE;
}

static void ecpuinsExecIns()
{
	t_nubit8 opcode;
	do {
		opcode = vramByte(evIP);
		evIP++;
		ExecFun(ecpuins.table[opcode]);
		ecpu.ip = (evIP - (ecpu.cs << 4)) % 0x10000;
	} while (IsPrefix(opcode));
	ClrPrefix();
}
static void ecpuinsExecInt()
{	
	/* hardware interrupt handeler */
	t_nubit8 intr;
	if(ecpu.flagnmi) {
		INT(0x02);
	}
	ecpu.flagnmi = 0x00;

	if(GetBit(ecpu.flags, VCPU_EFLAGS_IF) && ecpuapiScanINTR()) {
		intr = ecpuapiGetINTR();
		INT(intr);
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
	ecpuins.table[0x60]=(t_faddrcc)OpcError;
	ecpuins.table[0x61]=(t_faddrcc)OpcError;
	ecpuins.table[0x62]=(t_faddrcc)OpcError;
	ecpuins.table[0x63]=(t_faddrcc)OpcError;
	ecpuins.table[0x64]=(t_faddrcc)OpcError;
	ecpuins.table[0x65]=(t_faddrcc)OpcError;
	ecpuins.table[0x66]=(t_faddrcc)OpdSize;
	ecpuins.table[0x67]=(t_faddrcc)AddrSize;
	ecpuins.table[0x68]=(t_faddrcc)PUSH_I16;
	ecpuins.table[0x69]=(t_faddrcc)OpcError;
	ecpuins.table[0x6A]=(t_faddrcc)OpcError;
	ecpuins.table[0x6B]=(t_faddrcc)OpcError;
	ecpuins.table[0x6C]=(t_faddrcc)OpcError;
	ecpuins.table[0x6D]=(t_faddrcc)OpcError;
	ecpuins.table[0x6E]=(t_faddrcc)OpcError;
	ecpuins.table[0x6F]=(t_faddrcc)OpcError;
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
	ecpuins.table[0x82]=(t_faddrcc)INS_82;
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
	ecpuins.table[0xA6]=(t_faddrcc)_CMPSB;
	ecpuins.table[0xA7]=(t_faddrcc)_CMPSW;
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
	ecpuins.table[0xC8]=(t_faddrcc)OpcError;
	ecpuins.table[0xC9]=(t_faddrcc)OpcError;
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
	ecpuins.table[0xD6]=(t_faddrcc)OpcError;
	ecpuins.table[0xD7]=(t_faddrcc)XLAT;
	ecpuins.table[0xD8]=(t_faddrcc)OpcError;
	ecpuins.table[0xD9]=(t_faddrcc)INS_D9;
	ecpuins.table[0xDA]=(t_faddrcc)OpcError;
	ecpuins.table[0xDB]=(t_faddrcc)INS_DB;
	ecpuins.table[0xDC]=(t_faddrcc)OpcError;
	ecpuins.table[0xDD]=(t_faddrcc)OpcError;
	ecpuins.table[0xDE]=(t_faddrcc)OpcError;
	ecpuins.table[0xDF]=(t_faddrcc)OpcError;
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
	Ins0FTable[0x00]=(t_faddrcc)OpcError;
	Ins0FTable[0x01]=(t_faddrcc)INS_0F01;
	Ins0FTable[0x02]=(t_faddrcc)OpcError;
	Ins0FTable[0x03]=(t_faddrcc)OpcError;
	Ins0FTable[0x04]=(t_faddrcc)OpcError;
	Ins0FTable[0x05]=(t_faddrcc)OpcError;
	Ins0FTable[0x06]=(t_faddrcc)OpcError;
	Ins0FTable[0x07]=(t_faddrcc)OpcError;
	Ins0FTable[0x08]=(t_faddrcc)OpcError;
	Ins0FTable[0x09]=(t_faddrcc)OpcError;
	Ins0FTable[0x0A]=(t_faddrcc)OpcError;
	Ins0FTable[0x0B]=(t_faddrcc)OpcError;
	Ins0FTable[0x0C]=(t_faddrcc)OpcError;
	Ins0FTable[0x0D]=(t_faddrcc)OpcError;
	Ins0FTable[0x0E]=(t_faddrcc)OpcError;
	Ins0FTable[0x0F]=(t_faddrcc)OpcError;
	Ins0FTable[0x10]=(t_faddrcc)OpcError;
	Ins0FTable[0x11]=(t_faddrcc)OpcError;
	Ins0FTable[0x12]=(t_faddrcc)OpcError;
	Ins0FTable[0x13]=(t_faddrcc)OpcError;
	Ins0FTable[0x14]=(t_faddrcc)OpcError;
	Ins0FTable[0x15]=(t_faddrcc)OpcError;
	Ins0FTable[0x16]=(t_faddrcc)OpcError;
	Ins0FTable[0x17]=(t_faddrcc)OpcError;
	Ins0FTable[0x18]=(t_faddrcc)OpcError;
	Ins0FTable[0x19]=(t_faddrcc)OpcError;
	Ins0FTable[0x1A]=(t_faddrcc)OpcError;
	Ins0FTable[0x1B]=(t_faddrcc)OpcError;
	Ins0FTable[0x1C]=(t_faddrcc)OpcError;
	Ins0FTable[0x1D]=(t_faddrcc)OpcError;
	Ins0FTable[0x1E]=(t_faddrcc)OpcError;
	Ins0FTable[0x1F]=(t_faddrcc)OpcError;
	Ins0FTable[0x20]=(t_faddrcc)OpcError;
	Ins0FTable[0x21]=(t_faddrcc)OpcError;
	Ins0FTable[0x22]=(t_faddrcc)OpcError;
	Ins0FTable[0x23]=(t_faddrcc)OpcError;
	Ins0FTable[0x24]=(t_faddrcc)OpcError;
	Ins0FTable[0x25]=(t_faddrcc)OpcError;
	Ins0FTable[0x26]=(t_faddrcc)OpcError;
	Ins0FTable[0x27]=(t_faddrcc)OpcError;
	Ins0FTable[0x28]=(t_faddrcc)OpcError;
	Ins0FTable[0x29]=(t_faddrcc)OpcError;
	Ins0FTable[0x2A]=(t_faddrcc)OpcError;
	Ins0FTable[0x2B]=(t_faddrcc)OpcError;
	Ins0FTable[0x2C]=(t_faddrcc)OpcError;
	Ins0FTable[0x2D]=(t_faddrcc)OpcError;
	Ins0FTable[0x2E]=(t_faddrcc)OpcError;
	Ins0FTable[0x2F]=(t_faddrcc)OpcError;
	Ins0FTable[0x30]=(t_faddrcc)OpcError;
	Ins0FTable[0x31]=(t_faddrcc)OpcError;
	Ins0FTable[0x32]=(t_faddrcc)OpcError;
	Ins0FTable[0x33]=(t_faddrcc)OpcError;
	Ins0FTable[0x34]=(t_faddrcc)OpcError;
	Ins0FTable[0x35]=(t_faddrcc)OpcError;
	Ins0FTable[0x36]=(t_faddrcc)OpcError;
	Ins0FTable[0x37]=(t_faddrcc)OpcError;
	Ins0FTable[0x38]=(t_faddrcc)OpcError;
	Ins0FTable[0x39]=(t_faddrcc)OpcError;
	Ins0FTable[0x3A]=(t_faddrcc)OpcError;
	Ins0FTable[0x3B]=(t_faddrcc)OpcError;
	Ins0FTable[0x3C]=(t_faddrcc)OpcError;
	Ins0FTable[0x3D]=(t_faddrcc)OpcError;
	Ins0FTable[0x3E]=(t_faddrcc)OpcError;
	Ins0FTable[0x3F]=(t_faddrcc)OpcError;
	Ins0FTable[0x40]=(t_faddrcc)OpcError;
	Ins0FTable[0x41]=(t_faddrcc)OpcError;
	Ins0FTable[0x42]=(t_faddrcc)OpcError;
	Ins0FTable[0x43]=(t_faddrcc)OpcError;
	Ins0FTable[0x44]=(t_faddrcc)OpcError;
	Ins0FTable[0x45]=(t_faddrcc)OpcError;
	Ins0FTable[0x46]=(t_faddrcc)OpcError;
	Ins0FTable[0x47]=(t_faddrcc)OpcError;
	Ins0FTable[0x48]=(t_faddrcc)OpcError;
	Ins0FTable[0x49]=(t_faddrcc)OpcError;
	Ins0FTable[0x4A]=(t_faddrcc)OpcError;
	Ins0FTable[0x4B]=(t_faddrcc)OpcError;
	Ins0FTable[0x4C]=(t_faddrcc)OpcError;
	Ins0FTable[0x4D]=(t_faddrcc)OpcError;
	Ins0FTable[0x4E]=(t_faddrcc)OpcError;
	Ins0FTable[0x4F]=(t_faddrcc)OpcError;
	Ins0FTable[0x50]=(t_faddrcc)OpcError;
	Ins0FTable[0x51]=(t_faddrcc)OpcError;
	Ins0FTable[0x52]=(t_faddrcc)OpcError;
	Ins0FTable[0x53]=(t_faddrcc)OpcError;
	Ins0FTable[0x54]=(t_faddrcc)OpcError;
	Ins0FTable[0x55]=(t_faddrcc)OpcError;
	Ins0FTable[0x56]=(t_faddrcc)OpcError;
	Ins0FTable[0x57]=(t_faddrcc)OpcError;
	Ins0FTable[0x58]=(t_faddrcc)OpcError;
	Ins0FTable[0x59]=(t_faddrcc)OpcError;
	Ins0FTable[0x5A]=(t_faddrcc)OpcError;
	Ins0FTable[0x5B]=(t_faddrcc)OpcError;
	Ins0FTable[0x5C]=(t_faddrcc)OpcError;
	Ins0FTable[0x5D]=(t_faddrcc)OpcError;
	Ins0FTable[0x5E]=(t_faddrcc)OpcError;
	Ins0FTable[0x5F]=(t_faddrcc)OpcError;
	Ins0FTable[0x60]=(t_faddrcc)OpcError;
	Ins0FTable[0x61]=(t_faddrcc)OpcError;
	Ins0FTable[0x62]=(t_faddrcc)OpcError;
	Ins0FTable[0x63]=(t_faddrcc)OpcError;
	Ins0FTable[0x64]=(t_faddrcc)OpcError;
	Ins0FTable[0x65]=(t_faddrcc)OpcError;
	Ins0FTable[0x66]=(t_faddrcc)OpcError;
	Ins0FTable[0x67]=(t_faddrcc)OpcError;
	Ins0FTable[0x68]=(t_faddrcc)OpcError;
	Ins0FTable[0x69]=(t_faddrcc)OpcError;
	Ins0FTable[0x6A]=(t_faddrcc)OpcError;
	Ins0FTable[0x6B]=(t_faddrcc)OpcError;
	Ins0FTable[0x6C]=(t_faddrcc)OpcError;
	Ins0FTable[0x6D]=(t_faddrcc)OpcError;
	Ins0FTable[0x6E]=(t_faddrcc)OpcError;
	Ins0FTable[0x6F]=(t_faddrcc)OpcError;
	Ins0FTable[0x70]=(t_faddrcc)OpcError;
	Ins0FTable[0x71]=(t_faddrcc)OpcError;
	Ins0FTable[0x72]=(t_faddrcc)OpcError;
	Ins0FTable[0x73]=(t_faddrcc)OpcError;
	Ins0FTable[0x74]=(t_faddrcc)OpcError;
	Ins0FTable[0x75]=(t_faddrcc)OpcError;
	Ins0FTable[0x76]=(t_faddrcc)OpcError;
	Ins0FTable[0x77]=(t_faddrcc)OpcError;
	Ins0FTable[0x78]=(t_faddrcc)OpcError;
	Ins0FTable[0x79]=(t_faddrcc)OpcError;
	Ins0FTable[0x7A]=(t_faddrcc)OpcError;
	Ins0FTable[0x7B]=(t_faddrcc)OpcError;
	Ins0FTable[0x7C]=(t_faddrcc)OpcError;
	Ins0FTable[0x7D]=(t_faddrcc)OpcError;
	Ins0FTable[0x7E]=(t_faddrcc)OpcError;
	Ins0FTable[0x7F]=(t_faddrcc)OpcError;
	Ins0FTable[0x80]=(t_faddrcc)OpcError;
	Ins0FTable[0x81]=(t_faddrcc)OpcError;
	Ins0FTable[0x82]=(t_faddrcc)JC;
	Ins0FTable[0x83]=(t_faddrcc)OpcError;
	Ins0FTable[0x84]=(t_faddrcc)JZ;
	Ins0FTable[0x85]=(t_faddrcc)JNZ;
	Ins0FTable[0x86]=(t_faddrcc)OpcError;
	Ins0FTable[0x87]=(t_faddrcc)JA;
	Ins0FTable[0x88]=(t_faddrcc)OpcError;
	Ins0FTable[0x89]=(t_faddrcc)OpcError;
	Ins0FTable[0x8A]=(t_faddrcc)OpcError;
	Ins0FTable[0x8B]=(t_faddrcc)OpcError;
	Ins0FTable[0x8C]=(t_faddrcc)OpcError;
	Ins0FTable[0x8D]=(t_faddrcc)OpcError;
	Ins0FTable[0x8E]=(t_faddrcc)OpcError;
	Ins0FTable[0x8F]=(t_faddrcc)OpcError;
	Ins0FTable[0x90]=(t_faddrcc)OpcError;
	Ins0FTable[0x91]=(t_faddrcc)OpcError;
	Ins0FTable[0x92]=(t_faddrcc)OpcError;
	Ins0FTable[0x93]=(t_faddrcc)OpcError;
	Ins0FTable[0x94]=(t_faddrcc)OpcError;
	Ins0FTable[0x95]=(t_faddrcc)OpcError;
	Ins0FTable[0x96]=(t_faddrcc)OpcError;
	Ins0FTable[0x97]=(t_faddrcc)OpcError;
	Ins0FTable[0x98]=(t_faddrcc)OpcError;
	Ins0FTable[0x99]=(t_faddrcc)OpcError;
	Ins0FTable[0x9A]=(t_faddrcc)OpcError;
	Ins0FTable[0x9B]=(t_faddrcc)OpcError;
	Ins0FTable[0x9C]=(t_faddrcc)OpcError;
	Ins0FTable[0x9D]=(t_faddrcc)OpcError;
	Ins0FTable[0x9E]=(t_faddrcc)OpcError;
	Ins0FTable[0x9F]=(t_faddrcc)OpcError;
	Ins0FTable[0xA0]=(t_faddrcc)OpcError;
	Ins0FTable[0xA1]=(t_faddrcc)POP_FS;
	Ins0FTable[0xA2]=(t_faddrcc)CPUID;
	Ins0FTable[0xA3]=(t_faddrcc)OpcError;
	Ins0FTable[0xA4]=(t_faddrcc)OpcError;
	Ins0FTable[0xA5]=(t_faddrcc)OpcError;
	Ins0FTable[0xA6]=(t_faddrcc)OpcError;
	Ins0FTable[0xA7]=(t_faddrcc)OpcError;
	Ins0FTable[0xA8]=(t_faddrcc)OpcError;
	Ins0FTable[0xA9]=(t_faddrcc)OpcError;
	Ins0FTable[0xAA]=(t_faddrcc)OpcError;
	Ins0FTable[0xAB]=(t_faddrcc)OpcError;
	Ins0FTable[0xAC]=(t_faddrcc)OpcError;
	Ins0FTable[0xAD]=(t_faddrcc)OpcError;
	Ins0FTable[0xAE]=(t_faddrcc)OpcError;
	Ins0FTable[0xAF]=(t_faddrcc)OpcError;
	Ins0FTable[0xB0]=(t_faddrcc)OpcError;
	Ins0FTable[0xB1]=(t_faddrcc)OpcError;
	Ins0FTable[0xB2]=(t_faddrcc)OpcError;
	Ins0FTable[0xB3]=(t_faddrcc)OpcError;
	Ins0FTable[0xB4]=(t_faddrcc)OpcError;
	Ins0FTable[0xB5]=(t_faddrcc)OpcError;
	Ins0FTable[0xB6]=(t_faddrcc)MOVZX_RM8;
	Ins0FTable[0xB7]=(t_faddrcc)MOVZX_RM16;
	Ins0FTable[0xB8]=(t_faddrcc)OpcError;
	Ins0FTable[0xB9]=(t_faddrcc)OpcError;
	Ins0FTable[0xBA]=(t_faddrcc)OpcError;
	Ins0FTable[0xBB]=(t_faddrcc)OpcError;
	Ins0FTable[0xBC]=(t_faddrcc)OpcError;
	Ins0FTable[0xBD]=(t_faddrcc)OpcError;
	Ins0FTable[0xBE]=(t_faddrcc)OpcError;
	Ins0FTable[0xBF]=(t_faddrcc)OpcError;
	Ins0FTable[0xC0]=(t_faddrcc)OpcError;
	Ins0FTable[0xC1]=(t_faddrcc)OpcError;
	Ins0FTable[0xC2]=(t_faddrcc)OpcError;
	Ins0FTable[0xC3]=(t_faddrcc)OpcError;
	Ins0FTable[0xC4]=(t_faddrcc)OpcError;
	Ins0FTable[0xC5]=(t_faddrcc)OpcError;
	Ins0FTable[0xC6]=(t_faddrcc)OpcError;
	Ins0FTable[0xC7]=(t_faddrcc)OpcError;
	Ins0FTable[0xC8]=(t_faddrcc)OpcError;
	Ins0FTable[0xC9]=(t_faddrcc)OpcError;
	Ins0FTable[0xCA]=(t_faddrcc)OpcError;
	Ins0FTable[0xCB]=(t_faddrcc)OpcError;
	Ins0FTable[0xCC]=(t_faddrcc)OpcError;
	Ins0FTable[0xCD]=(t_faddrcc)OpcError;
	Ins0FTable[0xCE]=(t_faddrcc)OpcError;
	Ins0FTable[0xCF]=(t_faddrcc)OpcError;
	Ins0FTable[0xD0]=(t_faddrcc)OpcError;
	Ins0FTable[0xD1]=(t_faddrcc)OpcError;
	Ins0FTable[0xD2]=(t_faddrcc)OpcError;
	Ins0FTable[0xD3]=(t_faddrcc)OpcError;
	Ins0FTable[0xD4]=(t_faddrcc)OpcError;
	Ins0FTable[0xD5]=(t_faddrcc)OpcError;
	Ins0FTable[0xD6]=(t_faddrcc)OpcError;
	Ins0FTable[0xD7]=(t_faddrcc)OpcError;
	Ins0FTable[0xD8]=(t_faddrcc)OpcError;
	Ins0FTable[0xD9]=(t_faddrcc)OpcError;
	Ins0FTable[0xDA]=(t_faddrcc)OpcError;
	Ins0FTable[0xDB]=(t_faddrcc)OpcError;
	Ins0FTable[0xDC]=(t_faddrcc)OpcError;
	Ins0FTable[0xDD]=(t_faddrcc)OpcError;
	Ins0FTable[0xDE]=(t_faddrcc)OpcError;
	Ins0FTable[0xDF]=(t_faddrcc)OpcError;
	Ins0FTable[0xE0]=(t_faddrcc)OpcError;
	Ins0FTable[0xE1]=(t_faddrcc)OpcError;
	Ins0FTable[0xE2]=(t_faddrcc)OpcError;
	Ins0FTable[0xE3]=(t_faddrcc)OpcError;
	Ins0FTable[0xE4]=(t_faddrcc)OpcError;
	Ins0FTable[0xE5]=(t_faddrcc)OpcError;
	Ins0FTable[0xE6]=(t_faddrcc)OpcError;
	Ins0FTable[0xE7]=(t_faddrcc)OpcError;
	Ins0FTable[0xE8]=(t_faddrcc)OpcError;
	Ins0FTable[0xE9]=(t_faddrcc)OpcError;
	Ins0FTable[0xEA]=(t_faddrcc)OpcError;
	Ins0FTable[0xEB]=(t_faddrcc)OpcError;
	Ins0FTable[0xEC]=(t_faddrcc)OpcError;
	Ins0FTable[0xED]=(t_faddrcc)OpcError;
	Ins0FTable[0xEE]=(t_faddrcc)OpcError;
	Ins0FTable[0xEF]=(t_faddrcc)OpcError;
	Ins0FTable[0xF0]=(t_faddrcc)OpcError;
	Ins0FTable[0xF1]=(t_faddrcc)OpcError;
	Ins0FTable[0xF2]=(t_faddrcc)OpcError;
	Ins0FTable[0xF3]=(t_faddrcc)OpcError;
	Ins0FTable[0xF4]=(t_faddrcc)OpcError;
	Ins0FTable[0xF5]=(t_faddrcc)OpcError;
	Ins0FTable[0xF6]=(t_faddrcc)OpcError;
	Ins0FTable[0xF7]=(t_faddrcc)OpcError;
	Ins0FTable[0xF8]=(t_faddrcc)OpcError;
	Ins0FTable[0xF9]=(t_faddrcc)OpcError;
	Ins0FTable[0xFA]=(t_faddrcc)OpcError;
	Ins0FTable[0xFB]=(t_faddrcc)OpcError;
	Ins0FTable[0xFC]=(t_faddrcc)OpcError;
	Ins0FTable[0xFD]=(t_faddrcc)OpcError;
	Ins0FTable[0xFE]=(t_faddrcc)OpcError;
	Ins0FTable[0xFF]=(t_faddrcc)OpcError;
}
void ecpuinsRefresh()
{
	ecpuinsExecIns();
	ecpuinsExecInt();
}
void ecpuinsFinal() {}
