/* Copyright 2012-2014 Neko. */

/* MSDOS is the virtual DOS kernel. */

#include "../../src/utils.h"
#include "../../src/device/device.h"
#include "../../src/device/vcpuins.h"
#include "../../src/device/vram.h"

#include "msdos.h"

#define say PRINTF

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

#define _BYTE vramRealByte
#define _WORD vramRealWord
#define _DWORD vramRealDWord

/* stack pointer size */
#define _GetStackSize   (vcpu.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_addrsize) ? 4 : 2)

static void GOTO(t_nubit16 seg, t_nubit16 off) {
    LOAD_CS(seg);
    vcpu.data.eip = off;
}
static void INT_OLD(t_nubit8 intId) {
    t_nubit16 newCS, newIP, oldCS, oldIP;
    newIP = _WORD(Zero16, intId * 4 + 0);
    newCS = _WORD(Zero16, intId * 4 + 2);
    oldIP = _WORD(newCS, newIP + 2);
    oldCS = _WORD(newCS, newIP + 4);
    GOTO(oldCS, oldIP);
}

static void CLI() {
    _ClrEFLAGS_IF;
}
static void STI() {
    _SetEFLAGS_IF;
}
static void CLD() {
    _ClrEFLAGS_DF;
}
static void STD() {
    _SetEFLAGS_DF;
}
static void PUSH(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(SP - byte);
        break;
    case 4:
        cesp = GetMax32(ESP - byte);
        break;
    default:
        break;
    }
    vramWritePhysical(vcpu.data.ss.base + cesp, rdata, byte);
    switch (_GetStackSize) {
    case 2:
        SP -= byte;
        break;
    case 4:
        ESP -= byte;
        break;
    default:
        break;
    }
}
static void POP(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    switch (_GetStackSize) {
    case 2:
        cesp = SP;
        break;
    case 4:
        cesp = ESP;
        break;
    default:
        break;
    }
    vramReadPhysical(vcpu.data.ss.base + cesp, rdata, byte);
    if (rdata != GetRef(vcpu.data.esp)) {
        switch (_GetStackSize) {
        case 2:
            vcpu.data.sp += byte;
            break;
        case 4:
            vcpu.data.esp += byte;
            break;
        default:
            break;
        }
    }
}
static void INT(t_nubit8 intId) {
    t_nubit16 intCS, intIP;
    t_nubit32 oldcs = vcpu.data.cs.selector;
    switch (_GetOperandSize) {
    case 2:
        PUSH(GetRef(vcpu.data.flags), _GetOperandSize);
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        PUSH(GetRef(oldcs), _GetOperandSize);
        PUSH(GetRef(vcpu.data.ip), _GetOperandSize);
        break;
    case 4:
        PUSH(GetRef(vcpu.data.eflags), _GetOperandSize);
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        PUSH(GetRef(oldcs), _GetOperandSize);
        PUSH(GetRef(vcpu.data.eip), _GetOperandSize);
        break;
    default:
        break;
    }
    /* intIP + 4 points to the original vector */
    intIP = _WORD(Zero16, intId * 4 + 0);
    intCS = _WORD(Zero16, intId * 4 + 2);
    GOTO(intCS, intIP);
}
static void IRET() {
    t_nubit16 newcs;
    t_nubit32 neweip = Zero32, neweflags = Zero32;
    t_nubit32 xs_sel;
    t_nubit32 mask = VCPU_EFLAGS_RESERVED;
    switch (_GetOperandSize) {
    case 2:
        POP(GetRef(neweip), 2);
        POP(GetRef(xs_sel), 2);
        newcs = GetMax16(xs_sel);
        POP(GetRef(neweflags), 2);
        mask |= 0xffff0000;
        break;
    case 4:
        POP(GetRef(neweip), 4);
        POP(GetRef(xs_sel), 4);
        newcs = GetMax16(xs_sel);
        POP(GetRef(neweflags), 4);
        /* vcpu.data.eflags = (neweflags & 0x00257fd5) | (vcpu.data.eflags & 0x001a0000); */
        break;
    default:
        break;
    }
    vcpu.data.eflags = (neweflags & ~mask) | (vcpu.data.eflags & mask);
    GOTO(newcs, neweip);
}

static void INT20();
static void INT21();
static void INT22();
static void INT23();
static void INT24();
static void INT25();
static void INT26();
static void INT27();
static void INT28();
static void INT29();
static void INT2A();
static void INT2B();
static void INT2C();
static void INT2D();
static void INT2E();
static void INT2F();

static t_faddrcc fpTable[0x100];
static t_nubit16 kernelSeg;

static void saveRegisters() {
    PUSH(GetRef(ES), 2);
    PUSH(GetRef(DS), 2);
    PUSH(GetRef(BP), 2);
    PUSH(GetRef(DI), 2);
    PUSH(GetRef(SI), 2);
    PUSH(GetRef(DX), 2);
    PUSH(GetRef(CX), 2);
    PUSH(GetRef(BX), 2);
    PUSH(GetRef(AX), 2);
}
static void restoreRegisters() {
    POP(GetRef(ES), 2);
    POP(GetRef(DS), 2);
    POP(GetRef(BP), 2);
    POP(GetRef(DI), 2);
    POP(GetRef(SI), 2);
    POP(GetRef(DX), 2);
    POP(GetRef(CX), 2);
    POP(GetRef(BX), 2);
    POP(GetRef(AX), 2);
}

/* INT 21,33 - Get/Set System Values (Ctl-Break/Boot Drive)
 *
 * AH = 33h
 * 	AL = 00 to get Ctrl-Break checking flag
 * 	   = 01 to set Ctrl-Break checking flag
 * 	   = 02 to set extended Ctrl-Break checking
 * 	   = 05 get boot drive (DOS 4.x)
 * 	DL = 00 to set Ctrl-Break checking off
 * 	   = 01 to set Ctrl-Break checking on
 * 	   = boot drive for subfunction 5;  (1=A:, 2=B:, ...)
 *
 * 	on return:
 * 	DL = 00 Ctrl-Break checking OFF (AL=0 or AL=2)
 * 	   = 01 Ctrl-Break checking ON	(AL=0 or AL=2)
 * 	   = boot drive number (1-26, A: - Z:) (function 05)
 *
 * 	- retrieves DOS Ctrl-Break or extended Ctrl-Break setting which
 * 	  determines if DOS will check for Ctrl-Break during INT 21 calls
 */
static void F33() {
    t_nubit8 temp8;
    /* ***** 0019:4052 ***** */
    switch (AL) {
    case 0x00:
        /* get ctrl-break status */
        DL = _BYTE(kernelSeg, 0x0337);
        break;
    case 0x01:
        /* set ctrl-break status according to DL */
        _BYTE(kernelSeg, 0x0337) = DL & 0x01;
        break;
    case 0x02:
        /* set ctrl-break status from DL,
         * and return original status to DL */
        temp8 = DL & 0x01;
        DL = _BYTE(kernelSeg, 0x0337);
        _BYTE(kernelSeg, 0x0337) = temp8;
        break;
    case 0x03:
        /* ? get something unknown */
        DL = _BYTE(kernelSeg, 0x0069);
        break;
    case 0x04:
        /* ? do something unknown */
        BX = 0x1606;
        DX = 0x0000;
        if (_BYTE(kernelSeg, 0x1211) != 0x00) {
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
    /* ***** 0019:40a6 ***** */
}

/* INT 21,50 - Set Current Process ID (Undocumented DOS 2.x)
 *
 *  AH = 50h
 *  BX = process ID number (process PSP segment address)
 *
 *  returns nothing
 *
 *  - the process ID number is actually the segment address of a
 *   program's PSP.  This is useful for TSR's to access their own
 *   file handle table inside their PSP.
 *  - this function cannot be called while in an INT 28 handler in
 *   DOS 2.x unless the critical error flag is set or stack corruption
 *   will occur
 *  - safe in DOS 3.x INT 28 handlers since they use a different stack
 *   by default
 *  - available in OS/2 compatibility box
 *  - see  INT 21,51   INT 21,62
 */
static void F50() {
    /* ***** 0019:40a9 ***** */
    _WORD(kernelSeg, 0x0330) = BX;
    /* ***** 0019:40b4 ***** */
}

/* INT 21,51 - Get Current Process ID (Undocumented DOS 2.x)
 *
 * 	AH = 51h
 *
 * 	on return:
 * 	BX = process ID
 *
 * 	- The process ID  number is actually the segment address of program's
 * 	  PSP.	 This in conjunction with INT 21,50 is useful for TSR's to
 * 	  access their own file handle table in their respective PSP.
 * 	- this function cannot be called while in an INT 28 handler in
 * 	  DOS 2.x unless the critical error flag is set or stack corruption
 * 	  will occur
 * 	- INT 21,62 is highly recommended for DOS 3.x due to a possible bug
 * 	  when activated from a TSR.  DOS may switch to the wrong internal
 * 	  stack which may cause a problems with TSR's if called during an
 * 	  INT 28.
 * 	- see INT 21,62 (Get PSP segment) for DOS 3.x applications
 */
static void F51() {
    /* ***** 0019:40b5 ***** */
    BX = _WORD(kernelSeg, 0x0330);
    /* ***** 0019:40c0 ***** */
}

/* INT 21,62 - Get PSP address (DOS 3.x)
 *
 * 	AH = 62h
 *
 * 	on return:
 * 	BX = segment address of current process
 *
 * 	- this is useful for TSR's to access their own file handles
 * 	- for DOS 2.x use INT 21,51
 *
 * 	- see also  INT 21,50
 */
static void F62() {
    /* ***** 0019:40b5 ***** */
    BX = _WORD(kernelSeg, 0x0330);
    /* ***** 0019:40c0 ***** */
}

/* INT 21,64 - Set Device Driver Look Ahead (Undocumented)

 *	AH = 64h
 *	AL = value for switch
 *	     00   causes DOS to perform look ahead to the device driver
 *		  before execution of INT 21,1, INT 21,8 and INT 21,A
 *	     other - no look ahead is performed
 *
 *	returns ???
 *
 *	- DOS internal; should not be called by user program
 *	- switch defaults to zero
 *	- undocumented; since DOS 3.3
 *	- some other subfunctions are know to have existed since DOS 3.2 but
 *	  details on their functionality are incomplete
 */
static void F64() {
    /* ***** 0019:40c1 ***** */
    _BYTE(kernelSeg, 0x0aa0) = AL;
    /* ***** 0019:40cb ***** */
}

/* MSDOS KERNEL ENTRY */
static void MSDOS() {
    t_nubit16 temp16;
    t_nubit16 kernelSeg = _WORD(CS, 0x3de7);

    /* ***** 0019:40fe ***** */
    switch (AH) {
    case 0x33:
        F33();
        IRET(); /* <--------------------------------- REMOVE */
        return;
    case 0x50:
        F50();
        IRET(); /* <--------------------------------- REMOVE */
        return;
    case 0x51:
        F51();
        IRET(); /* <--------------------------------- REMOVE */
        return;
    case 0x62:
        F62();
        IRET(); /* <--------------------------------- REMOVE */
        return;
    case 0x64:
        F64();
        IRET(); /* <--------------------------------- REMOVE */
        return;
    }

    /* ***** 0019:411b ***** */
    saveRegisters();

    /* ***** 0019:4124 ***** */
    /* save current DS:BX */
    _WORD(kernelSeg, 0x05ec) = DS;
    _WORD(kernelSeg, 0x05ea) = BX;

    /* save program stack pointer to data area and program psp */
    _WORD(kernelSeg, 0x05f2) = _WORD(kernelSeg, 0x0584);
    _WORD(kernelSeg, 0x05f0) = _WORD(kernelSeg, 0x0586);
    /* save current SS:SP for int 21 */
    _WORD(kernelSeg, 0x0584) = SP;
    _WORD(kernelSeg, 0x0586) = SS;

    /* ? clear env flag */
    _BYTE(kernelSeg, 0x0572) = 0x00;

    if (!GetBit(_BYTE(kernelSeg, 0x1030), 0x01)) {
        _WORD(kernelSeg, 0x033e) = 0x0000; /* ? */
    }

    /* set dos stack flag */
    _BYTE(kernelSeg, 0x0321) += 0x01;

    /* ? copy current psp seg address to sub-program psp seg address unit */
    _WORD(kernelSeg, 0x033c) = _WORD(kernelSeg, 0x0330);
    LOAD_DS(_WORD(kernelSeg, 0x0330));

    /* save current SS:SP to PSP */
    _WORD(DS, 0x002e) = SP;
    _WORD(DS, 0x0030) = SS;

    /* switch to DOS stack */
    LOAD_SS(kernelSeg);
    SP = 0x07a0;
    STI();

    _BYTE(kernelSeg, 0x05f6) = 0x00; /* ? */
    _WORD(kernelSeg, 0x0611) &= 0x0800; /* ? */
    /* ? set sub-folder mode */
    _BYTE(kernelSeg, 0x034c) = 0x00;
    /* ? clear int 24 failure flag */
    _BYTE(kernelSeg, 0x034a) = 0x00;
    /* ? clear I/O redirection */
    _BYTE(kernelSeg, 0x0357) = 0x00;
    _BYTE(kernelSeg, 0x0358) = 0x01;

    /* ***** 0019:4198 ***** */
    CLD();
    if (AH >= 0x01 && AH <= 0x0c) {
        /* modification flag is clear */
        if (_BYTE(kernelSeg, 0x0320) == 0x00) {
            /* set DOS internal stack pointer */
            SP = 0x0aa0;
        }
    } else if (AH != 0x59) {
        _WORD(kernelSeg, 0x033a) = AX;
        /* set flag for unknown error position */
        _BYTE(kernelSeg, 0x0323) = 0x01;
        /* clear modification flag */
        _BYTE(kernelSeg, 0x0320) = 0x00;
        /* set flag for 'cannot write into disk drive' */
        _BYTE(kernelSeg, 0x0322) = 0xff;
        /* clear dos critical section 0 - 7 */
        temp16 = AX;
        AH = 0x82;
        INT2A();
        AX = temp16;
        /* ? */
        _BYTE(kernelSeg, 0x0358) = 0x00;
        /* change DOS internal stack pointer */
        SP = 0x0920;
        /* test ctrl-break flag */
        if (_BYTE(kernelSeg, 0x0337) & 0xff) {
            /* do ctrl-break check */
            PUSH(GetRef(AX), 2);
            /* here is a function call to CS:9080 */
            LOAD_DS(SS);
            GOTO(CS, 0x41e6); /* <--------------------------------- TODO */
            return;
        }
    }

    /* ***** 0019:41ea ***** */
    /* recover DS:BX */
    BX = _WORD(kernelSeg, 0x05ea);
    LOAD_DS(_WORD(kernelSeg, 0x05ec));
    /* store the real int 21 function entry, but do not use it */
    _WORD(kernelSeg, 0x05ea) = _WORD(CS, AH * 2 + 0x3e9e);

    /* ***** 0019:41f9 ***** */
    if (!fpTable[AH]) {
        GOTO(CS, 0x41f9); /* <--------------------------------- TODO */
        return;
    }

    ExecFun(fpTable[AH]);
    _BYTE(kernelSeg, 0x0086) &= 0xfb; /* ? */
    CLI();
    /* copy dos kernel segment address to DS */
    LOAD_DS(kernelSeg);
    /* ? */
    if (_BYTE(kernelSeg, 0x0085) != 0x00) {
        GOTO(CS, 0x4238); /* <--------------------------------- TODO */
        return;
    }

    /* clear int 21 executing flag */
    _BYTE(kernelSeg, 0x0321) -= 0x01;

    /* restore SS:SP */
    LOAD_SS(_WORD(kernelSeg, 0x0586));
    SP = _WORD(kernelSeg, 0x0584);
    /* restore program stack pointer */
    _WORD(kernelSeg, 0x0586) = _WORD(kernelSeg, 0x05f0);
    _WORD(kernelSeg, 0x0584) = _WORD(kernelSeg, 0x05f2);

    /* send return value to caller's stack */
    _BYTE(SS, SP) = AL;

    /* ***** 0019:422e ***** */
    restoreRegisters();

    /* ***** 0019:4237 ***** */
    /* move IRET() from here to int21() when all GOTO are cleaned up in INT21() */
    /* IRET */
    IRET(); /* <--------------------------------- REMOVE */
    return;
}

static void INT20() {
    AH = 0x00;
    MSDOS();
}
static void INT21() {
    /* ***** 0019:40f8 ***** */
    CLI();
    if (AH > 0x6c) {
        AL = 0x00;
        IRET(); /* <--------------------------------- REMOVE */
    } else {
        MSDOS();
    }
}
static void INT2A() {}

void int20() {
    LOAD_CS(0x0019);
    INT20();
}
void int21() {
    LOAD_CS(0x0019);
    INT21();
    /* IRET(); <--------------------------------- ADD */
}
void int2a() {
    LOAD_CS(0x0019);
    INT2A();
    IRET();
}

void msdosInit() {
    t_nubitcc i;
    for (i = 0; i < 0x100; ++i) {
        fpTable[i] = (t_faddrcc) NULL;
    }
    fpTable[0x33] = (t_faddrcc) F33;
    fpTable[0x50] = (t_faddrcc) F50;
    fpTable[0x51] = (t_faddrcc) F51;
    fpTable[0x62] = (t_faddrcc) F62;
    fpTable[0x64] = (t_faddrcc) F64;
}
void msdosFinal() {}
