/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "core/product/utils.h"

#include "core/machine/dma.h"

#include "vm/machine/hdd.h"

static t_hdd *vmMachineHdd;

t_hdd *vm_machine_hdd_current(void) { return vmMachineHdd; }
void vm_machine_hdd_bind_live(t_hdd *hdd) { vmMachineHdd = hdd; }
void vm_machine_hdd_unbind_live(void) { vmMachineHdd = NULL; }

#define IsTrackEnd (vhdd.data.sector >= (vhdd.data.nsector + 1))
#define IsCylEnd   (vhdd.data.head == (vhdd.data.nhead - 1) && IsTrackEnd)

/* allocates space for hard disk */
static void allocate() {
    if (vhdd.connect.pImgBase) {
        FREE((void *) vhdd.connect.pImgBase);
    }
    vhdd.connect.pImgBase = (t_vaddrcc) MALLOC(vhddGetImageSize);
    MEMSET((void *) vhdd.connect.pImgBase, Zero8, vhddGetImageSize);
}

void vhddTransRead() {
    if (IsCylEnd) {
        return;
    }
    vlatch.data.byte = d_nubit8(vhdd.connect.pCurrByte);
    vhdd.connect.pCurrByte++;
    vhdd.connect.transCount++;
    if (!(vhdd.connect.transCount % vhdd.data.nbyte)) {
        vhdd.data.sector++;
        if (IsTrackEnd) {
            vhdd.data.sector = 1;
            vhdd.data.head++;
        }
        vhddSetPointer;
    }
}
void vhddTransWrite() {
    if (IsCylEnd) {
        return;
    }
    d_nubit8(vhdd.connect.pCurrByte) = vlatch.data.byte;
    vhdd.connect.pCurrByte++;
    vhdd.connect.transCount++;
    if (!(vhdd.connect.transCount % vhdd.data.nbyte)) {
        vhdd.data.sector++;
        if (IsTrackEnd) {
            vhdd.data.sector = 1;
            vhdd.data.head++;
        }
        vhddSetPointer;
    }
}
void vhddFormatTrack(t_nubit8 fillbyte) {
    t_nubitcc i;
    if (vhdd.data.cyl >= vhdd.data.ncyl) {
        return;
    }
    for (i = 0; i < vhdd.data.nhead; ++i) {
        vhdd.data.head = GetMax16(i);
        vhdd.data.sector = 1;
        vhddSetPointer;
        MEMSET((void *) vhdd.connect.pCurrByte, fillbyte, vhdd.data.nsector * vhdd.data.nbyte);
        vhdd.data.sector = vhdd.data.nsector;
    }
}

void vhddInit() {
    MEMSET((void *)(&vhdd), Zero8, sizeof(t_hdd));
    vhdd.data.ncyl     = 0;
    vhdd.data.nhead    = 16;
    vhdd.data.nsector  = 63;
    vhdd.data.nbyte    = 512;
    vhdd.connect.pImgBase = (t_vaddrcc) NULL;
}
void vhddReset() {
    t_nubit16 oldNumCyl = vhdd.data.ncyl;
    MEMSET((void *)(&vhdd.data), Zero8, sizeof(t_hdd_data));
    vhdd.data.ncyl = oldNumCyl;
    vhdd.data.nhead    = 16;
    vhdd.data.nsector  = 63;
    vhdd.data.nbyte    = 512;
}
void vhddRefresh() {}
void vhddFinal() {
    if (vhdd.connect.pImgBase) {
        FREE((void *) vhdd.connect.pImgBase);
    }
    vhdd.connect.pImgBase = (t_vaddrcc) NULL;
}

void vm_machine_hdd_create(uint16_t cylinders) {
    vhdd.data.ncyl = cylinders;
    allocate();
    vhdd.connect.flagDiskExist = True;
}
int vm_machine_hdd_insert(const char *file_name) {
    t_nubitcc count;
    FILE *image = FOPEN(file_name, "rb");
    if (image) {
        fseek(image, Zero32, SEEK_END);
        count = ftell(image);
        vhdd.data.ncyl = (t_nubit16)(count / vhdd.data.nhead / vhdd.data.nsector / vhdd.data.nbyte);
        fseek(image, Zero32, SEEK_SET);
        allocate();
        count = FREAD((void *) vhdd.connect.pImgBase, sizeof(t_nubit8), vhddGetImageSize, image);
        vhdd.connect.flagDiskExist = True;
        FCLOSE(image);
        return False;
    } else {
        return True;
    }
}
int vm_machine_hdd_remove(const char *file_name) {
    t_nubitcc count;
    FILE *image;
    if (file_name) {
        image = FOPEN(file_name, "wb");
        if (image) {
            if (!vhdd.connect.flagReadOnly)
                count = FWRITE((void *) vhdd.connect.pImgBase, sizeof(t_nubit8), vhddGetImageSize, image);
            vhdd.connect.flagDiskExist = False;
            FCLOSE(image);
        } else {
            return True;
        }
    }
    vhdd.connect.flagDiskExist = False;
    MEMSET((void *) vhdd.connect.pImgBase, Zero8, vhddGetImageSize);
    return False;
}
