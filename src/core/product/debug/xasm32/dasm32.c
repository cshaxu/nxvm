/* This file is a part of NXVM project. */

#include "core/product/utils.h"

#include "core/product/debug/xasm32/dasm32.h"

#define NTVDM64_TYPE_TRACE_CONTEXT    trace
#define NTVDM64_TYPE_TRACE_ERROR  flagError
#define NTVDM64_TYPE_TRACE_SET_ERROR (flagError = 1)

typedef uint8_t t_dasm_prefix;

typedef struct dasm32_context {
    ntvdm64_type_trace trace;
    uint8_t defsize;
    uint8_t flagError;
    uint8_t *drcode;
    char dstmt[0x100];
    char dop[0x100], dopr[0x100], drm[0x100], dr[0x100], dimm[0x100];
    char dmovsreg[0x100], doverds[0x100], doverss[0x100];
    char dimmoff8[0x100], dimmoff16[0x100], dimmsign[0x100];
    uint8_t flagmem, flaglock;
    t_dasm_prefix prefix_oprsize, prefix_addrsize;
    uint8_t cr;
    uint64_t cimm;
    uint8_t iop;
    void (*dtable[0x100])(),(*dtable_0f[0x100])();
    uint8_t initialized;
} dasm32_context;

static _Thread_local dasm32_context *dasmContext;

#define trace (dasmContext->trace)
#define defsize (dasmContext->defsize)
#define flagError (dasmContext->flagError)
#define drcode (dasmContext->drcode)
#define dstmt (dasmContext->dstmt)
#define dop (dasmContext->dop)
#define dopr (dasmContext->dopr)
#define drm (dasmContext->drm)
#define dr (dasmContext->dr)
#define dimm (dasmContext->dimm)
#define dmovsreg (dasmContext->dmovsreg)
#define doverds (dasmContext->doverds)
#define doverss (dasmContext->doverss)
#define dimmoff8 (dasmContext->dimmoff8)
#define dimmoff16 (dasmContext->dimmoff16)
#define dimmsign (dasmContext->dimmsign)
#define flagmem (dasmContext->flagmem)
#define flaglock (dasmContext->flaglock)
#define prefix_oprsize (dasmContext->prefix_oprsize)
#define prefix_addrsize (dasmContext->prefix_addrsize)
#define cr (dasmContext->cr)
#define cimm (dasmContext->cimm)
#define iop (dasmContext->iop)
#define dtable (dasmContext->dtable)
#define dtable_0f (dasmContext->dtable_0f)

/* stack pointer size (unused) */
/* #define _GetStackSize   (vcpu.ss.seg.data.big ? 4 : 2) */
/* operand size */
#define _GetOperandSize ((defsize ^ prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((defsize ^ prefix_addrsize) ? 4 : 2)
/* get modrm and sib bits */
#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)      (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib)   (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)    (((sib) & 0x07) >> 0)

#define _comment_
#define _newins_

static void SPRINTFSI(char *str, uint32_t imm, uint8_t byte) {
    char sign;
    uint8_t i8u;
    uint16_t i16u;
    uint32_t i32u;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SPRINTFSI");
    i8u = (uint8_t)(imm);
    i16u = (uint16_t)(imm);
    i32u = (uint32_t)(imm);
    switch (byte) {
    case 1:
        if ((uint8_t)(imm & 0x80)) {
            sign = '-';
            i8u = ((~i8u) + 0x01);
        } else {
            sign = '+';
        }
        SPRINTF(str, "%c%02X", sign, i8u);
        break;
    case 2:
        if ((uint16_t)(imm & 0x8000)) {
            sign = '-';
            i16u = ((~i16u) + 0x01);
        } else {
            sign = '+';
        }
        SPRINTF(str, "%c%04X", sign, i16u);
        break;
    case 4:
        if ((uint32_t)(imm & 0x80000000)) {
            sign = '-';
            i32u = ((~i32u) + 0x01);
        } else {
            sign = '+';
        }
        SPRINTF(str, "%c%08X", sign, i32u);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* kernel decoding function */
static uint8_t _kdf_check_prefix(uint8_t opcode) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_check_prefix");
    switch (opcode) {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        NTVDM64_TYPE_TRACE_CALL_END;
        return 1;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        NTVDM64_TYPE_TRACE_CALL_END;
        return 1;
        break;
    default:
        NTVDM64_TYPE_TRACE_CALL_END;
        return 0;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
    return 0;
}

static void _kdf_skip(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_skip");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(iop += byte);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _kdf_code(uint8_t *rdata, uint8_t byte) {
    size_t i;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_code");
    for (i = 0; i < byte; ++i)
        *(rdata + i) = *(drcode + iop + i);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_skip(byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _kdf_modrm(uint8_t regbyte, uint8_t rmbyte) {
    char disp8;
    uint16_t disp16;
    uint32_t disp32;
    char dsibindex[0x100], dptr[0x100];
    uint8_t modrm, sib;
    char sign;
    uint8_t disp8u;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_modrm");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(&modrm, 1));
    flagmem = 1;
    drm[0] = dr[0] = dsibindex[0] = 0;
    switch (rmbyte) {
    case 1:
        SPRINTF(dptr, "BYTE PTR ");
        break;
    case 2:
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        dptr[0] = 0;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[BX+SI]", doverds);
                break;
            case 1:
                SPRINTF(drm, "%s:[BX+DI]", doverds);
                break;
            case 2:
                SPRINTF(drm, "%s:[BP+SI]", doverss);
                break;
            case 3:
                SPRINTF(drm, "%s:[BP+DI]", doverss);
                break;
            case 4:
                SPRINTF(drm, "%s:[SI]",    doverds);
                break;
            case 5:
                SPRINTF(drm, "%s:[DI]",    doverds);
                break;
            case 6:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp16), 2));
                SPRINTF(drm, "%s:[%04X]", doverds, disp16);
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 7:
                SPRINTF(drm, "%s:[BX]", doverds);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }

            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[BX+SI%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                SPRINTF(drm, "%s:[BX+DI%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                SPRINTF(drm, "%s:[BP+SI%c%02X]", doverss, sign, disp8u);
                break;
            case 3:
                SPRINTF(drm, "%s:[BP+DI%c%02X]", doverss, sign, disp8u);
                break;
            case 4:
                SPRINTF(drm, "%s:[SI%c%02X]",    doverds, sign, disp8u);
                break;
            case 5:
                SPRINTF(drm, "%s:[DI%c%02X]",    doverds, sign, disp8u);
                break;
            case 6:
                SPRINTF(drm, "%s:[BP%c%02X]",    doverss, sign, disp8u);
                break;
            case 7:
                SPRINTF(drm, "%s:[BX%c%02X]",    doverds, sign, disp8u);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp16), 2));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[BX+SI+%04X]", doverds, disp16);
                break;
            case 1:
                SPRINTF(drm, "%s:[BX+DI+%04X]", doverds, disp16);
                break;
            case 2:
                SPRINTF(drm, "%s:[BP+SI+%04X]", doverss, disp16);
                break;
            case 3:
                SPRINTF(drm, "%s:[BP+DI+%04X]", doverss, disp16);
                break;
            case 4:
                SPRINTF(drm, "%s:[SI+%04X]",    doverds, disp16);
                break;
            case 5:
                SPRINTF(drm, "%s:[DI+%04X]",    doverds, disp16);
                break;
            case 6:
                SPRINTF(drm, "%s:[BP+%04X]",    doverss, disp16);
                break;
            case 7:
                SPRINTF(drm, "%s:[BX+%04X]",    doverds, disp16);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&sib), 1));
            switch (_GetSIB_Index(sib)) {
            case 0:
                SPRINTF(dsibindex, "+EAX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 1:
                SPRINTF(dsibindex, "+ECX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 2:
                SPRINTF(dsibindex, "+EDX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 3:
                SPRINTF(dsibindex, "+EBX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 4:
                break;
            case 5:
                SPRINTF(dsibindex, "+EBP*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 6:
                SPRINTF(dsibindex, "+ESI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 7:
                SPRINTF(dsibindex, "+EDI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
        }
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[EAX]", doverds);
                break;
            case 1:
                SPRINTF(drm, "%s:[ECX]", doverds);
                break;
            case 2:
                SPRINTF(drm, "%s:[EDX]", doverds);
                break;
            case 3:
                SPRINTF(drm, "%s:[EBX]", doverds);
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    SPRINTF(drm, "%s:[EAX%s]", doverds, dsibindex);
                    break;
                case 1:
                    SPRINTF(drm, "%s:[ECX%s]", doverds, dsibindex);
                    break;
                case 2:
                    SPRINTF(drm, "%s:[EDX%s]", doverds, dsibindex);
                    break;
                case 3:
                    SPRINTF(drm, "%s:[EBX%s]", doverds, dsibindex);
                    break;
                case 4:
                    SPRINTF(drm, "%s:[ESP%s]", doverss, dsibindex);
                    break;
                case 5:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp32), 4));
                    SPRINTF(drm, "%s:[%08X%s]", doverds, disp32, dsibindex);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 6:
                    SPRINTF(drm, "%s:[ESI%s]", doverds, dsibindex);
                    break;
                case 7:
                    SPRINTF(drm, "%s:[EDI%s]", doverds, dsibindex);
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp32), 4));
                SPRINTF(drm, "%s:[%08X]", doverds, disp32);
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 6:
                SPRINTF(drm, "%s:[ESI]", doverds);
                break;
            case 7:
                SPRINTF(drm, "%s:[EDI]", doverds);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[EAX%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                SPRINTF(drm, "%s:[ECX%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                SPRINTF(drm, "%s:[EDX%c%02X]", doverds, sign, disp8u);
                break;
            case 3:
                SPRINTF(drm, "%s:[EBX%c%02X]", doverds, sign, disp8u);
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    SPRINTF(drm, "%s:[EAX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 1:
                    SPRINTF(drm, "%s:[ECX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 2:
                    SPRINTF(drm, "%s:[EDX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 3:
                    SPRINTF(drm, "%s:[EBX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 4:
                    SPRINTF(drm, "%s:[ESP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 5:
                    SPRINTF(drm, "%s:[EBP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 6:
                    SPRINTF(drm, "%s:[ESI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 7:
                    SPRINTF(drm, "%s:[EDI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                SPRINTF(drm, "%s:[EBP%c%02X]", doverss, sign, disp8u);
                break;
            case 6:
                SPRINTF(drm, "%s:[ESI%c%02X]", doverds, sign, disp8u);
                break;
            case 7:
                SPRINTF(drm, "%s:[EDI%c%02X]", doverds, sign, disp8u);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code((uint8_t *)(&disp32), 4));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "%s:[EAX+%08X]", doverds, disp32);
                break;
            case 1:
                SPRINTF(drm, "%s:[ECX+%08X]", doverds, disp32);
                break;
            case 2:
                SPRINTF(drm, "%s:[EDX+%08X]", doverds, disp32);
                break;
            case 3:
                SPRINTF(drm, "%s:[EBX+%08X]", doverds, disp32);
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    SPRINTF(drm, "%s:[EAX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 1:
                    SPRINTF(drm, "%s:[ECX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 2:
                    SPRINTF(drm, "%s:[EDX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 3:
                    SPRINTF(drm, "%s:[EBX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 4:
                    SPRINTF(drm, "%s:[ESP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 5:
                    SPRINTF(drm, "%s:[EBP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 6:
                    SPRINTF(drm, "%s:[ESI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 7:
                    SPRINTF(drm, "%s:[EDI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                SPRINTF(drm, "%s:[EBP+%08X]", doverss, disp32);
                break;
            case 6:
                SPRINTF(drm, "%s:[ESI+%08X]", doverds, disp32);
                break;
            case 7:
                SPRINTF(drm, "%s:[EDI+%08X]", doverds, disp32);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (_GetModRM_MOD(modrm) == 3) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        flagmem = 0;
        switch (rmbyte) {
        case 1:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "AL");
                break;
            case 1:
                SPRINTF(drm, "CL");
                break;
            case 2:
                SPRINTF(drm, "DL");
                break;
            case 3:
                SPRINTF(drm, "BL");
                break;
            case 4:
                SPRINTF(drm, "AH");
                break;
            case 5:
                SPRINTF(drm, "CH");
                break;
            case 6:
                SPRINTF(drm, "DH");
                break;
            case 7:
                SPRINTF(drm, "BH");
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "AX");
                break;
            case 1:
                SPRINTF(drm, "CX");
                break;
            case 2:
                SPRINTF(drm, "DX");
                break;
            case 3:
                SPRINTF(drm, "BX");
                break;
            case 4:
                SPRINTF(drm, "SP");
                break;
            case 5:
                SPRINTF(drm, "BP");
                break;
            case 6:
                SPRINTF(drm, "SI");
                break;
            case 7:
                SPRINTF(drm, "DI");
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                SPRINTF(drm, "EAX");
                break;
            case 1:
                SPRINTF(drm, "ECX");
                break;
            case 2:
                SPRINTF(drm, "EDX");
                break;
            case 3:
                SPRINTF(drm, "EBX");
                break;
            case 4:
                SPRINTF(drm, "ESP");
                break;
            case 5:
                SPRINTF(drm, "EBP");
                break;
            case 6:
                SPRINTF(drm, "ESI");
                break;
            case 7:
                SPRINTF(drm, "EDI");
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (regbyte) {
    case 0:
        if (flagmem) {
            STRCAT(dptr, drm);
            STRCPY(drm, dptr);
        }
    case 9:
        /* reg is operation or segment */
        cr = _GetModRM_REG(modrm);
        break;
    case 1:
        switch (_GetModRM_REG(modrm)) {
        case 0:
            SPRINTF(dr, "AL");
            break;
        case 1:
            SPRINTF(dr, "CL");
            break;
        case 2:
            SPRINTF(dr, "DL");
            break;
        case 3:
            SPRINTF(dr, "BL");
            break;
        case 4:
            SPRINTF(dr, "AH");
            break;
        case 5:
            SPRINTF(dr, "CH");
            break;
        case 6:
            SPRINTF(dr, "DH");
            break;
        case 7:
            SPRINTF(dr, "BH");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 2:
        switch (_GetModRM_REG(modrm)) {
        case 0:
            SPRINTF(dr, "AX");
            break;
        case 1:
            SPRINTF(dr, "CX");
            break;
        case 2:
            SPRINTF(dr, "DX");
            break;
        case 3:
            SPRINTF(dr, "BX");
            break;
        case 4:
            SPRINTF(dr, "SP");
            break;
        case 5:
            SPRINTF(dr, "BP");
            break;
        case 6:
            SPRINTF(dr, "SI");
            break;
        case 7:
            SPRINTF(dr, "DI");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 4:
        switch (_GetModRM_REG(modrm)) {
        case 0:
            SPRINTF(dr, "EAX");
            break;
        case 1:
            SPRINTF(dr, "ECX");
            break;
        case 2:
            SPRINTF(dr, "EDX");
            break;
        case 3:
            SPRINTF(dr, "EBX");
            break;
        case 4:
            SPRINTF(dr, "ESP");
            break;
        case 5:
            SPRINTF(dr, "EBP");
            break;
        case 6:
            SPRINTF(dr, "ESI");
            break;
        case 7:
            SPRINTF(dr, "EDI");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_skip(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_skip");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_skip(byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_code(uint8_t *rdata, uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_code");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_imm(uint8_t byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_imm");
    cimm = 0;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&cimm), byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_moffs(uint8_t byte) {
    uint32_t offset = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_moffs");
    flagmem = 1;
    switch (_GetAddressSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&offset), 2));
        SPRINTF(drm, "%s:[%04X]", doverds, (uint16_t)(offset));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&offset), 4));
        SPRINTF(drm, "%s:[%08X]", doverds, (uint32_t)(offset));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_modrm_sreg(uint8_t rmbyte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_sreg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(0, rmbyte));
    switch (cr) {
    case 0:
        SPRINTF(dr, "ES");
        break;
    case 1:
        SPRINTF(dr, "CS");
        break;
    case 2:
        SPRINTF(dr, "SS");
        break;
    case 3:
        SPRINTF(dr, "DS");
        break;
    case 4:
        SPRINTF(dr, "FS");
        break;
    case 5:
        SPRINTF(dr, "GS");
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr");
        SPRINTF(dr, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_modrm(uint8_t regbyte, uint8_t rmbyte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(regbyte, rmbyte));
    if (!flagmem && flaglock) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0),flaglock(1)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

#define _adv NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_skip(1))

static void UndefinedOpcode() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("UndefinedOpcode");
    SPRINTF(dop, "<ERROR>");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "ADD");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "ADD");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "ADD");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "ADD");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "ADD");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADD_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_EAX_I32");
    _adv;
    SPRINTF(dop, "ADD");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ES");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "ES");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ES");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "ES");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "OR");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "OR");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "OR");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "OR");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "OR");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OR_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_EAX_I32");
    _adv;
    SPRINTF(dop, "OR");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void PUSH_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_CS");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "CS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_CS");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "CS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F() {
    uint8_t oldiop;
    uint8_t opcode;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F");
    _adv;
    oldiop = iop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&opcode), 1));
    iop = oldiop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN((*(dtable_0f[opcode]))());
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "ADC");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "ADC");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "ADC");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "ADC");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "ADC");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ADC_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_EAX_I32");
    _adv;
    SPRINTF(dop, "ADC");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void PUSH_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_SS");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "SS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_SS");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "SS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "SBB");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "SBB");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "SBB");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "SBB");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "SBB");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SBB_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_EAX_I32");
    _adv;
    SPRINTF(dop, "SBB");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void PUSH_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_DS");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "DS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_DS");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "DS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "AND");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "AND");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "AND");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "AND");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "AND");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AND_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_EAX_I32");
    _adv;
    SPRINTF(dop, "AND");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_ES() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_ES");
    _adv;
    SPRINTF(doverds, "ES");
    SPRINTF(doverss, "ES");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DAA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAA");
    _adv;
    SPRINTF(dop, "DAA");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "SUB");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "SUB");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "SUB");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "SUB");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "SUB");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SUB_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_EAX_I32");
    _adv;
    SPRINTF(dop, "SUB");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_CS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_CS");
    _adv;
    SPRINTF(doverds, "CS");
    SPRINTF(doverss, "CS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DAS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAS");
    _adv;
    SPRINTF(dop, "DAS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "XOR");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "XOR");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "XOR");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "XOR");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "XOR");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XOR_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_EAX_I32");
    _adv;
    SPRINTF(dop, "XOR");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_SS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_SS");
    _adv;
    SPRINTF(doverds, "SS");
    SPRINTF(doverss, "SS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAA");
    _adv;
    SPRINTF(dop, "AAA");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "CMP");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "CMP");
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dop, "CMP");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dop, "CMP");
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_AL_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dop, "CMP");
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMP_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_EAX_I32");
    _adv;
    SPRINTF(dop, "CMP");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_DS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_DS");
    _adv;
    SPRINTF(doverds, "DS");
    SPRINTF(doverss, "DS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAS");
    _adv;
    SPRINTF(dop, "AAS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EAX");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX");
        break;
    case 4:
        SPRINTF(dopr, "EAX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ECX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ECX");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "CX");
        break;
    case 4:
        SPRINTF(dopr, "ECX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDX");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX");
        break;
    case 4:
        SPRINTF(dopr, "EDX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EBX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBX");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BX");
        break;
    case 4:
        SPRINTF(dopr, "EBX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ESP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESP");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SP");
        break;
    case 4:
        SPRINTF(dopr, "ESP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EBP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBP");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BP");
        break;
    case 4:
        SPRINTF(dopr, "EBP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_ESI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESI");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SI");
        break;
    case 4:
        SPRINTF(dopr, "ESI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INC_EDI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDI");
    _adv;
    SPRINTF(dop, "INC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DI");
        break;
    case 4:
        SPRINTF(dopr, "EDI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EAX");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX");
        break;
    case 4:
        SPRINTF(dopr, "EAX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ECX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ECX");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "CX");
        break;
    case 4:
        SPRINTF(dopr, "ECX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDX");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX");
        break;
    case 4:
        SPRINTF(dopr, "EDX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EBX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBX");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BX");
        break;
    case 4:
        SPRINTF(dopr, "EBX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ESP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESP");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SP");
        break;
    case 4:
        SPRINTF(dopr, "ESP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EBP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBP");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BP");
        break;
    case 4:
        SPRINTF(dopr, "EBP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_ESI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESI");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SI");
        break;
    case 4:
        SPRINTF(dopr, "ESI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void DEC_EDI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDI");
    _adv;
    SPRINTF(dop, "DEC");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DI");
        break;
    case 4:
        SPRINTF(dopr, "EDI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EAX");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX");
        break;
    case 4:
        SPRINTF(dopr, "EAX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ECX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ECX");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "CX");
        break;
    case 4:
        SPRINTF(dopr, "ECX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDX");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX");
        break;
    case 4:
        SPRINTF(dopr, "EDX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EBX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBX");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BX");
        break;
    case 4:
        SPRINTF(dopr, "EBX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ESP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESP");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SP");
        break;
    case 4:
        SPRINTF(dopr, "ESP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EBP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBP");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BP");
        break;
    case 4:
        SPRINTF(dopr, "EBP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_ESI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESI");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SI");
        break;
    case 4:
        SPRINTF(dopr, "ESI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_EDI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDI");
    _adv;
    SPRINTF(dop, "PUSH");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DI");
        break;
    case 4:
        SPRINTF(dopr, "EDI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EAX");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX");
        break;
    case 4:
        SPRINTF(dopr, "EAX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ECX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ECX");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "CX");
        break;
    case 4:
        SPRINTF(dopr, "ECX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDX");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX");
        break;
    case 4:
        SPRINTF(dopr, "EDX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EBX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBX");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BX");
        break;
    case 4:
        SPRINTF(dopr, "EBX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ESP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESP");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SP");
        break;
    case 4:
        SPRINTF(dopr, "ESP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EBP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBP");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BP");
        break;
    case 4:
        SPRINTF(dopr, "EBP");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_ESI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESI");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SI");
        break;
    case 4:
        SPRINTF(dopr, "ESI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_EDI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDI");
    _adv;
    SPRINTF(dop, "POP");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DI");
        break;
    case 4:
        SPRINTF(dopr, "EDI");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}


static void PUSHA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHA");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "PUSHA");
        break;
    case 4:
        SPRINTF(dop, "PUSHAD");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POPA() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPA");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "POPA");
        break;
    case 4:
        SPRINTF(dop, "POPAD");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BOUND_R16_M16_16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    _adv;
    SPRINTF(dop, "BOUND");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize * 2));
    if (!flagmem) {
        SPRINTF(dopr, "<ERROR>");
    } else {
        SPRINTF(dopr, "%s,%s", dr, drm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ARPL_RM16_R16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    _adv;
    SPRINTF(dop, "ARPL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(2, 2));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_FS");
    _adv;
    SPRINTF(doverds, "FS");
    SPRINTF(doverss, "FS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_GS");
    _adv;
    SPRINTF(doverds, "GS");
    SPRINTF(doverss, "GS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_OprSize() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_OprSize");
    _adv;
    prefix_oprsize = 0x01;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_AddrSize() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    _adv;
    prefix_addrsize = 0x01;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I32");
    _adv;
    SPRINTF(dop, "PUSH");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _adv;
    SPRINTF(dop, "IMUL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "%s,%s,%04X", dr, drm, (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "%s,%s,%08X", dr, drm, (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I8");
    _adv;
    SPRINTF(dop, "PUSH");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    _adv;
    SPRINTF(dop, "IMUL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%s,%s,%02X", dr, drm, (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSB");
    _adv;
    SPRINTF(dop, "INSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI],DX");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI],DX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "INSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "INSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI],DX");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI],DX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUTSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSB");
    _adv;
    SPRINTF(dop, "OUTSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUTSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "OUTSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "OUTSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JO_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JO_REL8");
    _adv;
    SPRINTF(dop, "JO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNO_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNO_REL8");
    _adv;
    SPRINTF(dop, "JNO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JC_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JC_REL8");
    _adv;
    SPRINTF(dop, "JC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNC_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNC_REL8");
    _adv;
    SPRINTF(dop, "JNC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JZ_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JZ_REL8");
    _adv;
    SPRINTF(dop, "JZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNZ_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNZ_REL8");
    _adv;
    SPRINTF(dop, "JNZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNA_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNA_REL8");
    _adv;
    SPRINTF(dop, "JNA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JA_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JA_REL8");
    _adv;
    SPRINTF(dop, "JA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JS_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JS_REL8");
    _adv;
    SPRINTF(dop, "JS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNS_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNS_REL8");
    _adv;
    SPRINTF(dop, "JNS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JP_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JP_REL8");
    _adv;
    SPRINTF(dop, "JP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNP_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNP_REL8");
    _adv;
    SPRINTF(dop, "JNP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JL_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JL_REL8");
    _adv;
    SPRINTF(dop, "JL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNL_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNL_REL8");
    _adv;
    SPRINTF(dop, "JNL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNG_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNG_REL8");
    _adv;
    SPRINTF(dop, "JNG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JG_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JG_REL8");
    _adv;
    SPRINTF(dop, "JG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_80() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_80");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (cr) {
    case 0: /* ADD_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        SPRINTF(dop, "ADD");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        SPRINTF(dop, "OR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        SPRINTF(dop, "ADC");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        SPRINTF(dop, "SBB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        SPRINTF(dop, "AND");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        SPRINTF(dop, "SUB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        SPRINTF(dop, "XOR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        SPRINTF(dop, "CMP");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_81() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_81");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (cr) {
    case 0: /* ADD_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
        SPRINTF(dop, "ADD");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I32");
        SPRINTF(dop, "OR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
        SPRINTF(dop, "ADC");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
        SPRINTF(dop, "SBB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I32");
        SPRINTF(dop, "AND");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
        SPRINTF(dop, "SUB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
        SPRINTF(dop, "XOR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
        SPRINTF(dop, "CMP");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_83() {
    char dsimm[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_83");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (cr) {
    case 0: /* ADD_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
        SPRINTF(dop, "ADD");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I8");
        SPRINTF(dop, "OR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
        SPRINTF(dop, "ADC");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
        SPRINTF(dop, "SBB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I8");
        SPRINTF(dop, "AND");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
        SPRINTF(dop, "SUB");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
        SPRINTF(dop, "XOR");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
        SPRINTF(dop, "CMP");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    SPRINTFSI(dsimm, (uint8_t)(cimm), 1);
    SPRINTF(dopr, "%s,%s", drm, dsimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _adv;
    SPRINTF(dop, "TEST");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM32_R32");
    _adv;
    SPRINTF(dop, "TEST");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _adv;
    SPRINTF(dop, "XCHG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    _adv;
    SPRINTF(dop, "XCHG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM8_R8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM32_R32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R8_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R8_RM8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(1, 1));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_RM32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_RM16_SREG() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(2));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LEA_R32_M32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEA_R32_M32");
    _adv;
    SPRINTF(dop, "LEA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_SREG_RM16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(2));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_8F() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_8F");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(9, _GetOperandSize));
    switch (cr) {
    case 0: /* POP_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("POP_RM32");
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dop, "POP");
            break;
        case 4:
            SPRINTF(dop, "POPD");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void NOP() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("NOP");
    _adv;
    SPRINTF(dop, "NOP");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ECX_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "CX,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "ECX,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EDX_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "DX,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "EDX,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EBX_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "BX,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "EBX,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ESP_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "SP,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "ESP,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EBP_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "BP,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "EBP,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_ESI_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "SI,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "ESI,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XCHG_EDI_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    _adv;
    SPRINTF(dop, "XCHG");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        SPRINTF(dopr, "DI,AX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        SPRINTF(dopr, "EDI,EAX");
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CBW() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CBW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "CBW");
        break;
    case 4:
        SPRINTF(dop, "CWDE");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CWD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CWD");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "CWD");
        break;
    case 4:
        SPRINTF(dop, "CDQ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CALL_PTR16_32() {
    uint16_t newcs;
    uint32_t neweip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_PTR16_32");
    _adv;
    SPRINTF(dop, "CALL");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        neweip = (uint16_t)(cimm);
        newcs = (uint16_t)(cimm >> 16);
        SPRINTF(dopr, "%04X:%04X", newcs, (uint16_t)(neweip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(8));
        neweip = (uint32_t)(cimm);
        newcs = (uint16_t)(cimm >> 32);
        SPRINTF(dopr, "%04X:%08X", newcs, (uint32_t)(neweip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void WAIT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("WAIT");
    _adv;
    SPRINTF(dop, "WAIT");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSHF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHF");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "PUSHF");
        break;
    case 4:
        SPRINTF(dop, "PUSHFD");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POPF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPF");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "POPF");
        break;
    case 4:
        SPRINTF(dop, "POPFD");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SAHF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SAHF");
    _adv;
    SPRINTF(dop, "SAHF");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LAHF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAHF");
    _adv;
    SPRINTF(dop, "LAHF");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AL_MOFFS8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(1));
    SPRINTF(dopr, "AL,%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EAX_MOFFS32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX,%s", drm);
        break;
    case 4:
        SPRINTF(dopr, "EAX,%s", drm);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_MOFFS8_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(1));
    SPRINTF(dopr, "%s,AL", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_MOFFS32_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "%s,AX", drm);
        break;
    case 4:
        SPRINTF(dopr, "%s,EAX", drm);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVS");
    _adv;
    SPRINTF(dop, "MOVSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "MOVSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "MOVSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMPSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSB");
    _adv;
    SPRINTF(dop, "CMPSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMPSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "CMPSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "CMPSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_AL_I8");
    _adv;
    SPRINTF(dop, "TEST");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void TEST_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_EAX_I32");
    _adv;
    SPRINTF(dop, "TEST");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STOSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSB");
    _adv;
    SPRINTF(dop, "STOSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STOSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "STOSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "STOSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LODSB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSB");
    _adv;
    SPRINTF(dop, "LODSB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LODSW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "LODSW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "LODSD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "%s:[ESI]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SCASB() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASB");
    _adv;
    SPRINTF(dop, "SCASB");
    SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SCASW() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASW");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "SCASW");
        SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        SPRINTF(dop, "SCASD");
        SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CL_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "CL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DL_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "DL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_BL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BL_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "BL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_AH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AH_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "AH,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CH_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "CH,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DH_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "DH,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_BH_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BH_I8");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "BH,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EAX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ECX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ECX_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "CX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "ECX,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EDX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDX_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EDX,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EBX_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBX_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EBX,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ESP_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESP_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SP,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "ESP,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EBP_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBP_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "BP,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EBP,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_ESI_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESI_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "SI,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "ESI,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_EDI_I32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDI_I32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DI,%04X", (uint16_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EDI,%08X", (uint32_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C0() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C0");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (cr) {
    case 0: /* ROL_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C1() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C1");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (cr) {
    case 0: /* ROL_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
        SPRINTF(dop, "ROR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RET_I16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET_I16");
    _adv;
    SPRINTF(dop, "RET");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
    SPRINTF(dopr, "%04X", (uint16_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RET() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET");
    _adv;
    SPRINTF(dop, "RET");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LES_R32_M16_32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LES_R32_M16_32");
    _adv;
    SPRINTF(dop, "LES");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LDS_R32_M16_32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    _adv;
    SPRINTF(dop, "LDS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C6() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C6");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    switch (cr) {
    case 0: /* MOV_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        SPRINTF(dop, "MOV");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_C7() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C7");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    switch (cr) {
    case 0: /* MOV_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
        SPRINTF(dop, "MOV");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
            break;
        case 4:
            SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void ENTER() {
    char dframesize[0x100], dnestlevel[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ENTER");
    _adv;
    SPRINTF(dop, "ENTER");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
    SPRINTF(dframesize, "%04X", (uint16_t)(cimm));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dnestlevel, "%02X", (uint8_t)(cimm));
    SPRINTF(dopr, "%s,%s", dframesize, dnestlevel);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LEAVE() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEAVE");
    _adv;
    SPRINTF(dop, "LEAVE");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RETF_I16() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF_I16");
    _adv;
    SPRINTF(dop, "RETF");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
    SPRINTF(dopr, "%04X", (uint16_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RETF() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF");
    _adv;
    SPRINTF(dop, "RETF");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INT3() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT3");
    _adv;
    SPRINTF(dop, "INT3");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INT_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT_I8");
    _adv;
    SPRINTF(dop, "INT");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INTO() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INTO");
    _adv;
    SPRINTF(dop, "INTO");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IRET() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IRET");
    _adv;
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dop, "IRET");
        break;
    case 4:
        SPRINTF(dop, "IRETD");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D0() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D0");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    switch (cr) {
    case 0: /* ROL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8");
        SPRINTF(dop, "ROR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D1() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D1");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    switch (cr) {
    case 0: /* ROL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32");
        SPRINTF(dop, "ROR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,01", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D2() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D2");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    switch (cr) {
    case 0: /* ROL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        SPRINTF(dop, "ROR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_D3() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D3");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    switch (cr) {
    case 0: /* ROL_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
        SPRINTF(dop, "ROL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
        SPRINTF(dop, "ROR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
        SPRINTF(dop, "RCL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
        SPRINTF(dop, "RCR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
        SPRINTF(dop, "SHL");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
        SPRINTF(dop, "SHR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
        SPRINTF(dop, "SAR");
        SPRINTF(dopr, "%s,CL", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAM() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAM");
    _adv;
    SPRINTF(dop, "AAM");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    if ((uint8_t)(cimm) != 0x0a) SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void AAD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAD");
    _adv;
    SPRINTF(dop, "AAD");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    if ((uint8_t)(cimm) != 0x0a) SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void XLAT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XLAT");
    _adv;
    SPRINTF(dop, "XLATB");
    switch (_GetAddressSize) {
    case 2:
        SPRINTF(dopr, "%s:[BX+AL]", doverds);
        break;
    case 4:
        SPRINTF(dopr, "%s:[EBX+AL]", doverds);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LOOPNZ_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    _adv;
    SPRINTF(dop, "LOOPNZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LOOPZ_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOPZ_REL8");
    _adv;
    SPRINTF(dop, "LOOPZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LOOP_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOP_REL8");
    _adv;
    SPRINTF(dop, "LOOP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JCXZ_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JCXZ_REL8");
    _adv;
    SPRINTF(dop, "JCXZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_AL_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    _adv;
    SPRINTF(dop, "IN");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_EAX_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_EAX_I8");
    _adv;
    SPRINTF(dop, "IN");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX,%02X", (uint8_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "EAX,%02X", (uint8_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_I8_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_AL");
    _adv;
    SPRINTF(dop, "OUT");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%02X,AL", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_I8_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _adv;
    SPRINTF(dop, "OUT");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "%02X,AX", (uint8_t)(cimm));
        break;
    case 4:
        SPRINTF(dopr, "%02X,EAX", (uint8_t)(cimm));
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CALL_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_REL32");
    _adv;
    SPRINTF(dop, "CALL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_REL32");
    _adv;
    SPRINTF(dop, "JMP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP_PTR16_32() {
    uint16_t newcs;
    uint32_t neweip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_PTR16_32");
    _adv;
    SPRINTF(dop, "JMP");
    switch (_GetOperandSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        neweip = (uint16_t)(cimm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        newcs = (uint16_t)(cimm);
        SPRINTF(dopr, "%04X:%04X", newcs, (uint16_t)(neweip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        _newins_;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(4));
        neweip = (uint32_t)(cimm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(2));
        newcs = (uint16_t)(cimm);
        SPRINTF(dopr, "%04X:%08X", newcs, (uint32_t)(neweip));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JMP_REL8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_REL8");
    _adv;
    SPRINTF(dop, "JMP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTFSI(dopr, (uint8_t)(cimm), 1);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_AL_DX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_DX");
    _adv;
    SPRINTF(dop, "IN");
    SPRINTF(dopr, "AL,DX");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IN_EAX_DX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_EAX_DX");
    _adv;
    SPRINTF(dop, "IN");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "AX,DX");
        break;
    case 4:
        SPRINTF(dopr, "EAX,DX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_DX_AL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_AL");
    _adv;
    SPRINTF(dop, "OUT");
    SPRINTF(dopr, "DX,AL");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void OUT_DX_EAX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _adv;
    SPRINTF(dop, "OUT");
    switch (_GetOperandSize) {
    case 2:
        SPRINTF(dopr, "DX,AX");
        break;
    case 4:
        SPRINTF(dopr, "DX,EAX");
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_LOCK() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    _adv;
    SPRINTF(dop, "LOCK:");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_REPNZ() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    _adv;
    SPRINTF(dop, "REPNZ:");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PREFIX_REPZ() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPZ");
    _adv;
    SPRINTF(dop, "REPZ:");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void HLT() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("HLT");
    _adv;
    SPRINTF(dop, "HLT");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CMC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMC");
    _adv;
    SPRINTF(dop, "CMC");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_F6() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F6");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    switch (cr) {
    case 0: /* TEST_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        SPRINTF(dop, "TEST");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NOT_RM8");
        SPRINTF(dop, "NOT");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NEG_RM8");
        SPRINTF(dop, "NEG");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MUL_RM8");
        SPRINTF(dop, "MUL");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IMUL_RM8");
        SPRINTF(dop, "IMUL");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DIV_RM8");
        SPRINTF(dop, "DIV");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IDIV_RM8");
        SPRINTF(dop, "IDIV");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_F7() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F7");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    switch (cr) {
    case 0: /* TEST_RM32_I32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
        SPRINTF(dop, "TEST");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
            break;
        case 4:
            SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NOT_RM32");
        SPRINTF(dop, "NOT");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NEG_RM16");
        SPRINTF(dop, "NEG");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MUL_RM32");
        SPRINTF(dop, "MUL");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IMUL_RM32");
        SPRINTF(dop, "IMUL");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DIV_RM32");
        SPRINTF(dop, "DIV");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IDIV_RM32");
        SPRINTF(dop, "IDIV");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLC");
    _adv;
    SPRINTF(dop, "CLC");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STC() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STC");
    _adv;
    SPRINTF(dop, "STC");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLI");
    _adv;
    SPRINTF(dop, "CLI");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STI() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STI");
    _adv;
    SPRINTF(dop, "STI");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLD");
    _adv;
    SPRINTF(dop, "CLD");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void STD() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STD");
    _adv;
    SPRINTF(dop, "STD");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_FE() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FE");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    switch (cr) {
    case 0: /* INC_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("INC_RM8");
        SPRINTF(dop, "INC");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DEC_RM8");
        SPRINTF(dop, "DEC");
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_FF() {
    char dptr[0x100];
    uint8_t oldiop;
    uint8_t modrm;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FF");
    _adv;
    oldiop = iop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm)) {
    case 0: /* INC_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("INC_RM32");
        SPRINTF(dop, "INC");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DEC_RM32");
        SPRINTF(dop, "DEC");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* CALL_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_RM32");
        SPRINTF(dop, "CALL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* CALL_M16_32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_M16_32");
        SPRINTF(dop, "CALL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(9, _GetOperandSize + 2));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dptr, "WORD PTR ");
            break;
        case 4:
            SPRINTF(dptr, "DWORD PTR ");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        SPRINTF(dopr, "FAR %s%s", dptr, drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* JMP_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_RM32");
        SPRINTF(dop, "JMP");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* JMP_M16_32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_M16_32");
        SPRINTF(dop, "JMP");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(9, _GetOperandSize + 2));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dptr, "WORD PTR ");
            break;
        case 4:
            SPRINTF(dptr, "DWORD PTR ");
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        SPRINTF(dopr, "FAR %s%s", dptr, drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* PUSH_RM32 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PUSH_RM32");
        SPRINTF(dop, "PUSH");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void _d_modrm_creg() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_creg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(9, 4));
    if (flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (cr) {
    case 0:
        SPRINTF(dr, "CR0");
        break;
    case 2:
        SPRINTF(dr, "CR2");
        break;
    case 3:
        SPRINTF(dr, "CR3");
        break;;
    default:
        SPRINTF(dr, "<ERROR>");
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_modrm_dreg() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_dreg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(9, 4));
    if (flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (cr) {
    case 0:
        SPRINTF(dr, "DR0");
        break;
    case 1:
        SPRINTF(dr, "DR1");
        break;
    case 2:
        SPRINTF(dr, "DR2");
        break;
    case 3:
        SPRINTF(dr, "DR3");
        break;
    case 6:
        SPRINTF(dr, "DR6");
        break;
    case 7:
        SPRINTF(dr, "DR7");
        break;
    default:
        SPRINTF(dr, "<ERROR>");
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void _d_modrm_treg() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_treg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(9, 4));
    if (flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (cr) {
    case 6:
        SPRINTF(dr, "TR6");
        break;
    case 7:
        SPRINTF(dr, "TR7");
        break;
    default:
        SPRINTF(dr, "<ERROR>");
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static void INS_0F_00() {
    uint8_t modrm, oldiop;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    oldiop = iop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm)) {
    case 0: /* SLDT_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SLDT_RM16");
        SPRINTF(dop, "SLDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* STR_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("STR_RM16");
        SPRINTF(dop, "STR");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LLDT_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LLDT_RM16");
        SPRINTF(dop, "LLDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 2));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LTR_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LTR_RM16");
        SPRINTF(dop, "LTR");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 2));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* VERR_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("VERR_RM16");
        SPRINTF(dop, "VERR");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 2));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* VERW_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("VERW_RM16");
        SPRINTF(dop, "VERW");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 2));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F_01() {
    uint8_t modrm, oldiop;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    oldiop = iop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm)) {
    case 0: /* SGDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        SPRINTF(dop, "SGDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 6));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        SPRINTF(dop, "SIDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 6));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        SPRINTF(dop, "LGDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 6));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        SPRINTF(dop, "LIDT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 6));
        if (!flagmem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            SPRINTF(drm, "<ERROR>");
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SMSW_RM16");
        SPRINTF(dop, "SMSW");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LMSW_RM16");
        SPRINTF(dop, "LMSW");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 2));
        SPRINTF(dopr, "%s", drm);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LAR_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    SPRINTF(dop, "LAR");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSL_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    SPRINTF(dop, "LSL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CLTS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLTS");
    _adv;
    SPRINTF(dop, "CLTS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void WBINVD() {}
static void MOV_R32_CR() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_creg());
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_DR() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_DR");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg());
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_CR_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_creg());
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_DR_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DR_R32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg());
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_R32_TR() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_TR");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_treg());
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOV_TR_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_TR_R32");
    _adv;
    SPRINTF(dop, "MOV");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_treg());
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void WRMSR() {}
static void RDMSR() {}
static void JO_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JO_REL32");
    _adv;
    SPRINTF(dop, "JO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNO_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNO_REL32");
    _adv;
    SPRINTF(dop, "JNO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JC_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    SPRINTF(dop, "JC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNC_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNC_REL32");
    _adv;
    SPRINTF(dop, "JNC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JZ_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    SPRINTF(dop, "JZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNZ_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    SPRINTF(dop, "JNZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNA_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNA_REL32");
    _adv;
    SPRINTF(dop, "JNA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JA_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JA_REL32");
    _adv;
    SPRINTF(dop, "JA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JS_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JS_REL32");
    _adv;
    SPRINTF(dop, "JS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNS_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNS_REL32");
    _adv;
    SPRINTF(dop, "JNS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JP_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JP_REL32");
    _adv;
    SPRINTF(dop, "JP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNP_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNP_REL32");
    _adv;
    SPRINTF(dop, "JNP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JL_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JL_REL32");
    _adv;
    SPRINTF(dop, "JL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNL_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNL_REL32");
    _adv;
    SPRINTF(dop, "JNL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JNG_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNG_REL32");
    _adv;
    SPRINTF(dop, "JNG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void JG_REL32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JG_REL32");
    _adv;
    SPRINTF(dop, "JG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        SPRINTFSI(dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dopr, (uint32_t)(cimm), 4);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETO_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    SPRINTF(dop, "SETO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNO_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    SPRINTF(dop, "SETNO");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETC_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETC_RM8");
    _adv;
    SPRINTF(dop, "SETC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNC_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNC_RM8");
    _adv;
    SPRINTF(dop, "SETNC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETZ_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETZ_RM8");
    _adv;
    SPRINTF(dop, "SETZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNZ_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNZ_RM8");
    _adv;
    SPRINTF(dop, "SETNZ");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNA_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNA_RM8");
    _adv;
    SPRINTF(dop, "SETNA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETA_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETA_RM8");
    _adv;
    SPRINTF(dop, "SETA");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETS_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETS_RM8");
    _adv;
    SPRINTF(dop, "SETS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNS_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNS_RM8");
    _adv;
    SPRINTF(dop, "SETNS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETP_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETP_RM8");
    _adv;
    SPRINTF(dop, "SETP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNP_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNP_RM8");
    _adv;
    SPRINTF(dop, "SETNP");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETL_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETL_RM8");
    _adv;
    SPRINTF(dop, "SETL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNL_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNL_RM8");
    _adv;
    SPRINTF(dop, "SETNL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETNG_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNG_RM8");
    _adv;
    SPRINTF(dop, "SETNG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SETG_RM8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETG_RM8");
    _adv;
    SPRINTF(dop, "SETG");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, 1));
    SPRINTF(dopr, "%s", drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "FS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_FS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "FS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void CPUID() {}
static void BT_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    SPRINTF(dop, "BT");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHLD_RM32_R32_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    SPRINTF(dop, "SHLD");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%s,%s,%02X", drm, dr, (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHLD_RM32_R32_CL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    SPRINTF(dop, "SHLD");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s,CL", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void PUSH_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    SPRINTF(dop, "PUSH");
    SPRINTF(dopr, "GS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void POP_GS() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    SPRINTF(dop, "POP");
    SPRINTF(dopr, "GS");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void RSM() {}
static void BTS_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    SPRINTF(dop, "BTS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHRD_RM32_R32_I8() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    SPRINTF(dop, "SHRD");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%s,%s,%02X", drm, dr, (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void SHRD_RM32_R32_CL() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    SPRINTF(dop, "SHRD");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s,CL", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void IMUL_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    SPRINTF(dop, "IMUL");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LSS_R32_M16_32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    SPRINTF(dop, "LSS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTR_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    SPRINTF(dop, "BTR");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LFS_R32_M16_32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    SPRINTF(dop, "LFS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void LGS_R32_M16_32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _adv;
    SPRINTF(dop, "LGS");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!flagmem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        SPRINTF(drm, "<ERROR>");
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVZX_R32_RM8() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    SPRINTF(dop, "MOVZX");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, 1));
    if (flagmem) SPRINTF(dptr, "BYTE PTR ");
    else dptr[0] = 0;
    SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVZX_R32_RM16() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    SPRINTF(dop, "MOVZX");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(4, 2));
    if (flagmem) SPRINTF(dptr, "WORD PTR ");
    else dptr[0] = 0;
    SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void INS_0F_BA() {
    uint8_t modrm, oldiop;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_BA");
    _adv;
    oldiop = iop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code((uint8_t *)(&modrm), 1));
    iop = oldiop;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(0, _GetOperandSize));
    switch (cr) {
    case 0:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode());
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        SPRINTF(dop, "BT");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        SPRINTF(dop, "BTS");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        SPRINTF(dop, "BTR");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        SPRINTF(dop, "BTC");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
        SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BTC_RM32_R32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _adv;
    SPRINTF(dop, "BTC");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", drm, dr);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSF_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _adv;
    SPRINTF(dop, "BSF");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void BSR_R32_RM32() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _adv;
    SPRINTF(dop, "BSR");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, _GetOperandSize));
    SPRINTF(dopr, "%s,%s", dr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSX_R32_RM8() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    SPRINTF(dop, "MOVSX");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(_GetOperandSize, 1));
    if (flagmem) SPRINTF(dptr, "BYTE PTR ");
    else dptr[0] = 0;
    SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void MOVSX_R32_RM16() {
    char dptr[0x100];
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    SPRINTF(dop, "MOVSX");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(4, 2));
    if (flagmem) SPRINTF(dptr, "WORD PTR ");
    else dptr[0] = 0;
    SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static void QDX() {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("QDX");
    _adv;
    SPRINTF(dop, "QDX");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(1));
    SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    NTVDM64_TYPE_TRACE_CALL_END;
}

static uint8_t dasm32_execute(char *stmt, uint8_t *rcode, int flag32) {
    size_t i;
    uint8_t opcode, oldiop;
#if DASM_TRACE == 1
    ntvdm64_type_trace_initialize(&trace);
#endif
    if (!dasmContext->initialized) {
        dtable[0x00] = ADD_RM8_R8;
        dtable[0x01] = ADD_RM32_R32;
        dtable[0x02] = ADD_R8_RM8;
        dtable[0x03] = ADD_R32_RM32;
        dtable[0x04] = ADD_AL_I8;
        dtable[0x05] = ADD_EAX_I32;
        dtable[0x06] = PUSH_ES;
        dtable[0x07] = POP_ES;
        dtable[0x08] = OR_RM8_R8;
        dtable[0x09] = OR_RM32_R32;
        dtable[0x0a] = OR_R8_RM8;
        dtable[0x0b] = OR_R32_RM32;
        dtable[0x0c] = OR_AL_I8;
        dtable[0x0d] = OR_EAX_I32;
        dtable[0x0e] = PUSH_CS;
        dtable[0x0f] = INS_0F;
        dtable[0x10] = ADC_RM8_R8;
        dtable[0x11] = ADC_RM32_R32;
        dtable[0x12] = ADC_R8_RM8;
        dtable[0x13] = ADC_R32_RM32;
        dtable[0x14] = ADC_AL_I8;
        dtable[0x15] = ADC_EAX_I32;
        dtable[0x16] = PUSH_SS;
        dtable[0x17] = POP_SS;
        dtable[0x18] = SBB_RM8_R8;
        dtable[0x19] = SBB_RM32_R32;
        dtable[0x1a] = SBB_R8_RM8;
        dtable[0x1b] = SBB_R32_RM32;
        dtable[0x1c] = SBB_AL_I8;
        dtable[0x1d] = SBB_EAX_I32;
        dtable[0x1e] = PUSH_DS;
        dtable[0x1f] = POP_DS;
        dtable[0x20] = AND_RM8_R8;
        dtable[0x21] = AND_RM32_R32;
        dtable[0x22] = AND_R8_RM8;
        dtable[0x23] = AND_R32_RM32;
        dtable[0x24] = AND_AL_I8;
        dtable[0x25] = AND_EAX_I32;
        dtable[0x26] = PREFIX_ES;
        dtable[0x27] = DAA;
        dtable[0x28] = SUB_RM8_R8;
        dtable[0x29] = SUB_RM32_R32;
        dtable[0x2a] = SUB_R8_RM8;
        dtable[0x2b] = SUB_R32_RM32;
        dtable[0x2c] = SUB_AL_I8;
        dtable[0x2d] = SUB_EAX_I32;
        dtable[0x2e] = PREFIX_CS;
        dtable[0x2f] = DAS;
        dtable[0x30] = XOR_RM8_R8;
        dtable[0x31] = XOR_RM32_R32;
        dtable[0x32] = XOR_R8_RM8;
        dtable[0x33] = XOR_R32_RM32;
        dtable[0x34] = XOR_AL_I8;
        dtable[0x35] = XOR_EAX_I32;
        dtable[0x36] = PREFIX_SS;
        dtable[0x37] = AAA;
        dtable[0x38] = CMP_RM8_R8;
        dtable[0x39] = CMP_RM32_R32;
        dtable[0x3a] = CMP_R8_RM8;
        dtable[0x3b] = CMP_R32_RM32;
        dtable[0x3c] = CMP_AL_I8;
        dtable[0x3d] = CMP_EAX_I32;
        dtable[0x3e] = PREFIX_DS;
        dtable[0x3f] = AAS;
        dtable[0x40] = INC_EAX;
        dtable[0x41] = INC_ECX;
        dtable[0x42] = INC_EDX;
        dtable[0x43] = INC_EBX;
        dtable[0x44] = INC_ESP;
        dtable[0x45] = INC_EBP;
        dtable[0x46] = INC_ESI;
        dtable[0x47] = INC_EDI;
        dtable[0x48] = DEC_EAX;
        dtable[0x49] = DEC_ECX;
        dtable[0x4a] = DEC_EDX;
        dtable[0x4b] = DEC_EBX;
        dtable[0x4c] = DEC_ESP;
        dtable[0x4d] = DEC_EBP;
        dtable[0x4e] = DEC_ESI;
        dtable[0x4f] = DEC_EDI;
        dtable[0x50] = PUSH_EAX;
        dtable[0x51] = PUSH_ECX;
        dtable[0x52] = PUSH_EDX;
        dtable[0x53] = PUSH_EBX;
        dtable[0x54] = PUSH_ESP;
        dtable[0x55] = PUSH_EBP;
        dtable[0x56] = PUSH_ESI;
        dtable[0x57] = PUSH_EDI;
        dtable[0x58] = POP_EAX;
        dtable[0x59] = POP_ECX;
        dtable[0x5a] = POP_EDX;
        dtable[0x5b] = POP_EBX;
        dtable[0x5c] = POP_ESP;
        dtable[0x5d] = POP_EBP;
        dtable[0x5e] = POP_ESI;
        dtable[0x5f] = POP_EDI;
        dtable[0x60] = PUSHA;
        dtable[0x61] = POPA;
        dtable[0x62] = BOUND_R16_M16_16;
        dtable[0x63] = ARPL_RM16_R16;
        dtable[0x64] = PREFIX_FS;
        dtable[0x65] = PREFIX_GS;
        dtable[0x66] = PREFIX_OprSize;
        dtable[0x67] = PREFIX_AddrSize;
        dtable[0x68] = PUSH_I32;
        dtable[0x69] = IMUL_R32_RM32_I32;
        dtable[0x6a] = PUSH_I8;
        dtable[0x6b] = IMUL_R32_RM32_I8;
        dtable[0x6c] = INSB;
        dtable[0x6d] = INSW;
        dtable[0x6e] = OUTSB;
        dtable[0x6f] = OUTSW;
        dtable[0x70] = JO_REL8;
        dtable[0x71] = JNO_REL8;
        dtable[0x72] = JC_REL8;
        dtable[0x73] = JNC_REL8;
        dtable[0x74] = JZ_REL8;
        dtable[0x75] = JNZ_REL8;
        dtable[0x76] = JNA_REL8;
        dtable[0x77] = JA_REL8;
        dtable[0x78] = JS_REL8;
        dtable[0x79] = JNS_REL8;
        dtable[0x7a] = JP_REL8;
        dtable[0x7b] = JNP_REL8;
        dtable[0x7c] = JL_REL8;
        dtable[0x7d] = JNL_REL8;
        dtable[0x7e] = JNG_REL8;
        dtable[0x7f] = JG_REL8;
        dtable[0x80] = INS_80;
        dtable[0x81] = INS_81;
        dtable[0x82] = UndefinedOpcode;
        dtable[0x83] = INS_83;
        dtable[0x84] = TEST_RM8_R8;
        dtable[0x85] = TEST_RM32_R32;
        dtable[0x86] = XCHG_RM8_R8;
        dtable[0x87] = XCHG_RM32_R32;
        dtable[0x88] = MOV_RM8_R8;
        dtable[0x89] = MOV_RM32_R32;
        dtable[0x8a] = MOV_R8_RM8;
        dtable[0x8b] = MOV_R32_RM32;
        dtable[0x8c] = MOV_RM16_SREG;
        dtable[0x8d] = LEA_R32_M32;
        dtable[0x8e] = MOV_SREG_RM16;
        dtable[0x8f] = INS_8F;
        dtable[0x90] = NOP;
        dtable[0x91] = XCHG_ECX_EAX;
        dtable[0x92] = XCHG_EDX_EAX;
        dtable[0x93] = XCHG_EBX_EAX;
        dtable[0x94] = XCHG_ESP_EAX;
        dtable[0x95] = XCHG_EBP_EAX;
        dtable[0x96] = XCHG_ESI_EAX;
        dtable[0x97] = XCHG_EDI_EAX;
        dtable[0x98] = CBW;
        dtable[0x99] = CWD;
        dtable[0x9a] = CALL_PTR16_32;
        dtable[0x9b] = WAIT;
        dtable[0x9c] = PUSHF;
        dtable[0x9d] = POPF;
        dtable[0x9e] = SAHF;
        dtable[0x9f] = LAHF;
        dtable[0xa0] = MOV_AL_MOFFS8;
        dtable[0xa1] = MOV_EAX_MOFFS32;
        dtable[0xa2] = MOV_MOFFS8_AL;
        dtable[0xa3] = MOV_MOFFS32_EAX;
        dtable[0xa4] = MOVSB;
        dtable[0xa5] = MOVSW;
        dtable[0xa6] = CMPSB;
        dtable[0xa7] = CMPSW;
        dtable[0xa8] = TEST_AL_I8;
        dtable[0xa9] = TEST_EAX_I32;
        dtable[0xaa] = STOSB;
        dtable[0xab] = STOSW;
        dtable[0xac] = LODSB;
        dtable[0xad] = LODSW;
        dtable[0xae] = SCASB;
        dtable[0xaf] = SCASW;
        dtable[0xb0] = MOV_AL_I8;
        dtable[0xb1] = MOV_CL_I8;
        dtable[0xb2] = MOV_DL_I8;
        dtable[0xb3] = MOV_BL_I8;
        dtable[0xb4] = MOV_AH_I8;
        dtable[0xb5] = MOV_CH_I8;
        dtable[0xb6] = MOV_DH_I8;
        dtable[0xb7] = MOV_BH_I8;
        dtable[0xb8] = MOV_EAX_I32;
        dtable[0xb9] = MOV_ECX_I32;
        dtable[0xba] = MOV_EDX_I32;
        dtable[0xbb] = MOV_EBX_I32;
        dtable[0xbc] = MOV_ESP_I32;
        dtable[0xbd] = MOV_EBP_I32;
        dtable[0xbe] = MOV_ESI_I32;
        dtable[0xbf] = MOV_EDI_I32;
        dtable[0xc0] = INS_C0;
        dtable[0xc1] = INS_C1;
        dtable[0xc2] = RET_I16;
        dtable[0xc3] = RET;
        dtable[0xc4] = LES_R32_M16_32;
        dtable[0xc5] = LDS_R32_M16_32;
        dtable[0xc6] = INS_C6;
        dtable[0xc7] = INS_C7;
        dtable[0xc8] = ENTER;
        dtable[0xc9] = LEAVE;
        dtable[0xca] = RETF_I16;
        dtable[0xcb] = RETF;
        dtable[0xcc] = INT3;
        dtable[0xcd] = INT_I8;
        dtable[0xce] = INTO;
        dtable[0xcf] = IRET;
        dtable[0xd0] = INS_D0;
        dtable[0xd1] = INS_D1;
        dtable[0xd2] = INS_D2;
        dtable[0xd3] = INS_D3;
        dtable[0xd4] = AAM;
        dtable[0xd5] = AAD;
        dtable[0xd6] = UndefinedOpcode;
        dtable[0xd7] = XLAT;
        dtable[0xd8] = UndefinedOpcode;
        dtable[0xd9] = UndefinedOpcode;
        dtable[0xda] = UndefinedOpcode;
        dtable[0xdb] = UndefinedOpcode;
        dtable[0xdc] = UndefinedOpcode;
        dtable[0xdd] = UndefinedOpcode;
        dtable[0xde] = UndefinedOpcode;
        dtable[0xdf] = UndefinedOpcode;
        dtable[0xe0] = LOOPNZ_REL8;
        dtable[0xe1] = LOOPZ_REL8;
        dtable[0xe2] = LOOP_REL8;
        dtable[0xe3] = JCXZ_REL8;
        dtable[0xe4] = IN_AL_I8;
        dtable[0xe5] = IN_EAX_I8;
        dtable[0xe6] = OUT_I8_AL;
        dtable[0xe7] = OUT_I8_EAX;
        dtable[0xe8] = CALL_REL32;
        dtable[0xe9] = JMP_REL32;
        dtable[0xea] = JMP_PTR16_32;
        dtable[0xeb] = JMP_REL8;
        dtable[0xec] = IN_AL_DX;
        dtable[0xed] = IN_EAX_DX;
        dtable[0xee] = OUT_DX_AL;
        dtable[0xef] = OUT_DX_EAX;
        dtable[0xf0] = PREFIX_LOCK;
        dtable[0xf1] = QDX;
        dtable[0xf2] = PREFIX_REPNZ;
        dtable[0xf3] = PREFIX_REPZ;
        dtable[0xf4] = HLT;
        dtable[0xf5] = CMC;
        dtable[0xf6] = INS_F6;
        dtable[0xf7] = INS_F7;
        dtable[0xf8] = CLC;
        dtable[0xf9] = STC;
        dtable[0xfa] = CLI;
        dtable[0xfb] = STI;
        dtable[0xfc] = CLD;
        dtable[0xfd] = STD;
        dtable[0xfe] = INS_FE;
        dtable[0xff] = INS_FF;
        dtable_0f[0x00] = INS_0F_00;
        dtable_0f[0x01] = INS_0F_01;
        dtable_0f[0x02] = LAR_R32_RM32;
        dtable_0f[0x03] = LSL_R32_RM32;
        dtable_0f[0x04] = UndefinedOpcode;
        dtable_0f[0x05] = UndefinedOpcode;
        dtable_0f[0x06] = CLTS;
        dtable_0f[0x07] = UndefinedOpcode;
        dtable_0f[0x08] = UndefinedOpcode;
        dtable_0f[0x09] = WBINVD;
        dtable_0f[0x0a] = UndefinedOpcode;
        dtable_0f[0x0b] = UndefinedOpcode;
        dtable_0f[0x0c] = UndefinedOpcode;
        dtable_0f[0x0d] = UndefinedOpcode;
        dtable_0f[0x0e] = UndefinedOpcode;
        dtable_0f[0x0f] = UndefinedOpcode;
        dtable_0f[0x10] = UndefinedOpcode;
        dtable_0f[0x11] = UndefinedOpcode;
        dtable_0f[0x12] = UndefinedOpcode;
        dtable_0f[0x13] = UndefinedOpcode;
        dtable_0f[0x14] = UndefinedOpcode;
        dtable_0f[0x15] = UndefinedOpcode;
        dtable_0f[0x16] = UndefinedOpcode;
        dtable_0f[0x17] = UndefinedOpcode;
        dtable_0f[0x18] = UndefinedOpcode;
        dtable_0f[0x19] = UndefinedOpcode;
        dtable_0f[0x1a] = UndefinedOpcode;
        dtable_0f[0x1b] = UndefinedOpcode;
        dtable_0f[0x1c] = UndefinedOpcode;
        dtable_0f[0x1d] = UndefinedOpcode;
        dtable_0f[0x1e] = UndefinedOpcode;
        dtable_0f[0x1f] = UndefinedOpcode;
        dtable_0f[0x20] = MOV_R32_CR;
        dtable_0f[0x21] = MOV_R32_DR;
        dtable_0f[0x22] = MOV_CR_R32;
        dtable_0f[0x23] = MOV_DR_R32;
        dtable_0f[0x24] = MOV_R32_TR;
        dtable_0f[0x25] = UndefinedOpcode;
        dtable_0f[0x26] = MOV_TR_R32;
        dtable_0f[0x27] = UndefinedOpcode;
        dtable_0f[0x28] = UndefinedOpcode;
        dtable_0f[0x29] = UndefinedOpcode;
        dtable_0f[0x2a] = UndefinedOpcode;
        dtable_0f[0x2b] = UndefinedOpcode;
        dtable_0f[0x2c] = UndefinedOpcode;
        dtable_0f[0x2d] = UndefinedOpcode;
        dtable_0f[0x2e] = UndefinedOpcode;
        dtable_0f[0x2f] = UndefinedOpcode;
        dtable_0f[0x30] = WRMSR;
        dtable_0f[0x31] = UndefinedOpcode;
        dtable_0f[0x32] = RDMSR;
        dtable_0f[0x33] = UndefinedOpcode;
        dtable_0f[0x34] = UndefinedOpcode;
        dtable_0f[0x35] = UndefinedOpcode;
        dtable_0f[0x36] = UndefinedOpcode;
        dtable_0f[0x37] = UndefinedOpcode;
        dtable_0f[0x38] = UndefinedOpcode;
        dtable_0f[0x39] = UndefinedOpcode;
        dtable_0f[0x3a] = UndefinedOpcode;
        dtable_0f[0x3b] = UndefinedOpcode;
        dtable_0f[0x3c] = UndefinedOpcode;
        dtable_0f[0x3d] = UndefinedOpcode;
        dtable_0f[0x3e] = UndefinedOpcode;
        dtable_0f[0x3f] = UndefinedOpcode;
        dtable_0f[0x40] = UndefinedOpcode;
        dtable_0f[0x41] = UndefinedOpcode;
        dtable_0f[0x42] = UndefinedOpcode;
        dtable_0f[0x43] = UndefinedOpcode;
        dtable_0f[0x44] = UndefinedOpcode;
        dtable_0f[0x45] = UndefinedOpcode;
        dtable_0f[0x46] = UndefinedOpcode;
        dtable_0f[0x47] = UndefinedOpcode;
        dtable_0f[0x48] = UndefinedOpcode;
        dtable_0f[0x49] = UndefinedOpcode;
        dtable_0f[0x4a] = UndefinedOpcode;
        dtable_0f[0x4b] = UndefinedOpcode;
        dtable_0f[0x4c] = UndefinedOpcode;
        dtable_0f[0x4d] = UndefinedOpcode;
        dtable_0f[0x4e] = UndefinedOpcode;
        dtable_0f[0x4f] = UndefinedOpcode;
        dtable_0f[0x50] = UndefinedOpcode;
        dtable_0f[0x51] = UndefinedOpcode;
        dtable_0f[0x52] = UndefinedOpcode;
        dtable_0f[0x53] = UndefinedOpcode;
        dtable_0f[0x54] = UndefinedOpcode;
        dtable_0f[0x55] = UndefinedOpcode;
        dtable_0f[0x56] = UndefinedOpcode;
        dtable_0f[0x57] = UndefinedOpcode;
        dtable_0f[0x58] = UndefinedOpcode;
        dtable_0f[0x59] = UndefinedOpcode;
        dtable_0f[0x5a] = UndefinedOpcode;
        dtable_0f[0x5b] = UndefinedOpcode;
        dtable_0f[0x5c] = UndefinedOpcode;
        dtable_0f[0x5d] = UndefinedOpcode;
        dtable_0f[0x5e] = UndefinedOpcode;
        dtable_0f[0x5f] = UndefinedOpcode;
        dtable_0f[0x60] = UndefinedOpcode;
        dtable_0f[0x61] = UndefinedOpcode;
        dtable_0f[0x62] = UndefinedOpcode;
        dtable_0f[0x63] = UndefinedOpcode;
        dtable_0f[0x64] = UndefinedOpcode;
        dtable_0f[0x65] = UndefinedOpcode;
        dtable_0f[0x66] = UndefinedOpcode;
        dtable_0f[0x67] = UndefinedOpcode;
        dtable_0f[0x68] = UndefinedOpcode;
        dtable_0f[0x69] = UndefinedOpcode;
        dtable_0f[0x6a] = UndefinedOpcode;
        dtable_0f[0x6b] = UndefinedOpcode;
        dtable_0f[0x6c] = UndefinedOpcode;
        dtable_0f[0x6d] = UndefinedOpcode;
        dtable_0f[0x6e] = UndefinedOpcode;
        dtable_0f[0x6f] = UndefinedOpcode;
        dtable_0f[0x70] = UndefinedOpcode;
        dtable_0f[0x71] = UndefinedOpcode;
        dtable_0f[0x72] = UndefinedOpcode;
        dtable_0f[0x73] = UndefinedOpcode;
        dtable_0f[0x74] = UndefinedOpcode;
        dtable_0f[0x75] = UndefinedOpcode;
        dtable_0f[0x76] = UndefinedOpcode;
        dtable_0f[0x77] = UndefinedOpcode;
        dtable_0f[0x78] = UndefinedOpcode;
        dtable_0f[0x79] = UndefinedOpcode;
        dtable_0f[0x7a] = UndefinedOpcode;
        dtable_0f[0x7b] = UndefinedOpcode;
        dtable_0f[0x7c] = UndefinedOpcode;
        dtable_0f[0x7d] = UndefinedOpcode;
        dtable_0f[0x7e] = UndefinedOpcode;
        dtable_0f[0x7f] = UndefinedOpcode;
        dtable_0f[0x80] = JO_REL32;
        dtable_0f[0x81] = JNO_REL32;
        dtable_0f[0x82] = JC_REL32;
        dtable_0f[0x83] = JNC_REL32;
        dtable_0f[0x84] = JZ_REL32;
        dtable_0f[0x85] = JNZ_REL32;
        dtable_0f[0x86] = JNA_REL32;
        dtable_0f[0x87] = JA_REL32;
        dtable_0f[0x88] = JS_REL32;
        dtable_0f[0x89] = JNS_REL32;
        dtable_0f[0x8a] = JP_REL32;
        dtable_0f[0x8b] = JNP_REL32;
        dtable_0f[0x8c] = JL_REL32;
        dtable_0f[0x8d] = JNL_REL32;
        dtable_0f[0x8e] = JNG_REL32;
        dtable_0f[0x8f] = JG_REL32;
        dtable_0f[0x90] = SETO_RM8;
        dtable_0f[0x91] = SETNO_RM8;
        dtable_0f[0x92] = SETC_RM8;
        dtable_0f[0x93] = SETNC_RM8;
        dtable_0f[0x94] = SETZ_RM8;
        dtable_0f[0x95] = SETNZ_RM8;
        dtable_0f[0x96] = SETNA_RM8;
        dtable_0f[0x97] = SETA_RM8;
        dtable_0f[0x98] = SETS_RM8;
        dtable_0f[0x99] = SETNS_RM8;
        dtable_0f[0x9a] = SETP_RM8;
        dtable_0f[0x9b] = SETNP_RM8;
        dtable_0f[0x9c] = SETL_RM8;
        dtable_0f[0x9d] = SETNL_RM8;
        dtable_0f[0x9e] = SETNG_RM8;
        dtable_0f[0x9f] = SETG_RM8;
        dtable_0f[0xa0] = PUSH_FS;
        dtable_0f[0xa1] = POP_FS;
        dtable_0f[0xa2] = CPUID;
        dtable_0f[0xa3] = BT_RM32_R32;
        dtable_0f[0xa4] = SHLD_RM32_R32_I8;
        dtable_0f[0xa5] = SHLD_RM32_R32_CL;
        dtable_0f[0xa6] = UndefinedOpcode;
        dtable_0f[0xa7] = UndefinedOpcode;
        dtable_0f[0xa8] = PUSH_GS;
        dtable_0f[0xa9] = POP_GS;
        dtable_0f[0xaa] = RSM;
        dtable_0f[0xab] = BTS_RM32_R32;
        dtable_0f[0xac] = SHRD_RM32_R32_I8;
        dtable_0f[0xad] = SHRD_RM32_R32_CL;
        dtable_0f[0xae] = UndefinedOpcode;
        dtable_0f[0xaf] = IMUL_R32_RM32;
        dtable_0f[0xb0] = UndefinedOpcode;
        dtable_0f[0xb1] = UndefinedOpcode;
        dtable_0f[0xb2] = LSS_R32_M16_32;
        dtable_0f[0xb3] = BTR_RM32_R32;
        dtable_0f[0xb4] = LFS_R32_M16_32;
        dtable_0f[0xb5] = LGS_R32_M16_32;
        dtable_0f[0xb6] = MOVZX_R32_RM8;
        dtable_0f[0xb7] = MOVZX_R32_RM16;
        dtable_0f[0xb8] = UndefinedOpcode;
        dtable_0f[0xb9] = UndefinedOpcode;
        dtable_0f[0xba] = INS_0F_BA;
        dtable_0f[0xbb] = BTC_RM32_R32;
        dtable_0f[0xbc] = BSF_R32_RM32;
        dtable_0f[0xbd] = BSR_R32_RM32;
        dtable_0f[0xbe] = MOVSX_R32_RM8;
        dtable_0f[0xbf] = MOVSX_R32_RM16;
        dtable_0f[0xc0] = UndefinedOpcode;
        dtable_0f[0xc1] = UndefinedOpcode;
        dtable_0f[0xc2] = UndefinedOpcode;
        dtable_0f[0xc3] = UndefinedOpcode;
        dtable_0f[0xc4] = UndefinedOpcode;
        dtable_0f[0xc5] = UndefinedOpcode;
        dtable_0f[0xc6] = UndefinedOpcode;
        dtable_0f[0xc7] = UndefinedOpcode;
        dtable_0f[0xc8] = UndefinedOpcode;
        dtable_0f[0xc9] = UndefinedOpcode;
        dtable_0f[0xca] = UndefinedOpcode;
        dtable_0f[0xcb] = UndefinedOpcode;
        dtable_0f[0xcc] = UndefinedOpcode;
        dtable_0f[0xcd] = UndefinedOpcode;
        dtable_0f[0xce] = UndefinedOpcode;
        dtable_0f[0xcf] = UndefinedOpcode;
        dtable_0f[0xd0] = UndefinedOpcode;
        dtable_0f[0xd1] = UndefinedOpcode;
        dtable_0f[0xd2] = UndefinedOpcode;
        dtable_0f[0xd3] = UndefinedOpcode;
        dtable_0f[0xd4] = UndefinedOpcode;
        dtable_0f[0xd5] = UndefinedOpcode;
        dtable_0f[0xd6] = UndefinedOpcode;
        dtable_0f[0xd7] = UndefinedOpcode;
        dtable_0f[0xd8] = UndefinedOpcode;
        dtable_0f[0xd9] = UndefinedOpcode;
        dtable_0f[0xda] = UndefinedOpcode;
        dtable_0f[0xdb] = UndefinedOpcode;
        dtable_0f[0xdc] = UndefinedOpcode;
        dtable_0f[0xdd] = UndefinedOpcode;
        dtable_0f[0xde] = UndefinedOpcode;
        dtable_0f[0xdf] = UndefinedOpcode;
        dtable_0f[0xe0] = UndefinedOpcode;
        dtable_0f[0xe1] = UndefinedOpcode;
        dtable_0f[0xe2] = UndefinedOpcode;
        dtable_0f[0xe3] = UndefinedOpcode;
        dtable_0f[0xe4] = UndefinedOpcode;
        dtable_0f[0xe5] = UndefinedOpcode;
        dtable_0f[0xe6] = UndefinedOpcode;
        dtable_0f[0xe7] = UndefinedOpcode;
        dtable_0f[0xe8] = UndefinedOpcode;
        dtable_0f[0xe9] = UndefinedOpcode;
        dtable_0f[0xea] = UndefinedOpcode;
        dtable_0f[0xeb] = UndefinedOpcode;
        dtable_0f[0xec] = UndefinedOpcode;
        dtable_0f[0xed] = UndefinedOpcode;
        dtable_0f[0xee] = UndefinedOpcode;
        dtable_0f[0xef] = UndefinedOpcode;
        dtable_0f[0xf0] = UndefinedOpcode;
        dtable_0f[0xf1] = UndefinedOpcode;
        dtable_0f[0xf2] = UndefinedOpcode;
        dtable_0f[0xf3] = UndefinedOpcode;
        dtable_0f[0xf4] = UndefinedOpcode;
        dtable_0f[0xf5] = UndefinedOpcode;
        dtable_0f[0xf6] = UndefinedOpcode;
        dtable_0f[0xf7] = UndefinedOpcode;
        dtable_0f[0xf8] = UndefinedOpcode;
        dtable_0f[0xf9] = UndefinedOpcode;
        dtable_0f[0xfa] = UndefinedOpcode;
        dtable_0f[0xfb] = UndefinedOpcode;
        dtable_0f[0xfc] = UndefinedOpcode;
        dtable_0f[0xfd] = UndefinedOpcode;
        dtable_0f[0xfe] = UndefinedOpcode;
        dtable_0f[0xff] = UndefinedOpcode;
        dasmContext->initialized = 1;
    }

    stmt[0] = 0;
    drcode = rcode;
    defsize = flag32;

    iop = 0;

    flagmem = 0;
    flaglock = 0;
    prefix_oprsize = 0;
    prefix_addrsize = 0;

    SPRINTF(doverds, "DS");
    SPRINTF(doverss, "SS");

    do {
        NTVDM64_TYPE_TRACE_CALL_BEGIN("dasm32");
        dop[0] = 0;
        dopr[0] = 0;
        dstmt[0] = 0;
        oldiop = iop;
        NTVDM64_TYPE_TRACE_CHECK_BREAK(_d_code((uint8_t *)(&opcode), 1));
        iop = oldiop;
        NTVDM64_TYPE_TRACE_CHECK_BREAK((*(dtable[opcode]))());
        if (STRLEN(dop)) {
            STRCAT(dop, " ");
            STRCPY(dstmt, dop);
            for (i = STRLEN(dop); i < 8; ++i) STRCAT(dstmt, " ");
            STRCAT(dstmt, dopr);
            STRCAT(stmt, dstmt);
        }
        NTVDM64_TYPE_TRACE_CALL_END;
    } while (_kdf_check_prefix(opcode));
#if DASM_TRACE == 1
    if (trace.callCount || trace.flagError) {
        PRINTF("dasm32: bad machine code.\n");
    }
    ntvdm64_type_trace_finalize(&trace);
#endif
    return iop;
}

uint8_t dasm32(char *stmt, uint8_t *rcode, int flag32) {
    dasm32_context local_context;
    dasm32_context *previous = dasmContext;
    uint8_t result;

    if (previous != NULL) return dasm32_execute(stmt, rcode, flag32);
    MEMSET(&local_context, 0, sizeof(local_context));
    dasmContext = &local_context;
    result = dasm32_execute(stmt, rcode, flag32);
    dasmContext = previous;
    return result;
}
