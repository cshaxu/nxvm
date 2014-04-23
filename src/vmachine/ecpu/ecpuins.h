
#ifndef NXVM_ECPUINS_H
#define NXVM_ECPUINS_H

#include "../vport.h"

#define InTable vport.in
#define OutTable vport.out

extern t_faddrcc InsTable[0x100];
//extern t_vaddrcc evIP;	//evIP永远指向将要读的那个字节

// Eflags寄存器
#define CF 0x0001
#define PF 0x0004
#define AF 0x0010
#define ZF 0x0040
#define SF 0x0080
#define TF 0x0100
#define IF 0x0200
#define DF 0x0400
#define OF 0x0800

#define IOPL 0x3000
#define NT 0x4000
#define RF 0x10000
#define VM 0x20000
#define AC 0x40000
#define VIF 0x80000
#define VIP 0x100000
#define ID 0x200000

// 异常
#define IntDE 0x00001
#define IntDB 0x00002
#define IntBP 0x00008
#define IntOF 0x00010
#define IntBR 0x00020
#define IntUD 0x00040
#define IntNM 0x00080
#define IntDF 0x00100
#define IntTS 0x00400
#define IntNP 0x00800
#define IntSS 0x01000
#define IntGP 0x02000
#define IntPF 0x04000
#define IntMF 0x10000
#define IntAC 0x20000
#define IntMC 0x40000
#define IntXF 0x80000

void ecpuinsExecIns();
void ecpuinsExecInt();
void ecpuinsInit();
void ecpuinsFinal();

#endif
