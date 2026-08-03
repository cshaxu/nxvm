/* Copyright 2012-2014 Neko. */

/* VFDD implements Floppy Disk Drive: 3.5" 1.44MB. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/dma.h"


#include "vm/machine/fdd.h"

STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd)
{
    return (STD_SIZE_T)fdd->data.nbyte * fdd->data.nsector * fdd->data.nhead *
        fdd->data.ncyl;
}

C_VOID vm_machine_fdd_set_pointer(t_fdd *fdd)
{
    fdd->connect.pCurrByte = fdd->connect.pImgBase +
        ((fdd->data.cyl * fdd->data.nhead + fdd->data.head) *
        fdd->data.nsector + (fdd->data.sector - 1)) * fdd->data.nbyte;
}

C_VOID vm_machine_fdd_transfer_read(t_fdd *fdd, t_latch *latch)
{
    if (fdd == STD_NULL || latch == STD_NULL ||
        (fdd->data.head == 1 && fdd->data.sector >= fdd->data.nsector + 1)) return;
    latch->data.byte = NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pCurrByte);
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

C_VOID vm_machine_fdd_transfer_write(t_fdd *fdd, t_latch *latch)
{
    if (fdd == STD_NULL || latch == STD_NULL ||
        (fdd->data.head == 1 && fdd->data.sector >= fdd->data.nsector + 1)) return;
    NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(fdd->connect.pCurrByte) = latch->data.byte;
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

C_VOID vm_machine_fdd_format_track(t_fdd *fdd, ntvdm64_type_unsigned_8 fill_byte)
{
    if (fdd == STD_NULL || fdd->data.cyl >= fdd->data.ncyl) return;
    fdd->data.head = 0;
    fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdd);
    STD_MEMSET((C_VOID *)fdd->connect.pCurrByte, fill_byte,
        fdd->data.nsector * fdd->data.nbyte);
    fdd->data.head = 1;
    fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdd);
    STD_MEMSET((C_VOID *)fdd->connect.pCurrByte, fill_byte,
        fdd->data.nsector * fdd->data.nbyte);
    fdd->data.sector = fdd->data.nsector;
}

C_VOID vm_machine_fdd_initialize(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)fdd, NTVDM64_TYPE_ZERO_8, sizeof(*fdd));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
    fdd->connect.pImgBase = (ntvdm64_type_virtual_address)STD_MALLOC(vm_machine_fdd_image_size(fdd));
    STD_MEMSET((C_VOID *)fdd->connect.pImgBase, NTVDM64_TYPE_ZERO_8, vm_machine_fdd_image_size(fdd));
}

C_VOID vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)&fdd->data, NTVDM64_TYPE_ZERO_8, sizeof(fdd->data));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
}

C_VOID vm_machine_fdd_refresh(t_fdd *fdd) { (C_VOID)fdd; }

C_VOID vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != STD_NULL && fdd->connect.pImgBase) STD_FREE((C_VOID *)fdd->connect.pImgBase);
    if (fdd != STD_NULL) fdd->connect.pImgBase = (ntvdm64_type_virtual_address)STD_NULL;
}

C_VOID vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != STD_NULL) fdd->connect.flagDiskExist = NTVDM64_TYPE_TRUE;
}

C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name)
{
    STD_FILE *image = STD_FOPEN(file_name, "rb");
    if (fdd == STD_NULL || image == STD_NULL ||
        fdd->connect.pImgBase == (ntvdm64_type_virtual_address)STD_NULL) return NTVDM64_TYPE_TRUE;
    STD_FREAD((C_VOID *)fdd->connect.pImgBase, sizeof(ntvdm64_type_unsigned_8),
        vm_machine_fdd_image_size(fdd), image);
    fdd->connect.flagDiskExist = NTVDM64_TYPE_TRUE;
    STD_FCLOSE(image);
    return NTVDM64_TYPE_FALSE;
}

C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name)
{
    STD_FILE *image;
    if (fdd == STD_NULL) return NTVDM64_TYPE_TRUE;
    if (file_name != STD_NULL) {
        image = STD_FOPEN(file_name, "wb");
        if (image == STD_NULL) return NTVDM64_TYPE_TRUE;
        if (!fdd->connect.flagReadOnly) STD_FWRITE((C_VOID *)fdd->connect.pImgBase,
            sizeof(ntvdm64_type_unsigned_8), vm_machine_fdd_image_size(fdd), image);
        STD_FCLOSE(image);
    }
    fdd->connect.flagDiskExist = NTVDM64_TYPE_FALSE;
    STD_MEMSET((C_VOID *)fdd->connect.pImgBase, NTVDM64_TYPE_ZERO_8, vm_machine_fdd_image_size(fdd));
    return NTVDM64_TYPE_FALSE;
}

C_VOID vm_machine_fdd_print(const t_fdd *fdd) {
    if (fdd == STD_NULL) return;
    STD_PRINTF("FDD INFO\n========\n");
    STD_PRINTF("cyl = %x, head = %x, sector = %x, gpl = %x\n",
           fdd->data.cyl, fdd->data.head, fdd->data.sector, fdd->data.gpl);
    STD_PRINTF("nsector = %x, nbyte = %x, ncyl = %x, nhead = %x\n",
           fdd->data.nsector, fdd->data.nbyte, fdd->data.ncyl,
           fdd->data.nhead);
    STD_PRINTF("ReadOnly = %x, Exist = %x\n",
           fdd->connect.flagReadOnly, fdd->connect.flagDiskExist);
    STD_PRINTF("base = %x, curr = %x, count = %x\n",
           fdd->connect.pImgBase, fdd->connect.pCurrByte, fdd->connect.transCount);
}
