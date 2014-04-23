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

#define QDBIOS_VAR_VFDD  0
#define QDBIOS_VAR_QDFDD 1
/* debugging selectors */
#define QDBIOS_FDD QDBIOS_VAR_VFDD

#if (QDBIOS_FDD == QDBIOS_VAR_VFDD)
static void INT_13_00_FDD_ResetDrive()
{
	if (cmp(_dl, 0x00) || cmp(_dl, 0x01)) {
		mov(_ah, 0x00);
		clc;
	} else {
		/* only one drive */
		mov(_ah, 0x0c);
		stc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_01_FDD_GetDiskStatus()
{
	mov(_ah, mbp(0x0441)); /* get status */
}
static void INT_13_02_FDD_ReadSector()
{
/*	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;
	t_nubit8 nsec   = _al;*/
	if (_dl || _dh > 1 || _cl > 18 || _ch > 79) {
		/* sector not found */
		mov(_ah, 0x04);
		stc;
	} else {
//t_vaddrcc ifddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
//{return (vfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}
//		memcpy((void *)vramGetAddr(_es,_bx),
//			(void *)ifddGetAddress(cyl,head,sector), nsec * 512);
		/* set dma */
		push_bx;
		push_cx;
		push_dx;
		push_ax;
		out(0x000b, 0x86); /* set dma mode: block, inc, write, chn2 */
		mov(_cx, _bx);
		mov(_dx, _es);
		shr(_cx, 0x04);
		add(_dx, _cx);
		mov(_ax, _dx);
		shl(_ax, 0x04);
		mov(_cx, _bx);
		and(_cx, 0x000f);
		or(_ax, _cx);      /* calc base addr */
		shr(_dx, 0x0c);    /* calc page register */
		out(0x0004, _al);  /* set addr low byte */
		out(0x0004, _ah);  /* set addr high byte */
		out(0x0081, _dl);  /* set page register */
		pop_ax;
		pop_dx;
		pop_cx;
		pop_bx;
		mov(_ah, 0x00);    /* clear ah for wc */
		shl(_ax, 0x09);    /* calc wc from al */
		dec(_ax);
		out(0x0005, _al);  /* set wc low byte */
		out(0x0005, _ah);  /* set wc high byte */
		out(0x000a, 0x02); /* unmask dma1.chn2 */
		out(0x00d4, 0x00); /* unmask dma2.chn0 */
		/* set fdc */
		out(0x03f5, 0x0f); /* send seek command */
		shl(_dh, 0x02);    /* calc hds byte */
		or (_dl, _dh);     /* calc hds byte */
		shr(_dh, 0x02);    /* calc hds byte */
		out(0x03f5, _dl);  /* set seek hds byte */
		out(0x03f5, _ch);  /* set seek cyl */
/* Note: here vfdc sets IRQ6 and vcpu calls INT E */
		//vcpuinsExecInt();
		vpic1.isr |=  0x40;
		vpic1.irr &= ~0x40;
		INT_0E();
		do {
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready write status */
		} while(!cmp(_al, 0x80));
		out(0x03f5, 0xe6); /* send read command */
		out(0x03f5, _dl);  /* set stdi hds */
		out(0x03f5, _ch);  /* set stdi cyl */
		out(0x03f5, _dh);  /* set stdi head */
		out(0x03f5, _cl);  /* set stdi start sector id */
		out(0x03f5, 0x02); /* set stdi sector size code (512) */
		out(0x03f5, 0x12); /* set stdi end sector id */
		out(0x03f5, 0x1b); /* set stdi gap length */
		out(0x03f5, 0xff); /* set stdi customized sect size (0xff) */
		/* now everything is ready; DRQ also generated. */
		vdmaRefresh();
		/* vdma send eop to vfdc, vfdc set IRQ 6 */
		vpic1.isr |=  0x40;
		vpic1.irr &= ~0x40;
		INT_0E();
		do {
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready read status */
		} while(!cmp(_al, 0xc0));
		in(_al, 0x03f5); /* get stdo st0 */
		in(_al, 0x03f5); /* get stdo st1 */
		in(_al, 0x03f5); /* get stdo st2 */
		in(_al, 0x03f5); /* get stdo cyl */
		in(_al, 0x03f5); /* get stdo head */
		in(_al, 0x03f5); /* get stdo sector */
		in(_al, 0x03f5); /* get stdo sector size code */
		mov(_ah, 0x00);
		clc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
	//vapiPrint("read: drv=%x, head=%x, cyl=%x, sec=%x, es=%x, bx=%x\n",_dl,_dh,_ch,_cl,_es,_bx);
	//vmachineStop();
}
static void INT_13_03_FDD_WriteSector()
{
/*	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;*/
	if (_dl || _dh > 1 || _cl > 18 || _ch > 79) {
		/* sector not found */
		mov(_ah, 0x04);
		stc;
	} else {
//t_vaddrcc ifddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
//{return (vfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}
//		memcpy((void *)ifddGetAddress(cyl,head,sector),
//			(void *)vramGetAddr(_es,_bx), _al * 512);
		/* set dma */
		push_bx;
		push_cx;
		push_dx;
		push_ax;
		out(0x000b, 0x8a); /* set dma mode: block, inc, read, chn2 */
		mov(_cx, _bx);
		mov(_dx, _es);
		shr(_cx, 0x04);
		add(_dx, _cx);
		mov(_ax, _dx);
		shl(_ax, 0x04);
		mov(_cx, _bx);
		and(_cx, 0x000f);
		or(_ax, _cx);      /* calc base addr */
		shr(_dx, 0x0c);    /* calc page register */
		out(0x0004, _al);  /* set addr low byte */
		out(0x0004, _ah);  /* set addr high byte */
		out(0x0081, _dl);  /* set page register */
		pop_ax;
		pop_dx;
		pop_cx;
		pop_bx;
		mov(_ah, 0x00);    /* clear ah for wc */
		shl(_ax, 0x09);    /* calc wc from al */
		dec(_ax);
		out(0x0005, _al);  /* set wc low byte */
		out(0x0005, _ah);  /* set wc high byte */
		out(0x000a, 0x02); /* unmask dma1.chn2 */
		out(0x00d4, 0x00); /* unmask dma2.chn0 */
		/* set fdc */
		out(0x03f5, 0x0f); /* send seek command */
		shl(_dh, 0x02);    /* calc hds byte */
		or (_dl, _dh);      /* calc hds byte */
		shr(_dh, 0x02);    /* calc hds byte */
		out(0x03f5, _dl);  /* set seek hds byte */
		out(0x03f5, _ch);  /* set seek cyl */
/* Note: here vfdc set IRQ6 and vcpu calls INT E */
		vpic1.isr |=  0x40;
		vpic1.irr &= ~0x40;
		INT_0E();
		do {
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready write status */
		} while(!cmp(_al, 0x80));
		out(0x03f5, 0xc5); /* send write command */
		out(0x03f5, _dl);  /* set stdi hds */
		out(0x03f5, _ch);  /* set stdi cyl */
		out(0x03f5, _dh);  /* set stdi head */
		out(0x03f5, _cl);  /* set stdi start sector id */
		out(0x03f5, 0x02); /* set stdi sector size code (512) */
		out(0x03f5, 0x12); /* set stdi end sector id */
		out(0x03f5, 0x1b); /* set stdi gap length */
		out(0x03f5, 0xff); /* set stdi customized sect size (0xff) */
		/* now everything is ready; DRQ also generated. */
		vdmaRefresh();
		/* vdma send eop to vfdc, vfdc set IRQ 6 */
		vpic1.isr |=  0x40;
		vpic1.irr &= ~0x40;
		INT_0E();
		do {
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready read status */
		} while(!cmp(_al, 0xc0));
		in(_al, 0x03f5); /* get stdo st0 */
		in(_al, 0x03f5); /* get stdo st1 */
		in(_al, 0x03f5); /* get stdo st2 */
		in(_al, 0x03f5); /* get stdo cyl */
		in(_al, 0x03f5); /* get stdo head */
		in(_al, 0x03f5); /* get stdo sector */
		in(_al, 0x03f5); /* get stdo sector size code */
		mov(_ah, 0x00);
		clc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_08_FDD_GetParameter()
{
	if (_dl == 0x80) {
		mov(_ah, 0x01);
		stc;
		clp;
		cli;
	} else {
		mov(_ah, 0x00);
		mov(_bl, 0x04);
		mov(_cx, 0x4f12);
		mov(_dx, 0x0102);
		mov(_di, mwp(0x1e * 4 + 0));
		mov(_es, mwp(0x1e * 4 + 2));
		clc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_15_FDD_GetDriveType()
{
	switch (_dl) {
	case 0x00:
	case 0x01:
		mov(_ah, 0x01);
		clc;
		break;
	case 0x02:
	case 0x03:
		mov(_ah, 0x00);
		stc;
		break;
	default:
		break;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
#else
#define SetFddStatus (vramVarByte(0x0040, 0x0041) = _ah)
#define GetFddStatus (vramVarByte(0x0040, 0x0041))
#define SetHddStatus (vramVarByte(0x0040, 0x0074) = _ah)
#define GetHddStatus (vramVarByte(0x0040, 0x0074))
t_vaddrcc vfddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
{
	vfdd.cyl = cyl;
	vfdd.head = head;
	vfdd.sector = sector;
	vfddSetPointer;
	return vfdd.curr;
}
t_vaddrcc vhddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
{
	vhdd.cyl = cyl;
	vhdd.head = head;
	vhdd.sector = sector;
	vhddSetPointer;
	return vhdd.curr;
}
static void INT_13_00_FDD_ResetDrive()
{
	_ah = 0x00;
	ClrBit(_flags, VCPU_FLAG_CF);
	SetFddStatus;
	SetHddStatus;
}
static void INT_13_01_FDD_GetDiskStatus()
{
	if (_dl & 0x80) {/* Hard Disk */
		_ah = 0x80;
		SetHddStatus;
	} else {
		_ah = 0x00;
		SetFddStatus;
	}
	SetFddStatus;
}
static void INT_13_02_FDD_ReadSector()
{
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
static void INT_13_03_FDD_WriteSector()
{
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
static void INT_13_08_FDD_GetParameter()
{

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
static void INT_13_15_FDD_GetDriveType()
{
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
#endif

void INT_0E()
{
	push_ax;
	push_dx;
	in(_al, 0x03f4); /* get msr */
	and(_al, 0xc0);  /* get ready write status */
	if(cmp(_al, 0x80)) {
		mov(_dx, 0x03f5);
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
void INT_13()
{
	switch (_ah) {
	case 0x00: INT_13_00_FDD_ResetDrive();   break;
	case 0x01: INT_13_01_FDD_GetDiskStatus();break;
	case 0x02: INT_13_02_FDD_ReadSector();   break;
	case 0x03: INT_13_03_FDD_WriteSector();  break;
	case 0x08: INT_13_08_FDD_GetParameter(); break;
	case 0x15: INT_13_15_FDD_GetDriveType(); break;
	default:   break;
	}
}

void qddiskReset()
{
	qdbiosInt[0x0e] = (t_faddrcc)INT_0E; /* hard fdd */
	qdbiosInt[0x13] = (t_faddrcc)INT_13; /* soft fdd */
/* special: INT 0E */
	qdbiosMakeInt(0x09, "qdx 0e\niret");
/* special: INT 13 */
	qdbiosMakeInt(0x13, "qdx 13\niret");
	if (vramVarByte(0x0040, 0x0100) & 0x80) {
		if (!vhdd.flagexist) {
			vapiPrint("Insert boot disk and restart.\n");
			return;
		} else {
			memcpy((void *)vramGetAddr(0x0000,0x7c00), (void *)vhdd.base, 0x200);
			mov(_ax, 0xaa55);
		}
	} else {
		if (!vfdd.flagexist) {
			vapiPrint("Insert boot disk and restart.\n");
			return;
		} else {
			memcpy((void *)vramGetAddr(0x0000,0x7c00), (void *)vfdd.base, 0x200);
			mov(_ax, 0xaa55);
		}
	}
	mov(_cx, 0x0001);
	mov(_sp, 0xfffe);
}
