//////////////////////////////////////////////////////////////////////////
// 名称：Instruction.cpp
// 功能：模拟8086指令集
// 日期：2008年4月20日
// 作者：梁一信
//////////////////////////////////////////////////////////////////////////

#include "stdio.h"

#include "Instruction.h"
#include "cpu.h"

#define eIMS (evIP+MemoryStart)
#define EvSP (t=eCPU.ss,(t<<4)+eCPU.sp)
#define evSI (t=tmpDs,(t<<4)+eCPU.si)
#define evDI (t=eCPU.es,(t<<4)+eCPU.di)
#define toe8 (TranslateOpcExt(0,(char **)&r8,(char **)&rm8))
#define toe16 (TranslateOpcExt(1,(char **)&r16,(char **)&rm16))
#define toe32 (TranslateOpcExt(1,(char **)&r32,(char **)&rm32))

const unsigned short Glbffff=0xffff;		//当寻址超过0xfffff的时候，返回的是一个可以令程序Reset的地址
unsigned short GlbZero=0x0;			//有些寻址用到两个偏移寄存器；有些寻址只用到一个偏移寄存器，另外一个指向这里。

extern unsigned int evIP;			//evIP永远指向将要读的那个字节

unsigned int InsTable[0x100];
unsigned int Ins0FTable[0x100];

unsigned short *rm16,*r16;			//解释寻址字节的时候用
unsigned int *rm32,*r32;
unsigned char *rm8,*r8;				//
unsigned short t161,t162;			//可以随便使用的
unsigned int t321,t322;				//
unsigned char t81,t82;				//

unsigned int t,t2,t3;				//

unsigned short tmpDs;				//这是存储器寻址时的段寄存器，绝大多数情况下等于DS，遇到CS、ES、SS指令时在一条指令的时间下作出修改
unsigned short tmpSs;				//

unsigned short tmpOpdSize=2;			//Operand Size，由描述符里的D位和OpdSize前缀共同决定，初始值为2
unsigned short tmpAddrSize=2;			//Address Size，由描述符里的D位和AddrSize前缀共同决定，初始值为2


//在执行每一条指令之前，evIP要等于cs:ip。
//执行完一条指令之后，再按evIP修改cs:ip。


void LongCallNewIP(char OffsetByte)
{
	unsigned int tcs=eCPU.cs;
	unsigned int tevIP=evIP;
	tcs<<=4;
	tevIP+=OffsetByte;
	tevIP-=tcs;
	eCPU.cs+=tevIP/0x10000;
	eCPU.ip=tevIP%0x10000;
}

void SegOffInc(unsigned short *seg, unsigned short *off)
{
		(*off)++;
}

void SegOffDec(unsigned short *seg, unsigned short *off)
{
		(*off)--;
}

unsigned char GetM8(unsigned short off)
{
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(unsigned char *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xff;
}

unsigned char GetM8(unsigned int off)
{
	return *(unsigned char *)(off+(t=tmpDs,t<<4)+MemoryStart);
}

unsigned short GetM16(unsigned short off)
{
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(unsigned short *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xffff;
}

unsigned short GetM16(unsigned int off)
{	
	return *(unsigned short *)(off+(t=tmpDs,t<<4)+MemoryStart);
}

unsigned int GetM32(unsigned short off)
{
	if (off+(t=tmpDs,t<<4)<=0xfffff)
		return *(unsigned int *)(off+(t=tmpDs,t<<4)+MemoryStart);
	else
		return 0xffffffff;
}

unsigned int GetM32(unsigned int off)
{
	return *(unsigned int *)(off+(t=tmpDs,t<<4)+MemoryStart);
}


void SetM8(unsigned short off, unsigned char val)
{
	if (off+(t=tmpDs,t<<4))
		*(unsigned char *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;

}

void SetM16(unsigned short off, unsigned short val)
{
	if (off+(t=tmpDs,t<<4))
		*(unsigned short *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
}

void SetM32(unsigned short off, unsigned int val)
{
	if (off+(t=tmpDs,t<<4))
		*(unsigned int *)(off+(t=tmpDs,t<<4)+MemoryStart)=val;
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

unsigned int FindRemAddr(char rem , unsigned short **off1, unsigned short **off2)
{
	unsigned int ret;
	unsigned int tds=tmpDs;	
	unsigned int tes=eCPU.es;
	unsigned int tss=tmpSs;
	tds<<=4;
	tes<<=4;
	tss<<=4;
	if (tmpAddrSize==2)
	{
		switch(rem)
		{
		case 0:
			*off1=&eCPU.bx;
			*off2=&eCPU.si;
			ret=MemoryStart+(unsigned short)(eCPU.bx+eCPU.si)+(tds);
			break;
		case 1:
			*off1=&eCPU.bx;
			*off2=&eCPU.di;
			ret=MemoryStart+(unsigned short)(eCPU.bx+eCPU.di)+(tds);
			break;
		case 2:
			*off1=&eCPU.bp;
			*off2=&eCPU.si;
			ret=MemoryStart+(unsigned short)(eCPU.bp+eCPU.si)+(tss);
			break;
		case 3:
			*off1=&eCPU.bp;
			*off2=&eCPU.di;
			ret=MemoryStart+(unsigned short)(eCPU.bp+eCPU.di)+(tss);
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
			ret=MemoryStart+(unsigned short)eCPU.eax+(tds);
			break;
		case 1:
			*off1=&eCPU.cx;
			ret=MemoryStart+(unsigned short)eCPU.ecx+(tds);
			break;
		case 2:
			*off1=&eCPU.dx;
			ret=MemoryStart+(unsigned short)eCPU.edx+(tss);
			break;
		case 3:
			*off1=&eCPU.bx;
			ret=MemoryStart+(unsigned short)eCPU.ebx+(tss);
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
		return (unsigned int)&Glbffff;
}

void TranslateOpcExt(t_bool w,char** rg,char** rm)
{
	unsigned short *off1,*off2;
	unsigned int tds=tmpDs;
	unsigned char mod=*(char*)(evIP+MemoryStart) & 0xc0;
	unsigned char reg=*(char*)(evIP+MemoryStart) & 0x38;
	unsigned char rem=*(char*)(evIP+MemoryStart) & 0x07;
	tds<<=4;
	mod>>=6;
	reg>>=3;

	*rg=(char *)FindRegAddr(w,reg);

	switch(mod)
	{
	case 0:
		if (rem==6 && tmpAddrSize==2)
		{
			evIP++;
			*rm=(char *)((*(unsigned short *)eIMS) + tds);
			*rm+=MemoryStart;
			evIP++;
		}
		else if (rem==5 && tmpAddrSize==4)
		{
			evIP++;
			*rm=(char *)((*(unsigned int *)eIMS) + tds);
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
			*rm+=(*(unsigned int *)off1+*(unsigned int *)off2+*(char *)(evIP+MemoryStart))-(*(unsigned int *)off1+*(unsigned int *)off2);		//对偏移寄存器溢出的处理，对一字节的偏移是进行符号扩展的，用带符号char效果一样
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;		
		if (tmpAddrSize==2)
			*rm+=(unsigned short)(*off1+*off2+*(unsigned short *)(evIP+MemoryStart))-(*off1+*off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
		else
			*rm+=(unsigned short)(*(unsigned int *)off1+*(unsigned int *)off2+*(unsigned short *)(evIP+MemoryStart))-(*(unsigned int *)off1+*(unsigned int *)off2);			//Bochs把1094:59ae上的2B偏移解释成无符号数
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
	unsigned short *seg,*off1,*off2;
	unsigned int tds=tmpDs;
	unsigned char mod=*(char*)(evIP+MemoryStart) & 0xc0;
	unsigned char reg=*(char*)(evIP+MemoryStart) & 0x38;
	unsigned char rem=*(char*)(evIP+MemoryStart) & 0x07;
	w=0x01;				//与Seg有关的操作，w只有是1
	tds<<=4;

	mod>>=6;
	reg>>=3;

	*rg=(char *)FindSegAddr(w,reg);

	switch(mod)
	{
	case 0:
		if (rem==6 && tmpAddrSize==2)
		{
			evIP++;
			*rm=(char *)((*(unsigned short *)eIMS) + tds);
			*rm+=MemoryStart;
			evIP++;
		}
		else if (rem==5 && tmpAddrSize==4)
		{
			evIP++;
			*rm=(char *)((*(unsigned short *)eIMS) + tds);
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
			*rm+=(*(unsigned int *)off1+*(unsigned int *)off2+*(char *)(evIP+MemoryStart))-(*(unsigned int *)off1+*(unsigned int *)off2);		//对偏移寄存器溢出的处理
		break;
	case 2:
		*rm=(char *)FindRemAddr(rem,&off1,&off2);
		evIP++;	
		if (tmpAddrSize==2)
			*rm+=(unsigned short)(*off1+*off2+*(unsigned short *)(evIP+MemoryStart))-(*off1+*off2);	
		else
			*rm+=(unsigned short)(*(unsigned int *)off1+*(unsigned int *)off2+*(unsigned short *)(evIP+MemoryStart))-(*(unsigned int *)off1+*(unsigned int *)off2);	
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
	unsigned int tmp=(unsigned int)BitBase;
	tmp+=BitOffset/8;
	BitBase=(void *)tmp;
	return (*(unsigned char *)BitBase>>(BitOffset%8))&1;
}

// 读到的字节未编码指令
void OpcError()
{
	printf("unknown instruction\n");
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 高度抽象部分

void ADD(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des+=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des+=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des+=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
}

void PUSH(void *Src,int Len)
{
	switch(Len)
	{
	case 2:
		eCPU.sp-=2;
		*(unsigned short*)( EvSP +MemoryStart)=*(unsigned short *)Src;
		break;
	case 4:
		eCPU.sp-=4;
		*(unsigned int*)( EvSP +MemoryStart)=*(unsigned int *)Src;
		break;
	}
}

void POP(void *Des, int Len)
{
	switch(Len)
	{
	case 2:
		*(unsigned short *)Des=*(unsigned short*)(EvSP+MemoryStart);
		eCPU.sp+=2;
		break;
	case 4:
		*(unsigned int *)Des=*(unsigned int*)(EvSP+MemoryStart);
		eCPU.sp+=4;
		break;
	}
}

void OR(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des|=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des|=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des|=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
}

void ADC(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		t81=*(unsigned char *)*Des;
		t82=*(unsigned char *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov al,t81
		__asm adc al,t82	
		__asm mov t81,al
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned char *)*Des=t81;
		break;
	case 2:
		toe16;
		t161=*(unsigned short *)*Des;
		t162=*(unsigned short *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov ax,t161
		__asm adc ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned short *)*Des=t161;
		break;
	case 4:
		toe32;
		t321=*(unsigned int *)*Des;
		t322=*(unsigned int *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov eax,t321
		__asm adc eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned int *)*Des=t321;
		break;
	}
}

void SBB(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		t81=*(unsigned char *)*Des;
		t82=*(unsigned char *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned char *)*Des=t81;
		break;
	case 2:
		toe16;
		t161=*(unsigned short *)*Des;
		t162=*(unsigned short *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov ax,t161
		__asm sbb ax,t162	
		__asm mov t161,ax
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned short *)*Des=t161;
		break;
	case 4:
		toe32;
		t321=*(unsigned int *)*Des;
		t322=*(unsigned int *)*Src;
		__asm push eCPU.eflags
		__asm popfd
		__asm mov eax,t321
		__asm sbb eax,t322	
		__asm mov t321,eax
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned int *)*Des=t321;
		break;
	}
}

void AND(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des&=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des&=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des&=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
}

void SUB(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des-=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des-=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des-=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
}

void XOR(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des^=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des^=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des^=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		break;
	}
}

void CMP(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned char *)*Des-=*(unsigned char *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags			
		*(unsigned char *)*Des+=*(unsigned char *)*Src;
		break;
	case 2:
		toe16;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned short *)*Des-=*(unsigned short *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned short *)*Des+=*(unsigned short *)*Src;
		break;
	case 4:
		toe32;
		__asm push eCPU.eflags				//加操作只修改某些位，如果直接就pop eCPU.flags会把整个eCPU.flags都修改掉。
		__asm popfd
		*(unsigned int *)*Des-=*(unsigned int *)*Src;
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned int *)*Des+=*(unsigned int *)*Src;
		break;
	}
}

void INC(void *Des, int Len)
{
	switch(Len)
	{
	case 2:
		t161=*(unsigned short *)Des;
		__asm push eCPU.eflags
		__asm popfd
		__asm inc t161
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned short *)Des=t161;
		break;
	case 4:
		t321=*(unsigned int *)Des;
		__asm push eCPU.eflags
		__asm popfd
		__asm inc t321
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned int *)Des=t321;
		break;
	}
}

void DEC(void *Des, int Len)
{
	switch(Len)
	{
	case 2:
		t161=*(unsigned short *)Des;
		__asm push eCPU.eflags
		__asm popfd
		__asm dec t161
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned short *)Des=t161;
		break;
	case 4:
		t321=*(unsigned int *)Des;
		__asm push eCPU.eflags
		__asm popfd
		__asm dec t321
		__asm pushfd
		__asm pop eCPU.eflags
		*(unsigned int *)Des=t321;
		break;
	}
}

void TEST(void **Des, void **Src, int Len)
{
	switch(Len)
	{
	case 1:
		toe8;
		t81=*(unsigned char *)*Des,t82=*(unsigned char *)*Src;
		__asm mov al,t82
		__asm push eCPU.eflags
		__asm popfd
		__asm test t81,al			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	case 2:
		toe16;
		t161=*(unsigned short *)*Des,t162=*(unsigned short *)*Src;
		__asm mov ax,t162
		__asm push eCPU.eflags
		__asm popfd
		__asm test t161,ax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	case 4:
		toe32;
		t321=*(unsigned int *)*Des,t322=*(unsigned int *)*Src;
		__asm mov eax,t322
		__asm push eCPU.eflags
		__asm popfd
		__asm test t321,eax			//这里这样做，其实是因为我不确切知道test的行为……
		__asm pushfd
		__asm pop eCPU.eflags
		break;
	}
}

void XCHG(void *Des, void *Src, int Len)
{
	switch(Len)
	{
	case 1:
		t81=*(unsigned char *)Des;
		*(unsigned char *)Des=*(unsigned char *)Src;
		*(unsigned char *)Src=t81;
		break;
	case 2:
		t161=*(unsigned short *)Des;
		*(unsigned short *)Des=*(unsigned short *)Src;
		*(unsigned short *)Src=t161;
		break;
	case 4:
		t321=*(unsigned int *)Des;
		*(unsigned int *)Des=*(unsigned int *)Src;
		*(unsigned int *)Src=t321;
		break;
	}
}

void MOV(void *Des, void *Src, int Len)
{
	switch(Len)
	{
	case 1:
		*(unsigned char *)Des=*(unsigned char *)Src;
		break;
	case 2:
		*(unsigned short *)Des=*(unsigned short *)Src;
		break;
	case 4:
		*(unsigned int *)Des=*(unsigned int *)Src;
		break;
	}
}

void SHL(void *Des, unsigned char mb, int Len)
{
	unsigned char tSHLrm8;
	unsigned short tSHLrm16;
	unsigned int tSHLrm32;
	switch(Len)
	{
	case 1:
		tSHLrm8=*(unsigned char *)Des;		
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
		*(unsigned char *)Des=tSHLrm8;
		break;
	case 2:
		tSHLrm16=*(unsigned short *)Des;		
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
		*(unsigned short *)Des=tSHLrm16;
		break;
	case 4:
		tSHLrm32=*(unsigned int *)Des;		
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
		*(unsigned int *)Des=tSHLrm8;
		break;
	}
}

void Jcc(int Len)
{
	int tsj;
	switch(Len)
	{
	case 1:
		tsj=*(char *)eIMS;
		evIP+=tsj;
		break;
	case 2:
		tsj=*(short *)eIMS;
		evIP+=tsj;
		break;
	case 4:
		tsj=*(int *)eIMS;
		evIP+=tsj;
		break;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	ADD((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void ADD_AX_I16()
{
	unsigned int tevIP=evIP;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	OR((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void OR_AX_I16()
{
	unsigned int tevIP=evIP;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	ADC((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void ADC_AX_I16()
{
	unsigned int tevIP=evIP;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	SBB((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void SBB_AX_I16()
{
	unsigned int tevIP=evIP;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	AND((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void AND_AX_I16()
{
	unsigned int tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	AND((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}

void ES()
{
// 	unsigned short tds=eCPU.ds;
// 	unsigned short tes=eCPU.es;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	SUB((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void SUB_AX_I16()
{
	unsigned int tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	SUB((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}

void CS()
{
// 	unsigned short tds=eCPU.ds;
// 	unsigned short tcs=eCPU.cs;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	XOR((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void XOR_AX_I16()
{
	unsigned int tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)(evIP+MemoryStart);
	XOR((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}

void SS()
{
// 	unsigned short tds=eCPU.ds;
// 	unsigned short tss=eCPU.ss;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)(evIP+MemoryStart);
	CMP((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void CMP_AX_I16()
{
	unsigned int tevIP=evIP;
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
	unsigned short tsp=eCPU.sp;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	switch(oce)
	{
	case 0:		
		__asm push eCPU.flags
		__asm popf
		*rm8+=*(char *)eIMS;	
		break;
	case 1:
		__asm push eCPU.flags
		__asm popf
		*rm8|=*(char *)eIMS;
		break;
	case 2:
		t81=*rm8;
		t82=*(char *)eIMS;
		__asm push eCPU.flags
		__asm popf
		//*rm8+=(*(char *)eIMS+((eCPU.flags & CF)!=0));	//在VC6里，逻辑值“真”＝1
		__asm mov al,t81
		__asm adc al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 3:		
		t81=*rm8;
		t82=*(char *)eIMS;
		__asm push eCPU.flags
		__asm popf
		//*rm8-=(*(char *)eIMS+((eCPU.flags & CF)!=0));			
		__asm mov al,t81
		__asm sbb al,t82
		__asm mov t81,al
		*rm8=t81;
		break;
	case 4:		
		__asm push eCPU.flags
		__asm popf
		*rm8&=*(char *)eIMS;			
		break;
	case 5:		
		__asm push eCPU.flags
		__asm popf
		*rm8-=*(char *)eIMS;			
		break;
	case 6:		
		__asm push eCPU.flags
		__asm popf
		*rm8^=*(char *)eIMS;			
		break;
	case 7:		
		__asm push eCPU.flags
		__asm popf
		*rm8-=*(char *)eIMS;					
		break;
	}
	__asm pushf
	__asm pop eCPU.flags
	if (oce==7)
		*rm8+=*(char *)eIMS;
	evIP++;
}

void INS_81()	//这里是以81开头的指令的集。
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	unsigned int tevIP=evIP;
	unsigned int teIMS=eIMS;
	oce>>=3;
	// 这里先保存IP，再toe16，再eIMS，然后又恢复IP
	// 这样做是因为，这条指令跟在Opcode后面的就是寻址，然后才是eIMS立即数。但是下面8个操作里面就有toe16，所以又要把IP恢复到toe16之前。
	// 其实下面8个操作里面是不应该有toe16的，不过已经写了很多，改起来太麻烦，就不改了。
	toe16;
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
	unsigned short tfg=*(char *)eIMS;			//这个强制转换就是按符号扩展完成的
	oce>>=3;
	toe8;
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
		t82=tfg;
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
		t82=tfg;
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
	oce>>=3;
	unsigned int tevIP=evIP;
	toe16;
	unsigned short tfg=*(char *)eIMS;			//这个强制转换本身就是符号扩展的
	unsigned int ptfg=(unsigned int)&tfg;
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
	unsigned char mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	unsigned char rem=*(char*)(evIP+MemoryStart) & 0x07;
	toe16;
	switch(rem)
	{
	case 0:
	case 1:
	case 4:
	case 5:
	case 7:
		*r16=(unsigned short)((unsigned int)rm16-MemoryStart-(t=tmpDs,t<<4));
		break;
	case 2:
	case 3:	
		*r16=(unsigned short)((unsigned int)rm16-MemoryStart-(t=tmpSs,t<<4));
		break;
	case 6:
		if (mod==0)		
			*r16=(unsigned short)((unsigned int)rm16-MemoryStart-(t=tmpDs,t<<4));
		else
			*r16=(unsigned short)((unsigned int)rm16-MemoryStart-(t=tmpSs,t<<4));
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
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip=*(unsigned short *)eIMS;
	evIP+=2;
	eCPU.cs=*(unsigned short *)eIMS;
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
	*(unsigned char *)&eCPU.flags=eCPU.ah;
}

void LAHF()
{	
	eCPU.ah=*(unsigned char *)&eCPU.flags;
}

// 0xA0
void MOV_AL_M8()
{
	t81=GetM8(*(unsigned short *)eIMS);
	MOV(&eCPU.al,&t81,1);	
	evIP+=2;
}

void MOV_AX_M16()
{
	switch(tmpOpdSize)
	{
	case 2:
		t161=GetM16(*(unsigned short *)eIMS);
		MOV(&eCPU.ax,&t161,tmpOpdSize);		
		break;
	case 4:
		t321=GetM32(*(unsigned short *)eIMS);
		MOV(&eCPU.eax,&t321,tmpOpdSize);		
		break;
	}	
	evIP+=tmpAddrSize;
}

void MOV_M8_AL()
{
	SetM8(*(unsigned short *)eIMS,eCPU.al);
	evIP+=2;
}

void MOV_M16_AX()
{
	switch(tmpOpdSize)
	{
	case 2:
		SetM16(*(unsigned short *)eIMS,eCPU.ax);
		break;
	case 4:
		SetM32(*(unsigned short *)eIMS,eCPU.eax);
		break;
	}	
	evIP+=tmpAddrSize;
}

void MOVSB()
{	
	unsigned char tgm;
	if (tmpAddrSize==2)	
		tgm=GetM8(eCPU.si);			//因为t是全局变量！在GetM8里已经改变了t的值！！！
	else
		tgm=GetM8(eCPU.esi);
	(t=eCPU.es,t<<=4);
	*((unsigned char *)(MemoryStart+eCPU.di+t))=tgm;
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
			*(unsigned short *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM16(eCPU.si);
		else
			*(unsigned short *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM16(eCPU.esi);
		break;
	case 4:
		//全局变量t不可以在一个赋值式的两边同时使用
		if (tmpAddrSize==2)
			*(unsigned int *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM32(eCPU.si);
		else
			*(unsigned int *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart)=GetM32(eCPU.esi);
		break;
	}

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
	unsigned char ta=GetM8(eCPU.si);
	unsigned char tb;
	if (tmpAddrSize==2)
		tb=*(unsigned char *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart);
	else
		tb=*(unsigned char *)(eCPU.edi+((t2=eCPU.es,t2<<4))+MemoryStart);
	__asm push eCPU.flags
	__asm popf
	//unsigned char ta=(unsigned char)(GetM8(eCPU.si)-*(unsigned char *)(eCPU.di+((t2=eCPU.es,t2<<4))+MemoryStart));
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
	unsigned int tevIP=evIP;
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
	unsigned int tevIP=evIP;
	void *pa=&eCPU.al,*pb=(void *)eIMS;
	TEST((void **)&pa,(void **)&pb,1);	
	evIP=tevIP+1;
}

void TEST_AX_I16()
{
	unsigned int tevIP=evIP;
	void *pa=&eCPU.ax,*pb=(void *)eIMS;
	TEST((void **)&pa,(void **)&pb,tmpOpdSize);
	evIP=tevIP+tmpOpdSize;
}

void STOSB()
{
	if (tmpAddrSize==2)
		*(unsigned char *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.al;
	else
		*(unsigned char *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.al;
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
			*(unsigned short *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.ax;
		else
			*(unsigned short *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.ax;
		break;
	case 4:
		if (tmpAddrSize==2)
			*(unsigned int *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.eax;
		else
			*(unsigned int *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart)=eCPU.eax;
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
		eCPU.al=GetM8(eCPU.si);
	else
		eCPU.al=GetM8(eCPU.esi);
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
			eCPU.ax=GetM16(eCPU.si);
		else
			eCPU.ax=GetM16(eCPU.esi);
		break;
	case 4:
		if (tmpAddrSize==2)
			eCPU.eax=GetM32(eCPU.si);
		else
			eCPU.eax=GetM32(eCPU.esi);
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
	unsigned char ta=eCPU.al;
	unsigned char tb;
	if (tmpAddrSize==2)
		tb=*(unsigned char *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart);
	else
		tb=*(unsigned char *)(eCPU.edi+((t=eCPU.es,t<<4))+MemoryStart);
	__asm push eCPU.flags
	__asm popf
	//unsigned char ta=(unsigned char)(eCPU.al-*(unsigned char *)(eCPU.di+((t=eCPU.es,t<<4))+MemoryStart));
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
	unsigned int tevIP=evIP;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	unsigned char t=*rm8;
	unsigned short teIMS=(*(unsigned char *)eIMS);
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
	unsigned short t;
	unsigned int t2;
	unsigned short teIMS;
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	switch (tmpOpdSize)
	{
	case 2:	
		toe16;
		teIMS=*(unsigned char *)eIMS;
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
		teIMS=*(unsigned char *)eIMS;
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
	toe8;	
	unsigned char teIMS=*(unsigned char *)eIMS;
	SHL(rm8,teIMS,1);
	evIP++;
}

void SHL_RM16_I8()
{
	toe16;	
	unsigned char teIMS=*(unsigned char *)eIMS;
	SHL(rm16,teIMS,tmpOpdSize);
	evIP++;
}

void RET_I8()
{
	eCPU.ip=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.sp+=*(unsigned short*)eIMS;
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}

void RET_NEAR()
{
	eCPU.ip=*(unsigned short*)(EvSP+MemoryStart);
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
		eCPU.es=*(unsigned short *)(rm16+1);
		break;
	case 4:
		toe32;
		*r32=*rm32;
		eCPU.es=*(unsigned short *)(rm32+1);
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
		eCPU.ds=*(unsigned short *)(rm16+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
		break;
	case 4:
		toe32;	
		*r32=*rm32;
		eCPU.ds=*(unsigned short *)(rm32+1);		//因为rm16本来就是双字节的，所以这里+1即可求得下一双字节
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
	eCPU.ip=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.cs=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;	
	eCPU.sp+=*(unsigned short*)eIMS;
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}

void RET_FAR()
{
	eCPU.ip=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.cs=*(unsigned short*)(EvSP+MemoryStart);
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
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.flags;
	eCPU.flags&=~IF;
	eCPU.flags&=~TF;
	eCPU.flags&=~AF;
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip=*(unsigned short *)((*(unsigned char *)eIMS)*4+MemoryStart);	
	eCPU.cs=*(unsigned short *)((*(unsigned char *)eIMS)*4+2+MemoryStart);
	
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}

void INTO()
{
	if (eCPU.flags&OF)
	{
		GlobINT|=0x10;
	}
// 	eCPU.sp-=2;
// 	*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;
// 	eCPU.sp-=2;
// 	*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
// 	eCPU.ip=(4)*4+MemoryStart;
// 	evIP+=2;
// 	eCPU.cs=0;
// 	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}

void IRET()					//在实模式下，iret和ret far是一样的，这里可以直接调用RET_FAR()的，不过为了以后扩展着想就不这样做。
{
	eCPU.ip=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.cs=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	eCPU.flags=*(unsigned short*)(EvSP+MemoryStart);
	eCPU.sp+=2;		
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}

// 0xD0
void INS_D0()
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	unsigned char t=*rm8;
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
	unsigned short t;
	unsigned int t2;
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	unsigned char t=*rm8;
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
	unsigned short t;
	unsigned int t2;
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
	if ((*(unsigned char *)eIMS)==0)
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
	eCPU.al=GetM8((unsigned short)(eCPU.bx+eCPU.al));
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
	unsigned int t=InTable[*(unsigned char*)eIMS];
	__asm
	{
		mov eax,t
		call eax		
	}
	evIP++;
}

void IN_AX_N()
{
	// in ax,n 或者 in eax,n 是连续地读端口。InTable里的函数都是直接修改eCPU.al的，最好把每个eCPU.al都保存起来，再一次过赋给eCPU.ax或eCPU.eax
	int i;
	unsigned int t;
	for (i=0;i<tmpOpdSize;i++)
	{
		t=InTable[*(unsigned char*)eIMS+i];
		__asm
		{
			mov eax,t
			call eax			
		}
		*(unsigned char *)(&eCPU.al+i)=eCPU.al;
	}
	evIP++;
}

void OUT_N_AL()
{
	unsigned int t=OutTable[*(unsigned char*)eIMS];
	unsigned int tal=eCPU.al;
	__asm
	{
		push tal
		call t
		pop tal
	}
	evIP++;
}

void OUT_N_AX()
{
	int i;
	unsigned int t;
	for (i=0;i<tmpOpdSize;i++)
	{
		t=OutTable[*(unsigned char*)eIMS+i];
		unsigned int tax=*(unsigned char *)(&eCPU.ax+i);
		__asm
		{
			push tax
			call t
			pop tax
		}
	}
	evIP++;
}

void CALL_NEAR()
{
	LongCallNewIP(2);
	eCPU.sp-=2;										//段内CALL，CS不压栈
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip+=(*(unsigned short *)eIMS);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}

void JMP_NEAR_LABEL()	//立即数是两字节的
{
	LongCallNewIP(2);
	eCPU.ip+=(*(unsigned short *)eIMS);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}

void JMP_FAR_LABEL()
{
	LongCallNewIP(4);
	eCPU.ip=*(unsigned short *)eIMS;
	evIP+=2;
	eCPU.cs=*(unsigned short *)eIMS;
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}

void JMP_NEAR()			//立即数是一字节的
{	
	LongCallNewIP(1);
	eCPU.ip+=(*(char *)eIMS);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;	
}

void IN_AL_DX()
{
	unsigned int t=InTable[eCPU.dx];
	__asm
	{
		mov eax,t
		call eax		
	}
}

void IN_AX_DX()
{
	int i;
	unsigned int t;
	for (i=0;i<tmpOpdSize;i++)
	{
		t=InTable[eCPU.dx+i];
		__asm
		{
			mov eax,t
			call eax		
		}
		*(unsigned char *)(&eCPU.al+i)=eCPU.al;
	}
}

void OUT_DX_AL()
{
	unsigned int t=OutTable[eCPU.dx];
	unsigned int tal=eCPU.al;
	__asm
	{
		push tal
		call t		
		pop tal
	}
}

void OUT_DX_AX()
{
	int i;
	unsigned short tdx=eCPU.dx;
	unsigned int t;
	for (i=0;i<tmpOpdSize;i++)
	{
		t=OutTable[tdx+i];
		unsigned int tal=*(unsigned char *)(&eCPU.al+i);
		__asm
		{
			push tal
			call t	
			pop tal
		}
	}
}

// 0xF0
void LOCK()
{
	__asm nop;
}

void REPNE()
{
	unsigned int tevIP=evIP;
	unsigned char nopc;
	if (eCPU.cx==0)
	{
		nopc=*(unsigned char *)(MemoryStart+evIP);		
		if ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是CS、ES之类的指令，则再跳一字节
		{
			evIP++;
			nopc=*(unsigned char *)(MemoryStart+evIP);
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
	unsigned int tevIP=evIP;
	unsigned char nopc;
	eCPU.flags |= ZF;		//ZF位置1
	if (eCPU.cx==0)
	{
		nopc=*(unsigned char *)(MemoryStart+evIP);		
		while ((nopc&0xe7) == 0x26 || (nopc&0xfc) == 0x64 || (nopc&0xfc) == 0xf0)	//如果跟在REP指令之后的是指令前缀，则再跳一字节
		{
			evIP++;
			nopc=*(unsigned char *)(MemoryStart+evIP);
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
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	toe8;
	char trm8=*rm8;
	char tc=*(char *)eIMS;	
	__asm push eCPU.flags
	switch(oce)
	{
	case 0:			
		__asm mov al,tc
		__asm popf
		__asm test trm8,al
		break;
	case 1:
		OpcError();
		break;
	case 2:
		__asm popf
		__asm not trm8	
		*rm8=trm8;
		break;
	case 3:		
		__asm popf
		__asm neg trm8	
		*rm8=trm8;
		break;
	case 4:		
		__asm mov al,eCPU.al
		__asm popf
		__asm mul trm8	
		__asm mov eCPU.ax,ax
		break;
	case 5:		
		__asm mov al,eCPU.al
		__asm popf
		__asm imul trm8	
		__asm mov eCPU.ax,ax
		break;
	case 6:		
		if (trm8!=0)			//这里只针对8086的
		{		
			__asm mov ax,eCPU.ax
			__asm popf
			__asm div trm8	
			__asm mov eCPU.ax,ax
		}
		else
		{
			GlobINT|=1;
		}
		break;
	case 7:		
		if (trm8!=0)
		{		
			__asm mov ax,eCPU.ax
			__asm popf
			__asm idiv trm8	
			__asm mov eCPU.ax,ax
		}
		else
		{
			GlobINT|=1;
		}		
		break;
	}
	__asm pushf
	__asm pop eCPU.flags
	//*rm8=trm8;			//这里不能这样写，因为有时候那个rm8正好就是mul的结果，一改回原来的trm8，结果就被覆盖了
	//evIP++;
	if (oce==0)			//只有oce==0带有立即数
		evIP++;
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
		tc=*(unsigned short *)eIMS;
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
		tc2=*(unsigned int *)eIMS;
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
{	
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	char mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	mod&=0x3;
	char rem=*(char*)(evIP+MemoryStart) & 0x07;
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
	unsigned char trm8=*rm8;
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
		*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
		JMP_NEAR_LABEL();		
		break;
	case 3:		
		//__asm popf
		eCPU.sp-=2;
		*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
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
		*(unsigned short*)( EvSP +MemoryStart)=(unsigned short)*rm8;
		break;
	case 7:		
		OpcError();					
		break;
	}
	//evIP++;
}

void INS_FF()
{
	char oce=*(char*)(evIP+MemoryStart) & 0x38;
	oce>>=3;
	char mod=*(char*)(evIP+MemoryStart) & 0xc0;
	mod>>=6;
	mod&=0x3;
	char rem=*(char*)(evIP+MemoryStart) & 0x07;
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
	unsigned short trm16;
	unsigned int trm32;
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
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm16;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		case 4:
			eCPU.sp-=2;
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
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
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;
			eCPU.sp-=2;
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
			eCPU.ip=*rm16;
			eCPU.cs=*(rm16+1);
			evIP+=2;
			evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
			break;
		case 4:
			eCPU.sp-=2;
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;
			eCPU.sp-=2;
			*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
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
	unsigned char OpC=*(unsigned char *)(evIP+MemoryStart);
	evIP++;
	unsigned int InsFucAddr=Ins0FTable[OpC];
	__asm call InsFucAddr;
	unsigned int tcs=eCPU.cs;
	eCPU.ip=(evIP - (tcs << 4)) % 0x10000;
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
}



//////////////////////////////////////////////////////////////////////////
// 下面这部分是0F开头的指令

void ADDPS()
{
	unsigned char a=*(unsigned char *)(evIP+MemoryStart);
	unsigned char b=a&0x7;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags;
	__asm popf
	for (int i=0;i<4;i++)
	{
		eCPU.xmm[a].fp[i]+=eCPU.xmm[b].fp[i];
	}
	__asm pushf
	__asm pop eCPU.eflags;
	evIP++;
}

void ADDSS()
{
	unsigned char a=*(unsigned char *)(evIP+MemoryStart);
	unsigned char b=a&0x7;
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
	unsigned char a=*(unsigned char *)(evIP+MemoryStart);
	unsigned char b=a&0x7;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags
	__asm popf
	for (int i=0;i<4;i++)
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
	unsigned char a=*(unsigned char *)(evIP+MemoryStart);
	unsigned char b=a&0x7;
	a>>=4;a&=0x7;
	__asm push eCPU.eflags;
	__asm popf
	for (int i=0;i<4;i++)
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
	unsigned int temp;
	unsigned int *desc;
	desc=(unsigned int *)FindRegAddr(1,*(unsigned char *)(eIMS-1));
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
	unsigned char OpC=*(unsigned char *)(evIP+MemoryStart);
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
	unsigned char OpC=*(unsigned char *)(evIP+MemoryStart);
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
		*rm16=(unsigned short)eCPU.CR[0];
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

// yinX：	用这么笨的方法是为了让以后修改某条指令的时候更灵活
// 路人甲：	真的会更灵活吗？
// yinX：	…… -_-!
void SetupInstructionTable()
{
	InsTable[0x00]=(unsigned int)ADD_RM8_R8;
	InsTable[0x01]=(unsigned int)ADD_RM16_R16;
	InsTable[0x02]=(unsigned int)ADD_R8_RM8;
	InsTable[0x03]=(unsigned int)ADD_R16_RM16;
	InsTable[0x04]=(unsigned int)ADD_AL_I8;
	InsTable[0x05]=(unsigned int)ADD_AX_I16;
	InsTable[0x06]=(unsigned int)PUSH_ES;
	InsTable[0x07]=(unsigned int)POP_ES;
	InsTable[0x08]=(unsigned int)OR_RM8_R8;
	InsTable[0x09]=(unsigned int)OR_RM16_R16;
	InsTable[0x0A]=(unsigned int)OR_R8_RM8;
	InsTable[0x0B]=(unsigned int)OR_R16_RM16;
	InsTable[0x0C]=(unsigned int)OR_AL_I8;
	InsTable[0x0D]=(unsigned int)OR_AX_I16;
	InsTable[0x0E]=(unsigned int)PUSH_CS;
	InsTable[0x0F]=(unsigned int)INS_0F;
	InsTable[0x10]=(unsigned int)ADC_RM8_R8;
	InsTable[0x11]=(unsigned int)ADC_RM16_R16;
	InsTable[0x12]=(unsigned int)ADC_R8_RM8;
	InsTable[0x13]=(unsigned int)ADC_R16_RM16;
	InsTable[0x14]=(unsigned int)ADC_AL_I8;
	InsTable[0x15]=(unsigned int)ADC_AX_I16;
	InsTable[0x16]=(unsigned int)PUSH_SS;
	InsTable[0x17]=(unsigned int)POP_SS;
	InsTable[0x18]=(unsigned int)SBB_RM8_R8;
	InsTable[0x19]=(unsigned int)SBB_RM16_R16;
	InsTable[0x1A]=(unsigned int)SBB_R8_RM8;
	InsTable[0x1B]=(unsigned int)SBB_R16_RM16;
	InsTable[0x1C]=(unsigned int)SBB_AL_I8;
	InsTable[0x1D]=(unsigned int)SBB_AX_I16;
	InsTable[0x1E]=(unsigned int)PUSH_DS;
	InsTable[0x1F]=(unsigned int)POP_DS;
	InsTable[0x20]=(unsigned int)AND_RM8_R8;
	InsTable[0x21]=(unsigned int)AND_RM16_R16;
	InsTable[0x22]=(unsigned int)AND_R8_RM8;
	InsTable[0x23]=(unsigned int)AND_R16_RM16;
	InsTable[0x24]=(unsigned int)AND_AL_I8;
	InsTable[0x25]=(unsigned int)AND_AX_I16;
	InsTable[0x26]=(unsigned int)ES;
	InsTable[0x27]=(unsigned int)DAA;
	InsTable[0x28]=(unsigned int)SUB_RM8_R8;
	InsTable[0x29]=(unsigned int)SUB_RM16_R16;
	InsTable[0x2A]=(unsigned int)SUB_R8_RM8;
	InsTable[0x2B]=(unsigned int)SUB_R16_RM16;
	InsTable[0x2C]=(unsigned int)SUB_AL_I8;
	InsTable[0x2D]=(unsigned int)SUB_AX_I16;
	InsTable[0x2E]=(unsigned int)CS;
	InsTable[0x2F]=(unsigned int)DAS;
	InsTable[0x30]=(unsigned int)XOR_RM8_R8;
	InsTable[0x31]=(unsigned int)XOR_RM16_R16;
	InsTable[0x32]=(unsigned int)XOR_R8_RM8;
	InsTable[0x33]=(unsigned int)XOR_R16_RM16;
	InsTable[0x34]=(unsigned int)XOR_AL_I8;
	InsTable[0x35]=(unsigned int)XOR_AX_I16;
	InsTable[0x36]=(unsigned int)SS;
	InsTable[0x37]=(unsigned int)AAA;
	InsTable[0x38]=(unsigned int)CMP_RM8_R8;
	InsTable[0x39]=(unsigned int)CMP_RM16_R16;
	InsTable[0x3A]=(unsigned int)CMP_R8_RM8;
	InsTable[0x3B]=(unsigned int)CMP_R16_RM16;
	InsTable[0x3C]=(unsigned int)CMP_AL_I8;
	InsTable[0x3D]=(unsigned int)CMP_AX_I16;
	InsTable[0x3E]=(unsigned int)DS;
	InsTable[0x3F]=(unsigned int)AAS;
	InsTable[0x40]=(unsigned int)INC_AX;
	InsTable[0x41]=(unsigned int)INC_CX;
	InsTable[0x42]=(unsigned int)INC_DX;
	InsTable[0x43]=(unsigned int)INC_BX;
	InsTable[0x44]=(unsigned int)INC_SP;
	InsTable[0x45]=(unsigned int)INC_BP;
	InsTable[0x46]=(unsigned int)INC_SI;
	InsTable[0x47]=(unsigned int)INC_DI;
	InsTable[0x48]=(unsigned int)DEC_AX;
	InsTable[0x49]=(unsigned int)DEC_CX;
	InsTable[0x4A]=(unsigned int)DEC_DX;
	InsTable[0x4B]=(unsigned int)DEC_BX;
	InsTable[0x4C]=(unsigned int)DEC_SP;
	InsTable[0x4D]=(unsigned int)DEC_BP;
	InsTable[0x4E]=(unsigned int)DEC_SI;
	InsTable[0x4F]=(unsigned int)DEC_DI;
	InsTable[0x50]=(unsigned int)PUSH_AX;
	InsTable[0x51]=(unsigned int)PUSH_CX;
	InsTable[0x52]=(unsigned int)PUSH_DX;
	InsTable[0x53]=(unsigned int)PUSH_BX;
	InsTable[0x54]=(unsigned int)PUSH_SP;
	InsTable[0x55]=(unsigned int)PUSH_BP;
	InsTable[0x56]=(unsigned int)PUSH_SI;
	InsTable[0x57]=(unsigned int)PUSH_DI;
	InsTable[0x58]=(unsigned int)POP_AX;
	InsTable[0x59]=(unsigned int)POP_CX;
	InsTable[0x5A]=(unsigned int)POP_DX;
	InsTable[0x5B]=(unsigned int)POP_BX;
	InsTable[0x5C]=(unsigned int)POP_SP;
	InsTable[0x5D]=(unsigned int)POP_BP;
	InsTable[0x5E]=(unsigned int)POP_SI;
	InsTable[0x5F]=(unsigned int)POP_DI;
	InsTable[0x60]=(unsigned int)OpcError;
	InsTable[0x61]=(unsigned int)OpcError;
	InsTable[0x62]=(unsigned int)OpcError;
	InsTable[0x63]=(unsigned int)OpcError;
	InsTable[0x64]=(unsigned int)OpcError;
	InsTable[0x65]=(unsigned int)OpcError;
	InsTable[0x66]=(unsigned int)OpdSize;
	InsTable[0x67]=(unsigned int)AddrSize;
	InsTable[0x68]=(unsigned int)PUSH_I16;
	InsTable[0x69]=(unsigned int)OpcError;
	InsTable[0x6A]=(unsigned int)OpcError;
	InsTable[0x6B]=(unsigned int)OpcError;
	InsTable[0x6C]=(unsigned int)OpcError;
	InsTable[0x6D]=(unsigned int)OpcError;
	InsTable[0x6E]=(unsigned int)OpcError;
	InsTable[0x6F]=(unsigned int)OpcError;
	InsTable[0x70]=(unsigned int)JO;
	InsTable[0x71]=(unsigned int)JNO;
	InsTable[0x72]=(unsigned int)JC;
	InsTable[0x73]=(unsigned int)JNC;
	InsTable[0x74]=(unsigned int)JZ;
	InsTable[0x75]=(unsigned int)JNZ;
	InsTable[0x76]=(unsigned int)JBE;
	InsTable[0x77]=(unsigned int)JA;
	InsTable[0x78]=(unsigned int)JS;
	InsTable[0x79]=(unsigned int)JNS;
	InsTable[0x7A]=(unsigned int)JP;
	InsTable[0x7B]=(unsigned int)JNP;
	InsTable[0x7C]=(unsigned int)JL;
	InsTable[0x7D]=(unsigned int)JNL;
	InsTable[0x7E]=(unsigned int)JLE;
	InsTable[0x7F]=(unsigned int)JG;
	InsTable[0x80]=(unsigned int)INS_80;
	InsTable[0x81]=(unsigned int)INS_81;
	InsTable[0x82]=(unsigned int)INS_82;
	InsTable[0x83]=(unsigned int)INS_83;
	InsTable[0x84]=(unsigned int)TEST_RM8_M8;
	InsTable[0x85]=(unsigned int)TEST_RM16_M16;
	InsTable[0x86]=(unsigned int)XCHG_R8_RM8;
	InsTable[0x87]=(unsigned int)XCHG_R16_RM16;
	InsTable[0x88]=(unsigned int)MOV_RM8_R8;
	InsTable[0x89]=(unsigned int)MOV_RM16_R16;
	InsTable[0x8A]=(unsigned int)MOV_R8_RM8;
	InsTable[0x8B]=(unsigned int)MOV_R16_RM16;
	InsTable[0x8C]=(unsigned int)MOV_RM_SEG;
	InsTable[0x8D]=(unsigned int)LEA_R16_M16;
	InsTable[0x8E]=(unsigned int)MOV_SEG_RM;
	InsTable[0x8F]=(unsigned int)POP_RM16;
	InsTable[0x90]=(unsigned int)NOP;
	InsTable[0x91]=(unsigned int)XCHG_CX_AX;
	InsTable[0x92]=(unsigned int)XCHG_DX_AX;
	InsTable[0x93]=(unsigned int)XCHG_BX_AX;
	InsTable[0x94]=(unsigned int)XCHG_SP_AX;
	InsTable[0x95]=(unsigned int)XCHG_BP_AX;
	InsTable[0x96]=(unsigned int)XCHG_SI_AX;
	InsTable[0x97]=(unsigned int)XCHG_DI_AX;
	InsTable[0x98]=(unsigned int)CBW;
	InsTable[0x99]=(unsigned int)CWD;
	InsTable[0x9A]=(unsigned int)CALL_FAR;
	InsTable[0x9B]=(unsigned int)WAIT;
	InsTable[0x9C]=(unsigned int)PUSHF;
	InsTable[0x9D]=(unsigned int)POPF;
	InsTable[0x9E]=(unsigned int)SAHF;
	InsTable[0x9F]=(unsigned int)LAHF;
	InsTable[0xA0]=(unsigned int)MOV_AL_M8;
	InsTable[0xA1]=(unsigned int)MOV_AX_M16;
	InsTable[0xA2]=(unsigned int)MOV_M8_AL;
	InsTable[0xA3]=(unsigned int)MOV_M16_AX;
	InsTable[0xA4]=(unsigned int)MOVSB;
	InsTable[0xA5]=(unsigned int)MOVSW;
	InsTable[0xA6]=(unsigned int)CMPSB;
	InsTable[0xA7]=(unsigned int)CMPSW;
	InsTable[0xA8]=(unsigned int)TEST_AL_I8;
	InsTable[0xA9]=(unsigned int)TEST_AX_I16;
	InsTable[0xAA]=(unsigned int)STOSB;
	InsTable[0xAB]=(unsigned int)STOSW;
	InsTable[0xAC]=(unsigned int)LODSB;
	InsTable[0xAD]=(unsigned int)LODSW;
	InsTable[0xAE]=(unsigned int)SCASB;
	InsTable[0xAF]=(unsigned int)SCASW;
	InsTable[0xB0]=(unsigned int)MOV_AL_I8;
	InsTable[0xB1]=(unsigned int)MOV_CL_I8;
	InsTable[0xB2]=(unsigned int)MOV_DL_I8;
	InsTable[0xB3]=(unsigned int)MOV_BL_I8;
	InsTable[0xB4]=(unsigned int)MOV_AH_I8;
	InsTable[0xB5]=(unsigned int)MOV_CH_I8;
	InsTable[0xB6]=(unsigned int)MOV_DH_I8;
	InsTable[0xB7]=(unsigned int)MOV_BH_I8;
	InsTable[0xB8]=(unsigned int)MOV_AX_I16;
	InsTable[0xB9]=(unsigned int)MOV_CX_I16;
	InsTable[0xBA]=(unsigned int)MOV_DX_I16;
	InsTable[0xBB]=(unsigned int)MOV_BX_I16;
	InsTable[0xBC]=(unsigned int)MOV_SP_I16;
	InsTable[0xBD]=(unsigned int)MOV_BP_I16;
	InsTable[0xBE]=(unsigned int)MOV_SI_I16;
	InsTable[0xBF]=(unsigned int)MOV_DI_I16;
	InsTable[0xC0]=(unsigned int)INS_C0;
	InsTable[0xC1]=(unsigned int)INS_C1;
	InsTable[0xC2]=(unsigned int)RET_I8;
	InsTable[0xC3]=(unsigned int)RET_NEAR;
	InsTable[0xC4]=(unsigned int)LES_R16_M16;
	InsTable[0xC5]=(unsigned int)LDS_R16_M16;
	InsTable[0xC6]=(unsigned int)MOV_M8_I8;
	InsTable[0xC7]=(unsigned int)MOV_M16_I16;
	InsTable[0xC8]=(unsigned int)OpcError;
	InsTable[0xC9]=(unsigned int)OpcError;
	InsTable[0xCA]=(unsigned int)RET_I16;
	InsTable[0xCB]=(unsigned int)RET_FAR;
	InsTable[0xCC]=(unsigned int)INT3;
	InsTable[0xCD]=(unsigned int)INT_I8;
	InsTable[0xCE]=(unsigned int)INTO;
	InsTable[0xCF]=(unsigned int)IRET;
	InsTable[0xD0]=(unsigned int)INS_D0;
	InsTable[0xD1]=(unsigned int)INS_D1;
	InsTable[0xD2]=(unsigned int)INS_D2;
	InsTable[0xD3]=(unsigned int)INS_D3;
	InsTable[0xD4]=(unsigned int)AAM;
	InsTable[0xD5]=(unsigned int)AAD;
	InsTable[0xD6]=(unsigned int)OpcError;
	InsTable[0xD7]=(unsigned int)XLAT;
	InsTable[0xD8]=(unsigned int)OpcError;
	InsTable[0xD9]=(unsigned int)INS_D9;
	InsTable[0xDA]=(unsigned int)OpcError;
	InsTable[0xDB]=(unsigned int)INS_DB;
	InsTable[0xDC]=(unsigned int)OpcError;
	InsTable[0xDD]=(unsigned int)OpcError;
	InsTable[0xDE]=(unsigned int)OpcError;
	InsTable[0xDF]=(unsigned int)OpcError;
	InsTable[0xE0]=(unsigned int)LOOPNE;
	InsTable[0xE1]=(unsigned int)LOOPE;
	InsTable[0xE2]=(unsigned int)LOOP_NEAR;
	InsTable[0xE3]=(unsigned int)JCXZ_NEAR;
	InsTable[0xE4]=(unsigned int)IN_AL_N;
	InsTable[0xE5]=(unsigned int)IN_AX_N;
	InsTable[0xE6]=(unsigned int)OUT_N_AL;
	InsTable[0xE7]=(unsigned int)OUT_N_AX;
	InsTable[0xE8]=(unsigned int)CALL_NEAR;
	InsTable[0xE9]=(unsigned int)JMP_NEAR_LABEL;
	InsTable[0xEA]=(unsigned int)JMP_FAR_LABEL;
	InsTable[0xEB]=(unsigned int)JMP_NEAR;
	InsTable[0xEC]=(unsigned int)IN_AL_DX;
	InsTable[0xED]=(unsigned int)IN_AX_DX;
	InsTable[0xEE]=(unsigned int)OUT_DX_AL;
	InsTable[0xEF]=(unsigned int)OUT_DX_AX;
	InsTable[0xF0]=(unsigned int)LOCK;
	InsTable[0xF1]=(unsigned int)OpcError;
	InsTable[0xF2]=(unsigned int)REPNE;
	InsTable[0xF3]=(unsigned int)REP;
	InsTable[0xF4]=(unsigned int)HLT;
	InsTable[0xF5]=(unsigned int)CMC;
	InsTable[0xF6]=(unsigned int)INS_F6;
	InsTable[0xF7]=(unsigned int)INS_F7;
	InsTable[0xF8]=(unsigned int)CLC;
	InsTable[0xF9]=(unsigned int)STC;
	InsTable[0xFA]=(unsigned int)CLI;
	InsTable[0xFB]=(unsigned int)STI;
	InsTable[0xFC]=(unsigned int)CLD;
	InsTable[0xFD]=(unsigned int)STD;
	InsTable[0xFE]=(unsigned int)INS_FE;
	InsTable[0xFF]=(unsigned int)INS_FF;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	Ins0FTable[0x00]=(unsigned int)OpcError;
	Ins0FTable[0x01]=(unsigned int)INS_0F01;
	Ins0FTable[0x02]=(unsigned int)OpcError;
	Ins0FTable[0x03]=(unsigned int)OpcError;
	Ins0FTable[0x04]=(unsigned int)OpcError;
	Ins0FTable[0x05]=(unsigned int)OpcError;
	Ins0FTable[0x06]=(unsigned int)OpcError;
	Ins0FTable[0x07]=(unsigned int)OpcError;
	Ins0FTable[0x08]=(unsigned int)OpcError;
	Ins0FTable[0x09]=(unsigned int)OpcError;
	Ins0FTable[0x0A]=(unsigned int)OpcError;
	Ins0FTable[0x0B]=(unsigned int)OpcError;
	Ins0FTable[0x0C]=(unsigned int)OpcError;
	Ins0FTable[0x0D]=(unsigned int)OpcError;
	Ins0FTable[0x0E]=(unsigned int)OpcError;
	Ins0FTable[0x0F]=(unsigned int)OpcError;
	Ins0FTable[0x10]=(unsigned int)OpcError;
	Ins0FTable[0x11]=(unsigned int)OpcError;
	Ins0FTable[0x12]=(unsigned int)OpcError;
	Ins0FTable[0x13]=(unsigned int)OpcError;
	Ins0FTable[0x14]=(unsigned int)OpcError;
	Ins0FTable[0x15]=(unsigned int)OpcError;
	Ins0FTable[0x16]=(unsigned int)OpcError;
	Ins0FTable[0x17]=(unsigned int)OpcError;
	Ins0FTable[0x18]=(unsigned int)OpcError;
	Ins0FTable[0x19]=(unsigned int)OpcError;
	Ins0FTable[0x1A]=(unsigned int)OpcError;
	Ins0FTable[0x1B]=(unsigned int)OpcError;
	Ins0FTable[0x1C]=(unsigned int)OpcError;
	Ins0FTable[0x1D]=(unsigned int)OpcError;
	Ins0FTable[0x1E]=(unsigned int)OpcError;
	Ins0FTable[0x1F]=(unsigned int)OpcError;
	Ins0FTable[0x20]=(unsigned int)OpcError;
	Ins0FTable[0x21]=(unsigned int)OpcError;
	Ins0FTable[0x22]=(unsigned int)OpcError;
	Ins0FTable[0x23]=(unsigned int)OpcError;
	Ins0FTable[0x24]=(unsigned int)OpcError;
	Ins0FTable[0x25]=(unsigned int)OpcError;
	Ins0FTable[0x26]=(unsigned int)OpcError;
	Ins0FTable[0x27]=(unsigned int)OpcError;
	Ins0FTable[0x28]=(unsigned int)OpcError;
	Ins0FTable[0x29]=(unsigned int)OpcError;
	Ins0FTable[0x2A]=(unsigned int)OpcError;
	Ins0FTable[0x2B]=(unsigned int)OpcError;
	Ins0FTable[0x2C]=(unsigned int)OpcError;
	Ins0FTable[0x2D]=(unsigned int)OpcError;
	Ins0FTable[0x2E]=(unsigned int)OpcError;
	Ins0FTable[0x2F]=(unsigned int)OpcError;
	Ins0FTable[0x30]=(unsigned int)OpcError;
	Ins0FTable[0x31]=(unsigned int)OpcError;
	Ins0FTable[0x32]=(unsigned int)OpcError;
	Ins0FTable[0x33]=(unsigned int)OpcError;
	Ins0FTable[0x34]=(unsigned int)OpcError;
	Ins0FTable[0x35]=(unsigned int)OpcError;
	Ins0FTable[0x36]=(unsigned int)OpcError;
	Ins0FTable[0x37]=(unsigned int)OpcError;
	Ins0FTable[0x38]=(unsigned int)OpcError;
	Ins0FTable[0x39]=(unsigned int)OpcError;
	Ins0FTable[0x3A]=(unsigned int)OpcError;
	Ins0FTable[0x3B]=(unsigned int)OpcError;
	Ins0FTable[0x3C]=(unsigned int)OpcError;
	Ins0FTable[0x3D]=(unsigned int)OpcError;
	Ins0FTable[0x3E]=(unsigned int)OpcError;
	Ins0FTable[0x3F]=(unsigned int)OpcError;
	Ins0FTable[0x40]=(unsigned int)OpcError;
	Ins0FTable[0x41]=(unsigned int)OpcError;
	Ins0FTable[0x42]=(unsigned int)OpcError;
	Ins0FTable[0x43]=(unsigned int)OpcError;
	Ins0FTable[0x44]=(unsigned int)OpcError;
	Ins0FTable[0x45]=(unsigned int)OpcError;
	Ins0FTable[0x46]=(unsigned int)OpcError;
	Ins0FTable[0x47]=(unsigned int)OpcError;
	Ins0FTable[0x48]=(unsigned int)OpcError;
	Ins0FTable[0x49]=(unsigned int)OpcError;
	Ins0FTable[0x4A]=(unsigned int)OpcError;
	Ins0FTable[0x4B]=(unsigned int)OpcError;
	Ins0FTable[0x4C]=(unsigned int)OpcError;
	Ins0FTable[0x4D]=(unsigned int)OpcError;
	Ins0FTable[0x4E]=(unsigned int)OpcError;
	Ins0FTable[0x4F]=(unsigned int)OpcError;
	Ins0FTable[0x50]=(unsigned int)OpcError;
	Ins0FTable[0x51]=(unsigned int)OpcError;
	Ins0FTable[0x52]=(unsigned int)OpcError;
	Ins0FTable[0x53]=(unsigned int)OpcError;
	Ins0FTable[0x54]=(unsigned int)OpcError;
	Ins0FTable[0x55]=(unsigned int)OpcError;
	Ins0FTable[0x56]=(unsigned int)OpcError;
	Ins0FTable[0x57]=(unsigned int)OpcError;
	Ins0FTable[0x58]=(unsigned int)OpcError;
	Ins0FTable[0x59]=(unsigned int)OpcError;
	Ins0FTable[0x5A]=(unsigned int)OpcError;
	Ins0FTable[0x5B]=(unsigned int)OpcError;
	Ins0FTable[0x5C]=(unsigned int)OpcError;
	Ins0FTable[0x5D]=(unsigned int)OpcError;
	Ins0FTable[0x5E]=(unsigned int)OpcError;
	Ins0FTable[0x5F]=(unsigned int)OpcError;
	Ins0FTable[0x60]=(unsigned int)OpcError;
	Ins0FTable[0x61]=(unsigned int)OpcError;
	Ins0FTable[0x62]=(unsigned int)OpcError;
	Ins0FTable[0x63]=(unsigned int)OpcError;
	Ins0FTable[0x64]=(unsigned int)OpcError;
	Ins0FTable[0x65]=(unsigned int)OpcError;
	Ins0FTable[0x66]=(unsigned int)OpcError;
	Ins0FTable[0x67]=(unsigned int)OpcError;
	Ins0FTable[0x68]=(unsigned int)OpcError;
	Ins0FTable[0x69]=(unsigned int)OpcError;
	Ins0FTable[0x6A]=(unsigned int)OpcError;
	Ins0FTable[0x6B]=(unsigned int)OpcError;
	Ins0FTable[0x6C]=(unsigned int)OpcError;
	Ins0FTable[0x6D]=(unsigned int)OpcError;
	Ins0FTable[0x6E]=(unsigned int)OpcError;
	Ins0FTable[0x6F]=(unsigned int)OpcError;
	Ins0FTable[0x70]=(unsigned int)OpcError;
	Ins0FTable[0x71]=(unsigned int)OpcError;
	Ins0FTable[0x72]=(unsigned int)OpcError;
	Ins0FTable[0x73]=(unsigned int)OpcError;
	Ins0FTable[0x74]=(unsigned int)OpcError;
	Ins0FTable[0x75]=(unsigned int)OpcError;
	Ins0FTable[0x76]=(unsigned int)OpcError;
	Ins0FTable[0x77]=(unsigned int)OpcError;
	Ins0FTable[0x78]=(unsigned int)OpcError;
	Ins0FTable[0x79]=(unsigned int)OpcError;
	Ins0FTable[0x7A]=(unsigned int)OpcError;
	Ins0FTable[0x7B]=(unsigned int)OpcError;
	Ins0FTable[0x7C]=(unsigned int)OpcError;
	Ins0FTable[0x7D]=(unsigned int)OpcError;
	Ins0FTable[0x7E]=(unsigned int)OpcError;
	Ins0FTable[0x7F]=(unsigned int)OpcError;
	Ins0FTable[0x80]=(unsigned int)OpcError;
	Ins0FTable[0x81]=(unsigned int)OpcError;
	Ins0FTable[0x82]=(unsigned int)JC;
	Ins0FTable[0x83]=(unsigned int)OpcError;
	Ins0FTable[0x84]=(unsigned int)JZ;
	Ins0FTable[0x85]=(unsigned int)JNZ;
	Ins0FTable[0x86]=(unsigned int)OpcError;
	Ins0FTable[0x87]=(unsigned int)JA;
	Ins0FTable[0x88]=(unsigned int)OpcError;
	Ins0FTable[0x89]=(unsigned int)OpcError;
	Ins0FTable[0x8A]=(unsigned int)OpcError;
	Ins0FTable[0x8B]=(unsigned int)OpcError;
	Ins0FTable[0x8C]=(unsigned int)OpcError;
	Ins0FTable[0x8D]=(unsigned int)OpcError;
	Ins0FTable[0x8E]=(unsigned int)OpcError;
	Ins0FTable[0x8F]=(unsigned int)OpcError;
	Ins0FTable[0x90]=(unsigned int)OpcError;
	Ins0FTable[0x91]=(unsigned int)OpcError;
	Ins0FTable[0x92]=(unsigned int)OpcError;
	Ins0FTable[0x93]=(unsigned int)OpcError;
	Ins0FTable[0x94]=(unsigned int)OpcError;
	Ins0FTable[0x95]=(unsigned int)OpcError;
	Ins0FTable[0x96]=(unsigned int)OpcError;
	Ins0FTable[0x97]=(unsigned int)OpcError;
	Ins0FTable[0x98]=(unsigned int)OpcError;
	Ins0FTable[0x99]=(unsigned int)OpcError;
	Ins0FTable[0x9A]=(unsigned int)OpcError;
	Ins0FTable[0x9B]=(unsigned int)OpcError;
	Ins0FTable[0x9C]=(unsigned int)OpcError;
	Ins0FTable[0x9D]=(unsigned int)OpcError;
	Ins0FTable[0x9E]=(unsigned int)OpcError;
	Ins0FTable[0x9F]=(unsigned int)OpcError;
	Ins0FTable[0xA0]=(unsigned int)OpcError;
	Ins0FTable[0xA1]=(unsigned int)POP_FS;
	Ins0FTable[0xA2]=(unsigned int)CPUID;
	Ins0FTable[0xA3]=(unsigned int)OpcError;
	Ins0FTable[0xA4]=(unsigned int)OpcError;
	Ins0FTable[0xA5]=(unsigned int)OpcError;
	Ins0FTable[0xA6]=(unsigned int)OpcError;
	Ins0FTable[0xA7]=(unsigned int)OpcError;
	Ins0FTable[0xA8]=(unsigned int)OpcError;
	Ins0FTable[0xA9]=(unsigned int)OpcError;
	Ins0FTable[0xAA]=(unsigned int)OpcError;
	Ins0FTable[0xAB]=(unsigned int)OpcError;
	Ins0FTable[0xAC]=(unsigned int)OpcError;
	Ins0FTable[0xAD]=(unsigned int)OpcError;
	Ins0FTable[0xAE]=(unsigned int)OpcError;
	Ins0FTable[0xAF]=(unsigned int)OpcError;
	Ins0FTable[0xB0]=(unsigned int)OpcError;
	Ins0FTable[0xB1]=(unsigned int)OpcError;
	Ins0FTable[0xB2]=(unsigned int)OpcError;
	Ins0FTable[0xB3]=(unsigned int)OpcError;
	Ins0FTable[0xB4]=(unsigned int)OpcError;
	Ins0FTable[0xB5]=(unsigned int)OpcError;
	Ins0FTable[0xB6]=(unsigned int)MOVZX_RM8;
	Ins0FTable[0xB7]=(unsigned int)MOVZX_RM16;
	Ins0FTable[0xB8]=(unsigned int)OpcError;
	Ins0FTable[0xB9]=(unsigned int)OpcError;
	Ins0FTable[0xBA]=(unsigned int)OpcError;
	Ins0FTable[0xBB]=(unsigned int)OpcError;
	Ins0FTable[0xBC]=(unsigned int)OpcError;
	Ins0FTable[0xBD]=(unsigned int)OpcError;
	Ins0FTable[0xBE]=(unsigned int)OpcError;
	Ins0FTable[0xBF]=(unsigned int)OpcError;
	Ins0FTable[0xC0]=(unsigned int)OpcError;
	Ins0FTable[0xC1]=(unsigned int)OpcError;
	Ins0FTable[0xC2]=(unsigned int)OpcError;
	Ins0FTable[0xC3]=(unsigned int)OpcError;
	Ins0FTable[0xC4]=(unsigned int)OpcError;
	Ins0FTable[0xC5]=(unsigned int)OpcError;
	Ins0FTable[0xC6]=(unsigned int)OpcError;
	Ins0FTable[0xC7]=(unsigned int)OpcError;
	Ins0FTable[0xC8]=(unsigned int)OpcError;
	Ins0FTable[0xC9]=(unsigned int)OpcError;
	Ins0FTable[0xCA]=(unsigned int)OpcError;
	Ins0FTable[0xCB]=(unsigned int)OpcError;
	Ins0FTable[0xCC]=(unsigned int)OpcError;
	Ins0FTable[0xCD]=(unsigned int)OpcError;
	Ins0FTable[0xCE]=(unsigned int)OpcError;
	Ins0FTable[0xCF]=(unsigned int)OpcError;
	Ins0FTable[0xD0]=(unsigned int)OpcError;
	Ins0FTable[0xD1]=(unsigned int)OpcError;
	Ins0FTable[0xD2]=(unsigned int)OpcError;
	Ins0FTable[0xD3]=(unsigned int)OpcError;
	Ins0FTable[0xD4]=(unsigned int)OpcError;
	Ins0FTable[0xD5]=(unsigned int)OpcError;
	Ins0FTable[0xD6]=(unsigned int)OpcError;
	Ins0FTable[0xD7]=(unsigned int)OpcError;
	Ins0FTable[0xD8]=(unsigned int)OpcError;
	Ins0FTable[0xD9]=(unsigned int)OpcError;
	Ins0FTable[0xDA]=(unsigned int)OpcError;
	Ins0FTable[0xDB]=(unsigned int)OpcError;
	Ins0FTable[0xDC]=(unsigned int)OpcError;
	Ins0FTable[0xDD]=(unsigned int)OpcError;
	Ins0FTable[0xDE]=(unsigned int)OpcError;
	Ins0FTable[0xDF]=(unsigned int)OpcError;
	Ins0FTable[0xE0]=(unsigned int)OpcError;
	Ins0FTable[0xE1]=(unsigned int)OpcError;
	Ins0FTable[0xE2]=(unsigned int)OpcError;
	Ins0FTable[0xE3]=(unsigned int)OpcError;
	Ins0FTable[0xE4]=(unsigned int)OpcError;
	Ins0FTable[0xE5]=(unsigned int)OpcError;
	Ins0FTable[0xE6]=(unsigned int)OpcError;
	Ins0FTable[0xE7]=(unsigned int)OpcError;
	Ins0FTable[0xE8]=(unsigned int)OpcError;
	Ins0FTable[0xE9]=(unsigned int)OpcError;
	Ins0FTable[0xEA]=(unsigned int)OpcError;
	Ins0FTable[0xEB]=(unsigned int)OpcError;
	Ins0FTable[0xEC]=(unsigned int)OpcError;
	Ins0FTable[0xED]=(unsigned int)OpcError;
	Ins0FTable[0xEE]=(unsigned int)OpcError;
	Ins0FTable[0xEF]=(unsigned int)OpcError;
	Ins0FTable[0xF0]=(unsigned int)OpcError;
	Ins0FTable[0xF1]=(unsigned int)OpcError;
	Ins0FTable[0xF2]=(unsigned int)OpcError;
	Ins0FTable[0xF3]=(unsigned int)OpcError;
	Ins0FTable[0xF4]=(unsigned int)OpcError;
	Ins0FTable[0xF5]=(unsigned int)OpcError;
	Ins0FTable[0xF6]=(unsigned int)OpcError;
	Ins0FTable[0xF7]=(unsigned int)OpcError;
	Ins0FTable[0xF8]=(unsigned int)OpcError;
	Ins0FTable[0xF9]=(unsigned int)OpcError;
	Ins0FTable[0xFA]=(unsigned int)OpcError;
	Ins0FTable[0xFB]=(unsigned int)OpcError;
	Ins0FTable[0xFC]=(unsigned int)OpcError;
	Ins0FTable[0xFD]=(unsigned int)OpcError;
	Ins0FTable[0xFE]=(unsigned int)OpcError;
	Ins0FTable[0xFF]=(unsigned int)OpcError;
}
