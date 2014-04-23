#ifndef NXVM_ECPU_H
#define NXVM_ECPU_H

#include "../vglobal.h"
#include "../vcpu.h"

#if VGLOBAL_ECPU_MODE == TEST_ECPU
#define _eax    (ecpu.eax)
#define _ebx    (ecpu.ebx)
#define _ecx    (ecpu.ecx)
#define _edx    (ecpu.edx)
#define _esp    (ecpu.esp)
#define _ebp    (ecpu.ebp)
#define _esi    (ecpu.esi)
#define _edi    (ecpu.edi)
#define _eip    (ecpu.eip)
#define _eflags (ecpu.eflags)
#define _ax     (ecpu.ax)
#define _bx     (ecpu.bx)
#define _cx     (ecpu.cx)
#define _dx     (ecpu.dx)
#define _ah     (ecpu.ah)
#define _bh     (ecpu.bh)
#define _ch     (ecpu.ch)
#define _dh     (ecpu.dh)
#define _al     (ecpu.al)
#define _bl     (ecpu.bl)
#define _cl     (ecpu.cl)
#define _dl     (ecpu.dl)
#define _sp     (ecpu.sp)
#define _bp     (ecpu.bp)
#define _si     (ecpu.si)
#define _di     (ecpu.di)
#define _ip     (ecpu.ip)
#define _flags  (ecpu.flags)
#define _ds     (ecpu.ds.selector)
#define _cs     (ecpu.cs.selector)
#define _ss     (ecpu.ss.selector)
#define _ds     (ecpu.ds.selector)
#define _es     (ecpu.es.selector)
#define _fs     (ecpu.fs.selector)
#define _gs     (ecpu.gs.selector)
#endif

extern t_vaddrcc evIP;					//CS:IP所表示的线性地址，只供内部使用

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
	t_cpu_sreg cs,ds,ss,es,fs,gs;
	/*unsigned short cs;
	unsigned short ds;
	unsigned short es;
	unsigned short ss;
	unsigned short fs;
	unsigned short gs;*/
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

#if VGLOBAL_ECPU_MODE == TEST_ECPU
#define _GetEFLAGS_CF    (GetBit(ecpu.eflags, VCPU_EFLAGS_CF))
#define _GetEFLAGS_PF    (GetBit(ecpu.eflags, VCPU_EFLAGS_PF))
#define _GetEFLAGS_AF    (GetBit(ecpu.eflags, VCPU_EFLAGS_AF))
#define _GetEFLAGS_ZF    (GetBit(ecpu.eflags, VCPU_EFLAGS_ZF))
#define _GetEFLAGS_SF    (GetBit(ecpu.eflags, VCPU_EFLAGS_SF))
#define _GetEFLAGS_TF    (GetBit(ecpu.eflags, VCPU_EFLAGS_TF))
#define _GetEFLAGS_IF    (GetBit(ecpu.eflags, VCPU_EFLAGS_IF))
#define _GetEFLAGS_DF    (GetBit(ecpu.eflags, VCPU_EFLAGS_DF))
#define _GetEFLAGS_OF    (GetBit(ecpu.eflags, VCPU_EFLAGS_OF))
#define _GetEFLAGS_IOPLL (GetBit(ecpu.eflags, VCPU_EFLAGS_IOPLL))
#define _GetEFLAGS_IOPLH (GetBit(ecpu.eflags, VCPU_EFLAGS_IOPLH))
#define _GetEFLAGS_IOPL  ((ecpu.eflags & VCPU_EFLAGS_IOPL) >> 12)
#define _GetEFLAGS_NT    (GetBit(ecpu.eflags, VCPU_EFLAGS_NT))
#define _GetEFLAGS_RF    (GetBit(ecpu.eflags, VCPU_EFLAGS_RF))
#define _GetEFLAGS_VM    (GetBit(ecpu.eflags, VCPU_EFLAGS_VM))
/*
#define _GetEFLAGS_AC    (GetBit(ecpu.eflags, VCPU_EFLAGS_AC))
#define _GetEFLAGS_VIF   (GetBit(ecpu.eflags, VCPU_EFLAGS_VIF))
#define _GetEFLAGS_VIP   (GetBit(ecpu.eflags, VCPU_EFLAGS_VIP))
#define _GetEFLAGS_ID    (GetBit(ecpu.eflags, VCPU_EFLAGS_ID))*/
#define _SetEFLAGS_CF    (SetBit(ecpu.eflags, VCPU_EFLAGS_CF))
#define _SetEFLAGS_PF    (SetBit(ecpu.eflags, VCPU_EFLAGS_PF))
#define _SetEFLAGS_AF    (SetBit(ecpu.eflags, VCPU_EFLAGS_AF))
#define _SetEFLAGS_ZF    (SetBit(ecpu.eflags, VCPU_EFLAGS_ZF))
#define _SetEFLAGS_SF    (SetBit(ecpu.eflags, VCPU_EFLAGS_SF))
#define _SetEFLAGS_TF    (SetBit(ecpu.eflags, VCPU_EFLAGS_TF))
#define _SetEFLAGS_IF    (SetBit(ecpu.eflags, VCPU_EFLAGS_IF))
#define _SetEFLAGS_DF    (SetBit(ecpu.eflags, VCPU_EFLAGS_DF))
#define _SetEFLAGS_OF    (SetBit(ecpu.eflags, VCPU_EFLAGS_OF))
#define _SetEFLAGS_IOPLL (SetBit(ecpu.eflags, VCPU_EFLAGS_IOPLL))
#define _SetEFLAGS_IOPLH (SetBit(ecpu.eflags, VCPU_EFLAGS_IOPLH))
#define _SetEFLAGS_IOPL  (SetBit(ecpu.eflags, VCPU_EFLAGS_IOPL)
#define _SetEFLAGS_NT    (SetBit(ecpu.eflags, VCPU_EFLAGS_NT))
#define _SetEFLAGS_RF    (SetBit(ecpu.eflags, VCPU_EFLAGS_RF))
#define _SetEFLAGS_VM    (SetBit(ecpu.eflags, VCPU_EFLAGS_VM))
/*
#define _SetEFLAGS_AC    (SetBit(ecpu.eflags, VCPU_EFLAGS_AC))
#define _SetEFLAGS_VIF   (SetBit(ecpu.eflags, VCPU_EFLAGS_VIF))
#define _SetEFLAGS_VIP   (SetBit(ecpu.eflags, VCPU_EFLAGS_VIP))
#define _SetEFLAGS_ID    (SetBit(ecpu.eflags, VCPU_EFLAGS_ID))*/
#define _ClrEFLAGS_CF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_CF))
#define _ClrEFLAGS_PF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_PF))
#define _ClrEFLAGS_AF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_AF))
#define _ClrEFLAGS_ZF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_ZF))
#define _ClrEFLAGS_SF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_SF))
#define _ClrEFLAGS_TF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_TF))
#define _ClrEFLAGS_IF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_IF))
#define _ClrEFLAGS_DF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_DF))
#define _ClrEFLAGS_OF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_OF))
#define _ClrEFLAGS_IOPLL (ClrBit(ecpu.eflags, VCPU_EFLAGS_IOPLL))
#define _ClrEFLAGS_IOPLH (ClrBit(ecpu.eflags, VCPU_EFLAGS_IOPLH))
#define _ClrEFLAGS_IOPL  (ClrBit(ecpu.eflags, VCPU_EFLAGS_IOPL)
#define _ClrEFLAGS_NT    (ClrBit(ecpu.eflags, VCPU_EFLAGS_NT))
#define _ClrEFLAGS_RF    (ClrBit(ecpu.eflags, VCPU_EFLAGS_RF))
#define _ClrEFLAGS_VM    (ClrBit(ecpu.eflags, VCPU_EFLAGS_VM))
/*
#define _ClrEFLAGS_AC    (ClrBit(ecpu.eflags, VCPU_EFLAGS_AC))
#define _ClrEFLAGS_VIF   (ClrBit(ecpu.eflags, VCPU_EFLAGS_VIF))
#define _ClrEFLAGS_VIP   (ClrBit(ecpu.eflags, VCPU_EFLAGS_VIP))
#define _ClrEFLAGS_ID    (ClrBit(ecpu.eflags, VCPU_EFLAGS_ID))*/
#endif

void ecpuInit();
void ecpuReset();
void ecpuRefreshInit();
void ecpuRefresh();
void ecpuRefreshFinal();
void ecpuFinal();

#endif
