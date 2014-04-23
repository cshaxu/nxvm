#ifndef NXVM_ECPU_H
#define NXVM_ECPU_H

#include "../vram.h"
#include "ecpuins.h"

#define MemoryStart vram.base
extern t_vaddrcc evIP;					//CS:IP所表示的线性地址，只供内部使用
extern unsigned short tmpDs;				//ecpu.ds的一个副本。有时候会遇到ES之类的指令前缀，则修改这个副本。对内在的读写操作是以这个副本为准的，而不是ecpu.ds
extern unsigned short tmpSs;				//同上
extern void LongCallNewIP(char OffsetByte);	//LongCall的时候，把当前IP修改成下一条指令的IP，以便压栈
extern unsigned short *rm16;			//解释寻址字节的时候用
extern unsigned char *rm8;

typedef struct {
	__int64 Fraction;
	short Exponent;
} ExtendedReal;
typedef struct {
	union
	{
		unsigned char mmxi8[8];
		unsigned short mmxi16[4];
		unsigned int mmxi32[2];
		unsigned __int64 mmxi64;
	};
} MMXI;
typedef struct {
	double mmxf;
} MMXF;
typedef struct {
	union
	{
		ExtendedReal er;
		MMXI mi;
		MMXF mf;
	};
} ExtendedReg ;
typedef struct {
	union
	{
		union
		{
			struct  
			{
				unsigned char al,ah;
			};
			unsigned short ax;
		};
		unsigned int eax;
	};
	union
	{
		union
		{
			struct  
			{
				unsigned char cl,ch;
			};
			unsigned short cx;
		};
		unsigned int ecx;
	};
	union
	{
		union
		{
			struct  
			{
				unsigned char dl,dh;
			};
			unsigned short dx;
		};
		unsigned int edx;
	};
	union
	{
		union
		{
			struct  
			{
				unsigned char bl,bh;
			};
			unsigned short bx;
		};
		unsigned int ebx;
	};
	union
	{
		unsigned short sp;
		unsigned int esp;
	};	
	union
	{
		unsigned short bp;
		unsigned int ebp;
	};	
	union
	{
		unsigned short si;
		unsigned int esi;
	};	
	union
	{
		unsigned short di;
		unsigned int edi;
	};	
	union
	{
		unsigned short ip;
		unsigned int eip;
	};	
	union
	{
		unsigned short flags;
		unsigned int eflags;
	};
	unsigned short cs;
	unsigned short ds;
	unsigned short es;
	unsigned short ss;
	unsigned short fs;
	unsigned short gs;
//////////////////////////////////////////////////////////////////////////
	unsigned int CR[4];
	unsigned int DR[8];
//////////////////////////////////////////////////////////////////////////	
	ExtendedReg FpuR[8];
	unsigned short FpuCR;
	unsigned short FpuSR;
	unsigned short FpuTR;
	struct FpuIP 
	{
		unsigned short seg;
		unsigned int off;
	}Fpuip;
	struct FpuDP						//这里Instruction Pointer和Operand Pointer可以用同一个结构体，也可以像这样把结构体的定义写两遍
	{
		unsigned short seg;
		unsigned int off;
	}Fpudp;
	unsigned short FpuOpcode;			//实际上Opcode是11个bit的。
//////////////////////////////////////////////////////////////////////////
	struct  
	{
		union
		{
			float fp[4];
			unsigned int ip[4];
		};
	}xmm[8];
	t_bool flagignore;
	t_nubit8 intrid;
	t_bool flagnmi;
	t_nubit16 overss,overds;
} t_ecpu;

extern t_ecpu ecpu;

void ecpuapiSyncRegs();
t_bool ecpuapiHasDiff();
t_bool ecpuapiScanINTR();
t_nubit8 ecpuapiGetINTR();
void ecpuapiPrintRegs();

void ecpuRefresh();
void ecpuInit();
void ecpuFinal();

#endif
