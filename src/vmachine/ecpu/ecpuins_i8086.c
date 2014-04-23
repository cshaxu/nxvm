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
#include "ecpuins.h"

#include "../debug/aasm.h"
#include "../debug/dasm.h"

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

t_cpuins ecpuins;

#define bugfix(n) if(1)

#define CHECKED void
#define DIFF void
#define NOTIMP void
#define WRAPPER void

#define ASMCMP void
#define EXCEPT void

static t_cpu acpu;
static t_nubit8 ub1,ub2,ub3;
static t_nsbit8 sb1,sb2,sb3;
static t_nubit16 uw1,uw2,uw3;
static t_nsbit16 sw1,sw2,sw3;
#define aipcheck if (acpu.ip > 0xfffa && ecpu.ip < 0x0005) {\
	vapiPrint("ip overflow\n");} else
#ifdef VCPUASM

#define AFLAGS0   (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
                   VCPU_EFLAGS_DF | VCPU_EFLAGS_TF | VCPU_EFLAGS_IF)
#define AFLAGS1   (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
                   VCPU_EFLAGS_DF | VCPU_EFLAGS_TF)
#define AFLAGS2   (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
                   VCPU_EFLAGS_DF | VCPU_EFLAGS_TF)

static void acpuPrintRegs(t_cpu *xcpu)
{
	if (xcpu == &ecpu) vapiPrint("VCPU REGISTERS\n");
	else               vapiPrint("ACPU REGISTERS\n");
	vapiPrint(  "AX=%04X", xcpu->ax);
	vapiPrint("  BX=%04X", xcpu->bx);
	vapiPrint("  CX=%04X", xcpu->cx);
	vapiPrint("  DX=%04X", xcpu->dx);
	vapiPrint("  SP=%04X", xcpu->sp);
	vapiPrint("  BP=%04X", xcpu->bp);
	vapiPrint("  SI=%04X", xcpu->si);
	vapiPrint("  DI=%04X", xcpu->di);
	vapiPrint("\nDS=%04X", xcpu->ds);
	vapiPrint("  ES=%04X", xcpu->es);
	vapiPrint("  SS=%04X", xcpu->ss);
	vapiPrint("  CS=%04X", xcpu->cs);
	vapiPrint("  IP=%04X", xcpu->ip);
	vapiPrint("   ");
	if(xcpu->flags & VCPU_EFLAGS_OF) vapiPrint("OV ");
	else                           vapiPrint("NV ");
	if(xcpu->flags & VCPU_EFLAGS_DF) vapiPrint("DN ");
	else                           vapiPrint("UP ");
	if(xcpu->flags & VCPU_EFLAGS_IF) vapiPrint("E_ ");
	else                           vapiPrint("D_ ");
	if(xcpu->flags & VCPU_EFLAGS_SF) vapiPrint("NG ");
	else                           vapiPrint("PL ");
	if(xcpu->flags & VCPU_EFLAGS_ZF) vapiPrint("ZR ");
	else                           vapiPrint("NZ ");
	if(xcpu->flags & VCPU_EFLAGS_AF) vapiPrint("AC ");
	else                           vapiPrint("NA ");
	if(xcpu->flags & VCPU_EFLAGS_PF) vapiPrint("PE ");
	else                           vapiPrint("PO ");
	if(xcpu->flags & VCPU_EFLAGS_CF) vapiPrint("CY ");
	else                           vapiPrint("NC ");
	vapiPrint("\n");
}
static t_bool acpuCheck(t_nubit16 flags)
{
	t_bool flagdiff = 0x00;
	if (acpu.ax != ecpu.ax)       {vapiPrint("diff ax\n");flagdiff = 0x01;}
	if (acpu.bx != ecpu.bx)       {vapiPrint("diff bx\n");flagdiff = 0x01;}
	if (acpu.cx != ecpu.cx)       {vapiPrint("diff cx\n");flagdiff = 0x01;}
	if (acpu.dx != ecpu.dx)       {vapiPrint("diff dx\n");flagdiff = 0x01;}
	if (acpu.sp != ecpu.sp)       {vapiPrint("diff sp\n");flagdiff = 0x01;}
	if (acpu.bp != ecpu.bp)       {vapiPrint("diff bp\n");flagdiff = 0x01;}
	if (acpu.si != ecpu.si)       {vapiPrint("diff si\n");flagdiff = 0x01;}
	if (acpu.di != ecpu.di)       {vapiPrint("diff di\n");flagdiff = 0x01;}
	if (acpu.ip != ecpu.ip)       {vapiPrint("diff ip\n");flagdiff = 0x01;}
	if (acpu.cs != ecpu.cs.selector)       {vapiPrint("diff cs\n");flagdiff = 0x01;}
	if (acpu.ds != ecpu.ds.selector)       {vapiPrint("diff ds\n");flagdiff = 0x01;}
	if (acpu.es != ecpu.es.selector)       {vapiPrint("diff es\n");flagdiff = 0x01;}
	if (acpu.ss != ecpu.ss.selector)       {vapiPrint("diff ss\n");flagdiff = 0x01;}
	if ((acpu.flags & flags) != (ecpu.flags & flags))
		                          {vapiPrint("diff fg\n");flagdiff = 0x01;}
	if (flagdiff) {
		acpuPrintRegs(&acpu);
		acpuPrintRegs(&ecpu);
		vapiCallBackMachineStop();
	}
	return flagdiff;
}
#define async    if(acpu = ecpu,1)
#define aexec    __asm
#define acheck(f)   if(acpuCheck(f))
#define async8   if(1) {\
	acpu = ecpu;ub1 = d_nubit8(dest);ub2 = d_nubit8(src);} else
#define async12  if(1) {\
	acpu = ecpu;uw1 = d_nubit16(dest);uw2 = d_nsbit8(src);} else
#define async16  if(1) {\
	acpu = ecpu;uw1 = d_nubit16(dest);uw2 = d_nubit16(src);} else
#define asreg8   if(!vramIsAddrInMem(dest)) {\
	d_nubit8((t_vaddrcc)dest - (t_vaddrcc)&ecpu + (t_vaddrcc)&acpu) = ub3;} else
#define asreg16  if(!vramIsAddrInMem(dest)) {\
	d_nubit16((t_vaddrcc)dest - (t_vaddrcc)&ecpu + (t_vaddrcc)&acpu) = uw3;} else
#define asreg12  asreg16
#define acheck8(f)  if(acpuCheck(f) || d_nubit8(dest) != ub3) {\
	vapiPrint("ub1=%02X,ub2=%02X,ub3=%02X,dest=%02X\n",\
		ub1,ub2,ub3,d_nubit8(dest));} else
#define acheck16(f) if(acpuCheck(f) || d_nubit16(dest) != uw3) {\
	vapiPrint("uw1=%04X,uw2=%04X,uw3=%04X,dest=%04X\n",\
		uw1,uw2,uw3,d_nubit16(dest));} else
#define acheck12(f) acheck16(f)
#define aexec8  if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push acpu.flags\
	__asm popf\
	__asm mov al,ub1\
	__asm op  al,ub2\
	__asm mov ub3,al\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop eax\
	__asm popfd} else
#define aexec16  if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push acpu.flags\
	__asm popf\
	__asm mov ax,uw1\
	__asm op  ax,uw2\
	__asm mov uw3,ax\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop eax\
	__asm popfd} else
#define aexec12 aexec16
#define asyncall switch(len) {\
	case 8:  async8; break;\
	case 12: async12;break;\
	case 16: async16;break;}
#define aexecall switch(len) {\
	case 8: aexec8; break;\
	case 12:aexec12;break;\
	case 16:aexec16;break;}
#define asregall switch(len) {\
	case 8: asreg8; break;\
	case 12:asreg12;break;\
	case 16:asreg16;break;}
#define acheckall(f) switch(len) {\
	case 8: acheck8(f); break;\
	case 12:acheck12(f);break;\
	case 16:acheck16(f);break;}

#define aexec_csf if(1) {\
	__asm pushfd\
	__asm push acpu.flags\
	__asm popf\
	__asm op\
	__asm pushf\
	__asm pop acpu.flags\
	__asm popfd} else
#define aexec_idc8 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push acpu.flags\
	__asm popf\
	__asm mov al,ub1\
	__asm op al\
	__asm mov ub3,al\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop eax\
	__asm popfd} else
#define aexec_idc16 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push acpu.flags\
	__asm popf\
	__asm mov ax,uw1\
	__asm op ax\
	__asm mov uw3,ax\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop eax\
	__asm popfd} else
#define aexec_srd8 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push ecx\
	__asm push acpu.flags\
	__asm popf\
	__asm mov al,ub1\
	__asm mov cl,ub2\
	__asm op al,cl\
	__asm mov ub3,al\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop ecx\
	__asm pop eax\
	__asm popfd} else
#define aexec_srd16 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push ecx\
	__asm push acpu.flags\
	__asm popf\
	__asm mov ax,uw1\
	__asm mov cl,ub2\
	__asm op ax,cl\
	__asm mov uw3,ax\
	__asm pushf\
	__asm pop acpu.flags\
	__asm pop ecx\
	__asm pop eax\
	__asm popfd} else
#define aexec_cax8 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm mov al, acpu.al\
	__asm push acpu.flags\
	__asm popf\
	__asm op\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov acpu.al, al\
	__asm pop eax\
	__asm popfd} else
#define aexec_cax16 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm push edx\
	__asm mov ax, acpu.ax\
	__asm mov dx, acpu.dx\
	__asm push acpu.flags\
	__asm popf\
	__asm op\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov acpu.dx, dx\
	__asm mov acpu.ax, ax\
	__asm pop edx\
	__asm pop eax\
	__asm popfd} else
#define aexec_nxx8 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm mov al, ub1\
	__asm push acpu.flags\
	__asm popf\
	__asm op al\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov ub3, al\
	__asm pop eax\
	__asm popfd} else
#define aexec_nxx16 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm mov ax, uw1\
	__asm push acpu.flags\
	__asm popf\
	__asm op ax\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov uw3, ax\
	__asm pop eax\
	__asm popfd} else
#define aexec_mdx8 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm mov ax, acpu.ax\
	__asm push acpu.flags\
	__asm popf\
	__asm op ub2\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov acpu.ax, ax\
	__asm pop eax\
	__asm popfd} else
#define aexec_mdx16 if(1) {\
	__asm pushfd\
	__asm push eax\
	__asm mov ax, acpu.ax\
	__asm mov dx, acpu.dx\
	__asm push acpu.flags\
	__asm popf\
	__asm op uw2\
	__asm pushf\
	__asm pop acpu.flags\
	__asm mov acpu.dx, dx\
	__asm mov acpu.ax, ax\
	__asm pop eax\
	__asm popfd} else
#else
#define async     if(0)
#define aexec     __asm
#define acheck(f) if(0)
#define async8    if(0)
#define async12   if(0)
#define async16   if(0)
#define asreg8    if(0)
#define asreg12   if(0)
#define asreg16   if(0)
#define acheck8(f)  if(0)
#define acheck12(f) if(0)
#define acheck16(f) if(0)
#define aexec8    if(0)
#define aexec12   if(0)
#define aexec16   if(0)
#define asyncall
#define aexecall
#define asregall
#define acheckall(f)
#define aexec_csf   if(0)
#define aexec_idc8  if(0)
#define aexec_idc16 if(0)
#define aexec_srd8  if(0)
#define aexec_srd16 if(0)
#define aexec_cax8  if(0)
#define aexec_cax16 if(0)
#define aexec_nxx8  if(0)
#define aexec_nxx16 if(0)
#define aexec_mdx8  if(0)
#define aexec_mdx16 if(0)
#endif

static void CaseError(const char *str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: %s.\n",str);
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
	case 8:	MakeBit(ecpu.flags,VCPU_EFLAGS_SF,!!(ecpuins.result&0x80));break;
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
	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,vramRealWord(ecpu.cs.selector,ecpu.ip));
	ecpu.ip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns ecpuins.rimm
	ecpuins.rimm = vramGetRealAddr(ecpu.cs.selector,ecpu.ip);
	switch(immbit) {
	case 8:		ecpu.ip += 1;break;
	case 16:	ecpu.ip += 2;break;
	case 32:	ecpu.ip += 4;break;
	default:CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns ecpuins.rrm and ecpuins.rr
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramRealByte(ecpu.cs.selector,ecpu.ip++);
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.si);break;
		case 1:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.di);break;
		case 2:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.si);break;
		case 3:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.di);break;
		case 4:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.si);break;
		case 5:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.di);break;
		case 6: disp16 = vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;
			    ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,disp16); break;
		case 7:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);ecpu.ip += 1;}
		switch(RM) {
		case 0:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.si+disp8);break;
		case 1:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.di+disp8);break;
		case 2:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.si+disp8);break;
		case 3:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.di+disp8);break;
		case 4:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.si+disp8);break;
		case 5:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.di+disp8);break;
		case 6:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+disp8);break;
		case 7:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+disp8);break;
		default:CaseError("GetModRegRM::MOD1::RM");break;}
		bugfix(23) ;
		else {
			bugfix(3) ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);
			else ecpuins.rrm += vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;}
		switch(RM) {
		case 0:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.si+disp16);break;
		case 1:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+ecpu.di+disp16);break;
		case 2:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.si+disp16);break;
		case 3:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+ecpu.di+disp16);break;
		case 4:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.si+disp16);break;
		case 5:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.di+disp16);break;
		case 6:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp+disp16);break;
		case 7:	ecpuins.rrm = vramGetRealAddr(ecpuins.roverds->selector,ecpu.bx+disp16);break;
		default:CaseError("GetModRegRM::MOD2::RM");break;}
		bugfix(23) ;
		else {ecpuins.rrm += vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;}
		break;
	case 3:
		switch(RM) {
		case 0:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.al); else ecpuins.rrm = (t_vaddrcc)(&ecpu.ax); break;
		case 1:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.cl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.cx); break;
		case 2:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.dl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.dx); break;
		case 3:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.bl); else ecpuins.rrm = (t_vaddrcc)(&ecpu.bx); break;
		case 4:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.ah); else ecpuins.rrm = (t_vaddrcc)(&ecpu.sp); break;
		case 5:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.ch); else ecpuins.rrm = (t_vaddrcc)(&ecpu.bp); break;
		case 6:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.dh); else ecpuins.rrm = (t_vaddrcc)(&ecpu.si); break;
		case 7:	if(rmbit == 8) ecpuins.rrm = (t_vaddrcc)(&ecpu.bh); else ecpuins.rrm = (t_vaddrcc)(&ecpu.di); break;
		default:CaseError("GetModRegRM::MOD3::RM");break;}
		break;
	default:CaseError("GetModRegRM::MOD");break;}
	switch(regbit) {
	case 0:		ecpuins.rr = REG;					break;
	case 4:
		switch(REG) {
		case 0:	ecpuins.rr = (t_vaddrcc)(&ecpu.es.selector);	break;
		case 1:	ecpuins.rr = (t_vaddrcc)(&ecpu.cs.selector);	break;
		case 2:	ecpuins.rr = (t_vaddrcc)(&ecpu.ss.selector);	break;
		case 3:	ecpuins.rr = (t_vaddrcc)(&ecpu.ds.selector);	break;
		default:CaseError("GetModRegRM::regbit4::REG");break;}
		break;
	case 8:
		switch(REG) {
		case 0:	ecpuins.rr = (t_vaddrcc)(&ecpu.al);	break;
		case 1:	ecpuins.rr = (t_vaddrcc)(&ecpu.cl);	break;
		case 2:	ecpuins.rr = (t_vaddrcc)(&ecpu.dl);	break;
		case 3:	ecpuins.rr = (t_vaddrcc)(&ecpu.bl);	break;
		case 4:	ecpuins.rr = (t_vaddrcc)(&ecpu.ah);	break;
		case 5:	ecpuins.rr = (t_vaddrcc)(&ecpu.ch);	break;
		case 6:	ecpuins.rr = (t_vaddrcc)(&ecpu.dh);	break;
		case 7:	ecpuins.rr = (t_vaddrcc)(&ecpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::REG");break;}
		break;
	case 16:
		switch(REG) {
		case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.ax);	break;
		case 1:	ecpuins.rr = (t_vaddrcc)(&ecpu.cx);	break;
		case 2:	ecpuins.rr = (t_vaddrcc)(&ecpu.dx);	break;
		case 3:	ecpuins.rr = (t_vaddrcc)(&ecpu.bx);	break;
		case 4:	ecpuins.rr = (t_vaddrcc)(&ecpu.sp);	break;
		case 5:	ecpuins.rr = (t_vaddrcc)(&ecpu.bp);	break;
		case 6:	ecpuins.rr = (t_vaddrcc)(&ecpu.si);	break;
		case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramRealByte(ecpu.cs.selector,ecpu.ip++);
	ecpuins.rrm = ecpuins.rr = (t_vaddrcc)NULL;
	switch(MOD) {
	case 0:
		switch(RM) {
		case 0:	ecpuins.rrm = ecpu.bx+ecpu.si;break;
		case 1:	ecpuins.rrm = ecpu.bx+ecpu.di;break;
		case 2:	ecpuins.rrm = ecpu.bp+ecpu.si;break;
		case 3:	ecpuins.rrm = ecpu.bp+ecpu.di;break;
		case 4:	ecpuins.rrm = ecpu.si;break;
		case 5:	ecpuins.rrm = ecpu.di;break;
		case 6:	ecpuins.rrm = vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;break;
		case 7:	ecpuins.rrm = ecpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);ecpu.ip += 1;}
		switch(RM) {
		case 0:	ecpuins.rrm = ecpu.bx+ecpu.si+disp8;break;
		case 1:	ecpuins.rrm = ecpu.bx+ecpu.di+disp8;break;
		case 2:	ecpuins.rrm = ecpu.bp+ecpu.si+disp8;break;
		case 3:	ecpuins.rrm = ecpu.bp+ecpu.di+disp8;break;
		case 4:	ecpuins.rrm = ecpu.si+disp8;break;
		case 5:	ecpuins.rrm = ecpu.di+disp8;break;
		case 6:	ecpuins.rrm = ecpu.bp+disp8;break;
		case 7:	ecpuins.rrm = ecpu.bx+disp8;break;
		default:CaseError("GetModRegRMEA::MOD1::RM");break;}
		bugfix(23) {ecpuins.rrm %= 0x10000;}
		else {
			bugfix(3) ecpuins.rrm += (t_nsbit8)vramRealByte(ecpu.cs.selector,ecpu.ip);
			else ecpuins.rrm += vramRealByte(ecpu.cs.selector,ecpu.ip);
			ecpu.ip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;}
		switch(RM) {
		case 0:	ecpuins.rrm = ecpu.bx+ecpu.si+disp16;break;
		case 1:	ecpuins.rrm = ecpu.bx+ecpu.di+disp16;break;
		case 2:	ecpuins.rrm = ecpu.bp+ecpu.si+disp16;break;
		case 3:	ecpuins.rrm = ecpu.bp+ecpu.di+disp16;break;
		case 4:	ecpuins.rrm = ecpu.si+disp16;break;
		case 5:	ecpuins.rrm = ecpu.di+disp16;break;
		case 6:
			bugfix(14) ecpuins.rrm = ecpu.bp+disp16;
			else ecpuins.rrm = vramGetRealAddr(ecpuins.roverss->selector,ecpu.bp);
			break;
		case 7:	ecpuins.rrm = ecpu.bx+disp16;break;
		default:CaseError("GetModRegRMEA::MOD2::RM");break;}
		bugfix(23) {ecpuins.rrm %= 0x10000;}
		else {ecpuins.rrm += vramRealWord(ecpu.cs.selector,ecpu.ip);ecpu.ip += 2;}
		break;
	default:CaseError("GetModRegRMEA::MOD");break;}
	switch(REG) {
	case 0: ecpuins.rr = (t_vaddrcc)(&ecpu.ax);	break;
	case 1:	ecpuins.rr = (t_vaddrcc)(&ecpu.cx);	break;
	case 2:	ecpuins.rr = (t_vaddrcc)(&ecpu.dx);	break;
	case 3:	ecpuins.rr = (t_vaddrcc)(&ecpu.bx);	break;
	case 4:	ecpuins.rr = (t_vaddrcc)(&ecpu.sp);	break;
	case 5:	ecpuins.rr = (t_vaddrcc)(&ecpu.bp);	break;
	case 6:	ecpuins.rr = (t_vaddrcc)(&ecpu.si);	break;
	case 7: ecpuins.rr = (t_vaddrcc)(&ecpu.di);	break;
	default:CaseError("GetModRegRMEA::REG");break;}
}

static void INT(t_nubit8 intid);
static ASMCMP ADD(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op add
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP OR(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op or
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP ADC(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op adc
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP SBB(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op sbb
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP AND(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op and
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static EXCEPT SUB(void *dest, void *src, t_nubit8 len)
{
//	asyncall
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
//#define op sub
//	aexecall
//#undef op
//	asregall
//	acheckall(AFLAGS1)
}
static ASMCMP XOR(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
#define op xor
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP CMP(void *dest, void *src, t_nubit8 len)
{
	asyncall
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
	default:CaseError("CMP::len");break;}
	SetFlags(CMP_FLAG);
#define op cmp
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data = d_nubit16(src);
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		ecpu.sp -= 2;
		bugfix(13) vramRealWord(ecpu.ss.selector,ecpu.sp) = data;
		else vramRealWord(ecpu.ss.selector,ecpu.sp) = d_nubit16(src);
		break;
	default:CaseError("PUSH::len");break;}
}
static void POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		ecpuins.bit = 16;
		d_nubit16(dest) = vramRealWord(ecpu.ss.selector,ecpu.sp);
		ecpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
static ASMCMP INC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op inc
		aexec_idc8;
#undef op
		ecpuins.bit = 8;
		ecpuins.type = ADD8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = 0x01;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op inc
		aexec_idc16;
#undef op
		ecpuins.bit = 16;
		ecpuins.type = ADD16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = 0x0001;
		ecpuins.result = (ecpuins.opr1+ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("INC::len");break;}
	SetFlags(INC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP DEC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op dec
		aexec_idc8;
#undef op
		ecpuins.bit = 8;
		ecpuins.type = SUB8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = 0x01;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op dec
		aexec_idc16;
#undef op
		ecpuins.bit = 16;
		ecpuins.type = SUB16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = 0x0001;
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.result;
		break;
	default:CaseError("DEC::len");break;}
	SetFlags(DEC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static void JCC(void *src, t_bool flagj,t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		if(flagj)
			bugfix(5) ecpu.ip += d_nsbit8(src);
			else ecpu.ip += d_nubit8(src);
		break;
	default:CaseError("JCC::len");break;}
}
static ASMCMP TEST(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		//ecpuins.type = TEST8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2)&0xff;
		break;
	case 16:
		ecpuins.bit = 16;
		//ecpuins.type = TEST16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		ecpuins.result = (ecpuins.opr1&ecpuins.opr2)&0xffff;
		break;
	default:CaseError("TEST::len");break;}
	ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
	ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	SetFlags(TEST_FLAG);
#define op test
	aexecall
#undef op
	acheckall(AFLAGS1)
}
static void XCHG(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.opr1 = d_nubit8(dest) & 0xff;
		ecpuins.opr2 = d_nubit8(src) & 0xff;
		d_nubit8(dest) = (t_nubit8)ecpuins.opr2;
		d_nubit8(src) = (t_nubit8)ecpuins.opr1;
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.opr1 = d_nubit16(dest) & 0xffff;
		ecpuins.opr2 = d_nubit16(src) & 0xffff;
		d_nubit16(dest) = (t_nubit16)ecpuins.opr2;
		d_nubit16(src) = (t_nubit16)ecpuins.opr1;
		break;
	default:CaseError("XCHG::len");break;}
}
static void MOV(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		d_nubit8(dest) = d_nubit8(src);
		break;
	case 16:
		ecpuins.bit = 16;
		d_nubit16(dest) = d_nubit16(src);
		break;
	default:CaseError("MOV::len");break;}
}
static ASMCMP ROL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 8;
		ecpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+(t_nubit8)tempCF;
			tempcount--;
		}
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
#define op rol
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 16;
		ecpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
#define op rol
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP ROR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 8;
		ecpuins.bit = 8;
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(tempCF) d_nubit8(dest) |= 0x80;
			tempcount--;
		}
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^(!!(d_nubit8(dest)&0x40)));
#define op ror
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 16;
		ecpuins.bit = 16;
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(tempCF) d_nubit16(dest) |= 0x8000;
			tempcount--;
		}
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^(!!(d_nubit16(dest)&0x4000)));
#define op ror
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP RCL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 9;
		ecpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+GetBit(ecpu.flags, VCPU_EFLAGS_CF);
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
#define op rcl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 17;
		ecpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+GetBit(ecpu.flags, VCPU_EFLAGS_CF);
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
#define op rcl
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP RCR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 9;
		ecpuins.bit = 8;
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(GetBit(ecpu.flags, VCPU_EFLAGS_CF)) d_nubit8(dest) |= 0x80;
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
#define op rcr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 17;
		ecpuins.bit = 16;
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(GetBit(ecpu.flags, VCPU_EFLAGS_CF)) d_nubit16(dest) |= 0x8000;
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
#define op rcr
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SHL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		ecpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		ecpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHL::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SHR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		ecpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!(tempdest8&0x80));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		ecpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!(tempdest16&0x8000));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHR::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SAL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		ecpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		ecpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^GetBit(ecpu.flags, VCPU_EFLAGS_CF));
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAL::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SAR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		ecpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= tempdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,0);
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		ecpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(ecpu.flags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= tempdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(ecpu.flags,VCPU_EFLAGS_OF,0);
		bugfix(8) {
			if(count != 0) {
				ecpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				ecpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAR::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
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
		vramRealByte(ecpu.es.selector,ecpu.di) = vramRealByte(ecpuins.roverds->selector,ecpu.si);
		STRDIR(8,1,1);
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es.selector,ecpu.di) = vramRealWord(ecpuins.roverds->selector,ecpu.si);
		STRDIR(16,1,1);
		break;
	default:CaseError("MOVS::len");break;}
}
static void CMPS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpuins.type = CMP8;
		ecpuins.opr1 = vramRealByte(ecpuins.roverds->selector,ecpu.si);
		ecpuins.opr2 = vramRealByte(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = vramRealWord(ecpuins.roverds->selector,ecpu.si);
		ecpuins.opr2 = vramRealWord(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("CMPS::len");break;}
}
static void STOS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		vramRealByte(ecpu.es.selector,ecpu.di) = ecpu.al;
		STRDIR(8,0,1);
		break;
	case 16:
		ecpuins.bit = 16;
		vramRealWord(ecpu.es.selector,ecpu.di) = ecpu.ax;
		STRDIR(16,0,1);
		break;
	default:CaseError("STOS::len");break;}
}
static void LODS(t_nubit8 len)
{
	switch(len) {
	case 8:
		ecpuins.bit = 8;
		ecpu.al = vramRealByte(ecpuins.roverds->selector,ecpu.si);
		STRDIR(8,1,0);
		break;
	case 16:
		ecpuins.bit = 16;
		ecpu.ax = vramRealWord(ecpuins.roverds->selector,ecpu.si);
		STRDIR(16,1,0);
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
		ecpuins.opr2 = vramRealByte(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		ecpuins.bit = 16;
		ecpuins.type = CMP16;
		ecpuins.opr1 = ecpu.ax;
		ecpuins.opr2 = vramRealWord(ecpu.es.selector,ecpu.di);
		ecpuins.result = (ecpuins.opr1-ecpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::len");break;}
}
static ASMCMP NOT(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
		ecpuins.bit = 8;
		d_nubit8(dest) = ~d_nubit8(dest);
#define op not
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		ecpuins.bit = 16;
		d_nubit16(dest) = ~d_nubit16(dest);
#define op not
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NOT::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP NEG(void *dest, t_nubit8 len)
{
	t_nubitcc zero = 0;
	switch(len) {
	case 8:	
		async ub1 = d_nubit8(dest);
		ecpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		d_nubit8(dest) = (t_nubit8)zero;
#define op neg
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		ecpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		d_nubit16(dest) = (t_nubit16)zero;
#define op neg
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NEG::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP MUL(void *src, t_nubit8 len)
{
	t_nubit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		ecpuins.bit = 8;
		ecpu.ax = ecpu.al * d_nubit8(src);
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!ecpu.ah);
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,!!ecpu.ah);
#define op mul
		aexec_mdx8;
#undef op
		break;
	case 16:
		async uw2 = d_nubit16(src);
		ecpuins.bit = 16;
		tempresult = ecpu.ax * d_nubit16(src);
		ecpu.dx = (tempresult>>16)&0xffff;
		ecpu.ax = tempresult&0xffff;
		MakeBit(ecpu.flags,VCPU_EFLAGS_OF,!!ecpu.dx);
		MakeBit(ecpu.flags,VCPU_EFLAGS_CF,!!ecpu.dx);
#define op mul
		aexec_mdx16;
#undef op
		break;
	default:CaseError("MUL::len");break;}
	acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+MUL");
}
static ASMCMP IMUL(void *src, t_nubit8 len)
{
	t_nsbit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		ecpuins.bit = 8;
		ecpu.ax = (t_nsbit8)ecpu.al * d_nsbit8(src);
		if(ecpu.ax == ecpu.al) {
			ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
			ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(ecpu.flags,VCPU_EFLAGS_OF);
			SetBit(ecpu.flags,VCPU_EFLAGS_CF);
		}
#define op imul
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+IMUL");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		ecpuins.bit = 16;
		tempresult = (t_nsbit16)ecpu.ax * d_nsbit16(src);
		ecpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		ecpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)ecpu.ax) {
			ClrBit(ecpu.flags,VCPU_EFLAGS_OF);
			ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		} else {
			SetBit(ecpu.flags,VCPU_EFLAGS_OF);
			SetBit(ecpu.flags,VCPU_EFLAGS_CF);
		}
#define op imul
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+IMUL");
		break;
	default:CaseError("IMUL::len");break;}
}
static ASMCMP DIV(void *src, t_nubit8 len)
{
	t_nubit16 tempAX = ecpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)ecpu.dx)<<16)+ecpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		ecpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			ecpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			ecpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
#define op div
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+DIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		ecpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			ecpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			ecpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
#define op div
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+DIV");
		break;
	default:CaseError("DIV::len");break;}
}
static void IDIV(void *src, t_nubit8 len)
{
	t_nsbit16 tempAX = ecpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)ecpu.dx)<<16)+ecpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		ecpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			ecpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			ecpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
#define op idiv
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+IDIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		ecpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			ecpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			ecpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
#define op idiv
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(ecpu.cs.selector,ecpu.ip,"+IDIV");
		break;
	default:CaseError("IDIV::len");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&ecpu.flags,16);
	ClrBit(ecpu.flags, (VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
	PUSH((void *)&ecpu.cs.selector,16);
	PUSH((void *)&ecpu.ip,16);
	ecpu.ip = vramRealWord(0x0000,intid*4+0);
	ecpu.cs.selector = vramRealWord(0x0000,intid*4+2);
}

CHECKED UndefinedOpcode()
{
	vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
	vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
		ecpu.cs.selector, ecpu.ip, vramRealByte(ecpu.cs.selector,ecpu.ip+0),
		vramRealByte(ecpu.cs.selector,ecpu.ip+1), vramRealByte(ecpu.cs.selector,ecpu.ip+2),
		vramRealByte(ecpu.cs.selector,ecpu.ip+3), vramRealByte(ecpu.cs.selector,ecpu.ip+4));
	vapiCallBackMachineStop();
}
CHECKED ADD_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED ADD_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED ADD_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	ADD((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED ADD_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	ADD((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED ADD_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	ADD((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED ADD_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	ADD((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED PUSH_ES()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.es.selector,16);
}
CHECKED POP_ES()
{
	ecpu.ip++;
	POP((void *)&ecpu.es.selector,16);
}
CHECKED OR_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	OR((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED OR_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	OR((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED OR_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	OR((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED OR_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	OR((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED OR_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	OR((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED OR_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	OR((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED PUSH_CS()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.cs.selector,16);
}
CHECKED POP_CS()
{
	ecpu.ip++;
	POP((void *)&ecpu.cs.selector,16);
}
DIFF INS_0F()
{
	UndefinedOpcode();
}
CHECKED ADC_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED ADC_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED ADC_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	ADC((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED ADC_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	ADC((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED ADC_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	ADC((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED ADC_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	ADC((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED PUSH_SS()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.ss.selector,16);
}
CHECKED POP_SS()
{
	ecpu.ip++;
	POP((void *)&ecpu.ss.selector,16);
}
CHECKED SBB_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED SBB_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED SBB_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	SBB((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED SBB_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	SBB((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED SBB_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	SBB((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED SBB_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	SBB((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED PUSH_DS()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.ds.selector,16);
}
CHECKED POP_DS()
{
	ecpu.ip++;
	POP((void *)&ecpu.ds.selector,16);
}
CHECKED AND_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	AND((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED AND_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	AND((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED AND_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	AND((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED AND_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	AND((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED AND_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	AND((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED AND_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	AND((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED ES()
{
	ecpu.ip++;
	ecpuins.roverds = &ecpu.es;
	ecpuins.roverss = &ecpu.es;
}
ASMCMP DAA()
{
	t_nubit8 oldAL = ecpu.al;
	t_nubit8 newAL = ecpu.al + 0x06;
	ecpu.ip++;
	async;
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
	bugfix(18) {
		ecpuins.bit = 8;
		ecpuins.result = (t_nubitcc)ecpu.al;
		SetFlags(DAA_FLAG);
	} else ;
#define op daa
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"DAA");
}
CHECKED SUB_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED SUB_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED SUB_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	SUB((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED SUB_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	SUB((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED SUB_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	SUB((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED SUB_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	SUB((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED CS()
{
	ecpu.ip++;
	ecpuins.roverds = &ecpu.cs;
	ecpuins.roverss = &ecpu.cs;
}
ASMCMP DAS()
{
	t_nubit8 oldAL = ecpu.al;
	ecpu.ip++;
	async;
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
#define op das
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"DAS");
}
CHECKED XOR_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
CHECKED XOR_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
CHECKED XOR_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	XOR((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
CHECKED XOR_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	XOR((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
CHECKED XOR_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	XOR((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
CHECKED XOR_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	XOR((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
CHECKED SS()
{
	ecpu.ip++;
	ecpuins.roverds = &ecpu.ss;
	ecpuins.roverss = &ecpu.ss;
}
ASMCMP AAA()
{
	ecpu.ip++;
	async;
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
#define op aaa
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"AAA");
}
WRAPPER CMP_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
WRAPPER CMP_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
WRAPPER CMP_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
WRAPPER CMP_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	CMP((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
WRAPPER CMP_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	CMP((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
WRAPPER CMP_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	CMP((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
DIFF DS()
{
	ecpu.ip++;
	ecpuins.roverds = &ecpu.ds;
	ecpuins.roverss = &ecpu.ds;
}
ASMCMP AAS()
{
	ecpu.ip++;
	async;
	if(((ecpu.al&0x0f) > 0x09) || GetBit(ecpu.flags, VCPU_EFLAGS_AF)) {
		ecpu.al -= 0x06;
		ecpu.ah += 0x01;
		SetBit(ecpu.flags,VCPU_EFLAGS_AF);
		SetBit(ecpu.flags,VCPU_EFLAGS_CF);
	} else {
		ClrBit(ecpu.flags,VCPU_EFLAGS_CF);
		ClrBit(ecpu.flags,VCPU_EFLAGS_AF);
	}
	ecpu.al &= 0x0f;
#define op aas
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"AAS");
}
WRAPPER INC_AX()
{
	ecpu.ip++;
	INC((void *)&ecpu.ax,16);
}
WRAPPER INC_CX()
{
	ecpu.ip++;
	INC((void *)&ecpu.cx,16);
}
WRAPPER INC_DX()
{
	ecpu.ip++;
	INC((void *)&ecpu.dx,16);
}
WRAPPER INC_BX()
{
	ecpu.ip++;
	INC((void *)&ecpu.bx,16);
}
WRAPPER INC_SP()
{
	ecpu.ip++;
	INC((void *)&ecpu.sp,16);
}
WRAPPER INC_BP()
{
	ecpu.ip++;
	INC((void *)&ecpu.bp,16);
}
WRAPPER INC_SI()
{
	ecpu.ip++;
	INC((void *)&ecpu.si,16);
}
WRAPPER INC_DI()
{
	ecpu.ip++;
	INC((void *)&ecpu.di,16);
}
WRAPPER DEC_AX()
{
	ecpu.ip++;
	DEC((void *)&ecpu.ax,16);
}
WRAPPER DEC_CX()
{
	ecpu.ip++;
	DEC((void *)&ecpu.cx,16);
}
WRAPPER DEC_DX()
{
	ecpu.ip++;
	DEC((void *)&ecpu.dx,16);
}
WRAPPER DEC_BX()
{
	ecpu.ip++;
	DEC((void *)&ecpu.bx,16);
}
WRAPPER DEC_SP()
{
	ecpu.ip++;
	DEC((void *)&ecpu.sp,16);
}
WRAPPER DEC_BP()
{
	ecpu.ip++;
	DEC((void *)&ecpu.bp,16);
}
WRAPPER DEC_SI()
{
	ecpu.ip++;
	DEC((void *)&ecpu.si,16);
}
WRAPPER DEC_DI()
{
	ecpu.ip++;
	DEC((void *)&ecpu.di,16);
}
CHECKED PUSH_AX()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.ax,16);
}
CHECKED PUSH_CX()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.cx,16);
}
CHECKED PUSH_DX()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.dx,16);
}
CHECKED PUSH_BX()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.bx,16);
}
CHECKED PUSH_SP()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.sp,16);
}
CHECKED PUSH_BP()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.bp,16);
}
CHECKED PUSH_SI()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.si,16);
}
CHECKED PUSH_DI()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.di,16);
}
CHECKED POP_AX()
{
	ecpu.ip++;
	POP((void *)&ecpu.ax,16);
}
CHECKED POP_CX()
{
	ecpu.ip++;
	POP((void *)&ecpu.cx,16);
}
CHECKED POP_DX()
{
	ecpu.ip++;
	POP((void *)&ecpu.dx,16);
}
CHECKED POP_BX()
{
	ecpu.ip++;
	POP((void *)&ecpu.bx,16);
}
CHECKED POP_SP()
{
	ecpu.ip++;
	POP((void *)&ecpu.sp,16);
}
CHECKED POP_BP()
{
	ecpu.ip++;
	POP((void *)&ecpu.bp,16);
}
CHECKED POP_SI()
{
	ecpu.ip++;
	POP((void *)&ecpu.si,16);
}
CHECKED POP_DI()
{
	ecpu.ip++;
	POP((void *)&ecpu.di,16);
}
CHECKED JO()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_OF), 8);
}
CHECKED JNO()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_OF), 8);
}
CHECKED JC()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_CF), 8);
}
CHECKED JNC()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_CF), 8);
}
CHECKED JZ()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_ZF), 8);
}
CHECKED JNZ()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_ZF), 8);
}
CHECKED JBE()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (GetBit(ecpu.flags, VCPU_EFLAGS_CF) ||
		GetBit(ecpu.flags, VCPU_EFLAGS_ZF)), 8);
}
CHECKED JA()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (!GetBit(ecpu.flags, VCPU_EFLAGS_CF) &&
		!GetBit(ecpu.flags, VCPU_EFLAGS_ZF)), 8);
}
CHECKED JS()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_SF), 8);
}
CHECKED JNS()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_SF), 8);
}
CHECKED JP()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, GetBit(ecpu.flags, VCPU_EFLAGS_PF), 8);
}
CHECKED JNP()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, !GetBit(ecpu.flags, VCPU_EFLAGS_PF), 8);
}
CHECKED JL()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (GetBit(ecpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(ecpu.flags, VCPU_EFLAGS_OF)), 8);
}
CHECKED JNL()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (GetBit(ecpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(ecpu.flags, VCPU_EFLAGS_OF)), 8);
}
CHECKED JLE()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (GetBit(ecpu.flags, VCPU_EFLAGS_ZF) ||
		(GetBit(ecpu.flags, VCPU_EFLAGS_SF) !=
		GetBit(ecpu.flags, VCPU_EFLAGS_OF))), 8);
}
CHECKED JG()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void *)ecpuins.rimm, (!GetBit(ecpu.flags, VCPU_EFLAGS_ZF) &&
		(GetBit(ecpu.flags, VCPU_EFLAGS_SF) ==
		GetBit(ecpu.flags, VCPU_EFLAGS_OF))), 8);
}
void INS_80()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	default:CaseError("INS_80::ecpuins.rr");break;}
}
void INS_81()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	GetImm(16);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	default:CaseError("INS_81::ecpuins.rr");break;}
}
void INS_82()
{
	INS_80();
}
void INS_83()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ADD((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 1:	OR ((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 2:	ADC((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 3:	SBB((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 4:	AND((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 5:	SUB((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 6:	XOR((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	case 7:	CMP((void *)ecpuins.rrm,(void *)ecpuins.rimm,12);break;
	default:CaseError("INS_83::ecpuins.rr");break;}
}
WRAPPER TEST_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	TEST((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
WRAPPER TEST_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	TEST((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
WRAPPER XCHG_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	XCHG((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
WRAPPER XCHG_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	XCHG((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
WRAPPER MOV_RM8_R8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)ecpuins.rrm,(void *)ecpuins.rr,8);
}
WRAPPER MOV_RM16_R16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
WRAPPER MOV_R8_RM8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	MOV((void *)ecpuins.rr,(void *)ecpuins.rrm,8);
}
WRAPPER MOV_R16_RM16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
WRAPPER MOV_RM16_SEG()
{
	ecpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)ecpuins.rrm,(void *)ecpuins.rr,16);
}
void LEA_R16_M16()
{
	ecpu.ip++;
	GetModRegRMEA();
	d_nubit16(ecpuins.rr) = ecpuins.rrm & 0xffff;
}
WRAPPER MOV_SEG_RM16()
{
	ecpu.ip++;
	GetModRegRM(4,16);
	MOV((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
}
void POP_RM16()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:
		bugfix(14) POP((void *)ecpuins.rrm,16);
		else POP((void *)ecpuins.rrm,16);
		break;
	default:CaseError("POP_RM16::ecpuins.rr");break;}
}
CHECKED NOP()
{
	ecpu.ip++;
}
WRAPPER XCHG_CX_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.cx,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_DX_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.dx,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_BX_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.bx,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_SP_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.sp,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_BP_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.bp,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_SI_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.si,(void *)&ecpu.ax,16);
}
WRAPPER XCHG_DI_AX()
{
	ecpu.ip++;
	XCHG((void *)&ecpu.di,(void *)&ecpu.ax,16);
}
ASMCMP CBW()
{
	ecpu.ip++;
	async;
	ecpu.ax = (t_nsbit8)ecpu.al;
#define op cbw
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"CBW");
}
ASMCMP CWD()
{
	ecpu.ip++;
	async;
	if (ecpu.ax & 0x8000) ecpu.dx = 0xffff;
	else ecpu.dx = 0x0000;
#define op cwd
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"CWD");
}
void CALL_PTR16_16()
{
	t_nubit16 newcs,newip;
	async;
	ecpu.ip++;
	GetImm(16);
	newip = d_nubit16(ecpuins.rimm);
	GetImm(16);
	newcs = d_nubit16(ecpuins.rimm);
	aipcheck;
	PUSH((void *)&ecpu.cs.selector,16);
	PUSH((void *)&ecpu.ip,16);
	ecpu.ip = newip;
	ecpu.cs.selector = newcs;
}
NOTIMP WAIT()
{
	ecpu.ip++;
	/* not implemented */
}
CHECKED PUSHF()
{
	ecpu.ip++;
	PUSH((void *)&ecpu.flags,16);
}
CHECKED POPF()
{
	ecpu.ip++;
	POP((void *)&ecpu.flags,16);
}
void SAHF()
{
	ecpu.ip++;
	d_nubit8(&ecpu.flags) = ecpu.ah;
}
void LAHF()
{
	ecpu.ip++;
	ecpu.ah = d_nubit8(&ecpu.flags);
}
WRAPPER MOV_AL_M8()
{
	ecpu.ip++;
	GetMem();
	MOV((void *)&ecpu.al,(void *)ecpuins.rrm,8);
}
WRAPPER MOV_AX_M16()
{
	ecpu.ip++;
	GetMem();
	MOV((void *)&ecpu.ax,(void *)ecpuins.rrm,16);
}
WRAPPER MOV_M8_AL()
{
	ecpu.ip++;
	GetMem();
	MOV((void *)ecpuins.rrm,(void *)&ecpu.al,8);
}
WRAPPER MOV_M16_AX()
{
	ecpu.ip++;
	GetMem();
	MOV((void *)ecpuins.rrm,(void *)&ecpu.ax,16);
}
void MOVSB()
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
void MOVSW()
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
void CMPSB()
{
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
}
void CMPSW()
{
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
}
WRAPPER TEST_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	TEST((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
WRAPPER TEST_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	TEST((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
void STOSB()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			STOS(8);
			ecpu.cx--;
		}
	}
}
void STOSW()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) STOS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			STOS(16);
			ecpu.cx--;
		}
	}
}
void LODSB()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			LODS(8);
			ecpu.cx--;
		}
	}
}
void LODSW()
{
	ecpu.ip++;
	if(ecpuins.prefix_rep == PREFIX_REP_NONE) LODS(16);
	else {
		while(ecpu.cx) {
			//ecpuinsExecInt();
			LODS(16);
			ecpu.cx--;
		}
	}
}
void SCASB()
{
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
}
void SCASW()
{
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
}
WRAPPER MOV_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.al,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_CL_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.cl,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_DL_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.dl,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_BL_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.bl,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_AH_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.ah,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_CH_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.ch,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_DH_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.dh,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_BH_I8()
{
	ecpu.ip++;
	GetImm(8);
	MOV((void *)&ecpu.bh,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_AX_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.ax,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_CX_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.cx,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_DX_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.dx,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_BX_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.bx,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_SP_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.sp,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_BP_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.bp,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_SI_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.si,(void *)ecpuins.rimm,16);
}
WRAPPER MOV_DI_I16()
{
	ecpu.ip++;
	GetImm(16);
	MOV((void *)&ecpu.di,(void *)ecpuins.rimm,16);
}
void INS_C0()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 1:	ROR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 2:	RCL((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 3:	RCR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 4:	SHL((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 5:	SHR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 6:	SAL((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	case 7:	SAR((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);break;
	default:CaseError("INS_C0::ecpuins.rr");break;}
}
void INS_C1()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 1:	ROR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 2:	RCL((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 3:	RCR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 4:	SHL((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 5:	SHR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 6:	SAL((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	case 7:	SAR((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);break;
	default:CaseError("INS_C1::ecpuins.rr");break;}
}
void RET_I16()
{
	t_nubit16 addsp;
	ecpu.ip++;
	bugfix(15) {
		GetImm(16);
		addsp = d_nubit16(ecpuins.rimm);
	} else {
		GetImm(8);
		addsp = d_nubit8(ecpuins.rimm);
	}
	POP((void *)&ecpu.ip,16);
	ecpu.sp += addsp;
}
void RET()
{
	ecpu.ip++;
	POP((void *)&ecpu.ip,16);
}
void LES_R16_M16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	MOV((void *)&ecpu.es.selector,(void *)(ecpuins.rrm+2),16);
}
void LDS_R16_M16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	MOV((void *)ecpuins.rr,(void *)ecpuins.rrm,16);
	MOV((void *)&ecpu.ds.selector,(void *)(ecpuins.rrm+2),16);
}
WRAPPER MOV_M8_I8()
{
	ecpu.ip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);
}
WRAPPER MOV_M16_I16()
{
	ecpu.ip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);
}
void RETF_I16()
{
	t_nubit16 addsp;
	ecpu.ip++;
	GetImm(16);
	addsp = d_nubit16(ecpuins.rimm);
	POP((void *)&ecpu.ip,16);
	POP((void *)&ecpu.cs.selector,16);
	ecpu.sp += addsp;
}
void RETF()
{
	POP((void *)&ecpu.ip,16);
	POP((void *)&ecpu.cs.selector,16);
}
void INT3()
{
	async;
	ecpu.ip++;
	aipcheck;
	INT(0x03);
}
void INT_I8()
{
	async;
	ecpu.ip++;
	GetImm(8);
	aipcheck;
	INT(d_nubit8(ecpuins.rimm));
}
void INTO()
{
	async;
	ecpu.ip++;
	aipcheck;
	if(GetBit(ecpu.flags, VCPU_EFLAGS_OF)) INT(0x04);
}
void IRET()
{
	ecpu.ip++;
	POP((void *)&ecpu.ip,16);
	POP((void *)&ecpu.cs.selector,16);
	POP((void *)&ecpu.flags,16);
}
void INS_D0()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,NULL,8);break;
	case 1:	ROR((void *)ecpuins.rrm,NULL,8);break;
	case 2:	RCL((void *)ecpuins.rrm,NULL,8);break;
	case 3:	RCR((void *)ecpuins.rrm,NULL,8);break;
	case 4:	SHL((void *)ecpuins.rrm,NULL,8);break;
	case 5:	SHR((void *)ecpuins.rrm,NULL,8);break;
	case 6:	SAL((void *)ecpuins.rrm,NULL,8);break;
	case 7:	SAR((void *)ecpuins.rrm,NULL,8);break;
	default:CaseError("INS_D0::ecpuins.rr");break;}
}
void INS_D1()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,NULL,16);break;
	case 1:	ROR((void *)ecpuins.rrm,NULL,16);break;
	case 2:	RCL((void *)ecpuins.rrm,NULL,16);break;
	case 3:	RCR((void *)ecpuins.rrm,NULL,16);break;
	case 4:	SHL((void *)ecpuins.rrm,NULL,16);break;
	case 5:	SHR((void *)ecpuins.rrm,NULL,16);break;
	case 6:	SAL((void *)ecpuins.rrm,NULL,16);break;
	case 7:	SAR((void *)ecpuins.rrm,NULL,16);break;
	default:CaseError("INS_D1::ecpuins.rr");break;}
}
void INS_D2()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 1:	ROR((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 2:	RCL((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 3:	RCR((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 4:	SHL((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 5:	SHR((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 6:	SAL((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	case 7:	SAR((void *)ecpuins.rrm,(void *)&ecpu.cl,8);break;
	default:CaseError("INS_D2::ecpuins.rr");break;}
}
void INS_D3()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:	ROL((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 1:	ROR((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 2:	RCL((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 3:	RCR((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 4:	SHL((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 5:	SHR((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 6:	SAL((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	case 7:	SAR((void *)ecpuins.rrm,(void *)&ecpu.cl,16);break;
	default:CaseError("INS_D3::ecpuins.rr");break;}
}
ASMCMP AAM()
{
	t_nubit8 base,tempAL;
	ecpu.ip++;
	GetImm(8);
	async;
	base = d_nubit8(ecpuins.rimm);
	if(base == 0) INT(0x00);
	else {
		tempAL = ecpu.al;
		ecpu.ah = tempAL / base;
		ecpu.al = tempAL % base;
		ecpuins.bit = 0x08;
		ecpuins.result = ecpu.al & 0xff;
		SetFlags(AAM_FLAG);
	}
#define op aam
	aexec_cax16
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-2,"AAM");
}
ASMCMP AAD()
{
	t_nubit8 base,tempAL,tempAH;
	ecpu.ip++;
	GetImm(8);
	async;
	base = d_nubit8(ecpuins.rimm);
	if(base == 0) INT(0x00);
	else {
		tempAL = ecpu.al;
		tempAH = ecpu.ah;
		ecpu.al = (tempAL+(tempAH*base)) & 0xff;
		ecpu.ah = 0x00;
		ecpuins.bit = 0x08;
		ecpuins.result = ecpu.al & 0xff;
		SetFlags(AAD_FLAG);
	}
#define op aad
	aexec_cax16
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-2,"AAD");
}
void XLAT()
{
	ecpu.ip++;
	ecpu.al = vramRealByte(ecpuins.roverds->selector,ecpu.bx+ecpu.al);
}
/*
void INS_D9();
void INS_DB();
*/
void LOOPNZ()
{
	t_nsbit8 rel8;
	ecpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(ecpuins.rimm);
	else rel8 = d_nubit8(ecpuins.rimm);
	ecpu.cx--;
	if(ecpu.cx && !GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) ecpu.ip += rel8;
}
void LOOPZ()
{
	t_nsbit8 rel8;
	ecpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(ecpuins.rimm);
	else rel8 = d_nubit8(ecpuins.rimm);
	ecpu.cx--;
	if(ecpu.cx && GetBit(ecpu.flags, VCPU_EFLAGS_ZF)) ecpu.ip += rel8;
}
void LOOP()
{
	t_nsbit8 rel8;
	ecpu.ip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(ecpuins.rimm);
	else rel8 = d_nubit8(ecpuins.rimm);
	ecpu.cx--;
	if(ecpu.cx) ecpu.ip += rel8;
}
void JCXZ_REL8()
{
	ecpu.ip++;
	GetImm(8);
	JCC((void*)ecpuins.rimm,!ecpu.cx,8);
}
void IN_AL_I8()
{
	ecpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(ecpuins.rimm)]);
	ecpu.al = vport.iobyte;
}
void IN_AX_I8()
{
	ecpu.ip++;
	GetImm(8);
	ExecFun(vport.in[d_nubit8(ecpuins.rimm)]);
	ecpu.ax = vport.ioword;
}
void OUT_I8_AL()
{
	ecpu.ip++;
	GetImm(8);
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[d_nubit8(ecpuins.rimm)]);
}
void OUT_I8_AX()
{
	ecpu.ip++;
	GetImm(8);
	vport.ioword = ecpu.ax;
	ExecFun(vport.out[d_nubit8(ecpuins.rimm)]);
}
void CALL_REL16()
{
	t_nsbit16 rel16;
	async;
	ecpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(ecpuins.rimm);
	aipcheck;
	PUSH((void *)&ecpu.ip,16);
	bugfix(12) ecpu.ip += rel16;
	else ecpu.ip += d_nubit16(ecpuins.rimm);
}
void JMP_REL16()
{
	t_nsbit16 rel16;
	ecpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(ecpuins.rimm);
	bugfix(2) ecpu.ip += rel16;
	else ecpu.ip += d_nubit16(ecpuins.rimm);
}
void JMP_PTR16_16()
{
	t_nubit16 newip,newcs;
	ecpu.ip++;
	GetImm(16);
	newip = d_nubit16(ecpuins.rimm);
	GetImm(16);
	newcs = d_nubit16(ecpuins.rimm);
	ecpu.ip = newip;
	ecpu.cs.selector = newcs;
}
void JMP_REL8()
{
	t_nsbit8 rel8;
	ecpu.ip++;
	GetImm(8);
	rel8 = d_nsbit8(ecpuins.rimm);
	bugfix(9) ecpu.ip += rel8;
	else ecpu.ip += d_nubit8(ecpuins.rimm);
}
void IN_AL_DX()
{
	ecpu.ip++;
	ExecFun(vport.in[ecpu.dx]);
	ecpu.al = vport.iobyte;
}
void IN_AX_DX()
{
	ecpu.ip++;
	ExecFun(vport.in[ecpu.dx]);
	ecpu.ax = vport.ioword;
}
void OUT_DX_AL()
{
	ecpu.ip++;
	vport.iobyte = ecpu.al;
	ExecFun(vport.out[ecpu.dx]);
}
void OUT_DX_AX()
{
	ecpu.ip++;
	vport.ioword = ecpu.ax;
	ExecFun(vport.out[ecpu.dx]);
}
NOTIMP LOCK()
{
	ecpu.ip++;
	/* Not Implemented */
}
void REPNZ()
{
	// CMPS,SCAS
	ecpu.ip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZNZ;
}
void REP()
{	// MOVS,LODS,STOS,CMPS,SCAS
	ecpu.ip++;
	ecpuins.prefix_rep = PREFIX_REP_REPZ;
}
NOTIMP HLT()
{
	ecpu.ip++;
	/* Not Implemented */
}
ASMCMP CMC()
{
	ecpu.ip++;
	async;
	ecpu.flags ^= VCPU_EFLAGS_CF;
#define op cmc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"CMC");
}
void INS_F6()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	GetImm(8);
			TEST((void *)ecpuins.rrm,(void *)ecpuins.rimm,8);
			break;
	case 2:	NOT ((void *)ecpuins.rrm,8);	break;
	case 3:	NEG ((void *)ecpuins.rrm,8);	break;
	case 4:	MUL ((void *)ecpuins.rrm,8);	break;
	case 5:	IMUL((void *)ecpuins.rrm,8);	break;
	case 6:	DIV ((void *)ecpuins.rrm,8);	break;
	case 7:	IDIV((void *)ecpuins.rrm,8);	break;
	default:CaseError("INS_F6::ecpuins.rr");break;}
}
void INS_F7()
{
	ecpu.ip++;
	GetModRegRM(0,16);
	switch(ecpuins.rr) {
	case 0:	GetImm(16);
			TEST((void *)ecpuins.rrm,(void *)ecpuins.rimm,16);
			break;
	case 2:	NOT ((void *)ecpuins.rrm,16);	break;
	case 3:	NEG ((void *)ecpuins.rrm,16);	break;
	case 4:	MUL ((void *)ecpuins.rrm,16);	break;
	case 5:	IMUL((void *)ecpuins.rrm,16);	break;
	case 6:	DIV ((void *)ecpuins.rrm,16);	break;
	case 7:	IDIV((void *)ecpuins.rrm,16);	break;
	default:CaseError("INS_F7::ecpuins.rr");break;}
}
ASMCMP CLC()
{
	ecpu.ip++;
	async;
	ClrBit(ecpu.flags, VCPU_EFLAGS_CF);
#define op clc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"CLC");
}
ASMCMP STC()
{
	ecpu.ip++;
	async;
	SetBit(ecpu.flags, VCPU_EFLAGS_CF);
#define op stc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"STC");
}
void CLI()
{
	ecpu.ip++;
	ClrBit(ecpu.flags, VCPU_EFLAGS_IF);
}
void STI()
{
	ecpu.ip++;
	SetBit(ecpu.flags, VCPU_EFLAGS_IF);
}
ASMCMP CLD()
{
	ecpu.ip++;
	async;
	ClrBit(ecpu.flags, VCPU_EFLAGS_DF);
#define op cld
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"CLD");
}
ASMCMP STD()
{
	ecpu.ip++;
	async;
	SetBit(ecpu.flags,VCPU_EFLAGS_DF);
#define op STD
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(ecpu.cs.selector,ecpu.ip-1,"STD");
}
void INS_FE()
{
	ecpu.ip++;
	GetModRegRM(0,8);
	switch(ecpuins.rr) {
	case 0:	INC((void *)ecpuins.rrm,8);	break;
	case 1:	DEC((void *)ecpuins.rrm,8);	break;
	default:CaseError("INS_FE::ecpuins.rr");break;}
}
void INS_FF()
{
	async;
	ecpu.ip++;
	GetModRegRM(0,16);
	aipcheck;
	switch(ecpuins.rr) {
	case 0:	INC((void *)ecpuins.rrm,16);	break;
	case 1:	DEC((void *)ecpuins.rrm,16);	break;
	case 2:	/* CALL_RM16 */
		PUSH((void *)&ecpu.ip,16);
		ecpu.ip = d_nubit16(ecpuins.rrm);
		break;
	case 3:	/* CALL_M16_16 */
		PUSH((void *)&ecpu.cs.selector,16);
		PUSH((void *)&ecpu.ip,16);
		ecpu.ip = d_nubit16(ecpuins.rrm);
		bugfix(11) ecpu.cs.selector = d_nubit16(ecpuins.rrm+2);
		else ecpu.cs.selector = d_nubit16(ecpuins.rrm+1);
		break;
	case 4:	/* JMP_RM16 */
		ecpu.ip = d_nubit16(ecpuins.rrm);
		break;
	case 5:	/* JMP_M16_16 */
		ecpu.ip = d_nubit16(ecpuins.rrm);
		bugfix(11) ecpu.cs.selector = d_nubit16(ecpuins.rrm+2);
		else ecpu.cs.selector = d_nubit16(ecpuins.rrm+1);
		break;
	case 6:	/* PUSH_RM16 */
		PUSH((void *)ecpuins.rrm,16);
		break;
	default:CaseError("INS_FF::ecpuins.rr");break;}
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
static void ClrPrefix()
{
	ecpuins.roverds = &ecpu.ds;
	ecpuins.roverss = &ecpu.ss;
	ecpuins.prefix_rep = PREFIX_REP_NONE;
}
static void ExecIns()
{
	t_nubit8 opcode;

	bugfix(17) {
		/* Note: in this case, if we use two prefixes, the second prefix
		   will be discarded incorrectly */
		ClrPrefix();
		do {
			opcode = vramRealByte(ecpu.cs.selector, ecpu.ip);
			ExecFun(ecpuins.table[opcode]);
		} while (IsPrefix(opcode));
	} else {
		bugfix(16) {
			/* Note: in this bug, if an interrupt generated between
			   prefix and operation, the prefix may not be deployed */
			if (IsPrefix(opcode)) ExecFun(ecpuins.table[opcode]);
			opcode = vramRealByte(ecpu.cs.selector, ecpu.ip);
			ExecFun(ecpuins.table[opcode]);
			ClrPrefix();
		} else {
			ExecFun(ecpuins.table[opcode]);
			if (!IsPrefix(opcode)) ClrPrefix();
		}
	}
}
static void ExecInt()
{
	/* hardware interrupt handeler */
	if(ecpu.flagnmi) INT(0x02);
	ecpu.flagnmi = 0x00;

	if(GetBit(ecpu.flags, VCPU_EFLAGS_IF) && vpicScanINTR())
		INT(vpicGetINTR());

	if(GetBit(ecpu.flags, VCPU_EFLAGS_TF)) INT(0x01);
}

void QDX()
{
	ecpu.ip++;
	GetImm(8);
	switch (d_nubit8(ecpuins.rimm)) {
	case 0x00:
	case 0xff:
		vapiPrint("\nNXVM STOP at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			ecpu.cs.selector,ecpu.ip,d_nubit8(ecpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe:
		vapiPrint("\nNXVM RESET at CS:%04X IP:%04X INS:QDX IMM:%02X\n",
			ecpu.cs.selector,ecpu.ip,d_nubit8(ecpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	default:
		qdbiosExecInt(d_nubit8(ecpuins.rimm));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_ZF, GetBit(_flags, VCPU_EFLAGS_ZF));
		MakeBit(vramRealWord(_ss,_sp + 4), VCPU_EFLAGS_CF, GetBit(_flags, VCPU_EFLAGS_CF));
		break;
	}
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
	ecpuins.table[0x0f] = (t_faddrcc)POP_CS;
	//ecpuins.table[0x0f] = (t_faddrcc)INS_0F;
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
	ecpuins.table[0x66] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x67] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x68] = (t_faddrcc)UndefinedOpcode;
	//ecpuins.table[0x66] = (t_faddrcc)OpdSize;
	//ecpuins.table[0x67] = (t_faddrcc)AddrSize;
	//ecpuins.table[0x68] = (t_faddrcc)PUSH_I16;
	ecpuins.table[0x69] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6a] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6b] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6c] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6d] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6e] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x6f] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0x70] = (t_faddrcc)JO;
	ecpuins.table[0x71] = (t_faddrcc)JNO;
	ecpuins.table[0x72] = (t_faddrcc)JC;
	ecpuins.table[0x73] = (t_faddrcc)JNC;
	ecpuins.table[0x74] = (t_faddrcc)JZ;
	ecpuins.table[0x75] = (t_faddrcc)JNZ;
	ecpuins.table[0x76] = (t_faddrcc)JBE;
	ecpuins.table[0x77] = (t_faddrcc)JA;
	ecpuins.table[0x78] = (t_faddrcc)JS;
	ecpuins.table[0x79] = (t_faddrcc)JNS;
	ecpuins.table[0x7a] = (t_faddrcc)JP;
	ecpuins.table[0x7b] = (t_faddrcc)JNP;
	ecpuins.table[0x7c] = (t_faddrcc)JL;
	ecpuins.table[0x7d] = (t_faddrcc)JNL;
	ecpuins.table[0x7e] = (t_faddrcc)JLE;
	ecpuins.table[0x7f] = (t_faddrcc)JG;
	ecpuins.table[0x80] = (t_faddrcc)INS_80;
	ecpuins.table[0x81] = (t_faddrcc)INS_81;
	ecpuins.table[0x82] = (t_faddrcc)INS_82;
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
	ecpuins.table[0xa0] = (t_faddrcc)MOV_AL_M8;
	ecpuins.table[0xa1] = (t_faddrcc)MOV_AX_M16;
	ecpuins.table[0xa2] = (t_faddrcc)MOV_M8_AL;
	ecpuins.table[0xa3] = (t_faddrcc)MOV_M16_AX;
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
	ecpuins.table[0xc0] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xc1] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xc2] = (t_faddrcc)RET_I16;
	ecpuins.table[0xc3] = (t_faddrcc)RET;
	ecpuins.table[0xc4] = (t_faddrcc)LES_R16_M16;
	ecpuins.table[0xc5] = (t_faddrcc)LDS_R16_M16;
	ecpuins.table[0xc6] = (t_faddrcc)MOV_M8_I8;
	ecpuins.table[0xc7] = (t_faddrcc)MOV_M16_I16;
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
	ecpuins.table[0xd9] = (t_faddrcc)UndefinedOpcode;
	//ecpuins.table[0xd9] = (t_faddrcc)INS_D9;
	ecpuins.table[0xda] = (t_faddrcc)UndefinedOpcode;
	ecpuins.table[0xdb] = (t_faddrcc)UndefinedOpcode;
	//ecpuins.table[0xdb] = (t_faddrcc)INS_DB;
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
	ecpuins.table[0xf3] = (t_faddrcc)REP;
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
}
void ecpuinsReset()
{
	ecpuins.rrm = ecpuins.rr = ecpuins.rimm = (t_vaddrcc)NULL;
	ecpuins.opr1 = ecpuins.opr2 = ecpuins.result = ecpuins.bit = 0;
	ClrPrefix();
}
void ecpuinsRefresh()
{
	ExecIns();
	ExecInt();
}
void ecpuinsFinal() {}
