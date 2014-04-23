//////////////////////////////////////////////////////////////////////////
// 名称：Instruction.cpp
// 功能：模拟8086指令集
// 日期：2008年4月20日
// 作者：梁一信
//////////////////////////////////////////////////////////////////////////

#if (1)

#include "../vglobal.h"
#include "../vapi.h"
#include "../qdbios.h"
#include "Instruction.h"
#include "cpu.h"

t_faddrcc InsTable[0x100];
t_vaddrcc Ins0FTable[0x100];

#define eIMS (evIP+MemoryStart)
#define EvSP (t=eCPU.ss,(t<<4)+eCPU.sp)
#define evSI (t=tmpDs,(t<<4)+eCPU.si)
#define evDI (t=eCPU.es,(t<<4)+eCPU.di)
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


//在执行每一条指令之前，evIP要等于cs:ip。
//执行完一条指令之后，再按evIP修改cs:ip。
void LongCallNewIP(char OffsetByte)
{
	t_nubit32 tcs=eCPU.cs;
	t_nubit32 tevIP=evIP;
	tcs<<=4;
	tevIP+=OffsetByte;
	tevIP-=tcs;
	eCPU.cs+=tevIP/0x10000;
	eCPU.ip=tevIP%0x10000;
}
void SegOffInc(t_nubit16 *seg, t_nubit16 *off)
{
		(*off)++;
}
void SegOffDec(t_nubit16 *seg, t_nubit16 *off)
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
void SetM8(t_nubit16 off, t_nubit8 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit8 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;

}
void SetM16(t_nubit16 off, t_nubit16 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit16 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
}
void SetM32(t_nubit16 off, t_nubit32 val)
{
	if (off+(t=tmpDs,t<<4))
		*(t_nubit32 *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
}
void * FindRegAddr(t_bool w,char reg)
{
	switch(reg)
	{
	case 0:
		return &(eCPU.al);
		break;
	case 1:
		return &(eCPU.cl);
		break;
	case 2:
		return &(eCPU.dl);
		break;
	case 3:
		return &(eCPU.bl);
		break;
	case 4:
		if (w)
			return &(eCPU.sp);
		else
			return &(eCPU.ah);
		break;
	case 5:
		if (w)
			return &(eCPU.bp);
		else
			return &(eCPU.ch);
		break;
	case 6:
		if (w)
			return &(eCPU.si);
		else
			return &(eCPU.dh);
		break;
	case 7:
		if (w)
			return &(eCPU.di);
		else
			return &(eCPU.bh);
		break;
	default:
		return 0;
	}
}
void * FindSegAddr(t_bool w,char reg)
{
	reg&=0x03;
	switch(reg)
	{
	case 0:
		return &(eCPU.es);
		break;
	case 1:
		return &(eCPU.cs);
		break;
	case 2:
		return &(eCPU.ss);
		break;
	case 3:
		return &(eCPU.ds);
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
	tes=eCPU.es;
	tes<<=4;
	tss=tmpSs;
	tss<<=4;
	if (tmpAddrSize==2)
	{
		switch(rem)
		{
		case 0:
			*off1=&eCPU.bx;
			*off2=&eCPU.si;
			ret=MemoryStart+(t_nubit16)(eCPU.bx+eCPU.si)+(tds);
			break;
		case 1:
			*off1=&eCPU.bx;
			*off2=&eCPU.di;
			ret=MemoryStart+(t_nubit16)(eCPU.bx+eCPU.di)+(tds);
			break;
		case 2:
			*off1=&eCPU.bp;
			*off2=&eCPU.si;
			ret=MemoryStart+(t_nubit16)(eCPU.bp+eCPU.si)+(tss);
			break;
		case 3:
			*off1=&eCPU.bp;
			*off2=&eCPU.di;
			ret=MemoryStart+(t_nubit16)(eCPU.bp+eCPU.di)+(tss);
			break;
		case 4:
			*off1=&eCPU.si;	
			*off2=&GlbZero;
			ret=MemoryStart+eCPU.si+(tds);
			break;
		case 5:
			*off1=&eCPU.di;
			*off2=&GlbZero;
			ret=MemoryStart+eCPU.di+(tds);
			break;
		case 6:
			*off1=&eCPU.bp;
			*off2=&GlbZero;
			ret=MemoryStart+eCPU.bp+(tss);
			break;
		case 7:
			*off1=&eCPU.bx;
			*off2=&GlbZero;
			ret=MemoryStart+eCPU.bx+(tds);
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
			*off1=&eCPU.ax;
			ret=MemoryStart+(t_nubit16)eCPU.eax+(tds);
			break;
		case 1:
			*off1=&eCPU.cx;
			ret=MemoryStart+(t_nubit16)eCPU.ecx+(tds);
			break;
		case 2:
			*off1=&eCPU.dx;
			ret=MemoryStart+(t_nubit16)eCPU.edx+(tss);
			break;
		case 3:
			*off1=&eCPU.bx;
			ret=MemoryStart+(t_nubit16)eCPU.ebx+(tss);
			break;
		case 4:
			__asm nop		//SIB followed
			break;
		case 5:
			*off1=&eCPU.bp;
			ret=MemoryStart+eCPU.ebp+(tds);
			break;
		case 6:
			*off1=&eCPU.si;
			ret=MemoryStart+eCPU.esi+(tss);
			break;
		case 7:
			*off1=&eCPU.di;
			ret=MemoryStart+eCPU.edi+(tds);
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
void TranslateOpcExt(t_bool w,char** rg,char** rm)
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
void TranslateOpcExtSeg(t_bool w,char** rg,char** rm)
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
t_bool Bit(void *BitBase, int BitOffset)
{
	t_nubit32 tmp=(t_nubit32)BitBase;
	tmp+=BitOffset/8;
	BitBase=(void *)tmp;
	return (*(t_nubit8 *)BitBase>>(BitOffset%8))&1;
}
// 读到的字节未编码指令
void OpcError()
{
	t_nubit8 *pc=(t_nubit8 *)MemoryStart+evIP-1;
	vapiPrint("An unkown instruction [ %2x %2x %2x %2x %2x %2x ] was read at [ %4xh:%4xh ], easyVM only support 8086 instruction set in this version. easyVM will be terminated.",*(pc),*(pc+1),*(pc+2),*(pc+3),*(pc+4),*(pc+5),eCPU.cs,eCPU.ip);
	vapiCallBackMachineStop();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 高度抽象部分

void ADD(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)+=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)+=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)+=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void PUSH(void *Src,int Len)
{
	switch(Len)
	{
	case 2:
		eCPU.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=*(t_nubit16 *)Src;
		break;
	case 4:
		eCPU.sp-=4;
		*(t_nubit32*)( EvSP +MemoryStart)=*(t_nubit32 *)Src;
		break;
	}
}
void POP(void *Des, int Len)
{
	switch(Len)
	{
	case 2:
		d_nubit16(Des)=*(t_nubit16*)(EvSP+MemoryStart);
		eCPU.sp+=2;
		break;
	case 4:
		d_nubit32(Des)=*(t_nubit32*)(EvSP+MemoryStart);
		eCPU.sp+=4;
		break;
	}
}
void OR(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)|=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)|=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)|=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void ADC(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des);
		t82=d_nubit8(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov al,t81
		__asm adc al,t82	
		__asm mov t81,al
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit8(*Des)=t81;
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des);
		t162=d_nubit16(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov ax,t161
		__asm adc ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit16(*Des)=t161;
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des);
		t322=d_nubit32(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov eax,t321
		__asm adc eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit32(*Des)=t321;
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void SBB(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des);
		t82=d_nubit8(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit8(*Des)=t81;
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des);
		t162=d_nubit16(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov ax,t161
		__asm sbb ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit16(*Des)=t161;
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des);
		t322=d_nubit32(*Src);
		__asm push eCPU.eflags
		__asm popfd
		__asm mov eax,t321
		__asm sbb eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit32(*Des)=t321;
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void AND(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)&=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)&=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)&=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void SUB(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)-=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)-=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)-=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void XOR(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit8(*Des)^=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)^=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)^=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void CMP(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	t_nubit8 opr11,opr12;
	t_nubit16 opr21,opr22;
	t_nubit32 opr41,opr42;
	switch(Len)
	{
	case 1:
		toe8;
		opr11 = d_nubit8(*Des);
		opr12 = d_nubit8(*Src);
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		__asm mov al, opr11
		__asm cmp al, opr12
//		d_nubit8(*Des)-=d_nubit8(*Src);
		__asm pushfd
		__asm pop eCPU.eflags
//		d_nubit8(*Des)+=d_nubit8(*Src);
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit16(*Des)-=d_nubit16(*Src);
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit16(*Des)+=d_nubit16(*Src);
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		d_nubit32(*Des)-=d_nubit32(*Src);
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit32(*Des)+=d_nubit32(*Src);
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void INC(void *Des, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 2:
		t161=d_nubit16(Des);
		__asm push eCPU.eflags
		__asm popfd
		__asm inc t161
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit16(Des)=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		__asm push eCPU.eflags
		__asm popfd
		__asm inc t321
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit32(Des)=t321;
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void DEC(void *Des, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 2:
		t161=d_nubit16(Des);
		__asm push eCPU.eflags
		__asm popfd
		__asm dec t161
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit16(Des)=t161;
		break;
	case 4:
		t321=d_nubit32(Des);
		__asm push eCPU.eflags
		__asm popfd
		__asm dec t321
		__asm pushfd
		__asm pop eCPU.eflags
		d_nubit32(Des)=t321;
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void TEST(void **Des, void **Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	switch(Len)
	{
	case 1:
		toe8;
		t81=d_nubit8(*Des),t82=d_nubit8(*Src);
		__asm mov al,t82
		__asm push eCPU.eflags
		__asm popfd
		__asm test t81,al			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	case 2:
		toe16;
		t161=d_nubit16(*Des),t162=d_nubit16(*Src);
		__asm mov ax,t162
		__asm push eCPU.eflags
		__asm popfd
		__asm test t161,ax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	case 4:
		toe32;
		t321=d_nubit32(*Des),t322=d_nubit32(*Src);
		__asm mov eax,t322
		__asm push eCPU.eflags
		__asm popfd
		__asm test t321,eax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void XCHG(void *Des, void *Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
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
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void MOV(void *Des, void *Src, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
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
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void SHL(void *Des, t_nubit8 mb, int Len)
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
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
			push eCPU.eflags
			popfd
			shl al,cl
			pushfd
			pop eCPU.eflags
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
			push eCPU.eflags
			popfd
			shl ax,cl
			pushfd
			pop eCPU.eflags
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
			push eCPU.eflags
			popfd
			shl eax,cl
			pushfd
			pop eCPU.eflags
			mov tSHLrm32,eax
		}
		d_nubit32(Des)=tSHLrm8;
		break;
	}
	MakeBit(eCPU.flags, VCPU_FLAG_IF, intf);
}
void Jcc(int Len)
{
	switch(Len) {
	case 1: evIP += d_nsbit8(eIMS);break;
	case 2: evIP += d_nsbit16(eIMS);break;
	case 4: evIP += d_nsbit32(eIMS);break;
	default: break;
	}
}
void JCC(char code, t_bool J)
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 0x00
void ADD_RM8_R8()
{
	ADD((void **)&rm8,(void **)&r8,1);
}
void ADD_RM16_R16()
{
	ADD((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void ADD_R8_RM8()
{
	ADD((void **)&r8,(void **)&rm8,1);
}
void ADD_R16_RM16()
{
	ADD((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void ADD_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	ADD((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void ADD_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	ADD((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void PUSH_ES()
{
	PUSH(&eCPU.es,tmpOpdSize);
}
void POP_ES()
{
	POP(&eCPU.es,tmpOpdSize);
}
void OR_RM8_R8()
{
	OR((void **)&rm8,(void **)&r8,1);
}
void OR_RM16_R16()
{
	OR((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void OR_R8_RM8()
{
	OR((void **)&r8,(void **)&rm8,1);
}
void OR_R16_RM16()
{
	OR((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void OR_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	OR((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void OR_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	OR((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void PUSH_CS()
{
	PUSH(&eCPU.cs,tmpOpdSize);
}
// 0x10
void ADC_RM8_R8()
{
	ADC((void **)&rm8,(void **)&r8,1);
}
void ADC_RM16_R16()
{
	ADC((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void ADC_R8_RM8()
{
	ADC((void **)&r8,(void **)&rm8,1);
}
void ADC_R16_RM16()
{
	ADC((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void ADC_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	ADC((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void ADC_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	ADC((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void PUSH_SS()
{
	PUSH(&eCPU.ss,tmpOpdSize);
}
void POP_SS()
{
	POP(&eCPU.ss,tmpOpdSize);
}
void SBB_RM8_R8()
{
	SBB((void **)&rm8,(void **)&r8,1);
}
void SBB_RM16_R16()
{
	SBB((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void SBB_R8_RM8()
{
	SBB((void **)&r8,(void **)&rm8,1);
}
void SBB_R16_RM16()
{
	SBB((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void SBB_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	SBB((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void SBB_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	SBB((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void PUSH_DS()
{
	PUSH(&eCPU.ds,tmpOpdSize);
}
void POP_DS()
{
	POP(&eCPU.ds,tmpOpdSize);
}
// 0x20
void AND_RM8_R8()
{
	AND((void **)&rm8,(void **)&r8,1);
}
void AND_RM16_R16()
{
	AND((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void AND_R8_RM8()
{
	AND((void **)&r8,(void **)&rm8,1);
}
void AND_R16_RM16()
{
	AND((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void AND_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	AND((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void AND_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	AND((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void ES()
{
// 	t_nubit16 tds=eCPU.ds;
// 	t_nubit16 tes=eCPU.es;
// 	eCPU.ds=eCPU.es;
// 
	tmpDs=eCPU.es;
	tmpSs=eCPU.es;
	ExecIns();
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
// 	if (eCPU.ds==tes)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		eCPU.ds=tds;
}
void DAA()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,eCPU.al
		push eCPU.flags
		popf
		DAA
		pushf
		pop eCPU.flags
		mov eCPU.al,al
	}
}
void SUB_RM8_R8()
{
	SUB((void **)&rm8,(void **)&r8,1);
}
void SUB_RM16_R16()
{
	SUB((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void SUB_R8_RM8()
{
	SUB((void **)&r8,(void **)&rm8,1);
}
void SUB_R16_RM16()
{
	SUB((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void SUB_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	SUB((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void SUB_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	SUB((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void CS()
{
// 	t_nubit16 tds=eCPU.ds;
// 	t_nubit16 tcs=eCPU.cs;
// 	eCPU.ds=eCPU.cs;
	tmpDs=eCPU.cs;
	tmpSs=eCPU.cs;
	ExecIns();
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
// 	if (eCPU.ds==tcs)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		eCPU.ds=tds;
}
void DAS()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,eCPU.al
		push eCPU.flags
		popf
		DAS
		pushf
		pop eCPU.flags
		mov eCPU.al,al
	}
}
// 0x30
void XOR_RM8_R8()
{
	XOR((void **)&rm8,(void **)&r8,1);
}
void XOR_RM16_R16()
{
	XOR((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void XOR_R8_RM8()
{
	XOR((void **)&r8,(void **)&rm8,1);
}
void XOR_R16_RM16()
{
	XOR((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void XOR_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	XOR((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void XOR_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	XOR((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void SS()
{
// 	t_nubit16 tds=eCPU.ds;
// 	t_nubit16 tss=eCPU.ss;
// 	eCPU.ds=eCPU.ss;
	tmpDs=eCPU.ss;
	tmpSs=eCPU.ss;
	ExecIns();
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
// 	if (eCPU.ds==tss)			//只有在下一条指令未改变DS的值的情况下，才能将DS值复原
// 		eCPU.ds=tds;
}
void AAA()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	__asm
	{
		mov al,eCPU.al
		push eCPU.flags
		popf
		AAA
		pushf
		pop eCPU.flags
		mov eCPU.al,al
	}
}
void CMP_RM8_R8()
{
	CMP((void **)&rm8,(void **)&r8,1);
}
void CMP_RM16_R16()
{
	CMP((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void CMP_R8_RM8()
{
	CMP((void **)&r8,(void **)&rm8,1);
}
void CMP_R16_RM16()
{
	CMP((void **)&r16,(void **)&rm16,tmpOpdSize);
}
void CMP_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	CMP((void **)&pa,(void **)&pb,1);
	evIP=tevIP+1;
}
void CMP_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	CMP((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void DS()
{
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ds;
	ExecIns();
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
}
void AAS()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	__asm
	{
		mov ax,eCPU.ax
		push eCPU.flags
		popf
		AAS
		pushf
		pop eCPU.flags
		mov eCPU.ax,ax
	}
}
// 0x40
void INC_AX()
{
	INC(&eCPU.ax,tmpOpdSize);
}
void INC_CX()
{
	INC(&eCPU.cx,tmpOpdSize);
}
void INC_DX()
{
	INC(&eCPU.dx,tmpOpdSize);
}
void INC_BX()
{
	INC(&eCPU.bx,tmpOpdSize);
}
void INC_SP()
{
	INC(&eCPU.sp,tmpOpdSize);
}
void INC_BP()
{
	INC(&eCPU.bp,tmpOpdSize);
}
void INC_SI()
{
	INC(&eCPU.si,tmpOpdSize);
}
void INC_DI()
{
	INC(&eCPU.di,tmpOpdSize);
}
void DEC_AX()
{
	DEC(&eCPU.ax,tmpOpdSize);
}
void DEC_CX()
{
	DEC(&eCPU.cx,tmpOpdSize);
}
void DEC_DX()
{
	DEC(&eCPU.dx,tmpOpdSize);
}
void DEC_BX()
{
	DEC(&eCPU.bx,tmpOpdSize);
}
void DEC_SP()
{
	DEC(&eCPU.sp,tmpOpdSize);
}
void DEC_BP()
{
	DEC(&eCPU.bp,tmpOpdSize);
}
void DEC_SI()
{
	DEC(&eCPU.si,tmpOpdSize);
}
void DEC_DI()
{
	DEC(&eCPU.di,tmpOpdSize);
}
// 0x50
void PUSH_AX()
{
	PUSH(&eCPU.ax,tmpOpdSize);
}
void PUSH_CX()
{
	PUSH(&eCPU.cx,tmpOpdSize);
}
void PUSH_DX()
{
	PUSH(&eCPU.dx,tmpOpdSize);
}
void PUSH_BX()
{
	PUSH(&eCPU.bx,tmpOpdSize);
}
void PUSH_SP()
{
	t_nubit16 tsp=eCPU.sp;
	PUSH(&tsp,tmpOpdSize);			//不能PUSH修改之后的值
}
void PUSH_BP()
{
	PUSH(&eCPU.bp,tmpOpdSize);
}
void PUSH_SI()
{
	PUSH(&eCPU.si,tmpOpdSize);
}
void PUSH_DI()
{
	PUSH(&eCPU.di,tmpOpdSize);
}
void POP_AX()
{
	POP(&eCPU.ax,tmpOpdSize);
}
void POP_CX()
{
	POP(&eCPU.cx,tmpOpdSize);
}
void POP_DX()
{
	POP(&eCPU.dx,tmpOpdSize);
}
void POP_BX()
{
	POP(&eCPU.bx,tmpOpdSize);
}
void POP_SP()
{
	POP(&eCPU.sp,tmpOpdSize);
	eCPU.sp-=tmpOpdSize;				//POP()里是先赋值后加的，所以这里要减回去
}
void POP_BP()
{
	POP(&eCPU.bp,tmpOpdSize);
}
void POP_SI()
{
	POP(&eCPU.si,tmpOpdSize);
}
void POP_DI()
{
	POP(&eCPU.di,tmpOpdSize);
}
// 0x60
void ARPL()
{
	toe16;
	if (((*rm16)&3)<((*r16)&3))
	{
		eCPU.eflags|=ZF;
		(*rm16)&=0xfc;
		(*rm16)|=(*r16)&3;
	}
	else
		eCPU.eflags&=~ZF;
}
void OpdSize()
{
	tmpOpdSize=6-tmpOpdSize;
	ExecIns();
	tmpOpdSize=6-tmpOpdSize;
}
void AddrSize()
{
	tmpAddrSize=6-tmpAddrSize;
	ExecIns();
	tmpAddrSize=6-tmpAddrSize;
}
void PUSH_I16()
{	
	PUSH((void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
// 0x70
void JO()
{
	JCC(0x70,eCPU.eflags & OF);
}
void JNO()
{
	JCC(0x71,!(eCPU.flags & OF));
}
void JC()
{
	JCC(0x72,eCPU.flags & CF);
}
void JNC()
{
	JCC(0x73,!(eCPU.flags & CF));
}
void JZ()
{
	JCC(0x74,eCPU.flags & ZF);
}
void JNZ()
{
	JCC(0x75,!(eCPU.flags & ZF));
}
void JBE()
{
	JCC(0x76,eCPU.flags & CF || eCPU.flags & ZF);
}
void JA()
{
	JCC(0x77,!(eCPU.flags & CF) && !(eCPU.flags & ZF));
}
void JS()
{
	JCC(0x78,eCPU.flags & SF);
}
void JNS()
{
	JCC(0x79,!(eCPU.flags & SF));
}
void JP()
{
	JCC(0x7A,eCPU.flags & PF);
}
void JNP()
{
	JCC(0x7B,!(eCPU.flags & PF));
}
void JL()
{
	JCC(0x7C,((eCPU.flags & SF) != (eCPU.flags & OF)));
}
void JNL()
{
	JCC(0x7D,((eCPU.flags & SF) == (eCPU.flags & OF)));
}
void JLE()
{
	JCC(0x7E,((eCPU.flags & SF) != (eCPU.flags & OF)) || (eCPU.flags & ZF));
}
void JG()
{
	JCC(0x7F,((eCPU.flags & SF) == (eCPU.flags & OF)) && !(eCPU.flags & ZF));
}
// 0x80
void INS_80()	//这里是以80开头的指令的集。
{
	char oce=d_nsbit8(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	switch(oce) {
	case 0:
		__asm push eCPU.flags
		__asm popf
		*rm8+=d_nsbit8(eIMS);
		break;
	case 1:
		__asm push eCPU.flags
		__asm popf
		*rm8|=d_nsbit8(eIMS);
		break;
	case 2:
		t81=*rm8;
		t82=d_nsbit8(eIMS);
		__asm push eCPU.flags
		__asm popf
		//*rm8+=(d_nsbit8(eIMS)+((eCPU.flags & CF)!=0));	//在VC6里，逻辑值“真”＝1
		__asm mov al,t81
		__asm adc al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 3:		
		t81=*rm8;
		t82=d_nsbit8(eIMS);
		__asm push eCPU.flags
		__asm popf
		//*rm8-=(d_nsbit8(eIMS)+((eCPU.flags & CF)!=0));			
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 4:		
		__asm push eCPU.flags
		__asm popf
		*rm8&=d_nsbit8(eIMS);
		break;
	case 5:		
		__asm push eCPU.flags
		__asm popf
		*rm8-=d_nsbit8(eIMS);
		break;
	case 6:
		__asm push eCPU.flags
		__asm popf
		*rm8^=d_nsbit8(eIMS);
		break;
	case 7:
		t81 = *rm8;
		t82 = d_nsbit8(eIMS);
		__asm push eCPU.flags
		__asm popf
		__asm mov al, t81
		__asm cmp al, t82
//		*rm8-=d_nsbit8(eIMS);
		__asm pushf
		__asm pop eCPU.flags
		evIP++;
		return;
	}
	__asm pushf
	__asm pop eCPU.flags
	//if (oce==7)
	//	*rm8+=d_nsbit8(eIMS);
	evIP++;
}
void INS_81()	//这里是以81开头的指令的集。
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
		ADD((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 1:
		OR((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 2:
		ADC((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 3:		
		SBB((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 4:		
		AND((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 5:		
		SUB((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 6:		
		XOR((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	case 7:		
		CMP((void **)&rm16,(void **)&teIMS,tmpOpdSize);
		break;
	}
	evIP+=tmpOpdSize;
}
void INS_82()	//这里是以82开头的指令的集。
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
		__asm push eCPU.flags
		__asm popf
		*rm8+=tfg;	
		break;
	case 1:
		__asm push eCPU.flags
		__asm popf
		*rm8|=tfg;
		break;
	case 2:
		t81=*rm8;
		t82=(t_nubit8)tfg;
		__asm push eCPU.flags
		__asm popf
		//*rm8+=(tfg+((eCPU.flags & CF)!=0));	//在VC6里，逻辑值“真”＝1
		__asm mov al,t81
		__asm adc al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 3:		
		t81=*rm8;
		t82=(t_nubit8)tfg;
		__asm push eCPU.flags
		__asm popf
		//*rm8-=(tfg+((eCPU.flags & CF)!=0));			
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 4:		
		__asm push eCPU.flags
		__asm popf
		*rm8&=tfg;			
		break;
	case 5:		
		__asm push eCPU.flags
		__asm popf
		*rm8-=tfg;			
		break;
	case 6:		
		__asm push eCPU.flags
		__asm popf
		*rm8^=tfg;			
		break;
	case 7:		
		__asm push eCPU.flags
		__asm popf
		*rm8-=tfg;					
		break;
	}
	__asm pushf
	__asm pop eCPU.flags
	if (oce==7)
		*rm8+=tfg;
	evIP++;
}
void INS_83()	//这里是以83开头的指令的集。
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
		ADD((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 1:
		OR((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 2:
		ADC((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 3:		
		SBB((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 4:		
		AND((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 5:		
		SUB((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 6:		
		XOR((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	case 7:		
		CMP((void **)&rm16,(void **)&ptfg,tmpOpdSize);
		break;
	}
	evIP++;
}
void TEST_RM8_M8()
{
	TEST((void **)&rm8,(void **)&r8,1);
}
void TEST_RM16_M16()
{
	TEST((void **)&rm16,(void **)&r16,tmpOpdSize);
}
void XCHG_R8_RM8()
{
	toe8;
	XCHG(r8,rm8,1);
}
void XCHG_R16_RM16()
{
	toe16;
	XCHG(r16,rm16,tmpOpdSize);
}
void MOV_RM8_R8()
{
	toe8;
	MOV(rm8,r8,1);
}
void MOV_RM16_R16()
{
	toe16;
	MOV(rm16,r16,tmpOpdSize);
}
void MOV_R8_RM8()
{
	toe8;
	MOV(r8,rm8,1);
}
void MOV_R16_RM16()
{
	toe16;
	MOV(r16,rm16,tmpOpdSize);
}
void MOV_RM_SEG()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*r16=*rm16;
	MOV(r16,rm16,2);
}
void LEA_R16_M16()
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
void MOV_SEG_RM()
{
	TranslateOpcExtSeg(1,(char **)&rm16,(char **)&r16);
	//*rm16=*r16;
	MOV(rm16,r16,2);
}
void POP_RM16()
{
	toe16;
 	POP((void *)rm16,tmpOpdSize);
}
// 0x90
void NOP()
{
	__asm nop
}
void XCHG_CX_AX()
{
	XCHG(&eCPU.cx,&eCPU.ax,tmpOpdSize);
}
void XCHG_DX_AX()
{
	XCHG(&eCPU.dx,&eCPU.ax,tmpOpdSize);
}
void XCHG_BX_AX()
{
	XCHG(&eCPU.bx,&eCPU.ax,tmpOpdSize);
}
void XCHG_SP_AX()
{
	XCHG(&eCPU.sp,&eCPU.ax,tmpOpdSize);
}
void XCHG_BP_AX()
{
	XCHG(&eCPU.bp,&eCPU.ax,tmpOpdSize);
}
void XCHG_SI_AX()
{
	XCHG(&eCPU.si,&eCPU.ax,tmpOpdSize);
}
void XCHG_DI_AX()
{
	XCHG(&eCPU.di,&eCPU.ax,tmpOpdSize);
}
void CBW()
{
	switch(tmpOpdSize)
	{
	case 2:		
		eCPU.ax=(char)eCPU.al;
		break;
	case 4:
		eCPU.eax=(short)eCPU.ax;
		break;
	}
}
void CWD()
{
	switch(tmpOpdSize)
	{
	case 2:		
		if (eCPU.ax & 0x8000)
			eCPU.dx=0xffff;
		else
			eCPU.dx=0;
		break;
	case 4:
		if (eCPU.eax & 0x80000000)
			eCPU.edx=0xffffffff;
		else
			eCPU.edx=0;
		break;
	}
}
void CALL_FAR()
{
	LongCallNewIP(4);			//这个指令后带4个字节的数据

	eCPU.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.cs;
	eCPU.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip=d_nubit16(eIMS);
	evIP+=2;
	eCPU.cs=d_nubit16(eIMS);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void WAIT()
{
	__asm nop
}
void PUSHF()
{
	PUSH(&eCPU.flags,tmpOpdSize);
}
void POPF()
{
	POP(&eCPU.flags,tmpOpdSize);
}
void SAHF()
{	
	*(t_nubit8 *)&eCPU.flags=eCPU.ah;
}
void LAHF()
{	
	eCPU.ah=*(t_nubit8 *)&eCPU.flags;
}
// 0xA0
void MOV_AL_M8()
{
	t81=GetM8_16(d_nubit16(eIMS));
	MOV(&eCPU.al,&t81,1);	
	evIP+=2;
}
void MOV_AX_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		t161=GetM16_16(d_nubit16(eIMS));
		MOV(&eCPU.ax,&t161,tmpOpdSize);
		break;
	case 4:
		t321=GetM32_16(d_nubit16(eIMS));
		MOV(&eCPU.eax,&t321,tmpOpdSize);
		break;
	}	
	evIP+=tmpAddrSize;
}
void MOV_M8_AL()
{
	SetM8(d_nubit16(eIMS),eCPU.al);
	evIP+=2;
}
void MOV_M16_AX()
{
	switch(tmpOpdSize)
	{
	case 2:
		SetM16(d_nubit16(eIMS),eCPU.ax);
		break;
	case 4:
		SetM32(d_nubit16(eIMS),eCPU.eax);
		break;
	}	
	evIP+=tmpAddrSize;
}
void MOVSB()
{	
	t_nubit8 tgm;
	if (tmpAddrSize==2)	
		tgm=GetM8_16(eCPU.si);			//因为t是全局变量！在GetM8里已经改变了t的值！！！
	else
		tgm=GetM8_32(eCPU.esi);
	(t=eCPU.es,t<<=4);
	*((t_nubit8 *)(MemoryStart+eCPU.di+t))=tgm;
//	if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
//		WriteVideoRam(eCPU.di+t-0xa0000);
	if (eCPU.flags & DF)	
	{
		SegOffDec(&(eCPU.ds),&(eCPU.si));
		SegOffDec(&(eCPU.es),&(eCPU.di));
	}
	else
	{
		SegOffInc(&(eCPU.ds),&(eCPU.si));
		SegOffInc(&(eCPU.es),&(eCPU.di));
	}
}
void MOVSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		//全局变量t不可以在一个赋值式的两边同时使用
		if (tmpAddrSize==2)
			*(t_nubit16 *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM16_16(eCPU.si);
		else
			*(t_nubit16 *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM16_32(eCPU.esi);
		break;
	case 4:
		//全局变量t不可以在一个赋值式的两边同时使用
		if (tmpAddrSize==2)
			*(t_nubit32 *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM32_16(eCPU.si);
		else
			*(t_nubit32 *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM32_32(eCPU.esi);
		break;
	}

/*	if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000)
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
		}
	}*/
	if (eCPU.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(eCPU.ds),&(eCPU.si));		
			SegOffDec(&(eCPU.es),&(eCPU.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(eCPU.ds),&(eCPU.si));
			SegOffInc(&(eCPU.es),&(eCPU.di));
		}
	}
}
void CMPSB()
{
	//这里的t居然和全局变量t重了！
	//Release的时候，这里居然被编译器优化掉了。
	t_nubit8 ta=GetM8_16(eCPU.si);
	t_nubit8 tb;
	if (tmpAddrSize==2)
		tb=*(t_nubit8 *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart);
	else
		tb=*(t_nubit8 *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart);
	__asm push eCPU.flags
	__asm popf
	//t_nubit8 ta=(t_nubit8)(GetM8(eCPU.si)-*(t_nubit8 *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart));
	__asm mov al,ta
	__asm cmp al,tb
	__asm pushf
	__asm pop eCPU.flags
	if (eCPU.flags & DF)	
	{
		SegOffDec(&(eCPU.ds),&(eCPU.si));
		SegOffDec(&(eCPU.es),&(eCPU.di));		//这里理解错了，si和di都应该要变的		
	}
	else
	{
		SegOffInc(&(eCPU.ds),&(eCPU.si));
		SegOffInc(&(eCPU.es),&(eCPU.di));		
	}
}
void CMPSW()
{
	int i;
	t_nubit32 tevIP=evIP;
	void *ta;
	void *tb;
	if (tmpAddrSize==2)
	{
		ta=(void *)(eCPU.si+((t=tmpDs,t<<4))+MemoryStart);
		tb=(void *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart);
	}
	else
	{
		ta=(void *)(eCPU.esi+((t=tmpDs,t<<4))+MemoryStart);
		tb=(void *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart);
	}
	CMP(&ta,&tb,tmpOpdSize);
	evIP=tevIP;
	if (eCPU.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(eCPU.ds),&(eCPU.si));
			SegOffDec(&(eCPU.es),&(eCPU.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(eCPU.ds),&(eCPU.si));
			SegOffInc(&(eCPU.es),&(eCPU.di));
		}
	}
}
void TEST_AL_I8()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)eIMS;
	TEST((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}
void TEST_AX_I16()
{
	t_nubit32 tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)eIMS;
	TEST((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}
void STOSB()
{
	if (tmpAddrSize==2)
		*(t_nubit8 *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.al;
	else
		*(t_nubit8 *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.al;
	if (eCPU.flags & DF)	
	{
		SegOffDec(&(eCPU.es),&(eCPU.di));
	}
	else
	{
		SegOffInc(&(eCPU.es),&(eCPU.di));
	}
}
void STOSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		if (tmpAddrSize==2)
			*(t_nubit16 *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.ax;
		else
			*(t_nubit16 *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.ax;
		break;
	case 4:
		if (tmpAddrSize==2)
			*(t_nubit32 *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.eax;
		else
			*(t_nubit32 *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.eax;
		break;
	}
	if (eCPU.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)		
			SegOffDec(&(eCPU.es),&(eCPU.di));				
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)		
			SegOffInc(&(eCPU.es),&(eCPU.di));		
	}
}
void LODSB()
{
	if (tmpAddrSize==2)
		eCPU.al=GetM8_16(eCPU.si);
	else
		eCPU.al=GetM8_32(eCPU.esi);
	if (eCPU.flags & DF)	
	{
		SegOffDec(&(eCPU.ds),&(eCPU.si));
	}
	else
	{
		SegOffInc(&(eCPU.ds),&(eCPU.si));
	}
}
void LODSW()
{
	int i;
	switch(tmpOpdSize)
	{
	case 2:
		if (tmpAddrSize==2)
			eCPU.ax=GetM16_16(eCPU.si);
		else
			eCPU.ax=GetM16_32(eCPU.esi);
		break;
	case 4:
		if (tmpAddrSize==2)
			eCPU.eax=GetM32_16(eCPU.si);
		else
			eCPU.eax=GetM32_32(eCPU.esi);
		break;
	}	

	if (eCPU.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
			SegOffDec(&(eCPU.ds),&(eCPU.si));		
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
			SegOffInc(&(eCPU.ds),&(eCPU.si));		
	}
}
void SCASB()
{
	t_nubit8 ta=eCPU.al;
	t_nubit8 tb;
	if (tmpAddrSize==2)
		tb=d_nubit8(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart);
	else
		tb=d_nubit8(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart);
	__asm push eCPU.flags
	__asm popf
	//t_nubit8 ta=(t_nubit8)(eCPU.al-*(t_nubit8 *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart));
	__asm mov al,ta
	__asm cmp al,tb
	__asm pushf
	__asm pop eCPU.flags	
	if (eCPU.flags & DF)	
	{
		SegOffDec(&(eCPU.es),&(eCPU.di));
	}
	else
	{
		SegOffInc(&(eCPU.es),&(eCPU.di));
	}
}
void SCASW()
{
	int i;
	t_nubit32 tevIP=evIP;
	void *ta=(void *)&eCPU.ax;
	void *tb;
	if (tmpAddrSize==2)
		tb=(void *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart);
	else
		tb=(void *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart);
	CMP(&ta,&tb,tmpOpdSize);
	evIP=tevIP;
	if (eCPU.flags & DF)	
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffDec(&(eCPU.es),&(eCPU.di));
		}
	}
	else
	{
		for (i=0;i<tmpOpdSize;i++)
		{
			SegOffInc(&(eCPU.es),&(eCPU.di));
		}
	}

}
// 0xB0
void MOV_AL_I8()
{
	MOV(&eCPU.al,(void *)eIMS,1);
	evIP++;
}
void MOV_CL_I8()
{
	MOV(&eCPU.cl,(void *)eIMS,1);
	evIP++;
}
void MOV_DL_I8()
{
	MOV(&eCPU.dl,(void *)eIMS,1);
	evIP++;
}
void MOV_BL_I8()
{
	MOV(&eCPU.bl,(void *)eIMS,1);
	evIP++;
}
void MOV_AH_I8()
{
	MOV(&eCPU.ah,(void *)eIMS,1);
	evIP++;
}
void MOV_CH_I8()
{
	MOV(&eCPU.ch,(void *)eIMS,1);
	evIP++;
}
void MOV_DH_I8()
{
	MOV(&eCPU.dh,(void *)eIMS,1);
	evIP++;
}
void MOV_BH_I8()
{
	MOV(&eCPU.bh,(void *)eIMS,1);
	evIP++;
}
void MOV_AX_I16()
{
	MOV(&eCPU.ax,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_CX_I16()
{
	MOV(&eCPU.cx,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_DX_I16()
{
	MOV(&eCPU.dx,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_BX_I16()
{
	MOV(&eCPU.bx,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_SP_I16()
{
	MOV(&eCPU.sp,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_BP_I16()
{
	MOV(&eCPU.bp,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_SI_I16()
{
	MOV(&eCPU.si,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void MOV_DI_I16()
{
	MOV(&eCPU.di,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
// 0xC0
void INS_C0()
{
	t_nubit8 t,teIMS;
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	teIMS=d_nubit8(eIMS);
	__asm push teIMS
	__asm push eCPU.flags;
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
	__asm pop eCPU.flags
	*rm8=t;
	evIP++;
}
void INS_C1()
{
	t_nubit16 t;
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
		__asm push teIMS
		__asm push eCPU.eflags;
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
		__asm pop eCPU.eflags
		*rm16=t;
		break;
	case 4:	
		toe32;
		teIMS=*(t_nubit8 *)eIMS;
		t2=*rm32;
		__asm push teIMS
		__asm push eCPU.eflags;
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
		__asm pop eCPU.eflags
		*rm32=t2;
		break;
	}
	evIP++;
}
void SHL_RM8_I8()
{
	t_nubit8 teIMS;
	toe8;	
	teIMS=*(t_nubit8 *)eIMS;
	SHL(rm8,teIMS,1);
	evIP++;
}
void SHL_RM16_I8()
{
	t_nubit8 teIMS;
	toe16;
	teIMS=d_nubit8(eIMS);
	SHL(rm16,teIMS,tmpOpdSize);
	evIP++;
}
void RET_I8()
{
	eCPU.ip=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.sp+=*(t_nubit16*)eIMS;
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void RET_NEAR()
{
	eCPU.ip=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void LES_R16_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		*r16=*rm16;
		eCPU.es=*(t_nubit16 *)(rm16+1);
		break;
	case 4:
		toe32;
		*r32=*rm32;
		eCPU.es=*(t_nubit16 *)(rm32+1);
		break;
	}
}
void LDS_R16_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		toe16;	
		*r16=*rm16;
		eCPU.ds=*(t_nubit16 *)(rm16+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	case 4:
		toe32;	
		*r32=*rm32;
		eCPU.ds=*(t_nubit16 *)(rm32+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	}
}
void MOV_M8_I8()
{
	toe8;	
	MOV(rm8,(void *)eIMS,1);
	evIP++;
}
void MOV_M16_I16()
{
	toe16;	
	MOV(rm16,(void *)eIMS,tmpOpdSize);
	evIP+=tmpOpdSize;
}
void RET_I16()
{
	eCPU.ip=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.cs=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.sp+=*(t_nubit16*)eIMS;
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void RET_FAR()
{
	eCPU.ip=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.cs=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void INT3()
{
	GlobINT|=0x8;
}
void INT_I8()
{
	LongCallNewIP(1);
	eCPU.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.flags;
	eCPU.flags&=~IF;
	eCPU.flags&=~TF;
	eCPU.flags&=~AF;
	eCPU.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	eCPU.sp-=2;
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip=*(t_nubit16 *)((*(t_nubit8 *)eIMS)*4+MemoryStart);	
	eCPU.cs=*(t_nubit16 *)((*(t_nubit8 *)eIMS)*4+2+MemoryStart);
	
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void INTO()
{
	if (eCPU.flags&OF)
	{
		GlobINT|=0x10;
	}
// 	eCPU.sp-=2;
// 	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.cs;
// 	eCPU.sp-=2;
// 	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
// 	eCPU.ip=(4)*4+MemoryStart;
// 	evIP+=2;
// 	eCPU.cs=0;
// 	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}
void IRET()					//在实模式下，iret和ret far是一样的，这里可以直接调用RET_FAR()的，不过为了以后扩展着想就不这样做。
{
	eCPU.ip=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.cs=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.flags=*(t_nubit16*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}
// 0xD0
void INS_D0()
{
	t_nubit8 t;
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm push eCPU.flags;
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
	__asm pop eCPU.flags
	*rm8=t;
}
void INS_D1()
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
		__asm push eCPU.eflags;
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
		__asm pop eCPU.eflags
		*rm16=t;
		break;
	case 4:	
		toe32;
		t2=*rm32;
		__asm push eCPU.eflags;
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
		__asm pop eCPU.eflags
		*rm32=t2;
		break;
	}
}
void INS_D2()
{
	t_nubit8 t;
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	t=*rm8;
	__asm mov cl,eCPU.cl;
	__asm push eCPU.flags;
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
	__asm pop eCPU.flags
	*rm8=t;
}
void INS_D3()
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
			__asm mov cl,eCPU.cl;				//switch/case有可能要用到cl，所以不能把这两行提取出去
			__asm push eCPU.flags;
			__asm popf
			__asm rol t,cl
			break;
		case 1:
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm ror t,cl
			break;
		case 2:
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm rcl t,cl
			break;
		case 3:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm rcr t,cl
			break;
		case 4:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm shl t,cl
			break;
		case 5:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm shr t,cl
			break;
		case 6:		
			OpcError();
			break;
		case 7:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.flags;
			__asm popf
			__asm sar t,cl
			break;
		}
		__asm pushf
		__asm pop eCPU.flags
		*rm16=t;
		break;	
	case 4:
		toe32;
		t2=*rm32;
		switch(oce)
		{
		case 0:	
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm rol t2,cl
			break;
		case 1:
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm ror t2,cl
			break;
		case 2:
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm rcl t2,cl
			break;
		case 3:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm rcr t2,cl
			break;
		case 4:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm shl t2,cl
			break;
		case 5:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm shr t2,cl
			break;
		case 6:		
			OpcError();
			break;
		case 7:		
			__asm mov cl,eCPU.cl;
			__asm push eCPU.eflags;
			__asm popfd
			__asm sar t2,cl
			break;
		}
		__asm pushfd
		__asm pop eCPU.eflags
		*rm32=t2;
		break;
	}
}

// 只实现了以10为基数的情况，同AAD
void AAM()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	if ((*(t_nubit8 *)eIMS)==0)
		GlobINT|=IntDE;
	else
	__asm
	{
		mov ax,eCPU.ax
		push eCPU.flags
		popf
		aam
		pushf
		pop eCPU.flags
		mov eCPU.ax,ax
	}
	evIP++;			//aam的编码是D4 0A
}

// 下面引自Intel开发者手册。这里的aad只实现了以10为基数的情况。即只实现了 D5 0A
// The generalized version of this instruction allows adjustment of two unpacked digits of any
// number base (refer to the "Operation" section below), by setting the imm8 byte to the selected
// number base (for example, 08H for octal, 0AH for decimal, or 0CH for base 12 numbers).
void AAD()
{
	t_bool intf = GetBit(eCPU.flags, VCPU_FLAG_IF);
	__asm
	{
		mov ax,eCPU.ax
		push eCPU.flags
		popf
		aad
		pushf
		pop eCPU.flags
		mov eCPU.ax,ax
	}
	evIP++;			//aad的编码是D5 0A
}
void ESC_9()
{
	__asm nop;
}
void XLAT()
{
	eCPU.al=GetM8_16((t_nubit16)(eCPU.bx+eCPU.al));
}
void ESC_0()
{
	GlobINT|=0x80;
}
void ESC_1()
{	
	GlobINT|=0x80;
	toe16;				//这里纯粹是为了修改evIP
}
void ESC_2()
{
	GlobINT|=0x80;
}
void ESC_3()
{	
	GlobINT|=0x80;
	evIP++;
}
void ESC_4()
{
	GlobINT|=0x80;
}
void ESC_5()
{
	GlobINT|=0x80;
}
void ESC_6()
{
	GlobINT|=0x80;
}
void ESC_7()
{
	GlobINT|=0x80;
}
// 0xE0
void LOOPNE()
{
	eCPU.cx--;
	if (eCPU.cx && !(eCPU.flags &ZF))
		JMP_NEAR();
	else
		evIP++;
}
void LOOPE()
{
	eCPU.cx--;
	if (eCPU.cx && (eCPU.flags &ZF))
		JMP_NEAR();
	else
		evIP++;
}
void LOOP_NEAR()
{
	eCPU.cx--;
	if (eCPU.cx)
		JMP_NEAR();
	else
		evIP++;
}
void JCXZ_NEAR()
{	
	if (!eCPU.cx)
		JMP_NEAR();
	else
		evIP++;
}
void IN_AL_N()
{
	ExecFun(InTable[d_nubit8(eIMS)]);
	eCPU.al = vport.iobyte;
	evIP++;
}
void IN_AX_N()
{
	// in ax,n 或者 in eax,n 是连续地读端口。InTable里的函数都是直接修改eCPU.al的，最好把每个eCPU.al都保存起来，再一次过赋给eCPU.ax或eCPU.eax
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		ExecFun(InTable[d_nubit8(eIMS)+i]);
		d_nubit8(&eCPU.al+i)=vport.iobyte;
	}
	evIP++;
}
void OUT_N_AL()
{
	vport.iobyte = eCPU.al;
	ExecFun(OutTable[d_nubit8(eIMS)]);
	evIP++;
}
void OUT_N_AX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		vport.iobyte = d_nubit8(&eCPU.ax+i);
		ExecFun(OutTable[d_nubit8(eIMS)+i]);
	}
	evIP++;
}
void CALL_NEAR()
{
	LongCallNewIP(2);

	eCPU.sp-=2;										//段内CALL，CS不压栈
	*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip+=(d_nubit16(eIMS));
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void JMP_NEAR_LABEL()	//立即数是两字节的
{
	LongCallNewIP(2);
	eCPU.ip+=(d_nubit16(eIMS));
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}
void JMP_FAR_LABEL()
{
	LongCallNewIP(4);
	eCPU.ip=d_nubit16(eIMS);
	evIP+=2;
	eCPU.cs=d_nubit16(eIMS);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}
void JMP_NEAR()			//立即数是一字节的
{	
	LongCallNewIP(1);
	eCPU.ip+=(d_nsbit8(eIMS));
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}
void IN_AL_DX()
{
	ExecFun(InTable[eCPU.dx]);
	eCPU.al = vport.iobyte;
}
void IN_AX_DX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		ExecFun(InTable[eCPU.dx+i]);
		d_nubit8(&eCPU.al+i)=vport.iobyte;
	}
}
void OUT_DX_AL()
{
	vport.iobyte = eCPU.al;
	ExecFun(OutTable[eCPU.dx]);
}
void OUT_DX_AX()
{
	int i;
	for (i=0;i<tmpOpdSize;i++) {
		vport.iobyte = d_nubit8(&eCPU.al+i);
		ExecFun(OutTable[eCPU.dx+i]);
	}
}

// 0xF0
void LOCK()
{
	__asm nop;
}
void REPNE()
{
	t_nubit32 tevIP=evIP;
	t_nubit8 nopc;
	if (eCPU.cx==0)
	{
		nopc=d_nubit8(MemoryStart+evIP);
		if ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是CS、ES之类的指令，则再跳一字节
		{
			evIP++;
			nopc=d_nubit8(MemoryStart+evIP);
		}
		evIP++;
	}
	eCPU.flags &= (0xffff ^ ZF);		//ZF位置0
	while (eCPU.cx>0 && !(eCPU.flags & ZF))
	{
		evIP=tevIP;
		ExecIns();
		eCPU.cx--;
	}
	//evIP++;			//串操作指令都是一字节长的
}
void REP()
{
	t_nubit32 tevIP=evIP;
	t_nubit8 nopc;
	eCPU.flags |= ZF;		//ZF位置1
	if (eCPU.cx==0)
	{
		nopc=d_nubit8(MemoryStart+evIP);		
		while ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是指令前缀，则再跳一字节
		{
			evIP++;
			nopc=d_nubit8(MemoryStart+evIP);
		}
		evIP++;						//MOVSB之类的指令只有1字节长		
	}
	while (eCPU.cx>0 && (eCPU.flags & ZF))
	{
		evIP=tevIP;
		ExecIns();
		eCPU.cx--;
	}
	//evIP++;			//串操作指令都是一字节长的
}
void HLT()
{
	while (0)	//这里要检查是否有外部中断
	{
		__asm nop
	}
}
void CMC()
{
	eCPU.flags^=CF;
}
void INS_F6()
{
	char oce,trm8,tc;
	oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	trm8=*rm8;
	tc=d_nsbit8(eIMS);
	switch(oce) {
	case 0:
		__asm push eCPU.flags
		__asm mov al,tc
		__asm popf
		__asm test trm8,al
		__asm pushf
		__asm pop eCPU.flags
		evIP++;
		break;
	case 1:
		OpcError();
		break;
	case 2:
		__asm push eCPU.flags
		__asm popf
		__asm not trm8
		__asm pushf
		__asm pop eCPU.flags
		*rm8=trm8;
		break;
	case 3:
		__asm push eCPU.flags
		__asm popf
		__asm neg trm8
		__asm pushf
		__asm pop eCPU.flags
		*rm8=trm8;
		break;
	case 4:
		__asm push eCPU.flags
		__asm mov al,eCPU.al
		__asm popf
		__asm mul trm8
		__asm mov eCPU.ax,ax
		__asm pushf
		__asm pop eCPU.flags
		break;
	case 5:
		__asm push eCPU.flags
		__asm mov al,eCPU.al
		__asm popf
		__asm imul trm8
		__asm mov eCPU.ax,ax
		__asm pushf
		__asm pop eCPU.flags
		break;
	case 6:
		if (trm8!=0) {
			__asm push eCPU.flags
			__asm mov ax,eCPU.ax
			__asm popf
			__asm div trm8
			__asm mov eCPU.ax,ax
			__asm pushf
			__asm pop eCPU.flags
		} else GlobINT|=1;
		break;
	case 7:
		if (trm8!=0) {
			__asm push eCPU.flags
			__asm mov ax,eCPU.ax
			__asm popf
			__asm idiv trm8	
			__asm mov eCPU.ax,ax
			__asm pushf
			__asm pop eCPU.flags
		} else GlobINT|=1;
		break;
	}
}
void INS_F7()
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
		__asm push eCPU.flags
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
			__asm mov ax,eCPU.ax
			__asm popf
			__asm mul trm16	
			__asm mov eCPU.ax,ax
			__asm mov eCPU.dx,dx
			break;
		case 5:		
			__asm mov ax,eCPU.ax
			__asm popf
			__asm imul trm16	
			__asm mov eCPU.ax,ax
			__asm mov eCPU.dx,dx
			break;
		case 6:		
			if (trm16!=0)			//这里只针对8086的
			{		
				__asm mov ax,eCPU.ax
				__asm mov dx,eCPU.dx
				__asm popf
				__asm div trm16	
				__asm mov eCPU.ax,ax
				__asm mov eCPU.dx,dx
			}
			else
			{
				GlobINT|=1;
			}
			break;
		case 7:		
			if (trm16!=0)
			{		
				__asm mov ax,eCPU.ax
				__asm mov dx,eCPU.dx
				__asm popf
				__asm idiv trm16	
				__asm mov eCPU.ax,ax
				__asm mov eCPU.dx,dx
			}
			else
			{
				GlobINT|=1;
			}
			break;
		}
		__asm pushf				//这里一定要尽快把Flags搞出来，以免运算完后被修改
		__asm pop eCPU.flags
		//*rm16=trm16;				//这里不能这样写，因为有时候那个rm16正好就是div的结果，一改回原来的trm16，结果就被覆盖了
		//evIP++;
		if (oce==0)
			evIP+=tmpOpdSize;
		break;
	case 4:
		toe32;
		trm32=*rm32;
		tc2=*(t_nubit32 *)eIMS;
		__asm push eCPU.eflags
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
			__asm mov eax,eCPU.eax
			__asm popfd
			__asm mul trm32	
			__asm mov eCPU.eax,eax
			__asm mov eCPU.edx,edx
			break;
		case 5:		
			__asm mov eax,eCPU.eax
			__asm popfd
			__asm imul trm32	
			__asm mov eCPU.eax,eax
			__asm mov eCPU.edx,edx
			break;
		case 6:		
			if (trm32!=0)			//这里只针对8086的
			{		
				__asm mov eax,eCPU.eax
				__asm mov edx,eCPU.edx
				__asm popfd
				__asm div trm32	
				__asm mov eCPU.eax,eax
				__asm mov eCPU.edx,edx
			}
			else
			{
				GlobINT|=1;
			}
			break;
		case 7:		
			if (trm32!=0)
			{		
				__asm mov eax,eCPU.eax
				__asm mov edx,eCPU.edx
				__asm popfd
				__asm idiv trm32	
				__asm mov eCPU.eax,eax
				__asm mov eCPU.edx,edx
			}
			else
			{
				GlobINT|=1;
			}
			break;
		}
		__asm pushfd				//这里一定要尽快把Flags搞出来，以免运算完后被修改
		__asm pop eCPU.eflags
		if (oce==0)
			evIP+=tmpOpdSize;
		break;
	}
}
void CLC()
{
	eCPU.flags &= ~CF;
}
void STC()
{
	eCPU.flags |= CF;
}
void CLI()
{
	eCPU.flags &= ~IF;
}
void STI()
{
	eCPU.flags |= IF;
}
void CLD()
{
	eCPU.flags &= ~DF;
}
void STD()
{
	eCPU.flags |= DF;
}
void INS_FE()
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
	//__asm push eCPU.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
	switch(oce)
	{
	case 0:		
		__asm push eCPU.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
		__asm popf
		//(*rm8)++;	
		__asm inc trm8				//++会被编译成add，和inc所修改的标志位不一样
		__asm pushf
		__asm pop eCPU.flags
		*rm8=trm8;
		break;
	case 1:
		__asm push eCPU.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
		__asm popf
		//(*rm8)--;
		__asm dec trm8
		__asm pushf
		__asm pop eCPU.flags
		*rm8=trm8;
		break;
	case 2:
		//__asm popf
		eCPU.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
		JMP_NEAR_LABEL();		
		break;
	case 3:		
		//__asm popf
		eCPU.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
		eCPU.ip=*rm8;
		eCPU.cs=*(rm8+1);
		evIP+=2;		
		evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
		break;
	case 4:		
		//__asm popf
		evIP+=(*rm8+3);
		break;
	case 5:		
		//__asm popf
		eCPU.ip=*rm8;
		evIP+=2;	
		eCPU.cs=*(rm8+1);
		evIP+=2;
		evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
		break;
	case 6:		
		//__asm popf
		eCPU.sp-=2;
		*(t_nubit16*)( EvSP +MemoryStart)=(t_nubit16)*rm8;
		break;
	case 7:		
		OpcError();					
		break;
	}
	//evIP++;
}
void INS_FF()
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
			__asm push eCPU.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popf
			//(*rm16)++;	
			__asm inc trm16
			__asm pushf
			__asm pop eCPU.flags
			*rm16=trm16;
			break;
		case 4:
			__asm push eCPU.eflags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popfd
			//(*rm16)++;	
			__asm inc trm32
			__asm pushfd
			__asm pop eCPU.eflags
			*rm32=trm32;
			break;		
		}
		break;		
	case 1:
		switch (tmpOpdSize)
		{
		case 2:
			__asm push eCPU.flags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popf
			//(*rm16)--;
			__asm dec trm16
			__asm pushf
			__asm pop eCPU.flags
			*rm16=trm16;
			break;
		case 4:
			__asm push eCPU.eflags			//下面并不是每条指令都修改标志位，所以先把原来的保存起来。
			__asm popfd
			//(*rm16)--;
			__asm dec trm32
			__asm pushfd
			__asm pop eCPU.eflags
			*rm32=trm32;
			break;
		}
		break;
	case 2:		
		switch (tmpOpdSize)
		{
		case 2:
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm16;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		case 4:
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm32;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		}
		break;
	case 3:		
		switch (tmpOpdSize)
		{
		case 2:
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.cs;
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm16;
			eCPU.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		case 4:
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.cs;
			eCPU.sp-=2;
			*(t_nubit16*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm32;
			eCPU.cs=*(rm32+1);
			evIP+=tmpOpdSize;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		}
		break;
	case 4:
		switch (tmpOpdSize)
		{
		case 2:
			eCPU.ip=(*rm16);
			evIP=(t=eCPU.cs,t<<=4)+eCPU.ip;
			break;
		case 4:
			eCPU.ip=(*rm32);
			evIP=(t=eCPU.cs,t<<=4)+eCPU.ip;
			break;
		}
		break;
	case 5:		
		switch (tmpOpdSize)
		{
		case 2:
			eCPU.ip=*rm16;
			evIP+=2;		
			eCPU.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
			break;
		case 4:
			eCPU.ip=*rm32;
			evIP+=tmpOpdSize;		
			eCPU.cs=*(rm32+1);
			evIP+=tmpOpdSize;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
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
void INS_0F()
{
	t_nubit8 OpC=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit32 tcs,InsFucAddr;
	evIP++;
	InsFucAddr=Ins0FTable[OpC];
	__asm call InsFucAddr;
	tcs=eCPU.cs;
	eCPU.ip=(evIP - (tcs << 4)) % 0x10000;
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
}

//////////////////////////////////////////////////////////////////////////
// 下面这部分是0F开头的指令
void ADDPS()
{
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags;
	__asm popf
	for (i=0;i<4;i++)
	{
		eCPU.xmm[a].fp[i]+=eCPU.xmm[b].fp[i];
	}
	__asm pushf
	__asm pop eCPU.eflags;
	evIP++;
}
void ADDSS()
{
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit8 b=a&0x7;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags;
	__asm popf
	eCPU.xmm[a].fp[3]+=eCPU.xmm[b].fp[3];
	__asm pushf
	__asm pop eCPU.eflags;
	evIP++;
}
void ANDNPS()
{
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags
	__asm popf
	for (i=0;i<4;i++)
	{
		eCPU.xmm[a].ip[i]=~eCPU.xmm[a].ip[i];
		eCPU.xmm[a].ip[i]&=eCPU.xmm[b].ip[i];
	}
	__asm pushf
	__asm pop eCPU.eflags
	evIP++;	
}
void ANDPS()
{
	t_nubit8 a=*(t_nubit8 *)(evIP+MemoryStart);
	t_nubit8 b=a&0x7;
	int i;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags;
	__asm popf
	for (i=0;i<4;i++)
	{
		eCPU.xmm[a].ip[i]&=eCPU.xmm[b].ip[i];
	}
	__asm pushf
	__asm pop eCPU.eflags;
	evIP++;	
}
void BSF()
{
	int temp;
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		if (*rm16==0)
		{
			eCPU.eflags|=ZF;
		}
		else
		{
			eCPU.eflags&=~ZF;
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
			eCPU.eflags|=ZF;
		}
		else
		{
			eCPU.eflags&=~ZF;
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
void BSR()
{
	int temp;
	switch(tmpOpdSize)
	{
	case 2:
		toe16;
		if (*rm16==0)
		{
			eCPU.eflags|=ZF;
		}
		else
		{
			eCPU.eflags&=~ZF;
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
			eCPU.eflags|=ZF;
		}
		else
		{
			eCPU.eflags&=~ZF;
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
void BSWAP()
{
	t_nubit32 temp;
	t_nubit32 *desc;
	desc=(t_nubit32 *)FindRegAddr(1,*(t_nubit8 *)(eIMS-1));
	temp=*desc;
	*desc=((temp&0xff)<<24)+((temp&0xff00)<<8)+((temp&0xff0000)>>8)+((temp&0xff000000)>>24);
}
void CPUID()
{
	// do nothing
	__asm nop
}
void FINIT()
{
	eCPU.FpuCR=0x037f;
	eCPU.FpuSR=0;
	eCPU.FpuTR=0xffff;
	eCPU.Fpudp.seg=0;
	eCPU.Fpudp.off=0;
	eCPU.Fpuip.seg=0;
	eCPU.Fpuip.off=0;
	eCPU.FpuOpcode=0;
}
void INS_D9()
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
			*rm16=eCPU.FpuSR;
			break;
		default:
			OpcError();
			break;
		}
	}
}
void INS_DB()
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
void MOVZX_RM8()
{
	toe8;
	t321=*rm8;
	MOV(r16,&t321,tmpOpdSize);	
}
void MOVZX_RM16()
{
	toe16;
	t321=*rm16;
	MOV(r16,&t321,tmpOpdSize);	
}
void POP_FS()
{
	POP(&eCPU.fs,2);
}
void INS_0F01()
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
		*rm16=(t_nubit16)eCPU.CR[0];
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
void QDX()
{
	qdbiosExecInt(d_nubit8(eIMS));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_ZF, GetBit(_flags, VCPU_FLAG_ZF));
	MakeBit(vramVarWord(_ss,_sp + 4), VCPU_FLAG_CF, GetBit(_flags, VCPU_FLAG_CF));
	evIP++;
}

// yinX：	用这么笨的方法是为了让以后修改某条指令的时候更灵活
// 路人甲：	真的会更灵活吗？
// yinX：	…… -_-!
void SetupInstructionTable()
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
	InsTable[0xD8]=(t_faddrcc)QDX;
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
	InsTable[0xF1]=(t_faddrcc)OpcError;
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

#endif