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
    MEMSET((void*) qdcgaGetTextMemAddrPageCur, 0x00, qdcgaGetPageSize);
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
    } else if (qdcgaVarCursorPosRow(page) < vvadp.colSize -1) {
        qdcgaVarCursorPosCol(page) = 0;
        qdcgaVarCursorPosRow(page)++;
    }
}

static void CursorNewLine(t_nubit8 page) {
    qdcgaVarCursorPosCol(page) = 0;
    if (qdcgaVarCursorPosRow(page) < vvadp.colSize - 1) {
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
    if (qdcgaVarCursorPosRow(page) < vvadp.colSize - 1) {
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

t_bool deviceConnectDisplayGetCursorVisible() {
    /* qdcgaGetCursorVisible; */
    return (qdcgaVarCursorTop < qdcgaVarCursorBottom);
}
t_bool deviceConnectDisplayGetCursorChange() {
    if (vvadp.oldCurPosX != qdcgaVarCursorPosRow(qdcgaVarPageNum) ||
            vvadp.oldCurPosY != qdcgaVarCursorPosCol(qdcgaVarPageNum) ||
            vvadp.oldCurTop  != qdcgaVarCursorTop ||
            vvadp.oldCurBottom != qdcgaVarCursorBottom) {
        vvadp.oldCurPosX = qdcgaVarCursorPosRow(qdcgaVarPageNum);
        vvadp.oldCurPosY = qdcgaVarCursorPosCol(qdcgaVarPageNum);
        vvadp.oldCurTop  = qdcgaVarCursorTop;
        vvadp.oldCurBottom = qdcgaVarCursorBottom;
        return 1;
    } else {
        return 0;
    }
}
t_bool deviceConnectDisplayGetBufferChange() {
    if (MEMCMP((void *) vvadp.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize)) {
        MEMCPY((void *) vvadp.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
        return 1;
    } else {
        return 0;
    }
}
t_nubit16 deviceConnectDisplayGetRowSize() {
    return qdcgaVarRowSize;
}
t_nubit16 deviceConnectDisplayGetColSize() {
    return vvadp.colSize;
}
t_nubit8 deviceConnectDisplayGetCursorTop() {
    return qdcgaVarCursorTop;
}
t_nubit8 deviceConnectDisplayGetCursorBottom() {
    return qdcgaVarCursorBottom;
}
t_nubit8 deviceConnectDisplayGetCurrentCursorPosX() {
    return qdcgaVarCursorPosRow(qdcgaVarPageNum);
}
t_nubit8 deviceConnectDisplayGetCurrentCursorPosY() {
    return qdcgaVarCursorPosCol(qdcgaVarPageNum);
}
t_nubit8 deviceConnectDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y) {
    return qdcgaVarChar(qdcgaVarPageNum, x, y);
}
t_nubit8 deviceConnectDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y) {
    return qdcgaVarCharProp(qdcgaVarPageNum, x, y);
}

static void qdcgaSetDisplayMode() {
    qdcgaVarMode = _al;
    qdcgaVarPageNum = 0x00;
    qdcgaVarPageOffset = 0x00;
    qdcgaVarCursorPosRow(qdcgaVarPageNum) = 0x00;
    qdcgaVarCursorPosCol(qdcgaVarPageNum) = 0x00;
    switch (_al) {
    case 0x00:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        vvadp.colSize   = 0x19;
        vvadp.flagColor = 0;
        platformDisplaySetScreen();
        break;
    case 0x01:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        vvadp.colSize   = 0x19;
        vvadp.flagColor = 1;
        platformDisplaySetScreen();
        break;
    case 0x02:
    case 0x07:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        vvadp.colSize   = 0x19;
        vvadp.flagColor = 0;
        platformDisplaySetScreen();
        break;
    case 0x03:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        vvadp.colSize   = 0x19;
        vvadp.flagColor = 1;
        platformDisplaySetScreen();
        break;
    default:
        break;
    }
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    MEMCPY((void *) vvadp.bufcomp, (void *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
    ClearTextMemory();
}

static void qdcgaSetCursorShape() {
    qdcgaVarCursorBottom = _cl; /* & 0x0f; */
    qdcgaVarCursorTop    = _ch; /* & 0x0f; */
}

static void qdcgaSetCursorPos() {
    /*    qdcgaVarPageNum = _bh;
        qdcgaVarPageOffset = _bh * qdcgaModeBufSize[qdcgaVarMode]; */
    qdcgaVarCursorPosRow(_bh) = _dh;
    qdcgaVarCursorPosCol(_bh) = _dl;
}

static void qdcgaGetCursorPos() {
    /*    qdcgaVarPageNum = _bh;
        qdcgaVarPageOffset = _bh * qdcgaModeBufSize[qdcgaVarMode]; */
    _dh = qdcgaVarCursorPosRow(_bh);
    _dl = qdcgaVarCursorPosCol(_bh);
    _ch = qdcgaVarCursorTop;
    _cl = qdcgaVarCursorBottom;
}

static void qdcgaSetDisplayPage() {
    qdcgaVarPageNum = _al;
    qdcgaVarPageOffset = _al * qdcgaModeBufSize[qdcgaVarMode];
}

static void qdcgaScrollUp() {
    t_nsbitcc i, j;
    if (!_al) {
        for (i = _ch; i <= _dh; ++i) {
            for (j = _cl; j <= _dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = ' ';
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = _bh;
            }
        }
    } else {
        for (i = 0; i < _dh - _ch; ++i) {
            for (j = _cl; j <= _dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + _ch), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + _ch + _al), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch + _al), j);
            }
        }
        for (i = 0; i < _al; ++i) {
            for (j = _cl; j <= _dl; ++j) {
                qdcgaVarChar(0, (i + _dh - _al + 1), j) = ' ';
                qdcgaVarCharProp(0, (i + _dh - _al + 1), j) = _bh;
            }
        }
    }
}

static void qdcgaScrollDown() {
    t_nsbitcc i, j;
    if (!_al) {
        for (i = 0; i < vvadp.colSize; ++i) {
            for (j = 0; j < qdcgaVarRowSize; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = 0x20;
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = _bh;
            }
        }
    } else {
        for (i = _dh - _ch - 1; i >=0; --i) {
            for (j = _cl; j <= _dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + _ch + _al), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + _ch), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch + _al), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch), j);
            }
        }
        for (i = _al - 1; i >= 0; --i) {
            for (j = _cl; j <= _dl; ++j) {
                qdcgaVarChar(0, (i + _ch), j) = ' ';
                qdcgaVarCharProp(0, (i + _ch), j) = _bh;
            }
        }
    }
}

static void qdcgaGetCharProp() {
    _al = qdcgaVarChar(_bh, qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
    _ah = qdcgaVarCharProp(_bh, qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}

static void qdcgaDisplayCharProp9() {
    InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x00, _bl, _bh,
                 qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}

static void qdcgaDisplayCharPropE() {
    InsertString((t_vaddrcc)(&_al), 0x01, 0x01, 0x01, _bl, _bh,
                 qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}

static void qdcgaDisplayChar() {
    InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x01, 0x0f, _bh,
                 qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}

static void qdcgaGetAdapterStatus() {
    _ah = (t_nubit8)qdcgaVarRowSize;
    _al = qdcgaVarMode;
    _bh = qdcgaVarPageNum;
}

static void qdcgaGenerateChar() {
    if (_al == 0x30) {
        switch (_bh) {
        case 0x00:
            _bp = vramRealWord(0x0000, 0x001f * 4 + 0);
            _es = vramRealWord(0x0000, 0x001f * 4 + 2);
            break;
        case 0x01:
            _bp = vramRealWord(0x0000, 0x0043 * 4 + 0);
            _es = vramRealWord(0x0000, 0x0043 * 4 + 2);
            break;
        default:
            break;
        }
        _cx = 0x0010;
        _dl = vvadp.colSize - 0x01;
    }
}

static void qdcgaGetAdapterInfo() {
    _bh = 0x00;
    _bl = 0x03;
    _cl = 0x07;
    /*    _ch = 0x??; */
}

static void qdcgaDisplayStr() {
    InsertString(vramGetRealAddr(_es, _bp), _cl, 0x00, 0x01,
                 _bl, _bh, _dh,_dl);
}

static void INT_10() {
    switch (_ah) {
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
        if (_bl == 0x10)
            qdcgaGetAdapterInfo();
        break;
    case 0x13:
        qdcgaDisplayStr();
        break;
    case 0x1a:
        if (_al == 0x00) {
            _al = 0x1a;
            _bh = 0x00;
            _bl = 0x08;
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
    vvadp.colSize = 0x19;
    vvadp.flagColor = 1;

    qdcgaVarPageNum = 0x00;
    qdcgaVarMode = 0x03;
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    qdcgaVarCursorPosRow(0) = 0x05;
    qdcgaVarCursorPosCol(0) = 0x00;
    qdcgaVarCursorTop       = 0x06;
    qdcgaVarCursorBottom    = 0x07;
    vvadp.oldCurPosX = vvadp.oldCurPosY = 0x00;
    vvadp.oldCurTop  = vvadp.oldCurBottom = 0x00;
}
