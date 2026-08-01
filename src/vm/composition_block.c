#include "core/machine/block.h"
#include "vm/machine/vhdd.h"

static void vmBlockGeometry(void *context, core_block_geometry *out_geometry)
{
    (void)context;
    out_geometry->present = vhdd.connect.flagDiskExist;
    out_geometry->cylinders = vhdd.data.ncyl;
    out_geometry->heads = vhdd.data.nhead;
    out_geometry->sectors = vhdd.data.nsector;
    out_geometry->bytes_per_sector = vhdd.data.nbyte;
}

static int vmBlockTransfer(void *context, t_nubit8 cylinder, t_nubit8 head,
    t_nubit8 sector, void *buffer, t_nubitcc byte_count, int write)
{
    (void)context;
    if (!vhdd.connect.flagDiskExist || sector == Zero8 || head >= vhdd.data.nhead ||
        sector > vhdd.data.nsector || cylinder >= vhdd.data.ncyl ||
        byte_count > vhdd.data.nbyte * Max8) return False;
    vhdd.data.cyl = cylinder;
    vhdd.data.head = head;
    vhdd.data.sector = sector;
    vhddSetPointer;
    if (write) MEMCPY((void *)vhdd.connect.pCurrByte, buffer, byte_count);
    else MEMCPY(buffer, (void *)vhdd.connect.pCurrByte, byte_count);
    return True;
}

static int vmBlockRead(void *context, t_nubit8 cylinder, t_nubit8 head,
    t_nubit8 sector, void *buffer, t_nubitcc byte_count)
{
    return vmBlockTransfer(context, cylinder, head, sector, buffer, byte_count, False);
}

static int vmBlockWrite(void *context, t_nubit8 cylinder, t_nubit8 head,
    t_nubit8 sector, void *buffer, t_nubitcc byte_count)
{
    return vmBlockTransfer(context, cylinder, head, sector, buffer, byte_count, True);
}

void vmCompositionBindBlock(void)
{
    coreBlockBind(NULL, vmBlockGeometry, vmBlockRead, vmBlockWrite);
}
