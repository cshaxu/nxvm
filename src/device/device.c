/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "../utils.h"

#include "vfdd.h"
#include "vhdd.h"
/* Disk Drive Operations */
t_bool deviceConnectFloppyInsert(const t_strptr fname) {
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image && vfdd.base) {
		count = fread((void *) vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
		vfdd.flagexist = 1;
		fclose(image);
		return 0;
	} else {
		return 1;
	}
}
t_bool deviceConnectFloppyRemove(const t_strptr fname) {
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vfdd.flagro)
				count = fwrite((void *) vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
			vfdd.flagexist = 0;
			fclose(image);
		} else {
			return 1;
		}
	}
	vfdd.flagexist = 0;
	memset((void *) vfdd.base, 0x00, vfddGetImageSize);
	return 0;
}
t_bool deviceConnectHardDiskInsert(const t_strptr fname) {
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image) {
		fseek(image, 0, SEEK_END);
		count = ftell(image);
		vhdd.ncyl = (t_nubit16)(count / vhdd.nhead / vhdd.nsector / vhdd.nbyte);
		fseek(image, 0, SEEK_SET);
		vhddAlloc();
		count = fread((void *) vhdd.base, sizeof(t_nubit8), vhddGetImageSize, image);
		vhdd.flagexist = 1;
		fclose(image);
		return 0;
	} else {
		return 1;
	}
}
t_bool deviceConnectHardDiskRemove(const t_strptr fname) {
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vhdd.flagro)
				count = fwrite((void *) vhdd.base, sizeof(t_nubit8), vhddGetImageSize, image);
			vhdd.flagexist = 0;
			fclose(image);
		} else {
			return 1;
		}
	}
	vhdd.flagexist = 0;
	memset((void *) vhdd.base, 0x00, vhddGetImageSize);
	return 0;
}

#include "vcpu.h"
#include "vpic.h"
#include "vpit.h"
#include "vdma.h"
#include "vfdc.h"
/* Prints device status. */
/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static void print_sreg_seg(t_cpu_sreg *rsreg, const t_strptr label) {
	utilsPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
	if (rsreg->seg.executable) {
		utilsPrint("Code, %s, %s, %s\n",
			rsreg->seg.exec.conform ? "C" : "c",
			rsreg->seg.exec.readable ? "Rw" : "rw",
			rsreg->seg.exec.defsize ? "32" : "16");
	} else {
		utilsPrint("Data, %s, %s, %s\n",
			rsreg->seg.data.expdown ? "E" : "e",
			rsreg->seg.data.writable ? "RW" : "Rw",
			rsreg->seg.data.big ? "BIG" : "big");
	} 
}

/* Prints system segment registers (TR, LDTR) */
static void print_sreg_sys(t_cpu_sreg *rsreg, const t_strptr label) {
	utilsPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}

/* Prints segment registers */
void devicePrintCpuSreg() {
	print_sreg_seg(&vcpu.es, "ES");
	print_sreg_seg(&vcpu.cs, "CS");
	print_sreg_seg(&vcpu.ss, "SS");
	print_sreg_seg(&vcpu.ds, "DS");
	print_sreg_seg(&vcpu.fs, "FS");
	print_sreg_seg(&vcpu.gs, "GS");
	print_sreg_sys(&vcpu.tr, "TR  ");
	print_sreg_sys(&vcpu.ldtr, "LDTR");
	utilsPrint("GDTR Base=%08X, Limit=%04X\n",
		_gdtr.base, _gdtr.limit);
	utilsPrint("IDTR Base=%08X, Limit=%04X\n",
		_idtr.base, _idtr.limit);
}

/* Prints control registers */
void devicePrintCpuCreg() {
	utilsPrint("CR0=%08X: %s %s %s %s %s %s\n", vcpu.cr0,
		_GetCR0_PG ? "PG" : "pg",
		_GetCR0_ET ? "ET" : "et",
		_GetCR0_TS ? "TS" : "ts",
		_GetCR0_EM ? "EM" : "em",
		_GetCR0_MP ? "MP" : "mp",
		_GetCR0_PE ? "PE" : "pe");
	utilsPrint("CR2=PFLR=%08X\n", vcpu.cr2);
	utilsPrint("CR3=PDBR=%08X\n", vcpu.cr3);
}

/* Prints regular registers */
void devicePrintCpuReg() {
	utilsPrint( "EAX=%08X", vcpu.eax);
	utilsPrint(" EBX=%08X", vcpu.ebx);
	utilsPrint(" ECX=%08X", vcpu.ecx);
	utilsPrint(" EDX=%08X", vcpu.edx);
	utilsPrint("\nESP=%08X",vcpu.esp);
	utilsPrint(" EBP=%08X", vcpu.ebp);
	utilsPrint(" ESI=%08X", vcpu.esi);
	utilsPrint(" EDI=%08X", vcpu.edi);
	utilsPrint("\nEIP=%08X",vcpu.eip);
	utilsPrint(" EFL=%08X", vcpu.eflags);
	utilsPrint(": ");
	utilsPrint("%s ", _GetEFLAGS_VM ? "VM" : "vm");
	utilsPrint("%s ", _GetEFLAGS_RF ? "RF" : "rf");
	utilsPrint("%s ", _GetEFLAGS_NT ? "NT" : "nt");
	utilsPrint("IOPL=%01X ", _GetEFLAGS_IOPL);
	utilsPrint("%s ", _GetEFLAGS_OF ? "OF" : "of");
	utilsPrint("%s ", _GetEFLAGS_DF ? "DF" : "df");
	utilsPrint("%s ", _GetEFLAGS_IF ? "IF" : "if");
	utilsPrint("%s ", _GetEFLAGS_TF ? "TF" : "tf");
	utilsPrint("%s ", _GetEFLAGS_SF ? "SF" : "sf");
	utilsPrint("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
	utilsPrint("%s ", _GetEFLAGS_AF ? "AF" : "af");
	utilsPrint("%s ", _GetEFLAGS_PF ? "PF" : "pf");
	utilsPrint("%s ", _GetEFLAGS_CF ? "CF" : "cf");
	utilsPrint("\n");
}

/* Prints active memory info */
void devicePrintCpuMem() {
	t_nsbitcc i;
	for (i = 0;i < vcpu.msize;++i) {
		utilsPrint("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
			vcpu.mem[i].flagwrite ? "Write" : "Read",
			vcpu.mem[i].linear, vcpu.mem[i].data, vcpu.mem[i].byte);
	}
}

/* Print PIC status */
void devicePrintPic() {
	utilsPrint("INFO PIC 1\n==========\n");
	utilsPrint("Init Status = %d, IRX = %x\n",
		vpic1.status, vpic1.irx);
	utilsPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic1.irr, vpic1.isr, vpic1.imr, vpic1.irr & (~vpic1.imr));
	utilsPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic1.icw1, VPIC_GetLTIM(&vpic1), VPIC_GetSNGL(&vpic1), VPIC_GetIC4(&vpic1));
	utilsPrint("ICW2 = %x\n",vpic1.icw2);
	utilsPrint("ICW3 = %x\n", vpic1.icw3);
	utilsPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic1.icw4, VPIC_GetSFNM(&vpic1), VPIC_GetBUF(&vpic1), VPIC_GetMS(&vpic1),
		VPIC_GetAEOI(&vpic1), VPIC_GetuPM(&vpic1));
	utilsPrint("OCW1 = %x\n", vpic1.ocw1);
	utilsPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic1.ocw2, VPIC_GetR(&vpic1), VPIC_GetSL(&vpic1), VPIC_GetEOI(&vpic1),
		vpic1.ocw2 & 0x07);
	utilsPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic1.ocw3, VPIC_GetESMM(&vpic1), VPIC_GetSMM(&vpic1),
		VPIC_GetP(&vpic1), VPIC_GetRR(&vpic1), VPIC_GetRIS(&vpic1));

	utilsPrint("INFO PIC 2\n==========\n");
	utilsPrint("Init Status = %d, IRX = %x\n",
		vpic2.status, vpic2.irx);
	utilsPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic2.irr, vpic2.isr, vpic2.imr, vpic2.irr & (~vpic2.imr));
	utilsPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic2.icw1, VPIC_GetLTIM(&vpic2), VPIC_GetSNGL(&vpic2), VPIC_GetIC4(&vpic2));
	utilsPrint("ICW2 = %x\n",vpic2.icw2);
	utilsPrint("ICW3 = %x\n", vpic2.icw3);
	utilsPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic2.icw4, VPIC_GetSFNM(&vpic2), VPIC_GetBUF(&vpic2), VPIC_GetMS(&vpic2),
		VPIC_GetAEOI(&vpic2), VPIC_GetuPM(&vpic2));
	utilsPrint("OCW1 = %x\n", vpic2.ocw1);
	utilsPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic2.ocw2, VPIC_GetR(&vpic2), VPIC_GetSL(&vpic2), VPIC_GetEOI(&vpic2),
		vpic2.ocw2 & 0x07);
	utilsPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic2.ocw3, VPIC_GetESMM(&vpic2), VPIC_GetSMM(&vpic2),
		VPIC_GetP(&vpic2), VPIC_GetRR(&vpic2), VPIC_GetRIS(&vpic2));
}

/* Print PIT status */
void devicePrintPit() {
	t_nubit8 id;
	for (id = 0;id < 3;++id) {
		utilsPrint("PIT INFO %d\n========\n",id);
		utilsPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
			vpit.cw[id], VPIT_GetSC(vpit.cw[id]), VPIT_GetRW(vpit.cw[id]),
			VPIT_GetM(vpit.cw[id]), VPIT_GetBCD(vpit.cw[id]));
		utilsPrint("Init = %x, Count = %x, Latch = %x\n",
			vpit.init[id], vpit.count[id], vpit.latch[id]);
		utilsPrint("Flags: ready = %d, latch = %d, read = %d, write = %d, gate = %d, out = %x\n",
			vpit.flagready[id], vpit.flaglatch[id], vpit.flagread[id],
			vpit.flagwrite[id], vpit.flaggate[id], vpit.out[id]);
	}
	id = 3;
	utilsPrint("PIT INFO %d (read-back)\n========\n",id);
	utilsPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
	vpit.cw[id], VPIT_GetSC(vpit.cw[id]), VPIT_GetRW(vpit.cw[id]),
		VPIT_GetM(vpit.cw[id]), VPIT_GetBCD(vpit.cw[id]));
}

/* Print DMA status */
void devicePrintDma() {
	t_nubit8 i;
	utilsPrint("DMA 1 Info\n==========\n");
	utilsPrint("Command = %x, status = %x, mask = %x\n",
	          vdma1.command, vdma1.status, vdma1.mask);
	utilsPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma1.request, vdma1.temp, vdma1.flagmsb);
	utilsPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma1.drx, vdma1.flageop, vdma1.isr);
	for (i = 0;i < 4;++i) {
		utilsPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma1.channel[i].baseaddr, vdma1.channel[i].basewc,
		          vdma1.channel[i].curraddr, vdma1.channel[i].currwc);
		utilsPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma1.channel[i].mode, vdma1.channel[i].page,
		          vdma1.channel[i].devread, vdma1.channel[i].devwrite);
	}
	utilsPrint("\nDMA 2 Info\n==========\n");
	utilsPrint("Command = %x, status = %x, mask = %x\n",
	          vdma2.command, vdma2.status, vdma2.mask);
	utilsPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma2.request, vdma2.temp, vdma2.flagmsb);
	utilsPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma2.drx, vdma2.flageop, vdma2.isr);
	for (i = 0;i < 4;++i) {
		utilsPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma2.channel[i].baseaddr, vdma2.channel[i].basewc,
		          vdma2.channel[i].curraddr, vdma2.channel[i].currwc);
		utilsPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma2.channel[i].mode, vdma2.channel[i].page,
		          vdma2.channel[i].devread, vdma2.channel[i].devwrite);
	}
	utilsPrint("\nLatch: byte = %x, word = %x\n", vlatch.byte, vlatch.word);
}

/* Prints FDC status */
void devicePrintFdc() {
	t_nubit8 i;
	utilsPrint("FDC INFO\n========\n");
	utilsPrint("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
		vfdc.msr,vfdc.dir,vfdc.dor,vfdc.ccr,vfdc.dr);
	utilsPrint("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
		vfdc.hut,vfdc.hlt,vfdc.srt,vfdc.flagndma,vfdc.flagintr);
	utilsPrint("rwid = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
		vfdc.rwid,vfdc.st0,vfdc.st1,vfdc.st2,vfdc.st3);
	for (i = 0;i < 9;++i) {
		utilsPrint("cmd[%d] = %x, ", i,vfdc.cmd[i]);
	}
	utilsPrint("\n");
	for (i = 0;i < 7;++i) {
		utilsPrint("ret[%d] = %x, ", i,vfdc.ret[i]);
	}
	utilsPrint("\n");
	utilsPrint("FDD INFO\n========\n");
	utilsPrint("cyl = %x, head = %x, sector = %x\n",
		vfdd.cyl,vfdd.head,vfdd.sector);
	utilsPrint("nsector = %x, nbyte = %x, gaplen = %x\n",
		vfdd.nsector,vfdd.nbyte,vfdd.gaplen);
	utilsPrint("ReadOnly = %x, Exist = %x\n",
		vfdd.flagro,vfdd.flagexist);
	utilsPrint("base = %x, curr = %x, count = %x\n",
		vfdd.base,vfdd.curr,vfdd.count);
}

#include "vmachine.h"
#include "device.h"

t_device device;

/* Starts device thread */
void deviceStart() {
	device.flagRun = 1;
	device.flagFlip = 1 - device.flagFlip;
	while (device.flagRun) {
		if (device.flagReset) {
			vmachineReset();
			device.flagReset = 0;
		}
		vmachineRefresh();
	}
}

/* Issues resetting signal to device thread */
void deviceReset() {
	if (device.flagRun) {
		device.flagReset = 1;
	} else {
		vmachineReset();
		device.flagReset = 0;
	}
}

/* Issues stopping signal to device thread */
void deviceStop()  {
	device.flagRun = 0;
}

/* Initializes devices */
void deviceInit() {
	memset(&device, 0x00, sizeof(t_device));
	vmachineInit();
	device.flagBoot = 0;
}

/* Finalizes devices */
void deviceFinal() {
	vmachineFinal();
}
