/* Copyright 2012-2014 Neko. */

/* VFDD implements Floppy Disk Drive: 3.5" 1.44MB. */

#include "../utils.h"

#include "vdma.h"

#include "vmachine.h"
#include "vfdd.h"

t_fdd vfdd;

#define IsTrackEnd (vfdd.sector >= (vfdd.nsector + 1))
#define IsCylHalf  (vfdd.head == 0 && IsTrackEnd)
#define IsCylEnd   (vfdd.head == 1 && IsTrackEnd)

t_bool deviceConnectFloppyInsert(const t_strptr fname) {
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image && vfdd.base) {
		count = FREAD((void *) vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
		vfdd.flagexist = True;
		FCLOSE(image);
		return False;
	} else {
		return True;
	}
}

t_bool deviceConnectFloppyRemove(const t_strptr fname) {
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vfdd.flagro)
				count = FWRITE((void *) vfdd.base, sizeof(t_nubit8), vfddGetImageSize, image);
			vfdd.flagexist = False;
			FCLOSE(image);
		} else {
			return True;
		}
	}
	vfdd.flagexist = False;
	MEMSET((void *) vfdd.base, Zero8, vfddGetImageSize);
	return False;
}

void vfddTransRead() {
	if (IsCylEnd) {
		return;
	}
	vlatch.byte = d_nubit8(vfdd.curr);
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 1;
			vfdd.head   = 1;
		}
		vfddSetPointer;
	}
}

void vfddTransWrite() {
	if (IsCylEnd) {
		return;
	}
	d_nubit8(vfdd.curr) = vlatch.byte;
	vfdd.curr++;
	vfdd.count++;
	if (!(vfdd.count % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 1;
			vfdd.head   = 1;
		}
		vfddSetPointer;
	}
}

void vfddFormatTrack(t_nubit8 fillbyte) {
	if (vfdd.cyl >= vfdd.ncyl) {
		return;
	}
	vfdd.head   = 0;
	vfdd.sector = 1;
	vfddSetPointer;
	MEMSET((void *) vfdd.curr, fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.head   = 1;
	vfdd.sector = 1;
	vfddSetPointer;
	MEMSET((void *) vfdd.curr, fillbyte, vfdd.nsector * vfdd.nbyte);
	vfdd.sector = vfdd.nsector;
}

static void init() {
	MEMSET(&vfdd, Zero8, sizeof(t_fdd));
	vfdd.ncyl    = 0x0050;
	vfdd.nhead   = 0x0002;
	vfdd.nsector = 0x0012;
	vfdd.nbyte   = 0x0200;
	vfdd.base    = (t_vaddrcc) MALLOC(vfddGetImageSize);
	MEMSET((void *) vfdd.base, Zero8, vfddGetImageSize);
}

static void reset() {}

static void refresh() {}

static void final() {
	if (vfdd.base) {
		FREE((void *) vfdd.base);
	}
	vfdd.base = (t_vaddrcc) NULL;
}

void vfddRegister() {vmachineAddMe;}

void deviceConnectFloppyCreate() {vfdd.flagexist = True;}
