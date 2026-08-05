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

static type_virtual_address vm_machine_fdd_byte_address(const t_fdd *fdd,
    type_unsigned_16 cylinder, type_unsigned_16 head,
    type_unsigned_16 sector, type_unsigned_16 offset)
{
    return fdd->connect.pImgBase + (((cylinder * fdd->data.nhead + head) *
        fdd->data.nsector + (sector - 1u)) * fdd->data.nbyte) + offset;
}

C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 bytes)
{
    return fdd != STD_NULL && fdd->connect.flagDiskExist &&
        fdd->connect.pImgBase != (type_virtual_address)STD_NULL &&
        cylinder < fdd->data.ncyl && head < fdd->data.nhead && sector > 0u &&
        sector <= fdd->data.nsector && bytes == fdd->data.nbyte;
}

C_INT vm_machine_fdd_read_byte(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 *out_byte)
{
    if (out_byte == STD_NULL || !vm_machine_fdd_chs_valid(fdd, cylinder, head,
        sector, fdd != STD_NULL ? fdd->data.nbyte : 0u) ||
        offset >= fdd->data.nbyte) return TYPE_TRUE;
    *out_byte = TYPE_DEREFERENCE_UNSIGNED_8(vm_machine_fdd_byte_address(fdd,
        cylinder, head, sector, offset));
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_write_byte(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 value)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte) || offset >= fdd->data.nbyte) {
        return TYPE_TRUE;
    }
    TYPE_DEREFERENCE_UNSIGNED_8(vm_machine_fdd_byte_address(fdd, cylinder, head,
        sector, offset)) = value;
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_format_sector(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_8 fill_byte)
{
    if (fdd == STD_NULL || fdd->connect.flagReadOnly || !vm_machine_fdd_chs_valid(
        fdd, cylinder, head, sector, fdd->data.nbyte)) return TYPE_TRUE;
    STD_MEMSET((C_VOID *)vm_machine_fdd_byte_address(fdd, cylinder, head, sector,
        0u), fill_byte, fdd->data.nbyte);
    return TYPE_FALSE;
}

C_VOID vm_machine_fdd_initialize(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)fdd, TYPE_ZERO_8, sizeof(*fdd));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
    fdd->connect.pImgBase = (type_virtual_address)STD_MALLOC(vm_machine_fdd_image_size(fdd));
    STD_MEMSET((C_VOID *)fdd->connect.pImgBase, TYPE_ZERO_8, vm_machine_fdd_image_size(fdd));
}

C_VOID vm_machine_fdd_reset(t_fdd *fdd)
{
    if (fdd == STD_NULL) return;
    STD_MEMSET((C_VOID *)&fdd->data, TYPE_ZERO_8, sizeof(fdd->data));
    fdd->data.ncyl = 0x0050;
    fdd->data.nhead = 0x0002;
    fdd->data.nsector = 0x0012;
    fdd->data.nbyte = 0x0200;
}

C_VOID vm_machine_fdd_refresh(t_fdd *fdd) { (C_VOID)fdd; }

C_VOID vm_machine_fdd_finalize(t_fdd *fdd)
{
    if (fdd != STD_NULL && fdd->connect.pImgBase) STD_FREE((C_VOID *)fdd->connect.pImgBase);
    if (fdd != STD_NULL) fdd->connect.pImgBase = (type_virtual_address)STD_NULL;
}

C_VOID vm_machine_fdd_create_for(t_fdd *fdd)
{
    if (fdd != STD_NULL) {
        fdd->connect.flagDiskExist = TYPE_TRUE;
        fdd->connect.media_generation++;
    }
}

C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name)
{
    STD_FILE *image = STD_FOPEN(file_name, "rb");
    if (fdd == STD_NULL || image == STD_NULL ||
        fdd->connect.pImgBase == (type_virtual_address)STD_NULL) return TYPE_TRUE;
    STD_FREAD((C_VOID *)fdd->connect.pImgBase, sizeof(type_unsigned_8),
        vm_machine_fdd_image_size(fdd), image);
    fdd->connect.flagDiskExist = TYPE_TRUE;
    fdd->connect.media_generation++;
    STD_FCLOSE(image);
    return TYPE_FALSE;
}

C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name)
{
    STD_FILE *image;
    if (fdd == STD_NULL) return TYPE_TRUE;
    if (file_name != STD_NULL) {
        image = STD_FOPEN(file_name, "wb");
        if (image == STD_NULL) return TYPE_TRUE;
        if (!fdd->connect.flagReadOnly) STD_FWRITE((C_VOID *)fdd->connect.pImgBase,
            sizeof(type_unsigned_8), vm_machine_fdd_image_size(fdd), image);
        STD_FCLOSE(image);
    }
    fdd->connect.flagDiskExist = TYPE_FALSE;
    fdd->connect.media_generation++;
    STD_MEMSET((C_VOID *)fdd->connect.pImgBase, TYPE_ZERO_8, vm_machine_fdd_image_size(fdd));
    return TYPE_FALSE;
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
    STD_PRINTF("base = %x, media generation = %u\n",
           fdd->connect.pImgBase, fdd->connect.media_generation);
}
