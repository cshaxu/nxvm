/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"

#include "../vapi.h"
#include "../vmachine.h"

#include "qdbios.h"
#include "qddisk.h"

static t_nubit16 sax, sbx, scx, sdx;
#define push_ax  (sax = _ax)
#define push_bx  (sbx = _bx)
#define push_cx  (scx = _cx)
#define push_dx  (sdx = _dx)
#define pop_ax   (_ax = sax)
#define pop_bx   (_bx = sbx)
#define pop_cx   (_cx = scx)
#define pop_dx   (_dx = sdx)
#define add(a,b) ((a) += (b))
#define and(a,b) ((a) &= (b))
#define or(a,b)  ((a) |= (b))
#define cmp(a,b) ((a) == (b))
#define mov(a,b) ((a) = (b))
#define in(a,b)  (ExecFun(vport.in[(b)]), (a) = vport.iobyte)
#define out(a,b) (vport.iobyte = (b), ExecFun(vport.out[(a)]))
#define shl(a,b) ((a) <<= (b))
#define shr(a,b) ((a) >>= (b))
#define stc      (SetCF)
#define clc      (ClrCF)
#define clp      (ClrPF)
#define cli      (ClrIF)
#define mbp(n)   (vramVarByte(0x0000, (n)))
#define mwp(n)   (vramVarWord(0x0000, (n)))
#define nop
#define inc(n)   ((n)++)
#define dec(n)   ((n)--)
#define _int(n)

static void INT_0E() {
    push_ax;
    push_dx;
    in(_al, 0x03f4); /* get msr */
    and(_al, 0xc0);  /* get ready write status */
    mov(_dx, 0x03f5);
    if (cmp(_al, 0x80)) {
        mov(_al, 0x08);
        out(_dx, _al); /* send senseint command */
        in (_al, _dx); /* read senseint ret st0: 0x20 */
        in (_al, _dx); /* read senseint ret cyl: 0x00 */
    }
    /* FDC 3F5 4A to check id; not needed for vm */
    mov(_al, 0x20);
    out(0x20, _al); /* send eoi command to vpic */
    pop_dx;
    pop_ax;
}

#define SetFddStatus (vramVarByte(0x0040, 0x0041) = _ah)
#define GetFddStatus (vramVarByte(0x0040, 0x0041))
#define SetHddStatus (vramVarByte(0x0040, 0x0074) = _ah)
#define GetHddStatus (vramVarByte(0x0040, 0x0074))
t_vaddrcc vfddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector) {
    vfdd.cyl = cyl;
    vfdd.head = head;
    vfdd.sector = sector;
    vfddSetPointer;
    return vfdd.curr;
}
t_vaddrcc vhddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector) {
    vhdd.cyl = cyl;
    vhdd.head = head;
    vhdd.sector = sector;
    vhddSetPointer;
    return vhdd.curr;
}
static void INT_13_00_FDD_ResetDrive() {
    _ah = 0x00;
    ClrBit(_flags, VCPU_FLAG_CF);
    SetFddStatus;
    SetHddStatus;
}
static void INT_13_01_FDD_GetDiskStatus() {
    if (_dl & 0x80) {/* Hard Disk */
        _ah = 0x80;
        SetHddStatus;
    } else {
        _ah = 0x00;
        SetFddStatus;
    }
    SetFddStatus;
}
static void INT_13_02_FDD_ReadSector() {
    t_nubit8 drive  = _dl;
    t_nubit8 head   = _dh;
    t_nubit8 cyl    = _ch;
    t_nubit8 sector = _cl;
    if (_dl & 0x80) {
        drive &= 0x7f;
        if (drive || !sector || head >= vhdd.nhead || sector > vhdd.nsector || cyl >= vhdd.ncyl) {
            /* sector not found */
            _ah = 0x04;
            SetBit(_flags, VCPU_FLAG_CF);
            SetHddStatus;
        } else {
            memcpy((void *)vramGetAddr(_es,_bx),
                   (void *)vhddGetAddress(cyl,head,sector), _al * 512);
            _ah = 0x00;
            ClrBit(_flags, VCPU_FLAG_CF);
            SetHddStatus;
        }
    } else {
        if (drive || !sector || head >= vfdd.nhead || sector > vfdd.nsector || cyl >= vfdd.ncyl) {
            /* sector not found */
            _ah = 0x04;
            SetBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
        } else {
            memcpy((void *)vramGetAddr(_es,_bx),
                   (void *)vfddGetAddress(cyl,head,sector), _al * 512);
            _ah = 0x00;
            ClrBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
        }
    }
}
static void INT_13_03_FDD_WriteSector() {
    t_nubit8 drive  = _dl;
    t_nubit8 head   = _dh;
    t_nubit8 cyl    = _ch;
    t_nubit8 sector = _cl;
    if (_dl & 0x80) {
        drive &= 0x7f;
        if (drive || !sector || head >= vhdd.nhead || sector > vhdd.nsector || cyl >= vhdd.ncyl) {
            /* sector not found */
            _ah = 0x04;
            SetBit(_flags, VCPU_FLAG_CF);
            SetHddStatus;
        } else {
            memcpy((void *)vhddGetAddress(cyl,head,sector),
                   (void *)vramGetAddr(_es,_bx), _al * 512);
            _ah = 0x00;
            ClrBit(_flags, VCPU_FLAG_CF);
            SetHddStatus;
        }
    } else {
        if (drive || !sector || head >= vfdd.nhead || sector > vfdd.nsector || cyl >= vfdd.ncyl) {
            /* sector not found */
            _ah = 0x04;
            SetBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
        } else {
            memcpy((void *)vfddGetAddress(cyl,head,sector),
                   (void *)vramGetAddr(_es,_bx), _al * 512);
            _ah = 0x00;
            ClrBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
        }
    }
}
static void INT_13_08_FDD_GetParameter() {

    if (_dl & 0x7f) {
        _ah = 0x01;
        SetBit(_flags, VCPU_FLAG_CF);
        SetFddStatus;
        ClrBit(_flags, VCPU_FLAG_PF);
        ClrBit(_flags, VCPU_FLAG_IF);
        return;
    }
    if (_dl & 0x80) {
        _ah = 0x00;
        _ch = (t_nubit8)vhdd.ncyl - 1;
        _cl = ((t_nubit8)(vhdd.ncyl >> 2) & 0xc0) | ((t_nubit8)vhdd.nsector);
        _dh = (t_nubit8)vhdd.nhead - 1;
        _dl = vramVarByte(0x0040, 0x0075);
        SetHddStatus;
    } else {
        _ah = 0x00;
        _bl = 0x04;
        _ch = (t_nubit8)vfdd.ncyl - 1;
        _cl = (t_nubit8)vfdd.nsector;
        _dh = (t_nubit8)vfdd.nhead - 1;
        _dl = 0x02; /* installed 3 fdd (?) */
        _di = vramVarWord(0x0000, 0x1e * 4);
        _es = vramVarWord(0x0000, 0x1e * 4 + 2);
        ClrBit(_flags, VCPU_FLAG_CF);
        SetFddStatus;
    }
}
static void INT_13_15_FDD_GetDriveType() {
    t_nubit32 count;
    if (_dl & 0x80) {
        if (_dl & 0x7f) {
            _ax = _cx = _dx = 0x00;
            stc;
        } else {
            _ax = 0x0003;
            clc;
            count = (vhdd.ncyl - 1) * vhdd.nhead * vhdd.nsector;
            _cx = (t_nubit16)(count >> 16);
            _dx = (t_nubit16)count;
        }
    } else {
        switch (_dl) {
        case 0x00:
        case 0x01:
            ClrBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
            _ah = 0x01;
            break;
        case 0x02:
        case 0x03:
            ClrBit(_flags, VCPU_FLAG_CF);
            SetFddStatus;
            _ah = 0x00;
            break;
        default:
            break;
        }
    }
}

void INT_13() {
    switch (_ah) {
    case 0x00:
        INT_13_00_FDD_ResetDrive();
        break;
    case 0x01:
        INT_13_01_FDD_GetDiskStatus();
        break;
    case 0x02:
        INT_13_02_FDD_ReadSector();
        break;
    case 0x03:
        INT_13_03_FDD_WriteSector();
        break;
    case 0x08:
        INT_13_08_FDD_GetParameter();
        break;
    case 0x15:
        INT_13_15_FDD_GetDriveType();
        break;
    default:
        break;
    }
}

void qddiskReset() {
    qdbiosInt[0x13] = (t_faddrcc)INT_13;
    qdbiosInt[0xa0] = (t_faddrcc)INT_13_00_FDD_ResetDrive;
    qdbiosInt[0xa1] = (t_faddrcc)INT_13_01_FDD_GetDiskStatus;
    qdbiosInt[0xa2] = (t_faddrcc)INT_13_02_FDD_ReadSector;
    qdbiosInt[0xa3] = (t_faddrcc)INT_13_03_FDD_WriteSector;
    qdbiosInt[0xa8] = (t_faddrcc)INT_13_08_FDD_GetParameter;
    qdbiosInt[0xb5] = (t_faddrcc)INT_13_15_FDD_GetDriveType;
}
