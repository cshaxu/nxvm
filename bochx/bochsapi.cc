#define PRODUCT "BOCHX: BOCHs - nXvm Connector\n\
Copyright (c) 2012-2014 Neko.\n"

/* BOCHSAPI connects bochs cpu and ram with nxvm cpu and ram. */

#include "../src/utils.h"
#include "../src/device/device.h"
#include "../src/device/vcpuins.h"

#include "bochsapi.h"

/* DEBUG SETTINGS ******************************************************* */
#define BOCHSAPI_BOCHS        False      /* tells if connected with bochs */
#define BOCHSAPI_LINEAR_START 0x00007c00 /* where bochx comparison starts */
#define BOCHSAPI_LINEAR_STOP  0x00002eab /* where bochx comparison stops  */
/* ********************************************************************** */

#if BOCHSAPI_BOCHS == True
#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs-2.6/bochs.h"
#include "bochs-2.6/cpu/cpu.h"
#endif

static void PrintReg(t_cpu *rcpu) {
    t_cpu cpuCopy = vcpu;
    vcpu = *rcpu;
    devicePrintCpuReg();
    vcpu = cpuCopy;
}

static void PrintSreg(t_cpu *rcpu) {
    t_cpu cpuCopy = vcpu;
    vcpu = *rcpu;
    devicePrintCpuSreg();
    vcpu = cpuCopy;
}

static void PrintCreg(t_cpu *rcpu) {
    t_cpu cpuCopy = vcpu;
    vcpu = *rcpu;
    devicePrintCpuCreg();
    vcpu = cpuCopy;
}

#if BOCHSAPI_BOCHS == True
static void LoadSreg(t_cpu_data_sreg *rsreg, bx_dbg_sreg_t *rbsreg) {
    t_nubit64 cdesc;
    rsreg->selector = rbsreg->sel;
    rsreg->flagValid = rbsreg->valid;
    cdesc = ((t_nubit64)rbsreg->des_h << 32) | rbsreg->des_l;
    rsreg->dpl = (t_nubit4)_GetDesc_DPL(cdesc);
    if (_IsDescUser(cdesc)) {
        rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
        rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
                                    ((_GetDescSeg_Limit(cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
        rsreg->seg.accessed = _IsDescUserAccessed(cdesc);
        rsreg->seg.executable = _IsDescCode(cdesc);
        if (rsreg->seg.executable) {
            rsreg->seg.exec.conform = _IsDescCodeConform(cdesc);
            rsreg->seg.exec.defsize = _IsDescCode32(cdesc);
            rsreg->seg.exec.readable = _IsDescCodeReadable(cdesc);
        } else {
            rsreg->seg.data.big = _IsDescDataBig(cdesc);
            rsreg->seg.data.expdown = _IsDescDataExpDown(cdesc);
            rsreg->seg.data.writable = _IsDescDataWritable(cdesc);
        }
    } else {
        rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
        rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
                                    (_GetDescSeg_Limit(cdesc) << 12 | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
        rsreg->sys.type = (t_nubit4)_GetDesc_Type(cdesc);
    }
}

static void CopyBochsCpu(t_cpu *rcpu) {
    bx_dbg_sreg_t bsreg;
    bx_dbg_global_sreg_t bgsreg;

    rcpu->data.eax = EAX;
    rcpu->data.ecx = ECX;
    rcpu->data.edx = EDX;
    rcpu->data.ebx = EBX;
    rcpu->data.esp = ESP;
    rcpu->data.ebp = EBP;
    rcpu->data.esi = ESI;
    rcpu->data.edi = EDI;
    rcpu->data.eip = EIP;
    rcpu->data.eflags = BX_CPU_THIS_PTR read_eflags();

    rcpu->data.es.sregtype = SREG_DATA;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 0);
    LoadSreg(&rcpu->data.es, &bsreg);

    rcpu->data.cs.sregtype = SREG_CODE;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 1);
    LoadSreg(&rcpu->data.cs, &bsreg);

    rcpu->data.ss.sregtype = SREG_STACK;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 2);
    LoadSreg(&rcpu->data.ss, &bsreg);

    rcpu->data.ds.sregtype = SREG_DATA;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 3);
    LoadSreg(&rcpu->data.ds, &bsreg);

    rcpu->data.fs.sregtype = SREG_DATA;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 4);
    LoadSreg(&rcpu->data.fs, &bsreg);

    rcpu->data.gs.sregtype = SREG_DATA;
    BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 5);
    LoadSreg(&rcpu->data.gs, &bsreg);

    rcpu->data.ldtr.sregtype = SREG_LDTR;
    BX_CPU_THIS_PTR dbg_get_ldtr(&bsreg);
    LoadSreg(&rcpu->data.ldtr, &bsreg);

    rcpu->data.tr.sregtype = SREG_TR;
    BX_CPU_THIS_PTR dbg_get_tr(&bsreg);
    LoadSreg(&rcpu->data.tr, &bsreg);

    rcpu->data.gdtr.sregtype = SREG_GDTR;
    BX_CPU_THIS_PTR dbg_get_gdtr(&bgsreg);
    rcpu->data.gdtr.base = GetMax32(bgsreg.base);
    rcpu->data.gdtr.limit = bgsreg.limit;

    rcpu->data.idtr.sregtype = SREG_IDTR;
    BX_CPU_THIS_PTR dbg_get_idtr(&bgsreg);
    rcpu->data.idtr.base = GetMax32(bgsreg.base);
    rcpu->data.idtr.limit = bgsreg.limit;

    rcpu->data.cr0 = BX_CPU_THIS_PTR cr0.get32();
    rcpu->data.cr2 = GetMax32(BX_CPU_THIS_PTR cr2);
    rcpu->data.cr3 = GetMax32(BX_CPU_THIS_PTR cr3);
}
#endif

static t_cpu scpu, bcpu;
static t_cpuins bcpuins;

static t_bool CheckDiff() {
    t_nubitcc i;
    t_bool flagDiff = False;
    t_nubit32 mask = VCPU_EFLAGS_RESERVED | vcpuins.data.udf;

    if (!vcpuins.data.flagIgnore) {
        if (vcpu.data.cr0 != bcpu.data.cr0) {
            PRINTF("diff cr0\n");
            flagDiff = True;
        }
        if (vcpu.data.cr2 != bcpu.data.cr2) {
            PRINTF("diff cr2\n");
            flagDiff = True;
        }
        if (vcpu.data.cr3 != bcpu.data.cr3) {
            PRINTF("diff cr3\n");
            flagDiff = True;
        }
        if (vcpu.data.eax != bcpu.data.eax) {
            PRINTF("diff eax\n");
            flagDiff = True;
        }
        if (vcpu.data.ebx != bcpu.data.ebx) {
            PRINTF("diff ebx\n");
            flagDiff = True;
        }
        if (vcpu.data.ecx != bcpu.data.ecx) {
            PRINTF("diff ecx\n");
            flagDiff = True;
        }
        if (vcpu.data.edx != bcpu.data.edx) {
            PRINTF("diff edx\n");
            flagDiff = True;
        }
        if (vcpu.data.esp != bcpu.data.esp) {
            PRINTF("diff esp\n");
            flagDiff = True;
        }
        if (vcpu.data.ebp != bcpu.data.ebp) {
            PRINTF("diff ebp\n");
            flagDiff = True;
        }
        if (vcpu.data.esi != bcpu.data.esi) {
            PRINTF("diff esi\n");
            flagDiff = True;
        }
        if (vcpu.data.edi != bcpu.data.edi) {
            PRINTF("diff edi\n");
            flagDiff = True;
        }
        if (vcpu.data.eip != bcpu.data.eip) {
            PRINTF("diff eip\n");
            flagDiff = True;
        }
        if (vcpu.data.es.selector != bcpu.data.es.selector ||
                (vcpu.data.es.flagValid && (
                     vcpu.data.es.base != bcpu.data.es.base ||
                     vcpu.data.es.limit != bcpu.data.es.limit ||
                     vcpu.data.es.dpl != bcpu.data.es.dpl))) {
            PRINTF("diff es (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.es.selector, vcpu.data.es.base, vcpu.data.es.limit, vcpu.data.es.dpl,
                   bcpu.data.es.selector, bcpu.data.es.base, bcpu.data.es.limit, bcpu.data.es.dpl);
            flagDiff = True;
        }
        if (vcpu.data.cs.selector != bcpu.data.cs.selector ||
                vcpu.data.cs.base != bcpu.data.cs.base ||
                vcpu.data.cs.limit != bcpu.data.cs.limit ||
                vcpu.data.cs.dpl != bcpu.data.cs.dpl) {
            PRINTF("diff cs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.cs.selector, vcpu.data.cs.base, vcpu.data.cs.limit, vcpu.data.cs.dpl,
                   bcpu.data.cs.selector, bcpu.data.cs.base, bcpu.data.cs.limit, bcpu.data.cs.dpl);
            flagDiff = True;
        }
        if (vcpu.data.ss.selector != bcpu.data.ss.selector ||
                vcpu.data.ss.base != bcpu.data.ss.base ||
                vcpu.data.ss.limit != bcpu.data.ss.limit ||
                vcpu.data.ss.dpl != bcpu.data.ss.dpl) {
            PRINTF("diff ss (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.ss.selector, vcpu.data.ss.base, vcpu.data.ss.limit, vcpu.data.ss.dpl,
                   bcpu.data.ss.selector, bcpu.data.ss.base, bcpu.data.ss.limit, bcpu.data.ss.dpl);
            flagDiff = True;
        }
        if (vcpu.data.ds.selector != bcpu.data.ds.selector ||
                (vcpu.data.ds.flagValid && (
                     vcpu.data.ds.base != bcpu.data.ds.base ||
                     vcpu.data.ds.limit != bcpu.data.ds.limit ||
                     vcpu.data.ds.dpl != bcpu.data.ds.dpl))) {
            PRINTF("diff ds (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.ds.selector, vcpu.data.ds.base, vcpu.data.ds.limit, vcpu.data.ds.dpl,
                   bcpu.data.ds.selector, bcpu.data.ds.base, bcpu.data.ds.limit, bcpu.data.ds.dpl);
            flagDiff = True;
        }
        if (vcpu.data.fs.selector != bcpu.data.fs.selector ||
                (vcpu.data.fs.flagValid && (
                     vcpu.data.fs.base != bcpu.data.fs.base ||
                     vcpu.data.fs.limit != bcpu.data.fs.limit ||
                     vcpu.data.fs.dpl != bcpu.data.fs.dpl))) {
            PRINTF("diff fs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.fs.selector, vcpu.data.fs.base, vcpu.data.fs.limit, vcpu.data.fs.dpl,
                   bcpu.data.fs.selector, bcpu.data.fs.base, bcpu.data.fs.limit, bcpu.data.fs.dpl);
            flagDiff = True;
        }
        if (vcpu.data.gs.selector != bcpu.data.gs.selector ||
                (vcpu.data.gs.flagValid && (
                     vcpu.data.gs.base != bcpu.data.gs.base ||
                     vcpu.data.gs.limit != bcpu.data.gs.limit ||
                     vcpu.data.gs.dpl != bcpu.data.gs.dpl))) {
            PRINTF("diff gs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.gs.selector, vcpu.data.gs.base, vcpu.data.gs.limit, vcpu.data.gs.dpl,
                   bcpu.data.gs.selector, bcpu.data.gs.base, bcpu.data.gs.limit, bcpu.data.gs.dpl);
            flagDiff = True;
        }
        if (vcpu.data.tr.selector != bcpu.data.tr.selector ||
                vcpu.data.tr.base != bcpu.data.tr.base ||
                vcpu.data.tr.limit != bcpu.data.tr.limit ||
                vcpu.data.tr.dpl != bcpu.data.tr.dpl) {
            PRINTF("diff tr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.tr.selector, vcpu.data.tr.base, vcpu.data.tr.limit, vcpu.data.tr.dpl,
                   bcpu.data.tr.selector, bcpu.data.tr.base, bcpu.data.tr.limit, bcpu.data.tr.dpl);
            flagDiff = True;
        }
        if (vcpu.data.ldtr.selector != bcpu.data.ldtr.selector ||
                vcpu.data.ldtr.base != bcpu.data.ldtr.base ||
                vcpu.data.ldtr.limit != bcpu.data.ldtr.limit ||
                vcpu.data.ldtr.dpl != bcpu.data.ldtr.dpl) {
            PRINTF("diff ldtr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
                   vcpu.data.ldtr.selector, vcpu.data.ldtr.base, vcpu.data.ldtr.limit, vcpu.data.ldtr.dpl,
                   bcpu.data.ldtr.selector, bcpu.data.ldtr.base, bcpu.data.ldtr.limit, bcpu.data.ldtr.dpl);
            flagDiff = True;
        }
        if (vcpu.data.gdtr.base != bcpu.data.gdtr.base ||
                vcpu.data.gdtr.limit != bcpu.data.gdtr.limit) {
            PRINTF("diff gdtr (V=%08X/%08X, B=%08X/%08X)\n",
                   vcpu.data.gdtr.base, vcpu.data.gdtr.limit,
                   bcpu.data.gdtr.base, bcpu.data.gdtr.limit);
            flagDiff = True;
        }
        if (vcpu.data.idtr.base != bcpu.data.idtr.base ||
                vcpu.data.idtr.limit != bcpu.data.idtr.limit) {
            PRINTF("diff idtr (V=%08X/%08X, B=%08X/%08X)\n",
                   vcpu.data.idtr.base, vcpu.data.idtr.limit,
                   bcpu.data.idtr.base, bcpu.data.idtr.limit);
            flagDiff = True;
        }
        if ((vcpu.data.eflags & ~mask) != (bcpu.data.eflags & ~mask)) {
            PRINTF("diff flags: V=%08X, B=%08X\n", vcpu.data.eflags, bcpu.data.eflags);
            flagDiff = True;
        }
        if (vcpuins.data.except) flagDiff = True;
    }

    if (flagDiff) {
        PRINTF("BEFORE EXECUTION:\n");
        PrintReg(&scpu);
        PrintSreg(&scpu);
        PrintCreg(&scpu);
        PRINTF("---------------------------------------------------\n");
        PRINTF("AFTER EXECUTION:\n");
        PRINTF("CURRENT bcpu:\n");
        for (i = 0; i < bcpuins.data.msize; ++i) {
            PRINTF("[%c:L%08x/%1d/%08x]\n",
                   bcpuins.data.mem[i].flagWrite ? 'W' : 'R',
                   bcpuins.data.mem[i].linear,
                   bcpuins.data.mem[i].byte,
                   bcpuins.data.mem[i].data);
        }
        PrintReg(&bcpu);
        PrintSreg(&bcpu);
        PrintCreg(&bcpu);
        PRINTF("---------------------------------------------------\n");
        PRINTF("CURRENT VCPU:\n");
        PRINTF("[E:L%08X]\n", vcpuins.data.linear);
        for (i = 0; i < vcpuins.data.msize; ++i) {
            PRINTF("[%c:L%08x/%1d/%08x]\n",
                   vcpuins.data.mem[i].flagWrite ? 'W' : 'R',
                   vcpuins.data.mem[i].linear,
                   vcpuins.data.mem[i].byte,
                   vcpuins.data.mem[i].data);
        }
        PrintReg(&vcpu);
        PrintSreg(&vcpu);
        PrintCreg(&vcpu);
        PRINTF("---------------------------------------------------\n");
    }

    return flagDiff;
}

static void PrintPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte,
                          t_bool flagWrite) {
    return;
    PRINTF("%s phy=%08x, data=", flagWrite ? "write" : "read", physical);
    switch (byte) {
    case 1:
        PRINTF("%02x",    d_nubit8(rdata));
        break;
    case 2:
        PRINTF("%04x",    d_nubit16(rdata));
        break;
    case 3:
        PRINTF("%08x",    d_nubit24(rdata));
        break;
    case 4:
        PRINTF("%08x",    d_nubit32(rdata));
        break;
    case 6:
        PRINTF("%016llx", d_nubit48(rdata));
        break;
    case 8:
        PRINTF("%016llx", d_nubit64(rdata));
        break;
    default:
        PRINTF("invalid");
        break;
    }
    PRINTF(", byte=%01x\n", byte);
}

void bochsApiRamReadPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
#if BOCHSAPI_BOCHS == True
    BX_CPU_THIS_PTR access_read_physical(physical, byte, (void *) rdata);
    // BX_MEM(0)->readPhysicalPage(BX_CPU(0), physical,byte, (void *) rdata);
    PrintPhysical(physical, rdata, byte, False);
#endif
}

void bochsApiRamWritePhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
#if BOCHSAPI_BOCHS == True
    // BX_CPU_THIS_PTR access_write_physical(physical, byte, (void *) rdata);
    // BX_MEM(0)->writePhysicalPage(BX_CPU(0), physical, byte, (void *) rdata);
    PrintPhysical(physical, rdata, byte, True);
#endif
}

void bochsApiDeviceStop() {
#if BOCHSAPI_BOCHS == True
    BX_CPU_THIS_PTR magic_break = 1;
#endif
}

void bochsApiInit() {
    vcpuInit();
    scpu = bcpu = vcpu;
}

void bochsApiFinal() {
    vcpuFinal();
}

static t_bool flagExecVcpu = False;

void bochsApiExecBefore() {
    t_nubit32 linear = Zero32;
#if BOCHSAPI_BOCHS == True
    CopyBochsCpu(&scpu); /* get init value */
#endif
    linear = scpu.data.cs.base + scpu.data.eip;

    if (linear == BOCHSAPI_LINEAR_START) {
        flagExecVcpu = True;
        PRINTF("Bochx CPU comparison starts here.\n");
    }
    if (linear == BOCHSAPI_LINEAR_STOP) {
        flagExecVcpu = False;
        PRINTF("Bochx CPU comparison stops here.\n");
        bochsApiDeviceStop();
    }
    if (flagExecVcpu) {
        vcpu = scpu;
        vcpuinsRefresh();
    }
    bcpuins.data.msize = 0;
}

void bochsApiExecAfter() {
    if (flagExecVcpu) {
#if BOCHSAPI_BOCHS == True
        CopyBochsCpu(&bcpu);
#endif
        if (CheckDiff()) {
            bochsApiDeviceStop();
        }
    }
}

void bochsApiRecordRam(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte,
                       t_bool flagWrite) {
    if (flagExecVcpu) {
        bcpuins.data.mem[bcpuins.data.msize].byte = byte;
        bcpuins.data.mem[bcpuins.data.msize].data = Zero64;
        MEMCPY((void *)(&(bcpuins.data.mem[bcpuins.data.msize].data)),
               (void *)(rdata), byte);
        bcpuins.data.mem[bcpuins.data.msize].linear = linear;
        bcpuins.data.mem[bcpuins.data.msize].flagWrite = flagWrite;
        bcpuins.data.msize++;
    }
}

#if BOCHSAPI_BOCHS == False
int main(int argc, char **argv) {
    printf("%s\n", PRODUCT);
    printf("Built on %s at %s.\n", __DATE__, __TIME__);
    printf("\n");
    bochsApiInit();
    bochsApiFinal();
    return 0;
}
#endif
