/* Copyright 2012-2014 Neko. */

/* PRINT prints device status. */

#include "../vapi.h"
#include "../vcpu.h"
#include "../vpic.h"
#include "../vpit.h"
#include "../vdma.h"
#include "../vfdc.h"
#include "../vfdd.h"

#include "print.h"

/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static void print_sreg_seg(t_cpu_sreg *rsreg, const t_strptr label) {
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
	if (rsreg->seg.executable) {
		vapiPrint("Code, %s, %s, %s\n",
			rsreg->seg.exec.conform ? "C" : "c",
			rsreg->seg.exec.readable ? "Rw" : "rw",
			rsreg->seg.exec.defsize ? "32" : "16");
	} else {
		vapiPrint("Data, %s, %s, %s\n",
			rsreg->seg.data.expdown ? "E" : "e",
			rsreg->seg.data.writable ? "RW" : "Rw",
			rsreg->seg.data.big ? "BIG" : "big");
	} 
}

/* Prints system segment registers (TR, LDTR) */
static void print_sreg_sys(t_cpu_sreg *rsreg, const t_strptr label) {
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}

/* Prints segment registers */
void printCpuSreg() {
	print_sreg_seg(&vcpu.es, "ES");
	print_sreg_seg(&vcpu.cs, "CS");
	print_sreg_seg(&vcpu.ss, "SS");
	print_sreg_seg(&vcpu.ds, "DS");
	print_sreg_seg(&vcpu.fs, "FS");
	print_sreg_seg(&vcpu.gs, "GS");
	print_sreg_sys(&vcpu.tr, "TR  ");
	print_sreg_sys(&vcpu.ldtr, "LDTR");
	vapiPrint("GDTR Base=%08X, Limit=%04X\n",
		_gdtr.base, _gdtr.limit);
	vapiPrint("IDTR Base=%08X, Limit=%04X\n",
		_idtr.base, _idtr.limit);
}

/* Prints control registers */
void printCpuCreg() {
	vapiPrint("CR0=%08X: %s %s %s %s %s %s\n", vcpu.cr0,
		_GetCR0_PG ? "PG" : "pg",
		_GetCR0_ET ? "ET" : "et",
		_GetCR0_TS ? "TS" : "ts",
		_GetCR0_EM ? "EM" : "em",
		_GetCR0_MP ? "MP" : "mp",
		_GetCR0_PE ? "PE" : "pe");
	vapiPrint("CR2=PFLR=%08X\n", vcpu.cr2);
	vapiPrint("CR3=PDBR=%08X\n", vcpu.cr3);
}

/* Prints regular registers */
void printCpuReg() {
	vapiPrint( "EAX=%08X", vcpu.eax);
	vapiPrint(" EBX=%08X", vcpu.ebx);
	vapiPrint(" ECX=%08X", vcpu.ecx);
	vapiPrint(" EDX=%08X", vcpu.edx);
	vapiPrint("\nESP=%08X",vcpu.esp);
	vapiPrint(" EBP=%08X", vcpu.ebp);
	vapiPrint(" ESI=%08X", vcpu.esi);
	vapiPrint(" EDI=%08X", vcpu.edi);
	vapiPrint("\nEIP=%08X",vcpu.eip);
	vapiPrint(" EFL=%08X", vcpu.eflags);
	vapiPrint(": ");
	vapiPrint("%s ", _GetEFLAGS_VM ? "VM" : "vm");
	vapiPrint("%s ", _GetEFLAGS_RF ? "RF" : "rf");
	vapiPrint("%s ", _GetEFLAGS_NT ? "NT" : "nt");
	vapiPrint("IOPL=%01X ", _GetEFLAGS_IOPL);
	vapiPrint("%s ", _GetEFLAGS_OF ? "OF" : "of");
	vapiPrint("%s ", _GetEFLAGS_DF ? "DF" : "df");
	vapiPrint("%s ", _GetEFLAGS_IF ? "IF" : "if");
	vapiPrint("%s ", _GetEFLAGS_TF ? "TF" : "tf");
	vapiPrint("%s ", _GetEFLAGS_SF ? "SF" : "sf");
	vapiPrint("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
	vapiPrint("%s ", _GetEFLAGS_AF ? "AF" : "af");
	vapiPrint("%s ", _GetEFLAGS_PF ? "PF" : "pf");
	vapiPrint("%s ", _GetEFLAGS_CF ? "CF" : "cf");
	vapiPrint("\n");
}

/* Prints active memory info */
void printCpuMem() {
	t_nsbitcc i;
	for (i = 0;i < vcpu.msize;++i) {
		vapiPrint("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
			vcpu.mem[i].flagwrite ? "Write" : "Read",
			vcpu.mem[i].linear, vcpu.mem[i].data, vcpu.mem[i].byte);
	}
}

/* Print PIC status */
void printPic() {
	vapiPrint("INFO PIC 1\n==========\n");
	vapiPrint("Init Status = %d, IRX = %x\n",
		vpic1.status, vpic1.irx);
	vapiPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic1.irr, vpic1.isr, vpic1.imr, vpic1.irr & (~vpic1.imr));
	vapiPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic1.icw1, VPIC_GetLTIM(&vpic1), VPIC_GetSNGL(&vpic1), VPIC_GetIC4(&vpic1));
	vapiPrint("ICW2 = %x\n",vpic1.icw2);
	vapiPrint("ICW3 = %x\n", vpic1.icw3);
	vapiPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic1.icw4, VPIC_GetSFNM(&vpic1), VPIC_GetBUF(&vpic1), VPIC_GetMS(&vpic1),
		VPIC_GetAEOI(&vpic1), VPIC_GetuPM(&vpic1));
	vapiPrint("OCW1 = %x\n", vpic1.ocw1);
	vapiPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic1.ocw2, VPIC_GetR(&vpic1), VPIC_GetSL(&vpic1), VPIC_GetEOI(&vpic1),
		vpic1.ocw2 & 0x07);
	vapiPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic1.ocw3, VPIC_GetESMM(&vpic1), VPIC_GetSMM(&vpic1),
		VPIC_GetP(&vpic1), VPIC_GetRR(&vpic1), VPIC_GetRIS(&vpic1));

	vapiPrint("INFO PIC 2\n==========\n");
	vapiPrint("Init Status = %d, IRX = %x\n",
		vpic2.status, vpic2.irx);
	vapiPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic2.irr, vpic2.isr, vpic2.imr, vpic2.irr & (~vpic2.imr));
	vapiPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic2.icw1, VPIC_GetLTIM(&vpic2), VPIC_GetSNGL(&vpic2), VPIC_GetIC4(&vpic2));
	vapiPrint("ICW2 = %x\n",vpic2.icw2);
	vapiPrint("ICW3 = %x\n", vpic2.icw3);
	vapiPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic2.icw4, VPIC_GetSFNM(&vpic2), VPIC_GetBUF(&vpic2), VPIC_GetMS(&vpic2),
		VPIC_GetAEOI(&vpic2), VPIC_GetuPM(&vpic2));
	vapiPrint("OCW1 = %x\n", vpic2.ocw1);
	vapiPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic2.ocw2, VPIC_GetR(&vpic2), VPIC_GetSL(&vpic2), VPIC_GetEOI(&vpic2),
		vpic2.ocw2 & 0x07);
	vapiPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic2.ocw3, VPIC_GetESMM(&vpic2), VPIC_GetSMM(&vpic2),
		VPIC_GetP(&vpic2), VPIC_GetRR(&vpic2), VPIC_GetRIS(&vpic2));
}

/* Print PIT status */
void printPit() {
	t_nubit8 id;
	for (id = 0;id < 3;++id) {
		vapiPrint("PIT INFO %d\n========\n",id);
		vapiPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
			vpit.cw[id], VPIT_GetSC(vpit.cw[id]), VPIT_GetRW(vpit.cw[id]),
			VPIT_GetM(vpit.cw[id]), VPIT_GetBCD(vpit.cw[id]));
		vapiPrint("Init = %x, Count = %x, Latch = %x\n",
			vpit.init[id], vpit.count[id], vpit.latch[id]);
		vapiPrint("Flags: ready = %d, latch = %d, read = %d, write = %d, gate = %d, out = %x\n",
			vpit.flagready[id], vpit.flaglatch[id], vpit.flagread[id],
			vpit.flagwrite[id], vpit.flaggate[id], vpit.out[id]);
	}
	id = 3;
	vapiPrint("PIT INFO %d (read-back)\n========\n",id);
	vapiPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
	vpit.cw[id], VPIT_GetSC(vpit.cw[id]), VPIT_GetRW(vpit.cw[id]),
		VPIT_GetM(vpit.cw[id]), VPIT_GetBCD(vpit.cw[id]));
}

/* Print DMA status */
void printDma() {
	t_nubit8 i;
	vapiPrint("DMA 1 Info\n==========\n");
	vapiPrint("Command = %x, status = %x, mask = %x\n",
	          vdma1.command, vdma1.status, vdma1.mask);
	vapiPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma1.request, vdma1.temp, vdma1.flagmsb);
	vapiPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma1.drx, vdma1.flageop, vdma1.isr);
	for (i = 0;i < 4;++i) {
		vapiPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma1.channel[i].baseaddr, vdma1.channel[i].basewc,
		          vdma1.channel[i].curraddr, vdma1.channel[i].currwc);
		vapiPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma1.channel[i].mode, vdma1.channel[i].page,
		          vdma1.channel[i].devread, vdma1.channel[i].devwrite);
	}
	vapiPrint("\nDMA 2 Info\n==========\n");
	vapiPrint("Command = %x, status = %x, mask = %x\n",
	          vdma2.command, vdma2.status, vdma2.mask);
	vapiPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma2.request, vdma2.temp, vdma2.flagmsb);
	vapiPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma2.drx, vdma2.flageop, vdma2.isr);
	for (i = 0;i < 4;++i) {
		vapiPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma2.channel[i].baseaddr, vdma2.channel[i].basewc,
		          vdma2.channel[i].curraddr, vdma2.channel[i].currwc);
		vapiPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma2.channel[i].mode, vdma2.channel[i].page,
		          vdma2.channel[i].devread, vdma2.channel[i].devwrite);
	}
	vapiPrint("\nLatch: byte = %x, word = %x\n", vlatch.byte, vlatch.word);
}

void printFdc() {
	t_nubit8 i;
	vapiPrint("FDC INFO\n========\n");
	vapiPrint("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
		vfdc.msr,vfdc.dir,vfdc.dor,vfdc.ccr,vfdc.dr);
	vapiPrint("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
		vfdc.hut,vfdc.hlt,vfdc.srt,vfdc.flagndma,vfdc.flagintr);
	vapiPrint("rwid = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
		vfdc.rwid,vfdc.st0,vfdc.st1,vfdc.st2,vfdc.st3);
	for (i = 0;i < 9;++i) {
		vapiPrint("cmd[%d] = %x, ", i,vfdc.cmd[i]);
	}
	vapiPrint("\n");
	for (i = 0;i < 7;++i) {
		vapiPrint("ret[%d] = %x, ", i,vfdc.ret[i]);
	}
	vapiPrint("\n");
	vapiPrint("FDD INFO\n========\n");
	vapiPrint("cyl = %x, head = %x, sector = %x\n",
		vfdd.cyl,vfdd.head,vfdd.sector);
	vapiPrint("nsector = %x, nbyte = %x, gaplen = %x\n",
		vfdd.nsector,vfdd.nbyte,vfdd.gaplen);
	vapiPrint("ReadOnly = %x, Exist = %x\n",
		vfdd.flagro,vfdd.flagexist);
	vapiPrint("base = %x, curr = %x, count = %x\n",
		vfdd.base,vfdd.curr,vfdd.count);
}
