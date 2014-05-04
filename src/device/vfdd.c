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

t_bool deviceConnectFloppyInsert(const t_strptr fileName) {
	FILE *fpImage = FOPEN(fileName, "rb");
	if (fpImage && vfdd.pImgBase) {
		FREAD((void *) vfdd.pImgBase, sizeof(t_nubit8), vfddGetImageSize, fpImage);
		vfdd.flagDiskExist = True;
		FCLOSE(fpImage);
		return False;
	} else {
		return True;
	}
}

t_bool deviceConnectFloppyRemove(const t_strptr fileName) {
	FILE *fpImage;
	if (fileName) {
		fpImage = FOPEN(fileName, "wb");
		if(fpImage) {
			if (!vfdd.flagReadOnly) {
				FWRITE((void *) vfdd.pImgBase, sizeof(t_nubit8), vfddGetImageSize, fpImage);
			}
			vfdd.flagDiskExist = False;
			FCLOSE(fpImage);
		} else {
			return True;
		}
	}
	vfdd.flagDiskExist = False;
	MEMSET((void *) vfdd.pImgBase, Zero8, vfddGetImageSize);
	return False;
}

void vfddTransRead() {
	if (IsCylEnd) {
		return;
	}
	vlatch.byte = d_nubit8(vfdd.pCurrByte);
	vfdd.pCurrByte++;
	vfdd.transCount++;
	if (!(vfdd.transCount % vfdd.nbyte)) {
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
	d_nubit8(vfdd.pCurrByte) = vlatch.byte;
	vfdd.pCurrByte++;
	vfdd.transCount++;
	if (!(vfdd.transCount % vfdd.nbyte)) {
		vfdd.sector++;
		if (IsCylHalf) {
			vfdd.sector = 1;
			vfdd.head   = 1;
		}
		vfddSetPointer;
	}
}

void vfddFormatTrack(t_nubit8 fillByte) {
	if (vfdd.cyl >= vfdd.ncyl) {
		return;
	}
	vfdd.head   = 0;
	vfdd.sector = 1;
	vfddSetPointer;
	MEMSET((void *) vfdd.pCurrByte, fillByte, vfdd.nsector * vfdd.nbyte);
	vfdd.head   = 1;
	vfdd.sector = 1;
	vfddSetPointer;
	MEMSET((void *) vfdd.pCurrByte, fillByte, vfdd.nsector * vfdd.nbyte);
	vfdd.sector = vfdd.nsector;
}

static void init() {
	MEMSET(&vfdd, Zero8, sizeof(t_fdd));
	vfdd.ncyl    = 0x0050;
	vfdd.nhead   = 0x0002;
	vfdd.nsector = 0x0012;
	vfdd.nbyte   = 0x0200;
	vfdd.pImgBase    = (t_vaddrcc) MALLOC(vfddGetImageSize);
	MEMSET((void *) vfdd.pImgBase, Zero8, vfddGetImageSize);
}

static void reset() {}

static void refresh() {}

static void final() {
	if (vfdd.pImgBase) {
		FREE((void *) vfdd.pImgBase);
	}
	vfdd.pImgBase = (t_vaddrcc) NULL;
}

void vfddRegister() {vmachineAddMe;}

void deviceConnectFloppyCreate() {vfdd.flagDiskExist = True;}

void devicePrintFdd() {
	PRINTF("FDD INFO\n========\n");
	PRINTF("cyl = %x, head = %x, sector = %x\n",
		vfdd.cyl, vfdd.head, vfdd.sector);
	PRINTF("nsector = %x, nbyte = %x, gpl = %x\n",
		vfdd.nsector, vfdd.nbyte, vfdd.gpl);
	PRINTF("ReadOnly = %x, Exist = %x\n",
		vfdd.flagReadOnly, vfdd.flagDiskExist);
	PRINTF("base = %x, curr = %x, count = %x\n",
		vfdd.pImgBase, vfdd.pCurrByte, vfdd.transCount);
}
