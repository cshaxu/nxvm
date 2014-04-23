/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086 */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"
#include "ecpu/ecpu.h"

#define VCPU_DEBUG

#define VCPU_FLAG_CF 0x0001
#define VCPU_FLAG_PF 0x0004
#define VCPU_FLAG_AF 0x0010
#define VCPU_FLAG_ZF 0x0040
#define VCPU_FLAG_SF 0x0080
#define VCPU_FLAG_TF 0x0100
#define VCPU_FLAG_IF 0x0200
#define VCPU_FLAG_DF 0x0400
#define VCPU_FLAG_OF 0x0800

#define vcpu ecpu

void vcpuinsExecIns();
void vcpuinsExecInt();
void vcpuRefresh();
void vcpuInit();
void vcpuFinal();

#define _ax    (vcpu.ax)
#define _bx    (vcpu.bx)
#define _cx    (vcpu.cx)
#define _dx    (vcpu.dx)
#define _ah    (vcpu.ah)
#define _bh    (vcpu.bh)
#define _ch    (vcpu.ch)
#define _dh    (vcpu.dh)
#define _al    (vcpu.al)
#define _bl    (vcpu.bl)
#define _cl    (vcpu.cl)
#define _dl    (vcpu.dl)
#define _sp    (vcpu.sp)
#define _bp    (vcpu.bp)
#define _si    (vcpu.si)
#define _di    (vcpu.di)
#define _ip    (vcpu.ip)
#define _ds    (vcpu.ds)
#define _cs    (vcpu.cs)
#define _es    (vcpu.es)
#define _ss    (vcpu.ss)
#define _flags (vcpu.flags)
#define _of    (GetBit(_flags, VCPU_FLAG_OF))
#define _sf    (GetBit(_flags, VCPU_FLAG_SF))
#define _zf    (GetBit(_flags, VCPU_FLAG_ZF))
#define _cf    (GetBit(_flags, VCPU_FLAG_CF))
#define _af    (GetBit(_flags, VCPU_FLAG_AF))
#define _pf    (GetBit(_flags, VCPU_FLAG_PF))
#define _df    (GetBit(_flags, VCPU_FLAG_DF))
#define _tf    (GetBit(_flags, VCPU_FLAG_TF))
#define _if    (GetBit(_flags, VCPU_FLAG_IF))
#define SetOF  (SetBit(_flags, VCPU_FLAG_OF))
#define SetSF  (SetBit(_flags, VCPU_FLAG_SF))
#define SetZF  (SetBit(_flags, VCPU_FLAG_ZF))
#define SetCF  (SetBit(_flags, VCPU_FLAG_CF))
#define SetAF  (SetBit(_flags, VCPU_FLAG_AF))
#define SetPF  (SetBit(_flags, VCPU_FLAG_PF))
#define SetDF  (SetBit(_flags, VCPU_FLAG_DF))
#define SetTF  (SetBit(_flags, VCPU_FLAG_TF))
#define SetIF  (SetBit(_flags, VCPU_FLAG_IF))
#define ClrOF  (ClrBit(_flags, VCPU_FLAG_OF))
#define ClrSF  (ClrBit(_flags, VCPU_FLAG_SF))
#define ClrZF  (ClrBit(_flags, VCPU_FLAG_ZF))
#define ClrCF  (ClrBit(_flags, VCPU_FLAG_CF))
#define ClrAF  (ClrBit(_flags, VCPU_FLAG_AF))
#define ClrPF  (ClrBit(_flags, VCPU_FLAG_PF))
#define ClrDF  (ClrBit(_flags, VCPU_FLAG_DF))
#define ClrTF  (ClrBit(_flags, VCPU_FLAG_TF))
#define ClrIF  (ClrBit(_flags, VCPU_FLAG_IF))

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
