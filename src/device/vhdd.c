/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "../utils.h"

#include "vdma.h"

#include "vmachine.h"
#include "vhdd.h"

t_hdd vhdd;

#define IsTrackEnd (vhdd.sector >= (vhdd.nsector + 1))
#define IsCylEnd   (vhdd.head == (vhdd.nhead - 1) && IsTrackEnd)

/* allocates space for hard disk */
static void allocate() {
	if (vhdd.pImgBase) {
		FREE((void *) vhdd.pImgBase);
	}
	vhdd.pImgBase = (t_vaddrcc) MALLOC(vhddGetImageSize);
	MEMSET((void *) vhdd.pImgBase, Zero8, vhddGetImageSize);
}

void deviceConnectHardDiskCreate(t_nubit16 ncyl) {
	vhdd.ncyl = ncyl;
	allocate();
	vhdd.flagDiskExist = True;
}

t_bool deviceConnectHardDiskInsert(const t_strptr fname) {
	t_nubitcc count;
	FILE *image = FOPEN(fname, "rb");
	if (image) {
		fseek(image, Zero32, SEEK_END);
		count = ftell(image);
		vhdd.ncyl = (t_nubit16)(count / vhdd.nhead / vhdd.nsector / vhdd.nbyte);
		fseek(image, Zero32, SEEK_SET);
		allocate();
		count = FREAD((void *) vhdd.pImgBase, sizeof(t_nubit8), vhddGetImageSize, image);
		vhdd.flagDiskExist = True;
		FCLOSE(image);
		return False;
	} else {
		return True;
	}
}

t_bool deviceConnectHardDiskRemove(const t_strptr fname) {
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = FOPEN(fname, "wb");
		if(image) {
			if (!vhdd.flagReadOnly)
				count = FWRITE((void *) vhdd.pImgBase, sizeof(t_nubit8), vhddGetImageSize, image);
			vhdd.flagDiskExist = False;
			FCLOSE(image);
		} else {
			return True;
		}
	}
	vhdd.flagDiskExist = False;
	MEMSET((void *) vhdd.pImgBase, Zero8, vhddGetImageSize);
	return False;
}

void vhddTransRead() {
	if (IsCylEnd) {
		return;
	}
	vlatch.byte = d_nubit8(vhdd.pCurrByte);
	vhdd.pCurrByte++;
	vhdd.transCount++;
	if (!(vhdd.transCount % vhdd.nbyte)) {
		vhdd.sector++;
		if (IsTrackEnd) {
			vhdd.sector = 1;
			vhdd.head++;
		}
		vhddSetPointer;
	}
}

void vhddTransWrite() {
	if (IsCylEnd) {
		return;
	}
	d_nubit8(vhdd.pCurrByte) = vlatch.byte;
	vhdd.pCurrByte++;
	vhdd.transCount++;
	if (!(vhdd.transCount % vhdd.nbyte)) {
		vhdd.sector++;
		if (IsTrackEnd) {
			vhdd.sector = 1;
			vhdd.head++;
		}
		vhddSetPointer;
	}
}

void vhddFormatTrack(t_nubit8 fillbyte) {
	t_nubit8 i;
	if (vhdd.cyl >= vhdd.ncyl) {
		return;
	}
	for (i = 0;i < vhdd.nhead;++i) {
		vhdd.head = i;
		vhdd.sector = 1;
		vhddSetPointer;
		MEMSET((void *) vhdd.pCurrByte, fillbyte, vhdd.nsector * vhdd.nbyte);
		vhdd.sector = vhdd.nsector;
	}
}

static void init() {
	MEMSET(&vhdd, Zero8, sizeof(t_hdd));
	vhdd.ncyl     = 0;
	vhdd.nhead    = 16;
	vhdd.nsector  = 63;
	vhdd.nbyte    = 512;
	vhdd.pImgBase = (t_vaddrcc) NULL;
}

static void reset() {}

static void refresh() {}

static void final() {
	if (vhdd.pImgBase) {
		FREE((void *) vhdd.pImgBase);
	}
	vhdd.pImgBase = (t_vaddrcc) NULL;
}

void vhddRegister() {vmachineAddMe;}
