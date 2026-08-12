/* DEBUG is the debug console for users to break, trace, lookup,
 * and print virtual machine devices. */

#include "type.h"

#include "core/utils/wait.h"

#include "core/product/utils.h"

#include "core/product/debug/debug_access.h"

#include "core/product/debug/debug.h"

#define DEBUG_MAXNARG 256
#define DEBUG_MAXNASMARG 4

static C_VOID core_product_debug_flush_console_input(core_product_debug_context *debugContext)
{
    if (debugContext->input_provider != STD_NULL &&
        debugContext->input_provider->flush_console_input != STD_NULL)
    {
        debugContext->input_provider->flush_console_input(
            debugContext->input_provider->context);
    }
}

#define nErrPos (debugContext->error_position)
#define narg (debugContext->argument_count)
#define arg (debugContext->arguments)
#define flagExit (debugContext->exit_requested)
#define strCmdBuff (debugContext->command_buffer)
#define strCmdCopy (debugContext->command_copy)
#define strFileName (debugContext->file_name)
#define debugTarget (debugContext->target)
#define core_product_debug_is_running() core_product_debug_is_running(debugTarget)
#define core_product_debug_resume() core_product_debug_resume(debugTarget)
#define core_product_debug_is_paused() core_product_debug_is_paused(debugTarget)
#define core_product_debug_get_pause_reason() core_product_debug_get_pause_reason(debugTarget)
#define core_product_debug_request_pause(reason) core_product_debug_request_pause(debugTarget, reason)
#define core_product_debug_continue() core_product_debug_continue(debugTarget)
#define core_product_debug_step() core_product_debug_step(debugTarget)
#define core_product_debug_read_register(reg, value) core_product_debug_read_register(debugTarget, reg, value)
#define core_product_debug_write_register(reg, value) core_product_debug_write_register(debugTarget, reg, value)
#define core_product_debug_get_code_default_size() core_product_debug_get_code_default_size(debugTarget)
#define core_product_debug_get_code_base() core_product_debug_get_code_base(debugTarget)
#define core_product_debug_read_linear(address, out, size) core_product_debug_read_linear(debugTarget, address, out, size)
#define core_product_debug_write_linear(address, in, size) core_product_debug_write_linear(debugTarget, address, in, size)
#define core_product_debug_read_real(segment, offset, out, size) core_product_debug_read_real(debugTarget, segment, offset, out, size)
#define core_product_debug_write_real(segment, offset, in, size) core_product_debug_write_real(debugTarget, segment, offset, in, size)
#define core_product_debug_read_port(port) core_product_debug_read_port(debugTarget, port)
#define core_product_debug_write_port(port, value) core_product_debug_write_port(debugTarget, port, value)
#define core_product_debug_set_break_real(segment, offset) core_product_debug_set_break_real(debugTarget, segment, offset)
#define core_product_debug_set_break_linear(address) core_product_debug_set_break_linear(debugTarget, address)
#define core_product_debug_clear_break(linear) core_product_debug_clear_break(debugTarget, linear)
#define core_product_debug_set_trace(count) core_product_debug_set_trace(debugTarget, count)
#define core_product_debug_clear_trace() core_product_debug_clear_trace(debugTarget)
#define core_product_debug_get_break_count() core_product_debug_get_break_count(debugTarget)
#define core_product_debug_set_watch(kind, address) core_product_debug_set_watch(debugTarget, kind, address)
#define core_product_debug_clear_watch(kind) core_product_debug_clear_watch(debugTarget, kind)
#define core_product_debug_print_registers() core_product_debug_print_registers(debugTarget)
#define core_product_debug_print_segment_registers() core_product_debug_print_segment_registers(debugTarget)
#define core_product_debug_print_control_registers() core_product_debug_print_control_registers(debugTarget)
#define core_product_debug_print_memory() core_product_debug_print_memory(debugTarget)
#define core_product_debug_print_watchpoints() core_product_debug_print_watchpoints(debugTarget)

static type_unsigned_32 debug_register(core_product_debug_context *debugContext, core_product_debug_register reg)
{
    type_unsigned_32 value = 0;
    core_product_debug_read_register(reg, &value);
    return value;
}
static C_INT debug_set_register(core_product_debug_context *debugContext, core_product_debug_register reg, type_unsigned_32 value)
{
    return core_product_debug_write_register(reg, value);
}
static C_INT debug_flag(core_product_debug_context *debugContext, type_unsigned_32 mask)
{
    return (debug_register(debugContext, CORE_PRODUCT_DEBUG_EFLAGS) & mask) != 0;
}
static C_VOID debug_set_flag(core_product_debug_context *debugContext, type_unsigned_32 mask, C_INT set)
{
    type_unsigned_32 flags = debug_register(debugContext, CORE_PRODUCT_DEBUG_EFLAGS);
    debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EFLAGS, set ? flags | mask : flags & ~mask);
}

#define _eax debug_register(debugContext, CORE_PRODUCT_DEBUG_EAX)
#define _ecx debug_register(debugContext, CORE_PRODUCT_DEBUG_ECX)
#define _edx debug_register(debugContext, CORE_PRODUCT_DEBUG_EDX)
#define _ebx debug_register(debugContext, CORE_PRODUCT_DEBUG_EBX)
#define _esp debug_register(debugContext, CORE_PRODUCT_DEBUG_ESP)
#define _ebp debug_register(debugContext, CORE_PRODUCT_DEBUG_EBP)
#define _esi debug_register(debugContext, CORE_PRODUCT_DEBUG_ESI)
#define _edi debug_register(debugContext, CORE_PRODUCT_DEBUG_EDI)
#define _eflags debug_register(debugContext, CORE_PRODUCT_DEBUG_EFLAGS)
#define _eip debug_register(debugContext, CORE_PRODUCT_DEBUG_EIP)
#define _cr(i) debug_register(debugContext, (core_product_debug_register)(CORE_PRODUCT_DEBUG_CR0 + (i)))
#define _ax ((type_unsigned_16)_eax)
#define _cx ((type_unsigned_16)_ecx)
#define _dx ((type_unsigned_16)_edx)
#define _bx ((type_unsigned_16)_ebx)
#define _sp ((type_unsigned_16)_esp)
#define _bp ((type_unsigned_16)_ebp)
#define _si ((type_unsigned_16)_esi)
#define _di ((type_unsigned_16)_edi)
#define _ip ((type_unsigned_16)_eip)
#define _es ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_ES))
#define _cs ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_CS))
#define _ss ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_SS))
#define _ds ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_DS))
#define _fs ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_FS))
#define _gs ((type_unsigned_16)debug_register(debugContext, CORE_PRODUCT_DEBUG_GS))

static C_VOID seterr(core_product_debug_context *debugContext, STD_SIZE_T pos)
{
    nErrPos = (STD_SIZE_T)(arg[pos] - strCmdCopy + STD_STRLEN(arg[pos]) + 1);
}
static type_unsigned_8 scannubit8(core_product_debug_context *debugContext, C_CHAR *s)
{
    type_unsigned_8 ans = 0;
    STD_SIZE_T i = 0;
    if (s[0] == '\'' && s[2] == '\'')
    {
        return s[1];
    }
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 1)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}
static type_unsigned_16 scannubit16(core_product_debug_context *debugContext, C_CHAR *s)
{
    type_unsigned_16 ans = 0;
    STD_SIZE_T i = 0;
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 3)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}
static type_unsigned_32 scannubit32(core_product_debug_context *debugContext, C_CHAR *s)
{
    type_unsigned_32 ans = 0;
    STD_SIZE_T i = 0;
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 7)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}

#define dumpSegRec (debugContext->dump_segment)
#define dumpPtrRec (debugContext->dump_offset)
#define asmSegRec (debugContext->assemble_segment)
#define asmPtrRec (debugContext->assemble_offset)
#define uasmSegRec (debugContext->unassemble_segment)
#define uasmPtrRec (debugContext->unassemble_offset)
#define seg (debugContext->parsed_segment)
#define ptr (debugContext->parsed_offset)

static C_VOID addrparse(core_product_debug_context *debugContext, type_unsigned_16 defseg, const C_CHAR *addr)
{
    C_CHAR *cseg, *cptr;
    C_CHAR ccopy[0x100];
    STD_STRCPY(ccopy, addr);
    cseg = STD_STRTOK(ccopy, ":");
    cptr = STD_STRTOK(STD_NULL, "");
    if (!cptr)
    {
        seg = defseg;
        ptr = scannubit16(debugContext, cseg);
    }
    else
    {
        if (!STD_STRCMP(cseg, "es"))
        {
            seg = _es;
        }
        else if (!STD_STRCMP(cseg, "cs"))
        {
            seg = _cs;
        }
        else if (!STD_STRCMP(cseg, "ss"))
        {
            seg = _ss;
        }
        else if (!STD_STRCMP(cseg, "ds"))
        {
            seg = _ds;
        }
        else
        {
            seg = scannubit16(debugContext, cseg);
        }
        ptr = scannubit16(debugContext, cptr);
    }
}

/* DEBUG CMD BEGIN */
/* assemble */
static C_VOID aconsole(core_product_debug_context *debugContext)
{
    STD_SIZE_T i, len, errAsmPos;
    C_CHAR cmdAsmBuff[0x100];
    type_unsigned_8 acode[15];
    C_INT flagExitAsm = 0;
    while (!flagExitAsm)
    {
        STD_PRINTF("%04X:%04X ", asmSegRec, asmPtrRec);
        core_product_debug_flush_console_input(debugContext);
        STD_FGETS(cmdAsmBuff, 0x100, STD_STDIN);
        type_string_lower(cmdAsmBuff);
        if (!STD_STRLEN(cmdAsmBuff))
        {
            flagExitAsm = 1;
            continue;
        }
        if (cmdAsmBuff[0] == ';')
        {
            continue;
        }
        errAsmPos = 0;
        len = core_product_utils_aasm32(cmdAsmBuff, acode, core_product_debug_get_code_default_size());
        if (!len)
        {
            errAsmPos = STD_STRLEN(cmdAsmBuff) + 9;
        }
        else
        {
            if (core_product_debug_write_linear((asmSegRec << 4) + asmPtrRec, (C_VOID *)acode, (type_unsigned_8)len))
            {
                STD_PRINTF("debug: fail to write to L%08X\n", (asmSegRec << 4) + asmPtrRec);
                return;
            }
            asmPtrRec += (type_unsigned_16)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                STD_PRINTF(" ");
            }
            STD_PRINTF("^ Error\n");
        }
    }
}
static C_VOID a(core_product_debug_context *debugContext)
{
    if (narg == 1)
    {
        aconsole(debugContext);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos)
        {
            return;
        }
        asmSegRec = seg;
        asmPtrRec = ptr;
        aconsole(debugContext);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* compare */
static C_VOID c(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val1, val2;
    type_unsigned_16 seg1, ptr1, seg2, ptr2, range;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(debugContext, _ds, arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(debugContext, arg[2]) - ptr1;
        if (!nErrPos)
        {
            for (i = 0; i <= range; ++i)
            {
                core_product_debug_read_real(seg1, (type_unsigned_16)(ptr1 + i), (C_VOID *)(&val1), 1);
                core_product_debug_read_real(seg2, (type_unsigned_16)(ptr2 + i), (C_VOID *)(&val2), 1);
                if (val1 != val2)
                {
                    STD_PRINTF("%04X:%04X  ", seg1, (type_unsigned_16)(ptr1 + i));
                    STD_PRINTF("%02X  %02X", val1, val2);
                    STD_PRINTF("  %04X:%04X\n", seg2, (type_unsigned_16)(ptr2 + i));
                }
            }
        }
    }
}
/* dump */
static C_VOID dprint(core_product_debug_context *debugContext, type_unsigned_16 segment, type_unsigned_16 start, type_unsigned_16 end)
{
    C_CHAR t, c[0x11];
    type_unsigned_16 iaddr;
    if (start > end)
        end = 0xffff;
    if ((type_unsigned_32)((segment << 4) + end) > 0x000fffff)
    {
        end = (type_unsigned_16)(0x000fffff - (segment << 4));
    }
    c[0x10] = '\0';
    if (end < start)
    {
        end = 0xffff;
    }
    for (iaddr = start - (start % 0x10); iaddr <= end + (0x10 - end % 0x10) - 1; ++iaddr)
    {
        if (iaddr % 0x10 == 0)
        {
            STD_PRINTF("%04X:%04X  ", segment, iaddr);
        }
        if (iaddr < start || iaddr > end)
        {
            STD_PRINTF("  ");
            c[iaddr % 0x10] = ' ';
        }
        else
        {
            core_product_debug_read_real(segment, iaddr, (C_VOID *)(&c[iaddr % 0x10]), 1);
            STD_PRINTF("%02X", c[iaddr % 0x10] & 0xff);
            t = c[iaddr % 0x10];
            if ((t >= 1 && t <= 7) || t == ' ' ||
                (t >= 11 && t <= 12) ||
                (t >= 14 && t <= 31) ||
                (t >= 33))
            {
            }
            else
            {
                c[iaddr % 0x10] = '.';
            }
        }
        STD_PRINTF(" ");
        if (iaddr % 0x10 == 7 && iaddr >= start && iaddr < end)
        {
            STD_PRINTF("\b-");
        }
        if ((iaddr + 1) % 0x10 == 0)
        {
            STD_PRINTF("  %s\n", c);
        }
        if (iaddr == 0xffff)
        {
            break;
        }
    }
    dumpSegRec = segment;
    dumpPtrRec = end + 1;
}
static C_VOID d(core_product_debug_context *debugContext)
{
    type_unsigned_16 ptr2;
    if (narg == 1)
    {
        dprint(debugContext, dumpSegRec, dumpPtrRec, dumpPtrRec + 0x7f);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        dprint(debugContext, seg, ptr, ptr + 0x7f);
    }
    else if (narg == 3)
    {
        addrparse(debugContext, _ds, arg[1]);
        ptr2 = scannubit16(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        if (ptr > ptr2)
        {
            seterr(debugContext, 2);
        }
        else
        {
            dprint(debugContext, seg, ptr, ptr2);
        }
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* enter */
static C_VOID e(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    C_CHAR s[0x100];
    if (narg == 1)
    {
        seterr(debugContext, 0);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        STD_PRINTF("%04X:%04X  ", seg, ptr);
        core_product_debug_read_real(seg, ptr, (C_VOID *)(&val), 1);
        STD_PRINTF("%02X", val);
        STD_PRINTF(".");
        STD_FGETS(s, 0x100, STD_STDIN);
        type_string_lower(s);              /* MARK */
        val = scannubit8(debugContext, s); /* MARK */
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            core_product_debug_write_real(seg, ptr, (C_VOID *)(&val), 1);
        }
    }
    else if (narg > 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        for (i = 2; i < narg; ++i)
        {
            val = scannubit8(debugContext, arg[i]); /* MARK */
            if (!nErrPos)
            {
                core_product_debug_write_real(seg, ptr, (C_VOID *)(&val), 1);
            }
            else
            {
                break;
            }
            ptr++;
        }
    }
}
/* fill */
static C_VOID f(core_product_debug_context *debugContext)
{
    type_unsigned_8 nbyte;
    type_unsigned_8 val;
    STD_SIZE_T i, j;
    type_unsigned_16 end;
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        end = scannubit16(debugContext, arg[2]);
        if (end < ptr)
        {
            seterr(debugContext, 2);
        }
        if (!nErrPos)
        {
            nbyte = (type_unsigned_8)narg - 3;
            for (i = ptr, j = 0; i <= end; ++i, ++j)
            {
                val = scannubit8(debugContext, arg[j % nbyte + 3]);
                if (!nErrPos)
                {
                    core_product_debug_write_real(seg, (type_unsigned_16)i, (C_VOID *)(&val), 1);
                }
                else
                {
                    return;
                }
            }
        }
    }
}
/* go */
static C_VOID rprintregs(core_product_debug_context *debugContext);
static C_VOID g(core_product_debug_context *debugContext)
{
    if (core_product_debug_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        core_product_debug_clear_break(0);
        break;
    case 2:
        addrparse(debugContext, _cs, arg[1]);
        core_product_debug_set_break_real(seg, ptr);
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CS, seg))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EIP, ptr);
        addrparse(debugContext, _cs, arg[2]);
        core_product_debug_set_break_real(seg, ptr);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    core_product_debug_resume();
    while (core_product_debug_is_running())
    {
        core_utils_wait_milliseconds(debugContext->wait_scope, 10);
    }
    core_product_debug_clear_break(0);
    rprintregs(debugContext);
}
/* hex */
static C_VOID h(core_product_debug_context *debugContext)
{
    type_unsigned_16 val1, val2;
    if (narg != 3)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        val1 = scannubit16(debugContext, arg[1]);
        val2 = scannubit16(debugContext, arg[2]);
        if (!nErrPos)
        {
            STD_PRINTF("%04X", (type_unsigned_16)(val1 + val2));
            STD_PRINTF("  ");
            STD_PRINTF("%04X", (type_unsigned_16)(val1 - val2));
            STD_PRINTF("\n");
        }
    }
}
/* input */
static C_VOID i(core_product_debug_context *debugContext)
{
    type_unsigned_16 in;
    if (narg != 2)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        in = scannubit16(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        STD_PRINTF("%08X\n", core_product_debug_read_port(in));
    }
}
/* load */
static C_VOID l(core_product_debug_context *debugContext)
{
    type_unsigned_8 c;
    type_unsigned_16 i = 0;
    type_unsigned_32 len = 0;
    STD_FILE *load = STD_FOPEN(strFileName, "rb");
    if (!load)
        STD_PRINTF("File not found\n");
    else
    {
        switch (narg)
        {
        case 1:
            seg = _cs;
            ptr = 0x100;
            break;
        case 2:
            addrparse(debugContext, _cs, arg[1]);
            break;
        default:
            seterr(debugContext, narg - 1);
            break;
        }
        if (!nErrPos)
        {
            c = STD_FGETC(load);
            while (!STD_FEOF(load))
            {
                core_product_debug_write_real(seg + i, ptr + len++, (C_VOID *)(&c), 1);
                i = len / 0x10000;
                c = STD_FGETC(load);
            }
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ECX, (type_unsigned_16)(len & 0xffff));
            if (len > 0xffff)
                debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBX, (type_unsigned_16)(len >> 16));
            else
                debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBX, 0x0000u);
        }
        STD_FCLOSE(load);
    }
}
/* move */
static C_VOID m(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    type_unsigned_16 seg1, ptr1, range, seg2, ptr2;
    if (narg != 4)
        seterr(debugContext, narg - 1);
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(debugContext, _ds, arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(debugContext, arg[2]) - ptr1;
        if (!nErrPos)
        {
            if (((seg1 << 4) + ptr1) < ((seg2 << 4) + ptr2))
            {
                for (i = 0; i <= range; ++i)
                {
                    core_product_debug_read_real(seg1, (type_unsigned_16)(ptr1 + range - i), (C_VOID *)(&val), 1);
                    core_product_debug_write_real(seg2, (type_unsigned_16)(ptr2 + range - i), (C_VOID *)(&val), 1);
                }
            }
            else if (((seg1 << 4) + ptr1) > ((seg2 << 4) + ptr2))
            {
                for (i = 0; i <= range; ++i)
                {
                    core_product_debug_read_real(seg1, (type_unsigned_16)(ptr1 + i), (C_VOID *)(&val), 1);
                    core_product_debug_write_real(seg2, (type_unsigned_16)(ptr2 + i), (C_VOID *)(&val), 1);
                }
            }
        }
    }
}
/* name */
static C_VOID n(core_product_debug_context *debugContext)
{
    if (narg != 2)
        seterr(debugContext, narg - 1);
    else
        STD_STRCPY(strFileName, arg[1]);
}
/* output */
static C_VOID o(core_product_debug_context *debugContext)
{
    type_unsigned_16 out;
    type_unsigned_32 value;
    if (narg != 3)
        seterr(debugContext, narg - 1);
    else
    {
        out = scannubit16(debugContext, arg[1]);
        if (nErrPos)
            return;
        value = scannubit32(debugContext, arg[2]);
        if (nErrPos)
            return;
        core_product_debug_write_port(out, value);
    }
}
/* quit */
static C_VOID q(core_product_debug_context *debugContext)
{
    flagExit = 1;
}
/* register */
static type_unsigned_8 uprintins(core_product_debug_context *debugContext, type_unsigned_16 segment, type_unsigned_16 off)
{
    STD_SIZE_T i;
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    type_unsigned_8 ucode[15];
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (core_product_debug_read_linear((segment << 4) + off, (C_VOID *)ucode, 15))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>", segment, off);
    }
    else
    {
        len = core_product_utils_dasm32(stmt, ucode, core_product_debug_get_code_default_size());
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0 || (STD_SIZE_T)format_result >= sbin_remaining) {
                binary_failed = TYPE_TRUE;
                len = 0;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>",
                    segment, off);
                break;
            }
        }
        if (!binary_failed) {
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X %s", segment, off,
                sbin);
        }
        for (i = STD_STRLEN(str); i < 24; ++i)
        {
            STD_STRCAT(str, " ");
        }
        STD_STRCAT(str, stmt);
    }
    STD_PRINTF("%s\n", str);
    return len;
}
static C_VOID rprintflags(core_product_debug_context *debugContext)
{
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0800u) ? "OV" : "NV");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0400u) ? "DN" : "UP");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0200u) ? "EI" : "DI");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0080u) ? "NG" : "PL");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0040u) ? "ZR" : "NZ");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0010u) ? "AC" : "NA");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0004u) ? "PE" : "PO");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0001u) ? "CY" : "NC");
}
static C_VOID rprintregs(core_product_debug_context *debugContext)
{
    STD_PRINTF("AX=%04X", _ax);
    STD_PRINTF("  BX=%04X", _bx);
    STD_PRINTF("  CX=%04X", _cx);
    STD_PRINTF("  DX=%04X", _dx);
    STD_PRINTF("  SP=%04X", _sp);
    STD_PRINTF("  BP=%04X", _bp);
    STD_PRINTF("  SI=%04X", _si);
    STD_PRINTF("  DI=%04X", _di);
    STD_PRINTF("\nDS=%04X", _ds);
    STD_PRINTF("  ES=%04X", _es);
    STD_PRINTF("  SS=%04X", _ss);
    STD_PRINTF("  CS=%04X", _cs);
    STD_PRINTF("  IP=%04X", _ip);
    STD_PRINTF("   ");
    rprintflags(debugContext);
    STD_PRINTF("\n");
    uprintins(debugContext, _cs, _ip);
    uasmSegRec = _cs;
    uasmPtrRec = _ip;
}
static C_VOID rscanregs(core_product_debug_context *debugContext)
{
    type_unsigned_16 value;
    C_CHAR s[0x100];
    if (!STD_STRCMP(arg[1], "ax"))
    {
        STD_PRINTF("AX ");
        STD_PRINTF("%04X", _ax);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EAX, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "bx"))
    {
        STD_PRINTF("BX ");
        STD_PRINTF("%04X", _bx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBX, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cx"))
    {
        STD_PRINTF("CX ");
        STD_PRINTF("%04X", _cx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ECX, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "dx"))
    {
        STD_PRINTF("DX ");
        STD_PRINTF("%04X", _dx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EDX, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "bp"))
    {
        STD_PRINTF("BP ");
        STD_PRINTF("%04X", _bp);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBP, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "sp"))
    {
        STD_PRINTF("SP ");
        STD_PRINTF("%04X", _sp);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ESP, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "si"))
    {
        STD_PRINTF("SI ");
        STD_PRINTF("%04X", _si);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ESI, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "di"))
    {
        STD_PRINTF("DI ");
        STD_PRINTF("%04X", _di);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EDI, (type_unsigned_16)value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ss"))
    {
        STD_PRINTF("SS ");
        STD_PRINTF("%04X", _ss);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_SS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ss from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cs"))
    {
        STD_PRINTF("CS ");
        STD_PRINTF("%04X", _cs);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load cs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ds"))
    {
        STD_PRINTF("DS ");
        STD_PRINTF("%04X", _ds);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_DS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ds from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "es"))
    {
        STD_PRINTF("ES ");
        STD_PRINTF("%04X", _es);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ES, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load es from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ip"))
    {
        STD_PRINTF("IP ");
        STD_PRINTF("%04X", _ip);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EIP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "f"))
    {
        rprintflags(debugContext);
        STD_PRINTF(" -");
        STD_FGETS(s, 0x100, STD_STDIN);
        type_string_lower(s);
        if (!STD_STRCMP(s, "ov"))
        {
            debug_set_flag(debugContext, 0x0800u, 1);
        }
        else if (!STD_STRCMP(s, "nv"))
        {
            debug_set_flag(debugContext, 0x0800u, 0);
        }
        else if (!STD_STRCMP(s, "dn"))
        {
            debug_set_flag(debugContext, 0x0400u, 1);
        }
        else if (!STD_STRCMP(s, "up"))
        {
            debug_set_flag(debugContext, 0x0400u, 0);
        }
        else if (!STD_STRCMP(s, "ei"))
        {
            debug_set_flag(debugContext, 0x0200u, 1);
        }
        else if (!STD_STRCMP(s, "di"))
        {
            debug_set_flag(debugContext, 0x0200u, 0);
        }
        else if (!STD_STRCMP(s, "ng"))
        {
            debug_set_flag(debugContext, 0x0080u, 1);
        }
        else if (!STD_STRCMP(s, "pl"))
        {
            debug_set_flag(debugContext, 0x0080u, 0);
        }
        else if (!STD_STRCMP(s, "zr"))
        {
            debug_set_flag(debugContext, 0x0040u, 1);
        }
        else if (!STD_STRCMP(s, "nz"))
        {
            debug_set_flag(debugContext, 0x0040u, 0);
        }
        else if (!STD_STRCMP(s, "ac"))
        {
            debug_set_flag(debugContext, 0x0010u, 1);
        }
        else if (!STD_STRCMP(s, "na"))
        {
            debug_set_flag(debugContext, 0x0010u, 0);
        }
        else if (!STD_STRCMP(s, "pe"))
        {
            debug_set_flag(debugContext, 0x0004u, 1);
        }
        else if (!STD_STRCMP(s, "po"))
        {
            debug_set_flag(debugContext, 0x0004u, 0);
        }
        else if (!STD_STRCMP(s, "cy"))
        {
            debug_set_flag(debugContext, 0x0001u, 1);
        }
        else if (!STD_STRCMP(s, "nc"))
        {
            debug_set_flag(debugContext, 0x0001u, 0);
        }
        else
        {
            STD_PRINTF("bf Error\n");
        }
    }
    else
    {
        STD_PRINTF("br Error\n");
    }
}
static C_VOID r(core_product_debug_context *debugContext)
{
    if (narg == 1)
    {
        rprintregs(debugContext);
    }
    else if (narg == 2)
    {
        rscanregs(debugContext);
    }
    else
        seterr(debugContext, 2);
}
/* search */
static C_VOID s(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    C_INT flagFound = 0;
    type_unsigned_16 p, pfront, start, end;
    type_unsigned_8 cstart, val;
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        start = ptr;
        end = scannubit16(debugContext, arg[2]);
        if (!nErrPos)
        {
            p = start;
            cstart = scannubit8(debugContext, arg[3]);
            while (p <= end)
            {
                core_product_debug_read_real(seg, p, (C_VOID *)(&val), 1);
                if (val == cstart)
                {
                    pfront = p;
                    flagFound = 1;
                    for (i = 3; i < narg; ++i)
                    {
                        core_product_debug_read_real(seg, p, (C_VOID *)(&val), 1);
                        if (val != scannubit8(debugContext, arg[i]))
                        {
                            flagFound = 0;
                            p = pfront + 1;
                            break;
                        }
                        else
                        {
                            ++p;
                        }
                    }
                    if (flagFound)
                    {
                        STD_PRINTF("%04X:%04X  ", seg, pfront);
                        STD_PRINTF("\n");
                    }
                }
                else
                    ++p;
            }
        }
    }
}
/* trace */
static C_VOID t(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_16 count;
    if (core_product_debug_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit16(debugContext, arg[1]);
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CS, seg))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EIP, ptr);
        count = scannubit16(debugContext, arg[2]);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    if (count < 0x100)
    {
        for (i = 0; i < count; ++i)
        {
            core_product_debug_set_trace(1);
            core_product_debug_resume();
            while (core_product_debug_is_running())
            {
                core_utils_wait_milliseconds(debugContext->wait_scope, 10);
            }
            rprintregs(debugContext);
            if (i != count - 1)
            {
                STD_PRINTF("\n");
            }
        }
    }
    else
    {
        core_product_debug_set_trace(count);
        core_product_debug_resume();
        while (core_product_debug_is_running())
        {
            core_utils_wait_milliseconds(debugContext->wait_scope, 10);
        }
        rprintregs(debugContext);
    }
    core_product_debug_clear_trace();
}
/* unassemble */
static C_VOID uprint(core_product_debug_context *debugContext, type_unsigned_16 segment, type_unsigned_16 start, type_unsigned_16 end)
{
    type_unsigned_8 len;
    type_unsigned_32 boundary;
    if (start > end)
    {
        end = 0xffff;
    }
    if ((type_unsigned_32)((segment << 4) + end) > 0xfffff)
    {
        end = (0xfffff - (segment << 4));
    }
    while (start <= end)
    {
        len = uprintins(debugContext, segment, start);
        start += len;
        boundary = (type_unsigned_32)start + (type_unsigned_32)len;
        if (boundary > 0xffff)
        {
            break;
        }
    }
    uasmSegRec = segment;
    uasmPtrRec = start;
    return;
}
static C_VOID u(core_product_debug_context *debugContext)
{
    type_unsigned_16 ptr2;
    if (narg == 1)
    {
        uprint(debugContext, uasmSegRec, uasmPtrRec, uasmPtrRec + 0x1f);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos)
        {
            return;
        }
        uprint(debugContext, seg, ptr, ptr + 0x1f);
    }
    else if (narg == 3)
    {
        addrparse(debugContext, _ds, arg[1]);
        ptr2 = scannubit16(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        if (ptr > ptr2)
        {
            seterr(debugContext, 2);
        }
        else
        {
            uprint(debugContext, seg, ptr, ptr2);
        }
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* verbal */
static C_VOID v(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    C_CHAR str[0x100];
    STD_PRINTF(":");
    STD_FGETS(str, 0x100, STD_STDIN);
    str[STD_STRLEN(str) - 1] = '\0';
    for (i = 0; i < STD_STRLEN(str); ++i)
    {
        STD_PRINTF("%02X", str[i]);
        if (!((i + 1) % 0x10))
        {
            STD_PRINTF("\n");
        }
        else if (!((i + 1) % 0x08) && (str[i + 1] != '\0'))
        {
            STD_PRINTF("-");
        }
        else
        {
            STD_PRINTF(" ");
        }
    }
    if (i % 0x10)
    {
        STD_PRINTF("\n");
    }
}
/* write */
static C_VOID w(core_product_debug_context *debugContext)
{
    STD_SIZE_T i = 0;
    type_unsigned_8 val;
    type_unsigned_32 len = (_bx << 16) + _cx;
    STD_FILE *write;
    if (!STD_STRLEN(strFileName))
    {
        STD_PRINTF("(W)rite error, no destination defined\n");
        return;
    }
    else
    {
        write = STD_FOPEN(strFileName, "wb");
    }
    if (!write)
    {
        STD_PRINTF("File not found\n");
    }
    else
    {
        STD_PRINTF("Writing ");
        STD_PRINTF("%04X", _bx);
        STD_PRINTF("%04X", _cx);
        STD_PRINTF(" bytes\n");
        switch (narg)
        {
        case 1:
            seg = _cs;
            ptr = 0x100;
            break;
        case 2:
            addrparse(debugContext, _cs, arg[1]);
            break;
        default:
            seterr(debugContext, narg - 1);
            break;
        }
        if (!nErrPos)
        {
            while (i < len)
            {
                core_product_debug_read_real(seg, (type_unsigned_8)(ptr + i++), (C_VOID *)(&val), 1);
                STD_FPUTC(val, write);
            }
        }
        STD_FCLOSE(write);
    }
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
type_unsigned_32 xalin;
type_unsigned_32 xdlin;
type_unsigned_32 xulin;
/* print */
static type_unsigned_8 xuprintins(core_product_debug_context *debugContext, type_unsigned_32 linear)
{
    STD_SIZE_T i;
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    type_unsigned_8 ucode[15];
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (core_product_debug_read_linear(linear, (C_VOID *)ucode, 15))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
    }
    else
    {
        len = core_product_utils_dasm32(stmt, ucode, core_product_debug_get_code_default_size());
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0 || (STD_SIZE_T)format_result >= sbin_remaining) {
                binary_failed = TYPE_TRUE;
                len = 0;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
                break;
            }
        }
        if (!binary_failed) {
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X %s ", linear, sbin);
        }
        for (i = STD_STRLEN(str); i < 24; ++i)
        {
            STD_STRCAT(str, " ");
        }
        STD_STRCAT(str, stmt);
    }
    STD_PRINTF("%s\n", str);
    return len;
}
static C_VOID xrprintreg(core_product_debug_context *debugContext)
{
    core_product_debug_print_registers();
    xulin = core_product_debug_get_code_base() + _eip;
    xuprintins(debugContext, xulin);
}
/* assemble */
static C_VOID xaconsole(core_product_debug_context *debugContext, type_unsigned_32 linear)
{
    STD_SIZE_T i, len, errAsmPos;
    C_CHAR astmt[0x100];
    type_unsigned_8 acode[15];
    C_INT flagExitAsm = 0;
    while (!flagExitAsm)
    {
        STD_PRINTF("L%08X ", linear);
        STD_FGETS(astmt, 0x100, STD_STDIN);
        core_product_debug_flush_console_input(debugContext);
        astmt[STD_STRLEN(astmt) - 1] = 0;
        if (!STD_STRLEN(astmt))
        {
            flagExitAsm = 1;
            continue;
        }
        errAsmPos = 0;
        len = core_product_utils_aasm32(astmt, acode, core_product_debug_get_code_default_size());
        if (!len)
        {
            errAsmPos = STD_STRLEN(astmt) + 9;
        }
        else
        {
            if (core_product_debug_write_linear(linear, (C_VOID *)acode, (type_unsigned_8)len))
            {
                STD_PRINTF("debug: fail to write to L%08X\n", linear);
                return;
            }
            linear += (type_unsigned_32)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                STD_PRINTF(" ");
            }
            STD_PRINTF("^ Error\n");
        }
    }
    xalin = linear;
}
static C_VOID xa(core_product_debug_context *debugContext)
{
    if (narg == 1)
    {
        xaconsole(debugContext, xalin);
    }
    else if (narg == 2)
    {
        xalin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xaconsole(debugContext, xalin);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* compare */
static C_VOID xc(core_product_debug_context *debugContext)
{
    STD_SIZE_T i, count;
    type_unsigned_32 lin1, lin2;
    type_unsigned_8 val1, val2;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        lin1 = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        lin2 = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[3]);
        if (nErrPos)
        {
            return;
        }
        if (!count)
        {
            return;
        }
        for (i = 0; i < count; ++i)
        {
            if (core_product_debug_read_linear((type_unsigned_32)(lin1 + i), (C_VOID *)(&val1), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", (type_unsigned_32)(lin1 + i));
                return;
            }
            if (core_product_debug_read_linear((type_unsigned_32)(lin2 + i), (C_VOID *)(&val2), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", (type_unsigned_32)(lin2 + i));
                return;
            }
            if (val1 != val2)
                STD_PRINTF("L%08X  %02X  %02X  L%08X\n",
                           (type_unsigned_32)(lin1 + i), val1, val2, (type_unsigned_32)(lin2 + i));
        }
    }
}
/* dump */
static C_VOID xdprint(core_product_debug_context *debugContext, type_unsigned_32 linear, type_unsigned_32 count)
{
    C_CHAR t, c[0x11];
    type_unsigned_32 ilinear;
    type_unsigned_32 start = linear;
    type_unsigned_32 end = linear + count - 1;
    c[0x10] = '\0';
    if (!count)
    {
        return;
    }
    if (end < start)
        end = 0xffffffff;
    for (ilinear = start - (start % 0x10); ilinear <= end + 0x0f - (end % 0x10); ++ilinear)
    {
        if (ilinear % 0x10 == 0)
            STD_PRINTF("L%08X  ", ilinear);
        if (ilinear < start || ilinear > end)
        {
            STD_PRINTF("  ");
            c[ilinear % 0x10] = ' ';
        }
        else
        {
            if (core_product_debug_read_linear(ilinear, (C_VOID *)(&c[ilinear % 0x10]), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X\n", ilinear);
                return;
            }
            else
            {
                STD_PRINTF("%02X", c[ilinear % 0x10] & 0xff);
                t = c[ilinear % 0x10];
                if ((t >= 1 && t <= 7) || t == ' ' ||
                    (t >= 11 && t <= 12) ||
                    (t >= 14 && t <= 31) ||
                    (t >= 33))
                    ;
                else
                    c[ilinear % 0x10] = '.';
            }
        }
        STD_PRINTF(" ");
        if (ilinear % 0x10 == 7 && ilinear >= start && ilinear < end)
            STD_PRINTF("\b-");
        if ((ilinear + 1) % 0x10 == 0)
        {
            STD_PRINTF("  %s\n", c);
        }
        if (ilinear == 0xffffffff)
        {
            break;
        }
    }
    xdlin = ilinear;
}
static C_VOID xd(core_product_debug_context *debugContext)
{
    type_unsigned_32 count;
    if (narg == 1)
    {
        xdprint(debugContext, xdlin, 0x80);
    }
    else if (narg == 2)
    {
        xdlin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xdprint(debugContext, xdlin, 0x80);
    }
    else if (narg == 3)
    {
        xdlin = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        xdprint(debugContext, xdlin, count);
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* enter */
static C_VOID xe(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    type_unsigned_32 linear;
    C_CHAR s[0x100];
    if (narg == 1)
    {
        seterr(debugContext, 0);
    }
    else if (narg == 2)
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        if (core_product_debug_read_linear(linear, (C_VOID *)(&val), 1))
        {
            STD_PRINTF("debug: fail to read from L%08X.\n", linear);
            return;
        }
        STD_PRINTF("L%08X  %02X.", linear, val);
        STD_FGETS(s, 0x100, STD_STDIN);
        type_string_lower(s);
        val = scannubit8(debugContext, s);
        if (nErrPos)
        {
            return;
        }
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (core_product_debug_write_linear(linear, (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", linear);
            }
        }
    }
    else if (narg > 2)
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        for (i = 2; i < narg; ++i)
        {
            val = scannubit8(debugContext, arg[i]);
            if (!nErrPos)
            {
                if (core_product_debug_write_linear(linear, (C_VOID *)(&val), 1))
                {
                    STD_PRINTF("debug: fail to write to L%08X.\n", linear);
                    return;
                }
            }
            else
            {
                break;
            }
            linear++;
        }
    }
}
/* fill */
static C_VOID xf(core_product_debug_context *debugContext)
{
    type_unsigned_8 val;
    STD_SIZE_T i, j, count, bcount;
    type_unsigned_32 linear;
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        bcount = narg - 3;
        for (i = 0, j = 0; i < count; ++i, ++j)
        {
            val = scannubit8(debugContext, arg[j % bcount + 3]);
            if (nErrPos)
            {
                return;
            }
            if (core_product_debug_write_linear((type_unsigned_32)(linear + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", (type_unsigned_32)(linear + i));
                return;
            }
        }
    }
}
/* go */
static C_VOID xg(core_product_debug_context *debugContext)
{
    STD_SIZE_T i, count = 0;
    type_unsigned_32 linear;
    if (core_product_debug_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        count = 0;
        g(debugContext);
        break;
    case 2:
        linear = scannubit32(debugContext, arg[1]);
        count = 1;
        break;
    case 3:
        linear = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    for (i = 0; i < count; ++i)
    {
        core_product_debug_set_break_linear(linear);
        core_product_debug_resume();
        while (core_product_debug_is_running())
        {
            core_utils_wait_milliseconds(debugContext->wait_scope, 10);
        }
        STD_PRINTF("%d instructions executed before the break point.\n",
                   core_product_debug_get_break_count());
        xrprintreg(debugContext);
    }
    core_product_debug_clear_break(1);
}
/* move */
static C_VOID xm(core_product_debug_context *debugContext)
{
    type_unsigned_8 val;
    STD_SIZE_T i;
    type_unsigned_32 lin1, lin2, count;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        lin1 = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        lin2 = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[3]);
        if (nErrPos)
        {
            return;
        }
        for (i = 0; i < count; ++i)
        {
            if (core_product_debug_read_linear((type_unsigned_32)(lin1 + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", lin1 + i);
                return;
            }
            if (core_product_debug_write_linear((type_unsigned_32)(lin2 + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", lin2 + i);
                return;
            }
        }
    }
}
/* search */
static C_VOID xs(core_product_debug_context *debugContext)
{
    STD_SIZE_T i, count, bcount;
    type_unsigned_32 linear;
    type_unsigned_8 val, mem[256], line[256];
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        addrparse(debugContext, _ds, arg[1]);
        bcount = narg - 3;
        for (i = 0; i < bcount; ++i)
        {
            val = scannubit8(debugContext, arg[i + 3]);
            if (nErrPos)
            {
                return;
            }
            line[i] = val;
        }
        for (i = 0; i < count; ++i)
        {
            if (core_product_debug_read_linear((type_unsigned_32)(linear + i), (C_VOID *)mem, (type_unsigned_8)bcount))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", linear + i);
                return;
            }
            if (!STD_MEMCMP((C_VOID *)mem, (C_VOID *)line, bcount))
            {
                STD_PRINTF("L%08X\n", linear + i);
            }
        }
    }
}
/* trace */
static C_VOID xt(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_32 count;
    if (core_product_debug_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit32(debugContext, arg[1]);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    if (count < 0x0100)
    {
        for (i = 0; i < count; ++i)
        {
            core_product_debug_set_trace(1);
            core_product_debug_resume();
            while (core_product_debug_is_running())
            {
                core_utils_wait_milliseconds(debugContext->wait_scope, 10);
            }
            core_product_debug_print_memory();
            xrprintreg(debugContext);
            if (i != count - 1)
            {
                STD_PRINTF("\n");
            }
        }
    }
    else
    {
        core_product_debug_set_trace(count);
        core_product_debug_resume();
        while (core_product_debug_is_running())
        {
            core_utils_wait_milliseconds(debugContext->wait_scope, 10);
        }
        core_product_debug_print_memory();
        xrprintreg(debugContext);
    }
    core_product_debug_clear_trace();
}
/* register */
static C_VOID xrscanreg(core_product_debug_context *debugContext)
{
    type_unsigned_32 value;
    C_CHAR s[0x100];
    if (!STD_STRCMP(arg[1], "eax"))
    {
        STD_PRINTF("EAX ");
        STD_PRINTF("%08X", _eax);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EAX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ecx"))
    {
        STD_PRINTF("ECX ");
        STD_PRINTF("%08X", _ecx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ECX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "edx"))
    {
        STD_PRINTF("EDX ");
        STD_PRINTF("%08X", _edx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EDX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ebx"))
    {
        STD_PRINTF("EBX ");
        STD_PRINTF("%08X", _ebx);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "esp"))
    {
        STD_PRINTF("ESP ");
        STD_PRINTF("%08X", _esp);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ESP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ebp"))
    {
        STD_PRINTF("EBP ");
        STD_PRINTF("%08X", _ebp);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EBP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "esi"))
    {
        STD_PRINTF("ESI ");
        STD_PRINTF("%08X", _esi);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ESI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "edi"))
    {
        STD_PRINTF("EDI ");
        STD_PRINTF("%08X", _edi);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EDI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "eip"))
    {
        STD_PRINTF("EIP ");
        STD_PRINTF("%08X", _eip);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EIP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "eflags"))
    {
        STD_PRINTF("EFLAGS ");
        STD_PRINTF("%08X", _eflags);
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_EFLAGS, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "es"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_ES, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load es from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cs"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load cs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ss"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_SS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ss from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ds"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_DS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ds from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "fs"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_FS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load fs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "gs"))
    {
        core_product_debug_print_segment_registers();
        STD_PRINTF(":");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, CORE_PRODUCT_DEBUG_GS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load gs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cr0"))
    {
        STD_PRINTF("CR0 ");
        STD_PRINTF("%08X", _cr(0));
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CR0, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr1"))
    {
        STD_PRINTF("CR1 ");
        STD_PRINTF("%08X", _cr(1));
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CR1, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr2"))
    {
        STD_PRINTF("CR2 ");
        STD_PRINTF("%08X", _cr(2));
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CR2, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr3"))
    {
        STD_PRINTF("CR3 ");
        STD_PRINTF("%08X", _cr(3));
        STD_PRINTF("\n:");
        STD_FGETS(s, 0x100, STD_STDIN);
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, CORE_PRODUCT_DEBUG_CR3, value);
        }
    }
    else
    {
        STD_PRINTF("br Error\n");
    }
}
static C_VOID xr(core_product_debug_context *debugContext)
{
    if (narg == 1)
    {
        xrprintreg(debugContext);
    }
    else if (narg == 2)
    {
        xrscanreg(debugContext);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* unassemble */
static C_VOID xuprint(core_product_debug_context *debugContext, type_unsigned_32 linear, type_unsigned_8 count)
{
    type_unsigned_32 len = 0;
    STD_SIZE_T i;
    for (i = 0; i < count; ++i)
    {
        len = xuprintins(debugContext, linear);
        if (!len)
        {
            break;
        }
        linear += len;
    }
    xulin = linear;
}
static C_VOID xu(core_product_debug_context *debugContext)
{
    type_unsigned_32 count;
    if (narg == 1)
    {
        xuprint(debugContext, xulin, 10);
    }
    else if (narg == 2)
    {
        xulin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xuprint(debugContext, xulin, 0x10);
    }
    else if (narg == 3)
    {
        xulin = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        xuprint(debugContext, xulin, count);
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* watch */
static C_VOID xw(core_product_debug_context *debugContext)
{
    type_unsigned_32 linear;
    switch (narg)
    {
    case 1:
        core_product_debug_print_watchpoints();
        break;
    case 2:
        switch (arg[1][0])
        {
        case 'r':
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_READ);
            STD_PRINTF("Watch-read point removed.\n");
            break;
        case 'w':
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_WRITE);
            STD_PRINTF("Watch-write point removed.\n");
            break;
        case 'e':
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_EXECUTE);
            STD_PRINTF("Watch-exec point removed.\n");
            break;
        case 'u':
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_READ);
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_WRITE);
            core_product_debug_clear_watch(CORE_PRODUCT_DEBUG_WATCH_EXECUTE);
            STD_PRINTF("All watch points removed.\n");
            break;
        default:
            seterr(debugContext, 1);
            break;
        }
        break;
    case 3:
        switch (arg[1][0])
        {
        case 'r':
            linear = scannubit32(debugContext, arg[2]);
            core_product_debug_set_watch(CORE_PRODUCT_DEBUG_WATCH_READ, linear);
            break;
        case 'w':
            linear = scannubit32(debugContext, arg[2]);
            core_product_debug_set_watch(CORE_PRODUCT_DEBUG_WATCH_WRITE, linear);
            break;
        case 'e':
            linear = scannubit32(debugContext, arg[2]);
            core_product_debug_set_watch(CORE_PRODUCT_DEBUG_WATCH_EXECUTE, linear);
            break;
        default:
            seterr(debugContext, 2);
            break;
        }
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
}
static C_VOID xhelp(core_product_debug_context *debugContext)
{
    STD_PRINTF("assemble        XA [address]\n");
    STD_PRINTF("compare         XC addr1 addr2 count_byte\n");
    STD_PRINTF("dump            XD [address [count_byte]]\n");
    STD_PRINTF("enter           XE address [byte_list]\n");
    STD_PRINTF("fill            XF address count_byte byte_list\n");
    STD_PRINTF("go              XG [address [count_instr]]\n");
    STD_PRINTF("move            XM addr1 addr2 count_byte\n");
    STD_PRINTF("register        XR [register]\n");
    STD_PRINTF("  regular         XREG\n");
    STD_PRINTF("  segment         XSREG\n");
    STD_PRINTF("  control         XCREG\n");
    STD_PRINTF("search          XS address count_byte byte_list\n");
    STD_PRINTF("trace           XT [count_instr]\n");
    STD_PRINTF("unassemble      XU [address [count_instr]]\n");
    STD_PRINTF("watch           XW r/w/e address\n");
}
static C_VOID x(core_product_debug_context *debugContext)
{
    STD_SIZE_T i;
    arg[narg] = arg[0];
    for (i = 1; i < narg; ++i)
    {
        arg[i - 1] = arg[i];
    }
    arg[narg - 1] = arg[narg];
    arg[narg] = STD_NULL;
    narg--;
    if (!STD_STRCMP(arg[0], "\?"))
    {
        xhelp(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "a"))
    {
        xa(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "c"))
    {
        xc(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "d"))
    {
        xd(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "e"))
    {
        xe(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "f"))
    {
        xf(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "g"))
    {
        xg(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "m"))
    {
        xm(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "r"))
    {
        xr(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "s"))
    {
        xs(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "t"))
    {
        xt(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "u"))
    {
        xu(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "w"))
    {
        xw(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "reg"))
    {
        xrprintreg(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "sreg"))
    {
        core_product_debug_print_segment_registers();
    }
    else if (!STD_STRCMP(arg[0], "creg"))
    {
        core_product_debug_print_control_registers();
    }
    else
    {
        arg[0] = arg[narg];
        seterr(debugContext, 0);
    }
}
/* EXTENDED DEBUG CMD END */

/* main routines */
static C_VOID help(core_product_debug_context *debugContext)
{
    STD_PRINTF("assemble        A [address]\n");
    STD_PRINTF("compare         C range address\n");
    STD_PRINTF("dump            D [range]\n");
    STD_PRINTF("enter           E address [list]\n");
    STD_PRINTF("fill            F range list\n");
    STD_PRINTF("go              G [[address] breakpoint]\n");
    /* STD_PRINTF("go              G [=address] [addresses]\n"); */
    STD_PRINTF("hex             H value1 value2\n");
    STD_PRINTF("input           I port\n");
    STD_PRINTF("load            L [address]\n");
    /* STD_PRINTF("load            L [address] [drive] [firstsector] [number]\n"); */
    STD_PRINTF("move            M range address\n");
    STD_PRINTF("name            N pathname\n");
    /* STD_PRINTF("name            N [pathname] [arglist]\n"); */
    STD_PRINTF("output          O port byte\n");
    /* !STD_PRINTF("proceed           P [nx=address] [number]\n"); */
    STD_PRINTF("quit            Q \n");
    STD_PRINTF("register        R [register]\n");
    STD_PRINTF("search          S range list\n");
    STD_PRINTF("trace           T [[address] value]\n");
    /* STD_PRINTF("trace           T [=address] [value]\n"); */
    STD_PRINTF("unassemble      U [range]\n");
    STD_PRINTF("verbal          V \n");
    STD_PRINTF("write           W [address]\n");
    STD_PRINTF("debug32         X?\n");
    /* STD_PRINTF("write           W [address] [drive] [firstsector] [number]\n"); */
    /* STD_PRINTF("allocate expanded memory        XA [#pages]\n"); */
    /* STD_PRINTF("deallocate expanded memory      XD [handle]\n"); */
    /* STD_PRINTF("map expanded memory pages       XM [Lpage] [Ppage] [handle]\n"); */
    /* STD_PRINTF("display expanded memory status  XS\n"); */
}

static C_VOID parse(core_product_debug_context *debugContext)
{
    STD_STRCPY(strCmdCopy, strCmdBuff);
    narg = 0;
    arg[0] = STD_STRTOK(strCmdCopy, " ,\t\n\r\f");
    if (arg[narg])
    {
        type_string_lower(arg[narg]);
        narg++;
    }
    else
    {
        return;
    }
    if (STD_STRLEN(arg[narg - 1]) != 1)
    {
        arg[narg] = arg[narg - 1] + 1;
        narg++;
    }
    while (narg < DEBUG_MAXNARG)
    {
        arg[narg] = STD_STRTOK(STD_NULL, " ,\t\n\r\f");
        if (arg[narg])
        {
            type_string_lower(arg[narg]);
            narg++;
        }
        else
        {
            break;
        }
    }
}

static C_VOID exec(core_product_debug_context *debugContext)
{
    nErrPos = 0;
    if (!arg[0])
    {
        return;
    }
    switch (arg[0][0])
    {
    case '\?':
        help(debugContext);
        break;
    case 'a':
        a(debugContext);
        break;
    case 'c':
        c(debugContext);
        break;
    case 'd':
        d(debugContext);
        break;
    case 'e':
        e(debugContext);
        break;
    case 'f':
        f(debugContext);
        break;
    case 'g':
        g(debugContext);
        break;
    case 'h':
        h(debugContext);
        break;
    case 'i':
        i(debugContext);
        break;
    case 'l':
        l(debugContext);
        break;
    case 'm':
        m(debugContext);
        break;
    case 'n':
        n(debugContext);
        break;
    case 'o':
        o(debugContext);
        break;
    case 'q':
        q(debugContext);
        break;
    case 'r':
        r(debugContext);
        break;
    case 's':
        s(debugContext);
        break;
    case 't':
        t(debugContext);
        break;
    case 'u':
        u(debugContext);
        break;
    case 'v':
        v(debugContext);
        break;
    case 'w':
        w(debugContext);
        break;
    case 'x':
        x(debugContext);
        break;
    default:
        seterr(debugContext, 0);
        break;
    }
}

C_VOID core_product_debug_context_initialize(core_product_debug_context *context)
{
    if (context != STD_NULL)
        STD_MEMSET(context, 0, sizeof(*context));
}

C_VOID core_product_debug_main(core_product_debug_context *context,
                               const core_product_debug_target *target,
                               const core_product_debug_input_provider *input_provider)
{
    const core_utils_wait_scope *wait_scope;
    core_product_debug_context *debugContext = context;
    STD_SIZE_T i;
    core_product_debug_fault_outcome fault;

    if (context == STD_NULL || target == STD_NULL)
        return;
    wait_scope = context->wait_scope;
    core_product_debug_context_initialize(context);
    context->target = target;
    context->input_provider = input_provider;
    context->wait_scope = wait_scope;
    if (core_product_debug_get_fault_outcome(target, &fault) && fault.valid) {
        STD_PRINTF("fault: detail=%08X pc=%08X", fault.detail, fault.linear_pc);
        if (fault.diagnostic_valid) {
            STD_PRINTF(" exception=%08X code=%08X at %04X:%08X",
                fault.exception_mask, fault.exception_code, fault.cs, fault.eip);
        }
        STD_PRINTF("\n");
    }
    strFileName[0] = '\0';
    asmSegRec = uasmSegRec = _cs;
    asmPtrRec = uasmPtrRec = _ip;
    dumpSegRec = _ds;
    dumpPtrRec = (type_unsigned_16)(_ip) / 0x10 * 0x10;
    xalin = 0;
    xdlin = 0;
    xulin = core_product_debug_get_code_base() + _eip;
    arg = (C_CHAR **)STD_MALLOC(DEBUG_MAXNARG * sizeof(C_CHAR *));
    flagExit = 0;
    while (!flagExit)
    {
        core_product_debug_flush_console_input(debugContext);
        STD_PRINTF("-");
        STD_FGETS(strCmdBuff, 0x100, STD_STDIN);
        parse(debugContext);
        exec(debugContext);
        if (nErrPos)
        {
            for (i = 0; i < nErrPos; ++i)
            {
                STD_PRINTF(" ");
            }
            STD_PRINTF("^ Error\n");
        }
    }
    STD_FREE((C_VOID *)arg);
    context->target = STD_NULL;
    context->input_provider = STD_NULL;
}
