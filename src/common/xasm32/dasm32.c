#include "common/xasm32/xasm32.h"

#include "common/xasm32/dasm32.h"

/* Retain the original decoder tables; only their C-runtime vocabulary is
 * provided by lib/types. */
#define memset lib_memory_set
#define strlen lib_text_length
#define snprintf lib_text_format

typedef lib_u8 t_dasm_prefix;

typedef struct dasm32_context dasm32_context;

typedef void (*dasm32_handler)(dasm32_context *);

struct dasm32_context
{
    lib_u8 defsize;
    lib_u8 flagError;
    lib_u8 *drcode;
    char dstmt[0x100];
    char dop[0x100], dopr[0x100], drm[0x100], dr[0x100], dimm[0x100];
    char dmovsreg[0x100], doverds[0x100], doverss[0x100];
    char dimmoff8[0x100], dimmoff16[0x100], dimmsign[0x100];
    lib_u8 flagmem, flaglock;
    t_dasm_prefix prefix_oprsize, prefix_addrsize;
    lib_u8 cr;
    lib_u64 cimm;
    lib_u8 iop;
    dasm32_handler dtable[0x100], dtable_0f[0x100];
    lib_u8 initialized;
};

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
#define _GetModRM_RM(modrm) (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib) (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib) (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib) (((sib) & 0x07) >> 0)

#define _comment_
#define _newins_

/* Every disassembly field is an owned fixed array in dasm32_context or local scope. */
#define DASM_FORMAT_ARRAY(buffer, ...)                                             \
    do {                                                                            \
        int dasm_format_result = snprintf((buffer), sizeof(buffer),          \
            __VA_ARGS__);                                                           \
        if (dasm_format_result < 0 ||                                               \
            (lib_size)dasm_format_result >= sizeof(buffer)) {                     \
            XASM32_TRACE_SET_ERROR;                                                   \
        }                                                                           \
    } while (0)

#define DASM_COPY_ARRAY(destination, source)                                       \
    do {                                                                            \
        if (xasm32_copy_text((destination), XASM32_TEXT_CAPACITY,      \
                (source)) != LIB_STATUS_OK) XASM32_TRACE_SET_ERROR;                 \
    } while (0)

#define DASM_APPEND_ARRAY(destination, source)                                     \
    do {                                                                            \
        if (xasm32_append_text((destination), XASM32_TEXT_CAPACITY,    \
                (source)) != LIB_STATUS_OK) XASM32_TRACE_SET_ERROR;                 \
    } while (0)

static void SPRINTFSI(dasm32_context *dasmContext, char *str, lib_u32 imm, lib_u8 byte)
{
    char sign;
    lib_u8 i8u;
    lib_u16 i16u;
    lib_u32 i32u;
    XASM32_TRACE_CALL_BEGIN("SPRINTFSI");
    i8u = (lib_u8)(imm);
    i16u = (lib_u16)(imm);
    i32u = (lib_u32)(imm);
    switch (byte)
    {
    case 1:
        if ((lib_u8)(imm & 0x80))
        {
            sign = '-';
            i8u = ((~i8u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        DASM_FORMAT_ARRAY(str, "%c%02X", sign, i8u);
        break;
    case 2:
        if ((lib_u16)(imm & 0x8000))
        {
            sign = '-';
            i16u = ((~i16u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        DASM_FORMAT_ARRAY(str, "%c%04X", sign, i16u);
        break;
    case 4:
        if ((lib_u32)(imm & 0x80000000))
        {
            sign = '-';
            i32u = ((~i32u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        DASM_FORMAT_ARRAY(str, "%c%08X", sign, i32u);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

/* kernel decoding function */
static lib_u8 _kdf_check_prefix(dasm32_context *dasmContext, lib_u8 opcode)
{
    XASM32_TRACE_CALL_BEGIN("_kdf_check_prefix");
    switch (opcode)
    {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        XASM32_TRACE_CALL_END;
        return 1;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        XASM32_TRACE_CALL_END;
        return 1;
        break;
    default:
        XASM32_TRACE_CALL_END;
        return 0;
        break;
    }
    XASM32_TRACE_CALL_END;
    return 0;
}

static void _kdf_skip(dasm32_context *dasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("_kdf_skip");
    XASM32_TRACE_CHECK_RETURN(iop += byte);
    XASM32_TRACE_CALL_END;
}
static void _kdf_code(dasm32_context *dasmContext, lib_u8 *rdata, lib_u8 byte)
{
    lib_size i;
    XASM32_TRACE_CALL_BEGIN("_kdf_code");
    for (i = 0; i < byte; ++i)
        *(rdata + i) = *(drcode + iop + i);
    XASM32_TRACE_CHECK_RETURN(_kdf_skip(dasmContext, byte));
    XASM32_TRACE_CALL_END;
}
static void _kdf_modrm(dasm32_context *dasmContext, lib_u8 regbyte, lib_u8 rmbyte)
{
    char disp8;
    lib_u16 disp16;
    lib_u32 disp32;
    char dsibindex[0x100], dptr[0x100];
    lib_u8 modrm, sib;
    char sign;
    lib_u8 disp8u;
    XASM32_TRACE_CALL_BEGIN("_kdf_modrm");
    XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, &modrm, 1));
    flagmem = 1;
    drm[0] = dr[0] = dsibindex[0] = 0;
    switch (rmbyte)
    {
    case 1:
        DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
        break;
    case 2:
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        dptr[0] = 0;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+SI]", doverds);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+DI]", doverds);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+SI]", doverss);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+DI]", doverss);
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "%s:[SI]", doverds);
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "%s:[DI]", doverds);
                break;
            case 6:
                XASM32_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp16), 2));
                DASM_FORMAT_ARRAY(drm, "%s:[%04X]", doverds, disp16);
                XASM32_TRACE_BLOCK_END;
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[BX]", doverds);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }

            XASM32_TRACE_BLOCK_END;
            break;
        case 1:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+SI%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+DI%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+SI%c%02X]", doverss, sign, disp8u);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+DI%c%02X]", doverss, sign, disp8u);
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "%s:[SI%c%02X]", doverds, sign, disp8u);
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "%s:[DI%c%02X]", doverds, sign, disp8u);
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "%s:[BP%c%02X]", doverss, sign, disp8u);
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[BX%c%02X]", doverds, sign, disp8u);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case 2:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp16), 2));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+SI+%04X]", doverds, disp16);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+DI+%04X]", doverds, disp16);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+SI+%04X]", doverss, disp16);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+DI+%04X]", doverss, disp16);
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "%s:[SI+%04X]", doverds, disp16);
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "%s:[DI+%04X]", doverds, disp16);
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "%s:[BP+%04X]", doverss, disp16);
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[BX+%04X]", doverds, disp16);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4)
        {
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&sib), 1));
            switch (_GetSIB_Index(sib))
            {
            case 0:
                DASM_FORMAT_ARRAY(dsibindex, "+EAX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 1:
                DASM_FORMAT_ARRAY(dsibindex, "+ECX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 2:
                DASM_FORMAT_ARRAY(dsibindex, "+EDX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 3:
                DASM_FORMAT_ARRAY(dsibindex, "+EBX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 4:
                break;
            case 5:
                DASM_FORMAT_ARRAY(dsibindex, "+EBP*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 6:
                DASM_FORMAT_ARRAY(dsibindex, "+ESI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 7:
                DASM_FORMAT_ARRAY(dsibindex, "+EDI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
        }
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[EAX]", doverds);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[ECX]", doverds);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[EDX]", doverds);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[EBX]", doverds);
                break;
            case 4:
                XASM32_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    DASM_FORMAT_ARRAY(drm, "%s:[EAX%s]", doverds, dsibindex);
                    break;
                case 1:
                    DASM_FORMAT_ARRAY(drm, "%s:[ECX%s]", doverds, dsibindex);
                    break;
                case 2:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDX%s]", doverds, dsibindex);
                    break;
                case 3:
                    DASM_FORMAT_ARRAY(drm, "%s:[EBX%s]", doverds, dsibindex);
                    break;
                case 4:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESP%s]", doverss, dsibindex);
                    break;
                case 5:
                    XASM32_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp32), 4));
                    DASM_FORMAT_ARRAY(drm, "%s:[%08X%s]", doverds, disp32, dsibindex);
                    XASM32_TRACE_BLOCK_END;
                    break;
                case 6:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESI%s]", doverds, dsibindex);
                    break;
                case 7:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDI%s]", doverds, dsibindex);
                    break;
                default:
                    XASM32_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            case 5:
                XASM32_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp32), 4));
                DASM_FORMAT_ARRAY(drm, "%s:[%08X]", doverds, disp32);
                XASM32_TRACE_BLOCK_END;
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "%s:[ESI]", doverds);
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[EDI]", doverds);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case 1:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[EAX%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[ECX%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[EDX%c%02X]", doverds, sign, disp8u);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[EBX%c%02X]", doverds, sign, disp8u);
                break;
            case 4:
                XASM32_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    DASM_FORMAT_ARRAY(drm, "%s:[EAX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 1:
                    DASM_FORMAT_ARRAY(drm, "%s:[ECX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 2:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 3:
                    DASM_FORMAT_ARRAY(drm, "%s:[EBX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 4:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 5:
                    DASM_FORMAT_ARRAY(drm, "%s:[EBP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 6:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 7:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                default:
                    XASM32_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "%s:[EBP%c%02X]", doverss, sign, disp8u);
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "%s:[ESI%c%02X]", doverds, sign, disp8u);
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[EDI%c%02X]", doverds, sign, disp8u);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case 2:
            XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (lib_u8 *)(&disp32), 4));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "%s:[EAX+%08X]", doverds, disp32);
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "%s:[ECX+%08X]", doverds, disp32);
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "%s:[EDX+%08X]", doverds, disp32);
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "%s:[EBX+%08X]", doverds, disp32);
                break;
            case 4:
                XASM32_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    DASM_FORMAT_ARRAY(drm, "%s:[EAX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 1:
                    DASM_FORMAT_ARRAY(drm, "%s:[ECX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 2:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 3:
                    DASM_FORMAT_ARRAY(drm, "%s:[EBX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 4:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 5:
                    DASM_FORMAT_ARRAY(drm, "%s:[EBP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 6:
                    DASM_FORMAT_ARRAY(drm, "%s:[ESI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 7:
                    DASM_FORMAT_ARRAY(drm, "%s:[EDI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                default:
                    XASM32_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                XASM32_TRACE_BLOCK_END;
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "%s:[EBP+%08X]", doverss, disp32);
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "%s:[ESI+%08X]", doverds, disp32);
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "%s:[EDI+%08X]", doverds, disp32);
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            XASM32_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (_GetModRM_MOD(modrm) == 3)
    {
        XASM32_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        flagmem = 0;
        switch (rmbyte)
        {
        case 1:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "AL");
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "CL");
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "DL");
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "BL");
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "AH");
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "CH");
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "DH");
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "BH");
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "AX");
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "CX");
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "DX");
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "BX");
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "SP");
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "BP");
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "SI");
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "DI");
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                DASM_FORMAT_ARRAY(drm, "EAX");
                break;
            case 1:
                DASM_FORMAT_ARRAY(drm, "ECX");
                break;
            case 2:
                DASM_FORMAT_ARRAY(drm, "EDX");
                break;
            case 3:
                DASM_FORMAT_ARRAY(drm, "EBX");
                break;
            case 4:
                DASM_FORMAT_ARRAY(drm, "ESP");
                break;
            case 5:
                DASM_FORMAT_ARRAY(drm, "EBP");
                break;
            case 6:
                DASM_FORMAT_ARRAY(drm, "ESI");
                break;
            case 7:
                DASM_FORMAT_ARRAY(drm, "EDI");
                break;
            default:
                XASM32_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
    }
    switch (regbyte)
    {
    case 0:
        if (flagmem)
        {
            DASM_APPEND_ARRAY(dptr, drm);
            DASM_COPY_ARRAY(drm, dptr);
        }
    case 9:
        /* reg is operation or segment */
        cr = _GetModRM_REG(modrm);
        break;
    case 1:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            DASM_FORMAT_ARRAY(dr, "AL");
            break;
        case 1:
            DASM_FORMAT_ARRAY(dr, "CL");
            break;
        case 2:
            DASM_FORMAT_ARRAY(dr, "DL");
            break;
        case 3:
            DASM_FORMAT_ARRAY(dr, "BL");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dr, "AH");
            break;
        case 5:
            DASM_FORMAT_ARRAY(dr, "CH");
            break;
        case 6:
            DASM_FORMAT_ARRAY(dr, "DH");
            break;
        case 7:
            DASM_FORMAT_ARRAY(dr, "BH");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 2:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            DASM_FORMAT_ARRAY(dr, "AX");
            break;
        case 1:
            DASM_FORMAT_ARRAY(dr, "CX");
            break;
        case 2:
            DASM_FORMAT_ARRAY(dr, "DX");
            break;
        case 3:
            DASM_FORMAT_ARRAY(dr, "BX");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dr, "SP");
            break;
        case 5:
            DASM_FORMAT_ARRAY(dr, "BP");
            break;
        case 6:
            DASM_FORMAT_ARRAY(dr, "SI");
            break;
        case 7:
            DASM_FORMAT_ARRAY(dr, "DI");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 4:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            DASM_FORMAT_ARRAY(dr, "EAX");
            break;
        case 1:
            DASM_FORMAT_ARRAY(dr, "ECX");
            break;
        case 2:
            DASM_FORMAT_ARRAY(dr, "EDX");
            break;
        case 3:
            DASM_FORMAT_ARRAY(dr, "EBX");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dr, "ESP");
            break;
        case 5:
            DASM_FORMAT_ARRAY(dr, "EBP");
            break;
        case 6:
            DASM_FORMAT_ARRAY(dr, "ESI");
            break;
        case 7:
            DASM_FORMAT_ARRAY(dr, "EDI");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void _d_skip(dasm32_context *dasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("_d_skip");
    XASM32_TRACE_CHECK_RETURN(_kdf_skip(dasmContext, byte));
    XASM32_TRACE_CALL_END;
}
static void _d_code(dasm32_context *dasmContext, lib_u8 *rdata, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("_d_code");
    XASM32_TRACE_CHECK_RETURN(_kdf_code(dasmContext, rdata, byte));
    XASM32_TRACE_CALL_END;
}
static void _d_imm(dasm32_context *dasmContext, lib_u8 byte)
{
    XASM32_TRACE_CALL_BEGIN("_d_imm");
    cimm = 0;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&cimm), byte));
    XASM32_TRACE_CALL_END;
}
static void _d_moffs(dasm32_context *dasmContext, lib_u8 byte)
{
    lib_u32 offset = 0;
    XASM32_TRACE_CALL_BEGIN("_d_moffs");
    flagmem = 1;
    switch (_GetAddressSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("AddressSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&offset), 2));
        DASM_FORMAT_ARRAY(drm, "%s:[%04X]", doverds, (lib_u16)(offset));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("AddressSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&offset), 4));
        DASM_FORMAT_ARRAY(drm, "%s:[%08X]", doverds, (lib_u32)(offset));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void _d_modrm_sreg(dasm32_context *dasmContext, lib_u8 rmbyte)
{
    XASM32_TRACE_CALL_BEGIN("_d_modrm_sreg");
    XASM32_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 0, rmbyte));
    switch (cr)
    {
    case 0:
        DASM_FORMAT_ARRAY(dr, "ES");
        break;
    case 1:
        DASM_FORMAT_ARRAY(dr, "CS");
        break;
    case 2:
        DASM_FORMAT_ARRAY(dr, "SS");
        break;
    case 3:
        DASM_FORMAT_ARRAY(dr, "DS");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dr, "FS");
        break;
    case 5:
        DASM_FORMAT_ARRAY(dr, "GS");
        break;
    default:
        XASM32_TRACE_BLOCK_BEGIN("cr");
        DASM_FORMAT_ARRAY(dr, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void _d_modrm(dasm32_context *dasmContext, lib_u8 regbyte, lib_u8 rmbyte)
{
    XASM32_TRACE_CALL_BEGIN("_d_modrm");
    XASM32_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, regbyte, rmbyte));
    if (!flagmem && flaglock)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0),flaglock(1)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    XASM32_TRACE_CALL_END;
}

#define _adv XASM32_TRACE_CHECK_RETURN(_d_skip(dasmContext, 1))

static void UndefinedOpcode(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("UndefinedOpcode");
    DASM_FORMAT_ARRAY(dop, "<ERROR>");
    XASM32_TRACE_CALL_END;
}
static void ADD_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "ADD");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void ADD_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "ADD");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void ADD_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "ADD");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void ADD_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "ADD");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void ADD_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "ADD");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void ADD_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADD_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "ADD");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_ES(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ES");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "ES");
    XASM32_TRACE_CALL_END;
}
static void POP_ES(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_ES");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "ES");
    XASM32_TRACE_CALL_END;
}
static void OR_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "OR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void OR_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "OR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void OR_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "OR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void OR_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "OR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void OR_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "OR");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void OR_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OR_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OR");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void PUSH_CS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_CS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "CS");
    XASM32_TRACE_CALL_END;
}
static void POP_CS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_CS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "CS");
    XASM32_TRACE_CALL_END;
}
static void INS_0F(dasm32_context *dasmContext)
{
    lib_u8 oldiop;
    lib_u8 opcode;
    XASM32_TRACE_CALL_BEGIN("INS_0F");
    _adv;
    oldiop = iop;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&opcode), 1));
    iop = oldiop;
    XASM32_TRACE_CHECK_RETURN((*(dtable_0f[opcode]))(dasmContext));
    XASM32_TRACE_CALL_END;
}
static void ADC_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "ADC");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void ADC_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "ADC");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void ADC_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "ADC");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void ADC_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "ADC");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void ADC_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "ADC");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void ADC_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ADC_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "ADC");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void PUSH_SS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_SS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "SS");
    XASM32_TRACE_CALL_END;
}
static void POP_SS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_SS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "SS");
    XASM32_TRACE_CALL_END;
}
static void SBB_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "SBB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SBB_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "SBB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SBB_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "SBB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void SBB_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "SBB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void SBB_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "SBB");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void SBB_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SBB_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SBB");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void PUSH_DS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_DS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "DS");
    XASM32_TRACE_CALL_END;
}
static void POP_DS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_DS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "DS");
    XASM32_TRACE_CALL_END;
}
static void AND_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "AND");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void AND_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "AND");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void AND_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "AND");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void AND_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "AND");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void AND_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "AND");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void AND_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AND_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "AND");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_ES(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_ES");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "ES");
    DASM_FORMAT_ARRAY(doverss, "ES");
    XASM32_TRACE_CALL_END;
}
static void DAA(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DAA");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DAA");
    XASM32_TRACE_CALL_END;
}
static void SUB_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "SUB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SUB_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "SUB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SUB_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "SUB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void SUB_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "SUB");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void SUB_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "SUB");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void SUB_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SUB_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SUB");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_CS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_CS");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "CS");
    DASM_FORMAT_ARRAY(doverss, "CS");
    XASM32_TRACE_CALL_END;
}
static void DAS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DAS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DAS");
    XASM32_TRACE_CALL_END;
}
static void XOR_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "XOR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void XOR_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "XOR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void XOR_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "XOR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void XOR_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "XOR");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void XOR_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "XOR");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void XOR_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XOR_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XOR");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_SS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_SS");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "SS");
    DASM_FORMAT_ARRAY(doverss, "SS");
    XASM32_TRACE_CALL_END;
}
static void AAA(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAA");
    _adv;
    DASM_FORMAT_ARRAY(dop, "AAA");
    XASM32_TRACE_CALL_END;
}
static void CMP_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "CMP");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void CMP_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_RM32_R32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "CMP");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void CMP_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dop, "CMP");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void CMP_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_R32_RM32");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dop, "CMP");
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void CMP_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_AL_I8");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dop, "CMP");
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void CMP_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMP_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CMP");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_DS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_DS");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "DS");
    DASM_FORMAT_ARRAY(doverss, "DS");
    XASM32_TRACE_CALL_END;
}
static void AAS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "AAS");
    XASM32_TRACE_CALL_END;
}
static void INC_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_ECX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_ECX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "CX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ECX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_EDX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_EDX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_EBX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_EBX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_ESP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_ESP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_EBP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_EBP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_ESI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_ESI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INC_EDI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INC_EDI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_ECX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ECX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "CX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ECX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_EDX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EDX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_EBX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EBX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_ESP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ESP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_EBP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EBP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_ESI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_ESI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void DEC_EDI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("DEC_EDI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_ECX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ECX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "CX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ECX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_EDX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EDX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_EBX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EBX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_ESP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ESP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_EBP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EBP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_ESI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_ESI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_EDI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_EDI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_ECX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_ECX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "CX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ECX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_EDX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_EDX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_EBX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_EBX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_ESP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_ESP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_EBP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_EBP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BP");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBP");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_ESI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_ESI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POP_EDI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_EDI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DI");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDI");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void PUSHA(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSHA");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "PUSHA");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "PUSHAD");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POPA(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POPA");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "POPA");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "POPAD");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void BOUND_R16_M16_16(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BOUND");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize * 2));
    if (!flagmem)
    {
        DASM_FORMAT_ARRAY(dopr, "<ERROR>");
    }
    else
    {
        DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    }
    XASM32_TRACE_CALL_END;
}
static void ARPL_RM16_R16(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "ARPL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 2, 2));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void PREFIX_FS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_FS");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "FS");
    DASM_FORMAT_ARRAY(doverss, "FS");
    XASM32_TRACE_CALL_END;
}
static void PREFIX_GS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_GS");
    _adv;
    DASM_FORMAT_ARRAY(doverds, "GS");
    DASM_FORMAT_ARRAY(doverss, "GS");
    XASM32_TRACE_CALL_END;
}
static void PREFIX_OprSize(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_OprSize");
    _adv;
    prefix_oprsize = 0x01;
    XASM32_TRACE_CALL_END;
}
static void PREFIX_AddrSize(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    _adv;
    prefix_addrsize = 0x01;
    XASM32_TRACE_CALL_END;
}
static void PUSH_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IMUL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s,%s,%04X", dr, drm, (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s,%s,%08X", dr, drm, (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PUSH_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IMUL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s,%02X", dr, drm, (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void INSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("INSB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI],DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI],DX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("INSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "INSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "INSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI],DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI],DX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OUTSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("OUTSB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OUTSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OUTSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("OUTSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "OUTSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "OUTSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JO_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JO_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JO");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNO_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNO_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNO");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JC_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JC_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JC");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNC_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNC_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNC");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JZ_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JZ_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNZ_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNZ_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNA_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNA_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNA");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JA_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JA_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JA");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JS_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JS_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JS");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNS_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNS_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNS");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JP_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JP_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNP_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNP_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JL_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JL_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JL");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNL_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNL_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNL");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JNG_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNG_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNG");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JG_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JG_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JG");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void INS_80(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_80");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ADD_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "ADD");
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "OR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "ADC");
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "SBB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "AND");
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "SUB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "XOR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "CMP");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void INS_81(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_81");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ADD_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "ADD");
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("OR_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "OR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "ADC");
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "SBB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("AND_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "AND");
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "SUB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "XOR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "CMP");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s,%04X", drm, (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s,%08X", drm, (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_83(dasm32_context *dasmContext)
{
    char dsimm[0x100];
    XASM32_TRACE_CALL_BEGIN("INS_83");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ADD_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "ADD");
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("OR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "OR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "ADC");
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "SBB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("AND_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "AND");
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "SUB");
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "XOR");
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "CMP");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    SPRINTFSI(dasmContext, dsimm, (lib_u8)(cimm), 1);
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dsimm);
    XASM32_TRACE_CALL_END;
}
static void TEST_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "TEST");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void TEST_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "TEST");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void XCHG_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void XCHG_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_RM8_R8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_R8_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R8_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_RM16_SREG(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_sreg(dasmContext, 2));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void LEA_R32_M32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LEA_R32_M32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LEA");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_SREG_RM16(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_sreg(dasmContext, 2));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void INS_8F(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_8F");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize));
    switch (cr)
    {
    case 0: /* POP_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("POP_RM32");
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dop, "POP");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dop, "POPD");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1:
        XASM32_TRACE_BLOCK_BEGIN("cr(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("cr(2)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3:
        XASM32_TRACE_BLOCK_BEGIN("cr(3)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("cr(4)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5:
        XASM32_TRACE_BLOCK_BEGIN("cr(5)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6:
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("cr(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void NOP(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("NOP");
    _adv;
    DASM_FORMAT_ARRAY(dop, "NOP");
    XASM32_TRACE_CALL_END;
}
static void XCHG_ECX_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "CX,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "ECX,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_EDX_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "DX,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "EDX,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_EBX_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "BX,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "EBX,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_ESP_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "SP,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "ESP,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_EBP_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "BP,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "EBP,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_ESI_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "SI,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "ESI,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void XCHG_EDI_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        DASM_FORMAT_ARRAY(dopr, "DI,AX");
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        DASM_FORMAT_ARRAY(dopr, "EDI,EAX");
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CBW(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CBW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "CBW");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "CWDE");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CWD(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CWD");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "CWD");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "CDQ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CALL_PTR16_32(dasm32_context *dasmContext)
{
    lib_u16 newcs;
    lib_u32 neweip;
    XASM32_TRACE_CALL_BEGIN("CALL_PTR16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CALL");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        neweip = (lib_u16)(cimm);
        newcs = (lib_u16)(cimm >> 16);
        DASM_FORMAT_ARRAY(dopr, "%04X:%04X", newcs, (lib_u16)(neweip));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 8));
        neweip = (lib_u32)(cimm);
        newcs = (lib_u16)(cimm >> 32);
        DASM_FORMAT_ARRAY(dopr, "%04X:%08X", newcs, (lib_u32)(neweip));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void WAIT(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("WAIT");
    _adv;
    DASM_FORMAT_ARRAY(dop, "WAIT");
    XASM32_TRACE_CALL_END;
}
static void PUSHF(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSHF");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "PUSHF");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "PUSHFD");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void POPF(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POPF");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "POPF");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "POPFD");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SAHF(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SAHF");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SAHF");
    XASM32_TRACE_CALL_END;
}
static void LAHF(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LAHF");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LAHF");
    XASM32_TRACE_CALL_END;
}
static void MOV_AL_MOFFS8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_moffs(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "AL,%s", drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_EAX_MOFFS32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_moffs(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX,%s", drm);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX,%s", drm);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_MOFFS8_AL(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_moffs(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,AL", drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_MOFFS32_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_moffs(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s,AX", drm);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s,EAX", drm);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOVSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOVSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "MOVSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "MOVSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CMPSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("CMPSB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CMPSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CMPSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("CMPSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "CMPSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "CMPSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void TEST_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_AL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "TEST");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void TEST_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("TEST_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "TEST");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void STOSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("STOSB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "STOSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI]");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI]");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void STOSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("STOSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "STOSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "STOSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI]");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI]");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LODSB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("LODSB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LODSB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LODSW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("LODSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "LODSW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "LODSD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s:[ESI]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SCASB(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("SCASB");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SCASB");
    DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI]");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI]");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SCASW(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("SCASW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "SCASW");
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "SCASD");
        DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "ES:[DI]");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ES:[EDI]");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_CL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "CL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_DL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "DL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_BL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_BL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "BL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_AH_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_AH_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "AH,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_CH_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CH_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "CH,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_DH_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DH_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "DH,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_BH_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_BH_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "BH,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void MOV_EAX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EAX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ECX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ECX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "CX,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ECX,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EDX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EDX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDX,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EBX_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EBX_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BX,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBX,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ESP_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ESP_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SP,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESP,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EBP_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EBP_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "BP,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EBP,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_ESI_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_ESI_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "SI,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "ESI,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void MOV_EDI_I32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_EDI_I32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DI,%04X", (lib_u16)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EDI,%08X", (lib_u32)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_C0(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_C0");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_C1(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_C1");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ROL_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "ROR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void RET_I16(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RET_I16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "RET");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    DASM_FORMAT_ARRAY(dopr, "%04X", (lib_u16)(cimm));
    XASM32_TRACE_CALL_END;
}
static void RET(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RET");
    _adv;
    DASM_FORMAT_ARRAY(dop, "RET");
    XASM32_TRACE_CALL_END;
}
static void LES_R32_M16_32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LES_R32_M16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LES");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void LDS_R32_M16_32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LDS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void INS_C6(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_C6");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* MOV_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "MOV");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 1:
        XASM32_TRACE_BLOCK_BEGIN("cr(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("cr(2)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3:
        XASM32_TRACE_BLOCK_BEGIN("cr(3)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("cr(4)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5:
        XASM32_TRACE_BLOCK_BEGIN("cr(5)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6:
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("cr(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_C7(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_C7");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* MOV_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "MOV");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "%s,%04X", drm, (lib_u16)(cimm));
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "%s,%08X", drm, (lib_u32)(cimm));
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 1:
        XASM32_TRACE_BLOCK_BEGIN("cr(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("cr(2)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3:
        XASM32_TRACE_BLOCK_BEGIN("cr(3)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("cr(4)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5:
        XASM32_TRACE_BLOCK_BEGIN("cr(5)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6:
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("cr(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void ENTER(dasm32_context *dasmContext)
{
    char dframesize[0x100], dnestlevel[0x100];
    XASM32_TRACE_CALL_BEGIN("ENTER");
    _adv;
    DASM_FORMAT_ARRAY(dop, "ENTER");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    DASM_FORMAT_ARRAY(dframesize, "%04X", (lib_u16)(cimm));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dnestlevel, "%02X", (lib_u8)(cimm));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dframesize, dnestlevel);
    XASM32_TRACE_CALL_END;
}
static void LEAVE(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LEAVE");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LEAVE");
    XASM32_TRACE_CALL_END;
}
static void RETF_I16(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RETF_I16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "RETF");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    DASM_FORMAT_ARRAY(dopr, "%04X", (lib_u16)(cimm));
    XASM32_TRACE_CALL_END;
}
static void RETF(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("RETF");
    _adv;
    DASM_FORMAT_ARRAY(dop, "RETF");
    XASM32_TRACE_CALL_END;
}
static void INT3(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INT3");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INT3");
    XASM32_TRACE_CALL_END;
}
static void INT_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INT_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INT");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void INTO(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INTO");
    _adv;
    DASM_FORMAT_ARRAY(dop, "INTO");
    XASM32_TRACE_CALL_END;
}
static void IRET(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IRET");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dop, "IRET");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dop, "IRETD");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_D0(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_D0");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM8");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM8");
        DASM_FORMAT_ARRAY(dop, "ROR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM8");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM8");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM8");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM8");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM8");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_D1(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_D1");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ROL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM32");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM32");
        DASM_FORMAT_ARRAY(dop, "ROR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM32");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM32");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM32");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM32");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM32");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,01", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_D2(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_D2");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "ROR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_D3(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_D3");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ROL_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "ROL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "ROR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "RCL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "RCR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "SHL");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "SHR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_CL */
        XASM32_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
        DASM_FORMAT_ARRAY(dop, "SAR");
        DASM_FORMAT_ARRAY(dopr, "%s,CL", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void AAM(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAM");
    _adv;
    DASM_FORMAT_ARRAY(dop, "AAM");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    if ((lib_u8)(cimm) != 0x0a)
        DASM_FORMAT_ARRAY(dopr, "%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void AAD(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("AAD");
    _adv;
    DASM_FORMAT_ARRAY(dop, "AAD");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    if ((lib_u8)(cimm) != 0x0a)
        DASM_FORMAT_ARRAY(dopr, "%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void XLAT(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("XLAT");
    _adv;
    DASM_FORMAT_ARRAY(dop, "XLATB");
    switch (_GetAddressSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%s:[BX+AL]", doverds);
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%s:[EBX+AL]", doverds);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LOOPNZ_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LOOPNZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void LOOPZ_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LOOPZ_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LOOPZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void LOOP_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LOOP_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LOOP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void JCXZ_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JCXZ_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JCXZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void IN_AL_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_AL_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IN");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "AL,%02X", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void IN_EAX_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_EAX_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IN");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX,%02X", (lib_u8)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX,%02X", (lib_u8)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OUT_I8_AL(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_I8_AL");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OUT");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%02X,AL", (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void OUT_I8_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OUT");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "%02X,AX", (lib_u8)(cimm));
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "%02X,EAX", (lib_u8)(cimm));
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CALL_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CALL_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CALL");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JMP_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JMP_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JMP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JMP_PTR16_32(dasm32_context *dasmContext)
{
    lib_u16 newcs;
    lib_u32 neweip;
    XASM32_TRACE_CALL_BEGIN("JMP_PTR16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JMP");
    switch (_GetOperandSize)
    {
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(2)");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        neweip = (lib_u16)(cimm);
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        newcs = (lib_u16)(cimm);
        DASM_FORMAT_ARRAY(dopr, "%04X:%04X", newcs, (lib_u16)(neweip));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("OperandSize(4)");
        _newins_;
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        neweip = (lib_u32)(cimm);
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        newcs = (lib_u16)(cimm);
        DASM_FORMAT_ARRAY(dopr, "%04X:%08X", newcs, (lib_u32)(neweip));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JMP_REL8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JMP_REL8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JMP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (lib_u8)(cimm), 1);
    XASM32_TRACE_CALL_END;
}
static void IN_AL_DX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_AL_DX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IN");
    DASM_FORMAT_ARRAY(dopr, "AL,DX");
    XASM32_TRACE_CALL_END;
}
static void IN_EAX_DX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IN_EAX_DX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IN");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "AX,DX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "EAX,DX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void OUT_DX_AL(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_DX_AL");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OUT");
    DASM_FORMAT_ARRAY(dopr, "DX,AL");
    XASM32_TRACE_CALL_END;
}
static void OUT_DX_EAX(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _adv;
    DASM_FORMAT_ARRAY(dop, "OUT");
    switch (_GetOperandSize)
    {
    case 2:
        DASM_FORMAT_ARRAY(dopr, "DX,AX");
        break;
    case 4:
        DASM_FORMAT_ARRAY(dopr, "DX,EAX");
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void PREFIX_LOCK(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_LOCK");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LOCK:");
    XASM32_TRACE_CALL_END;
}
static void PREFIX_REPNZ(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    _adv;
    DASM_FORMAT_ARRAY(dop, "REPNZ:");
    XASM32_TRACE_CALL_END;
}
static void PREFIX_REPZ(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PREFIX_REPZ");
    _adv;
    DASM_FORMAT_ARRAY(dop, "REPZ:");
    XASM32_TRACE_CALL_END;
}
static void HLT(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("HLT");
    _adv;
    DASM_FORMAT_ARRAY(dop, "HLT");
    XASM32_TRACE_CALL_END;
}
static void CMC(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CMC");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CMC");
    XASM32_TRACE_CALL_END;
}
static void INS_F6(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_F6");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* TEST_RM8_I8 */
        XASM32_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        DASM_FORMAT_ARRAY(dop, "TEST");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("NOT_RM8");
        DASM_FORMAT_ARRAY(dop, "NOT");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("NEG_RM8");
        DASM_FORMAT_ARRAY(dop, "NEG");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("MUL_RM8");
        DASM_FORMAT_ARRAY(dop, "MUL");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("IMUL_RM8");
        DASM_FORMAT_ARRAY(dop, "IMUL");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("DIV_RM8");
        DASM_FORMAT_ARRAY(dop, "DIV");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("IDIV_RM8");
        DASM_FORMAT_ARRAY(dop, "IDIV");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_F7(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_F7");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* TEST_RM32_I32 */
        XASM32_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
        DASM_FORMAT_ARRAY(dop, "TEST");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "%s,%04X", drm, (lib_u16)(cimm));
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "%s,%08X", drm, (lib_u32)(cimm));
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("NOT_RM32");
        DASM_FORMAT_ARRAY(dop, "NOT");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("NEG_RM16");
        DASM_FORMAT_ARRAY(dop, "NEG");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("MUL_RM32");
        DASM_FORMAT_ARRAY(dop, "MUL");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("IMUL_RM32");
        DASM_FORMAT_ARRAY(dop, "IMUL");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("DIV_RM32");
        DASM_FORMAT_ARRAY(dop, "DIV");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("IDIV_RM32");
        DASM_FORMAT_ARRAY(dop, "IDIV");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void CLC(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLC");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CLC");
    XASM32_TRACE_CALL_END;
}
static void STC(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STC");
    _adv;
    DASM_FORMAT_ARRAY(dop, "STC");
    XASM32_TRACE_CALL_END;
}
static void CLI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CLI");
    XASM32_TRACE_CALL_END;
}
static void STI(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STI");
    _adv;
    DASM_FORMAT_ARRAY(dop, "STI");
    XASM32_TRACE_CALL_END;
}
static void CLD(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLD");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CLD");
    XASM32_TRACE_CALL_END;
}
static void STD(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("STD");
    _adv;
    DASM_FORMAT_ARRAY(dop, "STD");
    XASM32_TRACE_CALL_END;
}
static void INS_FE(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("INS_FE");
    _adv;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* INC_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("INC_RM8");
        DASM_FORMAT_ARRAY(dop, "INC");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        XASM32_TRACE_BLOCK_BEGIN("DEC_RM8");
        DASM_FORMAT_ARRAY(dop, "DEC");
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("cr(2)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3:
        XASM32_TRACE_BLOCK_BEGIN("cr(3)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4:
        XASM32_TRACE_BLOCK_BEGIN("cr(4)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5:
        XASM32_TRACE_BLOCK_BEGIN("cr(5)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6:
        XASM32_TRACE_BLOCK_BEGIN("cr(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("cr(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_FF(dasm32_context *dasmContext)
{
    char dptr[0x100];
    lib_u8 oldiop;
    lib_u8 modrm;
    XASM32_TRACE_CALL_BEGIN("INS_FF");
    _adv;
    oldiop = iop;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* INC_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("INC_RM32");
        DASM_FORMAT_ARRAY(dop, "INC");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("DEC_RM32");
        DASM_FORMAT_ARRAY(dop, "DEC");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* CALL_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("CALL_RM32");
        DASM_FORMAT_ARRAY(dop, "CALL");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* CALL_M16_32 */
        XASM32_TRACE_BLOCK_BEGIN("CALL_M16_32");
        DASM_FORMAT_ARRAY(dop, "CALL");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize + 2));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        DASM_FORMAT_ARRAY(dopr, "FAR %s%s", dptr, drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* JMP_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("JMP_RM32");
        DASM_FORMAT_ARRAY(dop, "JMP");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* JMP_M16_32 */
        XASM32_TRACE_BLOCK_BEGIN("JMP_M16_32");
        DASM_FORMAT_ARRAY(dop, "JMP");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize + 2));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
            break;
        case 4:
            DASM_FORMAT_ARRAY(dptr, "DWORD PTR ");
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        DASM_FORMAT_ARRAY(dopr, "FAR %s%s", dptr, drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* PUSH_RM32 */
        XASM32_TRACE_BLOCK_BEGIN("PUSH_RM32");
        DASM_FORMAT_ARRAY(dop, "PUSH");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* UndefinedOpcode */
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void _d_modrm_creg(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("_d_modrm_creg");
    XASM32_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(1)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 0:
        DASM_FORMAT_ARRAY(dr, "CR0");
        break;
    case 2:
        DASM_FORMAT_ARRAY(dr, "CR2");
        break;
    case 3:
        DASM_FORMAT_ARRAY(dr, "CR3");
        break;
        ;
    default:
        DASM_FORMAT_ARRAY(dr, "<ERROR>");
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void _d_modrm_dreg(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("_d_modrm_dreg");
    XASM32_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(1)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 0:
        DASM_FORMAT_ARRAY(dr, "DR0");
        break;
    case 1:
        DASM_FORMAT_ARRAY(dr, "DR1");
        break;
    case 2:
        DASM_FORMAT_ARRAY(dr, "DR2");
        break;
    case 3:
        DASM_FORMAT_ARRAY(dr, "DR3");
        break;
    case 6:
        DASM_FORMAT_ARRAY(dr, "DR6");
        break;
    case 7:
        DASM_FORMAT_ARRAY(dr, "DR7");
        break;
    default:
        DASM_FORMAT_ARRAY(dr, "<ERROR>");
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void _d_modrm_treg(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("_d_modrm_treg");
    XASM32_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(1)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 6:
        DASM_FORMAT_ARRAY(dr, "TR6");
        break;
    case 7:
        DASM_FORMAT_ARRAY(dr, "TR7");
        break;
    default:
        DASM_FORMAT_ARRAY(dr, "<ERROR>");
        break;
    }
    XASM32_TRACE_CALL_END;
}

static void INS_0F_00(dasm32_context *dasmContext)
{
    lib_u8 modrm, oldiop;
    XASM32_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    oldiop = iop;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SLDT_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("SLDT_RM16");
        DASM_FORMAT_ARRAY(dop, "SLDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* STR_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("STR_RM16");
        DASM_FORMAT_ARRAY(dop, "STR");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* LLDT_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("LLDT_RM16");
        DASM_FORMAT_ARRAY(dop, "LLDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* LTR_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("LTR_RM16");
        DASM_FORMAT_ARRAY(dop, "LTR");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* VERR_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("VERR_RM16");
        DASM_FORMAT_ARRAY(dop, "VERR");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* VERW_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("VERW_RM16");
        DASM_FORMAT_ARRAY(dop, "VERW");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 6:
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void INS_0F_01(dasm32_context *dasmContext)
{
    lib_u8 modrm, oldiop;
    XASM32_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    oldiop = iop;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SGDT_M32_16 */
        XASM32_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        DASM_FORMAT_ARRAY(dop, "SGDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "DWORD PTR %s", drm);
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        XASM32_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        DASM_FORMAT_ARRAY(dop, "SIDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "DWORD PTR %s", drm);
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        XASM32_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        DASM_FORMAT_ARRAY(dop, "LGDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "DWORD PTR %s", drm);
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        XASM32_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        DASM_FORMAT_ARRAY(dop, "LIDT");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
            DASM_FORMAT_ARRAY(drm, "<ERROR>");
            XASM32_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            DASM_FORMAT_ARRAY(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            DASM_FORMAT_ARRAY(dopr, "DWORD PTR %s", drm);
            break;
        default:
            XASM32_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("SMSW_RM16");
        DASM_FORMAT_ARRAY(dop, "SMSW");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 5:
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        XASM32_TRACE_BLOCK_BEGIN("LMSW_RM16");
        DASM_FORMAT_ARRAY(dop, "LMSW");
        XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        DASM_FORMAT_ARRAY(dopr, "%s", drm);
        XASM32_TRACE_BLOCK_END;
        break;
    case 7:
        XASM32_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void LAR_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LAR");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void LSL_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LSL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void CLTS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("CLTS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "CLTS");
    XASM32_TRACE_CALL_END;
}
static void WBINVD(dasm32_context *dasmContext) {}
static void MOV_R32_CR(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_creg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_DR(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_DR");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_dreg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_CR_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_creg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_DR_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_DR_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_dreg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOV_R32_TR(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_R32_TR");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_treg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void MOV_TR_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("MOV_TR_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOV");
    XASM32_TRACE_CHECK_RETURN(_d_modrm_treg(dasmContext));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void WRMSR(dasm32_context *dasmContext) {}
static void RDMSR(dasm32_context *dasmContext) {}
static void JO_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JO_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JO");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNO_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNO_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNO");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JC_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JC");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNC_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNC_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNC");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JZ_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNZ_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNZ");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNA_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNA_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNA");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JA_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JA_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JA");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JS_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JS_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JS");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNS_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNS_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNS");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JP_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JP_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNP_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNP_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNP");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JL_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JL_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JL");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNL_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNL_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNL");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JNG_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JNG_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JNG");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void JG_REL32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("JG_REL32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "JG");
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (lib_u16)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (lib_u32)(cimm), 4);
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void SETO_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETO");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNO_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNO");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETC_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETC_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETC");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNC_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNC_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNC");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETZ_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETZ_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETZ");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNZ_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNZ_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNZ");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNA_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNA_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNA");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETA_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETA_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETA");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETS_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETS_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNS_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNS_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETP_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETP_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETP");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNP_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNP_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNP");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETL_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETL_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNL_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNL_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETNG_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETNG_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETNG");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void SETG_RM8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SETG_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SETG");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    DASM_FORMAT_ARRAY(dopr, "%s", drm);
    XASM32_TRACE_CALL_END;
}
static void PUSH_FS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "FS");
    XASM32_TRACE_CALL_END;
}
static void POP_FS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "FS");
    XASM32_TRACE_CALL_END;
}
static void CPUID(dasm32_context *dasmContext) {}
static void BT_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BT");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SHLD_RM32_R32_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SHLD");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s,%02X", drm, dr, (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void SHLD_RM32_R32_CL(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SHLD");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s,CL", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void PUSH_GS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "PUSH");
    DASM_FORMAT_ARRAY(dopr, "GS");
    XASM32_TRACE_CALL_END;
}
static void POP_GS(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    DASM_FORMAT_ARRAY(dop, "POP");
    DASM_FORMAT_ARRAY(dopr, "GS");
    XASM32_TRACE_CALL_END;
}
static void RSM(dasm32_context *dasmContext) {}
static void BTS_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BTS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void SHRD_RM32_R32_I8(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SHRD");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    DASM_FORMAT_ARRAY(dopr, "%s,%s,%02X", drm, dr, (lib_u8)(cimm));
    XASM32_TRACE_CALL_END;
}
static void SHRD_RM32_R32_CL(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    DASM_FORMAT_ARRAY(dop, "SHRD");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s,CL", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void IMUL_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "IMUL");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void LSS_R32_M16_32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LSS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void BTR_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BTR");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void LFS_R32_M16_32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LFS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void LGS_R32_M16_32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "LGS");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        XASM32_TRACE_BLOCK_BEGIN("flagmem(0)");
        DASM_FORMAT_ARRAY(drm, "<ERROR>");
        XASM32_TRACE_BLOCK_END;
    }
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOVZX_R32_RM8(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOVZX");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, 1));
    if (flagmem)
        DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    else
        dptr[0] = 0;
    DASM_FORMAT_ARRAY(dopr, "%s,%s%s", dr, dptr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOVZX_R32_RM16(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOVZX");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 4, 2));
    if (flagmem)
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
    else
        dptr[0] = 0;
    DASM_FORMAT_ARRAY(dopr, "%s,%s%s", dr, dptr, drm);
    XASM32_TRACE_CALL_END;
}
static void INS_0F_BA(dasm32_context *dasmContext)
{
    lib_u8 modrm, oldiop;
    XASM32_TRACE_CALL_BEGIN("INS_0F_BA");
    _adv;
    oldiop = iop;
    XASM32_TRACE_CHECK_RETURN(_d_code(dasmContext, (lib_u8 *)(&modrm), 1));
    iop = oldiop;
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0:
        XASM32_TRACE_BLOCK_BEGIN("cr(0)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 1:
        XASM32_TRACE_BLOCK_BEGIN("cr(1)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 2:
        XASM32_TRACE_BLOCK_BEGIN("cr(2)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 3:
        XASM32_TRACE_BLOCK_BEGIN("cr(3)");
        XASM32_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        XASM32_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "BT");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "BTS");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "BTR");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        XASM32_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        DASM_FORMAT_ARRAY(dop, "BTC");
        XASM32_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        DASM_FORMAT_ARRAY(dopr, "%s,%02X", drm, (lib_u8)(cimm));
        XASM32_TRACE_BLOCK_END;
        break;
    default:
        XASM32_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    XASM32_TRACE_CALL_END;
}
static void BTC_RM32_R32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BTC");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", drm, dr);
    XASM32_TRACE_CALL_END;
}
static void BSF_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BSF");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void BSR_R32_RM32(dasm32_context *dasmContext)
{
    XASM32_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _adv;
    DASM_FORMAT_ARRAY(dop, "BSR");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    DASM_FORMAT_ARRAY(dopr, "%s,%s", dr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOVSX_R32_RM8(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOVSX");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, 1));
    if (flagmem)
        DASM_FORMAT_ARRAY(dptr, "BYTE PTR ");
    else
        dptr[0] = 0;
    DASM_FORMAT_ARRAY(dopr, "%s,%s%s", dr, dptr, drm);
    XASM32_TRACE_CALL_END;
}
static void MOVSX_R32_RM16(dasm32_context *dasmContext)
{
    char dptr[0x100];
    XASM32_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    DASM_FORMAT_ARRAY(dop, "MOVSX");
    XASM32_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 4, 2));
    if (flagmem)
        DASM_FORMAT_ARRAY(dptr, "WORD PTR ");
    else
        dptr[0] = 0;
    DASM_FORMAT_ARRAY(dopr, "%s,%s%s", dr, dptr, drm);
    XASM32_TRACE_CALL_END;
}
static lib_u8 dasm32_execute(dasm32_context *dasmContext, char *stmt, lib_u8 *rcode, int flag32)
{
    lib_size i;
    lib_u8 opcode, oldiop;
    if (!dasmContext->initialized)
    {
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

    DASM_FORMAT_ARRAY(doverds, "DS");
    DASM_FORMAT_ARRAY(doverss, "SS");

    do
    {
        XASM32_TRACE_CALL_BEGIN("dasm32");
        dop[0] = 0;
        dopr[0] = 0;
        dstmt[0] = 0;
        oldiop = iop;
        XASM32_TRACE_CHECK_BREAK(_d_code(dasmContext, (lib_u8 *)(&opcode), 1));
        iop = oldiop;
        XASM32_TRACE_CHECK_BREAK((*(dtable[opcode]))(dasmContext));
        if (strlen(dop))
        {
            DASM_APPEND_ARRAY(dop, " ");
            DASM_COPY_ARRAY(dstmt, dop);
            for (i = strlen(dop); i < 8; ++i)
                DASM_APPEND_ARRAY(dstmt, " ");
            DASM_APPEND_ARRAY(dstmt, dopr);
            DASM_APPEND_ARRAY(stmt, dstmt);
        }
        XASM32_TRACE_CALL_END;
    } while (_kdf_check_prefix(dasmContext, opcode));
    if (flagError) {
        stmt[0] = 0;
        return 0u;
    }
    return iop;
}

lib_u8 dasm32(char *stmt, lib_u8 *rcode, int flag32)
{
    dasm32_context local_context;

    memset(&local_context, 0, sizeof(local_context));
    return dasm32_execute(&local_context, stmt, rcode, flag32);
}
