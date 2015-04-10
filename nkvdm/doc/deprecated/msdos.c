/* Copyright 2012-2014 Neko. */

/* MSDOS is the virtual DOS kernel. */

#include "nxvmsrc/utils.h"
#include "nxvmsrc/device/device.h"
#include "nxvmsrc/device/vmachine.h"
#include "nxvmsrc/device/vcpuins.h"
#include "nxvmsrc/device/vram.h"

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

#define dbyte vramRealByte
#define dword vramRealWord
#define ddword vramRealDWord

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
    intIP = dword(Zero16, intId * 4 + 0);
    intCS = dword(Zero16, intId * 4 + 2);
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

static t_faddrcc fpTable[0x100];
static t_nubit16 kernelSeg;

#define kb(off) dbyte(kernelSeg, (off))
#define kw(off) dword(kernelSeg, (off))
#define kd(off) ddword(kernelSeg, (off))

#define KCODE_ADDR_DATA   0x3de7
#define KCODE_ADDR_FUNC   0x3e9e

#define KDATA_FLAG_MODIFY 0x0320
#define KDATA_FLAG_INT21  0x0321
#define KDATA_ADDR_PSP    0x0330
#define KDATA_FLAG_ENV    0x0572
#define KDATA_SAVE_SP     0x0584
#define KDATA_SAVE_SS     0x0586
#define KDATA_SAVE_BX     0x05ea
#define KDATA_SAVE_DS     0x05ec

/* ***** UTILITY SECTION ***** */

/* true if need to restart msdos kernel processing */
static t_bool checkCtrlBreak() {
    /* ***** 0019:9080 ***** */
    /* check if currently on DOS internal stack */
    /* this is the int 21 executing flag */
    if (dbyte(SS, KDATA_FLAG_INT21) != 0x01) {
        return False;
    }

    /* ***** 0019:9089 ***** */
    /* save registers */
    PUSH(GetRef(CX), 2);
    PUSH(GetRef(ES), 2);
    PUSH(GetRef(BX), 2);
    PUSH(GetRef(DS), 2);
    PUSH(GetRef(SI), 2);
    LOAD_ES(SS);
    LOAD_DS(SS);

    /* build device driver request header for command 05 peek*/
    dbyte(SS, 0x0394) = 0x05; /* command 05 */
    dbyte(SS, 0x0392) = 0x0e; /* length of header is 0e */
    dword(SS, 0x0395) = 0x0000; /* initialize status word */

    /* send request header address to DS:BX */
    BX = 0x0392;

    /* send device header address to DS:SI */
    SI = dword(SS, 0x0032);
    LOAD_DS(dword(SS, 0x0034));

    /* send peek command */
    CALL_NEAR(0x85ed, 0x90b2); /* <--------------------------------- TODO */

    /* ***** 0019:90b2 ***** */
    /* test if device is busy */
    if (GetBit(dbyte(SS, 0x0396), 0x02)) {
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
    AL = dbyte(SS, 0x039f);

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
    dbyte(SS, 0x0394) = 0x04; /* command = 04 */
    dbyte(SS, 0x0392) = 0x16; /* length = 16 */
    dbyte(SS, 0x039f) = CL; /* media descriptor */
    dword(SS, 0x0395) = 0x0000; /* initialize status word */
    dword(SS, 0x03a4) = 0x0001; /* input 1 char */

    /* send input command */
    CALL_NEAR(0x85ed, 0x90bc); /* <--------------------------------- TODO */

    POP(GetRef(SI), 2);
    POP(GetRef(DS), 2);
    LOAD_DS(DS);
    POP(GetRef(BX), 2);
    POP(GetRef(ES), 2);
    LOAD_ES(ES);
    POP(GetRef(CX), 2);

    /* ***** 0019:91c4 ***** */
    /* check flag that do not output char ^C */
    if (!GetBit(dbyte(SS, 0x0612), 0x02)) {
        /* output ^C */
        AL = 0x03;
        CALL_NEAR(0x5582, 0x91d1); /* <--------------------------------- TODO */
        /* output enter */
        CALL_NEAR(0x5412, 0x91d4); /* <--------------------------------- TODO */
    }

    /* ***** 0019:91d4 ***** */
    LOAD_DS(SS);
    if (kb(0x0357) == 0x00) {
        CALL_NEAR(0x71e3, 0x91e0); /* <--------------------------------- TODO */
    }

    /* ***** 0019:91e0 ***** */
    _ClrEFLAGS_IF;
    /* switch from DOS stack to program stack */
    LOAD_SS(kw(KDATA_SAVE_SS));
    SP = kw(KDATA_SAVE_SP);

    /* what the fuck? */
    CALL_NEAR(0x424c, 0x91ec); /* <--------------------------------- TODO */
    POP(GetRef(ES), 2);
    LOAD_ES(ES);

    /* set DOS internal stack flag */
    kb(KDATA_FLAG_INT21) = 0x00;
    /* set flag for int 24 not ready */
    kb(KDATA_FLAG_MODIFY) = 0x00;
    /* save SP and pop one word because now in a near call */
    kw(0x0332) = SP;
    kw(0x0332) += 0x0002;

    /* if program has ctrl-break processor */
    if (kb(0x1211) == 0x00) {
        /* ***** 0019:9213 ***** */
        _ClrEFLAGS_CF;
        /* call default ctrl-break processor */
        /* <--------------------------------- TODO */
        CALL_FAR(dword(CS, 0x9076), dword(CS, 0x9074), 0x9219);
    } else {
        /* ***** 0019:920e ***** */
        _ClrEFLAGS_CF;
        /* call program defined ctrl-break processor */
        INT(0x23, 0x9211);
    }

    /* ***** 0019:9219 ***** */
    _ClrEFLAGS_IF;
    kw(0x0d0a) = DS; /* save DS value */
    kw(0x033a) = AX; /* save return value */
    LOAD_DS(kernelSeg);
    AX = FLAGS;

    /* if in dos int service */
    if (SP != kw(0x0332)) {
        /* ***** 0019:923b ***** */
        SP += 0x0002; /* because the ctrl-break check function is a near call */
        if (GetBit(AL, 0x01)) {
            LOAD_DS(kw(0x0d0a)); /* restore DS value */
            AH = 0x4c; /* invoke dos int 21 function 4c */
            AL = 0x00; /* return value from ctrl-break is 00 */
            kb(0x034d) = 0xff;
            return True;
        }
    }

    /* ***** 0019:9231 ***** */
    AX = kw(0x033a); /* restore return value to AX */
    LOAD_DS(kw(0x0d0a)); /* restore DS value */
    return True;
}

/* ***** EXEC SECTION ***** */

/* Program Segment Prefix (PSP)
 *
 * 00   word	machine code INT 20 instruction (CDh 20h)
 * 02   word	top of memory in segment (paragraph) form
 * 04   byte	reserved for DOS, usually 0
 * 05  5bytes	machine code instruction long call to the DOS
 *              function dispatcher (obsolete CP/M)
 * 06   word	.COM programs bytes available in segment (CP/M)
 * 0A   dword	INT 22 terminate address;  DOS loader jumps to this
 *              address upon exit;  the EXEC function forces a child
 *              process to return to the parent by setting this
 *              vector to code within the parent (IP,CS)
 * 0E   dword	INT 23 Ctrl-Break exit address; the original INT 23
 *              vector is NOT restored from this pointer (IP,CS)
 * 12   dword	INT 24 critical error exit address; the original
 *              INT 24 vector is NOT restored from this field (IP,CS)
 * 16   word	parent process segment addr (Undoc. DOS 2.x+)
 *              COMMAND.COM has a parent id of zero, or its own PSP
 * 18  20bytes	file handle array (Undocumented DOS 2.x+); if handle
 *              array element is FF then handle is available.  Network
 *              redirectors often indicate remotes files by setting
 *              these to values between 80-FE.
 * 2C   word	segment address of the environment, or zero (DOS 2.x+)
 * 2E   dword	SS:SP on entry to last INT 21 function (Undoc. 2.x+) Ø
 * 32   word	handle array size (Undocumented DOS 3.x+)
 * 34   dword	handle array pointer (Undocumented DOS 3.x+)
 * 38   dword	pointer to previous PSP (deflt FFFF:FFFF, Undoc 3.x+) Ø
 * 3C  20bytes	unused in DOS before 4.01  Ø
 * 50   3bytes	DOS function dispatcher CDh 21h CBh (Undoc. 3.x+) Ø
 * 53   9bytes	unused
 * 5C  36bytes	default unopened FCB #1 (parts overlayed by FCB #2)
 * 6C  20bytes	default unopened FCB #2 (overlays part of FCB #1)
 * 80 128bytes  default DTA
 * 80   byte    count of characters in command tail;  all bytes
 *              following command name;
 * 81 127bytes	all characters entered after the program name followed
 *              by a CR byte
 * - offset 5 contains a jump address which is 2 bytes too low for
 *   PSP's created by the DOS EXEC function in DOS 2.x+  Ø
 * - program name and complete path can be found after the environment
 *   in DOS versions after 3.0.  See offset 2Ch.
 */
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

/* ***** FUNCTION SECTION ***** */

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
        DL = kb(0x0337);
        break;
    case 0x01:
        /* set ctrl-break status according to DL */
        kb(0x0337) = DL & 0x01;
        break;
    case 0x02:
        /* set ctrl-break status from DL,
         * and return original status to DL */
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
    /* ***** 0019:40a6 ***** */
    IRET();
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
    kw(KDATA_ADDR_PSP) = BX;
    /* ***** 0019:40b4 ***** */
    IRET();
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
    BX = kw(KDATA_ADDR_PSP);
    /* ***** 0019:40c0 ***** */
    IRET();
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
    BX = kw(KDATA_ADDR_PSP);
    /* ***** 0019:40c0 ***** */
    IRET();
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
    kb(0x0aa0) = AL;
    /* ***** 0019:40cb ***** */
    IRET();
}

/* MSDOS KERNEL ENTRY */
static void MSDOS() {
    t_bool flagStartOver;
    t_nubit16 temp16;
    t_nubit8 funId = AH;
    t_nubit8 retVal = AL;

    LOAD_CS(0x0019);
    kernelSeg = dword(CS, KCODE_ADDR_DATA);

    do {
        flagStartOver = False;
        /* ***** 0019:40fe ***** */
        switch (funId) {
        case 0x33:
            F33();
            return;
        case 0x50:
            F50();
            return;
        case 0x51:
            F51();
            return;
        case 0x62:
            F62();
            return;
        case 0x64:
            F64();
            return;
        }

        /* ***** 0019:411b ***** */

        /* ***** 0019:4124 ***** */
        /* save program stack pointer to data area and program psp */
        kw(0x05f2) = kw(KDATA_SAVE_SP);
        kw(0x05f0) = kw(KDATA_SAVE_SS);

        /* ? clear env flag */
        kb(KDATA_FLAG_ENV) = 0x00;

        if (!GetBit(kb(0x1030), 0x01)) {
            kw(0x033e) = 0x0000; /* ? */
        }

        /* set dos stack flag */
        kb(KDATA_FLAG_INT21) += 0x01;

        /* ? copy current psp seg address to sub-program psp seg address unit */
        kw(0x033c) = kw(KDATA_ADDR_PSP);

        kb(0x05f6) = 0x00; /* ? */
        kw(0x0611) &= 0x0800; /* ? */
        /* ? set sub-folder mode */
        kb(0x034c) = 0x00;
        /* ? clear int 24 failure flag */
        kb(0x034a) = 0x00;
        /* ? clear I/O redirection */
        kb(0x0357) = 0x00;
        kb(0x0358) = 0x01;

        /* ***** 0019:4198 ***** */
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
        /* save current SS:SP for int 21 */
        kw(KDATA_SAVE_SP) = SP;
        kw(KDATA_SAVE_SS) = SS;
        /* save current SS:SP to PSP */
        dword(kw(KDATA_ADDR_PSP), PSP_STACK_SP) = SP;
        dword(kw(KDATA_ADDR_PSP), PSP_STACK_SS) = SS;
        /* switch to DOS stack */
        LOAD_DS(kw(KDATA_ADDR_PSP));
        LOAD_SS(kernelSeg);
        SP = 0x07a0;
        _SetEFLAGS_IF;
        _ClrEFLAGS_DF;

        if (funId >= 0x01 && funId <= 0x0c) {
            /* modification flag is clear */
            if (kb(KDATA_FLAG_MODIFY) == 0x00) {
                /* set DOS internal stack pointer */
                SP = 0x0aa0;
            }
        } else if (funId != 0x59) {
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
                /* need to verify */
                if (checkCtrlBreak()) {
                    flagStartOver = True;
                    continue;
                }
                AX = temp16;
            }
        }
    } while (flagStartOver);

    /* ***** 0019:41ea ***** */
    /* recover DS:BX */
    BX = kw(KDATA_SAVE_BX);
    LOAD_DS(kw(KDATA_SAVE_DS));

    /* ***** 0019:41f9 ***** */
    if (!fpTable[funId]) {
        CALL_NEAR(dword(CS, funId * 2 + KCODE_ADDR_FUNC), 0x41fe);
    } else {
        ExecFun(fpTable[funId]);
    }

    /* ? */
    kb(0x0086) &= 0xfb;
    _ClrEFLAGS_IF;

    /* ? */
    if (kb(0x0085) != 0x00) {
        /* who equals to program's psp? */
        if (kw(0x0063) == kw(KDATA_ADDR_PSP)) {
            /* ***** 0019:4242 ***** */
            kb(0x0085) -= 0x01;
            LOAD_DS(kernelSeg);
            /* this is shit! it jumps to invalid code section. */
            JMP_FAR(kernelSeg, 0x112d);
            return;
        }
    }

    /* ***** 0019:4211 ***** */
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
    dbyte(SS, SP) = AL;

    /* ***** 0019:422e ***** */
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

    /* ***** 0019:4237 ***** */
    IRET();
}

/* MSDOS INT SERVICE ENTRY */
void int20() {
    AH = 0x00;
    MSDOS();
}
void int21() {
    /* ***** 0019:40f8 ***** */
    _ClrEFLAGS_IF;
    if (AH > 0x6c) {
        AL = 0x00;
        IRET();
    } else {
        MSDOS();
    }
}
void int2a() {
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
