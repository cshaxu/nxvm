#ifndef NXVM_ECPU_H
#define NXVM_ECPU_H

#include "../vglobal.h"
#include "../vcpu.h"

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
	union {
		union {
			struct {t_nubit8 al,ah;};
			t_nubit16 ax;
		};
		t_nubit32 eax;
	};
	union {
		union {
			struct {t_nubit8 bl,bh;};
			t_nubit16 bx;
		};
		t_nubit32 ebx;
	};
	union {
		union {
			struct {t_nubit8 cl,ch;};
			t_nubit16 cx;
		};
		t_nubit32 ecx;
	};
	union {
		union {
			struct {t_nubit8 dl,dh;};
			t_nubit16 dx;
		};
		t_nubit32 edx;
	};
	union {
		t_nubit16 sp;
		t_nubit32 esp;
	};
	union {
		t_nubit16 bp;
		t_nubit32 ebp;
	};
	union {
		t_nubit16 si;
		t_nubit32 esi;
	};
	union {
		t_nubit16 di;
		t_nubit32 edi;
	};
	union {
		t_nubit16 ip;
		t_nubit32 eip;
	};
	union {
		t_nubit16 flags;
		t_nubit32 eflags;
	};
	t_cpu_sreg cs, ss, ds, es, fs, gs, ldtr, tr;
	t_nubit48 gdtr, idtr;
	t_nubit32 ldtrcr, trcr;
	t_nubit32 cr0, cr1, cr2, cr3;
	t_nubit32 dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7;
	t_nubit32 tr6, tr7;
	t_bool flagmasknmi;
	t_bool flagnmi, flaglock, flaghalt;
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
	t_nubit16 overss,overds;
} t_ecpu;

extern t_ecpu ecpu;

void ecpuInit();
void ecpuReset();
void ecpuRefreshInit();
void ecpuRefresh();
void ecpuRefreshFinal();
void ecpuFinal();

#endif
