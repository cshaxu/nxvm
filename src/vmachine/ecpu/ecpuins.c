//////////////////////////////////////////////////////////////////////////
// 名称：Instruction.cpp
// 功能：模拟8086指令集
// 日期：2008年4月20日
// 作者：梁一信
//////////////////////////////////////////////////////////////////////////

#include "../vglobal.h"
#include "../vpic.h"
#include "../vapi.h"
#include "../qdbios.h"
#include "ecpuins.h"
#include "ecpu.h"

t_faddrcc InsTable[0x100];
t_vaddrcc Ins0FTable[0x100];

#define eIMS (evIP+MemoryStart)
#define EvSP (t=ecpu.ss,(t<<4)+ecpu.sp)
#define evSI (t=tmpDs,(t<<4)+ecpu.si)
#define evDI (t=ecpu.es,(t<<4)+ecpu.di)
#define toe8 (TranslateOpcExt(0,(char **)&r8,(char **)&rm8))
#define toe16 (TranslateOpcExt(1,(char **)&r16,(char **)&rm16))
#define toe32 (TranslateOpcExt(1,(char **)&r32,(char **)&rm32))

const t_nubit16 Glbffff=0xffff;		//当寻址超过0xfffff的时候，返回的是一个可以令程序Reset的地址
t_nubit16 GlbZero=0x0;			//有些寻址用到两个偏移寄存器；有些寻址只用到一个偏移寄存器，另外一个指向这里。

t_vaddrcc evIP;			//evIP读指令时的指针
t_nubitcc GlobINT;		//所有非INT指令产生的中断


t_nubit16 *rm16,*r16;			//解释寻址字节的时候用
t_nubit32 *rm32,*r32;
t_nubit8 *rm8,*r8;				//
t_nubit16 t161,t162;			//可以随便使用的
t_nubit32 t321,t322;				//
t_nubit8 t81,t82;				//

t_nubit32 t,t2,t3;				//

t_nubit16 tmpDs;				//这是存储器寻址时的段寄存器，绝大多数情况下等于DS，遇到CS、ES、SS指令时在一条指令的时间下作出修改
t_nubit16 tmpSs;				//

t_nubit16 tmpOpdSize=2;			//Operand Size，由描述符里的D位和OpdSize前缀共同决定，初始值为2
t_nubit16 tmpAddrSize=2;			//Address Size，由描述符里的D位和AddrSize前缀共同决定，初始值为2

#define VOID static void

//在执行每一条指令之前，evIP要等于cs:ip。
//执行完一条指令之后，再按evIP修改cs:ip。
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
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(t_nubit8 *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xff;
}
t_nubit8 GetM8_32(t_nubit32 off)
{
	return *(t_nubit8 *)(off+(t=tmpDs,t<<4)+MemoryStart);
}
t_nubit16 GetM16_16(t_nubit16 off)
{
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(t_nubit16 *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xffff;
}
t_nubit16 GetM16_32(t_nubit32 off)
{	
	return *(t_nubit16 *)(off+(t=tmpDs,t<<4)+MemoryStart);
}
t_nubit32 GetM32_16(t_nubit16 off)
{
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(t_nubit32 *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xffffffff;
}
t_nubit32 GetM32_32(t_nubit32 off)
{
	return *(t_nubit32 *)(off+(t=tmpDs,t<<4)+MemoryStart);
}
VOID SetM8(t_nubit16 off, t_nubit8 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit8 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;

}
VOID SetM16(t_nubit16 off, t_nubit16 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit16 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
}
VOID SetM32(t_nubit16 off, t_nubit32 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit32 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
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
	tds=tmpDs;
	tds<<=4;
	tes=ecpu.es;
	tes<<=4;
	tss=tmpSs;
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
	t_nubit32 tds=tmpDs;
	t_nubit8 mod, reg, rem;
	tds<<=4;
	mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	reg=*(char*)(evIP+MemoryStart) & 0x38;
	reg>>=3;
	rem=*(char*)(evIP+MemoryStart) & 0x07;

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
			*rm+=(*off1+*off2+*(char *)(evIP+MemoryStart))-(*off1+*off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+*(char *)(evIP+MemoryStart))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;		
		if (tmpAddrSize==2)
			*rm+=(t_nubit16)(*off1+*off2+*(t_nubit16 *)(evIP+MemoryStart))-(*off1+*off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
		else
			*rm+=(t_nubit16)(*(t_nubit32 *)off1+*(t_nubit32 *)off2+*(t_nubit16 *)(evIP+MemoryStart))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
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
	tds=tmpDs;
	tds<<=4;

	mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	reg=*(char*)(evIP+MemoryStart) & 0x38;
	reg>>=3;
	rem=*(char*)(evIP+MemoryStart) & 0x07;

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
			*rm+=(*off1+*off2+*(char *)(evIP+MemoryStart))-(*off1+*off2);		//对偏移寄存器溢出的处理
		else
			*rm+=(*(t_nubit32 *)off1+*(t_nubit32 *)off2+*(char *)(evIP+MemoryStart))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);		//对偏移寄存器溢出的处理
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;	
		if (tmpAddrSize==2)
			*rm+=(t_nubit16)(*off1+*off2+*(t_nubit16 *)(evIP+MemoryStart))-(*off1+*off2);	
		else
			*rm+=(t_nubit16)(*(t_nubit32 *)off1+*(t_nubit32 *)off2+*(t_nubit16 *)(evIP+MemoryStart))-(*(t_nubit32 *)off1+*(t_nubit32 *)off2);	
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
	t_nubit8 *pc=(t_nubit8 *)MemoryStart+evIP-1;
	vapiPrint("An unkown instruction [ %2x %2x %2x %2x %2x %2x ] was read at [ %4xh:%4xh ], easyVM only support 8086 instruction set in this version. easyVM will be terminated.",*(pc),*(pc+1),*(pc+2),*(pc+3),*(pc+4),*(pc+5),ecpu.cs,ecpu.ip);
	vapiCallBackMachineStop();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 高度抽象部分

VOID ADD(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)+=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)+=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)+=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags			
		break;
	}
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID PUSH(void*Src,int Len)
{
	switch(Len)
	{
	case 2:
		ecpu.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=*(t_nubit16 *)Src;
		break;
	case 4:
		ecpu.sp-=4;
		*(t_nubit32*)( EvSP +MemoryStart)=*(t_nubit32 *)Src;
		break;
	}
}
VOID POP(void*Des, int Len)
{
	switch(Len)
	{
	case 2:
		d_nubit16(Des)=*(t_nubit16*)(EvSP+MemoryStart);
		ecpu.sp+=2;
		break;
	case 4:
		d_nubit32(Des)=*(t_nubit32*)(EvSP+MemoryStart);
		ecpu.sp+=4;
		break;
	}
}
VOID OR(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID ADC(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID SBB(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID AND(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID SUB(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID XOR(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID CMP(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	t_nubit8 opr11,opr12;
	t_nubit16 opr21,opr22;
	t_nubit32 opr41,opr42;
	switch(Len)
	{
	case 1:
		toe8;
		opr11 = d_nubit8(*Des);
		opr12 = d_nubit8(*Src);
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		__asm mov al, opr11
		__asm cmp al, opr12
//		d_nubit8(*Des)-=d_nubit8(*Src);
		__asm pushfd
		__asm pop ecpu.eflags
//		d_nubit8(*Des)+=d_nubit8(*Src);
		break;
	case 2:
		toe16;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)-=d_nubit16(*Src);
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit16(*Des)+=d_nubit16(*Src);
		break;
	case 4:
		toe32;
		__asm push ecpu.eflags				//加操作只修改某些位，如果直接就pop ecpu.flags会把整个ecpu.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)-=d_nubit32(*Src);
		__asm pushfd
		__asm pop ecpu.eflags
		d_nubit32(*Des)+=d_nubit32(*Src);
		break;
	}
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID INC(void*Des, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID DEC(void*Des, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID TEST(void**Des, void**Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID XCHG(void*Des, void*Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		t81=d_nubit8(Des);
		d_nubit8(Des)=d_nubit8(Src);
		d_nubit8(Src)=t81;
		break;
	case 2:
		t161=d_nubit16(Des);
		d_nubit16(Des)=*(t_nubit16 *)Src;
		*(t_nubit16 *)Src=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		d_nubit32(Des)=*(t_nubit32 *)Src;
		*(t_nubit32 *)Src=t321;
		break;
	}
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID MOV(void*Des, void*Src, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		d_nubit8(Des)=d_nubit8(Src);
		break;
	case 2:
		d_nubit16(Des)=*(t_nubit16 *)Src;
		break;
	case 4:
		d_nubit32(Des)=*(t_nubit32 *)Src;
		break;
	}
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
}
VOID SHL(void*Des, t_nubit8 mb, int Len)
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	MakeBit(ecpu.flags, VCPU_FLAG_IF, intf);
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
	if (*(char *)(eIMS-1)==code)
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 0x00
VOID ADD_RM8_R8()
{
	ADD((void**)&rm8,(void**)&r8,1);
}
VOID ADD_RM16_R16()
{
	ADD((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID ADD_R8_RM8()
{
	ADD((void**)&r8,(void**)&rm8,1);
}
VOID ADD_R16_RM16()
{
	ADD((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID ADD_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	ADD((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID ADD_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	ADD((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID PUSH_ES()
{
	PUSH(&ecpu.es,tmpOpdSize);
}
VOID POP_ES()
{
	POP(&ecpu.es,tmpOpdSize);
}
VOID OR_RM8_R8()
{
	OR((void**)&rm8,(void**)&r8,1);
}
VOID OR_RM16_R16()
{
	OR((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID OR_R8_RM8()
{
	OR((void**)&r8,(void**)&rm8,1);
}
VOID OR_R16_RM16()
{
	OR((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID OR_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	OR((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID OR_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	OR((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID PUSH_CS()
{
	PUSH(&ecpu.cs,tmpOpdSize);
}
// 0x10
VOID ADC_RM8_R8()
{
	ADC((void**)&rm8,(void**)&r8,1);
}
VOID ADC_RM16_R16()
{
	ADC((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID ADC_R8_RM8()
{
	ADC((void**)&r8,(void**)&rm8,1);
}
VOID ADC_R16_RM16()
{
	ADC((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID ADC_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	ADC((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID ADC_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	ADC((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID PUSH_SS()
{
	PUSH(&ecpu.ss,tmpOpdSize);
}
VOID POP_SS()
{
	POP(&ecpu.ss,tmpOpdSize);
}
VOID SBB_RM8_R8()
{
	SBB((void**)&rm8,(void**)&r8,1);
}
VOID SBB_RM16_R16()
{
	SBB((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID SBB_R8_RM8()
{
	SBB((void**)&r8,(void**)&rm8,1);
}
VOID SBB_R16_RM16()
{
	SBB((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID SBB_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	SBB((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID SBB_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	SBB((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID PUSH_DS()
{
	PUSH(&ecpu.ds,tmpOpdSize);
}
VOID POP_DS()
{
	POP(&ecpu.ds,tmpOpdSize);
}
// 0x20
VOID AND_RM8_R8()
{
	AND((void**)&rm8,(void**)&r8,1);
}
VOID AND_RM16_R16()
{
	AND((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID AND_R8_RM8()
{
	AND((void**)&r8,(void**)&rm8,1);
}
VOID AND_R16_RM16()
{
	AND((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID AND_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	AND((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID AND_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	AND((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID ES()
{
// 	t_nubit16 tds=ecpu.ds;
// 	t_nubit16 tes=ecpu.es;
// 	ecpu.ds=ecpu.es;
// 
	tmpDs=ecpu.es;
	tmpSs=ecpu.es;
	ecpuinsExecIns();
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
// 	if (ecpu.ds==tes)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		ecpu.ds=tds;
}
VOID DAA()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,ecpu.al
		push ecpu.flags
		popf
		DAA
		pushf
		pop ecpu.flags
		mov ecpu.al,al
	}
}
VOID SUB_RM8_R8()
{
	SUB((void**)&rm8,(void**)&r8,1);
}
VOID SUB_RM16_R16()
{
	SUB((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID SUB_R8_RM8()
{
	SUB((void**)&r8,(void**)&rm8,1);
}
VOID SUB_R16_RM16()
{
	SUB((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID SUB_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	SUB((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID SUB_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	SUB((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID CS()
{
// 	t_nubit16 tds=ecpu.ds;
// 	t_nubit16 tcs=ecpu.cs;
// 	ecpu.ds=ecpu.cs;
	tmpDs=ecpu.cs;
	tmpSs=ecpu.cs;
	ecpuinsExecIns();
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
// 	if (ecpu.ds==tcs)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		ecpu.ds=tds;
}
VOID DAS()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,ecpu.al
		push ecpu.flags
		popf
		DAS
		pushf
		pop ecpu.flags
		mov ecpu.al,al
	}
}
// 0x30
VOID XOR_RM8_R8()
{
	XOR((void**)&rm8,(void**)&r8,1);
}
VOID XOR_RM16_R16()
{
	XOR((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID XOR_R8_RM8()
{
	XOR((void**)&r8,(void**)&rm8,1);
}
VOID XOR_R16_RM16()
{
	XOR((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID XOR_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	XOR((void**)&pa,(void**)&pb,1);	
	evIP=tevIP+1;
}
VOID XOR_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	XOR((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID SS()
{
// 	t_nubit16 tds=ecpu.ds;
// 	t_nubit16 tss=ecpu.ss;
// 	ecpu.ds=ecpu.ss;
	tmpDs=ecpu.ss;
	tmpSs=ecpu.ss;
	ecpuinsExecIns();
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
// 	if (ecpu.ds==tss)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		ecpu.ds=tds;
}
VOID AAA()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,ecpu.al
		push ecpu.flags
		popf
		AAA
		pushf
		pop ecpu.flags
		mov ecpu.al,al
	}
}
VOID CMP_RM8_R8()
{
	CMP((void**)&rm8,(void**)&r8,1);
}
VOID CMP_RM16_R16()
{
	CMP((void**)&rm16,(void**)&r16,tmpOpdSize);
}
VOID CMP_R8_RM8()
{
	CMP((void**)&r8,(void**)&rm8,1);
}
VOID CMP_R16_RM16()
{
	CMP((void**)&r16,(void**)&rm16,tmpOpdSize);
}
VOID CMP_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.al,*pb=(void*)(evIP+MemoryStart);
	CMP((void**)&pa,(void**)&pb,1);
	evIP=tevIP+1;
}
VOID CMP_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void*pa=&ecpu.ax,*pb=(void*)(evIP+MemoryStart);
	CMP((void**)&pa,(void**)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
VOID DS()
{
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ds;
	ecpuinsExecIns();
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
}
VOID AAS()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
	PUSH(&ecpu.ax,tmpOpdSize);
}
VOID PUSH_CX()
{
	PUSH(&ecpu.cx,tmpOpdSize);
}
VOID PUSH_DX()
{
	PUSH(&ecpu.dx,tmpOpdSize);
}
VOID PUSH_BX()
{
	PUSH(&ecpu.bx,tmpOpdSize);
}
VOID PUSH_SP()
{
	t_nubit16 tsp=ecpu.sp;
	PUSH(&tsp,tmpOpdSize);			//不能PUSH修改之后的值
}
VOID PUSH_BP()
{
	PUSH(&ecpu.bp,tmpOpdSize);
}
VOID PUSH_SI()
{
	PUSH(&ecpu.si,tmpOpdSize);
}
VOID PUSH_DI()
{
	PUSH(&ecpu.di,tmpOpdSize);
}
VOID POP_AX()
{
	POP(&ecpu.ax,tmpOpdSize);
}
VOID POP_CX()
{
	POP(&ecpu.cx,tmpOpdSize);
}
VOID POP_DX()
{
	POP(&ecpu.dx,tmpOpdSize);
}
VOID POP_BX()
{
	POP(&ecpu.bx,tmpOpdSize);
}
VOID POP_SP()
{
	POP(&ecpu.sp,tmpOpdSize);
	ecpu.sp-=tmpOpdSize;				//POP()里是先赋值后加的，所以这里要减回去
}
VOID POP_BP()
{
	POP(&ecpu.bp,tmpOpdSize);
}
VOID POP_SI()
{
	POP(&ecpu.si,tmpOpdSize);
}
VOID POP_DI()
{
	POP(&ecpu.di,tmpOpdSize);
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
	PUSH((void*)eIMS,tmpOpdSize);
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
	char oce=d_nsbit8(evIP+MemoryStart) & 0x38;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
		ADD((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 1:
		OR((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 2:
		ADC((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 3:		
		SBB((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 4:		
		AND((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 5:		
		SUB((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 6:		
		XOR((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	case 7:		
		CMP((void**)&rm16,(void**)&teIMS,tmpOpdSize);
		break;
	}
	evIP+=tmpOpdSize;
}
VOID INS_82()	//这里是以82开头的指令的集。
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
		ADD((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 1:
		OR((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 2:
		ADC((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 3:		
		SBB((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 4:		
		AND((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 5:		
		SUB((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 6:		
		XOR((void**)&rm16,(void**)&ptfg,tmpOpdSize);
		break;
	case 7:		
		CMP((void**)&rm16,(void**)&ptfg,tmpOpdSize);
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
	mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	rem=*(char*)(evIP+MemoryStart) & 0x07;
	toe16;
	switch(rem)
	{
	case 0:
	case 1:
	case 4:
	case 5:
	case 7:
		*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=tmpDs,t<<4));
		break;
	case 2:
	case 3:	
		*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=tmpSs,t<<4));
		break;
	case 6:
		if (mod==0)		
			*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=tmpDs,t<<4));
		else
			*r16=(t_nubit16)((t_nubit32)rm16-MemoryStart-(t=tmpSs,t<<4));
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
 	POP((void*)rm16,tmpOpdSize);
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
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.cs;
	ecpu.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
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
	PUSH(&ecpu.flags,tmpOpdSize);
}
VOID POPF()
{
	POP(&ecpu.flags,tmpOpdSize);
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
VOID MOVSB()
{	
	t_nubit8 tgm;
	if (tmpAddrSize==2)	
		tgm=GetM8_16(ecpu.si);			//因为t是全局变量！在GetM8里已经改变了t的值！！！
	else
		tgm=GetM8_32(ecpu.esi);
	(t=ecpu.es,t<<=4);
	*((t_nubit8 *)(MemoryStart+ecpu.di+t))=tgm;
//	if (ecpu.di+t<0xc0000 && ecpu.di+t>=0xa0000)
//		WriteVideoRam(ecpu.di+t-0xa0000);
	if (ecpu.flags & DF)	
	{
		SegOffDec(&(ecpu.ds),&(ecpu.si));
		SegOffDec(&(ecpu.es),&(ecpu.di));
	}
	else
	{
		SegOffInc(&(ecpu.ds),&(ecpu.si));
		SegOffInc(&(ecpu.es),&(ecpu.di));
	}
}
VOID MOVSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		//全局变量t不可以在一个赋值式的两边同时使用
		if (tmpAddrSize==2)
			*(t_nubit16 *)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart)=GetM16_16(ecpu.si);
		else
			*(t_nubit16 *)(ecpu.edi+((t2=ecpu.es,t2<<4))+MemoryStart)=GetM16_32(ecpu.esi);
		break;
	case 4:
		//全局变量t不可以在一个赋值式的两边同时使用
		if (tmpAddrSize==2)
			*(t_nubit32 *)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart)=GetM32_16(ecpu.si);
		else
			*(t_nubit32 *)(ecpu.edi+((t2=ecpu.es,t2<<4))+MemoryStart)=GetM32_32(ecpu.esi);
		break;
	}

/*	if (ecpu.di+((t2=ecpu.es,t2<<4))<0xc0000 && ecpu.di+((t2=ecpu.es,t2<<4))>=0xa0000)
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			WriteVideoRam(ecpu.di+((t2=ecpu.es,t2<<4))-0xa0000+i);
		}
	}*/
	if (ecpu.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(ecpu.ds),&(ecpu.si));		
			SegOffDec(&(ecpu.es),&(ecpu.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(ecpu.ds),&(ecpu.si));
			SegOffInc(&(ecpu.es),&(ecpu.di));
		}
	}
}
VOID CMPSB()
{
	//这里的t居然和全局变量t重了！
	//Release的时候，这里居然被编译器优化掉了。
	t_nubit8 ta=GetM8_16(ecpu.si);
	t_nubit8 tb;
	if (tmpAddrSize==2)
		tb=*(t_nubit8 *)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart);
	else
		tb=*(t_nubit8 *)(ecpu.edi+((t2=ecpu.es,t2<<4))+MemoryStart);
	__asm push ecpu.flags
	__asm popf
	//t_nubit8 ta=(t_nubit8)(GetM8(ecpu.si)-*(t_nubit8 *)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart));
	__asm mov al,ta
	__asm cmp al,tb
	__asm pushf
	__asm pop ecpu.flags
	if (ecpu.flags & DF)	
	{
		SegOffDec(&(ecpu.ds),&(ecpu.si));
		SegOffDec(&(ecpu.es),&(ecpu.di));		//这里理解错了，si和di都应该要变的		
	}
	else
	{
		SegOffInc(&(ecpu.ds),&(ecpu.si));
		SegOffInc(&(ecpu.es),&(ecpu.di));		
	}
}
VOID CMPSW()
{
	int i;
	t_nubit32 tevIP=evIP;
	void*ta;
	void*tb;
	if (tmpAddrSize==2)
	{
		ta=(void*)(ecpu.si+((t=tmpDs,t<<4))+MemoryStart);
		tb=(void*)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart);
	}
	else
	{
		ta=(void*)(ecpu.esi+((t=tmpDs,t<<4))+MemoryStart);
		tb=(void*)(ecpu.edi+((t2=ecpu.es,t2<<4))+MemoryStart);
	}
	CMP(&ta,&tb,tmpOpdSize);
	evIP=tevIP;
	if (ecpu.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(ecpu.ds),&(ecpu.si));
			SegOffDec(&(ecpu.es),&(ecpu.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(ecpu.ds),&(ecpu.si));
			SegOffInc(&(ecpu.es),&(ecpu.di));
		}
	}
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
VOID STOSB()
{
	if (tmpAddrSize==2)
		*(t_nubit8 *)(ecpu.di+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.al;
	else
		*(t_nubit8 *)(ecpu.edi+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.al;
	if (ecpu.flags & DF)	
	{
		SegOffDec(&(ecpu.es),&(ecpu.di));
	}
	else
	{
		SegOffInc(&(ecpu.es),&(ecpu.di));
	}
}
VOID STOSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		if (tmpAddrSize==2)
			*(t_nubit16 *)(ecpu.di+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.ax;
		else
			*(t_nubit16 *)(ecpu.edi+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.ax;
		break;
	case 4:
		if (tmpAddrSize==2)
			*(t_nubit32 *)(ecpu.di+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.eax;
		else
			*(t_nubit32 *)(ecpu.edi+((t=ecpu.es,t<<4))+MemoryStart)=ecpu.eax;
		break;
	}
	if (ecpu.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)		
			SegOffDec(&(ecpu.es),&(ecpu.di));				
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)		
			SegOffInc(&(ecpu.es),&(ecpu.di));		
	}
}
VOID LODSB()
{
	if (tmpAddrSize==2)
		ecpu.al=GetM8_16(ecpu.si);
	else
		ecpu.al=GetM8_32(ecpu.esi);
	if (ecpu.flags & DF)	
	{
		SegOffDec(&(ecpu.ds),&(ecpu.si));
	}
	else
	{
		SegOffInc(&(ecpu.ds),&(ecpu.si));
	}
}
VOID LODSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		if (tmpAddrSize==2)
			ecpu.ax=GetM16_16(ecpu.si);
		else
			ecpu.ax=GetM16_32(ecpu.esi);
		break;
	case 4:
		if (tmpAddrSize==2)
			ecpu.eax=GetM32_16(ecpu.si);
		else
			ecpu.eax=GetM32_32(ecpu.esi);
		break;
	}	

	if (ecpu.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
			SegOffDec(&(ecpu.ds),&(ecpu.si));		
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
			SegOffInc(&(ecpu.ds),&(ecpu.si));		
	}
}
VOID SCASB()
{
	t_nubit8 ta=ecpu.al;
	t_nubit8 tb;
	if (tmpAddrSize==2)
		tb=d_nubit8(ecpu.di+((t=ecpu.es,t<<4))+MemoryStart);
	else
		tb=d_nubit8(ecpu.edi+((t=ecpu.es,t<<4))+MemoryStart);
	__asm push ecpu.flags
	__asm popf
	//t_nubit8 ta=(t_nubit8)(ecpu.al-*(t_nubit8 *)(ecpu.di+((t=ecpu.es,t<<4))+MemoryStart));
	__asm mov al,ta
	__asm cmp al,tb
	__asm pushf
	__asm pop ecpu.flags	
	if (ecpu.flags & DF)	
	{
		SegOffDec(&(ecpu.es),&(ecpu.di));
	}
	else
	{
		SegOffInc(&(ecpu.es),&(ecpu.di));
	}
}
VOID SCASW()
{
	int i;
	t_nubit32 tevIP=evIP;
	void*ta=(void*)&ecpu.ax;
	void*tb;
	if (tmpAddrSize==2)
		tb=(void*)(ecpu.di+((t2=ecpu.es,t2<<4))+MemoryStart);
	else
		tb=(void*)(ecpu.edi+((t2=ecpu.es,t2<<4))+MemoryStart);
	CMP(&ta,&tb,tmpOpdSize);
	evIP=tevIP;
	if (ecpu.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(ecpu.es),&(ecpu.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(ecpu.es),&(ecpu.di));
		}
	}

}
// 0xB0
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	switch (tmpOpdSize)
	{
	case 2:	
		toe16;
		teIMS=*(t_nubit8 *)eIMS;
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
		teIMS=*(t_nubit8 *)eIMS;
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
	teIMS=*(t_nubit8 *)eIMS;
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
	ecpu.ip=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;	
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID RET_NEAR()
{
	ecpu.ip=*(t_nubit16*)(EvSP+MemoryStart);
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
		ecpu.es=*(t_nubit16 *)(rm16+1);
		break;
	case 4:
		toe32;
		*r32=*rm32;
		ecpu.es=*(t_nubit16 *)(rm32+1);
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
		ecpu.ds=*(t_nubit16 *)(rm16+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	case 4:
		toe32;	
		*r32=*rm32;
		ecpu.ds=*(t_nubit16 *)(rm32+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
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
	ecpu.ip=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;	
	ecpu.cs=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;	
	ecpu.sp+=*(t_nubit16*)eIMS;
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID RET_FAR()
{
	ecpu.ip=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;		
	ecpu.cs=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;		
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID INT3()
{
	GlobINT|=0x8;
}
VOID INT_I8()
{
	LongCallNewIP(1);
	ecpu.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.flags;
	ecpu.flags&=~IF;
	ecpu.flags&=~TF;
	ecpu.flags&=~AF;
	ecpu.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	ecpu.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
	ecpu.ip=*(t_nubit16 *)((*(t_nubit8 *)eIMS)*4+MemoryStart);	
	ecpu.cs=*(t_nubit16 *)((*(t_nubit8 *)eIMS)*4+2+MemoryStart);
	
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
}
VOID INTO()
{
	if (ecpu.flags&OF)
	{
		GlobINT|=0x10;
	}
// 	ecpu.sp-=2;
// 	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.cs;
// 	ecpu.sp-=2;
// 	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
// 	ecpu.ip=(4)*4+MemoryStart;
// 	evIP+=2;
// 	ecpu.cs=0;
// 	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
}
VOID IRET()					//在实模式下，iret和ret far是一样的，这里可以直接调用RET_FAR()的，不过为了以后扩展着想就不这样做。
{
	ecpu.ip=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;		
	ecpu.cs=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;		
	ecpu.flags=*(t_nubit16*)(EvSP+MemoryStart);
	ecpu.sp+=2;		
	evIP=((t=ecpu.cs,t<<4))+ecpu.ip;	
}
// 0xD0
VOID INS_D0()
{
	t_nubit8 t;
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	oce=*(char*)(evIP+MemoryStart) & 0x38;
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

// 只实现了以10为基数的情况，同AAD
VOID AAM()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
	if ((*(t_nubit8 *)eIMS)==0)
		GlobINT|=IntDE;
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

// 下面引自Intel开发者手册。这里的aad只实现了以10为基数的情况。即只实现了 D5 0A
// The generalized version of this instruction allows adjustment of two unpacked digits of any
// number base (refer to the "Operation" section below), by setting the imm8 byte to the selected
// number base (for example, 08H for octal, 0AH for decimal, or 0CH for base 12 numbers).
VOID AAD()
{
	t_bool intf = GetBit(ecpu.flags, VCPU_FLAG_IF);
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
VOID ESC_0()
{
	GlobINT|=0x80;
}
VOID ESC_1()
{	
	GlobINT|=0x80;
	toe16;				//这里纯粹是为了修改evIP
}
VOID ESC_2()
{
	GlobINT|=0x80;
}
VOID ESC_3()
{	
	GlobINT|=0x80;
	evIP++;
}
VOID ESC_4()
{
	GlobINT|=0x80;
}
VOID ESC_5()
{
	GlobINT|=0x80;
}
VOID ESC_6()
{
	GlobINT|=0x80;
}
VOID ESC_7()
{
	GlobINT|=0x80;
}
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
VOID IN_AL_N()
{
	ExecFun(InTable[d_nubit8(eIMS)]);
	ecpu.al = vport.iobyte;
	evIP++;
}
VOID IN_AX_N()
{
	// in ax,n 或者 in eax,n 是连续地读端口。InTable里的函数都是直接修改ecpu.al的，最好把每个ecpu.al都保存起来，再一次过赋给ecpu.ax或ecpu.eax
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		ExecFun(InTable[d_nubit8(eIMS)+i]);
		d_nubit8(&ecpu.al+i)=vport.iobyte;
	}
	evIP++;
}
VOID OUT_N_AL()
{
	vport.iobyte = ecpu.al;
	ExecFun(OutTable[d_nubit8(eIMS)]);
	evIP++;
}
VOID OUT_N_AX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		vport.iobyte = d_nubit8(&ecpu.ax+i);
		ExecFun(OutTable[d_nubit8(eIMS)+i]);
	}
	evIP++;
}
VOID CALL_NEAR()
{
	LongCallNewIP(2);

	ecpu.sp-=2;										//段内CALL，CS不压栈
	*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
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
VOID IN_AL_DX()
{
	ExecFun(InTable[ecpu.dx]);
	ecpu.al = vport.iobyte;
}
VOID IN_AX_DX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		ExecFun(InTable[ecpu.dx+i]);
		d_nubit8(&ecpu.al+i)=vport.iobyte;
	}
}
VOID OUT_DX_AL()
{
	vport.iobyte = ecpu.al;
	ExecFun(OutTable[ecpu.dx]);
}
VOID OUT_DX_AX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		vport.iobyte = d_nubit8(&ecpu.al+i);
		ExecFun(OutTable[ecpu.dx+i]);
	}
}

// 0xF0
VOID LOCK()
{
	__asm nop;
}
VOID REPNE()
{
	t_nubit32 tevIP=evIP;
	t_nubit8 nopc;
	if (ecpu.cx==0)
	{
		nopc=d_nubit8(MemoryStart+evIP);
		if ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是CS、ES之类的指令，则再跳一字节
		{
			evIP++;
			nopc=d_nubit8(MemoryStart+evIP);
		}
		evIP++;
	}
	ecpu.flags &= (0xffff ^ ZF);		//ZF位置0
	while (ecpu.cx>0 && !(ecpu.flags & ZF))
	{
		evIP=tevIP;
		ecpuinsExecIns();
		ecpu.cx--;
	}
	//evIP++;			//串操作指令都是一字节长的
}
VOID REP()
{
	t_nubit32 tevIP=evIP;
	t_nubit8 nopc;
	ecpu.flags |= ZF;		//ZF位置1
	if (ecpu.cx==0)
	{
		nopc=d_nubit8(MemoryStart+evIP);		
		while ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是指令前缀，则再跳一字节
		{
			evIP++;
			nopc=d_nubit8(MemoryStart+evIP);
		}
		evIP++;						//MOVSB之类的指令只有1字节长		
	}
	while (ecpu.cx>0 && (ecpu.flags & ZF))
	{
		evIP=tevIP;
		ecpuinsExecIns();
		ecpu.cx--;
	}
	//evIP++;			//串操作指令都是一字节长的
}
VOID HLT()
{
	while (0)	//这里要检查是否有外部中断
	{
		__asm nop
	}
}
VOID CMC()
{
	ecpu.flags^=CF;
}
VOID INS_F6()
{
	char oce,trm8,tc;
	oce=*(char*)(evIP+MemoryStart) & 0x38;
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
		else GlobINT |= 0x01;
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
		else GlobINT |= 0x01;
		break;
	}
}
VOID INS_F7()
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
				GlobINT|=1;
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
				GlobINT|=1;
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
				GlobINT|=1;
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
				GlobINT|=1;
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
	oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	mod&=0x3;
	rem=*(char*)(evIP+MemoryStart) & 0x07;
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
		*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
		JMP_NEAR_LABEL();		
		break;
	case 3:		
		//__asm popf
		ecpu.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
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
		*(t_nubit16*)( EvSP +MemoryStart)=(t_nubit16)*rm8;
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
	oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	mod&=0x3;
	rem=*(char*)(evIP+MemoryStart) & 0x07;
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
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
			ecpu.ip=*rm16;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		case 4:
			ecpu.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
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
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.cs;
			ecpu.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
			ecpu.ip=*rm16;
			ecpu.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=ecpu.cs,t<<4))+ecpu.ip;
			break;
		case 4:
			ecpu.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.cs;
			ecpu.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=ecpu.ip;
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
			PUSH(rm16,tmpOpdSize);
			break;
		case 4:
			PUSH(rm32,tmpOpdSize);
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
	t_nubit8 OpC=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit32 tcs,InsFucAddr;
	evIP++;
	InsFucAddr=Ins0FTable[OpC];
	__asm call InsFucAddr;
	tcs=ecpu.cs;
	ecpu.ip=(evIP - (tcs << 4)) % 0x10000;
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
}

//////////////////////////////////////////////////////////////////////////
// 下面这部分是0F开头的指令
VOID ADDPS()
{
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
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
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
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
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
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
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
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
	t_nubit8 OpC=*(t_nubit8 *)(evIP+MemoryStart);
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
	t_nubit8 OpC=*(t_nubit8 *)(evIP+MemoryStart);
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
	POP(&ecpu.fs,2);
}
VOID INS_0F01()
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
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
	qdbiosExecInt(d_nubit8(eIMS));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_ZF, GetBit(_flags, VCPU_FLAG_ZF));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_CF, GetBit(_flags, VCPU_FLAG_CF));
	evIP++;
}

// yinX：	用这么笨的方法是为了让以后修改某条指令的时候更灵活
// 路人甲：	真的会更灵活吗？
// yinX：	…… -_-!

//////////////////////////////////////////////////////////////////////////
// 前往某个中断服务程序
VOID GoInt(int r)
{
	//LongCallNewIP(1);;					//因为ExcIns里面是先执行指令再修改CS:IP的，所以运行到这里的CS:IP是未被执行的，所以不用再LongCallNewIP了。
	t_vaddrcc t;
//	vapiPrint("goint.1: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	ecpu.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = ecpu.flags;
	ecpu.flags &= ~IF;
	ecpu.flags &= ~TF;
	ecpu.flags &= ~AF;
	ecpu.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = ecpu.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	ecpu.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = ecpu.ip;
	ecpu.ip = d_nubit16((r) * 4 + MemoryStart + 0);
	ecpu.cs = d_nubit16((r) * 4 + MemoryStart + 2);
	evIP = (ecpu.cs << 4) + ecpu.ip;
//	vapiPrint("goint.2: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
}

//////////////////////////////////////////////////////////////////////////
// 执行evIP所指向的那一条指令
void ecpuinsExecIns()
{
	t_nubit8 opcode = d_nubit8(evIP+MemoryStart);
	t_nubit8 op1 = d_nubit8(evIP+MemoryStart+1);
	t_nubit8 op2 = d_nubit8(evIP+MemoryStart+2);
	t_nubit8 op3 = d_nubit8(evIP+MemoryStart+3);
	t_nubit8 op4 = d_nubit8(evIP+MemoryStart+4);
//	t_faddrcc InsFucAddr = InsTable[opcode];
//	vapiPrint("execins.1: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	evIP++;

	//InstructionCount++;
	ExecFun(InsTable[opcode]);
//	__asm call InsFucAddr;
	ecpu.ip=(evIP - (ecpu.cs << 4)) % 0x10000;
	tmpDs=ecpu.ds;
	tmpSs=ecpu.ss;
//	vapiPrint("execins.2: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
}

//////////////////////////////////////////////////////////////////////////
// 检查是否有中断请求，并且作出相应处理
void ecpuinsExecInt()
{	
	char intR;
	unsigned int tmpGlobINT;
	unsigned int IntNo;

	if (ecpu.flags&TF) {
		GlobINT|=0x2;
		ecpu.flags&=~TF;
	}
	tmpGlobINT=GlobINT;
	IntNo=0;
	while (!(tmpGlobINT&1) && IntNo<8) {		//所有少于7的中断（即不连接到8259上的中断）
		IntNo++;
	}

	if (IntNo==2) {
		GoInt(2);							//不可屏蔽中断
		GlobINT &= ~0x2;
		return;
	}

	if (ecpu.flags & IF) {	
		if (IntNo < 8) {
			GoInt(IntNo);
			GlobINT&=~(unsigned int)(1<<IntNo);
			return;
		}
		if (vpicScanINTR()) {
			intR=vpicGetINTR();
			GoInt(intR);
		}
	}
}

void ecpuinsInit()
{
	InsTable[0x00]=(t_faddrcc)ADD_RM8_R8;
	InsTable[0x01]=(t_faddrcc)ADD_RM16_R16;
	InsTable[0x02]=(t_faddrcc)ADD_R8_RM8;
	InsTable[0x03]=(t_faddrcc)ADD_R16_RM16;
	InsTable[0x04]=(t_faddrcc)ADD_AL_I8;
	InsTable[0x05]=(t_faddrcc)ADD_AX_I16;
	InsTable[0x06]=(t_faddrcc)PUSH_ES;
	InsTable[0x07]=(t_faddrcc)POP_ES;
	InsTable[0x08]=(t_faddrcc)OR_RM8_R8;
	InsTable[0x09]=(t_faddrcc)OR_RM16_R16;
	InsTable[0x0A]=(t_faddrcc)OR_R8_RM8;
	InsTable[0x0B]=(t_faddrcc)OR_R16_RM16;
	InsTable[0x0C]=(t_faddrcc)OR_AL_I8;
	InsTable[0x0D]=(t_faddrcc)OR_AX_I16;
	InsTable[0x0E]=(t_faddrcc)PUSH_CS;
	InsTable[0x0F]=(t_faddrcc)INS_0F;
	InsTable[0x10]=(t_faddrcc)ADC_RM8_R8;
	InsTable[0x11]=(t_faddrcc)ADC_RM16_R16;
	InsTable[0x12]=(t_faddrcc)ADC_R8_RM8;
	InsTable[0x13]=(t_faddrcc)ADC_R16_RM16;
	InsTable[0x14]=(t_faddrcc)ADC_AL_I8;
	InsTable[0x15]=(t_faddrcc)ADC_AX_I16;
	InsTable[0x16]=(t_faddrcc)PUSH_SS;
	InsTable[0x17]=(t_faddrcc)POP_SS;
	InsTable[0x18]=(t_faddrcc)SBB_RM8_R8;
	InsTable[0x19]=(t_faddrcc)SBB_RM16_R16;
	InsTable[0x1A]=(t_faddrcc)SBB_R8_RM8;
	InsTable[0x1B]=(t_faddrcc)SBB_R16_RM16;
	InsTable[0x1C]=(t_faddrcc)SBB_AL_I8;
	InsTable[0x1D]=(t_faddrcc)SBB_AX_I16;
	InsTable[0x1E]=(t_faddrcc)PUSH_DS;
	InsTable[0x1F]=(t_faddrcc)POP_DS;
	InsTable[0x20]=(t_faddrcc)AND_RM8_R8;
	InsTable[0x21]=(t_faddrcc)AND_RM16_R16;
	InsTable[0x22]=(t_faddrcc)AND_R8_RM8;
	InsTable[0x23]=(t_faddrcc)AND_R16_RM16;
	InsTable[0x24]=(t_faddrcc)AND_AL_I8;
	InsTable[0x25]=(t_faddrcc)AND_AX_I16;
	InsTable[0x26]=(t_faddrcc)ES;
	InsTable[0x27]=(t_faddrcc)DAA;
	InsTable[0x28]=(t_faddrcc)SUB_RM8_R8;
	InsTable[0x29]=(t_faddrcc)SUB_RM16_R16;
	InsTable[0x2A]=(t_faddrcc)SUB_R8_RM8;
	InsTable[0x2B]=(t_faddrcc)SUB_R16_RM16;
	InsTable[0x2C]=(t_faddrcc)SUB_AL_I8;
	InsTable[0x2D]=(t_faddrcc)SUB_AX_I16;
	InsTable[0x2E]=(t_faddrcc)CS;
	InsTable[0x2F]=(t_faddrcc)DAS;
	InsTable[0x30]=(t_faddrcc)XOR_RM8_R8;
	InsTable[0x31]=(t_faddrcc)XOR_RM16_R16;
	InsTable[0x32]=(t_faddrcc)XOR_R8_RM8;
	InsTable[0x33]=(t_faddrcc)XOR_R16_RM16;
	InsTable[0x34]=(t_faddrcc)XOR_AL_I8;
	InsTable[0x35]=(t_faddrcc)XOR_AX_I16;
	InsTable[0x36]=(t_faddrcc)SS;
	InsTable[0x37]=(t_faddrcc)AAA;
	InsTable[0x38]=(t_faddrcc)CMP_RM8_R8;
	InsTable[0x39]=(t_faddrcc)CMP_RM16_R16;
	InsTable[0x3A]=(t_faddrcc)CMP_R8_RM8;
	InsTable[0x3B]=(t_faddrcc)CMP_R16_RM16;
	InsTable[0x3C]=(t_faddrcc)CMP_AL_I8;
	InsTable[0x3D]=(t_faddrcc)CMP_AX_I16;
	InsTable[0x3E]=(t_faddrcc)DS;
	InsTable[0x3F]=(t_faddrcc)AAS;
	InsTable[0x40]=(t_faddrcc)INC_AX;
	InsTable[0x41]=(t_faddrcc)INC_CX;
	InsTable[0x42]=(t_faddrcc)INC_DX;
	InsTable[0x43]=(t_faddrcc)INC_BX;
	InsTable[0x44]=(t_faddrcc)INC_SP;
	InsTable[0x45]=(t_faddrcc)INC_BP;
	InsTable[0x46]=(t_faddrcc)INC_SI;
	InsTable[0x47]=(t_faddrcc)INC_DI;
	InsTable[0x48]=(t_faddrcc)DEC_AX;
	InsTable[0x49]=(t_faddrcc)DEC_CX;
	InsTable[0x4A]=(t_faddrcc)DEC_DX;
	InsTable[0x4B]=(t_faddrcc)DEC_BX;
	InsTable[0x4C]=(t_faddrcc)DEC_SP;
	InsTable[0x4D]=(t_faddrcc)DEC_BP;
	InsTable[0x4E]=(t_faddrcc)DEC_SI;
	InsTable[0x4F]=(t_faddrcc)DEC_DI;
	InsTable[0x50]=(t_faddrcc)PUSH_AX;
	InsTable[0x51]=(t_faddrcc)PUSH_CX;
	InsTable[0x52]=(t_faddrcc)PUSH_DX;
	InsTable[0x53]=(t_faddrcc)PUSH_BX;
	InsTable[0x54]=(t_faddrcc)PUSH_SP;
	InsTable[0x55]=(t_faddrcc)PUSH_BP;
	InsTable[0x56]=(t_faddrcc)PUSH_SI;
	InsTable[0x57]=(t_faddrcc)PUSH_DI;
	InsTable[0x58]=(t_faddrcc)POP_AX;
	InsTable[0x59]=(t_faddrcc)POP_CX;
	InsTable[0x5A]=(t_faddrcc)POP_DX;
	InsTable[0x5B]=(t_faddrcc)POP_BX;
	InsTable[0x5C]=(t_faddrcc)POP_SP;
	InsTable[0x5D]=(t_faddrcc)POP_BP;
	InsTable[0x5E]=(t_faddrcc)POP_SI;
	InsTable[0x5F]=(t_faddrcc)POP_DI;
	InsTable[0x60]=(t_faddrcc)OpcError;
	InsTable[0x61]=(t_faddrcc)OpcError;
	InsTable[0x62]=(t_faddrcc)OpcError;
	InsTable[0x63]=(t_faddrcc)OpcError;
	InsTable[0x64]=(t_faddrcc)OpcError;
	InsTable[0x65]=(t_faddrcc)OpcError;
	InsTable[0x66]=(t_faddrcc)OpdSize;
	InsTable[0x67]=(t_faddrcc)AddrSize;
	InsTable[0x68]=(t_faddrcc)PUSH_I16;
	InsTable[0x69]=(t_faddrcc)OpcError;
	InsTable[0x6A]=(t_faddrcc)OpcError;
	InsTable[0x6B]=(t_faddrcc)OpcError;
	InsTable[0x6C]=(t_faddrcc)OpcError;
	InsTable[0x6D]=(t_faddrcc)OpcError;
	InsTable[0x6E]=(t_faddrcc)OpcError;
	InsTable[0x6F]=(t_faddrcc)OpcError;
	InsTable[0x70]=(t_faddrcc)JO;
	InsTable[0x71]=(t_faddrcc)JNO;
	InsTable[0x72]=(t_faddrcc)JC;
	InsTable[0x73]=(t_faddrcc)JNC;
	InsTable[0x74]=(t_faddrcc)JZ;
	InsTable[0x75]=(t_faddrcc)JNZ;
	InsTable[0x76]=(t_faddrcc)JBE;
	InsTable[0x77]=(t_faddrcc)JA;
	InsTable[0x78]=(t_faddrcc)JS;
	InsTable[0x79]=(t_faddrcc)JNS;
	InsTable[0x7A]=(t_faddrcc)JP;
	InsTable[0x7B]=(t_faddrcc)JNP;
	InsTable[0x7C]=(t_faddrcc)JL;
	InsTable[0x7D]=(t_faddrcc)JNL;
	InsTable[0x7E]=(t_faddrcc)JLE;
	InsTable[0x7F]=(t_faddrcc)JG;
	InsTable[0x80]=(t_faddrcc)INS_80;
	InsTable[0x81]=(t_faddrcc)INS_81;
	InsTable[0x82]=(t_faddrcc)INS_82;
	InsTable[0x83]=(t_faddrcc)INS_83;
	InsTable[0x84]=(t_faddrcc)TEST_RM8_M8;
	InsTable[0x85]=(t_faddrcc)TEST_RM16_M16;
	InsTable[0x86]=(t_faddrcc)XCHG_R8_RM8;
	InsTable[0x87]=(t_faddrcc)XCHG_R16_RM16;
	InsTable[0x88]=(t_faddrcc)MOV_RM8_R8;
	InsTable[0x89]=(t_faddrcc)MOV_RM16_R16;
	InsTable[0x8A]=(t_faddrcc)MOV_R8_RM8;
	InsTable[0x8B]=(t_faddrcc)MOV_R16_RM16;
	InsTable[0x8C]=(t_faddrcc)MOV_RM_SEG;
	InsTable[0x8D]=(t_faddrcc)LEA_R16_M16;
	InsTable[0x8E]=(t_faddrcc)MOV_SEG_RM;
	InsTable[0x8F]=(t_faddrcc)POP_RM16;
	InsTable[0x90]=(t_faddrcc)NOP;
	InsTable[0x91]=(t_faddrcc)XCHG_CX_AX;
	InsTable[0x92]=(t_faddrcc)XCHG_DX_AX;
	InsTable[0x93]=(t_faddrcc)XCHG_BX_AX;
	InsTable[0x94]=(t_faddrcc)XCHG_SP_AX;
	InsTable[0x95]=(t_faddrcc)XCHG_BP_AX;
	InsTable[0x96]=(t_faddrcc)XCHG_SI_AX;
	InsTable[0x97]=(t_faddrcc)XCHG_DI_AX;
	InsTable[0x98]=(t_faddrcc)CBW;
	InsTable[0x99]=(t_faddrcc)CWD;
	InsTable[0x9A]=(t_faddrcc)CALL_FAR;
	InsTable[0x9B]=(t_faddrcc)WAIT;
	InsTable[0x9C]=(t_faddrcc)PUSHF;
	InsTable[0x9D]=(t_faddrcc)POPF;
	InsTable[0x9E]=(t_faddrcc)SAHF;
	InsTable[0x9F]=(t_faddrcc)LAHF;
	InsTable[0xA0]=(t_faddrcc)MOV_AL_M8;
	InsTable[0xA1]=(t_faddrcc)MOV_AX_M16;
	InsTable[0xA2]=(t_faddrcc)MOV_M8_AL;
	InsTable[0xA3]=(t_faddrcc)MOV_M16_AX;
	InsTable[0xA4]=(t_faddrcc)MOVSB;
	InsTable[0xA5]=(t_faddrcc)MOVSW;
	InsTable[0xA6]=(t_faddrcc)CMPSB;
	InsTable[0xA7]=(t_faddrcc)CMPSW;
	InsTable[0xA8]=(t_faddrcc)TEST_AL_I8;
	InsTable[0xA9]=(t_faddrcc)TEST_AX_I16;
	InsTable[0xAA]=(t_faddrcc)STOSB;
	InsTable[0xAB]=(t_faddrcc)STOSW;
	InsTable[0xAC]=(t_faddrcc)LODSB;
	InsTable[0xAD]=(t_faddrcc)LODSW;
	InsTable[0xAE]=(t_faddrcc)SCASB;
	InsTable[0xAF]=(t_faddrcc)SCASW;
	InsTable[0xB0]=(t_faddrcc)MOV_AL_I8;
	InsTable[0xB1]=(t_faddrcc)MOV_CL_I8;
	InsTable[0xB2]=(t_faddrcc)MOV_DL_I8;
	InsTable[0xB3]=(t_faddrcc)MOV_BL_I8;
	InsTable[0xB4]=(t_faddrcc)MOV_AH_I8;
	InsTable[0xB5]=(t_faddrcc)MOV_CH_I8;
	InsTable[0xB6]=(t_faddrcc)MOV_DH_I8;
	InsTable[0xB7]=(t_faddrcc)MOV_BH_I8;
	InsTable[0xB8]=(t_faddrcc)MOV_AX_I16;
	InsTable[0xB9]=(t_faddrcc)MOV_CX_I16;
	InsTable[0xBA]=(t_faddrcc)MOV_DX_I16;
	InsTable[0xBB]=(t_faddrcc)MOV_BX_I16;
	InsTable[0xBC]=(t_faddrcc)MOV_SP_I16;
	InsTable[0xBD]=(t_faddrcc)MOV_BP_I16;
	InsTable[0xBE]=(t_faddrcc)MOV_SI_I16;
	InsTable[0xBF]=(t_faddrcc)MOV_DI_I16;
	InsTable[0xC0]=(t_faddrcc)INS_C0;
	InsTable[0xC1]=(t_faddrcc)INS_C1;
	InsTable[0xC2]=(t_faddrcc)RET_I8;
	InsTable[0xC3]=(t_faddrcc)RET_NEAR;
	InsTable[0xC4]=(t_faddrcc)LES_R16_M16;
	InsTable[0xC5]=(t_faddrcc)LDS_R16_M16;
	InsTable[0xC6]=(t_faddrcc)MOV_M8_I8;
	InsTable[0xC7]=(t_faddrcc)MOV_M16_I16;
	InsTable[0xC8]=(t_faddrcc)OpcError;
	InsTable[0xC9]=(t_faddrcc)OpcError;
	InsTable[0xCA]=(t_faddrcc)RET_I16;
	InsTable[0xCB]=(t_faddrcc)RET_FAR;
	InsTable[0xCC]=(t_faddrcc)INT3;
	InsTable[0xCD]=(t_faddrcc)INT_I8;
	InsTable[0xCE]=(t_faddrcc)INTO;
	InsTable[0xCF]=(t_faddrcc)IRET;
	InsTable[0xD0]=(t_faddrcc)INS_D0;
	InsTable[0xD1]=(t_faddrcc)INS_D1;
	InsTable[0xD2]=(t_faddrcc)INS_D2;
	InsTable[0xD3]=(t_faddrcc)INS_D3;
	InsTable[0xD4]=(t_faddrcc)AAM;
	InsTable[0xD5]=(t_faddrcc)AAD;
	InsTable[0xD6]=(t_faddrcc)OpcError;
	InsTable[0xD7]=(t_faddrcc)XLAT;
	InsTable[0xD8]=(t_faddrcc)OpcError;
	InsTable[0xD9]=(t_faddrcc)INS_D9;
	InsTable[0xDA]=(t_faddrcc)OpcError;
	InsTable[0xDB]=(t_faddrcc)INS_DB;
	InsTable[0xDC]=(t_faddrcc)OpcError;
	InsTable[0xDD]=(t_faddrcc)OpcError;
	InsTable[0xDE]=(t_faddrcc)OpcError;
	InsTable[0xDF]=(t_faddrcc)OpcError;
	InsTable[0xE0]=(t_faddrcc)LOOPNE;
	InsTable[0xE1]=(t_faddrcc)LOOPE;
	InsTable[0xE2]=(t_faddrcc)LOOP_NEAR;
	InsTable[0xE3]=(t_faddrcc)JCXZ_NEAR;
	InsTable[0xE4]=(t_faddrcc)IN_AL_N;
	InsTable[0xE5]=(t_faddrcc)IN_AX_N;
	InsTable[0xE6]=(t_faddrcc)OUT_N_AL;
	InsTable[0xE7]=(t_faddrcc)OUT_N_AX;
	InsTable[0xE8]=(t_faddrcc)CALL_NEAR;
	InsTable[0xE9]=(t_faddrcc)JMP_NEAR_LABEL;
	InsTable[0xEA]=(t_faddrcc)JMP_FAR_LABEL;
	InsTable[0xEB]=(t_faddrcc)JMP_NEAR;
	InsTable[0xEC]=(t_faddrcc)IN_AL_DX;
	InsTable[0xED]=(t_faddrcc)IN_AX_DX;
	InsTable[0xEE]=(t_faddrcc)OUT_DX_AL;
	InsTable[0xEF]=(t_faddrcc)OUT_DX_AX;
	InsTable[0xF0]=(t_faddrcc)LOCK;
	InsTable[0xF1]=(t_faddrcc)QDX;
	InsTable[0xF2]=(t_faddrcc)REPNE;
	InsTable[0xF3]=(t_faddrcc)REP;
	InsTable[0xF4]=(t_faddrcc)HLT;
	InsTable[0xF5]=(t_faddrcc)CMC;
	InsTable[0xF6]=(t_faddrcc)INS_F6;
	InsTable[0xF7]=(t_faddrcc)INS_F7;
	InsTable[0xF8]=(t_faddrcc)CLC;
	InsTable[0xF9]=(t_faddrcc)STC;
	InsTable[0xFA]=(t_faddrcc)CLI;
	InsTable[0xFB]=(t_faddrcc)STI;
	InsTable[0xFC]=(t_faddrcc)CLD;
	InsTable[0xFD]=(t_faddrcc)STD;
	InsTable[0xFE]=(t_faddrcc)INS_FE;
	InsTable[0xFF]=(t_faddrcc)INS_FF;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
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
void ecpuinsFinal() {}