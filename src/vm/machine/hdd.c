/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "core/product/utils.h"

#include "core/machine/dma.h"

#include "vm/machine/hdd.h"

static int vm_machine_hdd_track_end(const t_hdd *hdd) {
    return hdd->data.sector >= hdd->data.nsector + 1;
}

static int vm_machine_hdd_cylinder_end(const t_hdd *hdd) {
    return hdd->data.head == hdd->data.nhead - 1 &&
        vm_machine_hdd_track_end(hdd);
}

size_t vm_machine_hdd_image_size(const t_hdd *hdd) {
    return (size_t)hdd->data.nbyte * hdd->data.nsector * hdd->data.nhead *
        hdd->data.ncyl;
}

void vm_machine_hdd_set_pointer(t_hdd *hdd) {
    hdd->connect.pCurrByte = hdd->connect.pImgBase +
        ((hdd->data.cyl * hdd->data.nhead + hdd->data.head) *
        hdd->data.nsector + (hdd->data.sector - 1)) * hdd->data.nbyte;
}

static void vm_machine_hdd_allocate(t_hdd *hdd) {
    size_t image_size = vm_machine_hdd_image_size(hdd);
    if (hdd->connect.pImgBase) {
        FREE((void *)hdd->connect.pImgBase);
    }
    hdd->connect.pImgBase = (t_vaddrcc)MALLOC(image_size);
    MEMSET((void *)hdd->connect.pImgBase, Zero8, image_size);
}

void vm_machine_hdd_transfer_read(t_hdd *hdd, t_latch *latch) {
    if (hdd == NULL || latch == NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    latch->data.byte = d_nubit8(hdd->connect.pCurrByte);
    hdd->connect.pCurrByte++;
    hdd->connect.transCount++;
    if (!(hdd->connect.transCount % hdd->data.nbyte)) {
        hdd->data.sector++;
        if (vm_machine_hdd_track_end(hdd)) {
            hdd->data.sector = 1;
            hdd->data.head++;
        }
        vm_machine_hdd_set_pointer(hdd);
    }
}
void vm_machine_hdd_transfer_write(t_hdd *hdd, t_latch *latch) {
    if (hdd == NULL || latch == NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    d_nubit8(hdd->connect.pCurrByte) = latch->data.byte;
    hdd->connect.pCurrByte++;
    hdd->connect.transCount++;
    if (!(hdd->connect.transCount % hdd->data.nbyte)) {
        hdd->data.sector++;
        if (vm_machine_hdd_track_end(hdd)) {
            hdd->data.sector = 1;
            hdd->data.head++;
        }
        vm_machine_hdd_set_pointer(hdd);
    }
}
void vm_machine_hdd_format_track(t_hdd *hdd, t_nubit8 fill_byte) {
    t_nubitcc i;
    if (hdd == NULL || hdd->data.cyl >= hdd->data.ncyl) {
        return;
    }
    for (i = 0; i < hdd->data.nhead; ++i) {
        hdd->data.head = GetMax16(i);
        hdd->data.sector = 1;
        vm_machine_hdd_set_pointer(hdd);
        MEMSET((void *)hdd->connect.pCurrByte, fill_byte,
            hdd->data.nsector * hdd->data.nbyte);
        hdd->data.sector = hdd->data.nsector;
    }
}

void vm_machine_hdd_initialize(t_hdd *hdd) {
    if (hdd == NULL) return;
    MEMSET((void *)hdd, Zero8, sizeof(*hdd));
    hdd->data.ncyl = 0;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
void vm_machine_hdd_reset(t_hdd *hdd) {
    t_nubit16 old_cylinders;
    if (hdd == NULL) return;
    old_cylinders = hdd->data.ncyl;
    MEMSET((void *)&hdd->data, Zero8, sizeof(hdd->data));
    hdd->data.ncyl = old_cylinders;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
void vm_machine_hdd_refresh(t_hdd *hdd) { (void)hdd; }
void vm_machine_hdd_finalize(t_hdd *hdd) {
    if (hdd != NULL && hdd->connect.pImgBase) {
        FREE((void *)hdd->connect.pImgBase);
    }
    if (hdd != NULL) hdd->connect.pImgBase = (t_vaddrcc)NULL;
}

void vm_machine_hdd_create(t_hdd *hdd, uint16_t cylinders) {
    if (hdd == NULL) return;
    hdd->data.ncyl = cylinders;
    vm_machine_hdd_allocate(hdd);
    hdd->connect.flagDiskExist = True;
}
int vm_machine_hdd_insert(t_hdd *hdd, const char *file_name) {
    t_nubitcc count;
    FILE *image = FOPEN(file_name, "rb");
    if (hdd != NULL && image) {
        fseek(image, Zero32, SEEK_END);
        count = ftell(image);
        hdd->data.ncyl = (t_nubit16)(count / hdd->data.nhead /
            hdd->data.nsector / hdd->data.nbyte);
        fseek(image, Zero32, SEEK_SET);
        vm_machine_hdd_allocate(hdd);
        count = FREAD((void *)hdd->connect.pImgBase, sizeof(t_nubit8),
            vm_machine_hdd_image_size(hdd), image);
        hdd->connect.flagDiskExist = True;
        FCLOSE(image);
        return False;
    } else {
        return True;
    }
}
int vm_machine_hdd_remove(t_hdd *hdd, const char *file_name) {
    t_nubitcc count;
    FILE *image;
    if (hdd == NULL) return True;
    if (file_name) {
        image = FOPEN(file_name, "wb");
        if (image) {
            if (!hdd->connect.flagReadOnly)
                count = FWRITE((void *)hdd->connect.pImgBase, sizeof(t_nubit8),
                    vm_machine_hdd_image_size(hdd), image);
            hdd->connect.flagDiskExist = False;
            FCLOSE(image);
        } else {
            return True;
        }
    }
    hdd->connect.flagDiskExist = False;
    MEMSET((void *)hdd->connect.pImgBase, Zero8, vm_machine_hdd_image_size(hdd));
    return False;
}
