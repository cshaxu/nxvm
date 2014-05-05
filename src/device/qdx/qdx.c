/* Copyright 2012-2014 Neko. */

/* QDX implements quick and dirty instruction loader */

#include "../../utils.h"
#include "../device.h"
#include "../vcpuins.h"

#include "qdcga.h"
#include "qdkeyb.h"
#include "qddisk.h"

#include "qdx.h"

t_faddrcc qdxTable[0x100];

static void QDX() {
	t_nubit8 cmdId;
    t_nubit16 flags;
	vcpu.data.eip++;
	if (vcpuinsReadLinear(vcpu.data.cs.base + vcpu.data.eip, GetRef(cmdId), 1)) {
		PRINTF("Cannot read data from L%08X.\n", vcpu.data.cs.base + vcpu.data.eip);
		deviceStop();
	} else {
		vcpu.data.eip++;
	}
    switch (cmdId) {
    case 0x00:
    case 0xff: /* STOP */
        PRINTF("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
               vcpu.data.cs.selector, vcpu.data.eip, cmdId);
        PRINTF("This happens because of the special instruction.\n");
        deviceStop();
        break;
    case 0x01:
    case 0xfe: /* RESET */
        PRINTF("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
               vcpu.data.cs.selector, vcpu.data.eip, cmdId);
        PRINTF("This happens because of the special instruction.\n");
        deviceReset();
        break;
    default: /* QDINT */
        ExecFun(qdxTable[cmdId]);
        if (vcpuins.data.cimm < 0x80) {
			/* all interrupt handlers taken over have intId less than 0x80 */
			/* set flags after executing of qdx interrupt handlers */
			if (vcpuinsReadLinear(vcpu.data.ss.base + vcpu.data.sp + 4, GetRef(flags), 2)) {
				PRINTF("Cannot read data from L%08X.\n", vcpu.data.ss.base + vcpu.data.sp + 4);
				deviceStop();
			}
            MakeBit(flags, VCPU_EFLAGS_ZF, _GetEFLAGS_ZF);
            MakeBit(flags, VCPU_EFLAGS_CF, _GetEFLAGS_CF);
            if (vcpuinsWriteLinear(vcpu.data.ss.base + vcpu.data.sp + 4, GetRef(flags), 2)) {
				PRINTF("Cannot write data to L%08X.\n", vcpu.data.ss.base + vcpu.data.sp + 4);
				deviceStop();
			}
        }
        break;
    }
    vcpuins.data.flagIgnore = 1;
}

void qdxInit() {
    int i;
    for (i = 0; i < 0x100; ++i) {
        qdxTable[i] = (t_faddrcc) NULL;
    }
    qdkeybInit();
    qdcgaInit();
    qddiskInit();
    /* takes over opcode 0xf1 */
    vcpuins.connect.insTable[0xf1] = (t_faddrcc) QDX;
}

void qdxReset() {
    qdcgaReset();
}

void qdxRefresh() {}

void qdxFinal() {}
