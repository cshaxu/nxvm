#include "type.h"

#include "core/product/utils.h"

#include "core/product/debug/xasm32/dasm32.h"

#define TYPE_TRACE_CONTEXT trace
#define TYPE_TRACE_ERROR flagError
#define TYPE_TRACE_SET_ERROR (flagError = 1)

typedef uint8_t t_dasm_prefix;

typedef struct dasm32_context dasm32_context;

typedef C_VOID (*dasm32_handler)(dasm32_context *);

struct dasm32_context
{
    type_trace trace;
    uint8_t defsize;
    uint8_t flagError;
    uint8_t *drcode;
    C_CHAR dstmt[0x100];
    C_CHAR dop[0x100], dopr[0x100], drm[0x100], dr[0x100], dimm[0x100];
    C_CHAR dmovsreg[0x100], doverds[0x100], doverss[0x100];
    C_CHAR dimmoff8[0x100], dimmoff16[0x100], dimmsign[0x100];
    uint8_t flagmem, flaglock;
    t_dasm_prefix prefix_oprsize, prefix_addrsize;
    uint8_t cr;
    uint64_t cimm;
    uint8_t iop;
    dasm32_handler dtable[0x100], dtable_0f[0x100];
    uint8_t initialized;
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

static C_VOID SPRINTFSI(dasm32_context *dasmContext, C_CHAR *str, uint32_t imm, uint8_t byte)
{
    C_CHAR sign;
    uint8_t i8u;
    uint16_t i16u;
    uint32_t i32u;
    TYPE_TRACE_CALL_BEGIN("SPRINTFSI");
    i8u = (uint8_t)(imm);
    i16u = (uint16_t)(imm);
    i32u = (uint32_t)(imm);
    switch (byte)
    {
    case 1:
        if ((uint8_t)(imm & 0x80))
        {
            sign = '-';
            i8u = ((~i8u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        STD_SPRINTF(str, "%c%02X", sign, i8u);
        break;
    case 2:
        if ((uint16_t)(imm & 0x8000))
        {
            sign = '-';
            i16u = ((~i16u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        STD_SPRINTF(str, "%c%04X", sign, i16u);
        break;
    case 4:
        if ((uint32_t)(imm & 0x80000000))
        {
            sign = '-';
            i32u = ((~i32u) + 0x01);
        }
        else
        {
            sign = '+';
        }
        STD_SPRINTF(str, "%c%08X", sign, i32u);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

/* kernel decoding function */
static uint8_t _kdf_check_prefix(dasm32_context *dasmContext, uint8_t opcode)
{
    TYPE_TRACE_CALL_BEGIN("_kdf_check_prefix");
    switch (opcode)
    {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        TYPE_TRACE_CALL_END;
        return 1;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        TYPE_TRACE_CALL_END;
        return 1;
        break;
    default:
        TYPE_TRACE_CALL_END;
        return 0;
        break;
    }
    TYPE_TRACE_CALL_END;
    return 0;
}

static C_VOID _kdf_skip(dasm32_context *dasmContext, uint8_t byte)
{
    TYPE_TRACE_CALL_BEGIN("_kdf_skip");
    TYPE_TRACE_CHECK_RETURN(iop += byte);
    TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_code(dasm32_context *dasmContext, uint8_t *rdata, uint8_t byte)
{
    STD_SIZE_T i;
    TYPE_TRACE_CALL_BEGIN("_kdf_code");
    for (i = 0; i < byte; ++i)
        *(rdata + i) = *(drcode + iop + i);
    TYPE_TRACE_CHECK_RETURN(_kdf_skip(dasmContext, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_modrm(dasm32_context *dasmContext, uint8_t regbyte, uint8_t rmbyte)
{
    C_CHAR disp8;
    uint16_t disp16;
    uint32_t disp32;
    C_CHAR dsibindex[0x100], dptr[0x100];
    uint8_t modrm, sib;
    C_CHAR sign;
    uint8_t disp8u;
    TYPE_TRACE_CALL_BEGIN("_kdf_modrm");
    TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, &modrm, 1));
    flagmem = 1;
    drm[0] = dr[0] = dsibindex[0] = 0;
    switch (rmbyte)
    {
    case 1:
        STD_SPRINTF(dptr, "BYTE PTR ");
        break;
    case 2:
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        dptr[0] = 0;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[BX+SI]", doverds);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[BX+DI]", doverds);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[BP+SI]", doverss);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[BP+DI]", doverss);
                break;
            case 4:
                STD_SPRINTF(drm, "%s:[SI]", doverds);
                break;
            case 5:
                STD_SPRINTF(drm, "%s:[DI]", doverds);
                break;
            case 6:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp16), 2));
                STD_SPRINTF(drm, "%s:[%04X]", doverds, disp16);
                TYPE_TRACE_BLOCK_END;
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[BX]", doverds);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }

            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[BX+SI%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[BX+DI%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[BP+SI%c%02X]", doverss, sign, disp8u);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[BP+DI%c%02X]", doverss, sign, disp8u);
                break;
            case 4:
                STD_SPRINTF(drm, "%s:[SI%c%02X]", doverds, sign, disp8u);
                break;
            case 5:
                STD_SPRINTF(drm, "%s:[DI%c%02X]", doverds, sign, disp8u);
                break;
            case 6:
                STD_SPRINTF(drm, "%s:[BP%c%02X]", doverss, sign, disp8u);
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[BX%c%02X]", doverds, sign, disp8u);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp16), 2));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[BX+SI+%04X]", doverds, disp16);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[BX+DI+%04X]", doverds, disp16);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[BP+SI+%04X]", doverss, disp16);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[BP+DI+%04X]", doverss, disp16);
                break;
            case 4:
                STD_SPRINTF(drm, "%s:[SI+%04X]", doverds, disp16);
                break;
            case 5:
                STD_SPRINTF(drm, "%s:[DI+%04X]", doverds, disp16);
                break;
            case 6:
                STD_SPRINTF(drm, "%s:[BP+%04X]", doverss, disp16);
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[BX+%04X]", doverds, disp16);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4)
        {
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&sib), 1));
            switch (_GetSIB_Index(sib))
            {
            case 0:
                STD_SPRINTF(dsibindex, "+EAX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 1:
                STD_SPRINTF(dsibindex, "+ECX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 2:
                STD_SPRINTF(dsibindex, "+EDX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 3:
                STD_SPRINTF(dsibindex, "+EBX*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 4:
                break;
            case 5:
                STD_SPRINTF(dsibindex, "+EBP*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 6:
                STD_SPRINTF(dsibindex, "+ESI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            case 7:
                STD_SPRINTF(dsibindex, "+EDI*%02X", (1 << _GetSIB_SS(sib)));
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
        }
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[EAX]", doverds);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[ECX]", doverds);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[EDX]", doverds);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[EBX]", doverds);
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    STD_SPRINTF(drm, "%s:[EAX%s]", doverds, dsibindex);
                    break;
                case 1:
                    STD_SPRINTF(drm, "%s:[ECX%s]", doverds, dsibindex);
                    break;
                case 2:
                    STD_SPRINTF(drm, "%s:[EDX%s]", doverds, dsibindex);
                    break;
                case 3:
                    STD_SPRINTF(drm, "%s:[EBX%s]", doverds, dsibindex);
                    break;
                case 4:
                    STD_SPRINTF(drm, "%s:[ESP%s]", doverss, dsibindex);
                    break;
                case 5:
                    TYPE_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp32), 4));
                    STD_SPRINTF(drm, "%s:[%08X%s]", doverds, disp32, dsibindex);
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 6:
                    STD_SPRINTF(drm, "%s:[ESI%s]", doverds, dsibindex);
                    break;
                case 7:
                    STD_SPRINTF(drm, "%s:[EDI%s]", doverds, dsibindex);
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp32), 4));
                STD_SPRINTF(drm, "%s:[%08X]", doverds, disp32);
                TYPE_TRACE_BLOCK_END;
                break;
            case 6:
                STD_SPRINTF(drm, "%s:[ESI]", doverds);
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[EDI]", doverds);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp8), 1));
            sign = (disp8 & 0x80) ? '-' : '+';
            disp8u = (disp8 & 0x80) ? ((~disp8) + 0x01) : disp8;
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[EAX%c%02X]", doverds, sign, disp8u);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[ECX%c%02X]", doverds, sign, disp8u);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[EDX%c%02X]", doverds, sign, disp8u);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[EBX%c%02X]", doverds, sign, disp8u);
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    STD_SPRINTF(drm, "%s:[EAX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 1:
                    STD_SPRINTF(drm, "%s:[ECX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 2:
                    STD_SPRINTF(drm, "%s:[EDX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 3:
                    STD_SPRINTF(drm, "%s:[EBX%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 4:
                    STD_SPRINTF(drm, "%s:[ESP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 5:
                    STD_SPRINTF(drm, "%s:[EBP%s%c%02X]", doverss, dsibindex, sign, disp8u);
                    break;
                case 6:
                    STD_SPRINTF(drm, "%s:[ESI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                case 7:
                    STD_SPRINTF(drm, "%s:[EDI%s%c%02X]", doverds, dsibindex, sign, disp8u);
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                STD_SPRINTF(drm, "%s:[EBP%c%02X]", doverss, sign, disp8u);
                break;
            case 6:
                STD_SPRINTF(drm, "%s:[ESI%c%02X]", doverds, sign, disp8u);
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[EDI%c%02X]", doverds, sign, disp8u);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, (uint8_t *)(&disp32), 4));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "%s:[EAX+%08X]", doverds, disp32);
                break;
            case 1:
                STD_SPRINTF(drm, "%s:[ECX+%08X]", doverds, disp32);
                break;
            case 2:
                STD_SPRINTF(drm, "%s:[EDX+%08X]", doverds, disp32);
                break;
            case 3:
                STD_SPRINTF(drm, "%s:[EBX+%08X]", doverds, disp32);
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
                case 0:
                    STD_SPRINTF(drm, "%s:[EAX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 1:
                    STD_SPRINTF(drm, "%s:[ECX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 2:
                    STD_SPRINTF(drm, "%s:[EDX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 3:
                    STD_SPRINTF(drm, "%s:[EBX%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 4:
                    STD_SPRINTF(drm, "%s:[ESP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 5:
                    STD_SPRINTF(drm, "%s:[EBP%s+%08X]", doverss, dsibindex, disp32);
                    break;
                case 6:
                    STD_SPRINTF(drm, "%s:[ESI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                case 7:
                    STD_SPRINTF(drm, "%s:[EDI%s+%08X]", doverds, dsibindex, disp32);
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                STD_SPRINTF(drm, "%s:[EBP+%08X]", doverss, disp32);
                break;
            case 6:
                STD_SPRINTF(drm, "%s:[ESI+%08X]", doverds, disp32);
                break;
            case 7:
                STD_SPRINTF(drm, "%s:[EDI+%08X]", doverds, disp32);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (_GetModRM_MOD(modrm) == 3)
    {
        TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        flagmem = 0;
        switch (rmbyte)
        {
        case 1:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "AL");
                break;
            case 1:
                STD_SPRINTF(drm, "CL");
                break;
            case 2:
                STD_SPRINTF(drm, "DL");
                break;
            case 3:
                STD_SPRINTF(drm, "BL");
                break;
            case 4:
                STD_SPRINTF(drm, "AH");
                break;
            case 5:
                STD_SPRINTF(drm, "CH");
                break;
            case 6:
                STD_SPRINTF(drm, "DH");
                break;
            case 7:
                STD_SPRINTF(drm, "BH");
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "AX");
                break;
            case 1:
                STD_SPRINTF(drm, "CX");
                break;
            case 2:
                STD_SPRINTF(drm, "DX");
                break;
            case 3:
                STD_SPRINTF(drm, "BX");
                break;
            case 4:
                STD_SPRINTF(drm, "SP");
                break;
            case 5:
                STD_SPRINTF(drm, "BP");
                break;
            case 6:
                STD_SPRINTF(drm, "SI");
                break;
            case 7:
                STD_SPRINTF(drm, "DI");
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                STD_SPRINTF(drm, "EAX");
                break;
            case 1:
                STD_SPRINTF(drm, "ECX");
                break;
            case 2:
                STD_SPRINTF(drm, "EDX");
                break;
            case 3:
                STD_SPRINTF(drm, "EBX");
                break;
            case 4:
                STD_SPRINTF(drm, "ESP");
                break;
            case 5:
                STD_SPRINTF(drm, "EBP");
                break;
            case 6:
                STD_SPRINTF(drm, "ESI");
                break;
            case 7:
                STD_SPRINTF(drm, "EDI");
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
    }
    switch (regbyte)
    {
    case 0:
        if (flagmem)
        {
            STD_STRCAT(dptr, drm);
            STD_STRCPY(drm, dptr);
        }
    case 9:
        /* reg is operation or segment */
        cr = _GetModRM_REG(modrm);
        break;
    case 1:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            STD_SPRINTF(dr, "AL");
            break;
        case 1:
            STD_SPRINTF(dr, "CL");
            break;
        case 2:
            STD_SPRINTF(dr, "DL");
            break;
        case 3:
            STD_SPRINTF(dr, "BL");
            break;
        case 4:
            STD_SPRINTF(dr, "AH");
            break;
        case 5:
            STD_SPRINTF(dr, "CH");
            break;
        case 6:
            STD_SPRINTF(dr, "DH");
            break;
        case 7:
            STD_SPRINTF(dr, "BH");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 2:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            STD_SPRINTF(dr, "AX");
            break;
        case 1:
            STD_SPRINTF(dr, "CX");
            break;
        case 2:
            STD_SPRINTF(dr, "DX");
            break;
        case 3:
            STD_SPRINTF(dr, "BX");
            break;
        case 4:
            STD_SPRINTF(dr, "SP");
            break;
        case 5:
            STD_SPRINTF(dr, "BP");
            break;
        case 6:
            STD_SPRINTF(dr, "SI");
            break;
        case 7:
            STD_SPRINTF(dr, "DI");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    case 4:
        switch (_GetModRM_REG(modrm))
        {
        case 0:
            STD_SPRINTF(dr, "EAX");
            break;
        case 1:
            STD_SPRINTF(dr, "ECX");
            break;
        case 2:
            STD_SPRINTF(dr, "EDX");
            break;
        case 3:
            STD_SPRINTF(dr, "EBX");
            break;
        case 4:
            STD_SPRINTF(dr, "ESP");
            break;
        case 5:
            STD_SPRINTF(dr, "EBP");
            break;
        case 6:
            STD_SPRINTF(dr, "ESI");
            break;
        case 7:
            STD_SPRINTF(dr, "EDI");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_skip(dasm32_context *dasmContext, uint8_t byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_skip");
    TYPE_TRACE_CHECK_RETURN(_kdf_skip(dasmContext, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_code(dasm32_context *dasmContext, uint8_t *rdata, uint8_t byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_code");
    TYPE_TRACE_CHECK_RETURN(_kdf_code(dasmContext, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_imm(dasm32_context *dasmContext, uint8_t byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_imm");
    cimm = 0;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&cimm), byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_moffs(dasm32_context *dasmContext, uint8_t byte)
{
    uint32_t offset = 0;
    TYPE_TRACE_CALL_BEGIN("_d_moffs");
    flagmem = 1;
    switch (_GetAddressSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&offset), 2));
        STD_SPRINTF(drm, "%s:[%04X]", doverds, (uint16_t)(offset));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&offset), 4));
        STD_SPRINTF(drm, "%s:[%08X]", doverds, (uint32_t)(offset));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_sreg(dasm32_context *dasmContext, uint8_t rmbyte)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_sreg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 0, rmbyte));
    switch (cr)
    {
    case 0:
        STD_SPRINTF(dr, "ES");
        break;
    case 1:
        STD_SPRINTF(dr, "CS");
        break;
    case 2:
        STD_SPRINTF(dr, "SS");
        break;
    case 3:
        STD_SPRINTF(dr, "DS");
        break;
    case 4:
        STD_SPRINTF(dr, "FS");
        break;
    case 5:
        STD_SPRINTF(dr, "GS");
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("cr");
        STD_SPRINTF(dr, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm(dasm32_context *dasmContext, uint8_t regbyte, uint8_t rmbyte)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, regbyte, rmbyte));
    if (!flagmem && flaglock)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0),flaglock(1)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}

#define _adv TYPE_TRACE_CHECK_RETURN(_d_skip(dasmContext, 1))

static C_VOID UndefinedOpcode(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("UndefinedOpcode");
    STD_SPRINTF(dop, "<ERROR>");
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "ADD");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "ADD");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "ADD");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "ADD");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "ADD");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADD_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "ADD");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ES(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ES");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "ES");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ES(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_ES");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "ES");
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "OR");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "OR");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "OR");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "OR");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "OR");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OR_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "OR");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID PUSH_CS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_CS");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "CS");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_CS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_CS");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "CS");
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F(dasm32_context *dasmContext)
{
    uint8_t oldiop;
    uint8_t opcode;
    TYPE_TRACE_CALL_BEGIN("INS_0F");
    _adv;
    oldiop = iop;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&opcode), 1));
    iop = oldiop;
    TYPE_TRACE_CHECK_RETURN((*(dtable_0f[opcode]))(dasmContext));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "ADC");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "ADC");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "ADC");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "ADC");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "ADC");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ADC_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "ADC");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID PUSH_SS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_SS");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "SS");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_SS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_SS");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "SS");
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "SBB");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "SBB");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "SBB");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "SBB");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "SBB");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SBB_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "SBB");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID PUSH_DS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_DS");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "DS");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_DS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_DS");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "DS");
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "AND");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "AND");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "AND");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "AND");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "AND");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AND_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "AND");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_ES(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_ES");
    _adv;
    STD_SPRINTF(doverds, "ES");
    STD_SPRINTF(doverss, "ES");
    TYPE_TRACE_CALL_END;
}
static C_VOID DAA(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DAA");
    _adv;
    STD_SPRINTF(dop, "DAA");
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "SUB");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "SUB");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "SUB");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "SUB");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "SUB");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SUB_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "SUB");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_CS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_CS");
    _adv;
    STD_SPRINTF(doverds, "CS");
    STD_SPRINTF(doverss, "CS");
    TYPE_TRACE_CALL_END;
}
static C_VOID DAS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DAS");
    _adv;
    STD_SPRINTF(dop, "DAS");
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "XOR");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "XOR");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "XOR");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "XOR");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "XOR");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XOR_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "XOR");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_SS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_SS");
    _adv;
    STD_SPRINTF(doverds, "SS");
    STD_SPRINTF(doverss, "SS");
    TYPE_TRACE_CALL_END;
}
static C_VOID AAA(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AAA");
    _adv;
    STD_SPRINTF(dop, "AAA");
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "CMP");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "CMP");
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dop, "CMP");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dop, "CMP");
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_AL_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dop, "CMP");
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMP_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "CMP");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_DS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_DS");
    _adv;
    STD_SPRINTF(doverds, "DS");
    STD_SPRINTF(doverss, "DS");
    TYPE_TRACE_CALL_END;
}
static C_VOID AAS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AAS");
    _adv;
    STD_SPRINTF(dop, "AAS");
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_EAX");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ECX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_ECX");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "CX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ECX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_EDX");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_EBX");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_ESP");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SP");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_EBP");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BP");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_ESI");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SI");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INC_EDI");
    _adv;
    STD_SPRINTF(dop, "INC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DI");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EAX");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ECX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ECX");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "CX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ECX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EDX");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EBX");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ESP");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SP");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EBP");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BP");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ESI");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SI");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EDI");
    _adv;
    STD_SPRINTF(dop, "DEC");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DI");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EAX");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ECX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ECX");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "CX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ECX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EDX");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EBX");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ESP");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SP");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EBP");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BP");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ESI");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SI");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EDI");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DI");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_EAX");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ECX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_ECX");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "CX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ECX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_EDX");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_EBX");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_ESP");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SP");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_EBP");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BP");
        break;
    case 4:
        STD_SPRINTF(dopr, "EBP");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_ESI");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SI");
        break;
    case 4:
        STD_SPRINTF(dopr, "ESI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_EDI");
    _adv;
    STD_SPRINTF(dop, "POP");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DI");
        break;
    case 4:
        STD_SPRINTF(dopr, "EDI");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID PUSHA(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSHA");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "PUSHA");
        break;
    case 4:
        STD_SPRINTF(dop, "PUSHAD");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POPA(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POPA");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "POPA");
        break;
    case 4:
        STD_SPRINTF(dop, "POPAD");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID BOUND_R16_M16_16(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    _adv;
    STD_SPRINTF(dop, "BOUND");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize * 2));
    if (!flagmem)
    {
        STD_SPRINTF(dopr, "<ERROR>");
    }
    else
    {
        STD_SPRINTF(dopr, "%s,%s", dr, drm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ARPL_RM16_R16(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    _adv;
    STD_SPRINTF(dop, "ARPL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 2, 2));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_FS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_FS");
    _adv;
    STD_SPRINTF(doverds, "FS");
    STD_SPRINTF(doverss, "FS");
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_GS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_GS");
    _adv;
    STD_SPRINTF(doverds, "GS");
    STD_SPRINTF(doverss, "GS");
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_OprSize(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_OprSize");
    _adv;
    prefix_oprsize = 0x01;
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_AddrSize(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    _adv;
    prefix_addrsize = 0x01;
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_I32");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    _adv;
    STD_SPRINTF(dop, "IMUL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s,%s,%04X", dr, drm, (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "%s,%s,%08X", dr, drm, (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_I8");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    _adv;
    STD_SPRINTF(dop, "IMUL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%s,%s,%02X", dr, drm, (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID INSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("INSB");
    _adv;
    STD_SPRINTF(dop, "INSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI],DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI],DX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("INSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "INSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "INSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI],DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI],DX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUTSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("OUTSB");
    _adv;
    STD_SPRINTF(dop, "OUTSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUTSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("OUTSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "OUTSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "OUTSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX,%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "DX,%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JO_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JO_REL8");
    _adv;
    STD_SPRINTF(dop, "JO");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNO_REL8");
    _adv;
    STD_SPRINTF(dop, "JNO");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JC_REL8");
    _adv;
    STD_SPRINTF(dop, "JC");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNC_REL8");
    _adv;
    STD_SPRINTF(dop, "JNC");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JZ_REL8");
    _adv;
    STD_SPRINTF(dop, "JZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNZ_REL8");
    _adv;
    STD_SPRINTF(dop, "JNZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNA_REL8");
    _adv;
    STD_SPRINTF(dop, "JNA");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JA_REL8");
    _adv;
    STD_SPRINTF(dop, "JA");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JS_REL8");
    _adv;
    STD_SPRINTF(dop, "JS");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNS_REL8");
    _adv;
    STD_SPRINTF(dop, "JNS");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JP_REL8");
    _adv;
    STD_SPRINTF(dop, "JP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNP_REL8");
    _adv;
    STD_SPRINTF(dop, "JNP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JL_REL8");
    _adv;
    STD_SPRINTF(dop, "JL");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNL_REL8");
    _adv;
    STD_SPRINTF(dop, "JNL");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNG_REL8");
    _adv;
    STD_SPRINTF(dop, "JNG");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JG_REL8");
    _adv;
    STD_SPRINTF(dop, "JG");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_80(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_80");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ADD_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        STD_SPRINTF(dop, "ADD");
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        STD_SPRINTF(dop, "OR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        STD_SPRINTF(dop, "ADC");
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        STD_SPRINTF(dop, "SBB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        STD_SPRINTF(dop, "AND");
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        STD_SPRINTF(dop, "SUB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        STD_SPRINTF(dop, "XOR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        STD_SPRINTF(dop, "CMP");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_81(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_81");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ADD_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
        STD_SPRINTF(dop, "ADD");
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I32");
        STD_SPRINTF(dop, "OR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
        STD_SPRINTF(dop, "ADC");
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
        STD_SPRINTF(dop, "SBB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I32");
        STD_SPRINTF(dop, "AND");
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
        STD_SPRINTF(dop, "SUB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
        STD_SPRINTF(dop, "XOR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
        STD_SPRINTF(dop, "CMP");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_83(dasm32_context *dasmContext)
{
    C_CHAR dsimm[0x100];
    TYPE_TRACE_CALL_BEGIN("INS_83");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ADD_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
        STD_SPRINTF(dop, "ADD");
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I8");
        STD_SPRINTF(dop, "OR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
        STD_SPRINTF(dop, "ADC");
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
        STD_SPRINTF(dop, "SBB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I8");
        STD_SPRINTF(dop, "AND");
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
        STD_SPRINTF(dop, "SUB");
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
        STD_SPRINTF(dop, "XOR");
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
        STD_SPRINTF(dop, "CMP");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    SPRINTFSI(dasmContext, dsimm, (uint8_t)(cimm), 1);
    STD_SPRINTF(dopr, "%s,%s", drm, dsimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _adv;
    STD_SPRINTF(dop, "TEST");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("TEST_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "TEST");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM8_R8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R8_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R8_RM8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 1, 1));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM16_SREG(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(dasmContext, 2));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID LEA_R32_M32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LEA_R32_M32");
    _adv;
    STD_SPRINTF(dop, "LEA");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_SREG_RM16(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(dasmContext, 2));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_8F(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_8F");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize));
    switch (cr)
    {
    case 0: /* POP_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("POP_RM32");
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dop, "POP");
            break;
        case 4:
            STD_SPRINTF(dop, "POPD");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID NOP(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("NOP");
    _adv;
    STD_SPRINTF(dop, "NOP");
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ECX_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "CX,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "ECX,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDX_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "DX,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "EDX,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBX_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "BX,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "EBX,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESP_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "SP,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "ESP,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBP_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "BP,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "EBP,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESI_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "SI,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "ESI,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDI_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    _adv;
    STD_SPRINTF(dop, "XCHG");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        STD_SPRINTF(dopr, "DI,AX");
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        STD_SPRINTF(dopr, "EDI,EAX");
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CBW(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CBW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "CBW");
        break;
    case 4:
        STD_SPRINTF(dop, "CWDE");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CWD(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CWD");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "CWD");
        break;
    case 4:
        STD_SPRINTF(dop, "CDQ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CALL_PTR16_32(dasm32_context *dasmContext)
{
    uint16_t newcs;
    uint32_t neweip;
    TYPE_TRACE_CALL_BEGIN("CALL_PTR16_32");
    _adv;
    STD_SPRINTF(dop, "CALL");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        neweip = (uint16_t)(cimm);
        newcs = (uint16_t)(cimm >> 16);
        STD_SPRINTF(dopr, "%04X:%04X", newcs, (uint16_t)(neweip));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 8));
        neweip = (uint32_t)(cimm);
        newcs = (uint16_t)(cimm >> 32);
        STD_SPRINTF(dopr, "%04X:%08X", newcs, (uint32_t)(neweip));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID WAIT(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("WAIT");
    _adv;
    STD_SPRINTF(dop, "WAIT");
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSHF(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSHF");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "PUSHF");
        break;
    case 4:
        STD_SPRINTF(dop, "PUSHFD");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POPF(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POPF");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "POPF");
        break;
    case 4:
        STD_SPRINTF(dop, "POPFD");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SAHF(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SAHF");
    _adv;
    STD_SPRINTF(dop, "SAHF");
    TYPE_TRACE_CALL_END;
}
static C_VOID LAHF(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LAHF");
    _adv;
    STD_SPRINTF(dop, "LAHF");
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_MOFFS8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_moffs(dasmContext, 1));
    STD_SPRINTF(dopr, "AL,%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_MOFFS32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_moffs(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX,%s", drm);
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX,%s", drm);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS8_AL(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_moffs(dasmContext, 1));
    STD_SPRINTF(dopr, "%s,AL", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS32_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_moffs(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s,AX", drm);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s,EAX", drm);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVS");
    _adv;
    STD_SPRINTF(dop, "MOVSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "MOVSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "MOVSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI],%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI],%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMPSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("CMPSB");
    _adv;
    STD_SPRINTF(dop, "CMPSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMPSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("CMPSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "CMPSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "CMPSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s:[SI],ES:[DI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s:[ESI],ES:[EDI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("TEST_AL_I8");
    _adv;
    STD_SPRINTF(dop, "TEST");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("TEST_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "TEST");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STOSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("STOSB");
    _adv;
    STD_SPRINTF(dop, "STOSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STOSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("STOSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "STOSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "STOSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LODSB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("LODSB");
    _adv;
    STD_SPRINTF(dop, "LODSB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LODSW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("LODSW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "LODSW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "LODSD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s:[SI]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s:[ESI]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SCASB(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("SCASB");
    _adv;
    STD_SPRINTF(dop, "SCASB");
    STD_SPRINTF(dptr, "BYTE PTR ");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SCASW(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("SCASW");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "SCASW");
        STD_SPRINTF(dptr, "WORD PTR ");
        break;
    case 4:
        STD_SPRINTF(dop, "SCASD");
        STD_SPRINTF(dptr, "DWORD PTR ");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "ES:[DI]");
        break;
    case 4:
        STD_SPRINTF(dopr, "ES:[EDI]");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AL_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CL_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "CL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DL_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "DL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_BL_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "BL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AH_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AH_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "AH,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CH_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CH_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "CH,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DH_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DH_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "DH,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BH_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_BH_I8");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "BH,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EAX_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ECX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ECX_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "CX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "ECX,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EDX_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EDX,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBX_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EBX_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BX,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EBX,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESP_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ESP_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SP,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "ESP,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBP_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EBP_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "BP,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EBP,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESI_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ESI_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "SI,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "ESI,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDI_I32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EDI_I32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DI,%04X", (uint16_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EDI,%08X", (uint32_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C0(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_C0");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C1(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_C1");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (cr)
    {
    case 0: /* ROL_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
        STD_SPRINTF(dop, "ROR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID RET_I16(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("RET_I16");
    _adv;
    STD_SPRINTF(dop, "RET");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    STD_SPRINTF(dopr, "%04X", (uint16_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID RET(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("RET");
    _adv;
    STD_SPRINTF(dop, "RET");
    TYPE_TRACE_CALL_END;
}
static C_VOID LES_R32_M16_32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LES_R32_M16_32");
    _adv;
    STD_SPRINTF(dop, "LES");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID LDS_R32_M16_32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    _adv;
    STD_SPRINTF(dop, "LDS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C6(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_C6");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* MOV_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        STD_SPRINTF(dop, "MOV");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C7(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_C7");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* MOV_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
        STD_SPRINTF(dop, "MOV");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
            break;
        case 4:
            STD_SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ENTER(dasm32_context *dasmContext)
{
    C_CHAR dframesize[0x100], dnestlevel[0x100];
    TYPE_TRACE_CALL_BEGIN("ENTER");
    _adv;
    STD_SPRINTF(dop, "ENTER");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    STD_SPRINTF(dframesize, "%04X", (uint16_t)(cimm));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dnestlevel, "%02X", (uint8_t)(cimm));
    STD_SPRINTF(dopr, "%s,%s", dframesize, dnestlevel);
    TYPE_TRACE_CALL_END;
}
static C_VOID LEAVE(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LEAVE");
    _adv;
    STD_SPRINTF(dop, "LEAVE");
    TYPE_TRACE_CALL_END;
}
static C_VOID RETF_I16(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("RETF_I16");
    _adv;
    STD_SPRINTF(dop, "RETF");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
    STD_SPRINTF(dopr, "%04X", (uint16_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID RETF(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("RETF");
    _adv;
    STD_SPRINTF(dop, "RETF");
    TYPE_TRACE_CALL_END;
}
static C_VOID INT3(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INT3");
    _adv;
    STD_SPRINTF(dop, "INT3");
    TYPE_TRACE_CALL_END;
}
static C_VOID INT_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INT_I8");
    _adv;
    STD_SPRINTF(dop, "INT");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID INTO(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INTO");
    _adv;
    STD_SPRINTF(dop, "INTO");
    TYPE_TRACE_CALL_END;
}
static C_VOID IRET(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IRET");
    _adv;
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dop, "IRET");
        break;
    case 4:
        STD_SPRINTF(dop, "IRETD");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D0(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_D0");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM8");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM8");
        STD_SPRINTF(dop, "ROR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM8");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM8");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM8");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM8");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM8");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D1(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_D1");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ROL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM32");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM32");
        STD_SPRINTF(dop, "ROR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM32");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM32");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM32");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM32");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM32");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,01", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D2(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_D2");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* ROL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        STD_SPRINTF(dop, "ROR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D3(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_D3");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* ROL_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
        STD_SPRINTF(dop, "ROL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
        STD_SPRINTF(dop, "ROR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
        STD_SPRINTF(dop, "RCL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
        STD_SPRINTF(dop, "RCR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
        STD_SPRINTF(dop, "SHL");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
        STD_SPRINTF(dop, "SHR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM32_CL */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
        STD_SPRINTF(dop, "SAR");
        STD_SPRINTF(dopr, "%s,CL", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AAM(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AAM");
    _adv;
    STD_SPRINTF(dop, "AAM");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    if ((uint8_t)(cimm) != 0x0a)
        STD_SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID AAD(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("AAD");
    _adv;
    STD_SPRINTF(dop, "AAD");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    if ((uint8_t)(cimm) != 0x0a)
        STD_SPRINTF(dopr, "%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID XLAT(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("XLAT");
    _adv;
    STD_SPRINTF(dop, "XLATB");
    switch (_GetAddressSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%s:[BX+AL]", doverds);
        break;
    case 4:
        STD_SPRINTF(dopr, "%s:[EBX+AL]", doverds);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOPNZ_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    _adv;
    STD_SPRINTF(dop, "LOOPNZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOPZ_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LOOPZ_REL8");
    _adv;
    STD_SPRINTF(dop, "LOOPZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOP_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LOOP_REL8");
    _adv;
    STD_SPRINTF(dop, "LOOP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID JCXZ_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JCXZ_REL8");
    _adv;
    STD_SPRINTF(dop, "JCXZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    _adv;
    STD_SPRINTF(dop, "IN");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "AL,%02X", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IN_EAX_I8");
    _adv;
    STD_SPRINTF(dop, "IN");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX,%02X", (uint8_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX,%02X", (uint8_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_I8_AL(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OUT_I8_AL");
    _adv;
    STD_SPRINTF(dop, "OUT");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%02X,AL", (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_I8_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _adv;
    STD_SPRINTF(dop, "OUT");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "%02X,AX", (uint8_t)(cimm));
        break;
    case 4:
        STD_SPRINTF(dopr, "%02X,EAX", (uint8_t)(cimm));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CALL_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CALL_REL32");
    _adv;
    STD_SPRINTF(dop, "CALL");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JMP_REL32");
    _adv;
    STD_SPRINTF(dop, "JMP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_PTR16_32(dasm32_context *dasmContext)
{
    uint16_t newcs;
    uint32_t neweip;
    TYPE_TRACE_CALL_BEGIN("JMP_PTR16_32");
    _adv;
    STD_SPRINTF(dop, "JMP");
    switch (_GetOperandSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        neweip = (uint16_t)(cimm);
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        newcs = (uint16_t)(cimm);
        STD_SPRINTF(dopr, "%04X:%04X", newcs, (uint16_t)(neweip));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
        _newins_;
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 4));
        neweip = (uint32_t)(cimm);
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 2));
        newcs = (uint16_t)(cimm);
        STD_SPRINTF(dopr, "%04X:%08X", newcs, (uint32_t)(neweip));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JMP_REL8");
    _adv;
    STD_SPRINTF(dop, "JMP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    SPRINTFSI(dasmContext, dopr, (uint8_t)(cimm), 1);
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_DX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IN_AL_DX");
    _adv;
    STD_SPRINTF(dop, "IN");
    STD_SPRINTF(dopr, "AL,DX");
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_DX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IN_EAX_DX");
    _adv;
    STD_SPRINTF(dop, "IN");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "AX,DX");
        break;
    case 4:
        STD_SPRINTF(dopr, "EAX,DX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_AL(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OUT_DX_AL");
    _adv;
    STD_SPRINTF(dop, "OUT");
    STD_SPRINTF(dopr, "DX,AL");
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_EAX(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _adv;
    STD_SPRINTF(dop, "OUT");
    switch (_GetOperandSize)
    {
    case 2:
        STD_SPRINTF(dopr, "DX,AX");
        break;
    case 4:
        STD_SPRINTF(dopr, "DX,EAX");
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_LOCK(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    _adv;
    STD_SPRINTF(dop, "LOCK:");
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPNZ(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    _adv;
    STD_SPRINTF(dop, "REPNZ:");
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPZ(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_REPZ");
    _adv;
    STD_SPRINTF(dop, "REPZ:");
    TYPE_TRACE_CALL_END;
}
static C_VOID HLT(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("HLT");
    _adv;
    STD_SPRINTF(dop, "HLT");
    TYPE_TRACE_CALL_END;
}
static C_VOID CMC(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CMC");
    _adv;
    STD_SPRINTF(dop, "CMC");
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_F6(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_F6");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* TEST_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        STD_SPRINTF(dop, "TEST");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("NOT_RM8");
        STD_SPRINTF(dop, "NOT");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("NEG_RM8");
        STD_SPRINTF(dop, "NEG");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("MUL_RM8");
        STD_SPRINTF(dop, "MUL");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("IMUL_RM8");
        STD_SPRINTF(dop, "IMUL");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("DIV_RM8");
        STD_SPRINTF(dop, "DIV");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("IDIV_RM8");
        STD_SPRINTF(dop, "IDIV");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_F7(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_F7");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0: /* TEST_RM32_I32 */
        TYPE_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
        STD_SPRINTF(dop, "TEST");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "%s,%04X", drm, (uint16_t)(cimm));
            break;
        case 4:
            STD_SPRINTF(dopr, "%s,%08X", drm, (uint32_t)(cimm));
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("NOT_RM32");
        STD_SPRINTF(dop, "NOT");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("NEG_RM16");
        STD_SPRINTF(dop, "NEG");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("MUL_RM32");
        STD_SPRINTF(dop, "MUL");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("IMUL_RM32");
        STD_SPRINTF(dop, "IMUL");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("DIV_RM32");
        STD_SPRINTF(dop, "DIV");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("IDIV_RM32");
        STD_SPRINTF(dop, "IDIV");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CLC(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CLC");
    _adv;
    STD_SPRINTF(dop, "CLC");
    TYPE_TRACE_CALL_END;
}
static C_VOID STC(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("STC");
    _adv;
    STD_SPRINTF(dop, "STC");
    TYPE_TRACE_CALL_END;
}
static C_VOID CLI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CLI");
    _adv;
    STD_SPRINTF(dop, "CLI");
    TYPE_TRACE_CALL_END;
}
static C_VOID STI(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("STI");
    _adv;
    STD_SPRINTF(dop, "STI");
    TYPE_TRACE_CALL_END;
}
static C_VOID CLD(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CLD");
    _adv;
    STD_SPRINTF(dop, "CLD");
    TYPE_TRACE_CALL_END;
}
static C_VOID STD(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("STD");
    _adv;
    STD_SPRINTF(dop, "STD");
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_FE(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("INS_FE");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    switch (cr)
    {
    case 0: /* INC_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("INC_RM8");
        STD_SPRINTF(dop, "INC");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("DEC_RM8");
        STD_SPRINTF(dop, "DEC");
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_FF(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    uint8_t oldiop;
    uint8_t modrm;
    TYPE_TRACE_CALL_BEGIN("INS_FF");
    _adv;
    oldiop = iop;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* INC_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("INC_RM32");
        STD_SPRINTF(dop, "INC");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("DEC_RM32");
        STD_SPRINTF(dop, "DEC");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* CALL_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("CALL_RM32");
        STD_SPRINTF(dop, "CALL");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* CALL_M16_32 */
        TYPE_TRACE_BLOCK_BEGIN("CALL_M16_32");
        STD_SPRINTF(dop, "CALL");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize + 2));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dptr, "WORD PTR ");
            break;
        case 4:
            STD_SPRINTF(dptr, "DWORD PTR ");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        STD_SPRINTF(dopr, "FAR %s%s", dptr, drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* JMP_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("JMP_RM32");
        STD_SPRINTF(dop, "JMP");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* JMP_M16_32 */
        TYPE_TRACE_BLOCK_BEGIN("JMP_M16_32");
        STD_SPRINTF(dop, "JMP");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 9, _GetOperandSize + 2));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dptr, "WORD PTR ");
            break;
        case 4:
            STD_SPRINTF(dptr, "DWORD PTR ");
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        STD_SPRINTF(dopr, "FAR %s%s", dptr, drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* PUSH_RM32 */
        TYPE_TRACE_BLOCK_BEGIN("PUSH_RM32");
        STD_SPRINTF(dop, "PUSH");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _d_modrm_creg(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_creg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 0:
        STD_SPRINTF(dr, "CR0");
        break;
    case 2:
        STD_SPRINTF(dr, "CR2");
        break;
    case 3:
        STD_SPRINTF(dr, "CR3");
        break;
        ;
    default:
        STD_SPRINTF(dr, "<ERROR>");
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_dreg(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_dreg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 0:
        STD_SPRINTF(dr, "DR0");
        break;
    case 1:
        STD_SPRINTF(dr, "DR1");
        break;
    case 2:
        STD_SPRINTF(dr, "DR2");
        break;
    case 3:
        STD_SPRINTF(dr, "DR3");
        break;
    case 6:
        STD_SPRINTF(dr, "DR6");
        break;
    case 7:
        STD_SPRINTF(dr, "DR7");
        break;
    default:
        STD_SPRINTF(dr, "<ERROR>");
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_treg(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_treg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(dasmContext, 9, 4));
    if (flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(1)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    switch (cr)
    {
    case 6:
        STD_SPRINTF(dr, "TR6");
        break;
    case 7:
        STD_SPRINTF(dr, "TR7");
        break;
    default:
        STD_SPRINTF(dr, "<ERROR>");
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID INS_0F_00(dasm32_context *dasmContext)
{
    uint8_t modrm, oldiop;
    TYPE_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    oldiop = iop;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SLDT_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("SLDT_RM16");
        STD_SPRINTF(dop, "SLDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* STR_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("STR_RM16");
        STD_SPRINTF(dop, "STR");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LLDT_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("LLDT_RM16");
        STD_SPRINTF(dop, "LLDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LTR_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("LTR_RM16");
        STD_SPRINTF(dop, "LTR");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* VERR_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("VERR_RM16");
        STD_SPRINTF(dop, "VERR");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* VERW_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("VERW_RM16");
        STD_SPRINTF(dop, "VERW");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_01(dasm32_context *dasmContext)
{
    uint8_t modrm, oldiop;
    TYPE_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    oldiop = iop;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&modrm), 1));
    iop = oldiop;
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SGDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        STD_SPRINTF(dop, "SGDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            STD_SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        STD_SPRINTF(dop, "SIDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            STD_SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        STD_SPRINTF(dop, "LGDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            STD_SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        STD_SPRINTF(dop, "LIDT");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 6));
        if (!flagmem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
            STD_SPRINTF(drm, "<ERROR>");
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            STD_SPRINTF(dopr, "WORD PTR %s", drm);
            break;
        case 4:
            STD_SPRINTF(dopr, "DWORD PTR %s", drm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("SMSW_RM16");
        STD_SPRINTF(dop, "SMSW");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("LMSW_RM16");
        STD_SPRINTF(dop, "LMSW");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 2));
        STD_SPRINTF(dopr, "%s", drm);
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LAR_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "LAR");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID LSL_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "LSL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID CLTS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("CLTS");
    _adv;
    STD_SPRINTF(dop, "CLTS");
    TYPE_TRACE_CALL_END;
}
static C_VOID WBINVD(dasm32_context *dasmContext) {}
static C_VOID MOV_R32_CR(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_DR(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_DR");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CR_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DR_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DR_R32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_TR(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_TR");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_TR_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("MOV_TR_R32");
    _adv;
    STD_SPRINTF(dop, "MOV");
    TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(dasmContext));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID WRMSR(dasm32_context *dasmContext) {}
static C_VOID RDMSR(dasm32_context *dasmContext) {}
static C_VOID JO_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JO_REL32");
    _adv;
    STD_SPRINTF(dop, "JO");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNO_REL32");
    _adv;
    STD_SPRINTF(dop, "JNO");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    STD_SPRINTF(dop, "JC");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNC_REL32");
    _adv;
    STD_SPRINTF(dop, "JNC");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    STD_SPRINTF(dop, "JZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    STD_SPRINTF(dop, "JNZ");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNA_REL32");
    _adv;
    STD_SPRINTF(dop, "JNA");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JA_REL32");
    _adv;
    STD_SPRINTF(dop, "JA");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JS_REL32");
    _adv;
    STD_SPRINTF(dop, "JS");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNS_REL32");
    _adv;
    STD_SPRINTF(dop, "JNS");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JP_REL32");
    _adv;
    STD_SPRINTF(dop, "JP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNP_REL32");
    _adv;
    STD_SPRINTF(dop, "JNP");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JL_REL32");
    _adv;
    STD_SPRINTF(dop, "JL");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNL_REL32");
    _adv;
    STD_SPRINTF(dop, "JNL");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JNG_REL32");
    _adv;
    STD_SPRINTF(dop, "JNG");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("JG_REL32");
    _adv;
    STD_SPRINTF(dop, "JG");
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        SPRINTFSI(dasmContext, dopr, (uint16_t)(cimm), 2);
        break;
    case 4:
        SPRINTFSI(dasmContext, dopr, (uint32_t)(cimm), 4);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SETO_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    STD_SPRINTF(dop, "SETO");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNO_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNO");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETC_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETC_RM8");
    _adv;
    STD_SPRINTF(dop, "SETC");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNC_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNC_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNC");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETZ_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETZ_RM8");
    _adv;
    STD_SPRINTF(dop, "SETZ");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNZ_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNZ_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNZ");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNA_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNA_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNA");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETA_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETA_RM8");
    _adv;
    STD_SPRINTF(dop, "SETA");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETS_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETS_RM8");
    _adv;
    STD_SPRINTF(dop, "SETS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNS_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNS_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETP_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETP_RM8");
    _adv;
    STD_SPRINTF(dop, "SETP");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNP_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNP_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNP");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETL_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETL_RM8");
    _adv;
    STD_SPRINTF(dop, "SETL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNL_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNL_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNG_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETNG_RM8");
    _adv;
    STD_SPRINTF(dop, "SETNG");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID SETG_RM8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SETG_RM8");
    _adv;
    STD_SPRINTF(dop, "SETG");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, 1));
    STD_SPRINTF(dopr, "%s", drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_FS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "FS");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_FS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "FS");
    TYPE_TRACE_CALL_END;
}
static C_VOID CPUID(dasm32_context *dasmContext) {}
static C_VOID BT_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "BT");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    STD_SPRINTF(dop, "SHLD");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%s,%s,%02X", drm, dr, (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_CL(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    STD_SPRINTF(dop, "SHLD");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s,CL", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_GS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    STD_SPRINTF(dop, "PUSH");
    STD_SPRINTF(dopr, "GS");
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_GS(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    STD_SPRINTF(dop, "POP");
    STD_SPRINTF(dopr, "GS");
    TYPE_TRACE_CALL_END;
}
static C_VOID RSM(dasm32_context *dasmContext) {}
static C_VOID BTS_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "BTS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_I8(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    STD_SPRINTF(dop, "SHRD");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
    STD_SPRINTF(dopr, "%s,%s,%02X", drm, dr, (uint8_t)(cimm));
    TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_CL(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    STD_SPRINTF(dop, "SHRD");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s,CL", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "IMUL");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID LSS_R32_M16_32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    STD_SPRINTF(dop, "LSS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID BTR_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "BTR");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID LFS_R32_M16_32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    STD_SPRINTF(dop, "LFS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID LGS_R32_M16_32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _adv;
    STD_SPRINTF(dop, "LGS");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize + 2));
    if (!flagmem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagmem(0)");
        STD_SPRINTF(drm, "<ERROR>");
        TYPE_TRACE_BLOCK_END;
    }
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM8(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    STD_SPRINTF(dop, "MOVZX");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, 1));
    if (flagmem)
        STD_SPRINTF(dptr, "BYTE PTR ");
    else
        dptr[0] = 0;
    STD_SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM16(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    STD_SPRINTF(dop, "MOVZX");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 4, 2));
    if (flagmem)
        STD_SPRINTF(dptr, "WORD PTR ");
    else
        dptr[0] = 0;
    STD_SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_BA(dasm32_context *dasmContext)
{
    uint8_t modrm, oldiop;
    TYPE_TRACE_CALL_BEGIN("INS_0F_BA");
    _adv;
    oldiop = iop;
    TYPE_TRACE_CHECK_RETURN(_d_code(dasmContext, (uint8_t *)(&modrm), 1));
    iop = oldiop;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 0, _GetOperandSize));
    switch (cr)
    {
    case 0:
        TYPE_TRACE_BLOCK_BEGIN("cr(0)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(dasmContext));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        STD_SPRINTF(dop, "BT");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        STD_SPRINTF(dop, "BTS");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        STD_SPRINTF(dop, "BTR");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        STD_SPRINTF(dop, "BTC");
        TYPE_TRACE_CHECK_RETURN(_d_imm(dasmContext, 1));
        STD_SPRINTF(dopr, "%s,%02X", drm, (uint8_t)(cimm));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID BTC_RM32_R32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _adv;
    STD_SPRINTF(dop, "BTC");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", drm, dr);
    TYPE_TRACE_CALL_END;
}
static C_VOID BSF_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "BSF");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID BSR_R32_RM32(dasm32_context *dasmContext)
{
    TYPE_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _adv;
    STD_SPRINTF(dop, "BSR");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, _GetOperandSize));
    STD_SPRINTF(dopr, "%s,%s", dr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM8(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    STD_SPRINTF(dop, "MOVSX");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, _GetOperandSize, 1));
    if (flagmem)
        STD_SPRINTF(dptr, "BYTE PTR ");
    else
        dptr[0] = 0;
    STD_SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM16(dasm32_context *dasmContext)
{
    C_CHAR dptr[0x100];
    TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    STD_SPRINTF(dop, "MOVSX");
    TYPE_TRACE_CHECK_RETURN(_d_modrm(dasmContext, 4, 2));
    if (flagmem)
        STD_SPRINTF(dptr, "WORD PTR ");
    else
        dptr[0] = 0;
    STD_SPRINTF(dopr, "%s,%s%s", dr, dptr, drm);
    TYPE_TRACE_CALL_END;
}
static uint8_t dasm32_execute(dasm32_context *dasmContext, C_CHAR *stmt, uint8_t *rcode, C_INT flag32)
{
    STD_SIZE_T i;
    uint8_t opcode, oldiop;
#if DASM_TRACE == 1
    type_trace_initialize(&trace);
#endif
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

    STD_SPRINTF(doverds, "DS");
    STD_SPRINTF(doverss, "SS");

    do
    {
        TYPE_TRACE_CALL_BEGIN("dasm32");
        dop[0] = 0;
        dopr[0] = 0;
        dstmt[0] = 0;
        oldiop = iop;
        TYPE_TRACE_CHECK_BREAK(_d_code(dasmContext, (uint8_t *)(&opcode), 1));
        iop = oldiop;
        TYPE_TRACE_CHECK_BREAK((*(dtable[opcode]))(dasmContext));
        if (STD_STRLEN(dop))
        {
            STD_STRCAT(dop, " ");
            STD_STRCPY(dstmt, dop);
            for (i = STD_STRLEN(dop); i < 8; ++i)
                STD_STRCAT(dstmt, " ");
            STD_STRCAT(dstmt, dopr);
            STD_STRCAT(stmt, dstmt);
        }
        TYPE_TRACE_CALL_END;
    } while (_kdf_check_prefix(dasmContext, opcode));
#if DASM_TRACE == 1
    if (trace.callCount || trace.flagError)
    {
        STD_PRINTF("dasm32: bad machine code.\n");
    }
    type_trace_finalize(&trace);
#endif
    return iop;
}

uint8_t dasm32(C_CHAR *stmt, uint8_t *rcode, C_INT flag32)
{
    dasm32_context local_context;

    STD_MEMSET(&local_context, 0, sizeof(local_context));
    return dasm32_execute(&local_context, stmt, rcode, flag32);
}
