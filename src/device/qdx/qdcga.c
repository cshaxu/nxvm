/* Copyright 2012-2014 Neko. */

/* QDCGA implements quick and dirty video control routines. */

#include "../../utils.h"

#include "../../platform/platform.h"
#include "../vcpu.h"

#include "qdx.h"
#include "qdcga.h"

t_nubit32 qdcgaModeBufSize[0x14] = {
    2048, 2048, 4096, 4096, 16000, 16000, 16000,
    4096, 16000, 32000, 32000,
    0, 0, 32000, 64000,
    28000, 224000, 38400, 153600, 64000
};

static void ClearTextMemory() {
    MEMSET((void *) qdcgaGetTextMemAddrPageCur, 0x00, qdcgaGetPageSize);
}
static void CursorBackward(t_nubit8 page) {
    if (qdcgaVarCursorPosCol(page)) {
        qdcgaVarCursorPosCol(page)--;
    } else if (qdcgaVarCursorPosRow(page)) {
        qdcgaVarCursorPosCol(page) = qdcgaVarRowSize - 1;
        qdcgaVarCursorPosRow(page)--;
    }
}
static void CursorForward(t_nubit8 page) {
    if (qdcgaVarCursorPosCol(page) < qdcgaVarRowSize -1) {
        qdcgaVarCursorPosCol(page)++;
    } else if (qdcgaVarCursorPosRow(page) < vvadp.data.colSize -1) {
        qdcgaVarCursorPosCol(page) = 0;
        qdcgaVarCursorPosRow(page)++;
    }
}
static void CursorNewLine(t_nubit8 page) {
    qdcgaVarCursorPosCol(page) = 0;
    if (qdcgaVarCursorPosRow(page) < vvadp.data.colSize - 1) {
        qdcgaVarCursorPosRow(page)++;
    } else {
        /* move up video memory content */
        MEMCPY((void *) qdcgaGetCharAddr(page, 0, 0),
               (void *) qdcgaGetCharAddr(page, 1, 0),
               qdcgaGetPageSize - 2 * qdcgaVarRowSize);
        /* zero the last line */
        MEMSET((void *) qdcgaGetCharAddr(page + 1, -1, 0),
               0x00, 2 * qdcgaVarRowSize);
    }
}
static void CursorCarriageReturn(t_nubit8 page) {
    qdcgaVarCursorPosCol(page) = 0;
}
static void CursorLineFeed(t_nubit8 page) {
    if (qdcgaVarCursorPosRow(page) < vvadp.data.colSize - 1) {
        qdcgaVarCursorPosRow(page)++;
    } else {
        /* move up video memory content */
        MEMCPY((void *) qdcgaGetCharAddr(page, 0, 0),
               (void *) qdcgaGetCharAddr(page, 1, 0),
               qdcgaGetPageSize - 2 * qdcgaVarRowSize);
        /* zero the last line */
        MEMSET((void *) qdcgaGetCharAddr(page + 1, -1, 0),
               0x00, 2 * qdcgaVarRowSize);
    }
}
static void InsertString(t_vaddrcc string, t_nubitcc count, t_bool flagdup,
                         t_bool move, t_nubit8 charprop, t_nubit8 page, t_nubit8 x, t_nubit8 y) {
    t_nubitcc i;
    qdcgaVarCursorPosRow(page) = x;
    qdcgaVarCursorPosCol(page) = y;
    for (i = 0; i < count; ++i) {
        switch (d_nubit8(string)) {
        case 0x07:
            /* bell */
            break;
        case 0x08:
            /* backspace */
            CursorBackward(page);
            break;
        case 0x0a:
            /* new line */
            CursorLineFeed(page);
            break;
        case 0x0d:
            CursorCarriageReturn(page);
            break;
        default:
            qdcgaVarChar(page, qdcgaVarCursorPosRow(page),
                         qdcgaVarCursorPosCol(page)) = d_nubit8(string);
            qdcgaVarCharProp(page, qdcgaVarCursorPosRow(page),
                             qdcgaVarCursorPosCol(page)) = charprop;
            CursorForward(page);
            break;
        }
        if (!flagdup) {
            string++;
        }
    }
    if (!move) {
        qdcgaVarCursorPosRow(page) = x;
        qdcgaVarCursorPosCol(page) = y;
    }
}

static void qdcgaSetDisplayMode() {
    qdcgaVarMode = vcpu.data.al;
    qdcgaVarPageNum = 0x00;
    qdcgaVarPageOffset = 0x00;
    qdcgaVarCursorPosRow(qdcgaVarPageNum) = 0x00;
    qdcgaVarCursorPosCol(qdcgaVarPageNum) = 0x00;
    switch (vcpu.data.al) {
    case 0x00:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        vvadp.data.colSize   = 0x19;
        vvadp.data.flagColor = False;
        platformDisplaySetScreen();
        break;
    case 0x01:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        vvadp.data.colSize   = 0x19;
        vvadp.data.flagColor = True;
        platformDisplaySetScreen();
        break;
    case 0x02:
    case 0x07:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        vvadp.data.colSize   = 0x19;
        vvadp.data.flagColor = False;
        platformDisplaySetScreen();
        break;
    case 0x03:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        vvadp.data.colSize   = 0x19;
        vvadp.data.flagColor = True;
        platformDisplaySetScreen();
        break;
    default:
        break;
    }
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    MEMCPY((void *) vvadp.data.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
    ClearTextMemory();
}
static void qdcgaSetCursorShape() {
    qdcgaVarCursorBottom = vcpu.data.cl; /* & 0x0f; */
    qdcgaVarCursorTop    = vcpu.data.ch; /* & 0x0f; */
}
static void qdcgaSetCursorPos() {
    /* qdcgaVarPageNum = vcpu.data.bh;
     * qdcgaVarPageOffset = vcpu.data.bh * qdcgaModeBufSize[qdcgaVarMode]; */
    qdcgaVarCursorPosRow(vcpu.data.bh) = vcpu.data.dh;
    qdcgaVarCursorPosCol(vcpu.data.bh) = vcpu.data.dl;
}
static void qdcgaGetCursorPos() {
    /* qdcgaVarPageNum = vcpu.data.bh;
     * qdcgaVarPageOffset = vcpu.data.bh * qdcgaModeBufSize[qdcgaVarMode]; */
    vcpu.data.dh = qdcgaVarCursorPosRow(vcpu.data.bh);
    vcpu.data.dl = qdcgaVarCursorPosCol(vcpu.data.bh);
    vcpu.data.ch = qdcgaVarCursorTop;
    vcpu.data.cl = qdcgaVarCursorBottom;
}
static void qdcgaSetDisplayPage() {
    qdcgaVarPageNum = vcpu.data.al;
    qdcgaVarPageOffset = vcpu.data.al * qdcgaModeBufSize[qdcgaVarMode];
}
static void qdcgaScrollUp() {
    t_nubitcc i, j;
    if (! vcpu.data.al) {
        for (i = vcpu.data.ch; i <= vcpu.data.dh; ++i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = ' ';
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = vcpu.data.bh;
            }
        }
    } else {
        for (i = 0; i < (t_nubitcc)(vcpu.data.dh - vcpu.data.ch); ++i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + vcpu.data.ch), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + vcpu.data.ch + vcpu.data.al), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + vcpu.data.ch), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + vcpu.data.ch + vcpu.data.al), j);
            }
        }
        for (i = 0; i < vcpu.data.al; ++i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(0, (i + vcpu.data.dh - vcpu.data.al + 1), j) = ' ';
                qdcgaVarCharProp(0, (i + vcpu.data.dh - vcpu.data.al + 1), j) = vcpu.data.bh;
            }
        }
    }
}
static void qdcgaScrollDown() {
    t_nubitcc i, j;
    if (! vcpu.data.al) {
        for (i = 0; i < vvadp.data.colSize; ++i) {
            for (j = 0; j < qdcgaVarRowSize; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = 0x20;
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = vcpu.data.bh;
            }
        }
    } else {
        for (i = 0; i < (t_nubitcc)(vcpu.data.dh - vcpu.data.ch); ++i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (vcpu.data.dh - 1 - i + vcpu.data.al), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (vcpu.data.dh - 1 - i), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (vcpu.data.dh - 1 - i + vcpu.data.al), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (vcpu.data.dh - 1 - i), j);
            }
        }
        for (i = 0; i < vcpu.data.al; ++i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(0, (vcpu.data.al - 1 - i + vcpu.data.ch), j) = ' ';
                qdcgaVarCharProp(0, (vcpu.data.al - 1 - i + vcpu.data.ch), j) = vcpu.data.bh;
            }
        }
        /* for (i = vcpu.data.dh - vcpu.data.ch - 1; i >= 0; --i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + vcpu.data.ch + vcpu.data.al), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + vcpu.data.ch), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + vcpu.data.ch + vcpu.data.al), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + vcpu.data.ch), j);
            }
        }
        for (i = vcpu.data.al - 1; i >= 0; --i) {
            for (j = vcpu.data.cl; j <= vcpu.data.dl; ++j) {
                qdcgaVarChar(0, (i + vcpu.data.ch), j) = ' ';
                qdcgaVarCharProp(0, (i + vcpu.data.ch), j) = vcpu.data.bh;
            }
        } */
    }
}
static void qdcgaGetCharProp() {
    vcpu.data.al = qdcgaVarChar(vcpu.data.bh, qdcgaVarCursorPosRow(vcpu.data.bh), qdcgaVarCursorPosCol(vcpu.data.bh));
    vcpu.data.ah = qdcgaVarCharProp(vcpu.data.bh, qdcgaVarCursorPosRow(vcpu.data.bh), qdcgaVarCursorPosCol(vcpu.data.bh));
}
static void qdcgaDisplayCharProp9() {
    InsertString((t_vaddrcc)(&vcpu.data.al), vcpu.data.cx, 0x01, 0x00, vcpu.data.bl, vcpu.data.bh,
                 qdcgaVarCursorPosRow(vcpu.data.bh), qdcgaVarCursorPosCol(vcpu.data.bh));
}
static void qdcgaDisplayCharPropE() {
    InsertString((t_vaddrcc)(&vcpu.data.al), 0x01, 0x01, 0x01, vcpu.data.bl, vcpu.data.bh,
                 qdcgaVarCursorPosRow(vcpu.data.bh), qdcgaVarCursorPosCol(vcpu.data.bh));
}
static void qdcgaDisplayChar() {
    InsertString((t_vaddrcc)(&vcpu.data.al), vcpu.data.cx, 0x01, 0x01, 0x0f, vcpu.data.bh,
                 qdcgaVarCursorPosRow(vcpu.data.bh), qdcgaVarCursorPosCol(vcpu.data.bh));
}
static void qdcgaGetAdapterStatus() {
    vcpu.data.ah = (t_nubit8) qdcgaVarRowSize;
    vcpu.data.al = qdcgaVarMode;
    vcpu.data.bh = qdcgaVarPageNum;
}
static void qdcgaGenerateChar() {
    if (vcpu.data.al == 0x30) {
        switch (vcpu.data.bh) {
        case 0x00:
            vcpu.data.bp = vramRealWord(0x0000, 0x001f * 4 + 0);
            vcpu.data.es.selector = vramRealWord(0x0000, 0x001f * 4 + 2);
            break;
        case 0x01:
            vcpu.data.bp = vramRealWord(0x0000, 0x0043 * 4 + 0);
            vcpu.data.es.selector = vramRealWord(0x0000, 0x0043 * 4 + 2);
            break;
        default:
            break;
        }
        vcpu.data.cx = 0x0010;
        vcpu.data.dl = vvadp.data.colSize - 0x01;
    }
}
static void qdcgaGetAdapterInfo() {
    vcpu.data.bh = 0x00;
    vcpu.data.bl = 0x03;
    vcpu.data.cl = 0x07;
    /*     vcpu.data.ch = 0x??; */
}
static void qdcgaDisplayStr() {
    InsertString(vramGetRealAddr(vcpu.data.es.selector, vcpu.data.bp), vcpu.data.cl, 0x00, 0x01,
                 vcpu.data.bl, vcpu.data.bh, vcpu.data.dh, vcpu.data.dl);
}

static void INT_10() {
    switch (vcpu.data.ah) {
    case 0x00:
        qdcgaSetDisplayMode();
        break;
    case 0x01:
        qdcgaSetCursorShape();
        break;
    case 0x02:
        qdcgaSetCursorPos();
        break;
    case 0x03:
        qdcgaGetCursorPos();
        break;
    case 0x04:
        break;
    case 0x05:
        qdcgaSetDisplayPage();
        break;
    case 0x06:
        qdcgaScrollUp();
        break;
    case 0x07:
        qdcgaScrollDown();
        break;
    case 0x08:
        qdcgaGetCharProp();
        break;
    case 0x09:
        qdcgaDisplayCharProp9();
        break;
    case 0x0a:
        qdcgaDisplayChar();
        break;
    case 0x0e:
        qdcgaDisplayCharPropE();
        break;
    case 0x0f:
        qdcgaGetAdapterStatus();
        break;
    case 0x10:
        break;
    case 0x11:
        qdcgaGenerateChar();
        break;
    case 0x12:
        if (vcpu.data.bl == 0x10)
            qdcgaGetAdapterInfo();
        break;
    case 0x13:
        qdcgaDisplayStr();
        break;
    case 0x1a:
        if (vcpu.data.al == 0x00) {
            vcpu.data.al = 0x1a;
            vcpu.data.bh = 0x00;
            vcpu.data.bl = 0x08;
        }
        break;
    default:
        break;
    }
}

void qdcgaInit() {
    qdxTable[0x10] = (t_faddrcc) INT_10; /* soft cga*/
}
void qdcgaReset() {
    /* 80 x 25 */
    qdcgaVarRowSize = 0x50;
    vvadp.data.colSize = 0x19;
    vvadp.data.flagColor = True;

    qdcgaVarPageNum = 0x00;
    qdcgaVarMode = 0x03;
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    qdcgaVarCursorPosRow(0) = 0x05;
    qdcgaVarCursorPosCol(0) = 0x00;
    qdcgaVarCursorTop       = 0x06;
    qdcgaVarCursorBottom    = 0x07;
    vvadp.data.oldCurPosX = vvadp.data.oldCurPosY = 0x00;
    vvadp.data.oldCurTop  = vvadp.data.oldCurBottom = 0x00;
}

int deviceConnectDisplayGetCursorVisible() {
    /* qdcgaGetCursorVisible; */
    return (qdcgaVarCursorTop < qdcgaVarCursorBottom);
}
int deviceConnectDisplayGetCursorChange() {
    if (vvadp.data.oldCurPosX != qdcgaVarCursorPosRow(qdcgaVarPageNum) ||
            vvadp.data.oldCurPosY != qdcgaVarCursorPosCol(qdcgaVarPageNum) ||
            vvadp.data.oldCurTop  != qdcgaVarCursorTop ||
            vvadp.data.oldCurBottom != qdcgaVarCursorBottom) {
        vvadp.data.oldCurPosX = qdcgaVarCursorPosRow(qdcgaVarPageNum);
        vvadp.data.oldCurPosY = qdcgaVarCursorPosCol(qdcgaVarPageNum);
        vvadp.data.oldCurTop  = qdcgaVarCursorTop;
        vvadp.data.oldCurBottom = qdcgaVarCursorBottom;
        return True;
    } else {
        return False;
    }
}
int deviceConnectDisplayGetBufferChange() {
    if (MEMCMP((void *) vvadp.data.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize)) {
        MEMCPY((void *) vvadp.data.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
        return True;
    } else {
        return False;
    }
}
uint16_t deviceConnectDisplayGetRowSize() {
    return qdcgaVarRowSize;
}
uint16_t deviceConnectDisplayGetColSize() {
    return vvadp.data.colSize;
}
uint8_t deviceConnectDisplayGetCursorTop() {
    return qdcgaVarCursorTop;
}
uint8_t deviceConnectDisplayGetCursorBottom() {
    return qdcgaVarCursorBottom;
}
uint8_t deviceConnectDisplayGetCurrentCursorPosX() {
    return qdcgaVarCursorPosRow(qdcgaVarPageNum);
}
uint8_t deviceConnectDisplayGetCurrentCursorPosY() {
    return qdcgaVarCursorPosCol(qdcgaVarPageNum);
}
uint8_t deviceConnectDisplayGetCurrentChar(uint8_t x, uint8_t y) {
    return qdcgaVarChar(qdcgaVarPageNum, x, y);
}
uint8_t deviceConnectDisplayGetCurrentCharProp(uint8_t x, uint8_t y) {
    return qdcgaVarCharProp(qdcgaVarPageNum, x, y);
}
