/* Copyright 2012-2014 Neko. */

/* MSDOS is the virtual DOS kernel. */

#include "../../src/utils.h"
#include "../../src/device/device.h"
#include "../../src/device/vmachine.h"
#include "../../src/device/vcpuins.h"
#include "../../src/device/vram.h"

#include "msdos.h"

#define _ret(n) do {if (n) return;} while (0)

/* ***** CPU AND RAM INTERFACE ***** */
#define EAX vcpu.data.eax
#define ECX vcpu.data.ecx
#define EDX vcpu.data.edx
#define EBX vcpu.data.ebx
#define ESP vcpu.data.esp
#define EBP vcpu.data.ebp
#define ESI vcpu.data.esi
#define EDI vcpu.data.edi
#define EFLAGS vcpu.data.eflags
#define EIP vcpu.data.eip

#define AX vcpu.data.ax
#define CX vcpu.data.cx
#define DX vcpu.data.dx
#define BX vcpu.data.bx
#define SP vcpu.data.sp
#define BP vcpu.data.bp
#define SI vcpu.data.si
#define DI vcpu.data.di
#define FLAGS vcpu.data.flags
#define IP vcpu.data.ip

#define ES vcpu.data.es.selector
#define CS vcpu.data.cs.selector
#define SS vcpu.data.ss.selector
#define DS vcpu.data.ds.selector
#define FS vcpu.data.fs.selector
#define GS vcpu.data.gs.selector

#define AH vcpu.data.ah
#define CH vcpu.data.ch
#define DH vcpu.data.dh
#define BH vcpu.data.bh
#define AL vcpu.data.al
#define CL vcpu.data.cl
#define DL vcpu.data.dl
#define BL vcpu.data.bl

#define LOAD_ES deviceConnectCpuLoadES
#define LOAD_CS deviceConnectCpuLoadCS
#define LOAD_SS deviceConnectCpuLoadSS
#define LOAD_DS deviceConnectCpuLoadDS
#define LOAD_FS deviceConnectCpuLoadFS
#define LOAD_GS deviceConnectCpuLoadGS

#define _BYTE  vramRealByte
#define _WORD  vramRealWord
#define _DWORD vramRealDWord

/* stack pointer size */
#define _GetStackSize   (vcpu.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_addrsize) ? 4 : 2)

static void PUSH(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    cesp = GetMax16(SP - byte);
    vramWritePhysical(vcpu.data.ss.base + cesp, rdata, byte);
    SP -= byte;
}
static void POP(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp = SP;
    vramReadPhysical(vcpu.data.ss.base + cesp, rdata, byte);
    if (rdata != GetRef(ESP)) {
        SP += byte;
    }
}

static void GOTO(t_nubit16 seg, t_nubit16 off) {
    LOAD_CS(seg);
    EIP = off;
}
static void INVOKE(t_nubit16 seg, t_nubit16 off, t_nubit16 retSeg, t_nubit16 retOff) {
    GOTO(seg, off);
    while (CS != retSeg || IP != retOff) {
        vmachineRefresh();
    }
}

static void INT(t_nubit8 intId, t_nubit16 retOff) {
    t_nubit16 intCS, intIP;
    PUSH(GetRef(FLAGS), 2);
    _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    PUSH(GetRef(CS), 2);
    PUSH(GetRef(retOff), 2);
    intIP = _WORD(Zero16, intId * 4 + 0);
    intCS = _WORD(Zero16, intId * 4 + 2);
    INVOKE(intCS, intIP, CS, retOff);
}
static void IRET() {
    t_nubit16 flags = Zero32;
    t_nubit16 mask = GetMax16(VCPU_EFLAGS_RESERVED);
    POP(GetRef(IP), 2);
    POP(GetRef(CS), 2);
    LOAD_CS(CS);
    POP(GetRef(flags), 2);
    FLAGS = (flags & ~mask) | (FLAGS & mask);
}
static void CALL_NEAR(t_nubit16 off, t_nubit16 retOff) {
    PUSH(GetRef(retOff), 2);
    INVOKE(CS, off, CS, retOff);
}
static void CALL_FAR(t_nubit16 seg, t_nubit16 off, t_nubit16 retOff) {
    PUSH(GetRef(CS), 2);
    PUSH(GetRef(retOff), 2);
    INVOKE(seg, off, CS, retOff);
}
static void RET_NEAR() {
    POP(GetRef(IP), 2);
}
static void RET_FAR(t_nubit8 count) {
    POP(GetRef(IP), 2);
    POP(GetRef(CS), 2);
    LOAD_CS(CS);
    SP += count;
}
static void JMP_NEAR(t_nubit16 off) {
    GOTO(CS, off);
}
static void JMP_FAR(t_nubit16 seg, t_nubit16 off) {
    GOTO(seg, off);
}

/* ***** MSDOS KERNEL VARIABLES ***** */
static t_faddrcc fpTable[0x100];
static t_nubit16 kernelDataSeg; /* kernel segment address */

#define CODE_STARTS_AT(seg, off)

#define kb(off) _BYTE(kernelDataSeg, (off))
#define kw(off) _WORD(kernelDataSeg, (off))
#define kd(off) _DWORD (kernelDataSeg, (off))

#define KCODE_ADDR_DATA_SEG 0x3de7
#define KCODE_ADDR_FUNC_OFF 0x3e9e

#define KDATA_FLAG_MODIFY    0x0320
#define KDATA_FLAG_INT21     0x0321
#define KDATA_ADDR_PSP       0x0330
#define KDATA_FLAG_ENV       0x0572
#define KDATA_FLAG_TERMINATE 0x057c
#define KDATA_SAVE_SP        0x0584
#define KDATA_SAVE_SS        0x0586
#define KDATA_SAVE_BX        0x05ea
#define KDATA_SAVE_DS        0x05ec
#define KDATA_ADDR_RET_TEMP  0x05ee

#define PSP_INT_20    0x00
#define PSP_RAM_TOP   0x02
#define PSP_CPM_CALL  0x05
#define PSP_CPM_COM   0x06
#define PSP_INT_22    0x0a
#define PSP_INT_23    0x0e
#define PSP_INT_24    0x12
#define PSP_PARENT    0x16
#define PSP_FHA       0x18
#define PSP_ADDR_ENV  0x2c
#define PSP_STACK_SP  0x2e
#define PSP_STACK_SS  0x30
#define PSP_FHA_SIZE  0x32
#define PSP_FHA_PTR   0x34
#define PSP_PREVIOUS  0x38
#define PSP_DOS_FUNC  0x50
#define PSP_FCB_1     0x5c
#define PSP_FCB_2     0x6c
#define PSP_DTA       0x80
#define PSP_CMD_LEN   0x80
#define PSP_CMD_STR   0x81

/* ***** KERNEL UTILITY SECTION ***** */
/**
 * Releases all file hpts opened in current psp
 * Input: (unknown)
 * Output: (unknown)
 */
static void _6DC1_releaseFileHpt() {
    CALL_NEAR(0x6dc1, 0x946c);
}
/**
 * Restores 8 register values
 * Input: (null)
 * Output: restored AX, BX, CX, DX, SI, DI, BP, DS
 */
static void _424C_restoreRegisters() {
    CODE_STARTS_AT(0x0019, 0x424c);
    POP(GetRef(AX), 2);
    POP(GetRef(BX), 2);
    POP(GetRef(CX), 2);
    POP(GetRef(DX), 2);
    POP(GetRef(SI), 2);
    POP(GetRef(DI), 2);
    POP(GetRef(BP), 2);
    POP(GetRef(DS), 2);
    LOAD_DS(DS);
    /* note that ES is not restored, still in program stack */
}
/**
 * Validates current mcb segment address
 * Input: AX = current mcb segment address, DI = Zero16
 * Output: ES = current mcb segment address
 *         CF is set if next mcb is not valid
 */
static void _A25C_validateMcb() {
    LOAD_ES(AX);
    if (_BYTE(ES, 0x0000) != 'M' && _BYTE(ES, 0x0000) != 'Z') {
        _SetEFLAGS_CF;
    } else {
        _ClrEFLAGS_CF;
    }
}
/**
 * Gets next mcb segment address and validate this new mcb
 * Input: DS = current mcb segment address
 * Output: ES = next mcb segment address
 *         CF is set if next mcb is not valid
 */
static void _A255_getAndValidateNextMcb() {
    AX = DS + _WORD(DS, 0x0003) + 0x0001; /* calculate next mcb segment address */
    _A25C_validateMcb();
}
/**
 * Gets current program stack pointer
 * Input: (null)
 * Output: DS = program's stack segment, SI = program's stack offset
 */
static void _4282_getCurrentStackPointer() {
    LOAD_DS(kw(KDATA_SAVE_SS));
    SI = kw(KDATA_SAVE_SP);
}
/**
 * Gets interrupt vector address
 * Input: AL = interrupt id
 * Output: ES = 0, BX = AL * 4
 */
static void _4CBE_getIntVecAddr() {
    LOAD_ES(Zero16);
    BX = AL * 4;
}
/**
 * Gets interrupt vector and load to current stack
 * Input: AL = interrupt id
 * Output: ES = 0, BX = AL * 4, DS = Current Program Stack
 * Load: DS:[SI + 10] = ES, DS:[SI+02] = BX
 */
static void _4C8A_getInterruptVector() {
    t_nubit16 intSeg, intOff;
    _4CBE_getIntVecAddr();
    intSeg = _WORD(ES, BX + 0x02);
    intOff = _WORD(ES, BX + 0x00);
    LOAD_ES(intSeg);
    BX = intOff;
    _4282_getCurrentStackPointer();
    _WORD(DS, SI + 0x10) = ES;
    _WORD(DS, SI + 0x02) = BX;
}
/**
 * Releases memory occupied by current program
 * Input: BX = process id (psp segment addr of program)
 */
static void _A222_freeMemory() {
    AX = _WORD(SS, 0x0024); /* get first mcb address */
    DI = Zero16; /* free mcb flag */
    _A25C_validateMcb(); /* validate mcb */

    do {
        CODE_STARTS_AT(0x0019, 0xa22c);
        /* mcb is invalid */
        if (_GetEFLAGS_CF) {
            return;
        }
        LOAD_DS(ES);
        /* mcb is owned by current program */
        if (_WORD(DS, 0x0001) == BX) {
            _WORD(DS, 0x0001) = Zero16;
        }
        /* last mcb starts with char 'Z' */
        if (_BYTE(DS, 0x0000) == 'Z') {
            AX = _WORD(SS, 0x008c);
            /* if something is out of range (?) */
            if (AX == 0xffff || DS >= AX) {
                return;
            } else {
                DI = Zero16; /* free mcb flag */
                _A25C_validateMcb(); /* validate mcb */
            }
        } else {
            /* get next mcb and validate */
            _A255_getAndValidateNextMcb();
        }
    } while (1);
}
/**
 * Input from stdin
 * Output: AL = character from stdin
 */
static void _544B_readFromStdin() {
    t_nubit16 saveBX;
    do {
        CODE_STARTS_AT(0x0019, 0x544b);
        saveBX = BX;
        BX = 0x0000;
        CALL_NEAR(0x7583, 0x5451); /* get stdin device hpt */
        BX = saveBX;
        /* failed to get hpt */
        if (_GetEFLAGS_CF) {
            return;
        }
        AH = 0x01;
        CALL_NEAR(0x8413, 0x5459); /* i/o command peek */
        /* stdin is ready */
        if (!_GetEFLAGS_ZF) {
            AH = 0x00;
            CALL_NEAR(0x8413, 0x5466); /* i/o command input */
            return;
        }
        AH = 0x84;
        INT(0x2a, 0x545f); /* keyboard busy loop */
        CALL_NEAR(0x9109, 0x544b); /* check redirection status */
    } while (1);
}
/*
 * Output to stdout
 * Input: AL = char to output
 */
static void _5467_writeToStdout() {
    t_nubit16 saveBX, tempDS;
    CODE_STARTS_AT(0x0019, 0x5467);
    saveBX = BX;
    BX = 0x0001;
    CALL_NEAR(0x7583, 0x546e); /* get stdout device hpt */
    /* failed to get hpt */
    if (_GetEFLAGS_CF) {
        BX = saveBX;
        return;
    }
    BX = _WORD(DS, SI + 0x05) & 0x8080; /* get device attrib from hpt */
    /* is not char device */
    if (BX != 0x0080) {
        CALL_NEAR(0x5497, 0x5490); /* send char to device of 0x0080 */
    } else {
        /* get device header table  */
        BX = _WORD(DS, SI + 0x07);
        tempDS = _WORD(DS, SI + 0x09);
        /* device is CON */
        if (GetBit(_BYTE(tempDS, BX + 0x04), 0x10)) {
            INT(0x29, 0x548a); /* send char to con (crt) */
        } else {
            CALL_NEAR(0x5497, 0x5490); /* send char to device in current BX */
        }
    }
    _ClrEFLAGS_CF;
    BX = saveBX;
}
/**
 * Outputs regular char to display
 * Input: AL = char to output
 */
static void _54F8_printRegularChar() {
    t_nubit8 count8 = _BYTE(SS, 0x0300);
    t_nubit16 saveAX, saveBX, saveDS = DS, saveSI = SI;
    /* check ^CSPN every 64 chars */
    _BYTE(SS, 0x0300) = count8 = (count8 + 0x01) & 0x3f;
    if (count8 == 0x00) {
        saveAX = AX;
        /* check ^CSPN */
        CALL_NEAR(0x9137, 0x550b);
        AX = saveAX;
    }
    _5467_writeToStdout(); /* print char to stdout */
    LOAD_DS(saveDS);
    SI = saveSI;
    /* ^P flag is clear, finish */
    if (!_BYTE(SS, 0x02fe)) {
        return;
    }
    saveBX = BX;
    /* get stdout handler param table */
    BX = 0x0001;
    CALL_NEAR(0x7583, 0x5522);
    /* fail to get stdout hpt */
    if (_GetEFLAGS_CF) {
        BX = saveBX;
        LOAD_DS(saveDS);
        SI = saveSI;
        return;
    }
    /* get device type */
    BX = _WORD(DS, SI + 0x05);
    /* remote device or block device */
    if (GetBit(BH, 0x80) || !GetBit(BL, 0x80)) {
        BX = saveBX;
        LOAD_DS(saveDS);
        SI = saveSI;
        return;
    }
    /* get std printer handle param table */
    BX = 0x0004;
    CALL_NEAR(0x7583, 0x5537);
    /* fail to get stdout hpt */
    if (_GetEFLAGS_CF) {
        BX = saveBX;
        LOAD_DS(saveDS);
        SI = saveSI;
        return;
    }
    /* printer not enabled */
    if (GetBit(_BYTE(DS, SI + 0x06), 0x08)) {
        _BYTE(SS, 0x02fe) = 0x00;
        BX = saveBX;
        LOAD_DS(saveDS);
        SI = saveSI;
        return;
    }

    CODE_STARTS_AT(0x0019, 0x55cf);
    CALL_NEAR(0x5492, 0x55d2); /* output char to device specified in BX */
    BX = saveBX;
    LOAD_DS(saveDS);
    SI = saveSI;
}
/**
 * Outputs char to display
 * Input: AL = char to output
 */
static void _54EB_printChar() {
    t_nubit8 count;
    CODE_STARTS_AT(0x0019, 0x554b);
    if (AL == 0x0d) {
        /* AL is CR enter char */
        _BYTE(SS, 0x01f9) = 0x00; /* move cursor to the begging of line */
    } else if (AL == 0x08) {
        /* AL is backspace char */
        _BYTE(SS, 0x01f9) -= 0x01; /* move cursor right */
    } else if (AL == 0x09) {
        /* AL is tab char */
        count = 0x00 - (_BYTE(SS, 0x01f9) | 0xf8);
        while (count > 0) {
            AL = 0x20; /* output space char */
            _54F8_printRegularChar();
            count--;
        }
        return;
    } else if (AL != 0x7f) {
        /* AL is not DEL char */
        _BYTE(SS, 0x01f9) += 0x01; /* move cursor right */
    }
    _54F8_printRegularChar();
}
/**
 * Outputs char to device
 * Input: BX = device type (3=stdaux, 4=stdprn), DL = character to output
 */
static void _55C6_printCharToDevice() {
    t_nubit16 saveAX, saveDS, saveSI;
    AL = DL;
    /* check ^CSPN */
    saveAX = AX;
    CALL_NEAR(0x9137, 0x55cc);
    AX = saveAX;
    /* output char to device specified in BX */
    saveDS = DS;
    saveSI = SI;
    CALL_NEAR(0x5492, 0x55d2);
    LOAD_DS(saveDS);
    SI = saveSI;
}
/**
 * Terminates program
 * Input: AH = return type, AL = return code
 * Output: Free memory, CS:IP = return address
 */
static t_bool execTerminate_A1FF() {
    t_nubit16 parentPspSeg;
    t_nubit16 returnPtr, returnSeg;
    t_nubit16 pspSeg = kw(KDATA_ADDR_PSP);
    t_nubit16 tempFlags;

    CODE_STARTS_AT(0x0019, 0xa1ff);
    AH = kb(KDATA_FLAG_TERMINATE);
    kw(0x0334) = AX; /* save return type and return code */
    _4282_getCurrentStackPointer(); /* load current program stack pointer to ds:si */
    LOAD_DS(_WORD(DS, SI + 0x14));
    AX = Zero16;
    _DWORD(Zero16, 0x0088) = _DWORD(pspSeg, PSP_INT_22);
    _DWORD(Zero16, 0x008c) = _DWORD(pspSeg, PSP_INT_23);
    _DWORD(Zero16, 0x0090) = _DWORD(pspSeg, PSP_INT_24);

    INT(0x2f, 0x9433);
    PUSH(GetRef(DS), 2);
    AH = 0x82;
    INT(0x2a, 0x9438); /* clear dos critical section */
    kb(0x0359) = 0xff; /* set flag for exiting program */
    AX = 0x1122;
    INT(0x2f, 0x9443);
    /* copy int 22 vector to return stack */
    AL = 0x22;
    _4C8A_getInterruptVector();
    POP(GetRef(CX), 2); /* CX = current psp segment, comes from DS:[SI+14] */

    CODE_STARTS_AT(0x0019, 0x9449);
    /* save int 22 vector */
    PUSH(GetRef(ES), 2);
    PUSH(GetRef(BX), 2);
    LOAD_DS(BX = pspSeg); /* BX = current psp segment, comes from KDS:[0330] */
    AX = parentPspSeg = _WORD(pspSeg, PSP_PARENT); /* AX = parent psp segment */
    /* parent psp is not the current psp */
    if (AX != BX) {
        /* and current psp is not built by int 21 fun 26 */
        if (BX == CX) {
            PUSH(GetRef(AX), 2);
            /* this is not a tsr */
            if (kb(KDATA_FLAG_TERMINATE) != 0x03) {
                /* free memory */
                _A222_freeMemory();
                /* close handlers */
                _6DC1_releaseFileHpt();
            }
            /* transfer control to parent */
            POP(GetRef(pspSeg), 2);
            kw(KDATA_ADDR_PSP) = pspSeg;
        }
    }

    CODE_STARTS_AT(0x0019, 0x9471);
    LOAD_DS(kernelDataSeg);

    AL = 0xff;
    CALL_NEAR(0x513a, 0x9478); /* ? */
    CALL_NEAR(0x9a35, 0x947b); /* flush buffer to disk */
    CALL_NEAR(0x515a, 0x947e); /* ? */
    CALL_NEAR(0x8bd6, 0x9481); /* close devices */
    _ClrEFLAGS_IF;
    kb(0x0321) = 0x00; /* clear dos kernel flag */
    kb(0x0322) = 0xff; /* set disk not writable */
    kb(0x0359) = 0x00; /* ? */
    POP(GetRef(returnPtr), 2);
    POP(GetRef(returnSeg), 2);
    kw(0x0580) = returnPtr;
    kw(0x0582) = returnSeg;

    CODE_STARTS_AT(0x0019, 0x9499);
    /* switch to program stack */
    LOAD_SS(_WORD(pspSeg, PSP_STACK_SS));
    SP = _WORD(pspSeg, PSP_STACK_SP);
    LOAD_DS(pspSeg);
    _424C_restoreRegisters();
    POP(GetRef(ES), 2);
    LOAD_ES(ES);

    SP += 0x0004; /* pop out values when executing this program: IP, CS */
    POP(GetRef(tempFlags), 2); /* load previous program's flag register */
    FLAGS = 0xf200 | (GetMax8(FLAGS) & 0x02);

    /* return to parent program */
    GOTO(kw(0x0582), kw(0x0580));
    return True;
}

static void kernelEntry();
static t_bool checkCtrlBreak() {
    CODE_STARTS_AT(0x0019, 0x9080);
    /* check if currently on DOS internal stack */
    if (_BYTE(SS, KDATA_FLAG_INT21) != 0x01) {
        return False;
    }
    CODE_STARTS_AT(0x0019, 0x9089);
    /* save registers */
    PUSH(GetRef(CX), 2);
    PUSH(GetRef(ES), 2);
    PUSH(GetRef(BX), 2);
    PUSH(GetRef(DS), 2);
    PUSH(GetRef(SI), 2);
    LOAD_ES(SS);
    LOAD_DS(SS);

    /* build device driver request header for command 05 peek*/
    _BYTE(SS, 0x0394) = 0x05; /* command 05 */
    _BYTE(SS, 0x0392) = 0x0e; /* length of header is 0e */
    _WORD(SS, 0x0395) = 0x0000; /* initialize status word */

    /* send request header address to DS:BX */
    BX = 0x0392;

    /* send device header address to DS:SI */
    SI = _WORD(SS, 0x0032);
    LOAD_DS(_WORD(SS, 0x0034));

    /* send peek command */
    CALL_NEAR(0x85ed, 0x90b2);

    /* ***** 0019:90b2 ***** */
    /* test if device is busy */
    if (GetBit(_BYTE(SS, 0x0396), 0x02)) {
        AL = 0x00; /* op fails */
        POP(GetRef(SI), 2);
        POP(GetRef(DS), 2);
        LOAD_DS(DS);
        POP(GetRef(BX), 2);
        POP(GetRef(ES), 2);
        LOAD_ES(ES);
        POP(GetRef(CX), 2);
        return False;
    }

    /* ***** 0019:90c2 ***** */
    /* get char from device */
    AL = _BYTE(SS, 0x039f);

    /* char is not ^C */
    if (AL != 0x03) {
        POP(GetRef(SI), 2);
        POP(GetRef(DS), 2);
        LOAD_DS(DS);
        POP(GetRef(BX), 2);
        POP(GetRef(ES), 2);
        LOAD_ES(ES);
        POP(GetRef(CX), 2);
        return False;
    }

    /* build device driver request header for command 04 input */
    _BYTE(SS, 0x0394) = 0x04; /* command = 04 */
    _BYTE(SS, 0x0392) = 0x16; /* length = 16 */
    _BYTE(SS, 0x039f) = CL; /* media descriptor */
    _WORD(SS, 0x0395) = 0x0000; /* initialize status word */
    _WORD(SS, 0x03a4) = 0x0001; /* input 1 char */

    /* send input command */
    CALL_NEAR(0x85ed, 0x90bc);

    POP(GetRef(SI), 2);
    POP(GetRef(DS), 2);
    LOAD_DS(DS);
    POP(GetRef(BX), 2);
    POP(GetRef(ES), 2);
    LOAD_ES(ES);
    POP(GetRef(CX), 2);

    CODE_STARTS_AT(0x0019, 0x91c4);
    /* check flag that do not output char ^C */
    if (!GetBit(_BYTE(SS, 0x0612), 0x02)) {
        /* output ^C */
        AL = 0x03;
        CALL_NEAR(0x5582, 0x91d1);
        /* output enter */
        CALL_NEAR(0x5412, 0x91d4);
    }

    /* ***** 0019:91d4 ***** */
    LOAD_DS(SS);
    if (kb(0x0357) != 0x00) {
        CALL_NEAR(0x71e3, 0x91e0);
    }

    /* ***** 0019:91e0 ***** */
    _ClrEFLAGS_IF;
    /* switch from DOS stack to program stack */
    LOAD_SS(kw(KDATA_SAVE_SS));
    SP = kw(KDATA_SAVE_SP);

    /* restore registers */
    _424C_restoreRegisters();
    POP(GetRef(ES), 2);
    LOAD_ES(ES);

    /* set DOS internal stack flag */
    kb(KDATA_FLAG_INT21) = 0x00;
    /* set flag for int 24 not ready */
    kb(KDATA_FLAG_MODIFY) = 0x00;
    /* save SP and pop one word because now in a near call */
    kw(0x0332) = SP;
    kw(0x0332) += 0x0002;

    _ClrEFLAGS_CF;

    /* if program has ctrl-break processor */
    if (kb(0x1211) == 0x00) {
        CODE_STARTS_AT(0x0019, 0x9213);
        /* call default ctrl-break processor */
        CALL_FAR(_WORD(CS, 0x9076), _WORD(CS, 0x9074), 0x9219);
    } else {
        CODE_STARTS_AT(0x0019, 0x920e);
        /* call program defined ctrl-break processor */
        INT(0x23, 0x9211);
    }

    CODE_STARTS_AT(0x0019, 0x9219);
    _ClrEFLAGS_IF;
    kw(0x0d0a) = DS; /* save DS value */
    kw(0x033a) = AX; /* save return value */

    LOAD_DS(kernelDataSeg);

    /* if not in dos int service */
    if (SP != kw(0x0332)) {
        CODE_STARTS_AT(0x0019, 0x923b);
        SP += 0x0002; /* because the ctrl-break check function is a near call */
        /* PUSHF; POP AX; TEST AL, 01 */
        if (_GetEFLAGS_CF) {
            LOAD_DS(kw(0x0d0a)); /* restore DS value */
            AH = 0x4c; /* invoke dos int 21 function 4c */
            AL = 0x00; /* return value from ctrl-break is 00 */
            kb(0x034d) = 0xff;
            kernelEntry();
            return True;
        }
    }

    CODE_STARTS_AT(0x0019, 0x9231);
    AX = kw(0x033a); /* restore return value to AX */
    LOAD_DS(kw(0x0d0a)); /* restore DS value */
    kernelEntry();
    return True;
}
static t_bool asmFuncBegin() {
    t_nubit16 temp16;
    CODE_STARTS_AT(0x0019, 0x411b);
    /* save 9 registers */
    PUSH(GetRef(ES), 2);
    PUSH(GetRef(DS), 2);
    PUSH(GetRef(BP), 2);
    PUSH(GetRef(DI), 2);
    PUSH(GetRef(SI), 2);
    PUSH(GetRef(DX), 2);
    PUSH(GetRef(CX), 2);
    PUSH(GetRef(BX), 2);
    PUSH(GetRef(AX), 2);
    /* save current DS:BX */
    kw(KDATA_SAVE_DS) = DS;
    kw(KDATA_SAVE_BX) = BX;
    /* save program stack pointer to data area and program psp */
    kw(0x05f2) = kw(KDATA_SAVE_SP);
    kw(0x05f0) = kw(KDATA_SAVE_SS);
    /* clear env flag */
    kb(KDATA_FLAG_ENV) = 0x00;
    /* ? */
    if (!GetBit(kb(0x1030), 0x01)) {
        kw(0x033e) = 0x0000;
    }
    CODE_STARTS_AT(0x0019, 0x414d);
    /* set dos stack flag */
    kb(KDATA_FLAG_INT21) += 0x01;
    /* save current SS:SP for dos kernel */
    kw(KDATA_SAVE_SP) = SP;
    kw(KDATA_SAVE_SS) = SS;
    /* ? copy current psp seg address to sub-program psp seg address unit */
    kw(0x033c) = kw(KDATA_ADDR_PSP);
    /* load psp seg address to DS */
    LOAD_DS(kw(KDATA_ADDR_PSP));
    /* save current SS:SP to PSP */
    _WORD(kw(KDATA_ADDR_PSP), PSP_STACK_SP) = SP;
    _WORD(kw(KDATA_ADDR_PSP), PSP_STACK_SS) = SS;
    /* switch to DOS kernel data segment */
    LOAD_SS(kernelDataSeg);
    SP = 0x07a0;
    _SetEFLAGS_IF;
    /* ? */
    kb(0x05f6) = 0x00;
    /* ? */
    kw(0x0611) &= 0x0800;
    /* set sub-folder mode */
    kb(0x034c) = 0x00;
    /* clear int 24 failure flag */
    kb(0x034a) = 0x00;
    /* clear I/O redirection */
    kb(0x0357) = 0x00;
    kb(0x0358) = 0x01;
    /* calculate function entry offset */
    BH = 0x00;
    BL = AH * 2;
    _ClrEFLAGS_DF;

    if (AH >= 0x01 && AH <= 0x0c) {
        /* modification flag is clear */
        if (kb(KDATA_FLAG_MODIFY) == 0x00) {
            /* set DOS internal stack pointer */
            SP = 0x0aa0;
        }
    } else if (AH != 0x59) {
        kw(0x033a) = AX;
        /* set flag for unknown error position */
        kb(0x0323) = 0x01;
        /* clear modification flag */
        kb(KDATA_FLAG_MODIFY) = 0x00;
        /* set flag for 'cannot write into disk drive' */
        kb(0x0322) = 0xff;
        /* clear dos critical section 0 - 7 */
        temp16 = AX;
        AH = 0x82;
        INT(0x2a, 0x41d3);
        AX = temp16;
        /* ? */
        kb(0x0358) = 0x00;
        /* change DOS internal stack pointer */
        SP = 0x0920;
        /* test ctrl-break flag */
        if (kb(0x0337) & 0xff) {
            /* do ctrl-break check */
            temp16 = AX;
            if (checkCtrlBreak()) {
                return True;
            }
            AX = temp16;
        }
    }
    CODE_STARTS_AT(0x0019, 0x41ea);
    /* recover DS:BX */
    BX = kw(KDATA_SAVE_BX);
    LOAD_DS(kw(KDATA_SAVE_DS));
    kw(KDATA_SAVE_BX) = AH * 2 + KCODE_ADDR_FUNC_OFF;
    return False;
}
static t_bool asmFuncEnd() {
    CODE_STARTS_AT(0x0019, 0x41f9);
    /* ? */
    kb(0x0086) &= 0xfb;
    _ClrEFLAGS_IF;
    LOAD_DS(kernelDataSeg);
    /* ? */
    if (kb(0x0085) != 0x00) {
        /* who equals to program's psp? */
        if (kw(0x0063) == kw(KDATA_ADDR_PSP)) {
            CODE_STARTS_AT(0x0019, 0x4242);
            kb(0x0085) -= 0x01;
            /* this is shit! it jumps to invalid code section. */
            JMP_FAR(kernelDataSeg, 0x112d);
            return True;
        }
    }

    CODE_STARTS_AT(0x0019, 0x4211);
    /* clear int 21 executing flag */
    kb(KDATA_FLAG_INT21) -= 0x01;

    /* restore SS:SP */
    LOAD_SS(kw(KDATA_SAVE_SS));
    SP = kw(KDATA_SAVE_SP);

    /* restore program stack pointer */
    kw(KDATA_SAVE_SS) = kw(0x05f0);
    kw(KDATA_SAVE_SP) = kw(0x05f2);

    /* send return value to caller's stack */
    /* AH in stack is already modified by function routine */
    _BYTE(SS, SP) = AL;

    /* restore 9 registers */
    POP(GetRef(AX), 2);
    POP(GetRef(BX), 2);
    POP(GetRef(CX), 2);
    POP(GetRef(DX), 2);
    POP(GetRef(SI), 2);
    POP(GetRef(DI), 2);
    POP(GetRef(BP), 2);
    POP(GetRef(DS), 2);
    LOAD_DS(DS);
    POP(GetRef(ES), 2);
    LOAD_ES(ES);

    IRET();
    return True;
}

/* ***** MSDOS FUNCTION ENTRY ***** */
/* ALL FUNCTIONS IN THIS SECTION SHOULD BE INVOKED AT LAST */
/**
 * INT 21,00 - Program Terminate
 * Input: AH = 00, CS = psp segment address
 */
static void F00() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0xa1f7);
    AL = 0x00; /* no return code */
    kb(KDATA_FLAG_TERMINATE) = 0x00; /* terminate type 00 */
    _ret(execTerminate_A1FF());
}
/**
 * INT 21,01 - Keyboard Input with Echo
 * Input: AH = 01
 * Output: AL = character from stdin
 */
static void F01() {
    t_nubit16 tempAX;
    CODE_STARTS_AT(0x0019, 0x54e0);
    _ret(asmFuncBegin());
    CALL_NEAR(0x51ba, 0x54e3); /* input char from keyboard */
    tempAX = AX;
    CALL_NEAR(0x54eb, 0x54e7); /* output to display */
    AX = tempAX;
    _ret(asmFuncEnd());
}
/**
 * INT 21,2 - Display Output
 * Input: AH = 02, DL = character to stdout
 */
static void F02() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x54e9);
    AL = DL;
    _54EB_printChar();
    _ret(asmFuncEnd());
}
/**
 * INT 21,3 - Wait for Auxiliary Device Input
 * Input: AH = 03
 * Output: AL = character from stdaux
 */
static void F03() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x559f);
    CALL_NEAR(0x9137, 0x55a2); /* check ^CSPN */
    /* get stdaux hpt */
    BX = 0x0003;
    CALL_NEAR(0x7583, 0x55a8);
    /* succeed to get stdaux hpt */
    if (!_GetEFLAGS_CF) {
        /* get input char, blocking */
        AH = 0x01;
        CALL_NEAR(0x8413, 0x55b4); /* AH = 1, I/O Command 05, Peek */
        while (_GetEFLAGS_ZF) {
            CALL_NEAR(0x9109, 0x55af); /* sleep */
            /* AH = 1, I/O Command 05, Peek */
            AH = 0x01;
            CALL_NEAR(0x8413, 0x55b4);
        }
        AH = 0x00;
        CALL_NEAR(0x8413, 0x55b4); /* AH = 1, I/O Command 04, Input */
    }
    _ret(asmFuncEnd());
}
/**
 * INT 21,4 - Auxiliary Output
 * Input: AH = 04, DL = character to stdaux
 */
static void F04() {
    t_nubit16 saveBX;
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x55bc);
    saveBX = BX;
    BX = 0x0003;
    _55C6_printCharToDevice();
    BX = saveBX;
    _ret(asmFuncEnd());
}
/**
 * INT 21,5 - Printer Output
 * Input: AH = 05, DL = character to stdprn
 */
static void F05() {
    t_nubit16 saveBX;
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x55c2);
    saveBX = BX;
    BX = 0x0004;
    _55C6_printCharToDevice();
    BX = saveBX;
    _ret(asmFuncEnd());
}
/**
 * INT 21,6 - Direct Console I/O
 * Input: AH = 06
 *        DL = (00-FE character to output)
 *        DL = FF for console input
 * Output: (null) if DL != FF; otherwise:
 *         AL = intput character from console when DL = FF
 *         ZF is clear if console input char ready in AL
 */
static void F06() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x541c);
    AL = DL;
    if (DL == 0xff) {
        /* stdin */
        CODE_STARTS_AT(0x0019, 0x5425);
        /* load program stack ptr */
        LOAD_ES(_WORD(SS, KDATA_SAVE_SS));
        DI = _WORD(SS, KDATA_SAVE_SP);
        BX = 0x0000; /* stdin device id */
        CALL_NEAR(0x7583, 0x542f); /* get device hpt */
        if (!_GetEFLAGS_CF) {
            /* succeed to get device hpt */
            AH = 0x01;
            CALL_NEAR(0x8413, 0x5436); /* check device stdin status */
            if (!_GetEFLAGS_ZF) {
                /* stdin is ready */
                CODE_STARTS_AT(0x0019, 0x5443);
                _BYTE(ES, DI + 0x16) &= 0xbf;
                CALL_NEAR(0x9109, 0x544b); /* check redirection */
                _544B_readFromStdin();
            } else {
                /* stdin is not ready */
                CALL_NEAR(0x9109, 0x543b); /* check redirection */
                _BYTE(ES, DI + 0x16) |= 0x40;
                AL = 0x00;
            }
        }
    } else {
        /* stdout */
        _5467_writeToStdout();
    }
    _ret(asmFuncEnd());
}
/**
 * INT 21,7 - Direct Console Input Without Echo
 * Input: AH = 07
 * Output: AL = character from stdin
 */
static void F07() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x544b);
    _544B_readFromStdin();
    _ret(asmFuncEnd());
}
/**
 * INT 21,8 - Console Input Without Echo
 * Input: AH = 08
 * Output: AL = character from stdin
 */
static void F08() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x51ba, 0x41f9);
    PUSH(GetRef(DS), 2);
    PUSH(GetRef(SI), 2);

    CODE_STARTS_AT(0x0019, 0x51bc);
    CALL_NEAR(0x9137, 0x51bf); /* check stdin ready */
    while (_GetEFLAGS_ZF) {
        /* stdin not ready */
        if (_BYTE(SS, 0x0aa0) == 0x00) {
            CODE_STARTS_AT(0x0019, 0x51c1);
            /* do stdin status check */
            AH = 0x05;
            CALL_NEAR(0x8413, 0x51ce); /* io command peek */
        }
        /* keyboard busy loop */
        AH = 0x84;
        INT(0x2a, 0x51d2);
        /* get date and time */
        if (_BYTE(SS, 0x0d91) == 0xff) {
            PUSH(GetRef(AX), 2);
            PUSH(GetRef(BX), 2);
            PUSH(GetRef(CX), 2);
            PUSH(GetRef(DX), 2);
            PUSH(GetRef(DS), 2);
            LOAD_DS(SS);
            AX = 0x0000;
            CALL_NEAR(0x54b9, 0x51e7); /* copy io request header */
            CALL_NEAR(0x48f8, 0x51ea); /* read device */
            AX = 0x0001;
            CALL_NEAR(0x54b9, 0x51f0); /* copy device driver result to some table */
            POP(GetRef(DS), 2);
            POP(GetRef(DX), 2);
            POP(GetRef(CX), 2);
            POP(GetRef(BX), 2);
            POP(GetRef(AX), 2);
        }
        CODE_STARTS_AT(0x0019, 0x51f5);
        _WORD(SS, 0x0d91) += 0x0001; /* increase counter */
        CALL_NEAR(0x9137, 0x51bf); /* check stdin ready */
    };
    /* ready, get 1 char */
    CODE_STARTS_AT(0x0019, 0x51fc);
    AH = 0x00;
    CALL_NEAR(0x8413, 0x5201); /* get 1 cahr from stdin */

    POP(GetRef(SI), 2);
    POP(GetRef(DS), 2);
    LOAD_DS(DS);

    if (AL == 0x00) {
        /* is control char */
        _BYTE(SS, 0x0d90) = 0x01; /* set control char flag */
    } else {
        _BYTE(SS, 0x0d90) = 0x00; /* clear control char flag */
    }
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,9 - Print String
 * Input: AH = 09, DS:DX = pointer to string ending in "$"
 * Output: (null)
 */
static void F09() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x5214);
    SI = DX; /* DS:SI points to the starting address of string */
    AL = _BYTE(DS, SI); /* load char */
    SI += 0x0001;
    while (AL != '$') {
        _54EB_printChar(); /* print char */
        AL = _BYTE(DS, SI); /* load char */
        SI += 0x0001;
    }
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,A - Buffered Keyboard Input
 * Input: AH = 0A, DS:DX = pointer to input buffer of the format:
 *  | max | count |  BUFFER (N bytes)
 *     |      |      `------ input buffer
 *     |      `------------ number of characters returned (byte)
 *     `-------------- maximum number of characters to read (byte)
 * Output: (null)
 */
static void F0A() {
    t_nubit16 t = 0xffff;
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0x5220);
    LOAD_ES(SS); /* ES:DI points to kernel keyboard buffer (diff w/ kb buf, temporary) */
    SI = DX; /* DS:DX points to input buffer (user buff, destination) */
    CH = 0x00; /* clear counter */
    AX = _WORD(DS, SI); /* read first input buffer word */
    SI += 0x02;
    /* buffer size is not 0 */
    if (AL != 0x00) {
        CODE_STARTS_AT(0x0019, 0x522d);
        BL = AH; /* maximum char number allowed */
        BH = 0x00; /* count of char number in input buff */
        /* buff sz <= max char num, or max char num doesnt include ENTER char */
        if (AL <= BL || _BYTE(DS, BX + SI) != 0x0d) {
            CODE_STARTS_AT(0x0019, 0x523a);
            BL = 0x00; /* maximum char number limit doesnt make sense */
        }
        CODE_STARTS_AT(0x0019, 0x523c);
        DL = AL - 0x01; /* get buffer size and exclude ENTER */
        CODE_STARTS_AT(0x0019, 0x523f);
        AL = _BYTE(SS, 0x01f9); /* get column position */
        _BYTE(SS, 0x01fa) = AL; /* save current position */
        PUSH(GetRef(SI), 2); /* save offset of first char in target buff */
        DI = 0x01fb; /* get starting address of temp buffer (keyb buff) */
        _BYTE(SS, 0x0579) = 0x00; /* clear INSERT flag */
        BH = DH = 0x00; /* clear char counter/ptr */
        CALL_NEAR(0x51ba, 0x5254); /* read one char to AL */
        /* char is LF */
        if (AL == 0x0a) {
            CALL_NEAR(0x51ba, 0x525e); /* get new char */
        }
        do {
            CODE_STARTS_AT(0x0019, 0x525e);
            if (AL == 0x17 || AL == 0x15) {
                /* char is ^W or ^U */
                /* do nothing: nop */
            } else if (AL == 0x06) {
                /* char is ^F */
                CALL_NEAR(0x51ba, 0x525e); /* get new char */
                continue; /* loop back */
            } else if (AL == _BYTE(CS, 0x47f4)) {
                /* char is ESC */
                AL = '\\';
                CALL_NEAR(0x54eb, 0x52f8); /* output char '\' */
                POP(GetRef(SI), 2); /* clear stack */
                CALL_NEAR(0x5412, 0x52fc); /* print CRLF */
                AL = _BYTE(SS, 0x01fa); /* get number of space */
                CALL_NEAR(0x555f, 0x5303); /* print spaces */
                CODE_STARTS_AT(0x0019, 0x523f);
                AL = _BYTE(SS, 0x01f9); /* get column position */
                _BYTE(SS, 0x01fa) = AL; /* save current position */
                PUSH(GetRef(SI), 2); /* save offset of first char in target buff */
                DI = 0x01fb; /* get starting address of temp buffer (keyb buff) */
                _BYTE(SS, 0x0579) = 0x00; /* clear INSERT flag */
                BH = DH = 0x00; /* clear char counter/ptr */
                CALL_NEAR(0x51ba, 0x5254); /* read one char to AL */
                /* char is LF */
                if (AL == 0x0a) {
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                }
                continue; /* loop back */
            } else if (AL == 0x7f || AL == 0x08) {
                /* char is DEL or BACKSPACE */
                CODE_STARTS_AT(0x0019, 0x5306);
                CALL_NEAR(0x530c, 0x5309); /* execute delete char */
                CALL_NEAR(0x51ba, 0x525e); /* get new char */
                continue;
            } else if (AL == 0x0a) {
                /* char is line feed */
                CODE_STARTS_AT(0x0019, 0x52c5);
                CALL_NEAR(0x0019, 0x5412); /* print CRLF */
                CALL_NEAR(0x51ba, 0x525e); /* get new char */
                continue; /* loop back */
            } else if (AL == 0x0d) {
                /* char is carriage return */
                t_nubitcc i;
                t_nubit16 tempDS, tempES;
                CODE_STARTS_AT(0x0019, 0x52af);
                _BYTE(ES, DI) = AL; /* save char to temp buffer */
                CALL_NEAR(0x54eb, 0x52b3);
                _BYTE(DS, DS - 0x01) = DH; /* save actual char number to target buff */
                DH += 0x01; /* include ENTER char */
                /* DS:SI -> temp buff, ES:DI -> target buff */
                POP(GetRef(DI), 2); /* get offset of first char in target buff */
                tempDS = DS;
                tempES = ES;
                LOAD_DS(tempES);
                LOAD_ES(tempDS);
                SI = 0x01fb;
                /* copy data from temp buff to target buff */
                for (i = 0; i < DH; ++i) {
                    _BYTE(ES, DI) = _BYTE(DS, SI);
                    DI += 0x0001;
                    SI += 0x0001;
                }
                CL = 0x00;
                break;
            } else if (AL == _BYTE(CS, 0x47f5)) {
                /* char is extended ascii */
                CODE_STARTS_AT(0x0019, 0x4820);
                CALL_NEAR(0x51ba, 0x4823); /* read char to AL */
                /*
                 TABLE 0x47f6 + (0x0000, 0x000e);
                 41 525b    F7  goto F0A
                 41 537d *  F7  send NUL to temp buff and print
                 52 5405    INS change INSERT flag
                 52 5405 *  INS change INSERT flag
                 4B 5306 *  <-  same as DEL
                 3F 53f1 *  F5  print '@', copy temp buff to user buff, jmp to next line for edit
                 3D 52f3    F3  same as ESC for second F3
                 3D 5384 *  F3  copy chars from curr ptr in user buff to temp buff
                 3E 53b7 *  F4  starting from curr ptr, search in user buff for specific char
                 3C 538a *  F2  copy chars from curr ptr to specific char in user buff to temp buff
                 53 53ad *  DEL user buff ptr skip current char, cursor moves down
                 3B 538f *  F1  copy current 1 char in user buff to temp buff
                 4D 538f *  ->  same as F1
                 40 540d *  F6  send 1A (^Z) to AL
                 */
                if (AL == 0x41) {
                    /* F7 */
                    CODE_STARTS_AT(0x0019, 0x537d);
                    AL = _BYTE(CS, 0x47f5); /* convert F7 to NUL */
                    CODE_STARTS_AT(0x0019, 0x5288);
                } else if (AL == 0x40) {
                    /* F6 */
                    CODE_STARTS_AT(0x0019, 0x540d);
                    AL = 0x1a;
                    CODE_STARTS_AT(0x0019, 0x5288);
                } else if (AL == 0x52) {
                    /* INSERT */
                    CODE_STARTS_AT(0x0019, 0x5405);
                    _BYTE(SS, 0x0579) = ~_BYTE(SS, 0x0579); /* revert INSERT flag */
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    continue; /* loop back */
                } else if (AL == 0x4b) {
                    /* <- */
                    CODE_STARTS_AT(0x0019, 0x5306);
                    CALL_NEAR(0x530c, 0x5309);
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    continue; /* loop back */
                } else if (AL == 0x53) {
                    /* extended DEL */
                    CODE_STARTS_AT(0x0019, 0x53ad);
                    if (BH != BL) {
                        /* user buff is not full */
                        BH += 0x01;
                        SI += 0x0001;
                    }
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    continue; /* loop back */
                } else if (AL == 0x3f) {
                    /* F5 */
                    t_nubit16 saveES, saveDS;
                    CODE_STARTS_AT(0x0019, 0x53f1);
                    AL = '@';
                    CALL_NEAR(0x54eb, 0x53f6); /* output @ */
                    POP(GetRef(DI), 2); /* load DI with SI, starting position  */
                    PUSH(GetRef(DI), 2);
                    saveES = ES;
                    saveDS = DS;
                    CALL_NEAR(0x52b9, 0x53fd); /* copy temp buff to user buff */
                    LOAD_DS(saveDS);
                    LOAD_ES(saveES);
                    POP(GetRef(SI), 2);
                    CALL_NEAR(0x5412, 0x52fc); /* print CRLF */
                    AL = _BYTE(SS, 0x01fa); /* get number of space */
                    CALL_NEAR(0x555f, 0x5303); /* print spaces */
                    CODE_STARTS_AT(0x0019, 0x523f);
                    AL = _BYTE(SS, 0x01f9); /* get column position */
                    _BYTE(SS, 0x01fa) = AL; /* save current position */
                    PUSH(GetRef(SI), 2); /* save offset of first char in target buff */
                    DI = 0x01fb; /* get starting address of temp buffer (keyb buff) */
                    _BYTE(SS, 0x0579) = 0x00; /* clear INSERT flag */
                    BH = DH = 0x00; /* clear char counter/ptr */
                    CALL_NEAR(0x51ba, 0x5254); /* read one char to AL */
                    /* char is LF */
                    if (AL == 0x0a) {
                        CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    }
                    continue; /* loop back */
                } else if (AL == 0x3d || AL == 0x3c || AL == 0x3b || AL == 0x4d) {
                    /* F3 or F2 or F1 or -> */
                    /* copy CL chars from user buff to temp buff */
                    if (AL == 0x3d) {
                        /* F3 */
                        CODE_STARTS_AT(0x0019, 0x5384);
                        CL = BL - BH; /* copy remaining chars, usr bf -> tmp bf */
                    } else if (AL == 0x3c) {
                        /* F2 */
                        t_nubit16 saveES, saveDI;
                        CODE_STARTS_AT(0x0019, 0x538a);
                        CODE_STARTS_AT(0x0019, 0x53c1);
                        CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                        if (AL == _BYTE(CS, 0x47f5)) {
                            /* is control char */
                            CALL_NEAR(0x51ba, 0x53c4); /* get one more char */
                            CALL_NEAR(0x51ba, 0x53c4); /* get even more char */
                            continue; /* loop back */
                        }
                        CL = BL - BH; /* get range to search */
                        if (CL < 0x02) {
                            CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                            continue; /* loop back */
                        }
                        CL -= 0x01;
                        saveES = ES;
                        saveDI = DI;
                        LOAD_ES(DS);
                        DI = SI + 0x0001;
                        while (CL) {
                            CL -= 0x01;
                            if (_BYTE(ES, DI) == AL) {
                                break;
                            }
                            DI += 0x0001;
                        }
                        DI = saveDI;
                        LOAD_ES(saveES);
                        if (!_GetEFLAGS_ZF) {
                            CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                            continue; /* loop back */
                        }
                        CL = BL - BH - CL;
                    } else {
                        /* F1 or -> */
                        CODE_STARTS_AT(0x0019, 0x538f);
                        CL = 0x01; /* copy 1 char, usr bf -> tmp bf */
                    }
                    /* copy CL chars from user buff to temp buff */
                    CODE_STARTS_AT(0x0019, 0x5391);
                    do {
                        _BYTE(SS, 0x0579) = 0x00; /* clear INSERT flag */
                        if (DH != DL && BH != BL) {
                            /* neither temp buf nor user buf is full */
                            /* copy char from user buff to temp buff */
                            AL = _BYTE(DS, SI);
                            SI += 0x0001;
                            _BYTE(ES, DI) = AL;
                            DI += 0x0001;
                            CALL_NEAR(0x5582, 0x53a4); /* print char */
                            BH += 0x01;
                            DH += 0x01;
                        } else {
                            break;
                        }
                        CL -= 0x01;
                    } while (CL != 0x00);
                    CODE_STARTS_AT(0x0019, 0x53aa);
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    continue; /* loop back */
                } else if (AL == 0x3e) {
                    /* F4 */
                    t_nubit16 saveES, saveDI;
                    CODE_STARTS_AT(0x0019, 0x53b7);
                    CODE_STARTS_AT(0x0019, 0x53c1);
                    CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                    if (AL == _BYTE(CS, 0x47f5)) {
                        /* is control char */
                        CALL_NEAR(0x51ba, 0x53c4); /* get one more char */
                        CALL_NEAR(0x51ba, 0x53c4); /* get even more char */
                        continue; /* loop back */
                    }
                    CL = BL - BH; /* get range to search */
                    if (CL < 0x02) {
                        CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                        continue; /* loop back */
                    }
                    CL -= 0x01;
                    saveES = ES;
                    saveDI = DI;
                    LOAD_ES(DS);
                    DI = SI + 0x0001;
                    while (CL) {
                        CL -= 0x01;
                        if (_BYTE(ES, DI) == AL) {
                            break;
                        }
                        DI += 0x0001;
                    }
                    DI = saveDI;
                    LOAD_ES(saveES);
                    if (!_GetEFLAGS_ZF) {
                        CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                        continue; /* loop back */
                    }
                    CL = BL - BH - CL;
                    CODE_STARTS_AT(0x0019, 0x53ba);
                    SI += CX; /* ? */
                    BH += CL; /* ? */
                    CALL_NEAR(0x51ba, 0x53c4); /* get one char */
                    continue; /* loop back */
                } else {
                    /* all others */
                    CALL_NEAR(0x51ba, 0x525e); /* get new char */
                    continue; /* loop back */
                }
            }
            CODE_STARTS_AT(0x0019, 0x5288);
            /* BH = char num in user buff, BL = max char num allowed */
            /* DH = char num in temp buff, DL = user buff size */
            if (DH >= DL) {
                /* temp buffer is full */
                CODE_STARTS_AT(0x0019, 0x52a5);
                AL = 0x07;
                CALL_NEAR(0x54eb, 0x52aa); /* ring the bell */
            } else {
                CODE_STARTS_AT(0x0019, 0x528c);
                _BYTE(ES, DI) = AL; /* send char to temp buffer */
                DI += 0x0001; /* increase temp buffer pointer */
                DH += 0x01; /* increase counter */
                CALL_NEAR(0x5582, 0x5292); /* output the char */
                if (_BYTE(SS, 0x0579) == 0x00 && BH < BL) {
                    /* INSERT flag is clear and not reach end of buff */
                    SI += 0x0001; /* increase buffer pointer */
                    BH += 0x01; /* increase char counter  */
                }
            }
            CALL_NEAR(0x51ba, 0x525e); /* get new char */
            /* loop back */
        } while (1);
    }
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,B - Check Standard Input Status
 * Input: AH = 0B
 * Output: AL = 00: no char available from stdin; vise versa.
 */
static void F0B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x55d6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,C - Clear Keyboard Buffer and Invoke Keyboard Function
 * Input: AH = 0C, AL = sub-function id
 * Output: see different sub-functions
 */
static void F0C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x55e0, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,D - Disk Reset, flush all file bufs to disk w/o updating dir entry
 * Input: AH = 0D
 * Output: (null)
 */
static void F0D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4da1, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,E - Select Disk
 * Input: AH = 0E, DL = drive number 0 based (0-25, A:-Z:)
 * Output: AL = total num of logical drive (1-26)
 */
static void F0E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c78, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,F - Open a File Using FCB
 * Input: AH = 0F, DS:DX = pointer to unopened FCB
 * Output: AL = 00 if file opened, or FF if unable to open
 */
static void F0F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5ccc, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,10 - Close a File Using FCB
 * Input: AH = 10, DS:DX = pointer to opened FCB
 * Output: AL = 00 if file closed, or FF if unable to close
 */
static void F10() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5688, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,11 - Search for First Entry Using FCB
 * Input: AH = 11, DS:DX = pointer to unopened FCB
 * Output: AL = 00 if maching file found, or FF if file not found
 */
static void F11() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5ddf, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,12 - Search for Next Entry Using FCB
 * Input: AH = 12, DS:DX = pointer to unopened FCB returned from F11 or F12
 * Output: AL = 00 if maching file found, or FF if file not found
 */
static void F12() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5e73, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,13 - Delete File Using FCB
 * Input: AH = 13, DS:DX = pointer to an unopened FCB
 * Output: AL = 00 if file deleted, or FF if file not found
 */
static void F13() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5625, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,14 - Sequential Read Using FCB
 * Input: AH = 14, DS:DX = pointer to an opened FCB
 * Output: AL = 00 if successful read
 *            = 01 if end of file (no data read)
 *            = 02 if DTA is too small
 *            = 03 if end of file or partial record read
 */
static void F14() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dcb, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,15 - Sequential Write Using FCB
 * Input: AH = 15, DS:DX = pointer to an opened FCB
 * Output: AL = 00 if write was successful
 *            = 01 if diskette is full or read only
 *            = 02 if DTA is too small
 */
static void F15() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dd0, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,16 - Create a File Using FCB
 * Input: AH = 16, DS:DX = pointer to an unopened FCB
 * Output: AL = 00 if file created, FF if file creation failed
 */
static void F16() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5db1, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,17 - Rename a File Using FCB
 * Input: AH = 17, DS:DX = pointer to a modified FCB of the format:
 *     Offset     Description
 *      00	   drive designator
 *      01	   original file name
 *      09	   original file extension
 *      11	   new file name
 *      19	   new extension
 * Output: 00 if file renamed, FF if file not renamed
 */
static void F17() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x56f9, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,18 - ?
 * Input: AH = 18, ?
 * Output: ?
 */
static void F18() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,19 - Get Current Default Drive
 * Input: AH = 19
 * Output: AL = current default drive (0=A,1=B,etc)
 */
static void F19() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c73, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1A - Set Disk Transfer Address (DTA)
 * Input: AH = 1A, DS:DX = pointer to disk transfer address (DTA)
 * Output: (null)
 */
static void F1A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c68, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1B - Get Allocation Table Information
 * Input: AH = 1B
 * Output: AL = sectors per cluster
 *         CX = bytes per sector
 *         DX = clusters on disk
 *         DS:BX = pointer to Media Descriptor Byte found in FAT
 */
static void F1B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d2d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1C - Get Allocation Table Info for Specified Drive
 * Input: AH = 1C, DL = drive number (0 for default, 1 = A:, Z: = 26)
 * Output: AL = sectors per cluster
 *         CX = bytes per sector
 *         DX = clusters on disk
 *         DS:BX = pointer to Media Descriptor Byte found in FAT
 */
static void F1C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d2f, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1D - ?
 * Input: AH = 1D, ?
 * Output: ?
 */
static void F1D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1E - ?
 * Input: AH = 1E, ?
 * Output: ?
 */
static void F1E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,1F - Get Pointer to Current Drive Parameter Table
 * Input: AH = 1F, DL = drive number (0=default, 1=A, ...)
 * Output: AL = 00  DS:BX is pointer to drive parameter table  (DPT)
 *              FF  drive does not exist
 *         DS:BX = pointer to drive parameter table (DPT) if AL=0
 */
static void F1F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d71, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,20 - ?
 * Input: AH = 20, ?
 * Output: ?
 */
static void F20() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,21 - Random Read Using FCB
 * Input: AH = 21, DS:DX = pointer to an opened FCB
 * Output: AL = 00 if read successful
 *            = 01 if EOF (no data read)
 *            = 02 if DTA is too small
 *            = 03 if EOF (partial record read)
 */
static void F21() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dd5, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,22 - Random Write Using FCB
 * Input: AH = 22, DS:DX = far pointer to an opened FCB
 * Output: AL = 00 if write successful
 *            = 01 if diskette full or read only
 *            = 02 if DTA is too small
 */
static void F22() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dda, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,23 - Get File Size Using FCB
 * Input: AH = 23, DS:DX = pointer to an unopened FCB
 * Output: AL = 00 if successful, FF if file not found
 */
static void F23() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5639, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,24 - Set Relative Record Field in FCB
 * Input: AH = 24, DS:DX = pointer to an opened FCB
 * Output: (null)
 */
static void F24() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x560d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,25 - Set Interrupt Vector
 * Input: AH = 25, AL = interrupt number, DS:DX = pointer to interrupt handler
 * Output: (null)
 */
static void F25() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c9a, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,26 - Create New Program Segment Prefix
 * Input: AH = 26, DX = segment address of new PSP
 * Output: (null)
 */
static void F26() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4eb6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,27 - Random Block Read Using FCB
 * Input: AH = 27, CX = number of records to read
 *        DS:DX = pointer to an opened FCB
 * Output: AL = 00 if read was successful
 *            = 01 if EOF (no data read)
 *            = 02 if DTA is too small
 *            = 03 if EOF (partial record read)
 *         CX = actual number of records read
 */
static void F27() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dc6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,28 - Random Block Write Using FCB
 * Input: AH = 28, CX = number of records to write
 *        DS:DX = pointer to an opened FCB
 * Output: AL = 00 if write successful
 *            = 01 if diskette full or read only
 *            = 02 if DTA is too small
 *         CX = number of records written
 */
static void F28() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5dc1, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,29 - Parse a Filename for FCB
 * Input: AH = 29
 *        DS:SI = pointer to a filespec to parse
 *        ES:DI = pointer to a buffer for unopened FCB
 *        AL = bit pattern to control parsing (see bit meanings below)
 *  Bit patterns for parsing control found in AL:
	|7|6|5|4|3|2|1|0|  AL
	 | | | | | | | `---- 1 = ignore leading separators
	 | | | | | | |       0 = don't ignore leading separators
	 | | | | | | `----- 1 = modify drive ID if specified
	 | | | | | |        0 = modify drive ID regardless
	 | | | | | `------ 1 = modify filename if specified
	 | | | | |         0 = modify filename regardless
	 | | | | `------- 1 = modify extension if specified
	 | | | |          0 = modify extension regardless
	 `-------------- unused
 * Output: AL = 00 if no wildcard characters present
 *            = 01 if wildcards present in string
 *            = FF if drive specifier is invalid
 *         DS:SI = pointer to the first character after parsed filename
 *         ES:DI = pointer to the updated unopened FCB
 */
static void F29() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d22, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2A - Get Date
 * Input: AH = 2A
 * Output: AL = day of the week (0=Sunday)
 *         CX = year (1980-2099)
 *         DH = month (1-12)
 *         DL = day (1-31)
 */
static void F2A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4839, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2B - Set Date
 * Input: AH = 2B
 * CX = year (1980-2099)
 * DH = month (1-12)
 * DL = day (1-31)
 * Output: AL = 00 if date change successful, FF if invalid date
 */
static void F2B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4856, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2C - Get Time
 * Input: AH = 2C
 * Output: CH = hour (0-23)
 *         CL = minutes (0-59)
 *         DH = seconds (0-59)
 *         DL = hundredths (0-99)
 */
static void F2C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4876, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2D - Set Time
 * Input: AH = 2D
 *        CH = hour (0-23)
 *        CL = minutes (0-59)
 *        DH = seconds (0-59)
 *        DL = hundredths (0-99)
 * Output: AL = 00 if time change successful, FF if time invalid
 */
static void F2D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4887, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2E - Set/Reset Verify Switch
 * Input: AH = 2E
 *        AL = 00 to set off
 *           = 01 to set verify on
 *        DH = 00 for DOS versions before 3.0
 * Output: (null)
 */
static void F2E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4a46, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,2F - Get Disk Transfer Address (DTA)
 * Input: AH = 2F
 * Output: ES:BX = pointer to current DTA
 */
static void F2F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c54, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,30 - Get DOS Version Number
 * Input: AH = 30
 * Output: AL = major version number (2-5)
 *         AH = minor version number (in hundredths decimal)
 *         BH = FF  indicates MS-DOS
 *            = 00  indicates PC-DOS
 *         BL:CX = 24 bit OEM serial number if BH is FF
 */
static void F30() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4a1c, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,31 - Terminate Process and Remain Resident
 * Input: AH = 31h
 *        AL = exit code (returned to batch files)
 *        DX = memory size in paragraphs to reserve
 * Output: (null)
 */
static void F31() {
    t_nubit16 pspSeg = kw(KDATA_ADDR_PSP);

    _ret(asmFuncBegin());

    CODE_STARTS_AT(0x0019, 0xa19b);
    kb(KDATA_FLAG_TERMINATE) = 0x03; /* terminate type 03 */

    /* memory to reserve: > 60b? */
    if (DX < 0x06) {
        DX = 0x06;
    }

    /* call function 4a, reserve memory */
    BX = DX;
    LOAD_ES(pspSeg);
    PUSH(GetRef(BX), 2);
    PUSH(GetRef(ES), 2);
    CALL_NEAR(0xa3ef, 0xa1b6);
    POP(GetRef(DS), 2); /* high memory segment address */
    POP(GetRef(BX), 2); /* size of memory reserved */

    /* failed to reserve memory */
    if (_GetEFLAGS_CF) {
        JMP_NEAR(0xa1c1); /* error processor */
        return;
    }
    _WORD(DS, 0x0002) = DS + BX; /* current highest available address */

    _ret(execTerminate_A1FF());
}
/** TODO
 * INT 21,32 - Get Pointer to Drive Parameter Table
 * Input: AH = 32, DL = drive (0=default, 1=A:, 2=B:, 3=C:, ...)
 * Output: AL =  FF if the drive number in DL was invalid
 *         DS:BX = address of drive parameter table (DPT)
 */
static void F32() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d73, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,33 - Get/Set System Values (Ctl-Break/Boot Drive)
 * Input: AH = 33h
 *        AL = 00 to get Ctrl-Break checking flag
 *           = 01 to set Ctrl-Break checking flag
 *           = 02 to set extended Ctrl-Break checking
 *           = 05 get boot drive (DOS 4.x)
 *        DL = 00 to set Ctrl-Break checking off
 *           = 01 to set Ctrl-Break checking on
 *           = boot drive for subfunction 5;  (1=A:, 2=B:, ...)
 * Output: DL = 00 Ctrl-Break checking OFF (AL=0 or AL=2)
 *            = 01 Ctrl-Break checking ON	(AL=0 or AL=2)
 *            = boot drive number (1-26, A: - Z:) (function 05)
 */
static void F33() {
    t_nubit8 temp8;
    CODE_STARTS_AT(0x0019, 0x4052);
    switch (AL) {
    case 0x00:
        /* get ctrl-break status */
        DL = kb(0x0337);
        break;
    case 0x01:
        /* set ctrl-break status according to DL */
        kb(0x0337) = DL & 0x01;
        break;
    case 0x02:
        /* set ctrl-break status from DL, and return original status to DL */
        temp8 = DL & 0x01;
        DL = kb(0x0337);
        kb(0x0337) = temp8;
        break;
    case 0x03:
        /* ? get something unknown */
        DL = kb(0x0069);
        break;
    case 0x04:
        /* ? do something unknown */
        BX = 0x1606;
        DX = 0x0000;
        if (kb(0x1211) != 0x00) {
            DH |= 0x10;
        }
        break;
    default:
        /* invalid sub function */
        if (AL > 0x06) {
            AL = 0xff;
        }
        break;
    }
    IRET();
}
/** TODO
 * INT 21,34 - Get Address to DOS Critical Flag
 * Input: AH = 34
 * Output: ES:BX = address of a byte indicating whether a DOS call is
 *                 in progress.  No DOS calls should be made if set.
 */
static void F34() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d59, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,35 - Get Interrupt Vector
 * Input: AH = 35h, AL = interrupt vector number
 * Output: ES:BX = pointer to interrupt handler
 */
static void F35() {
    _ret(asmFuncBegin());
    _4C8A_getInterruptVector();
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,36 - Get Disk Free Space
 * Input: AH = 36, DL = drive number (0=default, 1=A:)
 * Output: AX = sectors per cluster
 *            = FFFF if drive is invalid
 *         BX = number of available clusters
 *         CX = number of bytes per sector
 *         DX = number of clusters per drive
 */
static void F36() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4c2b, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,37 - Get/Set Switch Character
 * Input: AH = 37
 *        AL = 0 get switch character into DL;  some systems return "-"
 *           = 1 set switch character to value in DL
 *           = 2 read device prefix flag into DL;  returns DL = 0 indicating
 *               devices must be accessed using /DEV/device. A non-zero value
 *               indicates devices may be accessed without prefix
 *           = 3 set device prefix flag, device names must begin with \DEV\.
 *        DL = new switch character (AL=1)
 *           = 00 \DEV\ must preceed device names (AL=3)
 *           = 01 \DEV\ is not neccesary in device names (AL=3)
 * Output: AL = FF illegal subfunction code specified
 *         DL = current switch character (AL=0)
 *            = device availability (AL=2, always FF with DOS 4.x+)
 */
static void F37() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4cc9, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,38 - Get/Set Country Dependent Information
 * Input: AH = 38
 *        AL = 00 to get current country information
 *           = 00-FE country codes  (DOS 3.x+)
 *           = FF for country codes >= FF, country codes are in BX
 *        BX = country code if AL = FF (country code > 255)
 *        DX = FFFF to set country information
 *        DS:DX = pointer to buffer to contain country data (if get data)
 * Output: AX = error code if CF set
 *            = 02 invalid country
 *         BX = country code (DOS 3.x+)
 *         DS:DX = pointer to returned country data (see COUNTRY CODES)
 */
static void F38() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4a4d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,39 - Create Subdirectory (mkdir)
 * Input: AH = 39, DS:DX = pointer to ASCIIZ path name
 * Output: CF = 0 if successful, 1 if error
 *         AX = error code  (see DOS ERROR CODES)
 */
static void F39() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x60e1, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3A - Remove Subdirectory (rmdir)
 * Input: AH = 3A, DS:DX = pointer to ASCIIZ path name
 * Output: CF = 0 if successful, 1 if error
 *         AX = error code  (see DOS ERROR CODES)
 */
static void F3A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x6029, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3B - Change Current Directory (chdir)
 * Input: AH = 3B, DS:DX = pointer to ASCIIZ path name
 * Output: CF = 0 if successful, 1 if error
 *         AX = error code  (see DOS ERROR CODES)
 */
static void F3B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x6065, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3C - Create File Using Handle
 * Input: AH = 3C, CX = file attribute  (see FILE ATTRIBUTES)
 *        DS:DX = pointer to ASCIIZ path name
 * Output: CF = 0 if successful, 1 if error
 *         AX = files handle if successful
 *            = error code if failure (see DOS ERROR CODES)
 */
static void F3C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xafe7, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3D - Open File Using Handle
 * Input: AH = 3D
 *        AL = open access mode
 *             00  read only
 *             01  write only
 *             02  read/write
 *        DS:DX = pointer to an ASCIIZ file name
 * Output: AX = file handle if CF not set
 *            = error code if CF set  (see DOS ERROR CODES)
 */
static void F3D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xaf10, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3E - Close File Using Handle
 * Input: AH = 3E, BX = file handle to close
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F3E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa72b, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,3F - Read From File or Device Using Handle
 * Input: AH = 3F
 *        BX = file handle
 *        CX = number of bytes to read
 *        DS:DX = pointer to read buffer
 * Output: AX = number of bytes read is CF not set
 *            = error code if CF set  (see DOS ERROR CODES)
 */
static void F3F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa83a, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,40 - Write To File or Device Using Handle
 * Input: AH = 40
 *        BX = file handle
 *        CX = number of bytes to write, a zero value truncates/extends
 *             the file to the current file position
 *        DS:DX = pointer to write buffer
 * Output: AX = number of bytes written if CF not set
 *            = error code if CF set  (see DOS ERROR CODES)
 */
static void F40() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa8a0, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,41 - Delete File
 * Input: AH = 41, DS:DX = pointer to an ASCIIZ filename
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F41() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb039, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,42 - Move File Pointer Using Handle
 * Input: AH = 42
 *        AL = origin of move:
 *             00 = beginning of file plus offset  (SEEK_SET)
 *             01 = current location plus offset	(SEEK_CUR)
 *             02 = end of file plus offset  (SEEK_END)
 *        BX = file handle
 *        CX = high order word of number of bytes to move
 *        DX = low order word of number of bytes to move
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 *         DX:AX = new pointer location if CF not set
 */
static void F42() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa8a5, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,43 - Get/Set File Attributes
 * Input: AH = 43
 *        AL = 00 to get attribute
 *           = 01 to set attribute
 *        DS:DX = pointer to an ASCIIZ path name
 *        CX = attribute to set
 *  |5|4|3|2|1|0|  CX  valid file attributes
 *   | | | | | `---- 1 = read only
 *   | | | | `----- 1 = hidden
 *   | | | `------ 1 = system
 *   | `--------- not used for this call
 *   `---------- 1 = archive
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 *         CX = the attribute if AL was 00
 */
static void F43() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xaff4, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,44 - I/O Control for Devices (IOCTL)
 * Input: AH = 44
 *        AL = function value
 *        BX = file handle
 *        BL = logical device number (0=default, 1=A:, 2=B:, 3=C:, ...)
 *        CX = number of bytes to read or write
 *        DS:DX = data or buffer
 * Output: AX = error code if CF set
 *         AX = # of bytes transferred if CF not set
 */
static void F44() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x6142, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,45 - Duplicate File Handle
 * Input: AH = 45, BX = file handle
 * Output: AX = new file handle if CF not set
 *            = error code if CF set  (see DOS ERROR CODES)
 */
static void F45() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa95b, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,46 - Force Duplicate File Handle
 * Input: AH = 46
 *        BX = existing file handle
 *        CX = second file handle
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F46() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa97b, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,47 - Get Current Directory
 * Input: AH = 47
 *        DL = drive number (0 = default, 1 = A:)
 *        DS:SI = pointer to a 64 byte user buffer
 * Output: DS:SI = pointer ASCIIZ directory path string
 *         AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F47() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5fb0, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,48 - Allocate Memory
 * Input: AH = 48, BX = number of memory paragraphs requested
 * Output: AX = segment address of allocated memory block (MCB + 1para)
 *            = error code if CF set  (see DOS ERROR CODES)
 *         BX = size in paras of the largest block of memory available
 *              if CF set, and AX = 08 (Not Enough Mem)
 *         CF = 0 if successful, 1 if error
 */
static void F48() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa28e, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,49 - Free Allocated Memory
 * Input: AH = 49, ES = segment of the block to be returned (MCB + 1para)
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F49() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa413, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,4A - Modify Allocated Memory Block (SETBLOCK)
 * Input: AH = 4A
 *        BX = new requested block size in paragraphs
 *        ES = segment of the block (MCB + 1 para)
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 *         BX = maximum block size possible, if CF set and AX = 8
 */
static void F4A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa3ef, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,4B - EXEC/Load and Execute Program
 * Input: AH = 4B
 *        AL = 00  to load and execute program
 *           = 01  (Undocumented)  create program segment prefix and load
 *                 program, but don't execute.  The CS:IP and SS:SP of the
 *                 program is placed in parameter block. Used by debuggers.
 *           = 03  load program only
 *           = 04  called by MSC spawn() when P_NOWAIT is specified
 *        DS:DX = pointer to an ASCIIZ filename
 *        ES:BX = pointer to a parameter block
 * Output: AX = error code if CF set (see DOS ERROR CODES)
 *         ES:BX = when AL=1, pointer to parameter block similar to:
 *   Offset Sise  Description
 *     00   word   when AL=1, segment of env. or zero if using parents env.
 *          word   when AL=3, segment of load point for overlay
 *     02   dword  when AL=1, pointer to cmd line at PSP 80h
 *          word   when AL=3, relocation factor for EXE overlay
 *     06   dword  pointer to default FCB passed at PSP 5Ch
 *     0A   dword  pointer to default FCB passes at PSP 6Ch
 *     0E   dword  value of program SS:SP
 *     12   dword  value of program CS:IP
 */
static void F4B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x9b5f, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,4C - Terminate Process With Return Code
 * Input: AH = 4C, AL = return code (for batch files)
 * Output: (null)
 */
static void F4C() {
    _ret(asmFuncBegin());
    CODE_STARTS_AT(0x0019, 0xa1d3);

    /* fetch and clear ctrl-break flag */
    AH = kb(0x034d);
    kb(0x034d) = 0x00;
    kb(KDATA_FLAG_TERMINATE) = !!AH;
    _4282_getCurrentStackPointer();
    _WORD(DS, SI + 0x14) = kw(KDATA_ADDR_PSP);
    _ret(execTerminate_A1FF());
}
/** TODO
 * INT 21,4D - Get Return Code of Sub-process
 * Input: AH = 4D
 * Output: AH = system exit code  (indicates normal termination)
 *            = 00 for normal termination
 *            = 01 if terminated by ctl-break
 *            = 02 if critical device error
 *            = 03 if terminated by INT 21,31
 *         AL = child exit code
 */
static void F4D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x9b55, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,4E - Find First Matching File
 * Input: AH = 4E
 *        CX = attribute used during search  (see FILE ATTRIBUTES)
 *        DS:DX = pointer to ASCIIZ filespec, including wildcards
 * Output: AX = error code if CF set  (see DOS ERROR CODE)
 *         DTA = data returned from call in the format:
 *         http://stanislavs.org/helppc/int_21-4e.html
 */
static void F4E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5ee6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,4F - Find Next Matching File
 * Input: AH = 4F, DS:DX = unchanged from previous function 4E
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F4F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x5f3e, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,50 - Set Current Process ID (Undocumented DOS 2.x)
 * Input: AH = 50h, BX = process ID number (process PSP segment address)
 * Output: (null)
 */
static void F50() {
    CODE_STARTS_AT(0x0019, 0x40a9);
    kw(KDATA_ADDR_PSP) = BX;
    IRET();
}
/**
 * INT 21,51 - Get Current Process ID (Undocumented DOS 2.x)
 * Input: AH = 51
 * Output: BX = process ID
 */
static void F51() {
    CODE_STARTS_AT(0x0019, 0x40b5);
    BX = kw(KDATA_ADDR_PSP);
    IRET();
}
/** TODO
 * INT 21,52 - Get Pointer to DOS "INVARS" (Undocumented)
 * Input: AH = 52
 * Output: ES:BX = pointer to DOS "invars", a table of pointers used by DOS.
 *         Known "invars" fields follow (varies with DOS version):
 *         http://stanislavs.org/helppc/int_21-52.html
 */
static void F52() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d65, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,53 - Generate Drive Parameter Table (Undocumented)
 * Input: AH = 53
 *        DS:SI = address of BIOS Parameter Block (BPB)
 *        ES:BP = pointer to buffer to hold first Drive Parameter Table (DPT)
 * Output: ES:BP = pointer to buffer containing the first DPT in chain
 */
static void F53() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4dd6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,54 - Get Verify Setting
 * Input: AH = 54
 * Output: AL = 00 verify off, 01 verify on
 */
static void F54() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4a41, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,55 - Create New PSP (Undocumented)
 * Input: AH = 55
 *        DX = New PSP segment address
 *        SI = memory size value to place in PSP offset 02h (DOS 3+
 * Output: (null)
 */
static void F55() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4ea5, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,56 - Rename File
 * Input: AH = 56
 *        DS:DX = pointer to old ASCIIZ path/filename
 *        ES:DI = pointer to new ASCIIZ path/filename
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F56() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb05f, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,57 - Get/Set File Date and Time Using Handle
 * Input: AH = 57
 *        AL = 00  get date and time
 *           = 01  set date and time
 *           = 02  ??? (DOS 4.0+ undocumented)
 *           = 03  ??? (DOS 4.0+ undocumented)
 *           = 04  ??? (DOS 4.0+ undocumented)
 *        BX = file handle
 *        CX = time to set (if setting)
 *        DX = date to set (if setting)
 *        ES:DI = pointer to buffer to contain results
 * Output: AX = error code if CF set  (see  DOS ERROR CODES)
 *         CX = file time (if reading, see below)
 *         DX = file date (if reading, see below)
 */
static void F57() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa90d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,58 - Get/Set Memory Allocation Strategy (Undocumented, DOS 3.x)
 * Input: AH = 58
 *        AL = 00  get strategy code
 *           = 01  set strategy code
 *        BX = strategy code (when AL = 01)
 *           = 00  first fit  (default)
 *           = 01  best fit
 *           = 02  last fit
 * Output: AX = strategy code if CF clear
 *            = error if CF set, see  DOS ERROR CODES
 */
static void F58() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa449, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,59 - Get Extended Error Information (DOS 3.0+)
 * Input: AH = 59, BX = 00 for versions  3.0, 3.1, 3.2
 * Output: AX = extended error code (see DOS ERROR CODES), 0 if no error
 *         BH = error class
 *         BL = suggested action
 *         CH = locus
 */
static void F59() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4cdd, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5A - Create Temporary File (DOS 3.0+)
 * Input: AH = 5A
 *        CX = attribute
 *        DS:DX = pointer to ASCIIZ path ending in '\'
 * Output: AX = handle if call CF clear
 *            = error code if CF set  (see DOS ERROR CODES)
 *         DS:DX = pointer to updated ASCIIZ filespec
 */
static void F5A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb0ea, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5B - Create File (DOS 3.0+)
 * Input: AH = 5B
 *        CX = attribute
 *        DS:DX = pointer to ASCIIZ path/filename
 * Output: AX = handle if CF not set
 *            = error code if CF set  (see DOS ERROR CODES)
 */
static void F5B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb0d2, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5C - Lock/Unlock File Access (DOS 3.0+)
 * Input: AH = 5C
 *        AL = 00  lock file
 *           = 01  unlock file
 *        BX = file handle
 *        CX = most significant word of region offset
 *        DX = least significant word of region offset
 *        SI = most significant word of region length
 *        DI = least significant word of region length
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F5C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb2d9, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5D - Critical Error Information (Undocumented, DOS 3.x+)
 * Input: AH = 5D
 *        AL = 00  server function call  (DOS 3.1+)
 *           = 01  commit all files  (DOS 3.1+)
 *           = 02  SHARE: close file by name  (DOS 3.1+)
 *           = 03  SHARE: close all files for given computer  (DOS 3.1+)
 *           = 04  SHARE: close all files for given process  (DOS 3.1+)
 *           = 05  SHARE: get open file list entry  (DOS 3.1+)
 *           = 06  get address of DOS swappable area into DS:SI  (DOS 3.0+)
 *           = 07  get network redirected printer mode  (DOS 3.1+)
 *           = 08  set network redirected printer mode  (DOS 3.1+)
 *           = 09  flush network redirected printer output  (DOS 3.1+)
 *           = 0A  set extended error information  (DOS 3.1+)
 *           = 0B  get DOS swappable data areas  (DOS 4.x+)
 *        DS:DX = pointer to 18 byte DOS Parameter List (DPL, if AL=00)
 *              = pointer to 9 byte data block of the form (AL=0A):
 *              Offset Size	Description
 *        00   word   extended error code to set
 *        02   dword  pointer to driver address to set
 *        06   byte   action code to set
 *        07   byte   class code to set
 *        08   byte   locus code to set
 * Output: DS:SI = (if AL was 6) address of critical flag of the form:
 *      Offset Size      Description
 *        00   word   extended error code
 *        02   byte   action code
 *        03   byte   class code
 *        04   byte   pointer to driver address
 */
static void F5D() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa532, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5E - Machine and Printer Utility Functions
 * http://stanislavs.org/helppc/int_21-5e.html
 */
static void F5E() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xaa4a, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,5F - Redirection Utility Functions
 * http://stanislavs.org/helppc/int_21-5f.html
 */
static void F5F() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa9ab, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,60 - Get Fully Qualified File Name (Undocumented 3.x+)
 * Input: AH = 60
 *        DS:SI = pointer to ASCIIZ string containing unqualified filename
 *        ES:DI = pointer to 128 byte buffer to contain fully qualified filename
 * Output: ES:DI = address of fully qualified filename string
 *         AH = error code if CF set
 */
static void F60() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xaea9, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,61 - ?
 * Input: AH = 61, ?
 * Output: ?
 */
static void F61() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,62 - Get PSP address (DOS 3.x)
 * Input: AH = 62
 * Output: BX = segment address of current process
 */
static void F62() {
    CODE_STARTS_AT(0x0019, 0x40b5);
    BX = kw(KDATA_ADDR_PSP);
    IRET();
}
/** TODO
 * INT 21,63 - Get Lead Byte Table (MSDOS 2.25, Asian Dos 3.2+)
 * http://stanislavs.org/helppc/int_21-63.html
 */
static void F63() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4d0a, 0x41f9);
    _ret(asmFuncEnd());
}
/**
 * INT 21,64 - Set Device Driver Look Ahead (Undocumented)
 * Input: AH = 64h
 *        AL = value for switch
 *             00   causes DOS to perform look ahead to the device driver
 *                  before execution of INT 21,1, INT 21,8 and INT 21,A
 *             other - no look ahead is performed
 * Output: (unknown)
 */
static void F64() {
    CODE_STARTS_AT(0x0019, 0x40c1);
    kb(0x0aa0) = AL;
    IRET();
}
/** TODO
 * INT 21,65 - Get Extended Country Information (DOS 3.3+)
 * http://stanislavs.org/helppc/int_21-65.html
 */
static void F65() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4ad8, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,66 - Get/Set Global Code Page (DOS 3.3+)
 * Input: AH = 66
 *        AL = 01  get global code page
 *           = 02  set global code page
 *        BX = active code page if setting
 *        DX = system code page if setting
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 *         BX = active code page  (for AL=1 only)
 *         DX = system code page  (for AL=1 only)
 */
static void F66() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4bd6, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,67 - Set Handle Count (DOS 3.3+)
 * Input: AH = 67, BX = new maximum open handles allowed
 * Output: CF = 0 if successful, 1 if error
 *         AX = error code if CF is set  (see DOS ERROR CODES)
 */
static void F67() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa778, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,68 - Flush Buffer Using Handle	(DOS 3.3+)
 * Input: AH = 68, BX = file handle
 * Output: AX = error code if CF set  (see DOS ERROR CODES)
 */
static void F68() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa75e, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,69 - Get/Set Disk Serial Number (Undocumented DOS 4+)
 * Input: AH = 69
 *        AL = 00 get serial number
 *           = 01 set serial number
 *        BL = drive (0 = default, 1=A:, 2 = B:, ... )
 *        DS:DX = buffer to contain extended BIOS Parameter Block (BPB, AL=0)
 *              = disk information to set of the form:
 *        Offset  Size    Description
 *            00  word	  info level (zero)
 *            02  dword   disk serial number (binary)
 *            06  11bytes volume label or "NO NAME    " if not present
 *            11  8bytes  filesystem type string "FAT12   " or "FAT16	"
 * Output: CF = set on error
 *            AX = error code if CF set
 *               = 01  network drive not supported
 *               = 05  no extended BPB found on disk
 *         CF = clear if successful and value in AX is destroyed
 *            AL = 00  buffer filled from Extended BPB on disk
 *               = 01  disk Extended BPB updated from buffer
 *         DS:DX = pointer to returned data, when AL = 0
 */
static void F69() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x4fbe, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,6A - ?
 * Input: AH = 6A, ?
 * Output: ?
 */
static void F6A() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xa75e, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,6B - ?
 * Input: AH = 6B, ?
 * Output: ?
 */
static void F6B() {
    _ret(asmFuncBegin());
    CALL_NEAR(0x440d, 0x41f9);
    _ret(asmFuncEnd());
}
/** TODO
 * INT 21,6C - Extended Open/Create (DOS 4.x+)
 * Input: AH = 6C
 *        AL = 00
 *        BX = open mode (see below)
 *        CX = file attribute (see FILE ATTRIBUTES, bits 15-8 are zero)
 *        DX = function control indicator (see below)
 *        DS:SI = ASCIIZ filespec
 * Output: AX = handle if CF clear
 *            = error code if CF set  (see DOS ERROR CODES)
 *         CX = 01  file opened
 *            = 02  file created and opened
 *            = 03  file truncated and opened (replaced)
 *         CF = 0 if successful, 1 if error
 */
static void F6C() {
    _ret(asmFuncBegin());
    CALL_NEAR(0xb184, 0x41f9);
    _ret(asmFuncEnd());
}

/* MSDOS KERNEL SERVICE ENTRY */
static void kernelEntry() {
    CODE_STARTS_AT(0x0019, 0x40fe);
    if (!fpTable[AH]) {
        JMP_FAR(0x0019, 0x40fe);
    } else {
        ExecFun(fpTable[AH]);
    }
}

/* ***** INTERRUPT SERVICE ENTRY ***** */
void int20() {
    LOAD_CS(0x0019);
    kernelDataSeg = _WORD(CS, KCODE_ADDR_DATA_SEG);
    CODE_STARTS_AT(0x0019, 0x40cc);
    AH = 0x00;
    F00();
}
void int21() {
    LOAD_CS(0x0019);
    kernelDataSeg = _WORD(CS, KCODE_ADDR_DATA_SEG);
    CODE_STARTS_AT(0x0019, 0x40f8);
    _ClrEFLAGS_IF;
    if (AH > 0x6c) {
        AL = 0x00;
        IRET();
    } else {
        kernelEntry();
    }
}
void int27() {
    t_bool tempCF;
    LOAD_CS(0x0019);
    kernelDataSeg = _WORD(CS, KCODE_ADDR_DATA_SEG);
    CODE_STARTS_AT(0x0019, 0xa1c4);
    AX = 0x3100;
    DX += 0x0f;
    tempCF = GetLSB(DX);
    DX = (_GetEFLAGS_CF ? MSB16 : Zero16) | (DX >> 1);
    MakeBit(EFLAGS, VCPU_EFLAGS_CF, tempCF);
    DX >>= 3;
    execTerminate_A1FF();
}
void int2a() {
    LOAD_CS(0x0019);
    kernelDataSeg = _WORD(CS, KCODE_ADDR_DATA_SEG);
    CODE_STARTS_AT(0x0019, 0x40d2);
    IRET();
}

void msdosInit() {
    t_nubitcc i;
    for (i = 0; i < 0x100; ++i) {
        fpTable[i] = (t_faddrcc) NULL;
    }
    fpTable[0x00] = (t_faddrcc) F00;
    fpTable[0x01] = (t_faddrcc) F01;
    fpTable[0x02] = (t_faddrcc) F02;
    fpTable[0x03] = (t_faddrcc) F03;
    fpTable[0x04] = (t_faddrcc) F04;
    fpTable[0x05] = (t_faddrcc) F05;
    fpTable[0x06] = (t_faddrcc) F06;
    fpTable[0x07] = (t_faddrcc) F07;
    fpTable[0x08] = (t_faddrcc) F08;
    fpTable[0x09] = (t_faddrcc) F09;
    fpTable[0x0A] = (t_faddrcc) F0A;
    /*
        fpTable[0x0B] = (t_faddrcc) F0B;
        fpTable[0x0C] = (t_faddrcc) F0C;
        fpTable[0x0D] = (t_faddrcc) F0D;
        fpTable[0x0E] = (t_faddrcc) F0E;
        fpTable[0x0F] = (t_faddrcc) F0F;
        fpTable[0x10] = (t_faddrcc) F10;
        fpTable[0x11] = (t_faddrcc) F11;
        fpTable[0x12] = (t_faddrcc) F12;
        fpTable[0x13] = (t_faddrcc) F13;
        fpTable[0x14] = (t_faddrcc) F14;
        fpTable[0x15] = (t_faddrcc) F15;
        fpTable[0x16] = (t_faddrcc) F16;
        fpTable[0x17] = (t_faddrcc) F17;
        fpTable[0x18] = (t_faddrcc) F18;
        fpTable[0x19] = (t_faddrcc) F19;
        fpTable[0x1A] = (t_faddrcc) F1A;
        fpTable[0x1B] = (t_faddrcc) F1B;
        fpTable[0x1C] = (t_faddrcc) F1C;
        fpTable[0x1D] = (t_faddrcc) F1D;
        fpTable[0x1E] = (t_faddrcc) F1E;
        fpTable[0x1F] = (t_faddrcc) F1F;
        fpTable[0x20] = (t_faddrcc) F20;
        fpTable[0x21] = (t_faddrcc) F21;
        fpTable[0x22] = (t_faddrcc) F22;
        fpTable[0x23] = (t_faddrcc) F23;
        fpTable[0x24] = (t_faddrcc) F24;
        fpTable[0x25] = (t_faddrcc) F25;
        fpTable[0x26] = (t_faddrcc) F26;
        fpTable[0x27] = (t_faddrcc) F27;
        fpTable[0x28] = (t_faddrcc) F28;
        fpTable[0x29] = (t_faddrcc) F29;
        fpTable[0x2A] = (t_faddrcc) F2A;
        fpTable[0x2B] = (t_faddrcc) F2B;
        fpTable[0x2C] = (t_faddrcc) F2C;
        fpTable[0x2D] = (t_faddrcc) F2D;
        fpTable[0x2E] = (t_faddrcc) F2E;
        fpTable[0x2F] = (t_faddrcc) F2F;
        fpTable[0x30] = (t_faddrcc) F30;
        fpTable[0x31] = (t_faddrcc) F31;
        fpTable[0x32] = (t_faddrcc) F32;
        fpTable[0x33] = (t_faddrcc) F33;
        fpTable[0x34] = (t_faddrcc) F34;
        fpTable[0x35] = (t_faddrcc) F35;
        fpTable[0x36] = (t_faddrcc) F36;
        fpTable[0x37] = (t_faddrcc) F37;
        fpTable[0x38] = (t_faddrcc) F38;
        fpTable[0x39] = (t_faddrcc) F39;
        fpTable[0x3A] = (t_faddrcc) F3A;
        fpTable[0x3B] = (t_faddrcc) F3B;
        fpTable[0x3C] = (t_faddrcc) F3C;
        fpTable[0x3D] = (t_faddrcc) F3D;
        fpTable[0x3E] = (t_faddrcc) F3E;
        fpTable[0x3F] = (t_faddrcc) F3F;
        fpTable[0x40] = (t_faddrcc) F40;
        fpTable[0x41] = (t_faddrcc) F41;
        fpTable[0x42] = (t_faddrcc) F42;
        fpTable[0x43] = (t_faddrcc) F43;
        fpTable[0x44] = (t_faddrcc) F44;
        fpTable[0x45] = (t_faddrcc) F45;
        fpTable[0x46] = (t_faddrcc) F46;
        fpTable[0x47] = (t_faddrcc) F47;
        fpTable[0x48] = (t_faddrcc) F48;
        fpTable[0x49] = (t_faddrcc) F49;
        fpTable[0x4A] = (t_faddrcc) F4A;
        fpTable[0x4B] = (t_faddrcc) F4B;
        fpTable[0x4C] = (t_faddrcc) F4C;
        fpTable[0x4D] = (t_faddrcc) F4D;
        fpTable[0x4E] = (t_faddrcc) F4E;
        fpTable[0x4F] = (t_faddrcc) F4F;
        fpTable[0x50] = (t_faddrcc) F50;
        fpTable[0x51] = (t_faddrcc) F51;
        fpTable[0x52] = (t_faddrcc) F52;
        fpTable[0x53] = (t_faddrcc) F53;
        fpTable[0x54] = (t_faddrcc) F54;
        fpTable[0x55] = (t_faddrcc) F55;
        fpTable[0x56] = (t_faddrcc) F56;
        fpTable[0x57] = (t_faddrcc) F57;
        fpTable[0x58] = (t_faddrcc) F58;
        fpTable[0x59] = (t_faddrcc) F59;
        fpTable[0x5A] = (t_faddrcc) F5A;
        fpTable[0x5B] = (t_faddrcc) F5B;
        fpTable[0x5C] = (t_faddrcc) F5C;
        fpTable[0x5D] = (t_faddrcc) F5D;
        fpTable[0x5E] = (t_faddrcc) F5E;
        fpTable[0x5F] = (t_faddrcc) F5F;
        fpTable[0x60] = (t_faddrcc) F60;
        fpTable[0x61] = (t_faddrcc) F61;
        fpTable[0x62] = (t_faddrcc) F62;
        fpTable[0x63] = (t_faddrcc) F63;
        fpTable[0x64] = (t_faddrcc) F64;
        fpTable[0x65] = (t_faddrcc) F65;
        fpTable[0x66] = (t_faddrcc) F66;
        fpTable[0x67] = (t_faddrcc) F67;
        fpTable[0x68] = (t_faddrcc) F68;
        fpTable[0x69] = (t_faddrcc) F69;
        fpTable[0x6A] = (t_faddrcc) F6A;
        fpTable[0x6B] = (t_faddrcc) F6B;
        fpTable[0x6C] = (t_faddrcc) F6C;
    */
}
void msdosFinal() {}
