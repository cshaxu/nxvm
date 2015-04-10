/* Copyright 2012-2014 Neko. */

#include "../../src/machine.h"
#include "msdos.h"
#include "exec.h"

/* test only */
#include "../../src/utils.h"
#include "../../src/debug.h"
#include "../../src/device/device.h"
#include "../../src/device/vram.h"
#include "../../src/device/vbios.h"
#include "../../src/device/qdx/qdx.h"
#include "../../src/platform/platform.h"

static void hack(t_faddrcc fpInt, t_nubit8 intId) {
    t_nubit16 intIP = vramRealWord(Zero16, intId * 4 + 0);
    t_nubit16 intCS = vramRealWord(Zero16, intId * 4 + 2);
    /* redirect interrupt vector */
    vramRealWord(Zero16, intId * 4 + 0) = vbios.data.buildIP;
    vramRealWord(Zero16, intId * 4 + 2) = vbios.data.buildCS;
    /* install new interrupt service routine */
    qdxTable[intId] = (t_faddrcc) fpInt;
    vramRealByte(vbios.data.buildCS, vbios.data.buildIP) = 0xf1; /* qdx */
    vbios.data.buildIP++;
    vramRealByte(vbios.data.buildCS, vbios.data.buildIP) = intId; /* imm8 */
    vbios.data.buildIP++;
    /* save old interrupt vector */
    vramRealWord(vbios.data.buildCS, vbios.data.buildIP) = intIP;
    vbios.data.buildIP += 2;
    vramRealWord(vbios.data.buildCS, vbios.data.buildIP) = intCS;
    vbios.data.buildIP += 2;
}

static void test() {
    char cmd[0x100];
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
    deviceConnectFloppyInsert("d:/fd.img");
    deviceConnectHardDiskInsert("d:/hd.img");
    platform.flagMode = 1;
#else
    deviceConnectFloppyInsert("/Users/xha/fd.img");
    deviceConnectHardDiskInsert("/Users/xha/hd.img");
#endif
    cmd[0] = 0;
    while (STRCMP(cmd, "exit")) {
        PRINTF("Console> ");
        FGETS(cmd, 0x100, stdin);
        cmd[STRLEN(cmd) - 1] = 0;
        if (!STRCMP(cmd, "start")) {
            machineStart();
        } else if (!STRCMP(cmd, "resume")) {
            machineResume();
        } else if (!STRCMP(cmd, "reset")) {
            machineReset();
        } else if (!STRCMP(cmd, "stop")) {
            machineStop();
        } else if (!STRCMP(cmd, "hack")) {
            hack((t_faddrcc) int21, 0x21);
        } else if (!STRCMP(cmd, "debug")) {
            debugMain();
        } else if (!STRCMP(cmd, "record.start")) {
            PRINTF("Log file to dump: ");
            FGETS(cmd, 0x100, stdin);
            cmd[STRLEN(cmd) - 1] = 0;
            deviceConnectDebugRecordStart(cmd);
        } else if (!STRCMP(cmd, "record.stop")) {
            deviceConnectDebugRecordStop();
        }
    }
}

int exec(int argc, char **argv) {
    int exitCode = 0;
    machineInit();
#if 1
    test();
#else
    msdosInit();
    load(argc, argv);
    run();
    exitCode = ret();
    msdosFinal();
#endif
    machineFinal();
    return exitCode;
}
