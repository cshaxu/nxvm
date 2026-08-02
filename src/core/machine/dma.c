/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */

#include "type.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"
#include "core/machine/dma.h"

static t_latch *coreMachineDmaLatch;
static t_dma *coreMachineDmaPrimary;
static t_dma *coreMachineDmaSecondary;
t_latch *core_machine_dma_latch_current(void) { return coreMachineDmaLatch; }
t_dma *core_machine_dma_primary_current(void) { return coreMachineDmaPrimary; }
t_dma *core_machine_dma_secondary_current(void) { return coreMachineDmaSecondary; }
void core_machine_dma_bind_live(t_latch *latch, t_dma *primary, t_dma *secondary) { coreMachineDmaLatch=latch; coreMachineDmaPrimary=primary; coreMachineDmaSecondary=secondary; }
void core_machine_dma_unbind_live(void) { coreMachineDmaLatch=NULL; coreMachineDmaPrimary=NULL; coreMachineDmaSecondary=NULL; }

static void doReset(t_dma *rdma) {
    MEMSET((void *)(&rdma->data), Zero8, sizeof(t_dma_data));
    rdma->data.mask = VDMA_MASK_VALID;
}

static void io_read_CurrentAddress(t_dma *rdma, t_nubit8 id) {
    if (!rdma->data.flagMSB) {
        vport.data.ioByte = GetMax8(rdma->data.currAddr[id]);
    } else {
        vport.data.ioByte = GetMax8(rdma->data.currAddr[id] >> 8);
    }
    rdma->data.flagMSB = !rdma->data.flagMSB;
}
static void io_read_CurrentWordCount(t_dma *rdma, t_nubit8 id) {
    if (!rdma->data.flagMSB) {
        vport.data.ioByte = GetMax8(rdma->data.currCount[id]);
    } else {
        vport.data.ioByte = GetMax8(rdma->data.currCount[id] >> 8);
    }
    rdma->data.flagMSB = !rdma->data.flagMSB;
}
static void io_read_Status(t_dma *rdma) {
    vport.data.ioByte = rdma->data.status;
    ClrBit(rdma->data.status, VDMA_STATUS_TCS);
}
#define     io_read_Temp(rdma) (vport.data.ioByte = (rdma)->data.temp)
#define     io_read_Page(rdma, id) (vport.data.ioByte = (rdma)->data.page[(id)])

static void io_write_Address(t_dma *rdma, t_nubit8 id) {
    if (!rdma->data.flagMSB) {
        rdma->data.baseAddr[id]  = GetMax16(vport.data.ioByte);
    } else {
        rdma->data.baseAddr[id] |= GetMax16(vport.data.ioByte << 8);
    }
    rdma->data.currAddr[id] = rdma->data.baseAddr[id];
    rdma->data.flagMSB = !rdma->data.flagMSB;
}
static void io_write_WordCount(t_dma *rdma, t_nubit8 id) {
    if (!rdma->data.flagMSB) {
        rdma->data.baseCount[id]  = GetMax16(vport.data.ioByte);
    } else {
        rdma->data.baseCount[id] |= GetMax16(vport.data.ioByte << 8);
    }
    rdma->data.currCount[id] = rdma->data.baseCount[id];
    rdma->data.flagMSB = !rdma->data.flagMSB;
}
#define     io_write_Command(rdma) ((rdma)->data.command = vport.data.ioByte)
static void io_write_Request_Single(t_dma *rdma) {
    MakeBit(rdma->data.request, VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(vport.data.ioByte)),
            GetBit(vport.data.ioByte, VDMA_REQSC_SR));
}
static void io_write_Mask_Single(t_dma *rdma) {
    MakeBit(rdma->data.mask, VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(vport.data.ioByte)),
            GetBit(vport.data.ioByte, VDMA_MASKSC_SM));
}
#define     io_write_Mode(rdma) \
            ((rdma)->data.mode[VDMA_GetMODE_CS(vport.data.ioByte)] = vport.data.ioByte)
#define     io_write_Flipflop_Clear(rdma) ((rdma)->data.flagMSB = False)
#define     io_write_Reset(rdma) (doReset(rdma))
#define     io_write_Mask_Clear(rdma) ((rdma)->data.mask = Zero8)
#define     io_write_Mask_All(rdma) ((rdma)->data.mask = vport.data.ioByte & VDMA_MASKAC_VALID)
#define     io_write_Page(rdma, id, m) \
            ((rdma)->data.page[(id)] = vport.data.ioByte & (m))

static void dma_read_address(t_dma *dma, t_port *port, t_nubit8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        GetMax8(dma->data.currAddr[channel]) :
        GetMax8(dma->data.currAddr[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_read_count(t_dma *dma, t_port *port, t_nubit8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        GetMax8(dma->data.currCount[channel]) :
        GetMax8(dma->data.currCount[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_address(t_dma *dma, t_port *port, t_nubit8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseAddr[channel] = GetMax16(port->data.ioByte);
    } else {
        dma->data.baseAddr[channel] |= GetMax16(port->data.ioByte << 8);
    }
    dma->data.currAddr[channel] = dma->data.baseAddr[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_count(t_dma *dma, t_port *port, t_nubit8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseCount[channel] = GetMax16(port->data.ioByte);
    } else {
        dma->data.baseCount[channel] |= GetMax16(port->data.ioByte << 8);
    }
    dma->data.currCount[channel] = dma->data.baseCount[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_request(t_dma *dma, t_port *port)
{
    MakeBit(dma->data.request,
        VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(port->data.ioByte)),
        GetBit(port->data.ioByte, VDMA_REQSC_SR));
}

static void dma_write_mask(t_dma *dma, t_port *port)
{
    MakeBit(dma->data.mask,
        VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(port->data.ioByte)),
        GetBit(port->data.ioByte, VDMA_MASKSC_SM));
}

static t_dma *dma_controller(t_dma *primary, t_nubit16 port_id)
{
    return port_id >= 0x00c0u ? primary->connect.peer : primary;
}

static t_nubit8 dma_page_channel(t_nubit16 port_id)
{
    switch (port_id) {
    case 0x0081: case 0x0089: return 2u;
    case 0x0082: case 0x008a: return 3u;
    case 0x0083: case 0x008b: return 1u;
    default: return 0u;
    }
}

static void dma_port_read(t_port *port, t_nubit16 port_id, void *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    t_nubit8 channel;

    if (primary == NULL) return;
    if (port_id <= 0x0007u) {
        channel = (t_nubit8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_read_address(primary, port, channel);
        else dma_read_count(primary, port, channel);
        return;
    }
    if (port_id == 0x0008u) {
        port->data.ioByte = primary->data.status;
        ClrBit(primary->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x000du) {
        port->data.ioByte = primary->data.temp;
        return;
    }
    if (port_id >= 0x0081u && port_id <= 0x008fu) {
        dma = dma_controller(primary, port_id);
        port->data.ioByte = dma->data.page[dma_page_channel(port_id)];
        return;
    }
    if (port_id >= 0x00c0u && port_id <= 0x00ceu &&
        (port_id & 1u) == 0u) {
        dma = primary->connect.peer;
        channel = (t_nubit8)((port_id - 0x00c0u) >> 1);
        if ((channel & 1u) == 0u) dma_read_address(dma, port, channel >> 1);
        else dma_read_count(dma, port, channel >> 1);
        return;
    }
    if (port_id == 0x00d0u) {
        port->data.ioByte = primary->connect.peer->data.status;
        ClrBit(primary->connect.peer->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x00dau) {
        port->data.ioByte = primary->connect.peer->data.temp;
    }
}

static void dma_port_write(t_port *port, t_nubit16 port_id, void *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    t_nubit8 channel;
    t_nubit16 local_port;

    if (primary == NULL) return;
    if (port_id <= 0x0007u) {
        channel = (t_nubit8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_write_address(primary, port, channel);
        else dma_write_count(primary, port, channel);
        return;
    }
    if (port_id >= 0x0081u && port_id <= 0x008fu) {
        dma = dma_controller(primary, port_id);
        dma->data.page[dma_page_channel(port_id)] = port->data.ioByte &
            (port_id < 0x0089u ? Max8 : 0xfeu);
        return;
    }
    dma = port_id >= 0x00c0u ? primary->connect.peer : primary;
    local_port = port_id >= 0x00c0u ? port_id - 0x00c0u : port_id;
    if (port_id >= 0x00c0u && (local_port & 1u) == 0u &&
        local_port <= 0x000eu) {
        channel = (t_nubit8)(local_port >> 1);
        if ((channel & 1u) == 0u) dma_write_address(dma, port, channel >> 1);
        else dma_write_count(dma, port, channel >> 1);
        return;
    }
    if (port_id >= 0x00c0u) local_port -= 0x0008u;
    switch (local_port) {
    case 0x0008: dma->data.command = port->data.ioByte; break;
    case 0x0009: dma_write_request(dma, port); break;
    case 0x000a: dma_write_mask(dma, port); break;
    case 0x000b:
        dma->data.mode[VDMA_GetMODE_CS(port->data.ioByte)] = port->data.ioByte;
        break;
    case 0x000c: dma->data.flagMSB = False; break;
    case 0x000d: doReset(dma); break;
    case 0x000e: dma->data.mask = Zero8; break;
    case 0x000f: dma->data.mask = port->data.ioByte & VDMA_MASKAC_VALID; break;
    default: break;
    }
}

static t_nubit8 GetRegTopId(t_dma *rdma, t_nubit8 reg) {
    t_nubit8 id = 0;
    if (reg == Zero8) {
        return 0x08;
    }
    reg = (reg << (VDMA_CHANNEL_COUNT - (rdma->data.drx))) | (reg >> (rdma->data.drx));
    while ((id < VDMA_CHANNEL_COUNT) && !GetMax1(reg >> id)) {
        id++;
    }
    return (id + rdma->data.drx) % VDMA_CHANNEL_COUNT;
}
static void IncreaseCurrAddr(t_dma *rdma, t_nubit8 id) {
    rdma->data.currAddr[id]++;
    if (rdma->data.currAddr[id] == Zero16) {
        rdma->data.page[id]++;
    }
}
static void DecreaseCurrAddr(t_dma *rdma, t_nubit8 id) {
    rdma->data.currAddr[id]--;
    if (rdma->data.currAddr[id] == Max16) {
        rdma->data.page[id]--;
    }
}
static void Transmission(t_dma *rdma, t_latch *latch, t_ram *ram,
                         t_nubit8 id, t_bool flagWord) {
    switch (VDMA_GetMODE_TT(rdma->data.mode[id])) {
    case 0x00:
        /* verify */
        /* do nothing */
        rdma->data.currCount[id]--;
        if (GetBit(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x01:
        /* write */
        if (rdma->connect.read_provider[id] != NULL) {
            rdma->connect.read_provider[id](rdma->connect.device_owner[id], latch);
        } else {
            ExecFun(rdma->connect.fpReadDevice[id]);
        }
        if (!flagWord) {
            core_machine_memory_write_physical(ram,
                (rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                (t_vaddrcc)(&latch->data.byte), 1);
        } else {
            core_machine_memory_write_physical(ram,
                (rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                (t_vaddrcc)(&latch->data.word), 2);
        }
        rdma->data.currCount[id]--;
        if (GetBit(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x02:
        /* read */
        if (!flagWord) {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                (t_vaddrcc)(&latch->data.byte), 1);
        } else {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                (t_vaddrcc)(&latch->data.word), 2);
        }
        if (rdma->connect.write_provider[id] != NULL) {
            rdma->connect.write_provider[id](rdma->connect.device_owner[id], latch);
        } else {
            ExecFun(rdma->connect.fpWriteDevice[id]);
        }
        rdma->data.currCount[id]--;
        if (GetBit(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x03:
        /* illegal */
        break;
    default:
        break;
    }
}
static void Execute(t_dma *rdma, t_latch *latch, t_ram *ram,
                    t_nubit8 id, t_bool flagWord) {
    t_bool flagM2M = ((id == 0) &&
                      VDMA_GetREQUEST_DRQ(rdma->data.request, 1) &&
                      GetBit(rdma->data.command, VDMA_COMMAND_M2M));
    ClrBit(rdma->data.status, VDMA_STATUS_DRQ(id));
    ClrBit(rdma->data.request, VDMA_REQUEST_DRQ(id));
    if (GetBit(rdma->data.command, VDMA_COMMAND_R)) {
        rdma->data.drx = (id + 1) % VDMA_CHANNEL_COUNT;
    }
    if (flagM2M) {
        /* memory-to-memory */
        while (rdma->data.currCount[1] != 0xffff && !rdma->data.flagEOP) {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[0] << 16) + rdma->data.currAddr[0],
                (t_vaddrcc)(&rdma->data.temp), 1);
            core_machine_memory_write_physical(ram,
                (rdma->data.page[1] << 16) + rdma->data.currAddr[1],
                (t_vaddrcc)(&rdma->data.temp), 1);
            rdma->data.currCount[1]--;
            if (GetBit(rdma->data.mode[id], VDMA_MODE_AIDS)) {
                DecreaseCurrAddr(rdma, 1);
                if (!GetBit(rdma->data.command, VDMA_COMMAND_C0AD)) {
                    DecreaseCurrAddr(rdma, 0);
                }
            } else {
                IncreaseCurrAddr(rdma, 1);
                if (!GetBit(rdma->data.command, VDMA_COMMAND_C0AD)) {
                    IncreaseCurrAddr(rdma, 0);
                }
            }
        }
        if (rdma->data.currCount[1] == Max16) {
            SetBit(rdma->data.status, VDMA_STATUS_TC(0));
            rdma->data.flagEOP = True;
        }
    } else {
        /* select mode and command */
        switch (VDMA_GetMODE_M(rdma->data.mode[id])) {
        case 0x00:
            /* demand */
            while (rdma->data.currCount[id] != Max16 && !rdma->data.flagEOP
                    && VDMA_GetSTATUS_DRQ(rdma->data.status, id)) {
                Transmission(rdma, latch, ram, id, flagWord);
            }
            break;
        case 0x01:
            /* single */
            Transmission(rdma, latch, ram, id, flagWord);
            break;
        case 0x02:
            /* block */
            while (rdma->data.currCount[id] != Max16 && !rdma->data.flagEOP) {
                Transmission(rdma, latch, ram, id, flagWord);
            }
            break;
        case 0x03:
            /* cascade */
            /* do nothing */
            rdma->data.flagEOP = True;
            break;
        default:
            break;
        }
        if (rdma->data.currCount[id] == Max16) {
            SetBit(rdma->data.status, VDMA_STATUS_TC(id)); /* set termination count */
            rdma->data.flagEOP = True;
        }
    }
    if (rdma->data.flagEOP) {
        rdma->data.isr = Zero8;
        if (rdma->connect.close_provider[id] != NULL) {
            rdma->connect.close_provider[id](rdma->connect.device_owner[id], latch);
        } else {
            ExecFun(rdma->connect.fpCloseDevice[id]);
        }
        if (GetBit(rdma->data.mode[id], VDMA_MODE_AI)) {
            rdma->data.currAddr[id] = rdma->data.baseAddr[id];
            rdma->data.currCount[id] = rdma->data.baseCount[id];
            ClrBit(rdma->data.mask, VDMA_MASK_DRQ(id));
        } else {
            SetBit(rdma->data.mask, VDMA_MASK_DRQ(id));
        }
    }
    rdma->data.flagEOP = False;
}

void core_machine_dma_set_drq(t_dma *primary, t_dma *secondary,
                              t_nubit8 drq_id) {
    if (primary == NULL || secondary == NULL) return;
    switch (drq_id) {
    case 0:
    case 1:
    case 2:
    case 3:
        SetBit(primary->data.status, VDMA_STATUS_DRQ(drq_id));
        break;
    case 5:
    case 6:
    case 7:
        SetBit(secondary->data.status, VDMA_STATUS_DRQ(drq_id - 4));
        break;
    case 4:
    default:
        break;
    }
    if (primary->data.status & VDMA_STATUS_DRQS) {
        SetBit(secondary->data.status, VDMA_STATUS_DRQ(0));
    } else {
        ClrBit(secondary->data.status, VDMA_STATUS_DRQ(0));
    }
}
void vdmaSetDRQ(t_nubit8 drqId) {
    core_machine_dma_set_drq(core_machine_dma_primary_current(),
        core_machine_dma_secondary_current(), drqId);
}

void core_machine_dma_add_device(t_dma *primary, t_dma *secondary,
    t_nubit8 drq_id, t_faddrcc read_device, t_faddrcc write_device,
    t_faddrcc close_device) {
    if (primary == NULL || secondary == NULL) return;
    switch (drq_id) {
    case 0:
    case 1:
    case 2:
    case 3:
        primary->connect.fpReadDevice[drq_id] = read_device;
        primary->connect.fpWriteDevice[drq_id] = write_device;
        primary->connect.fpCloseDevice[drq_id] = close_device;
        break;
    case 5:
    case 6:
    case 7:
        secondary->connect.fpReadDevice[drq_id - 4] = read_device;
        secondary->connect.fpWriteDevice[drq_id - 4] = write_device;
        secondary->connect.fpCloseDevice[drq_id - 4] = close_device;
        break;
    case 4:
    default:
        break;
    }
}

void core_machine_dma_bind_device(t_dma *primary, t_dma *secondary,
    t_nubit8 drq_id, core_machine_dma_device_provider read_provider,
    core_machine_dma_device_provider write_provider,
    core_machine_dma_device_provider close_provider, void *owner)
{
    t_dma *dma;
    t_nubit8 channel;

    if (primary == NULL || secondary == NULL) return;
    if (drq_id <= 3u) {
        dma = primary;
        channel = drq_id;
    } else if (drq_id >= 5u && drq_id <= 7u) {
        dma = secondary;
        channel = drq_id - 4u;
    } else {
        return;
    }
    dma->connect.read_provider[channel] = read_provider;
    dma->connect.write_provider[channel] = write_provider;
    dma->connect.close_provider[channel] = close_provider;
    dma->connect.device_owner[channel] = owner;
}

void vdmaAddDevice(t_nubit8 drqId, t_faddrcc fpReadDevice,
                   t_faddrcc fpWriteDevice, t_faddrcc fpCloseDevice) {
    core_machine_dma_add_device(core_machine_dma_primary_current(),
        core_machine_dma_secondary_current(), drqId, fpReadDevice,
        fpWriteDevice, fpCloseDevice);
}

void core_machine_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port)
{
    static const t_nubit16 primary_reads[] = {
        0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
        0x0008, 0x000d
    };
    static const t_nubit16 page_ports[] = {
        0x0081, 0x0082, 0x0083, 0x0087, 0x0089, 0x008a, 0x008b, 0x008f
    };
    static const t_nubit16 secondary_reads[] = {
        0x00c0, 0x00c2, 0x00c4, 0x00c6, 0x00c8, 0x00ca, 0x00cc, 0x00ce,
        0x00d0, 0x00da
    };
    t_nubitcc index;

    if (latch == NULL || primary == NULL || secondary == NULL ||
        port == NULL) return;
    MEMSET((void *)latch, Zero8, sizeof(*latch));
    MEMSET((void *)primary, Zero8, sizeof(*primary));
    MEMSET((void *)secondary, Zero8, sizeof(*secondary));
    primary->connect.latch = latch;
    primary->connect.peer = secondary;
    secondary->connect.latch = latch;
    secondary->connect.peer = primary;
    for (index = 0; index < sizeof(primary_reads) / sizeof(primary_reads[0]);
         ++index) {
        core_machine_port_add_read(port, primary_reads[index], dma_port_read,
            primary);
    }
    for (index = 0; index < 0x10u; ++index) {
        core_machine_port_add_write(port, (t_nubit16)index, dma_port_write,
            primary);
    }
    for (index = 0; index < sizeof(page_ports) / sizeof(page_ports[0]);
         ++index) {
        core_machine_port_add_read(port, page_ports[index], dma_port_read,
            primary);
        core_machine_port_add_write(port, page_ports[index], dma_port_write,
            primary);
    }
    for (index = 0; index < sizeof(secondary_reads) /
         sizeof(secondary_reads[0]); ++index) {
        core_machine_port_add_read(port, secondary_reads[index], dma_port_read,
            primary);
    }
    for (index = 0; index <= 0x1eu; index += 2u) {
        core_machine_port_add_write(port, (t_nubit16)(0x00c0u + index),
            dma_port_write, primary);
    }
}

void vdmaInit() {
    core_machine_dma_initialize(core_machine_dma_latch_current(),
        core_machine_dma_primary_current(), core_machine_dma_secondary_current(),
        core_machine_port_current());
}
void core_machine_dma_reset(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    if (latch == NULL || primary == NULL || secondary == NULL) return;
    MEMSET((void *)(&latch->data), Zero8, sizeof(t_latch_data));
    doReset(primary);
    doReset(secondary);
}

void vdmaReset() {
    core_machine_dma_reset(core_machine_dma_latch_current(),
        core_machine_dma_primary_current(), core_machine_dma_secondary_current());
}

void core_machine_dma_refresh(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram) {
    t_nubit8 id;
    t_nubit8 realDRQ1, realDRQ2;
    if (latch == NULL || primary == NULL || secondary == NULL || ram == NULL) return;
    if (GetBit(secondary->data.command, VDMA_COMMAND_CTRL)) {
        return;
    }
    if (GetBit(secondary->data.isr, VDMA_ISR_IS)) {
        if (VDMA_GetISR_ISR(secondary->data.isr)) {
            Execute(secondary, latch, ram, VDMA_GetISR_ISR(secondary->data.isr), True);
        } else if (GetBit(primary->data.isr, VDMA_ISR_IS)) {
            Execute(primary, latch, ram, VDMA_GetISR_ISR(primary->data.isr), False);
        }
        if (!GetBit(primary->data.isr, VDMA_ISR_IS)) {
            secondary->data.isr = Zero8;
        }
    }
    if (!GetBit(secondary->data.isr, VDMA_ISR_IS)) {
        realDRQ2 = secondary->data.request | (VDMA_GetSTATUS_DRQS(secondary->data.status) & ~secondary->data.mask);
        if (realDRQ2 == Zero8) {
            return;
        }
        id = GetRegTopId(secondary, realDRQ2);
        if (id == 0) {
            if (GetBit(primary->data.command, VDMA_COMMAND_CTRL)) {
                return;
            }
            realDRQ1 = primary->data.request | (VDMA_GetSTATUS_DRQS(primary->data.status) & ~primary->data.mask);
            if (realDRQ1 == Zero8) {
                return;
            }
            id = GetRegTopId(primary, realDRQ1);
            VDMA_SetISR(secondary->data.isr, 0);
            VDMA_SetISR(primary->data.isr, id);
            Execute(primary, latch, ram, id, False);
            if (!GetBit(primary->data.isr, VDMA_ISR_IS)) {
                secondary->data.isr = Zero8;
            }
            if (!VDMA_GetSTATUS_DRQS(primary->data.status)) {
                ClrBit(secondary->data.status, VDMA_STATUS_DRQ(0));
            }
            if (!primary->data.request) {
                ClrBit(secondary->data.request, VDMA_REQUEST_DRQ(0));
            }
        } else {
            VDMA_SetISR(secondary->data.isr, id);
            Execute(secondary, latch, ram, id, True);
        }
    }
}

void vdmaRefresh() {
    core_machine_dma_refresh(core_machine_dma_latch_current(),
        core_machine_dma_primary_current(), core_machine_dma_secondary_current(),
        core_machine_memory_current());
}

void core_machine_dma_finalize(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    (void)latch;
    (void)primary;
    (void)secondary;
}

void vdmaFinal() {
    core_machine_dma_finalize(core_machine_dma_latch_current(),
        core_machine_dma_primary_current(), core_machine_dma_secondary_current());
}

static void printDma(t_dma *rdma) {
    t_nubitcc i;
    PRINTF("Command = %x, status = %x, mask = %x\n",
           rdma->data.command, rdma->data.status, rdma->data.mask);
    PRINTF("request = %x, temp = %x, flagMSB = %x\n",
           rdma->data.request, rdma->data.temp, rdma->data.flagMSB);
    PRINTF("drx = %x, flagEOP = %x, isr = %x\n",
           rdma->data.drx, rdma->data.flagEOP, rdma->data.isr);
    for (i = 0; i < VDMA_CHANNEL_COUNT; ++i) {
        PRINTF("Channel %d: baseAddr = %x, baseCount = %x, currAddr = %x, currCount = %x\n",
               i, rdma->data.baseAddr[i], rdma->data.baseCount[i],
               rdma->data.currAddr[i], rdma->data.currCount[i]);
        PRINTF("Channel %d: mode = %x, page = %x, fpReadDevice = %x, fpWriteDevice = %x, fpCloseDevice = %x\n",
               i, rdma->data.mode[i], rdma->data.page[i],
               rdma->connect.fpReadDevice[i], rdma->connect.fpWriteDevice[i],
               rdma->connect.fpCloseDevice[i]);
    }
}

/* Print DMA status */
void devicePrintDma() {
    PRINTF("DMA 1 Info\n==========\n");
    printDma(&vdma1);
    PRINTF("\nDMA 2 Info\n==========\n");
    printDma(&vdma2);
}

/*
debug
FOR FDC READ/WRITE
off00 0  reset dma
o0b 86   mode: block, increase, write, channel 2
         other options: c6(cascade)  46(single) 06(demand)
o0b 8a   mode: block, increase, read, channel 2
o04 00   write address to: channel 2 low
o04 00   write address to: channel 2 high
o05 00   write word count to: channel 2 low
o05 02   write word count to: channel 2 high
o0a 02   write mask: dma 1, channel 2
od4 00   write mask: dma 2, channel 0
off01 2  generate dreq; otherwise ASK DEVICE TO GENERATE DREQ
off02 0  refresh: exec
d0:0     show results

OTHER TEST CODES
off00 0
e6020 ab
o08 03
o0b 00
o03 5
o03 0
od4 0
o0a 0
o00 30
o00 60
o02 30
o02 80
o09 04
od2 04
off02 0
d6020
d8020
*/
