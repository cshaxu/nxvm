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
#include "dasm16s.h"

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

static void doHack() {
    hack((t_faddrcc) int20, 0x20);
    hack((t_faddrcc) int21, 0x21);
    hack((t_faddrcc) int27, 0x27);
    hack((t_faddrcc) int2a, 0x2a);
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
    /* very hacky */
    machineStart();
    while (vramRealDWord(Zero16, 0x0084) != 0x001940f8) utilsSleep(100);
    doHack();
    if (!device.flagRun) {
        machineResume();
    }
    /* **** ***** */
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
            doHack();
        } else if (!STRCMP(cmd, "debug")) {
            debugMain();
        } else if (!STRCMP(cmd, "record.start")) {
            PRINTF("Log file to output: ");
            FGETS(cmd, 0x100, stdin);
            cmd[STRLEN(cmd) - 1] = 0;
            deviceConnectDebugRecordStart(cmd);
        } else if (!STRCMP(cmd, "record.stop")) {
            deviceConnectDebugRecordStop();
        } else if (!STRCMP(cmd, "dasm")) {
            t_nubit16 len = 1;
            t_nubit16 seg = 0x0019;
            t_nubit16 off = 0x112d;
            t_nubit32 linear;
            t_nubit32 in;
            t_nubitcc i;
            char str[0x100];
            FILE *fp = NULL;
            PRINTF("Output file: ");
            FGETS(str, 0x100, stdin);
            str[STRLEN(str) - 1] = 0;
            fp = FOPEN(str, "w");
            PRINTF("Starting offset: ");
            scanf("%04x", &in);
            off = GetMax16(in);
            while (len && off < 0xfff0) {
                linear = (seg << 4) + off;
                len = dasm16s(str, (uint8_t *)(vram.connect.pBase + linear), off);
                FPRINTF(fp, "%04X:%04X(%08X)    %s", seg, off, linear, str);
                for (i = STRLEN(str); i < 36; ++i) {
                    FPRINTF(fp, " ");
                }
                FPRINTF(fp, "\n");
                off += len;
            }
            FCLOSE(fp);
        } else if (!STRCMP(cmd, "calc")) {
            t_nubit8 funId;
            t_nubit32 in;
            PRINTF("function id: ");
            scanf("%02x", &in);
            funId = GetMax8(in);
            PRINTF("0019:%04x\n", vramRealWord(0x0019, funId * 2 + 0x3e9e));
        }
    }
}

int exec(int argc, char **argv) {
    int exitCode = 0;
    machineInit();
    msdosInit();
#if 1
    test();
#else
    load(argc, argv);
    run();
    exitCode = ret();
#endif
    msdosFinal();
    machineFinal();
    return exitCode;
}
