#include "core/machine/block_provider.h"
#include "vm/machine/hdd.h"
#include "vm/composition_live_machine.h"

static void vmBlockGeometry(void *context, core_machine_block_geometry *out_geometry)
{
    t_hdd *hdd = (t_hdd *)context;
    if (hdd == NULL) return;
    out_geometry->present = hdd->connect.flagDiskExist;
    out_geometry->cylinders = hdd->data.ncyl;
    out_geometry->heads = hdd->data.nhead;
    out_geometry->sectors = hdd->data.nsector;
    out_geometry->bytes_per_sector = hdd->data.nbyte;
}

static int vmBlockTransfer(void *context, ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head,
    ntvdm64_type_unsigned_8 sector, void *buffer, ntvdm64_type_native_unsigned byte_count, int write)
{
    t_hdd *hdd = (t_hdd *)context;
    if (hdd == NULL || !hdd->connect.flagDiskExist || sector == NTVDM64_TYPE_ZERO_8 ||
        head >= hdd->data.nhead || sector > hdd->data.nsector ||
        cylinder >= hdd->data.ncyl || byte_count > hdd->data.nbyte * NTVDM64_TYPE_MAX_UNSIGNED_8) return NTVDM64_TYPE_FALSE;
    hdd->data.cyl = cylinder;
    hdd->data.head = head;
    hdd->data.sector = sector;
    hdd->connect.pCurrByte = hdd->connect.pImgBase +
        ((hdd->data.cyl * hdd->data.nhead + hdd->data.head) *
        hdd->data.nsector + (hdd->data.sector - 1)) * hdd->data.nbyte;
    if (write) STD_MEMCPY((void *)hdd->connect.pCurrByte, buffer, byte_count);
    else STD_MEMCPY(buffer, (void *)hdd->connect.pCurrByte, byte_count);
    return NTVDM64_TYPE_TRUE;
}

static int vmBlockRead(void *context, ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head,
    ntvdm64_type_unsigned_8 sector, void *buffer, ntvdm64_type_native_unsigned byte_count)
{
    return vmBlockTransfer(context, cylinder, head, sector, buffer, byte_count, NTVDM64_TYPE_FALSE);
}

static int vmBlockWrite(void *context, ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head,
    ntvdm64_type_unsigned_8 sector, void *buffer, ntvdm64_type_native_unsigned byte_count)
{
    return vmBlockTransfer(context, cylinder, head, sector, buffer, byte_count, NTVDM64_TYPE_TRUE);
}

void vm_composition_bind_block(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    core_machine_block_provider_slot_bind(machine->block_provider, machine->hdd,
        vmBlockGeometry, vmBlockRead, vmBlockWrite);
    core_machine_block_provider_slot_freeze(machine->block_provider);
}
