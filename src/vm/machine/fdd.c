/* Copyright 2012-2014 Neko. */

/* VFDD implements Floppy Disk Drive: 3.5" 1.44MB. */

#include "core/product/utils.h"

#include "core/machine/dma.h"

#include "vm/machine/fdd.h"

size_t vm_machine_fdd_image_size(const t_fdd *fdd)
{
    return (size_t)fdd->data.nbyte * fdd->data.nsector * fdd->data.nhead *
        fdd->data.ncyl;
}

void vm_machine_fdd_set_pointer(t_fdd *fdd)
{
    fdd->connect.pCurrByte = fdd->connect.pImgBase +
        ((fdd->data.cyl * fdd->data.nhead + fdd->data.head) *
        fdd->data.nsector + (fdd->data.sector - 1)) * fdd->data.nbyte;
}

void vm_machine_fdd_transfer_read(t_fdd *fdd, t_latch *latch)
{
    if (fdd == NULL || latch == NULL ||
        (fdd->data.head == 1 && fdd->data.sector >= fdd->data.nsector + 1)) return;
    latch->data.byte = d_nubit8(fdd->connect.pCurrByte);
    fdd->connect.pCurrByte++;
    fdd->connect.transCount++;
    if (!(fdd->connect.transCount % fdd->data.nbyte)) {
        fdd->data.sector++;
        if (fdd->data.head == 0 && fdd->data.sector >= fdd->data.nsector + 1) {
            fdd->data.sector = 1;
            fdd->data.head = 1;
        }
        vm_machine_fdd_set_pointer(fdd);
    }
}

void vm_machine_fdd_transfer_write(t_fdd *fdd, t_latch *latch)
{
    if (fdd == NULL || latch == NULL ||
        (fdd->data.head == 1 && fdd->data.sector >= fdd->data.nsector + 1)) return;
    d_nubit8(fdd->connect.pCurrByte) = latch->data.byte;
    fdd->connect.pCurrByte++;
    fdd->connect.transCount++;
    if (!(fdd->connect.transCount % fdd->data.nbyte)) {
        fdd->data.sector++;
        if (fdd->data.head == 0 && fdd->data.sector >= fdd->data.nsector + 1) {
            fdd->data.sector = 1;
            fdd->data.head = 1;
        }
        vm_machine_fdd_set_pointer(fdd);
    }
}

void vm_machine_fdd_format_track(t_fdd *fdd, t_nubit8 fill_byte)
{
    if (fdd == NULL || fdd->data.cyl >= fdd->data.ncyl) return;
    fdd->data.head = 0;
    fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdd);
    MEMSET((void *)fdd->connect.pCurrByte, fill_byte,
        fdd->data.nsector * fdd->data.nbyte);
    fdd->data.head = 1;
    fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdd);
    MEMSET((void *)fdd->connect.pCurrByte, fill_byte,
        fdd->data.nsector * fdd->data.nbyte);
    fdd->data.sector = fdd->data.nsector;
}

void vm_machine_fdd_initialize(t_fdd *fdd)
{
    if (fdd == NULL) return;
    MEMSET((void *)fdd, Zero8, sizeof(*fdd));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
    fdd->connect.pImgBase = (t_vaddrcc)MALLOC(vm_machine_fdd_image_size(fdd));
    MEMSET((void *)fdd->connect.pImgBase, Zero8, vm_machine_fdd_image_size(fdd));
}

void vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == NULL) return;
    MEMSET((void *)&fdd->data, Zero8, sizeof(fdd->data));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
}

void vm_machine_fdd_refresh(t_fdd *fdd) { (void)fdd; }

void vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != NULL && fdd->connect.pImgBase) FREE((void *)fdd->connect.pImgBase);
    if (fdd != NULL) fdd->connect.pImgBase = (t_vaddrcc)NULL;
}

void vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != NULL) fdd->connect.flagDiskExist = True;
}

int vm_machine_fdd_insert_for(t_fdd *fdd, const char *file_name)
{
    FILE *image = FOPEN(file_name, "rb");
    if (fdd == NULL || image == NULL ||
        fdd->connect.pImgBase == (t_vaddrcc)NULL) return True;
    FREAD((void *)fdd->connect.pImgBase, sizeof(t_nubit8),
        vm_machine_fdd_image_size(fdd), image);
    fdd->connect.flagDiskExist = True;
    FCLOSE(image);
    return False;
}

int vm_machine_fdd_remove_for(t_fdd *fdd, const char *file_name)
{
    FILE *image;
    if (fdd == NULL) return True;
    if (file_name != NULL) {
        image = FOPEN(file_name, "wb");
        if (image == NULL) return True;
        if (!fdd->connect.flagReadOnly) FWRITE((void *)fdd->connect.pImgBase,
            sizeof(t_nubit8), vm_machine_fdd_image_size(fdd), image);
        FCLOSE(image);
    }
    fdd->connect.flagDiskExist = False;
    MEMSET((void *)fdd->connect.pImgBase, Zero8, vm_machine_fdd_image_size(fdd));
    return False;
}

void vm_machine_fdd_print(const t_fdd *fdd) {
    if (fdd == NULL) return;
    PRINTF("FDD INFO\n========\n");
    PRINTF("cyl = %x, head = %x, sector = %x, gpl = %x\n",
           fdd->data.cyl, fdd->data.head, fdd->data.sector, fdd->data.gpl);
    PRINTF("nsector = %x, nbyte = %x, ncyl = %x, nhead = %x\n",
           fdd->data.nsector, fdd->data.nbyte, fdd->data.ncyl,
           fdd->data.nhead);
    PRINTF("ReadOnly = %x, Exist = %x\n",
           fdd->connect.flagReadOnly, fdd->connect.flagDiskExist);
    PRINTF("base = %x, curr = %x, count = %x\n",
           fdd->connect.pImgBase, fdd->connect.pCurrByte, fdd->connect.transCount);
}
