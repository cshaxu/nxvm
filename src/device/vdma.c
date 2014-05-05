/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */

#include "../utils.h"

#include "vram.h"

#include "vbios.h"
#include "vport.h"
#include "vdma.h"

t_latch vlatch;
t_dma vdma1, vdma2;

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

static void io_read_0000()  {
    io_read_CurrentAddress   (&vdma1, 0);
}
static void io_read_0001()  {
    io_read_CurrentWordCount (&vdma1, 0);
}
static void io_read_0002()  {
    io_read_CurrentAddress   (&vdma1, 1);
}
static void io_read_0003()  {
    io_read_CurrentWordCount (&vdma1, 1);
}
static void io_read_0004()  {
    io_read_CurrentAddress   (&vdma1, 2);
}
static void io_read_0005()  {
    io_read_CurrentWordCount (&vdma1, 2);
}
static void io_read_0006()  {
    io_read_CurrentAddress   (&vdma1, 3);
}
static void io_read_0007()  {
    io_read_CurrentWordCount (&vdma1, 3);
}
static void io_read_0008()  {
    io_read_Status           (&vdma1   );
}
static void io_read_000D()  {
    io_read_Temp             (&vdma1   );
}

static void io_write_0000() {
    io_write_Address         (&vdma1, 0);
}
static void io_write_0001() {
    io_write_WordCount       (&vdma1, 0);
}
static void io_write_0002() {
    io_write_Address         (&vdma1, 1);
}
static void io_write_0003() {
    io_write_WordCount       (&vdma1, 1);
}
static void io_write_0004() {
    io_write_Address         (&vdma1, 2);
}
static void io_write_0005() {
    io_write_WordCount       (&vdma1, 2);
}
static void io_write_0006() {
    io_write_Address         (&vdma1, 3);
}
static void io_write_0007() {
    io_write_WordCount       (&vdma1, 3);
}
static void io_write_0008() {
    io_write_Command         (&vdma1   );
}
static void io_write_0009() {
    io_write_Request_Single  (&vdma1   );
}
static void io_write_000A() {
    io_write_Mask_Single     (&vdma1   );
}
static void io_write_000B() {
    io_write_Mode            (&vdma1   );
}
static void io_write_000C() {
    io_write_Flipflop_Clear  (&vdma1   );
}
static void io_write_000D() {
    io_write_Reset           (&vdma1   );
}
static void io_write_000E() {
    io_write_Mask_Clear      (&vdma1   );
}
static void io_write_000F() {
    io_write_Mask_All        (&vdma1   );
}

static void io_read_0081()  {
    io_read_Page             (&vdma1, 2);
}
static void io_read_0082()  {
    io_read_Page             (&vdma1, 3);
}
static void io_read_0083()  {
    io_read_Page             (&vdma1, 1);
}
static void io_read_0087()  {
    io_read_Page             (&vdma1, 0);
}
static void io_read_0089()  {
    io_read_Page             (&vdma2, 2);
}
static void io_read_008A()  {
    io_read_Page             (&vdma2, 3);
}
static void io_read_008B()  {
    io_read_Page             (&vdma2, 1);
}
static void io_read_008F()  {
    io_read_Page             (&vdma2, 0);
}

static void io_write_0081() {
    io_write_Page            (&vdma1, 2, Max8);
}
static void io_write_0082() {
    io_write_Page            (&vdma1, 3, Max8);
}
static void io_write_0083() {
    io_write_Page            (&vdma1, 1, Max8);
}
static void io_write_0087() {
    io_write_Page            (&vdma1, 0, Max8);
}
static void io_write_0089() {
    io_write_Page            (&vdma2, 2, 0xfe);
}
static void io_write_008A() {
    io_write_Page            (&vdma2, 3, 0xfe);
}
static void io_write_008B() {
    io_write_Page            (&vdma2, 1, 0xfe);
}
static void io_write_008F() {
    io_write_Page            (&vdma2, 0, 0xfe);
}

static void io_read_00C0()  {
    io_read_CurrentAddress   (&vdma2, 0);
}
static void io_read_00C2()  {
    io_read_CurrentWordCount (&vdma2, 0);
}
static void io_read_00C4()  {
    io_read_CurrentAddress   (&vdma2, 1);
}
static void io_read_00C6()  {
    io_read_CurrentWordCount (&vdma2, 1);
}
static void io_read_00C8()  {
    io_read_CurrentAddress   (&vdma2, 2);
}
static void io_read_00CA()  {
    io_read_CurrentWordCount (&vdma2, 2);
}
static void io_read_00CC()  {
    io_read_CurrentAddress   (&vdma2, 3);
}
static void io_read_00CE()  {
    io_read_CurrentWordCount (&vdma2, 3);
}
static void io_read_00D0()  {
    io_read_Status           (&vdma2   );
}
static void io_read_00DA()  {
    io_read_Temp             (&vdma2   );
}

static void io_write_00C0() {
    io_write_Address         (&vdma2, 0);
}
static void io_write_00C2() {
    io_write_WordCount       (&vdma2, 0);
}
static void io_write_00C4() {
    io_write_Address         (&vdma2, 1);
}
static void io_write_00C6() {
    io_write_WordCount       (&vdma2, 1);
}
static void io_write_00C8() {
    io_write_Address         (&vdma2, 2);
}
static void io_write_00CA() {
    io_write_WordCount       (&vdma2, 2);
}
static void io_write_00CC() {
    io_write_Address         (&vdma2, 3);
}
static void io_write_00CE() {
    io_write_WordCount       (&vdma2, 3);
}
static void io_write_00D0() {
    io_write_Command         (&vdma2   );
}
static void io_write_00D2() {
    io_write_Request_Single  (&vdma2   );
}
static void io_write_00D4() {
    io_write_Mask_Single     (&vdma2   );
}
static void io_write_00D6() {
    io_write_Mode            (&vdma2   );
}
static void io_write_00D8() {
    io_write_Flipflop_Clear  (&vdma2   );
}
static void io_write_00DA() {
    io_write_Reset           (&vdma2   );
}
static void io_write_00DC() {
    io_write_Mask_Clear      (&vdma2   );
}
static void io_write_00DE() {
    io_write_Mask_All        (&vdma2   );
}

static t_nubit8 GetRegTopId(t_dma *rdma, t_nubit8 reg) {
    t_nubit8 id = Zero8;
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
static void Transmission(t_dma *rdma, t_nubit8 id, t_bool flagWord) {
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
        ExecFun(rdma->connect.fpReadDevice[id]);
        if (!flagWord) {
            vramWritePhysical((rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                              (t_vaddrcc)(&vlatch.data.byte), 1);
        } else {
            vramWritePhysical((rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                              (t_vaddrcc)(&vlatch.data.word), 2);
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
            vramReadPhysical((rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                             (t_vaddrcc)(&vlatch.data.byte), 1);
        } else {
            vramReadPhysical((rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                             (t_vaddrcc)(&vlatch.data.word), 2);
        }
        ExecFun(rdma->connect.fpWriteDevice[id]);
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
static void Execute(t_dma *rdma, t_nubit8 id, t_bool flagWord) {
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
            vramReadPhysical((rdma->data.page[0] << 16) + rdma->data.currAddr[0],
                             (t_vaddrcc)(&rdma->data.temp), 1);
            vramWritePhysical((rdma->data.page[1] << 16) + rdma->data.currAddr[1],
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
                Transmission(rdma, id, flagWord);
            }
            break;
        case 0x01:
            /* single */
            Transmission(rdma, id, flagWord);
            break;
        case 0x02:
            /* block */
            while (rdma->data.currCount[id] != Max16 && !rdma->data.flagEOP) {
                Transmission(rdma, id, flagWord);
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
        ExecFun(rdma->connect.fpCloseDevice[id]);
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

void vdmaSetDRQ(t_nubit8 drqId) {
    switch (drqId) {
    case 0:
    case 1:
    case 2:
    case 3:
        SetBit(vdma1.data.status, VDMA_STATUS_DRQ(drqId));
        break;
    case 5:
    case 6:
    case 7:
        SetBit(vdma2.data.status, VDMA_STATUS_DRQ(drqId - 4));
        break;
    case 4:
    default:
        break;
    }
    if (vdma1.data.status & VDMA_STATUS_DRQS) {
        SetBit(vdma2.data.status, VDMA_STATUS_DRQ(0));
    } else {
        ClrBit(vdma2.data.status, VDMA_STATUS_DRQ(0));
    }
}

void vdmaAddDevice(t_nubit8 drqId, t_faddrcc fpReadDevice,
                   t_faddrcc fpWriteDevice, t_faddrcc fpCloseDevice) {
    switch (drqId) {
    case 0:
    case 1:
    case 2:
    case 3:
        vdma1.connect.fpReadDevice[drqId] = fpReadDevice;
        vdma1.connect.fpWriteDevice[drqId] = fpWriteDevice;
        vdma1.connect.fpCloseDevice[drqId] = fpCloseDevice;
        break;
    case 5:
    case 6:
    case 7:
        vdma2.connect.fpReadDevice[drqId - 4] = fpReadDevice;
        vdma2.connect.fpWriteDevice[drqId - 4] = fpWriteDevice;
        vdma2.connect.fpCloseDevice[drqId - 4] = fpCloseDevice;
        break;
    case 4:
    default:
        break;
    }
}

void vdmaInit() {
    MEMSET((void *)(&vlatch), Zero8, sizeof(t_latch));
    MEMSET((void *)(&vdma1), Zero8, sizeof(t_dma));
    MEMSET((void *)(&vdma2), Zero8, sizeof(t_dma));

    vportAddRead(0x0000, (t_faddrcc) io_read_0000);
    vportAddRead(0x0001, (t_faddrcc) io_read_0001);
    vportAddRead(0x0002, (t_faddrcc) io_read_0002);
    vportAddRead(0x0003, (t_faddrcc) io_read_0003);
    vportAddRead(0x0004, (t_faddrcc) io_read_0004);
    vportAddRead(0x0005, (t_faddrcc) io_read_0005);
    vportAddRead(0x0006, (t_faddrcc) io_read_0006);
    vportAddRead(0x0007, (t_faddrcc) io_read_0007);
    vportAddRead(0x0008, (t_faddrcc) io_read_0008);
    vportAddRead(0x000d, (t_faddrcc) io_read_000D);

    vportAddWrite(0x0000, (t_faddrcc) io_write_0000);
    vportAddWrite(0x0001, (t_faddrcc) io_write_0001);
    vportAddWrite(0x0002, (t_faddrcc) io_write_0002);
    vportAddWrite(0x0003, (t_faddrcc) io_write_0003);
    vportAddWrite(0x0004, (t_faddrcc) io_write_0004);
    vportAddWrite(0x0005, (t_faddrcc) io_write_0005);
    vportAddWrite(0x0006, (t_faddrcc) io_write_0006);
    vportAddWrite(0x0007, (t_faddrcc) io_write_0007);
    vportAddWrite(0x0008, (t_faddrcc) io_write_0008);
    vportAddWrite(0x0009, (t_faddrcc) io_write_0009);
    vportAddWrite(0x000a, (t_faddrcc) io_write_000A);
    vportAddWrite(0x000b, (t_faddrcc) io_write_000B);
    vportAddWrite(0x000c, (t_faddrcc) io_write_000C);
    vportAddWrite(0x000d, (t_faddrcc) io_write_000D);
    vportAddWrite(0x000e, (t_faddrcc) io_write_000E);
    vportAddWrite(0x000f, (t_faddrcc) io_write_000F);

    vportAddRead(0x0081, (t_faddrcc) io_read_0081);
    vportAddRead(0x0082, (t_faddrcc) io_read_0082);
    vportAddRead(0x0083, (t_faddrcc) io_read_0083);
    vportAddRead(0x0087, (t_faddrcc) io_read_0087);
    vportAddRead(0x0089, (t_faddrcc) io_read_0089);
    vportAddRead(0x008a, (t_faddrcc) io_read_008A);
    vportAddRead(0x008b, (t_faddrcc) io_read_008B);
    vportAddRead(0x008f, (t_faddrcc) io_read_008F);

    vportAddWrite(0x0081, (t_faddrcc) io_write_0081);
    vportAddWrite(0x0082, (t_faddrcc) io_write_0082);
    vportAddWrite(0x0083, (t_faddrcc) io_write_0083);
    vportAddWrite(0x0087, (t_faddrcc) io_write_0087);
    vportAddWrite(0x0089, (t_faddrcc) io_write_0089);
    vportAddWrite(0x008a, (t_faddrcc) io_write_008A);
    vportAddWrite(0x008b, (t_faddrcc) io_write_008B);
    vportAddWrite(0x008f, (t_faddrcc) io_write_008F);

    vportAddRead(0x00c0, (t_faddrcc) io_read_00C0);
    vportAddRead(0x00c2, (t_faddrcc) io_read_00C2);
    vportAddRead(0x00c4, (t_faddrcc) io_read_00C4);
    vportAddRead(0x00c6, (t_faddrcc) io_read_00C6);
    vportAddRead(0x00c8, (t_faddrcc) io_read_00C8);
    vportAddRead(0x00ca, (t_faddrcc) io_read_00CA);
    vportAddRead(0x00cc, (t_faddrcc) io_read_00CC);
    vportAddRead(0x00ce, (t_faddrcc) io_read_00CE);
    vportAddRead(0x00d0, (t_faddrcc) io_read_00D0);
    vportAddRead(0x00da, (t_faddrcc) io_read_00DA);

    vportAddWrite(0x00c0, (t_faddrcc) io_write_00C0);
    vportAddWrite(0x00c2, (t_faddrcc) io_write_00C2);
    vportAddWrite(0x00c4, (t_faddrcc) io_write_00C4);
    vportAddWrite(0x00c6, (t_faddrcc) io_write_00C6);
    vportAddWrite(0x00c8, (t_faddrcc) io_write_00C8);
    vportAddWrite(0x00ca, (t_faddrcc) io_write_00CA);
    vportAddWrite(0x00cc, (t_faddrcc) io_write_00CC);
    vportAddWrite(0x00ce, (t_faddrcc) io_write_00CE);
    vportAddWrite(0x00d0, (t_faddrcc) io_write_00D0);
    vportAddWrite(0x00d2, (t_faddrcc) io_write_00D2);
    vportAddWrite(0x00d4, (t_faddrcc) io_write_00D4);
    vportAddWrite(0x00d6, (t_faddrcc) io_write_00D6);
    vportAddWrite(0x00d8, (t_faddrcc) io_write_00D8);
    vportAddWrite(0x00dc, (t_faddrcc) io_write_00DC);
    vportAddWrite(0x00de, (t_faddrcc) io_write_00DE);

    vbiosAddPost(VDMA_POST);
}

void vdmaReset() {
    MEMSET((void *)(&vlatch.data), Zero8, sizeof(t_latch_data));
    doReset(&vdma1);
    doReset(&vdma2);
}

void vdmaRefresh() {
    t_nubit8 id;
    t_nubit8 realDRQ1, realDRQ2;
    if (GetBit(vdma2.data.command, VDMA_COMMAND_CTRL)) {
        return;
    }
    if (GetBit(vdma2.data.isr, VDMA_ISR_IS)) {
        if (VDMA_GetISR_ISR(vdma2.data.isr)) {
            Execute(&vdma2, VDMA_GetISR_ISR(vdma2.data.isr), True);
        } else if (GetBit(vdma1.data.isr, VDMA_ISR_IS)) {
            Execute(&vdma1, VDMA_GetISR_ISR(vdma1.data.isr), False);
        }
        if (!GetBit(vdma1.data.isr, VDMA_ISR_IS)) {
            vdma2.data.isr = Zero8;
        }
    }
    if (!GetBit(vdma2.data.isr, VDMA_ISR_IS)) {
        realDRQ2 = vdma2.data.request | (VDMA_GetSTATUS_DRQS(vdma2.data.status) & ~vdma2.data.mask);
        if (realDRQ2 == Zero8) {
            return;
        }
        id = GetRegTopId(&vdma2, realDRQ2);
        if (id == 0) {
            if (GetBit(vdma1.data.command, VDMA_COMMAND_CTRL)) {
                return;
            }
            realDRQ1 = vdma1.data.request | (VDMA_GetSTATUS_DRQS(vdma1.data.status) & ~vdma1.data.mask);
            if (realDRQ1 == Zero8) {
                return;
            }
            id = GetRegTopId(&vdma1, realDRQ1);
            VDMA_SetISR(vdma2.data.isr, 0);
            VDMA_SetISR(vdma1.data.isr, id);
            Execute(&vdma1, id, False);
            if (!GetBit(vdma1.data.isr, VDMA_ISR_IS)) {
                vdma2.data.isr = Zero8;
            }
            if (!VDMA_GetSTATUS_DRQS(vdma1.data.status)) {
                ClrBit(vdma2.data.status, VDMA_STATUS_DRQ(0));
            }
            if (!vdma1.data.request) {
                ClrBit(vdma2.data.request, VDMA_REQUEST_DRQ(0));
            }
        } else {
            VDMA_SetISR(vdma2.data.isr, id);
            Execute(&vdma2, id, True);
        }
    }
}

void vdmaFinal() {}

static void printDma(t_dma *rdma) {
    t_nubit8 i;
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
