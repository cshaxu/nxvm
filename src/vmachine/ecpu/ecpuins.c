/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"

#include "../vapi.h"

#include "../vport.h"
#include "../vram.h"
#include "../vcpu.h"
#include "../vcpuins.h"
#include "../vpic.h"
#include "../bios/qdbios.h"

#include "ecpu.h"
#include "ecpuapi.h"
#include "ecpuins.h"

#include "../debug/aasm.h"
#include "../debug/dasm.h"

#define MOD ((modrm&0xc0)>>6)
#define REG ((modrm&0x38)>>3)
#define RM  ((modrm&0x07)>>0)

#if VGLOBAL_ECPU_MODE == TEST_ECPU
#define wm 1 /* 1 = allowed to write mem, 0 = not allowed */
#else
#define wm 0
#endif

/* stack pointer size */
#define _GetStackSize   (ecpu.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((ecpu.cs.seg.exec.defsize ^ ecpuins.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((ecpu.cs.seg.exec.defsize ^ ecpuins.prefix_addrsize) ? 4 : 2)

#define ECPUINS_FLAGS_MASKED (VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_VM)

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

t_cpuins ecpuins;

#define bugfix(n) if (1)

#define NOTIMP static void
#define DONEASM static void

static t_nubit32 efres;
static t_nubit8  uba,ubb,ubc;
static t_nsbit8  sba,sbb,sbc;
static t_nubit16 uwa,uwb,uwc;
static t_nsbit16 swa,swb,swc;
static t_nubit32 uda,udb,udc;
static t_nsbit32 sda,sdb,sdc;

#if ((VGLOBAL_SIZE_INTEGER == 32) && (VGLOBAL_PLATFORM == VGLOBAL_VAR_WIN32))

#define arith1                                        \
do {                                                  \
	switch (bit) {                                     \
	case 8:                                           \
		uba = d_nubit8(rdest);                        \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  al, uba\
		__asm operation al\
		__asm mov  ubc, al\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit8(rdest) = ubc;  \
		break;                                        \
	case 16:                                          \
		uwa = d_nubit16(rdest);                       \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  ax, uwa\
		__asm operation ax\
		__asm mov  uwc, ax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit16(rdest) = uwc; \
		break;                                        \
	case 32:                                          \
		uda = d_nubit32(rdest);                       \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  eax, uda\
		__asm operation eax\
		__asm mov  udc, eax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit32(rdest) = udc; \
		break;                                        \
	default:CaseError("arith1::bit");break;}          \
	ecpu.eflags = (efres & ~ECPUINS_FLAGS_MASKED) | (ecpu.eflags & ECPUINS_FLAGS_MASKED); \
} while (0)

#define arith2                                     \
do {                                                  \
	switch (bit) {                                     \
	case 8:                                           \
		uba = d_nubit8(rdest);                        \
		ubb = d_nubit8(rsrc);                         \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  al, uba\
		__asm operation al, ubb\
		__asm mov  ubc, al\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit8(rdest) = ubc;  \
		break;                                        \
	case 12:                                          \
		uwa = d_nubit16(rdest);                       \
		uwb = d_nsbit8(rsrc);                         \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  ax, uwa\
		__asm operation  ax, uwb\
		__asm mov  uwc, ax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit16(rdest) = uwc; \
		break;                                        \
	case 16:                                          \
		uwa = d_nubit16(rdest);                       \
		uwb = d_nubit16(rsrc);                        \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  ax, uwa\
		__asm operation  ax, uwb\
		__asm mov  uwc, ax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit16(rdest) = uwc; \
		break;                                        \
	case 20:                                          \
		uda = d_nubit32(rdest);                       \
		udb = d_nsbit8(rsrc);                         \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  eax, uda\
		__asm operation  eax, udb\
		__asm mov  udc, eax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit32(rdest) = udc; \
		break;                                        \
	case 32:                                          \
		uda = d_nubit32(rdest);                       \
		udb = d_nubit32(rsrc);                        \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  eax, uda\
		__asm operation  eax, udb\
		__asm mov  udc, eax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit32(rdest) = udc; \
		break;                                        \
	default:CaseError("arith2::bit");break;}          \
	ecpu.eflags = (efres & ~ECPUINS_FLAGS_MASKED) | (ecpu.eflags & ECPUINS_FLAGS_MASKED); \
} while (0)

#define shift \
do { \
	if (rsrc) count = d_nubit8(rsrc); \
	else count = 1; \
	switch (bit) { \
	case 8: \
		uba = d_nubit8(rdest);                        \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecx\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  al, uba\
		__asm mov  cl, count\
		__asm operation al, cl\
		__asm mov  ubc, al\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  ecx\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit8(rdest) = ubc;  \
		break;                                        \
	case 16:                                          \
		uwa = d_nubit16(rdest);                       \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecx\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  ax, uwa\
		__asm mov  cl, count\
		__asm operation ax, cl\
		__asm mov  uwc, ax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  ecx\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit16(rdest) = uwc; \
		break;                                        \
	case 32:                                          \
		uda = d_nubit32(rdest);                       \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecx\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  eax, uda\
		__asm mov  cl, count\
		__asm operation eax, cl\
		__asm mov  udc, eax\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  ecx\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		if (!im(rdest) || wm) d_nubit32(rdest) = udc; \
		break;                                        \
	default:CaseError("shift::bit");break;}           \
	ecpu.eflags = (efres & ~ECPUINS_FLAGS_MASKED) | (ecpu.eflags & ECPUINS_FLAGS_MASKED); \
} while (0);

#define arithax \
do { \
	switch (bit) { \
	case 8: \
		uba = d_nubit8(rsrc);                        \
		ubb = ecpu.ah;                               \
		ubc = ecpu.al;                               \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  ah, ubb\
		__asm mov  al, ubc\
		__asm operation uba\
		__asm mov  ubc, al\
		__asm mov  ubb, ah\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		ecpu.al = ubc;                                \
		ecpu.ah = ubb;                                \
		break;                                        \
	case 16:                                          \
		uwa = d_nubit16(rsrc);                        \
		uwb = ecpu.dx;                                \
		uwc = ecpu.ax;                                \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push edx\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  dx, uwb\
		__asm mov  ax, uwc\
		__asm operation uwa\
		__asm mov  uwc, ax\
		__asm mov  uwb, dx\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  edx\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		ecpu.ax = uwc;                                \
		ecpu.dx = uwb;                                \
		break;                                        \
	case 32:                                          \
		uda = d_nubit32(rsrc);                        \
		udb = ecpu.edx;                               \
		udc = ecpu.eax;                               \
		do { \
		__asm pushfd\
		__asm push eax\
		__asm push edx\
		__asm push ecpu.eflags\
		__asm popfd\
		__asm mov  edx, udb\
		__asm mov  eax, udc\
		__asm operation uda\
		__asm mov  udc, eax\
		__asm mov  udb, edx\
		__asm pushfd\
		__asm pop  efres\
		__asm pop  edx\
		__asm pop  eax\
		__asm popfd\
		} while (0); \
		ecpu.eax = udc;                               \
		ecpu.edx = udb;                               \
		break;                                        \
	default:CaseError("arithax::bit");break;}         \
	ecpu.eflags = (efres & ~ECPUINS_FLAGS_MASKED) | (ecpu.eflags & ECPUINS_FLAGS_MASKED); \
} while (0)

#define adjustax \
do { \
	uda = ecpu.eax;\
	udb = ecpu.edx;\
	do { \
	__asm pushfd\
	__asm push eax\
	__asm push edx\
	__asm push ecpu.eflags\
	__asm popfd\
	__asm mov  eax, uda\
	__asm mov  edx, udb\
	__asm operation\
	__asm mov  udb, edx\
	__asm mov  uda, eax\
	__asm pushfd\
	__asm pop  efres\
	__asm pop edx\
	__asm pop  eax\
	__asm popfd\
	} while (0); \
	ecpu.edx = udb;\
	ecpu.eax = uda;\
	ecpu.eflags = (efres & ~ECPUINS_FLAGS_MASKED) | (ecpu.eflags & ECPUINS_FLAGS_MASKED); \
} while (0)

#else

#define arith1
#define arith2
#define shift
#define arithax
#define adjustax

#endif

static void CaseError(const char *str)
{
	vapiPrint("The NXVM ECPU has encountered an internal case error: %s.\n",str);
	vapiCallBackMachineStop();
}

#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)      (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib)   (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)    (((sib) & 0x07) >> 0)

static void ParseModRM(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit32 sibindex = 0x00000000;
	t_nubit8 modrm = 0x00, sib = 0x00;
	modrm = vramRealByte(ecpu.cs.selector, ecpu.eip++);
	ecpuins.erm = 0x00000000;
	ecpuins.flagmem = 1;
	ecpuins.flagmss = 0;
	ecpuins.crm = ecpuins.cr = 0x0000000000000000;
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	ecpuins.prm = 0x00000000;
	ecpuins.lrm = 0x00000000;
	switch (_GetAddressSize) {
	case 2:
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = GetMax16(ecpu.bx + ecpu.si); ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = GetMax16(ecpu.bx + ecpu.di); ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = GetMax16(ecpu.bp + ecpu.si); ecpuins.flagmss = 1;break;
			case 3: ecpuins.erm = GetMax16(ecpu.bp + ecpu.di); ecpuins.flagmss = 1;break;
			case 4: ecpuins.erm = GetMax16(ecpu.si); ecpuins.flagmss = 0;break;
			case 5: ecpuins.erm = GetMax16(ecpu.di); ecpuins.flagmss = 0;break;
			case 6:
				disp16 = (t_nubit16)vramRealWord(ecpu.cs.selector, ecpu.eip);
				ecpu.eip += 2;
				ecpuins.erm = GetMax16(disp16); ecpuins.flagmss = 0;
				break;
			case 7: ecpuins.erm = GetMax16(ecpu.bx); ecpuins.flagmss = 0;break;
			}
			break;
		case 1:
			disp8 = (t_nsbit8)vramRealByte(ecpu.cs.selector, ecpu.eip);
			ecpu.eip += 1;
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = GetMax16(ecpu.bx + ecpu.si + disp8); ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = GetMax16(ecpu.bx + ecpu.di + disp8); ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = GetMax16(ecpu.bp + ecpu.si + disp8); ecpuins.flagmss = 1;break;
			case 3: ecpuins.erm = GetMax16(ecpu.bp + ecpu.di + disp8); ecpuins.flagmss = 1;break;
			case 4: ecpuins.erm = GetMax16(ecpu.si + disp8); ecpuins.flagmss = 0;break;
			case 5: ecpuins.erm = GetMax16(ecpu.di + disp8); ecpuins.flagmss = 0;break;
			case 6: ecpuins.erm = GetMax16(ecpu.bp + disp8); ecpuins.flagmss = 1;break;
			case 7: ecpuins.erm = GetMax16(ecpu.bx + disp8); ecpuins.flagmss = 0;break;
			}
			break;
		case 2:
			disp16 = (t_nubit16)vramRealWord(ecpu.cs.selector, ecpu.eip);
			ecpu.eip += 2;
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = GetMax16(ecpu.bx + ecpu.si + disp16); ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = GetMax16(ecpu.bx + ecpu.di + disp16); ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = GetMax16(ecpu.bp + ecpu.si + disp16); ecpuins.flagmss = 1;break;
			case 3: ecpuins.erm = GetMax16(ecpu.bp + ecpu.di + disp16); ecpuins.flagmss = 1;break;
			case 4: ecpuins.erm = GetMax16(ecpu.si + disp16); ecpuins.flagmss = 0;break;
			case 5: ecpuins.erm = GetMax16(ecpu.di + disp16); ecpuins.flagmss = 0;break;
			case 6: ecpuins.erm = GetMax16(ecpu.bp + disp16); ecpuins.flagmss = 1;break;
			case 7: ecpuins.erm = GetMax16(ecpu.bx + disp16); ecpuins.flagmss = 0;break;
			}
			break;
		case 3:
			break;
		}
		break;
	case 4:
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			sib = (t_nubit8)vramRealByte(ecpu.cs.selector, ecpu.eip);
			ecpu.eip += 1;
			switch (_GetSIB_Index(sib)) {
			case 0: sibindex = ecpu.eax;   break;
			case 1: sibindex = ecpu.ecx;   break;
			case 2: sibindex = ecpu.edx;   break;
			case 3: sibindex = ecpu.ebx;   break;
			case 4: sibindex = 0x00000000;break;
			case 5: sibindex = ecpu.ebp;   break;
			case 6: sibindex = ecpu.esi;   break;
			case 7: sibindex = ecpu.edi;   break;
			}
			sibindex <<= _GetSIB_SS(sib);
		}
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = ecpu.eax; ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = ecpu.ecx; ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = ecpu.edx; ecpuins.flagmss = 0;break;
			case 3: ecpuins.erm = ecpu.ebx; ecpuins.flagmss = 0;break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: ecpuins.erm = ecpu.eax + sibindex; ecpuins.flagmss = 0;break;
				case 1: ecpuins.erm = ecpu.ecx + sibindex; ecpuins.flagmss = 0;break;
				case 2: ecpuins.erm = ecpu.edx + sibindex; ecpuins.flagmss = 0;break;
				case 3: ecpuins.erm = ecpu.ebx + sibindex; ecpuins.flagmss = 0;break;
				case 4: ecpuins.erm = ecpu.esp + sibindex; ecpuins.flagmss = 1;break;
				case 5:
					disp32 = (t_nubit32)vramRealDWord(ecpu.cs.selector, ecpu.eip);
					ecpu.eip += 4;
					ecpuins.erm = disp32 + sibindex; ecpuins.flagmss = 0; 
					break;
				case 6: ecpuins.erm = ecpu.esi + sibindex; ecpuins.flagmss = 0;break;
				case 7: ecpuins.erm = ecpu.edi + sibindex; ecpuins.flagmss = 0;break;
				}
				break;
			case 5:
				disp32 = (t_nubit32)vramRealDWord(ecpu.cs.selector, ecpu.eip);
				ecpu.eip += 4;
				ecpuins.erm = disp32; ecpuins.flagmss = 0;
				break;
			case 6: ecpuins.erm = ecpu.esi; ecpuins.flagmss = 0;
			case 7: ecpuins.erm = ecpu.edi; ecpuins.flagmss = 0;
			}
			break;
		case 1:
			disp8 = (t_nsbit8)vramRealByte(ecpu.cs.selector, ecpu.eip);
			ecpu.eip += 1;
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = ecpu.eax + disp8; ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = ecpu.ecx + disp8; ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = ecpu.edx + disp8; ecpuins.flagmss = 0;break;
			case 3: ecpuins.erm = ecpu.ebx + disp8; ecpuins.flagmss = 0;break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: ecpuins.erm = ecpu.eax + sibindex + disp8; ecpuins.flagmss = 0;break;
				case 1: ecpuins.erm = ecpu.ecx + sibindex + disp8; ecpuins.flagmss = 0;break;
				case 2: ecpuins.erm = ecpu.edx + sibindex + disp8; ecpuins.flagmss = 0;break;
				case 3: ecpuins.erm = ecpu.ebx + sibindex + disp8; ecpuins.flagmss = 0;break;
				case 4: ecpuins.erm = ecpu.esp + sibindex + disp8; ecpuins.flagmss = 1;break;
				case 5: ecpuins.erm = ecpu.ebp + sibindex + disp8; ecpuins.flagmss = 1;break;
				case 6: ecpuins.erm = ecpu.esi + sibindex + disp8; ecpuins.flagmss = 0;break;
				case 7: ecpuins.erm = ecpu.edi + sibindex + disp8; ecpuins.flagmss = 0;break;
				}
				break;
			case 5: ecpuins.erm = ecpu.ebp + disp8; ecpuins.flagmss = 1;break;
			case 6: ecpuins.erm = ecpu.esi + disp8; ecpuins.flagmss = 0;break;
			case 7: ecpuins.erm = ecpu.edi + disp8; ecpuins.flagmss = 0;break;
			}
			break;
		case 2:
			disp32 = (t_nubit32)vramRealDWord(ecpu.cs.selector, ecpu.eip);
			ecpu.eip += 4;
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.erm = ecpu.eax + disp32; ecpuins.flagmss = 0;break;
			case 1: ecpuins.erm = ecpu.ecx + disp32; ecpuins.flagmss = 0;break;
			case 2: ecpuins.erm = ecpu.edx + disp32; ecpuins.flagmss = 0;break;
			case 3: ecpuins.erm = ecpu.ebx + disp32; ecpuins.flagmss = 0;break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: ecpuins.erm = ecpu.eax + sibindex + disp32; ecpuins.flagmss = 0;break;
				case 1: ecpuins.erm = ecpu.ecx + sibindex + disp32; ecpuins.flagmss = 0;break;
				case 2: ecpuins.erm = ecpu.edx + sibindex + disp32; ecpuins.flagmss = 0;break;
				case 3: ecpuins.erm = ecpu.ebx + sibindex + disp32; ecpuins.flagmss = 0;break;
				case 4: ecpuins.erm = ecpu.esp + sibindex + disp32; ecpuins.flagmss = 1;break;
				case 5: ecpuins.erm = ecpu.ebp + sibindex + disp32; ecpuins.flagmss = 1;break;
				case 6: ecpuins.erm = ecpu.esi + sibindex + disp32; ecpuins.flagmss = 0;break;
				case 7: ecpuins.erm = ecpu.edi + sibindex + disp32; ecpuins.flagmss = 0;break;
				}
				break;
			case 5: ecpuins.erm = ecpu.ebp + disp32; ecpuins.flagmss = 1;break;
			case 6: ecpuins.erm = ecpu.esi + disp32; ecpuins.flagmss = 0;break;
			case 7: ecpuins.erm = ecpu.edi + disp32; ecpuins.flagmss = 0;break;
			}
			break;
		case 3:
			break;
		}
		break;
	}
	if (_GetModRM_MOD(modrm) == 3) {
		ecpuins.flagmem = 0;
		switch (rmbyte) {
		case 1:
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.rrm = (t_vaddrcc)(&ecpu.al);break;
			case 1: ecpuins.rrm = (t_vaddrcc)(&ecpu.cl);break;
			case 2: ecpuins.rrm = (t_vaddrcc)(&ecpu.dl);break;
			case 3: ecpuins.rrm = (t_vaddrcc)(&ecpu.bl);break;
			case 4: ecpuins.rrm = (t_vaddrcc)(&ecpu.ah);break;
			case 5: ecpuins.rrm = (t_vaddrcc)(&ecpu.ch);break;
			case 6: ecpuins.rrm = (t_vaddrcc)(&ecpu.dh);break;
			case 7: ecpuins.rrm = (t_vaddrcc)(&ecpu.bh);break;
			}
			ecpuins.crm = d_nubit8(ecpuins.rrm);
			break;
		case 2:
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.rrm = (t_vaddrcc)(&ecpu.ax);break;
			case 1: ecpuins.rrm = (t_vaddrcc)(&ecpu.cx);break;
			case 2: ecpuins.rrm = (t_vaddrcc)(&ecpu.dx);break;
			case 3: ecpuins.rrm = (t_vaddrcc)(&ecpu.bx);break;
			case 4: ecpuins.rrm = (t_vaddrcc)(&ecpu.sp);break;
			case 5: ecpuins.rrm = (t_vaddrcc)(&ecpu.bp);break;
			case 6: ecpuins.rrm = (t_vaddrcc)(&ecpu.si);break;
			case 7: ecpuins.rrm = (t_vaddrcc)(&ecpu.di);break;
			}
			ecpuins.crm = d_nubit16(ecpuins.rrm);
			break;
		case 4:
			switch (_GetModRM_RM(modrm)) {
			case 0: ecpuins.rrm = (t_vaddrcc)(&ecpu.eax);break;
			case 1: ecpuins.rrm = (t_vaddrcc)(&ecpu.ecx);break;
			case 2: ecpuins.rrm = (t_vaddrcc)(&ecpu.edx);break;
			case 3: ecpuins.rrm = (t_vaddrcc)(&ecpu.ebx);break;
			case 4: ecpuins.rrm = (t_vaddrcc)(&ecpu.esp);break;
			case 5: ecpuins.rrm = (t_vaddrcc)(&ecpu.ebp);break;
			case 6: ecpuins.rrm = (t_vaddrcc)(&ecpu.esi);break;
			case 7: ecpuins.rrm = (t_vaddrcc)(&ecpu.edi);break;
			}
			ecpuins.crm = d_nubit32(ecpuins.rrm);
			break;
		default: CaseError("ParseModRM::rmbyte");break;}
	}
	switch (regbyte) {
	case 0:
		/* reg is operation or segment */
		ecpuins.cr = _GetModRM_REG(modrm);
		break;
	case 1:
		switch (_GetModRM_REG(modrm)) {
		case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.al);break;
		case 1: ecpuins.rr = (t_vaddrcc)(&ecpu.cl);break;
		case 2: ecpuins.rr = (t_vaddrcc)(&ecpu.dl);break;
		case 3: ecpuins.rr = (t_vaddrcc)(&ecpu.bl);break;
		case 4: ecpuins.rr = (t_vaddrcc)(&ecpu.ah);break;
		case 5: ecpuins.rr = (t_vaddrcc)(&ecpu.ch);break;
		case 6: ecpuins.rr = (t_vaddrcc)(&ecpu.dh);break;
		case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.bh);break;
		}
		ecpuins.cr = d_nubit8(ecpuins.rr);
		break;
	case 2:
		switch (_GetModRM_REG(modrm)) {
		case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.ax);break;
		case 1: ecpuins.rr = (t_vaddrcc)(&ecpu.cx);break;
		case 2: ecpuins.rr = (t_vaddrcc)(&ecpu.dx);break;
		case 3: ecpuins.rr = (t_vaddrcc)(&ecpu.bx);break;
		case 4: ecpuins.rr = (t_vaddrcc)(&ecpu.sp);break;
		case 5: ecpuins.rr = (t_vaddrcc)(&ecpu.bp);break;
		case 6: ecpuins.rr = (t_vaddrcc)(&ecpu.si);break;
		case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.di);break;
		}
		ecpuins.cr = d_nubit16(ecpuins.rr);
		break;
	case 4:
		switch (_GetModRM_REG(modrm)) {
		case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.eax);break;
		case 1: ecpuins.rr = (t_vaddrcc)(&ecpu.ecx);break;
		case 2: ecpuins.rr = (t_vaddrcc)(&ecpu.edx);break;
		case 3: ecpuins.rr = (t_vaddrcc)(&ecpu.ebx);break;
		case 4: ecpuins.rr = (t_vaddrcc)(&ecpu.esp);break;
		case 5: ecpuins.rr = (t_vaddrcc)(&ecpu.ebp);break;
		case 6: ecpuins.rr = (t_vaddrcc)(&ecpu.esi);break;
		case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.edi);break;
		}
		ecpuins.cr = d_nubit32(ecpuins.rr);
		break;
	default: CaseError("ParseModRM::regbyte");break;}
}

static void GetMem()
{
	/* returns ecpuins.rrm */
	ecpuins.flagmem = 1;
	ecpuins.erm = vramRealWord(ecpu.cs.selector, ecpu.eip);
	ecpuins.lrm = (ecpuins.roverds->selector << 4) + ecpuins.erm;
	ecpuins.prm = ecpuins.lrm;
	ecpuins.rrm = vramAddr(ecpuins.prm);
	ecpu.eip += _GetAddressSize;
}
static void GetImm(t_nubit8 byte)
{
	ecpuins.rimm = vramGetRealAddr(ecpu.cs.selector, ecpu.eip);
	ecpu.eip += byte;
}
static void GetModRegRM(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	ParseModRM(regbyte, rmbyte);
	if (ecpuins.flagmem) {
		ecpuins.lrm = ((ecpuins.flagmss ? ecpuins.roverss->selector : ecpuins.roverds->selector) << 4) + ecpuins.erm;
		ecpuins.prm = ecpuins.lrm;
		ecpuins.rrm = vramAddr(ecpuins.prm);
		switch (rmbyte) {
		case 1: ecpuins.crm = d_nubit8(ecpuins.rrm);break;
		case 2: ecpuins.crm = d_nubit16(ecpuins.rrm);break;
		case 4: ecpuins.crm = d_nubit32(ecpuins.rrm);break;
		case 6: ecpuins.crm = d_nubit48(ecpuins.rrm);break;
		default: CaseError("GetModRegRM::rmbyte");break;}
	}
}
static void GetModSegRM(t_nubit8 rmbyte)
{
	ParseModRM(0, rmbyte);
	if (ecpuins.flagmem) {
		ecpuins.lrm = ((ecpuins.flagmss ? ecpuins.roverss->selector : ecpuins.roverds->selector) << 4) + ecpuins.erm;
		ecpuins.prm = ecpuins.lrm;
		ecpuins.rrm = vramAddr(ecpuins.prm);
		switch (rmbyte) {
		case 1: ecpuins.crm = d_nubit8(ecpuins.rrm);break;
		case 2: ecpuins.crm = d_nubit16(ecpuins.rrm);break;
		case 4: ecpuins.crm = d_nubit32(ecpuins.rrm);break;
		case 6: ecpuins.crm = d_nubit48(ecpuins.rrm);break;
		default: CaseError("GetModSegRM::rmbyte");break;}
	}
	switch (ecpuins.cr) {
	case 0: ecpuins.rmovsreg = &ecpu.es;break;
	case 1: ecpuins.rmovsreg = &ecpu.cs;break;
	case 2: ecpuins.rmovsreg = &ecpu.ss;break;
	case 3: ecpuins.rmovsreg = &ecpu.ds;break;
	case 4: ecpuins.rmovsreg = &ecpu.fs;break;
	case 5: ecpuins.rmovsreg = &ecpu.gs;break;
	default: CaseError("GetModSegRM::ecpuins.cr");break;}
}
static void GetModRegRMEA(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	ParseModRM(regbyte, rmbyte);
	if (!ecpuins.flagmem)
		CaseError("GetModRegRMEA::flagmem(0)");
}

static void INT(t_nubit8 intid);

DONEASM ADD(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation add
	arith2;
#undef operation
}
DONEASM OR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation or
	arith2;
#undef operation
}
DONEASM ADC(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation adc
	arith2;
#undef operation
}
DONEASM SBB(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation sbb
	arith2;
#undef operation
}
DONEASM AND(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation and
	arith2;
#undef operation
}
DONEASM SUB(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation sub
	arith2;
#undef operation
}
DONEASM XOR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation xor
	arith2;
#undef operation
}
DONEASM CMP(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation cmp
	arith2;
#undef operation
}
DONEASM TEST(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation test
	arith2;
#undef operation
}
DONEASM INC(t_vaddrcc rdest, t_nubit8 bit)
{
#define operation inc
	arith1;
#undef operation
}
DONEASM DEC(t_vaddrcc rdest, t_nubit8 bit)
{
#define operation dec
	arith1;
#undef operation
}
DONEASM NOT(t_vaddrcc rdest, t_nubit8 bit)
{
#define operation not
	arith1;
#undef operation
}
DONEASM NEG(t_vaddrcc rdest, t_nubit8 bit)
{
#define operation neg
	arith1;
#undef operation
}
DONEASM ROL(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation rol
	shift;
#undef operation
}
DONEASM ROR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation ror
	shift;
#undef operation
}
DONEASM RCL(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation rcl
	shift;
#undef operation
}
DONEASM RCR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation rcr
	shift;
#undef operation
}
DONEASM SHL(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation shl
	shift;
#undef operation
}
DONEASM SHR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation shr
	shift;
#undef operation
}
DONEASM SAL(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation sal
	shift;
#undef operation
}
DONEASM SAR(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count;
#define operation sar
	shift;
#undef operation
}
DONEASM MUL(t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation mul
	arithax;
#undef operation
}
DONEASM IMUL(t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation imul
	arithax;
#undef operation
}
DONEASM DIV(t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation div
	arithax;
#undef operation
}
DONEASM IDIV(t_vaddrcc rsrc, t_nubit8 bit)
{
#define operation idiv
	arithax;
#undef operation
}

DONEASM XCHG(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 byte)
{
	switch (byte) {
	case 1:
		uba = d_nubit8(rdest);
		ubb = d_nubit8(rsrc);
		if (!im(rdest) || wm) d_nubit8(rdest) = ubb;
		if (!im(rsrc) || wm)  d_nubit8(rsrc) = uba;
		break;
	case 2:
		uwa = d_nubit16(rdest);
		uwb = d_nubit16(rsrc);
		if (!im(rdest) || wm) d_nubit16(rdest) = uwb;
		if (!im(rsrc) || wm)  d_nubit16(rsrc) = uwa;
		break;
	case 4:
		uda = d_nubit32(rdest);
		udb = d_nubit32(rsrc);
		if (!im(rdest) || wm) d_nubit32(rdest) = udb;
		if (!im(rsrc) || wm)  d_nubit32(rsrc) = uda;
		break;
	default:CaseError("XCHG::byte");break;}
}
DONEASM MOV(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 byte)
{
	switch (byte) {
	case 1:
		if (!im(rdest) || wm)  d_nubit8(rdest) = d_nubit8(rsrc);
		break;
	case 2:
		if (!im(rdest) || wm)  d_nubit16(rdest) = d_nubit16(rsrc);
		break;
	case 4:
		if (!im(rdest) || wm)  d_nubit32(rdest) = d_nubit32(rsrc);
		break;
	default:CaseError("MOV::byte");break;}
}
DONEASM PUSH(t_vaddrcc rsrc, t_nubit8 byte)
{
	t_nubit32 data;
	switch (byte) {
	case 2:
		data = d_nubit16(rsrc);
		switch (_GetStackSize) {
		case 2:
			ecpu.sp -= 2;
			if (wm) vramRealWord(ecpu.ss.selector, ecpu.sp) = GetMax16(data);
			break;
		case 4:
			ecpu.esp -= 2;
			if (wm) vramRealWord(ecpu.ss.selector, ecpu.esp) = GetMax16(data);
			break;
		}
		break;
	case 4:
		data = d_nubit32(rsrc);
		switch (_GetStackSize) {
		case 2:
			ecpu.sp -= 4;
			if (wm) vramRealDWord(ecpu.ss.selector, ecpu.sp) = GetMax32(data);
			break;
		case 4:
			ecpu.esp -= 4;
			if (wm) vramRealDWord(ecpu.ss.selector, ecpu.esp) = GetMax32(data);
			break;
		}
		break;
	default:
		CaseError("PUSH::byte");break;}
}
DONEASM POP(t_vaddrcc rdest, t_nubit8 byte)
{
	switch (byte) {
	case 2:
		switch (_GetStackSize) {
		case 2:
			if (!im(rdest) || wm) d_nubit16(rdest) = vramRealWord(ecpu.ss.selector, ecpu.sp);
			ecpu.sp += 2;
			break;
		case 4:
			if (!im(rdest) || wm) d_nubit16(rdest) = vramRealWord(ecpu.ss.selector, ecpu.esp);
			ecpu.esp += 2;
			break;
		}
		break;
	case 4:
		switch (_GetStackSize) {
		case 2:
			if (!im(rdest) || wm) d_nubit32(rdest) = vramRealDWord(ecpu.ss.selector, ecpu.sp);
			ecpu.sp += 4;
			break;
		case 4:
			if (!im(rdest) || wm) d_nubit32(rdest) = vramRealDWord(ecpu.ss.selector, ecpu.esp);
			ecpu.esp += 4;
			break;
		}
		break;
	default:CaseError("POP::byte");break;}
}
DONEASM JCC(t_vaddrcc rsrc, t_nubit8 byte, t_bool condition)
{
	if (condition) {
		switch (byte) {
		case 1: ecpu.eip += d_nsbit8(rsrc);break;
		case 2: ecpu.eip += d_nsbit16(rsrc);break;
		case 4: ecpu.eip += d_nsbit32(rsrc);break;
		default:CaseError("JCC::byte");break;}
		switch (_GetOperandSize) {
		case 2: ecpu.eip = GetMax16(ecpu.eip);break;
		case 4: ecpu.eip = GetMax32(ecpu.eip);break;
		}
	}
}
DONEASM LOOPCC(t_vaddrcc rsrc, t_bool condition)
{
	t_nubit32 cecx = 0x00000000;
	t_nubit32 neweip = ecpu.eip;
	switch (_GetAddressSize) {
	case 2:
		ecpu.cx--;
		cecx = GetMax16(ecpu.ecx);
		break;
	case 4:
		ecpu.ecx--;
		cecx = GetMax32(ecpu.ecx);
		break;
	}
	if (cecx && condition) {
		neweip += d_nsbit8(rsrc);
		switch (_GetOperandSize) {
		case 2: ecpu.eip = GetMax16(neweip);break;
		case 4: ecpu.eip = GetMax32(neweip);break;
		}
	}
}
DONEASM STRDIR(t_nubit8 byte, t_bool flagsi, t_bool flagdi)
{
	switch (_GetAddressSize) {
	case 2:
		if (GetBit(ecpu.flags, VCPU_EFLAGS_DF)) {
			if (flagdi) ecpu.di -= byte;
			if (flagsi) ecpu.si -= byte;
		} else {
			if (flagdi) ecpu.di += byte;
			if (flagsi) ecpu.si += byte;
		}
		break;
	case 4:
		if (GetBit(ecpu.flags, VCPU_EFLAGS_DF)) {
			if (flagdi) ecpu.edi -= byte;
			if (flagsi) ecpu.esi -= byte;
		} else {
			if (flagdi) ecpu.edi += byte;
			if (flagsi) ecpu.esi += byte;
		}
		break;
	default:
		vapiPrint("internal case error: byte(%x)\n", byte);
		break;
	}
}
DONEASM MOVS(t_nubit8 bit)
{
	t_vaddrcc rdest = vramGetRealAddr(ecpu.es.selector, ecpu.di);
	t_vaddrcc rsrc = vramGetRealAddr(ecpuins.roverds->selector, ecpu.si);
	MOV(rdest, rsrc, (bit >> 3));
	STRDIR((bit >> 3), 1, 1);
}
DONEASM CMPS(t_nubit8 bit)
{
	t_vaddrcc rdest = vramGetRealAddr(ecpuins.roverds->selector, ecpu.si);
	t_vaddrcc rsrc = vramGetRealAddr(ecpu.es.selector, ecpu.di);
	CMP(rdest, rsrc, bit);
	STRDIR((bit >> 3), 1, 1);
}
DONEASM STOS(t_nubit8 bit)
{
	t_vaddrcc rdest = vramGetRealAddr(ecpu.es.selector, ecpu.di);
	t_vaddrcc rsrc = (t_vaddrcc)&ecpu.eax;
	MOV(rdest, rsrc, (bit >> 3));
	STRDIR((bit >> 3), 0, 1);
}
DONEASM LODS(t_nubit8  bit)
{
	t_vaddrcc rdest = (t_vaddrcc)&ecpu.eax;
	t_vaddrcc rsrc = vramGetRealAddr(ecpuins.roverds->selector, ecpu.si);
	MOV(rdest, rsrc, (bit >> 3));
	STRDIR((bit >> 3), 1, 0);
}
DONEASM SCAS(t_nubit8 bit)
{
	t_vaddrcc rdest = (t_vaddrcc)&ecpu.eax;
	t_vaddrcc rsrc = vramGetRealAddr(ecpu.es.selector, ecpu.di);
	CMP(rdest, rsrc, bit);
	STRDIR((bit >> 3), 0, 1);
}
DONEASM INT(t_nubit8 intid)
{
	t_nubit32 oldcs = ecpu.cs.selector;
	switch (_GetOperandSize) {
	case 2:
		PUSH((t_vaddrcc)&ecpu.flags, 2);
		ClrBit(ecpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
		PUSH((t_vaddrcc)&oldcs, 2);
		PUSH((t_vaddrcc)&ecpu.ip, 2);
		ecpu.eip = vramWord(intid * 4);
		ecpu.cs.selector = vramWord(intid * 4 + 2);
		break;
	case 4:
		PUSH((t_vaddrcc)&ecpu.eflags, 4);
		ClrBit(ecpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
		PUSH((t_vaddrcc)&oldcs, 4);
		PUSH((t_vaddrcc)&ecpu.eip, 4);
		ecpu.eip = vramWord(intid * 4);
		ecpu.cs.selector = vramWord(intid * 4 + 2);
		break;
	}

}

DONEASM UndefinedOpcode()
{
	ecpu.cs = ecpuins.oldcs;
	ecpu.eip = ecpuins.oldeip;
	vapiPrint("The NXVM ECPU has encountered an illegal instruction.\n");
	vapiPrint("CS:%04X IP:%08X OP:%02X %02X %02X %02X\n",
		ecpu.cs.selector, ecpu.eip, vramRealByte(ecpu.cs.selector, ecpu.eip+0),
		vramRealByte(ecpu.cs.selector, ecpu.eip+1), vramRealByte(ecpu.cs.selector, ecpu.eip+2),
		vramRealByte(ecpu.cs.selector, ecpu.eip+3), vramRealByte(ecpu.cs.selector, ecpu.eip+4));
	vapiCallBackMachineStop();
}
DONEASM ADD_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	ADD(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM ADD_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	ADD(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM ADD_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	ADD(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM ADD_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	ADD(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM ADD_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	ADD((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM ADD_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	ADD((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM PUSH_ES()
{
	t_nubit32 oldes = ecpu.es.selector;
	ecpu.eip++;
	PUSH((t_vaddrcc)&oldes, _GetOperandSize);
}
DONEASM POP_ES()
{
	t_nubit32 newes;
	ecpu.eip++;
	POP((t_vaddrcc)&newes, _GetOperandSize);
	ecpu.es.selector = GetMax16(newes);
}
DONEASM OR_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	OR(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM OR_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	OR(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM OR_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	OR(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM OR_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	OR(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM OR_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	OR((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM OR_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	OR((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM PUSH_CS()
{
	t_nubit32 oldcs = ecpu.cs.selector;
	ecpu.eip++;
	PUSH((t_vaddrcc)&oldcs, _GetOperandSize);
}
DONEASM POP_CS()
{
	t_nubit32 newcs;
	ecpu.eip++;
	POP((t_vaddrcc)&newcs, _GetOperandSize);
	ecpu.cs.selector = GetMax16(newcs);
}
DONEASM INS_0F()
{
	t_nubit8 opcode;
	ecpu.eip++;
	opcode = vramRealByte(ecpu.cs.selector, ecpu.eip);
	ExecFun(ecpuins.table_0f[opcode]);
}
DONEASM ADC_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	ADC(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM ADC_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	ADC(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM ADC_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	ADC(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM ADC_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	ADC(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM ADC_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	ADC((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM ADC_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	ADC((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM PUSH_SS()
{
	t_nubit32 oldss = ecpu.ss.selector;
	ecpu.eip++;
	PUSH((t_vaddrcc)&oldss, _GetOperandSize);
}
DONEASM POP_SS()
{
	t_nubit32 newss;
	ecpu.eip++;
	POP((t_vaddrcc)&newss, _GetOperandSize);
	ecpu.ss.selector = GetMax16(newss);
}
DONEASM SBB_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	SBB(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM SBB_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	SBB(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM SBB_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	SBB(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM SBB_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	SBB(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM SBB_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	SBB((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM SBB_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	SBB((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM PUSH_DS()
{
	t_nubit32 oldds = ecpu.ds.selector;
	ecpu.eip++;
	PUSH((t_vaddrcc)&oldds, _GetOperandSize);
}
DONEASM POP_DS()
{
	t_nubit32 newds;
	ecpu.eip++;
	POP((t_vaddrcc)&newds, _GetOperandSize);
	ecpu.ds.selector = GetMax16(newds);
}
DONEASM AND_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	AND(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM AND_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	AND(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM AND_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	AND(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM AND_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	AND(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM AND_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	AND((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM AND_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	AND((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM ES()
{
	ecpu.eip++;
	ecpuins.roverds = &ecpu.es;
	ecpuins.roverss = &ecpu.es;
}
DONEASM DAA()
{
	ecpu.eip++;
#define operation daa
	adjustax;
#undef operation
}
DONEASM SUB_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	SUB(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM SUB_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	SUB(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM SUB_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	SUB(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM SUB_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	SUB(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM SUB_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	SUB((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM SUB_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	SUB((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM CS()
{
	ecpu.eip++;
	ecpuins.roverds = &ecpu.cs;
	ecpuins.roverss = &ecpu.cs;
}
DONEASM DAS()
{
	ecpu.eip++;
#define operation das
	adjustax;
#undef operation
}
DONEASM XOR_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	XOR(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM XOR_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	XOR(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM XOR_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	XOR(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM XOR_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	XOR(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM XOR_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	XOR((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM XOR_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	XOR((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM SS()
{
	ecpu.eip++;
	ecpuins.roverds = &ecpu.ss;
	ecpuins.roverss = &ecpu.ss;
}
DONEASM AAA()
{
	ecpu.eip++;
#define operation aaa
	adjustax;
#undef operation
}
DONEASM CMP_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	CMP(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM CMP_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	CMP(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM CMP_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	CMP(ecpuins.rr, ecpuins.rrm, 8);
}
DONEASM CMP_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	CMP(ecpuins.rr, ecpuins.rrm, _GetOperandSize * 8);
}
DONEASM CMP_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	CMP((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM CMP_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	CMP((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM DS()
{
	ecpu.eip++;
	ecpuins.roverds = &ecpu.ds;
	ecpuins.roverss = &ecpu.ds;
}
DONEASM AAS()
{
	ecpu.eip++;
#define operation aas
	adjustax;
#undef operation
}
DONEASM INC_AX()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.eax, _GetOperandSize * 8);
}
DONEASM INC_CX()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.ecx, _GetOperandSize * 8);
}
DONEASM INC_DX()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.edx, _GetOperandSize * 8);
}
DONEASM INC_BX()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.ebx, _GetOperandSize * 8);
}
DONEASM INC_SP()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.esp, _GetOperandSize * 8);
}
DONEASM INC_BP()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.ebp, _GetOperandSize * 8);
}
DONEASM INC_SI()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.esi, _GetOperandSize * 8);
}
DONEASM INC_DI()
{
	ecpu.eip++;
	INC((t_vaddrcc)&ecpu.edi, _GetOperandSize * 8);
}
DONEASM DEC_AX()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.eax, _GetOperandSize * 8);
}
DONEASM DEC_CX()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.ecx, _GetOperandSize * 8);
}
DONEASM DEC_DX()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.edx, _GetOperandSize * 8);
}
DONEASM DEC_BX()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.ebx, _GetOperandSize * 8);
}
DONEASM DEC_SP()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.esp, _GetOperandSize * 8);
}
DONEASM DEC_BP()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.ebp, _GetOperandSize * 8);
}
DONEASM DEC_SI()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.esi, _GetOperandSize * 8);
}
DONEASM DEC_DI()
{
	ecpu.eip++;
	DEC((t_vaddrcc)&ecpu.edi, _GetOperandSize * 8);
}
DONEASM PUSH_AX()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.eax, _GetOperandSize);
}
DONEASM PUSH_CX()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.ecx, _GetOperandSize);
}
DONEASM PUSH_DX()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.edx, _GetOperandSize);
}
DONEASM PUSH_BX()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.ebx, _GetOperandSize);
}
DONEASM PUSH_SP()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.esp, _GetOperandSize);
}
DONEASM PUSH_BP()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.ebp, _GetOperandSize);
}
DONEASM PUSH_SI()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.esi, _GetOperandSize);
}
DONEASM PUSH_DI()
{
	ecpu.eip++;
	PUSH((t_vaddrcc)&ecpu.edi, _GetOperandSize);
}
DONEASM POP_AX()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.eax, _GetOperandSize);
}
DONEASM POP_CX()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.ecx, _GetOperandSize);
}
DONEASM POP_DX()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.edx, _GetOperandSize);
}
DONEASM POP_BX()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.ebx, _GetOperandSize);
}
DONEASM POP_SP()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.esp, _GetOperandSize);
}
DONEASM POP_BP()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.ebp, _GetOperandSize);
}
DONEASM POP_SI()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.esi, _GetOperandSize);
}
DONEASM POP_DI()
{
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.edi, _GetOperandSize);
}
DONEASM OprSize()
{
	ecpu.eip++;
	ecpuins.prefix_oprsize = 1;
}
DONEASM AddrSize()
{
	ecpu.eip++;
	ecpuins.prefix_addrsize = 1;
}
DONEASM PUSH_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	PUSH(ecpuins.rimm, _GetOperandSize);
}
DONEASM JO_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, GetBit(ecpu.flags, VCPU_EFLAGS_OF));
}
DONEASM JNO_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, !GetBit(ecpu.flags, VCPU_EFLAGS_OF));
}
DONEASM JC_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, GetBit(ecpu.flags, VCPU_EFLAGS_CF));
}
DONEASM JNC_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, !GetBit(ecpu.flags, VCPU_EFLAGS_CF));
}
DONEASM JZ_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, GetBit(ecpu.flags, VCPU_EFLAGS_ZF));
}
DONEASM JNZ_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, !GetBit(ecpu.flags, VCPU_EFLAGS_ZF));
}
DONEASM JBE_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (GetBit(ecpu.flags, VCPU_EFLAGS_CF) ||
		GetBit(ecpu.flags, VCPU_EFLAGS_ZF)));
}
DONEASM JA_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (!GetBit(ecpu.flags, VCPU_EFLAGS_CF) &&
		!GetBit(ecpu.flags, VCPU_EFLAGS_ZF)));
}
DONEASM JS_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, GetBit(ecpu.flags, VCPU_EFLAGS_SF));
}
DONEASM JNS_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, !GetBit(ecpu.flags, VCPU_EFLAGS_SF));
}
DONEASM JP_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, GetBit(ecpu.flags, VCPU_EFLAGS_PF));
}
DONEASM JNP_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, !GetBit(ecpu.flags, VCPU_EFLAGS_PF));
}
DONEASM JL_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (GetBit(ecpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(ecpu.flags, VCPU_EFLAGS_OF)));
}
DONEASM JNL_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (GetBit(ecpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(ecpu.flags, VCPU_EFLAGS_OF)));
}
DONEASM JLE_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (GetBit(ecpu.flags, VCPU_EFLAGS_ZF) ||
		(GetBit(ecpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(ecpu.flags, VCPU_EFLAGS_OF))));
}
DONEASM JG_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, (!GetBit(ecpu.flags, VCPU_EFLAGS_ZF) &&
		(GetBit(ecpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(ecpu.flags, VCPU_EFLAGS_OF))));
}
DONEASM INS_80()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	GetImm(1);
	switch (ecpuins.cr) {
	case 0: ADD(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 1: OR(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 2: ADC(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 3: SBB(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 4: AND(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 5: SUB(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 6: XOR(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 7: CMP(ecpuins.rrm, ecpuins.rimm, 8);break;
	}
}
DONEASM INS_81()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	GetImm(_GetOperandSize);
	switch (ecpuins.cr) {
	case 0: ADD(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 1: OR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 2: ADC(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 3: SBB(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 4: AND(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 5: SUB(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 6: XOR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 7: CMP(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	}
}
DONEASM INS_83()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	GetImm(1);
	switch (ecpuins.cr) {
	case 0: ADD(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 1: OR(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 2: ADC(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 3: SBB(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 4: AND(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 5: SUB(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 6: XOR(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	case 7: CMP(ecpuins.rrm, ecpuins.rimm, (_GetOperandSize + 1) * 4);break;
	}
}
DONEASM TEST_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	TEST(ecpuins.rrm, ecpuins.rr, 8);
}
DONEASM TEST_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	TEST(ecpuins.rrm, ecpuins.rr, _GetOperandSize * 8);
}
DONEASM XCHG_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	XCHG(ecpuins.rr, ecpuins.rrm, 1);
}
DONEASM XCHG_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	XCHG(ecpuins.rr, ecpuins.rrm, _GetOperandSize);
}
DONEASM MOV_RM8_R8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	MOV(ecpuins.rrm, ecpuins.rr, 1);
}
DONEASM MOV_RM16_R16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	MOV(ecpuins.rrm, ecpuins.rr, _GetOperandSize);
}
DONEASM MOV_R8_RM8()
{
	ecpu.eip++;
	GetModRegRM(1, 1);
	MOV(ecpuins.rr, ecpuins.rrm, 1);
}
DONEASM MOV_R16_RM16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize);
	MOV(ecpuins.rr, ecpuins.rrm, _GetOperandSize);
}
DONEASM MOV_RM16_SEG()
{
	ecpu.eip++;
	GetModSegRM(2);
	MOV(ecpuins.rrm, (t_vaddrcc)&(ecpuins.rmovsreg->selector), 2);
}
DONEASM LEA_R16_M16()
{
	ecpu.eip++;
	GetModRegRMEA(_GetOperandSize, _GetOperandSize);
	switch (_GetOperandSize) {
	case 2: d_nubit16(ecpuins.rr) = GetMax16(ecpuins.erm);break;
	case 4: d_nubit32(ecpuins.rr) = GetMax32(ecpuins.erm);break;
	}
}
DONEASM MOV_SEG_RM16()
{
	ecpu.eip++;
	GetModSegRM(2);
	MOV((t_vaddrcc)&(ecpuins.rmovsreg->selector), ecpuins.rrm, 2);
}
DONEASM POP_RM16()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	switch (ecpuins.cr) {
	case 0: POP(ecpuins.rrm, _GetOperandSize);break;
	default:UndefinedOpcode();break;}
}
DONEASM NOP()
{
	ecpu.eip++;
}
DONEASM XCHG_CX_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.ecx, (t_vaddrcc)&ecpu.eax, _GetOperandSize);
}
DONEASM XCHG_DX_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.dx, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM XCHG_BX_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.bx, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM XCHG_SP_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.sp, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM XCHG_BP_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.bp, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM XCHG_SI_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.si, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM XCHG_DI_AX()
{
	ecpu.eip++;
	XCHG((t_vaddrcc)&ecpu.di, (t_vaddrcc)&ecpu.ax, _GetOperandSize);
}
DONEASM CBW()
{
	ecpu.eip++;
	switch (_GetOperandSize) {
	case 2:
#define operation cbw
		adjustax;
#undef operation
		break;
	case 4:
#define operation cwde
		adjustax;
#undef operation
		break;
	}
}
DONEASM CWD()
{
	ecpu.eip++;
	switch (_GetOperandSize) {
	case 2:
#define operation cwd
		adjustax;
#undef operation
		break;
	case 4:
#define operation cdq
		adjustax;
#undef operation
		break;
	}
}
DONEASM CALL_PTR16_16()
{
	t_nubit32 oldcs = ecpu.cs.selector, neweip;
	t_nubit16 newcs;
	ecpu.eip++;
	GetImm(_GetOperandSize);
	switch (_GetOperandSize) {
	case 2: neweip = d_nubit16(ecpuins.rimm);break;
	case 4: neweip = d_nubit32(ecpuins.rimm);break;
	}
	GetImm(2);
	newcs = d_nubit16(ecpuins.rimm);
	switch (_GetOperandSize) {
	case 2:
		PUSH((t_vaddrcc)&oldcs, 2);
		PUSH((t_vaddrcc)&ecpu.ip, 2);
		ecpu.eip = GetMax16(neweip);
		ecpu.cs.selector = newcs;
		break;
	case 4:
		PUSH((t_vaddrcc)&oldcs, 4);
		PUSH((t_vaddrcc)&ecpu.eip, 4);
		ecpu.eip = GetMax32(neweip);
		ecpu.cs.selector = newcs;
		break;
	}
}
NOTIMP WAIT()
{
	ecpu.eip++;
	/* not implemented */
}
DONEASM PUSHF()
{
	ecpu.eip++;
	switch (_GetOperandSize) {
	case 2: PUSH((t_vaddrcc)&ecpu.flags, 2);break;
	case 4: PUSH((t_vaddrcc)&ecpu.eflags, 4);break;
	}
}
DONEASM POPF()
{
	ecpu.eip++;
	switch (_GetOperandSize) {
	case 2:
		POP((t_vaddrcc)&efres, 2);
		ecpu.eflags = (efres & ~(VCPU_EFLAGS_RESERVED | 0xffff0000)) |
			(ecpu.eflags & (VCPU_EFLAGS_RESERVED | 0xffff0000));
		break;
	case 4:
		POP((t_vaddrcc)&efres, 4);
		ecpu.eflags = (efres & ~(VCPU_EFLAGS_RESERVED)) |
			(ecpu.eflags & (VCPU_EFLAGS_RESERVED));
		break;
	}
}
DONEASM SAHF()
{
	ecpu.eip++;
	ecpu.eflags = (ecpu.ah & ~(ECPUINS_FLAGS_MASKED | 0xffffff00)) |
		(ecpu.eflags & (ECPUINS_FLAGS_MASKED | 0xffffff00));
}
DONEASM LAHF()
{
	ecpu.eip++;
	ecpu.ah = GetMax8(ecpu.eflags);
}
DONEASM MOV_AL_MOFFS8()
{
	ecpu.eip++;
	GetMem();
	MOV((t_vaddrcc)&ecpu.al, ecpuins.rrm, 1);
}
DONEASM MOV_AX_MOFFS16()
{
	ecpu.eip++;
	GetMem();
	MOV((t_vaddrcc)&ecpu.eax, ecpuins.rrm, _GetOperandSize);
}
DONEASM MOV_MOFFS8_AL()
{
	ecpu.eip++;
	GetMem();
	MOV(ecpuins.rrm, (t_vaddrcc)&ecpu.al, 1);
}
DONEASM MOV_MOFFS16_AX()
{
	ecpu.eip++;
	GetMem();
	MOV(ecpuins.rrm, (t_vaddrcc)&ecpu.eax, _GetOperandSize);
}
DONEASM MOVSB()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(8);
	else {
		if (ecpu.cx) {
			MOVS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
}
DONEASM MOVSW()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) MOVS(_GetOperandSize * 8);
	else {
		switch (_GetAddressSize) {
		case 2:
			if (ecpu.cx) {
				MOVS(_GetOperandSize * 8);
				ecpu.cx--;
			}
			if (ecpu.cx) ecpuins.flaginsloop = 0x01;
			break;
		case 4:
			if (ecpu.ecx) {
				MOVS(_GetOperandSize * 8);
				ecpu.ecx--;
			}
			if (ecpu.ecx) ecpuins.flaginsloop = 0x01;
			break;
		}
	}
}
DONEASM CMPSB()
{
	ecpu.eip++;
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
}
DONEASM CMPSW()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) CMPS(_GetOperandSize * 8);
	else {
		switch (_GetAddressSize) {
		case 2:
			if (ecpu.cx) {
				CMPS(_GetOperandSize * 8);
				ecpu.cx--;
			}
			if (ecpu.cx &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
					ecpuins.flaginsloop = 1;
			break;
		case 4:
			if (ecpu.ecx) {
				CMPS(_GetOperandSize * 8);
				ecpu.ecx--;
			}
			if (ecpu.ecx &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
					ecpuins.flaginsloop = 1;
			break;
		}
	}
}
DONEASM TEST_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	TEST((t_vaddrcc)&ecpu.al, ecpuins.rimm, 8);
}
DONEASM TEST_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	TEST((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize * 8);
}
DONEASM STOSB()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
	else {
		if (ecpu.cx) {
			STOS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
}
DONEASM STOSW()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(_GetOperandSize * 8);
	else {
		switch (_GetAddressSize) {
		case 2:
			if (ecpu.cx) {
				STOS(_GetOperandSize * 8);
				ecpu.cx--;
			}
			if (ecpu.cx) ecpuins.flaginsloop = 0x01;
			break;
		case 4:
			if (ecpu.ecx) {
				STOS(_GetOperandSize * 8);
				ecpu.ecx--;
			}
			if (ecpu.ecx) ecpuins.flaginsloop = 0x01;
			break;
		}
	}
}
DONEASM LODSB()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
	else {
		if (ecpu.cx) {
			LODS(8);
			ecpu.cx--;
		}
		if (ecpu.cx) ecpuins.flaginsloop = 0x01;
	}
}
DONEASM LODSW()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(_GetOperandSize * 8);
	else {
		switch (_GetAddressSize) {
		case 2:
			if (ecpu.cx) {
				LODS(_GetOperandSize * 8);
				ecpu.cx--;
			}
			if (ecpu.cx) ecpuins.flaginsloop = 0x01;
			break;
		case 4:
			if (ecpu.ecx) {
				LODS(_GetOperandSize * 8);
				ecpu.ecx--;
			}
			if (ecpu.ecx) ecpuins.flaginsloop = 0x01;
			break;
		}
	}
}
DONEASM SCASB()
{
	ecpu.eip++;
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
}
DONEASM SCASW()
{
	ecpu.eip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) SCAS(_GetOperandSize * 8);
	else {
		switch (_GetAddressSize) {
		case 2:
			if (ecpu.cx) {
				SCAS(_GetOperandSize * 8);
				ecpu.cx--;
			}
			if (ecpu.cx &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
					ecpuins.flaginsloop = 1;
			break;
		case 4:
			if (ecpu.ecx) {
				SCAS(_GetOperandSize * 8);
				ecpu.ecx--;
			}
			if (ecpu.ecx &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZ && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) &&
				!(ecpuins.prefix_rep == PREFIX_REP_REPZNZ && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)))
					ecpuins.flaginsloop = 1;
			break;
		}
	}
}
DONEASM MOV_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.al, ecpuins.rimm, 1);
}
DONEASM MOV_CL_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.cl, ecpuins.rimm, 1);
}
DONEASM MOV_DL_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.dl, ecpuins.rimm, 1);
}
DONEASM MOV_BL_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.bl, ecpuins.rimm, 1);
}
DONEASM MOV_AH_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.ah, ecpuins.rimm, 1);
}
DONEASM MOV_CH_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.ch, ecpuins.rimm, 1);
}
DONEASM MOV_DH_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.dh, ecpuins.rimm, 1);
}
DONEASM MOV_BH_I8()
{
	ecpu.eip++;
	GetImm(1);
	MOV((t_vaddrcc)&ecpu.bh, ecpuins.rimm, 1);
}
DONEASM MOV_AX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.eax, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_CX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.ecx, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_DX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.edx, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_BX_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.ebx, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_SP_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.esp, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_BP_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.ebp, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_SI_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.esi, ecpuins.rimm, _GetOperandSize);
}
DONEASM MOV_DI_I16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	MOV((t_vaddrcc)&ecpu.edi, ecpuins.rimm, _GetOperandSize);
}
DONEASM INS_C0()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	GetImm(1);
	switch (ecpuins.cr) {
	case 0:	ROL(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 1:	ROR(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 2:	RCL(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 3:	RCR(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 4:	SHL(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 5:	SHR(ecpuins.rrm, ecpuins.rimm, 8);break;
	case 7:	SAR(ecpuins.rrm, ecpuins.rimm, 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_C1()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	GetImm(1);
	switch (ecpuins.cr) {
	case 0:	ROL(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 1:	ROR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 2:	RCL(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 3:	RCR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 4:	SHL(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 5:	SHR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	case 7:	SAR(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM RET_I16()
{
	t_nubit16 addsp;
	ecpu.eip++;
	GetImm(2);
	addsp = d_nubit16(ecpuins.rimm);
	switch (_GetOperandSize) {
	case 2:
		POP((t_vaddrcc)&ecpu.ip, 2);
		ecpu.eip = GetMax16(ecpu.eip);
		break;
	case 4:
		POP((t_vaddrcc)&ecpu.eip, 4);
		break;
	}
	switch (_GetStackSize) {
	case 2: ecpu.sp += addsp; break;
	case 4: ecpu.esp += addsp;break;
	}
}
DONEASM RET()
{
	ecpu.eip++;
	switch (_GetOperandSize) {
	case 2:
		POP((t_vaddrcc)&ecpu.ip, 2);
		ecpu.eip = GetMax16(ecpu.eip);
		break;
	case 4:
		POP((t_vaddrcc)&ecpu.eip, 4);
		break;
	}
}
DONEASM LES_R16_M16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize + 2);
	MOV(ecpuins.rr, ecpuins.rrm, _GetOperandSize);
	MOV((t_vaddrcc)&ecpu.es.selector, ecpuins.rrm + _GetOperandSize, 2);
}
DONEASM LDS_R16_M16()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, _GetOperandSize + 2);
	MOV(ecpuins.rr, ecpuins.rrm, _GetOperandSize);
	MOV((t_vaddrcc)&ecpu.ds.selector, ecpuins.rrm + _GetOperandSize, 2);
}
DONEASM INS_C6()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	GetImm(1);
	switch (ecpuins.cr) {
	case 0: /* MOV_M8_I8 */
		MOV(ecpuins.rrm, ecpuins.rimm, 1);
		break;
	default:
		UndefinedOpcode();
		break;
	}
}
DONEASM INS_C7()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	GetImm(_GetOperandSize);
	switch (ecpuins.cr) {
	case 0: /* MOV_M16_I16 */
		MOV(ecpuins.rrm, ecpuins.rimm, _GetOperandSize);
		break;
	default:
		UndefinedOpcode();
		break;
	}
}
DONEASM RETF_I16()
{
	t_nubit16 addsp;
	t_nubit32 newcs;
	ecpu.eip++;
	GetImm(2);
	addsp = d_nubit16(ecpuins.rimm);
	POP((t_vaddrcc)&ecpu.eip, _GetOperandSize);
	POP((t_vaddrcc)&newcs, _GetOperandSize);
	ecpu.cs.selector = GetMax16(newcs);
	if (_GetOperandSize == 2) ecpu.eip = GetMax16(ecpu.eip);
	ecpu.sp += addsp;
}
DONEASM RETF()
{
	t_nubit32 newcs;
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.eip, _GetOperandSize);
	POP((t_vaddrcc)&newcs, _GetOperandSize);
	ecpu.cs.selector = GetMax16(newcs);
	if (_GetOperandSize == 2) ecpu.eip = GetMax16(ecpu.eip);
}
DONEASM INT3()
{
	ecpu.eip++;
	INT(0x03);
}
DONEASM INT_I8()
{
	ecpu.eip++;
	GetImm(1);
	INT(d_nubit8(ecpuins.rimm));
}
DONEASM INTO()
{
	ecpu.eip++;
	if (GetBit(ecpu.flags, VCPU_EFLAGS_OF)) INT(0x04);
}
DONEASM IRET()
{
	t_nubit32 newcs;
	ecpu.eip++;
	POP((t_vaddrcc)&ecpu.eip, _GetOperandSize);
	POP((t_vaddrcc)&newcs, _GetOperandSize);
	POP((t_vaddrcc)&efres, _GetOperandSize);
	ecpu.cs.selector = GetMax16(newcs);
	if (_GetOperandSize == 2) ecpu.eip = GetMax16(ecpu.eip);
	switch (_GetOperandSize) {
	case 2:
		ecpu.eflags = (efres & 0x0000ffff) | (ecpu.eflags & 0xffff0000);
		break;
	case 4:
		ecpu.eflags = (efres & 0x00257fd5) | (ecpu.eflags & 0x001a0000);
		break;
	}
}
DONEASM INS_D0()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	switch (ecpuins.cr) {
	case 0: ROL(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 1: ROR(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 2: RCL(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 3: RCR(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 4: SHL(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 5: SHR(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	case 7: SAR(ecpuins.rrm, (t_vaddrcc)NULL, 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_D1()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	switch (ecpuins.cr) {
	case 0:	ROL(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 1:	ROR(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 2:	RCL(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 3:	RCR(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 4:	SHL(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 5:	SHR(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	case 7:	SAR(ecpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_D2()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	switch (ecpuins.cr) {
	case 0:	ROL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 1:	ROR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 2:	RCL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 3:	RCR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 4:	SHL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 5:	SHR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	case 7:	SAR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_D3()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	switch (ecpuins.cr) {
	case 0:	ROL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 1:	ROR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 2:	RCL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 3:	RCR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 4:	SHL(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 5:	SHR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	case 7:	SAR(ecpuins.rrm, (t_vaddrcc)&ecpu.cl, _GetOperandSize * 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM AAM()
{
	ecpu.eip++;
	GetImm(1);
	if (d_nubit8(ecpuins.rimm) != 0x0a) CaseError("AAM::imm");
#define operation aam
	adjustax;
#undef operation
}
DONEASM AAD()
{
	ecpu.eip++;
	GetImm(1);
	if (d_nubit8(ecpuins.rimm) != 0x0a) CaseError("AAD::imm");
#define operation aad
	adjustax;
#undef operation
}
DONEASM XLAT()
{
	ecpu.eip++;
	switch (_GetAddressSize) {
	case 2:
		ecpu.al = vramRealByte(ecpuins.roverds->selector,
			ecpu.bx + ecpu.al);
		break;
	case 4:
		ecpu.al = vramRealByte(ecpuins.roverds->selector,
			ecpu.ebx + ecpu.al);
		break;
	}
}
DONEASM LOOPNZ()
{
	ecpu.eip++;
	GetImm(1);
	LOOPCC(ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_ZF));
}
DONEASM LOOPZ()
{
	ecpu.eip++;
	GetImm(1);
	LOOPCC(ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_ZF));
}
DONEASM LOOP()
{
	ecpu.eip++;
	GetImm(1);
	LOOPCC(ecpuins.rimm, 1);
}
DONEASM JCXZ_REL8()
{
	t_nubit32 cecx = ecpu.ecx;
	ecpu.eip++;
	GetImm(1);
	switch (_GetAddressSize) {
	case 2: cecx = GetMax16(cecx);break;
	case 4: cecx = GetMax32(cecx);break;
	}
	JCC(ecpuins.rimm, 1, !cecx);
}
DONEASM IN_AL_I8()
{
	ecpu.eip++;
	GetImm(1);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	ExecFun(vport.in[d_nubit8(ecpuins.rimm)]);
	ecpu.al = vport.iobyte;
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM IN_AX_I8()
{
	ecpu.eip++;
	GetImm(1);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	ExecFun(vport.in[d_nubit8(ecpuins.rimm)]);
	switch (_GetOperandSize) {
	case 2: ecpu.ax = vport.ioword;break;
	case 4: ecpu.eax = vport.iodword;break;
	}
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM OUT_I8_AL()
{
	ecpu.eip++;
	GetImm(1);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[d_nubit8(ecpuins.rimm)]);
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM OUT_I8_AX()
{
	ecpu.eip++;
	GetImm(1);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	switch (_GetOperandSize) {
	case 2: vport.ioword = ecpu.ax;break;
	case 4: vport.iodword = ecpu.eax;break;
	}
	ExecFun(vport.out[d_nubit8(ecpuins.rimm)]);
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM CALL_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	switch (_GetOperandSize) {
	case 2:
		PUSH((t_vaddrcc)&ecpu.ip, 2);
		ecpu.eip = GetMax16(ecpu.ip + d_nsbit16(ecpuins.rimm));
		break;
	case 4:
		PUSH((t_vaddrcc)&ecpu.eip, 4);
		ecpu.eip = GetMax32(ecpu.eip + d_nsbit32(ecpuins.rimm));
		break;
	}
}
DONEASM JMP_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	switch (_GetOperandSize) {
	case 2:
		ecpu.eip = GetMax16(ecpu.ip + d_nsbit16(ecpuins.rimm));
		break;
	case 4:
		ecpu.eip = GetMax32(ecpu.eip + d_nsbit32(ecpuins.rimm));
		break;
	}
}
DONEASM JMP_PTR16_16()
{
	t_nubit32 neweip;
	ecpu.eip++;
	GetImm(_GetOperandSize);
	switch (_GetOperandSize) {
	case 2: neweip = d_nubit16(ecpuins.rimm);break;
	case 4: neweip = d_nubit32(ecpuins.rimm);break;
	}
	GetImm(2);
	ecpu.eip = neweip;
	ecpu.cs.selector = d_nubit16(ecpuins.rimm);
}
DONEASM JMP_REL8()
{
	ecpu.eip++;
	GetImm(1);
	JCC(ecpuins.rimm, 1, 1);
}
DONEASM IN_AL_DX()
{
	ecpu.eip++;
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	ExecFun(vport.in[ecpu.dx]);
	ecpu.al = vport.iobyte;
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM IN_AX_DX()
{
	ecpu.eip++;
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	ExecFun(vport.in[ecpu.dx]);
	switch (_GetOperandSize) {
	case 2: ecpu.ax = vport.ioword;break;
	case 4: ecpu.eax = vport.iodword;break;
	}
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM OUT_DX_AL()
{
	ecpu.eip++;
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[ecpu.dx]);
#else
	ecpu.flagignore = 1;
#endif
}
DONEASM OUT_DX_AX()
{
	ecpu.eip++;
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	switch (_GetOperandSize) {
	case 2: vport.ioword = ecpu.ax;break;
	case 4: vport.iodword = ecpu.eax;break;
	}
	ExecFun(vport.out[ecpu.dx]);
#else
	ecpu.flagignore = 1;
#endif
}
NOTIMP LOCK()
{
	ecpu.eip++;
	/* Not Implemented */
}
DONEASM REPNZ()
{
	// CMPS,SCAS
	ecpu.eip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZNZ;
}
DONEASM REPZ()
{	// MOVS,LODS,STOS,CMPS,SCAS
	ecpu.eip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZ;
}
NOTIMP HLT()
{
	ecpu.eip++;
	/* Not Implemented */
}
DONEASM CMC()
{
	ecpu.eip++;
	ecpu.eflags ^= VCPU_EFLAGS_CF;
}
DONEASM INS_F6()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	switch (ecpuins.cr) {
	case 0:
		GetImm(1);
		TEST(ecpuins.rrm, ecpuins.rimm, 8);
		break;
	case 2: NOT(ecpuins.rrm, 8);break;
	case 3: NEG(ecpuins.rrm, 8);break;
	case 4: MUL(ecpuins.rrm, 8);break;
	case 5: IMUL(ecpuins.rrm, 8);break;
	case 6: DIV(ecpuins.rrm, 8);break;
	case 7: IDIV(ecpuins.rrm, 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_F7()
{
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	switch (ecpuins.cr) {
	case 0:
		GetImm(_GetOperandSize);
		TEST(ecpuins.rrm, ecpuins.rimm, _GetOperandSize * 8);
		break;
	case 2: NOT(ecpuins.rrm, _GetOperandSize * 8);break;
	case 3: NEG(ecpuins.rrm, _GetOperandSize * 8);break;
	case 4: MUL(ecpuins.rrm, _GetOperandSize * 8);break;
	case 5: IMUL(ecpuins.rrm, _GetOperandSize * 8);break;
	case 6: DIV(ecpuins.rrm, _GetOperandSize * 8);break;
	case 7: IDIV(ecpuins.rrm, _GetOperandSize * 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM CLC()
{
	ecpu.eip++;
	ClrBit(ecpu.eflags, VCPU_EFLAGS_CF);
}
DONEASM STC()
{
	ecpu.eip++;
	SetBit(ecpu.eflags, VCPU_EFLAGS_CF);
}
DONEASM CLI()
{
	ecpu.eip++;
	ClrBit(ecpu.eflags, VCPU_EFLAGS_IF);
}
DONEASM STI()
{
	ecpu.eip++;
	SetBit(ecpu.eflags, VCPU_EFLAGS_IF);
}
DONEASM CLD()
{
	ecpu.eip++;
	ClrBit(ecpu.eflags, VCPU_EFLAGS_DF);
}
DONEASM STD()
{
	ecpu.eip++;
	SetBit(ecpu.eflags, VCPU_EFLAGS_DF);
}
DONEASM INS_FE()
{
	ecpu.eip++;
	GetModRegRM(0, 1);
	switch (ecpuins.cr) {
	case 0: INC(ecpuins.rrm, 8);break;
	case 1: DEC(ecpuins.rrm, 8);break;
	default:UndefinedOpcode();break;}
}
DONEASM INS_FF()
{
	t_nubit32 oldcs = ecpu.cs.selector;
	ecpu.eip++;
	GetModRegRM(0, _GetOperandSize);
	switch (ecpuins.cr) {
	case 0: INC(ecpuins.rrm, _GetOperandSize * 8);break;
	case 1: DEC(ecpuins.rrm, _GetOperandSize * 8);break;
	case 2: /* CALL_RM16 */
		switch (_GetOperandSize) {
		case 2:
			PUSH((t_vaddrcc)&ecpu.ip, 2);
			ecpu.eip = d_nubit16(ecpuins.rrm);
			break;
		case 4:
			PUSH((t_vaddrcc)&ecpu.eip, 4);
			ecpu.eip = d_nubit32(ecpuins.rrm);
			break;
		}
		break;
	case 3: /* CALL_M16_16 */
		switch (_GetOperandSize) {
		case 2:
			PUSH((t_vaddrcc)&oldcs, 2);
			PUSH((t_vaddrcc)&ecpu.ip, 2);
			ecpu.eip = d_nubit16(ecpuins.rrm);
			ecpu.cs.selector = d_nubit16(ecpuins.rrm + 2);
			break;
		case 4:
			PUSH((t_vaddrcc)&oldcs, 4);
			PUSH((t_vaddrcc)&ecpu.eip, 4);
			ecpu.eip = d_nubit32(ecpuins.rrm);
			ecpu.cs.selector = d_nubit16(ecpuins.rrm + 4);
			break;
		}
		break;
	case 4: /* JMP_RM16 */
		switch (_GetOperandSize) {
		case 2: ecpu.eip = d_nubit16(ecpuins.rrm);break;
		case 4: ecpu.eip = d_nubit32(ecpuins.rrm);break;
		}
		break;
	case 5: /* JMP_M16_16 */
		switch (_GetOperandSize) {
		case 2:
			ecpu.eip = d_nubit16(ecpuins.rrm);
			ecpu.cs.selector = d_nubit16(ecpuins.rrm + 2);
			break;
		case 4:
			ecpu.eip = d_nubit32(ecpuins.rrm);
			ecpu.cs.selector = d_nubit16(ecpuins.rrm + 4);
			break;
		}
		break;
	case 6: /* PUSH_RM16 */
		PUSH(ecpuins.rrm, _GetOperandSize);
		break;
	default:UndefinedOpcode();break;}
}

DONEASM INS_0F_01()
{
	ecpu.eip++;
	GetModRegRM(0, 2);
	switch (ecpuins.cr) {
	case 4:
		d_nubit16(ecpuins.rrm) = GetMax16(ecpu.cr0);
		break;
	default: UndefinedOpcode();break;
	}
}
DONEASM JC_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	JCC(ecpuins.rimm, _GetOperandSize, GetBit(ecpu.eflags, VCPU_EFLAGS_CF));
}
DONEASM JZ_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	JCC(ecpuins.rimm, _GetOperandSize, GetBit(ecpu.eflags, VCPU_EFLAGS_ZF));
}
DONEASM JNZ_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	JCC(ecpuins.rimm, _GetOperandSize, !GetBit(ecpu.eflags, VCPU_EFLAGS_ZF));
}
DONEASM JA_REL16()
{
	ecpu.eip++;
	GetImm(_GetOperandSize);
	JCC(ecpuins.rimm, _GetOperandSize,
		!(GetBit(ecpu.eflags, VCPU_EFLAGS_CF) || GetBit(ecpu.eflags, VCPU_EFLAGS_ZF)));
}
DONEASM MOVZX_R16_RM8()
{
	ecpu.eip++;
	GetModRegRM(_GetOperandSize, 1);
	switch (_GetOperandSize) {
	case 2: d_nubit16(ecpuins.rr) = d_nubit8(ecpuins.rrm);break;
	case 4: d_nubit32(ecpuins.rr) = d_nubit8(ecpuins.rrm);break;
	}
}
DONEASM MOVZX_R32_RM16()
{
	ecpu.eip++;
	GetModRegRM(4, 2);
	d_nubit32(ecpuins.rr) = d_nubit16(ecpuins.rrm);
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch (opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65:
	case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
static void ExecIns()
{
	t_nubit8 opcode;

	ecpuins.roverds = &ecpu.ds;
	ecpuins.roverss = &ecpu.ss;
	ecpuins.prefix_addrsize = 0;
	ecpuins.prefix_lock = 0;
	ecpuins.prefix_oprsize = 0;
	ecpuins.prefix_rep = PREFIX_REP_NONE;
	ecpuins.oldcs = ecpu.cs;
	ecpuins.oldss = ecpu.ss;
	ecpuins.oldeip = ecpu.eip;
	ecpuins.oldesp = ecpu.esp;
	ecpuins.flaginsloop = 0;
	ecpuins.flagmaskint = 0;

	do {
		opcode = vramRealByte(ecpu.cs.selector, ecpu.ip);
		ExecFun(ecpuins.table[opcode]);
	} while (IsPrefix(opcode));

	if (ecpuins.flaginsloop) {
		ecpu.cs = ecpuins.oldcs;
		ecpu.eip = ecpuins.oldeip;
	}
}
static void ExecInt()
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

static void QDX()
{
	ecpu.eip++;
	GetImm(1);
#if VGLOBAL_ECPU_MODE == TEST_ECPU
	switch (d_nubit8(ecpuins.rimm)) {
	case 0x00:
	case 0xff:
		vapiPrint("\nNXVM STOP at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			ecpu.cs.selector, ecpu.ip,d_nubit8(ecpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe:
		vapiPrint("\nNXVM RESET at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			ecpu.cs.selector, ecpu.ip,d_nubit8(ecpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	default:
		qdbiosExecInt(d_nubit8(ecpuins.rimm));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_ZF, GetBit(_flags, VCPU_EFLAGS_ZF));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_CF, GetBit(_flags, VCPU_EFLAGS_CF));
		break;
	}
#else
	ecpu.flagignore = 1;
#endif
}

void ecpuinsInit()
{
	memset(&ecpuins, 0x00, sizeof(t_cpuins));
	ecpuins.table[0x00] = (t_faddrcc)ADD_RM8_R8;
	ecpuins.table[0x01] = (t_faddrcc)ADD_RM16_R16;
	ecpuins.table[0x02] = (t_faddrcc)ADD_R8_RM8;
	ecpuins.table[0x03] = (t_faddrcc)ADD_R16_RM16;
	ecpuins.table[0x04] = (t_faddrcc)ADD_AL_I8;
	ecpuins.table[0x05] = (t_faddrcc)ADD_AX_I16;
	ecpuins.table[0x06] = (t_faddrcc)PUSH_ES;
	ecpuins.table[0x07] = (t_faddrcc)POP_ES;
	ecpuins.table[0x08] = (t_faddrcc)OR_RM8_R8;
	ecpuins.table[0x09] = (t_faddrcc)OR_RM16_R16;
	ecpuins.table[0x0a] = (t_faddrcc)OR_R8_RM8;
	ecpuins.table[0x0b] = (t_faddrcc)OR_R16_RM16;
	ecpuins.table[0x0c] = (t_faddrcc)OR_AL_I8;
	ecpuins.table[0x0d] = (t_faddrcc)OR_AX_I16;
	ecpuins.table[0x0e] = (t_faddrcc)PUSH_CS;
	ecpuins.table[0x0f] = (t_faddrcc)INS_0F;
	ecpuins.table[0x10] = (t_faddrcc)ADC_RM8_R8;
	ecpuins.table[0x11] = (t_faddrcc)ADC_RM16_R16;
	ecpuins.table[0x12] = (t_faddrcc)ADC_R8_RM8;
	ecpuins.table[0x13] = (t_faddrcc)ADC_R16_RM16;
	ecpuins.table[0x14] = (t_faddrcc)ADC_AL_I8;
	ecpuins.table[0x15] = (t_faddrcc)ADC_AX_I16;
	ecpuins.table[0x16] = (t_faddrcc)PUSH_SS;
	ecpuins.table[0x17] = (t_faddrcc)POP_SS;
	ecpuins.table[0x18] = (t_faddrcc)SBB_RM8_R8;
	ecpuins.table[0x19] = (t_faddrcc)SBB_RM16_R16;
	ecpuins.table[0x1a] = (t_faddrcc)SBB_R8_RM8;
	ecpuins.table[0x1b] = (t_faddrcc)SBB_R16_RM16;
	ecpuins.table[0x1c] = (t_faddrcc)SBB_AL_I8;
	ecpuins.table[0x1d] = (t_faddrcc)SBB_AX_I16;
	ecpuins.table[0x1e] = (t_faddrcc)PUSH_DS;
	ecpuins.table[0x1f] = (t_faddrcc)POP_DS;
	ecpuins.table[0x20] = (t_faddrcc)AND_RM8_R8;
	ecpuins.table[0x21] = (t_faddrcc)AND_RM16_R16;
	ecpuins.table[0x22] = (t_faddrcc)AND_R8_RM8;
	ecpuins.table[0x23] = (t_faddrcc)AND_R16_RM16;
	ecpuins.table[0x24] = (t_faddrcc)AND_AL_I8;
	ecpuins.table[0x25] = (t_faddrcc)AND_AX_I16;
	ecpuins.table[0x26] = (t_faddrcc)ES;
	ecpuins.table[0x27] = (t_faddrcc)DAA;
	ecpuins.table[0x28] = (t_faddrcc)SUB_RM8_R8;
	ecpuins.table[0x29] = (t_faddrcc)SUB_RM16_R16;
	ecpuins.table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	ecpuins.table[0x2b] = (t_faddrcc)SUB_R16_RM16;
	ecpuins.table[0x2c] = (t_faddrcc)SUB_AL_I8;
	ecpuins.table[0x2d] = (t_faddrcc)SUB_AX_I16;
	ecpuins.table[0x2e] = (t_faddrcc)CS;
	ecpuins.table[0x2f] = (t_faddrcc)DAS;
	ecpuins.table[0x30] = (t_faddrcc)XOR_RM8_R8;
	ecpuins.table[0x31] = (t_faddrcc)XOR_RM16_R16;
	ecpuins.table[0x32] = (t_faddrcc)XOR_R8_RM8;
	ecpuins.table[0x33] = (t_faddrcc)XOR_R16_RM16;
	ecpuins.table[0x34] = (t_faddrcc)XOR_AL_I8;
	ecpuins.table[0x35] = (t_faddrcc)XOR_AX_I16;
	ecpuins.table[0x36] = (t_faddrcc)SS;
	ecpuins.table[0x37] = (t_faddrcc)AAA;
	ecpuins.table[0x38] = (t_faddrcc)CMP_RM8_R8;
	ecpuins.table[0x39] = (t_faddrcc)CMP_RM16_R16;
	ecpuins.table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	ecpuins.table[0x3b] = (t_faddrcc)CMP_R16_RM16;
	ecpuins.table[0x3c] = (t_faddrcc)CMP_AL_I8;
	ecpuins.table[0x3d] = (t_faddrcc)CMP_AX_I16;
	ecpuins.table[0x3e] = (t_faddrcc)DS;
	ecpuins.table[0x3f] = (t_faddrcc)AAS;
	ecpuins.table[0x40] = (t_faddrcc)INC_AX;
	ecpuins.table[0x41] = (t_faddrcc)INC_CX;
	ecpuins.table[0x42] = (t_faddrcc)INC_DX;
	ecpuins.table[0x43] = (t_faddrcc)INC_BX;
	ecpuins.table[0x44] = (t_faddrcc)INC_SP;
	ecpuins.table[0x45] = (t_faddrcc)INC_BP;
	ecpuins.table[0x46] = (t_faddrcc)INC_SI;
	ecpuins.table[0x47] = (t_faddrcc)INC_DI;
	ecpuins.table[0x48] = (t_faddrcc)DEC_AX;
	ecpuins.table[0x49] = (t_faddrcc)DEC_CX;
	ecpuins.table[0x4a] = (t_faddrcc)DEC_DX;
	ecpuins.table[0x4b] = (t_faddrcc)DEC_BX;
	ecpuins.table[0x4c] = (t_faddrcc)DEC_SP;
	ecpuins.table[0x4d] = (t_faddrcc)DEC_BP;
	ecpuins.table[0x4e] = (t_faddrcc)DEC_SI;
	ecpuins.table[0x4f] = (t_faddrcc)DEC_DI;
	ecpuins.table[0x50] = (t_faddrcc)PUSH_AX;
	ecpuins.table[0x51] = (t_faddrcc)PUSH_CX;
	ecpuins.table[0x52] = (t_faddrcc)PUSH_DX;
	ecpuins.table[0x53] = (t_faddrcc)PUSH_BX;
	ecpuins.table[0x54] = (t_faddrcc)PUSH_SP;
	ecpuins.table[0x55] = (t_faddrcc)PUSH_BP;
	ecpuins.table[0x56] = (t_faddrcc)PUSH_SI;
	ecpuins.table[0x57] = (t_faddrcc)PUSH_DI;
	ecpuins.table[0x58] = (t_faddrcc)POP_AX;
	ecpuins.table[0x59] = (t_faddrcc)POP_CX;
	ecpuins.table[0x5a] = (t_faddrcc)POP_DX;
	ecpuins.table[0x5b] = (t_faddrcc)POP_BX;
	ecpuins.table[0x5c] = (t_faddrcc)POP_SP;
	ecpuins.table[0x5d] = (t_faddrcc)POP_BP;
	ecpuins.table[0x5e] = (t_faddrcc)POP_SI;
	ecpuins.table[0x5f] = (t_faddrcc)POP_DI;
	ecpuins.table[0x60] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x61] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x62] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x63] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x64] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x65] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x66] = (t_faddrcc)OprSize;//
	ecpuins.table[0x67] = (t_faddrcc)AddrSize;//
	ecpuins.table[0x68] = (t_faddrcc)PUSH_I16;//
	ecpuins.table[0x69] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x70] = (t_faddrcc)JO_REL8;
	ecpuins.table[0x71] = (t_faddrcc)JNO_REL8;
	ecpuins.table[0x72] = (t_faddrcc)JC_REL8;
	ecpuins.table[0x73] = (t_faddrcc)JNC_REL8;
	ecpuins.table[0x74] = (t_faddrcc)JZ_REL8;
	ecpuins.table[0x75] = (t_faddrcc)JNZ_REL8;
	ecpuins.table[0x76] = (t_faddrcc)JBE_REL8;
	ecpuins.table[0x77] = (t_faddrcc)JA_REL8;
	ecpuins.table[0x78] = (t_faddrcc)JS_REL8;
	ecpuins.table[0x79] = (t_faddrcc)JNS_REL8;
	ecpuins.table[0x7a] = (t_faddrcc)JP_REL8;
	ecpuins.table[0x7b] = (t_faddrcc)JNP_REL8;
	ecpuins.table[0x7c] = (t_faddrcc)JL_REL8;
	ecpuins.table[0x7d] = (t_faddrcc)JNL_REL8;
	ecpuins.table[0x7e] = (t_faddrcc)JLE_REL8;
	ecpuins.table[0x7f] = (t_faddrcc)JG_REL8;
	ecpuins.table[0x80] = (t_faddrcc)INS_80;
	ecpuins.table[0x81] = (t_faddrcc)INS_81;
	ecpuins.table[0x82] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x83] = (t_faddrcc)INS_83;
	ecpuins.table[0x84] = (t_faddrcc)TEST_RM8_R8;
	ecpuins.table[0x85] = (t_faddrcc)TEST_RM16_R16;
	ecpuins.table[0x86] = (t_faddrcc)XCHG_R8_RM8;
	ecpuins.table[0x87] = (t_faddrcc)XCHG_R16_RM16;
	ecpuins.table[0x88] = (t_faddrcc)MOV_RM8_R8;
	ecpuins.table[0x89] = (t_faddrcc)MOV_RM16_R16;
	ecpuins.table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	ecpuins.table[0x8b] = (t_faddrcc)MOV_R16_RM16;
	ecpuins.table[0x8c] = (t_faddrcc)MOV_RM16_SEG;
	ecpuins.table[0x8d] = (t_faddrcc)LEA_R16_M16;
	ecpuins.table[0x8e] = (t_faddrcc)MOV_SEG_RM16;
	ecpuins.table[0x8f] = (t_faddrcc)POP_RM16;
	ecpuins.table[0x90] = (t_faddrcc)NOP;
	ecpuins.table[0x91] = (t_faddrcc)XCHG_CX_AX;
	ecpuins.table[0x92] = (t_faddrcc)XCHG_DX_AX;
	ecpuins.table[0x93] = (t_faddrcc)XCHG_BX_AX;
	ecpuins.table[0x94] = (t_faddrcc)XCHG_SP_AX;
	ecpuins.table[0x95] = (t_faddrcc)XCHG_BP_AX;
	ecpuins.table[0x96] = (t_faddrcc)XCHG_SI_AX;
	ecpuins.table[0x97] = (t_faddrcc)XCHG_DI_AX;
	ecpuins.table[0x98] = (t_faddrcc)CBW;
	ecpuins.table[0x99] = (t_faddrcc)CWD;
	ecpuins.table[0x9a] = (t_faddrcc)CALL_PTR16_16;
	ecpuins.table[0x9b] = (t_faddrcc)WAIT;
	ecpuins.table[0x9c] = (t_faddrcc)PUSHF;
	ecpuins.table[0x9d] = (t_faddrcc)POPF;
	ecpuins.table[0x9e] = (t_faddrcc)SAHF;
	ecpuins.table[0x9f] = (t_faddrcc)LAHF;
	ecpuins.table[0xa0] = (t_faddrcc)MOV_AL_MOFFS8;
	ecpuins.table[0xa1] = (t_faddrcc)MOV_AX_MOFFS16;
	ecpuins.table[0xa2] = (t_faddrcc)MOV_MOFFS8_AL;
	ecpuins.table[0xa3] = (t_faddrcc)MOV_MOFFS16_AX;
	ecpuins.table[0xa4] = (t_faddrcc)MOVSB;
	ecpuins.table[0xa5] = (t_faddrcc)MOVSW;
	ecpuins.table[0xa6] = (t_faddrcc)CMPSB;
	ecpuins.table[0xa7] = (t_faddrcc)CMPSW;
	ecpuins.table[0xa8] = (t_faddrcc)TEST_AL_I8;
	ecpuins.table[0xa9] = (t_faddrcc)TEST_AX_I16;
	ecpuins.table[0xaa] = (t_faddrcc)STOSB;
	ecpuins.table[0xab] = (t_faddrcc)STOSW;
	ecpuins.table[0xac] = (t_faddrcc)LODSB;
	ecpuins.table[0xad] = (t_faddrcc)LODSW;
	ecpuins.table[0xae] = (t_faddrcc)SCASB;
	ecpuins.table[0xaf] = (t_faddrcc)SCASW;
	ecpuins.table[0xb0] = (t_faddrcc)MOV_AL_I8;
	ecpuins.table[0xb1] = (t_faddrcc)MOV_CL_I8;
	ecpuins.table[0xb2] = (t_faddrcc)MOV_DL_I8;
	ecpuins.table[0xb3] = (t_faddrcc)MOV_BL_I8;
	ecpuins.table[0xb4] = (t_faddrcc)MOV_AH_I8;
	ecpuins.table[0xb5] = (t_faddrcc)MOV_CH_I8;
	ecpuins.table[0xb6] = (t_faddrcc)MOV_DH_I8;
	ecpuins.table[0xb7] = (t_faddrcc)MOV_BH_I8;
	ecpuins.table[0xb8] = (t_faddrcc)MOV_AX_I16;
	ecpuins.table[0xb9] = (t_faddrcc)MOV_CX_I16;
	ecpuins.table[0xba] = (t_faddrcc)MOV_DX_I16;
	ecpuins.table[0xbb] = (t_faddrcc)MOV_BX_I16;
	ecpuins.table[0xbc] = (t_faddrcc)MOV_SP_I16;
	ecpuins.table[0xbd] = (t_faddrcc)MOV_BP_I16;
	ecpuins.table[0xbe] = (t_faddrcc)MOV_SI_I16;
	ecpuins.table[0xbf] = (t_faddrcc)MOV_DI_I16;
	ecpuins.table[0xc0] = (t_faddrcc)INS_C0;//
	ecpuins.table[0xc1] = (t_faddrcc)INS_C1;//
	ecpuins.table[0xc2] = (t_faddrcc)RET_I16;
	ecpuins.table[0xc3] = (t_faddrcc)RET;
	ecpuins.table[0xc4] = (t_faddrcc)LES_R16_M16;
	ecpuins.table[0xc5] = (t_faddrcc)LDS_R16_M16;
	ecpuins.table[0xc6] = (t_faddrcc)INS_C6;
	ecpuins.table[0xc7] = (t_faddrcc)INS_C7;
	ecpuins.table[0xc8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xc9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xca] = (t_faddrcc)RETF_I16;
	ecpuins.table[0xcb] = (t_faddrcc)RETF;
	ecpuins.table[0xcc] = (t_faddrcc)INT3;
	ecpuins.table[0xcd] = (t_faddrcc)INT_I8;
	ecpuins.table[0xce] = (t_faddrcc)INTO;
	ecpuins.table[0xcf] = (t_faddrcc)IRET;
	ecpuins.table[0xd0] = (t_faddrcc)INS_D0;
	ecpuins.table[0xd1] = (t_faddrcc)INS_D1;
	ecpuins.table[0xd2] = (t_faddrcc)INS_D2;
	ecpuins.table[0xd3] = (t_faddrcc)INS_D3;
	ecpuins.table[0xd4] = (t_faddrcc)AAM;
	ecpuins.table[0xd5] = (t_faddrcc)AAD;
	ecpuins.table[0xd6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xd7] = (t_faddrcc)XLAT;
	ecpuins.table[0xd8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xd9] = (t_faddrcc)UndefinedOpcode;//INS_D9;
	ecpuins.table[0xda] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xdb] = (t_faddrcc)UndefinedOpcode;//INS_DB;
	ecpuins.table[0xdc] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xdd] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xde] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xdf] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xe0] = (t_faddrcc)LOOPNZ;
	ecpuins.table[0xe1] = (t_faddrcc)LOOPZ;
	ecpuins.table[0xe2] = (t_faddrcc)LOOP;
	ecpuins.table[0xe3] = (t_faddrcc)JCXZ_REL8;
	ecpuins.table[0xe4] = (t_faddrcc)IN_AL_I8;
	ecpuins.table[0xe5] = (t_faddrcc)IN_AX_I8;
	ecpuins.table[0xe6] = (t_faddrcc)OUT_I8_AL;
	ecpuins.table[0xe7] = (t_faddrcc)OUT_I8_AX;
	ecpuins.table[0xe8] = (t_faddrcc)CALL_REL16;
	ecpuins.table[0xe9] = (t_faddrcc)JMP_REL16;
	ecpuins.table[0xea] = (t_faddrcc)JMP_PTR16_16;
	ecpuins.table[0xeb] = (t_faddrcc)JMP_REL8;
	ecpuins.table[0xec] = (t_faddrcc)IN_AL_DX;
	ecpuins.table[0xed] = (t_faddrcc)IN_AX_DX;
	ecpuins.table[0xee] = (t_faddrcc)OUT_DX_AL;
	ecpuins.table[0xef] = (t_faddrcc)OUT_DX_AX;
	ecpuins.table[0xf0] = (t_faddrcc)LOCK;
	ecpuins.table[0xf1] = (t_faddrcc)QDX;
	ecpuins.table[0xf2] = (t_faddrcc)REPNZ;
	ecpuins.table[0xf3] = (t_faddrcc)REPZ;
	ecpuins.table[0xf4] = (t_faddrcc)HLT;
	ecpuins.table[0xf5] = (t_faddrcc)CMC;
	ecpuins.table[0xf6] = (t_faddrcc)INS_F6;
	ecpuins.table[0xf7] = (t_faddrcc)INS_F7;
	ecpuins.table[0xf8] = (t_faddrcc)CLC;
	ecpuins.table[0xf9] = (t_faddrcc)STC;
	ecpuins.table[0xfa] = (t_faddrcc)CLI;
	ecpuins.table[0xfb] = (t_faddrcc)STI;
	ecpuins.table[0xfc] = (t_faddrcc)CLD;
	ecpuins.table[0xfd] = (t_faddrcc)STD;
	ecpuins.table[0xfe] = (t_faddrcc)INS_FE;
	ecpuins.table[0xff] = (t_faddrcc)INS_FF;
	ecpuins.table_0f[0x00] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x01] = (t_faddrcc)INS_0F_01;
	ecpuins.table_0f[0x02] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x03] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x04] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x05] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x06] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x07] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x08] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x09] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x0f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x10] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x11] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x12] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x13] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x14] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x15] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x16] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x17] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x18] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x19] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x1f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x20] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x21] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x22] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x23] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x24] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x25] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x26] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x27] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x28] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x29] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x2f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x30] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x31] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x32] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x33] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x34] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x35] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x36] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x37] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x38] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x39] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x3f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x40] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x41] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x42] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x43] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x44] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x45] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x46] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x47] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x48] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x49] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x4f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x50] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x51] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x52] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x53] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x54] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x55] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x56] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x57] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x58] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x59] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x5f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x60] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x61] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x62] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x63] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x64] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x65] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x66] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x67] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x68] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x69] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x6f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x70] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x71] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x72] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x73] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x74] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x75] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x76] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x77] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x78] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x79] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x7f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x80] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x81] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x82] = (t_faddrcc)JC_REL16;//
	ecpuins.table_0f[0x83] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x84] = (t_faddrcc)JZ_REL16;//
	ecpuins.table_0f[0x85] = (t_faddrcc)JNZ_REL16;//
	ecpuins.table_0f[0x86] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x87] = (t_faddrcc)JA_REL16;//
	ecpuins.table_0f[0x88] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x89] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x8f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x90] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x91] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x92] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x93] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x94] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x95] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x96] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x97] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x98] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x99] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0x9f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa1] = (t_faddrcc)UndefinedOpcode;//POP_FS;
	ecpuins.table_0f[0xa2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xa9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xaa] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xab] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xac] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xad] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xaf] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb6] = (t_faddrcc)MOVZX_R16_RM8;//
	ecpuins.table_0f[0xb7] = (t_faddrcc)MOVZX_R32_RM16;//
	ecpuins.table_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xba] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xbb] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xbc] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xbd] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xbe] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xbf] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc7] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xc9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xca] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xcb] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xcc] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xcd] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xce] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xcf] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd7] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xd9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xda] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xdb] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xdc] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xdd] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xde] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xdf] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe7] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xe9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xea] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xeb] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xec] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xed] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xee] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xef] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf2] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf3] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf4] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf5] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf6] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf7] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf8] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xf9] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xfa] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xfb] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xfc] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xfd] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xfe] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table_0f[0xff] = (t_faddrcc)UndefinedOpcode;
}
void ecpuinsReset() {}
void ecpuinsRefresh()
{
	ExecIns();
	ExecInt();
}
void ecpuinsFinal() {}
