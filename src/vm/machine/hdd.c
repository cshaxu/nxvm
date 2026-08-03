/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/dma.h"


#include "vm/machine/hdd.h"

static C_INT vm_machine_hdd_track_end(const t_hdd *hdd) {
    return hdd->data.sector >= hdd->data.nsector + 1;
}

static C_INT vm_machine_hdd_cylinder_end(const t_hdd *hdd) {
    return hdd->data.head == hdd->data.nhead - 1 &&
        vm_machine_hdd_track_end(hdd);
}

size_t vm_machine_hdd_image_size(const t_hdd *hdd) {
    return (size_t)hdd->data.nbyte * hdd->data.nsector * hdd->data.nhead *
        hdd->data.ncyl;
}

C_VOID vm_machine_hdd_set_pointer(t_hdd *hdd) {
    hdd->connect.pCurrByte = hdd->connect.pImgBase +
        ((hdd->data.cyl * hdd->data.nhead + hdd->data.head) *
        hdd->data.nsector + (hdd->data.sector - 1)) * hdd->data.nbyte;
}

static C_VOID vm_machine_hdd_allocate(t_hdd *hdd) {
    size_t image_size = vm_machine_hdd_image_size(hdd);
    if (hdd->connect.pImgBase) {
        STD_FREE((C_VOID *)hdd->connect.pImgBase);
    }
    hdd->connect.pImgBase = (ntvdm64_type_virtual_address)STD_MALLOC(image_size);
    STD_MEMSET((C_VOID *)hdd->connect.pImgBase, NTVDM64_TYPE_ZERO_8, image_size);
}

C_VOID vm_machine_hdd_transfer_read(t_hdd *hdd, t_latch *latch) {
    if (hdd == NULL || latch == NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    latch->data.byte = NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(hdd->connect.pCurrByte);
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
C_VOID vm_machine_hdd_transfer_write(t_hdd *hdd, t_latch *latch) {
    if (hdd == NULL || latch == NULL || vm_machine_hdd_cylinder_end(hdd)) {
        return;
    }
    NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(hdd->connect.pCurrByte) = latch->data.byte;
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
C_VOID vm_machine_hdd_format_track(t_hdd *hdd, ntvdm64_type_unsigned_8 fill_byte) {
    ntvdm64_type_native_unsigned i;
    if (hdd == NULL || hdd->data.cyl >= hdd->data.ncyl) {
        return;
    }
    for (i = 0; i < hdd->data.nhead; ++i) {
        hdd->data.head = NTVDM64_TYPE_MASK_UNSIGNED_16(i);
        hdd->data.sector = 1;
        vm_machine_hdd_set_pointer(hdd);
        STD_MEMSET((C_VOID *)hdd->connect.pCurrByte, fill_byte,
            hdd->data.nsector * hdd->data.nbyte);
        hdd->data.sector = hdd->data.nsector;
    }
}

C_VOID vm_machine_hdd_initialize(t_hdd *hdd) {
    if (hdd == NULL) return;
    STD_MEMSET((C_VOID *)hdd, NTVDM64_TYPE_ZERO_8, sizeof(*hdd));
    hdd->data.ncyl = 0;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
C_VOID vm_machine_hdd_reset(t_hdd *hdd) {
    ntvdm64_type_unsigned_16 old_cylinders;
    if (hdd == NULL) return;
    old_cylinders = hdd->data.ncyl;
    STD_MEMSET((C_VOID *)&hdd->data, NTVDM64_TYPE_ZERO_8, sizeof(hdd->data));
    hdd->data.ncyl = old_cylinders;
    hdd->data.nhead = 16;
    hdd->data.nsector = 63;
    hdd->data.nbyte = 512;
}
C_VOID vm_machine_hdd_refresh(t_hdd *hdd) { (C_VOID)hdd; }
C_VOID vm_machine_hdd_finalize(t_hdd *hdd) {
    if (hdd != NULL && hdd->connect.pImgBase) {
        STD_FREE((C_VOID *)hdd->connect.pImgBase);
    }
    if (hdd != NULL) hdd->connect.pImgBase = (ntvdm64_type_virtual_address)NULL;
}

C_VOID vm_machine_hdd_create(t_hdd *hdd, uint16_t cylinders) {
    if (hdd == NULL) return;
    hdd->data.ncyl = cylinders;
    vm_machine_hdd_allocate(hdd);
    hdd->connect.flagDiskExist = NTVDM64_TYPE_TRUE;
}
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name) {
    ntvdm64_type_native_unsigned count;
    FILE *image = STD_FOPEN(file_name, "rb");
    if (hdd != NULL && image) {
        STD_FSEEK(image, NTVDM64_TYPE_ZERO_32, SEEK_END);
        count = STD_FTELL(image);
        hdd->data.ncyl = (ntvdm64_type_unsigned_16)(count / hdd->data.nhead /
            hdd->data.nsector / hdd->data.nbyte);
        STD_FSEEK(image, NTVDM64_TYPE_ZERO_32, SEEK_SET);
        vm_machine_hdd_allocate(hdd);
        count = STD_FREAD((C_VOID *)hdd->connect.pImgBase, sizeof(ntvdm64_type_unsigned_8),
            vm_machine_hdd_image_size(hdd), image);
        hdd->connect.flagDiskExist = NTVDM64_TYPE_TRUE;
        STD_FCLOSE(image);
        return NTVDM64_TYPE_FALSE;
    } else {
        return NTVDM64_TYPE_TRUE;
    }
}
C_INT vm_machine_hdd_remove(t_hdd *hdd, const C_CHAR *file_name) {
    ntvdm64_type_native_unsigned count;
    FILE *image;
    if (hdd == NULL) return NTVDM64_TYPE_TRUE;
    if (file_name) {
        image = STD_FOPEN(file_name, "wb");
        if (image) {
            if (!hdd->connect.flagReadOnly)
                count = STD_FWRITE((C_VOID *)hdd->connect.pImgBase, sizeof(ntvdm64_type_unsigned_8),
                    vm_machine_hdd_image_size(hdd), image);
            hdd->connect.flagDiskExist = NTVDM64_TYPE_FALSE;
            STD_FCLOSE(image);
        } else {
            return NTVDM64_TYPE_TRUE;
        }
    }
    hdd->connect.flagDiskExist = NTVDM64_TYPE_FALSE;
    STD_MEMSET((C_VOID *)hdd->connect.pImgBase, NTVDM64_TYPE_ZERO_8, vm_machine_hdd_image_size(hdd));
    return NTVDM64_TYPE_FALSE;
}
