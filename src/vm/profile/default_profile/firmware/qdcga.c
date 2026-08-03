/* Copyright 2012-2014 Neko. */

/* QDCGA implements quick and dirty video control routines. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/display_interface.h"

#include "core/machine/vadp.h"

#include "core/machine/cpu.h"


#include "vm/profile/default_profile/firmware/context.h"

#include "vm/profile/default_profile/firmware/qdx.h"

#include "qdcga.h"

#define profile_cpu (*profile->execution->cpu)
#define profile_ram (profile->ram)
#define profile_vadp (*profile->vadp)

static const ntvdm64_type_unsigned_32 qdcgaModeBufSize[0x14] = {
    2048, 2048, 4096, 4096, 16000, 16000, 16000,
    4096, 16000, 32000, 32000,
    0, 0, 32000, 64000,
    28000, 224000, 38400, 153600, 64000
};

static C_VOID ClearTextMemory(vm_profile_default_context *profile) {
    STD_MEMSET((C_VOID *) qdcgaGetTextMemAddrPageCur, 0x00, qdcgaGetPageSize);
}
static C_VOID CursorBackward(vm_profile_default_context *profile, ntvdm64_type_unsigned_8 page) {
    if (qdcgaVarCursorPosCol(page)) {
        qdcgaVarCursorPosCol(page)--;
    } else if (qdcgaVarCursorPosRow(page)) {
        qdcgaVarCursorPosCol(page) = qdcgaVarRowSize - 1;
        qdcgaVarCursorPosRow(page)--;
    }
}
static C_VOID CursorForward(vm_profile_default_context *profile, ntvdm64_type_unsigned_8 page) {
    if (qdcgaVarCursorPosCol(page) < qdcgaVarRowSize -1) {
        qdcgaVarCursorPosCol(page)++;
    } else if (qdcgaVarCursorPosRow(page) < profile_vadp.data.colSize -1) {
        qdcgaVarCursorPosCol(page) = 0;
        qdcgaVarCursorPosRow(page)++;
    }
}
static C_VOID CursorNewLine(vm_profile_default_context *profile, ntvdm64_type_unsigned_8 page) {
    qdcgaVarCursorPosCol(page) = 0;
    if (qdcgaVarCursorPosRow(page) < profile_vadp.data.colSize - 1) {
        qdcgaVarCursorPosRow(page)++;
    } else {
        /* move up video memory content */
        STD_MEMCPY((C_VOID *) qdcgaGetCharAddr(page, 0, 0),
               (C_VOID *) qdcgaGetCharAddr(page, 1, 0),
               qdcgaGetPageSize - 2 * qdcgaVarRowSize);
        /* zero the last line */
        STD_MEMSET((C_VOID *) qdcgaGetCharAddr(page + 1, -1, 0),
               0x00, 2 * qdcgaVarRowSize);
    }
}
static C_VOID CursorCarriageReturn(vm_profile_default_context *profile,
    ntvdm64_type_unsigned_8 page) {
    qdcgaVarCursorPosCol(page) = 0;
}
static C_VOID CursorLineFeed(vm_profile_default_context *profile, ntvdm64_type_unsigned_8 page) {
    if (qdcgaVarCursorPosRow(page) < profile_vadp.data.colSize - 1) {
        qdcgaVarCursorPosRow(page)++;
    } else {
        /* move up video memory content */
        STD_MEMCPY((C_VOID *) qdcgaGetCharAddr(page, 0, 0),
               (C_VOID *) qdcgaGetCharAddr(page, 1, 0),
               qdcgaGetPageSize - 2 * qdcgaVarRowSize);
        /* zero the last line */
        STD_MEMSET((C_VOID *) qdcgaGetCharAddr(page + 1, -1, 0),
               0x00, 2 * qdcgaVarRowSize);
    }
}
static C_VOID InsertString(vm_profile_default_context *profile, ntvdm64_type_virtual_address string,
                         ntvdm64_type_native_unsigned count, ntvdm64_type_bool flagdup,
                         ntvdm64_type_bool move, ntvdm64_type_unsigned_8 charprop, ntvdm64_type_unsigned_8 page, ntvdm64_type_unsigned_8 x, ntvdm64_type_unsigned_8 y) {
    ntvdm64_type_native_unsigned i;
    qdcgaVarCursorPosRow(page) = x;
    qdcgaVarCursorPosCol(page) = y;
    for (i = 0; i < count; ++i) {
        switch (NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(string)) {
        case 0x07:
            /* bell */
            break;
        case 0x08:
            /* backspace */
            CursorBackward(profile, page);
            break;
        case 0x0a:
            /* new line */
            CursorLineFeed(profile, page);
            break;
        case 0x0d:
            CursorCarriageReturn(profile, page);
            break;
        default:
            qdcgaVarChar(page, qdcgaVarCursorPosRow(page),
                         qdcgaVarCursorPosCol(page)) = NTVDM64_TYPE_DEREFERENCE_UNSIGNED_8(string);
            qdcgaVarCharProp(page, qdcgaVarCursorPosRow(page),
                             qdcgaVarCursorPosCol(page)) = charprop;
            CursorForward(profile, page);
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

static C_VOID qdcgaSetDisplayMode(vm_profile_default_context *profile) {
    qdcgaVarMode = profile_cpu.data.al;
    qdcgaVarPageNum = 0x00;
    qdcgaVarPageOffset = 0x00;
    qdcgaVarCursorPosRow(qdcgaVarPageNum) = 0x00;
    qdcgaVarCursorPosCol(qdcgaVarPageNum) = 0x00;
    switch (profile_cpu.data.al) {
    case 0x00:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        profile_vadp.data.colSize   = 0x19;
        profile_vadp.data.flagColor = NTVDM64_TYPE_FALSE;
        core_machine_display_notify_mode_changed_to(profile->display_provider);
        break;
    case 0x01:
        /* 40 x 25 */
        qdcgaVarRowSize = 0x28;
        profile_vadp.data.colSize   = 0x19;
        profile_vadp.data.flagColor = NTVDM64_TYPE_TRUE;
        core_machine_display_notify_mode_changed_to(profile->display_provider);
        break;
    case 0x02:
    case 0x07:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        profile_vadp.data.colSize   = 0x19;
        profile_vadp.data.flagColor = NTVDM64_TYPE_FALSE;
        core_machine_display_notify_mode_changed_to(profile->display_provider);
        break;
    case 0x03:
        /* 80 x 25 */
        qdcgaVarRowSize = 0x50;
        profile_vadp.data.colSize   = 0x19;
        profile_vadp.data.flagColor = NTVDM64_TYPE_TRUE;
        core_machine_display_notify_mode_changed_to(profile->display_provider);
        break;
    default:
        break;
    }
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    STD_MEMCPY((C_VOID *) profile_vadp.data.bufcomp, (C_VOID *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
    ClearTextMemory(profile);
}
static C_VOID qdcgaSetCursorShape(vm_profile_default_context *profile) {
    qdcgaVarCursorBottom = profile_cpu.data.cl; /* & 0x0f; */
    qdcgaVarCursorTop    = profile_cpu.data.ch; /* & 0x0f; */
}
static C_VOID qdcgaSetCursorPos(vm_profile_default_context *profile) {
    /* qdcgaVarPageNum = profile_cpu.data.bh;
     * qdcgaVarPageOffset = profile_cpu.data.bh * qdcgaModeBufSize[qdcgaVarMode]; */
    qdcgaVarCursorPosRow(profile_cpu.data.bh) = profile_cpu.data.dh;
    qdcgaVarCursorPosCol(profile_cpu.data.bh) = profile_cpu.data.dl;
}
static C_VOID qdcgaGetCursorPos(vm_profile_default_context *profile) {
    /* qdcgaVarPageNum = profile_cpu.data.bh;
     * qdcgaVarPageOffset = profile_cpu.data.bh * qdcgaModeBufSize[qdcgaVarMode]; */
    profile_cpu.data.dh = qdcgaVarCursorPosRow(profile_cpu.data.bh);
    profile_cpu.data.dl = qdcgaVarCursorPosCol(profile_cpu.data.bh);
    profile_cpu.data.ch = qdcgaVarCursorTop;
    profile_cpu.data.cl = qdcgaVarCursorBottom;
}
static C_VOID qdcgaSetDisplayPage(vm_profile_default_context *profile) {
    qdcgaVarPageNum = profile_cpu.data.al;
    qdcgaVarPageOffset = profile_cpu.data.al * qdcgaModeBufSize[qdcgaVarMode];
}
static C_VOID qdcgaScrollUp(vm_profile_default_context *profile) {
    ntvdm64_type_native_unsigned i, j;
    if (! profile_cpu.data.al) {
        for (i = profile_cpu.data.ch; i <= profile_cpu.data.dh; ++i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = ' ';
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = profile_cpu.data.bh;
            }
        }
    } else {
        for (i = 0; i < (ntvdm64_type_native_unsigned)(profile_cpu.data.dh - profile_cpu.data.ch); ++i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + profile_cpu.data.ch), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + profile_cpu.data.ch + profile_cpu.data.al), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + profile_cpu.data.ch), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + profile_cpu.data.ch + profile_cpu.data.al), j);
            }
        }
        for (i = 0; i < profile_cpu.data.al; ++i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(0, (i + profile_cpu.data.dh - profile_cpu.data.al + 1), j) = ' ';
                qdcgaVarCharProp(0, (i + profile_cpu.data.dh - profile_cpu.data.al + 1), j) = profile_cpu.data.bh;
            }
        }
    }
}
static C_VOID qdcgaScrollDown(vm_profile_default_context *profile) {
    ntvdm64_type_native_unsigned i, j;
    if (! profile_cpu.data.al) {
        for (i = 0; i < profile_vadp.data.colSize; ++i) {
            for (j = 0; j < qdcgaVarRowSize; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, i, j) = 0x20;
                qdcgaVarCharProp(qdcgaVarPageNum, i, j) = profile_cpu.data.bh;
            }
        }
    } else {
        for (i = 0; i < (ntvdm64_type_native_unsigned)(profile_cpu.data.dh - profile_cpu.data.ch); ++i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (profile_cpu.data.dh - 1 - i + profile_cpu.data.al), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (profile_cpu.data.dh - 1 - i), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (profile_cpu.data.dh - 1 - i + profile_cpu.data.al), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (profile_cpu.data.dh - 1 - i), j);
            }
        }
        for (i = 0; i < profile_cpu.data.al; ++i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(0, (profile_cpu.data.al - 1 - i + profile_cpu.data.ch), j) = ' ';
                qdcgaVarCharProp(0, (profile_cpu.data.al - 1 - i + profile_cpu.data.ch), j) = profile_cpu.data.bh;
            }
        }
        /* for (i = profile_cpu.data.dh - profile_cpu.data.ch - 1; i >= 0; --i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(qdcgaVarPageNum, (i + profile_cpu.data.ch + profile_cpu.data.al), j) =
                    qdcgaVarChar(qdcgaVarPageNum, (i + profile_cpu.data.ch), j);
                qdcgaVarCharProp(qdcgaVarPageNum, (i + profile_cpu.data.ch + profile_cpu.data.al), j) =
                    qdcgaVarCharProp(qdcgaVarPageNum, (i + profile_cpu.data.ch), j);
            }
        }
        for (i = profile_cpu.data.al - 1; i >= 0; --i) {
            for (j = profile_cpu.data.cl; j <= profile_cpu.data.dl; ++j) {
                qdcgaVarChar(0, (i + profile_cpu.data.ch), j) = ' ';
                qdcgaVarCharProp(0, (i + profile_cpu.data.ch), j) = profile_cpu.data.bh;
            }
        } */
    }
}
static C_VOID qdcgaGetCharProp(vm_profile_default_context *profile) {
    profile_cpu.data.al = qdcgaVarChar(profile_cpu.data.bh, qdcgaVarCursorPosRow(profile_cpu.data.bh), qdcgaVarCursorPosCol(profile_cpu.data.bh));
    profile_cpu.data.ah = qdcgaVarCharProp(profile_cpu.data.bh, qdcgaVarCursorPosRow(profile_cpu.data.bh), qdcgaVarCursorPosCol(profile_cpu.data.bh));
}
static C_VOID qdcgaDisplayCharProp9(vm_profile_default_context *profile) {
    InsertString(profile, (ntvdm64_type_virtual_address)(&profile_cpu.data.al), profile_cpu.data.cx, 0x01, 0x00, profile_cpu.data.bl, profile_cpu.data.bh,
                 qdcgaVarCursorPosRow(profile_cpu.data.bh), qdcgaVarCursorPosCol(profile_cpu.data.bh));
}
static C_VOID qdcgaDisplayCharPropE(vm_profile_default_context *profile) {
    InsertString(profile, (ntvdm64_type_virtual_address)(&profile_cpu.data.al), 0x01, 0x01, 0x01, profile_cpu.data.bl, profile_cpu.data.bh,
                 qdcgaVarCursorPosRow(profile_cpu.data.bh), qdcgaVarCursorPosCol(profile_cpu.data.bh));
}
static C_VOID qdcgaDisplayChar(vm_profile_default_context *profile) {
    InsertString(profile, (ntvdm64_type_virtual_address)(&profile_cpu.data.al), profile_cpu.data.cx, 0x01, 0x01, 0x0f, profile_cpu.data.bh,
                 qdcgaVarCursorPosRow(profile_cpu.data.bh), qdcgaVarCursorPosCol(profile_cpu.data.bh));
}
static C_VOID qdcgaGetAdapterStatus(vm_profile_default_context *profile) {
    profile_cpu.data.ah = (ntvdm64_type_unsigned_8) qdcgaVarRowSize;
    profile_cpu.data.al = qdcgaVarMode;
    profile_cpu.data.bh = qdcgaVarPageNum;
}
static C_VOID qdcgaGenerateChar(vm_profile_default_context *profile) {
    if (profile_cpu.data.al == 0x30) {
        switch (profile_cpu.data.bh) {
        case 0x00:
            profile_cpu.data.bp = *(ntvdm64_type_unsigned_16 *)QDCGA_MEMORY_ADDRESS(0x0000, 0x001f * 4 + 0);
            profile_cpu.data.es.selector = *(ntvdm64_type_unsigned_16 *)QDCGA_MEMORY_ADDRESS(0x0000, 0x001f * 4 + 2);
            break;
        case 0x01:
            profile_cpu.data.bp = *(ntvdm64_type_unsigned_16 *)QDCGA_MEMORY_ADDRESS(0x0000, 0x0043 * 4 + 0);
            profile_cpu.data.es.selector = *(ntvdm64_type_unsigned_16 *)QDCGA_MEMORY_ADDRESS(0x0000, 0x0043 * 4 + 2);
            break;
        default:
            break;
        }
        profile_cpu.data.cx = 0x0010;
        profile_cpu.data.dl = profile_vadp.data.colSize - 0x01;
    }
}
static C_VOID qdcgaGetAdapterInfo(vm_profile_default_context *profile) {
    profile_cpu.data.bh = 0x00;
    profile_cpu.data.bl = 0x03;
    profile_cpu.data.cl = 0x07;
    /*     profile_cpu.data.ch = 0x??; */
}
static C_VOID qdcgaDisplayStr(vm_profile_default_context *profile) {
    InsertString(profile, (ntvdm64_type_virtual_address)QDCGA_MEMORY_ADDRESS(profile_cpu.data.es.selector, profile_cpu.data.bp), profile_cpu.data.cl, 0x00, 0x01,
                 profile_cpu.data.bl, profile_cpu.data.bh, profile_cpu.data.dh, profile_cpu.data.dl);
}

static C_VOID INT_10(vm_profile_default_context *profile) {
    switch (profile_cpu.data.ah) {
    case 0x00:
        qdcgaSetDisplayMode(profile);
        break;
    case 0x01:
        qdcgaSetCursorShape(profile);
        break;
    case 0x02:
        qdcgaSetCursorPos(profile);
        break;
    case 0x03:
        qdcgaGetCursorPos(profile);
        break;
    case 0x04:
        break;
    case 0x05:
        qdcgaSetDisplayPage(profile);
        break;
    case 0x06:
        qdcgaScrollUp(profile);
        break;
    case 0x07:
        qdcgaScrollDown(profile);
        break;
    case 0x08:
        qdcgaGetCharProp(profile);
        break;
    case 0x09:
        qdcgaDisplayCharProp9(profile);
        break;
    case 0x0a:
        qdcgaDisplayChar(profile);
        break;
    case 0x0e:
        qdcgaDisplayCharPropE(profile);
        break;
    case 0x0f:
        qdcgaGetAdapterStatus(profile);
        break;
    case 0x10:
        break;
    case 0x11:
        qdcgaGenerateChar(profile);
        break;
    case 0x12:
        if (profile_cpu.data.bl == 0x10)
            qdcgaGetAdapterInfo(profile);
        break;
    case 0x13:
        qdcgaDisplayStr(profile);
        break;
    case 0x1a:
        if (profile_cpu.data.al == 0x00) {
            profile_cpu.data.al = 0x1a;
            profile_cpu.data.bh = 0x00;
            profile_cpu.data.bl = 0x08;
        }
        break;
    default:
        break;
    }
}

static C_VOID vm_profile_default_cga_dispatch(vm_profile_default_context *profile)
{
    INT_10(profile);
}

C_VOID vm_profile_default_cga_initialize(t_qdx *qdx) {
    if (qdx != NULL) qdx->table[0x10] = vm_profile_default_cga_dispatch;
}
C_VOID vm_profile_default_cga_reset(vm_profile_default_context *profile) {
    /* 80 x 25 */
    qdcgaVarRowSize = 0x50;
    profile_vadp.data.colSize = 0x19;
    profile_vadp.data.flagColor = NTVDM64_TYPE_TRUE;

    qdcgaVarPageNum = 0x00;
    qdcgaVarMode = 0x03;
    qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
    qdcgaVarCursorPosRow(0) = 0x05;
    qdcgaVarCursorPosCol(0) = 0x00;
    qdcgaVarCursorTop       = 0x06;
    qdcgaVarCursorBottom    = 0x07;
    profile_vadp.data.oldCurPosX = profile_vadp.data.oldCurPosY = 0x00;
    profile_vadp.data.oldCurTop  = profile_vadp.data.oldCurBottom = 0x00;
}

static C_INT vm_profile_default_display_cursor_visible(
    vm_profile_default_context *profile) {
    /* qdcgaGetCursorVisible; */
    return (qdcgaVarCursorTop < qdcgaVarCursorBottom);
}
static C_INT vm_profile_default_display_cursor_changed(
    vm_profile_default_context *profile) {
    if (profile_vadp.data.oldCurPosX != qdcgaVarCursorPosRow(qdcgaVarPageNum) ||
            profile_vadp.data.oldCurPosY != qdcgaVarCursorPosCol(qdcgaVarPageNum) ||
            profile_vadp.data.oldCurTop  != qdcgaVarCursorTop ||
            profile_vadp.data.oldCurBottom != qdcgaVarCursorBottom) {
        profile_vadp.data.oldCurPosX = qdcgaVarCursorPosRow(qdcgaVarPageNum);
        profile_vadp.data.oldCurPosY = qdcgaVarCursorPosCol(qdcgaVarPageNum);
        profile_vadp.data.oldCurTop  = qdcgaVarCursorTop;
        profile_vadp.data.oldCurBottom = qdcgaVarCursorBottom;
        return NTVDM64_TYPE_TRUE;
    } else {
        return NTVDM64_TYPE_FALSE;
    }
}
static C_INT vm_profile_default_display_buffer_changed(
    vm_profile_default_context *profile) {
    if (STD_MEMCMP((C_VOID *) profile_vadp.data.bufcomp, (C_VOID *) qdcgaGetTextMemAddr, qdcgaVarRagenSize)) {
        STD_MEMCPY((C_VOID *) profile_vadp.data.bufcomp, (C_VOID *) qdcgaGetTextMemAddr, qdcgaVarRagenSize);
        return NTVDM64_TYPE_TRUE;
    } else {
        return NTVDM64_TYPE_FALSE;
    }
}
static uint16_t vm_profile_default_display_columns(
    vm_profile_default_context *profile) {
    return qdcgaVarRowSize;
}
static uint16_t vm_profile_default_display_rows(
    vm_profile_default_context *profile) {
    return profile_vadp.data.colSize;
}
static uint8_t vm_profile_default_display_cursor_top(
    vm_profile_default_context *profile) {
    return qdcgaVarCursorTop;
}
static uint8_t vm_profile_default_display_cursor_bottom(
    vm_profile_default_context *profile) {
    return qdcgaVarCursorBottom;
}
static uint8_t vm_profile_default_display_cursor_x(
    vm_profile_default_context *profile) {
    return qdcgaVarCursorPosRow(qdcgaVarPageNum);
}
static uint8_t vm_profile_default_display_cursor_y(
    vm_profile_default_context *profile) {
    return qdcgaVarCursorPosCol(qdcgaVarPageNum);
}
static uint8_t vm_profile_default_display_character(
    vm_profile_default_context *profile, uint8_t x, uint8_t y) {
    return qdcgaVarChar(qdcgaVarPageNum, x, y);
}
static uint8_t vm_profile_default_display_attribute(
    vm_profile_default_context *profile, uint8_t x, uint8_t y) {
    return qdcgaVarCharProp(qdcgaVarPageNum, x, y);
}

C_INT vm_profile_default_display_capture(C_VOID *context,
    core_machine_display_snapshot *out_snapshot)
{
    uint16_t row;
    uint16_t column;

    vm_profile_default_context *profile = context;

    if (profile == NULL || out_snapshot == NULL) return NTVDM64_TYPE_FALSE;
    out_snapshot->buffer_changed =
        vm_profile_default_display_buffer_changed(profile);
    out_snapshot->cursor_changed =
        vm_profile_default_display_cursor_changed(profile);
    out_snapshot->columns = vm_profile_default_display_columns(profile);
    out_snapshot->rows = vm_profile_default_display_rows(profile);
    out_snapshot->cursor_top = vm_profile_default_display_cursor_top(profile);
    out_snapshot->cursor_bottom =
        vm_profile_default_display_cursor_bottom(profile);
    out_snapshot->cursor_x = vm_profile_default_display_cursor_x(profile);
    out_snapshot->cursor_y = vm_profile_default_display_cursor_y(profile);
    out_snapshot->cursor_visible =
        vm_profile_default_display_cursor_visible(profile);
    if (!out_snapshot->buffer_changed && !out_snapshot->cursor_changed) {
        return NTVDM64_TYPE_TRUE;
    }
    for (row = 0u; row < out_snapshot->rows; ++row) {
        for (column = 0u; column < out_snapshot->columns; ++column) {
            uint16_t index = row * CORE_MACHINE_DISPLAY_MAX_COLUMNS + column;
            out_snapshot->characters[index] =
                vm_profile_default_display_character(profile, (uint8_t)row,
                    (uint8_t)column);
            out_snapshot->attributes[index] =
                vm_profile_default_display_attribute(profile, (uint8_t)row,
                    (uint8_t)column);
        }
    }
    return NTVDM64_TYPE_TRUE;
}

#undef profile_cpu
#undef profile_ram
#undef profile_vadp
