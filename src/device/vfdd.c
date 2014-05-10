/* Copyright 2012-2014 Neko. */

/* VFDD implements Floppy Disk Drive: 3.5" 1.44MB. */

#include "../utils.h"

#include "vdma.h"

#include "vfdd.h"

t_fdd vfdd;

#define IsTrackEnd (vfdd.data.sector >= (vfdd.data.nsector + 1))
#define IsCylHalf  (vfdd.data.head == 0 && IsTrackEnd)
#define IsCylEnd   (vfdd.data.head == 1 && IsTrackEnd)

void vfddTransRead() {
    if (IsCylEnd) {
        return;
    }
    vlatch.data.byte = d_nubit8(vfdd.connect.pCurrByte);
    vfdd.connect.pCurrByte++;
    vfdd.connect.transCount++;
    if (!(vfdd.connect.transCount % vfdd.data.nbyte)) {
        vfdd.data.sector++;
        if (IsCylHalf) {
            vfdd.data.sector = 1;
            vfdd.data.head   = 1;
        }
        vfddSetPointer;
    }
}
void vfddTransWrite() {
    if (IsCylEnd) {
        return;
    }
    d_nubit8(vfdd.connect.pCurrByte) = vlatch.data.byte;
    vfdd.connect.pCurrByte++;
    vfdd.connect.transCount++;
    if (!(vfdd.connect.transCount % vfdd.data.nbyte)) {
        vfdd.data.sector++;
        if (IsCylHalf) {
            vfdd.data.sector = 1;
            vfdd.data.head   = 1;
        }
        vfddSetPointer;
    }
}
void vfddFormatTrack(t_nubit8 fillByte) {
    if (vfdd.data.cyl >= vfdd.data.ncyl) {
        return;
    }
    vfdd.data.head   = 0;
    vfdd.data.sector = 1;
    vfddSetPointer;
    MEMSET((void *) vfdd.connect.pCurrByte, fillByte, vfdd.data.nsector * vfdd.data.nbyte);
    vfdd.data.head   = 1;
    vfdd.data.sector = 1;
    vfddSetPointer;
    MEMSET((void *) vfdd.connect.pCurrByte, fillByte, vfdd.data.nsector * vfdd.data.nbyte);
    vfdd.data.sector = vfdd.data.nsector;
}

void vfddInit() {
    MEMSET((void *)(&vfdd), Zero8, sizeof(t_fdd));
    vfdd.data.ncyl    = 0x0050;
    vfdd.data.nhead   = 0x0002;
    vfdd.data.nsector = 0x0012;
    vfdd.data.nbyte   = 0x0200;
    vfdd.connect.pImgBase = (t_vaddrcc) MALLOC(vfddGetImageSize);
    MEMSET((void *) vfdd.connect.pImgBase, Zero8, vfddGetImageSize);
}
void vfddReset() {
    MEMSET((void *)(&vfdd.data), Zero8, sizeof(t_fdd_data));
    vfdd.data.ncyl    = 0x0050;
    vfdd.data.nhead   = 0x0002;
    vfdd.data.nsector = 0x0012;
    vfdd.data.nbyte   = 0x0200;
}
void vfddRefresh() {}
void vfddFinal() {
    if (vfdd.connect.pImgBase) {
        FREE((void *) vfdd.connect.pImgBase);
    }
    vfdd.connect.pImgBase = (t_vaddrcc) NULL;
}

void deviceConnectFloppyCreate() {
    vfdd.connect.flagDiskExist = True;
}
int deviceConnectFloppyInsert(const char *fileName) {
    FILE *fpImage = FOPEN(fileName, "rb");
    if (fpImage && vfdd.connect.pImgBase) {
        FREAD((void *) vfdd.connect.pImgBase, sizeof(t_nubit8), vfddGetImageSize, fpImage);
        vfdd.connect.flagDiskExist = True;
        FCLOSE(fpImage);
        return False;
    } else {
        return True;
    }
}
int deviceConnectFloppyRemove(const char *fileName) {
    FILE *fpImage;
    if (fileName) {
        fpImage = FOPEN(fileName, "wb");
        if (fpImage) {
            if (!vfdd.connect.flagReadOnly) {
                FWRITE((void *) vfdd.connect.pImgBase, sizeof(t_nubit8), vfddGetImageSize, fpImage);
            }
            vfdd.connect.flagDiskExist = False;
            FCLOSE(fpImage);
        } else {
            return True;
        }
    }
    vfdd.connect.flagDiskExist = False;
    MEMSET((void *) vfdd.connect.pImgBase, Zero8, vfddGetImageSize);
    return False;
}

void devicePrintFdd() {
    PRINTF("FDD INFO\n========\n");
    PRINTF("cyl = %x, head = %x, sector = %x, gpl = %x\n",
           vfdd.data.cyl, vfdd.data.head, vfdd.data.sector, vfdd.data.gpl);
    PRINTF("nsector = %x, nbyte = %x, ncyl = %x, nhead = %x\n",
           vfdd.data.nsector, vfdd.data.nbyte, vfdd.data.ncyl,
           vfdd.data.nhead);
    PRINTF("ReadOnly = %x, Exist = %x\n",
           vfdd.connect.flagReadOnly, vfdd.connect.flagDiskExist);
    PRINTF("base = %x, curr = %x, count = %x\n",
           vfdd.connect.pImgBase, vfdd.connect.pCurrByte, vfdd.connect.transCount);
}
