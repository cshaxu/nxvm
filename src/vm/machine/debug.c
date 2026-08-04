/* Copyright 2012-2014 Neko. */

/*
 * VDEBUG provides hardware debug device which collects
 * device status and stops device thread at breakpoints.
 */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/cpu_instructions.h"


#include "vm/machine/debug.h"

static C_VOID debug_request_pause(t_debug *debug,
    vm_machine_debug_pause_reason reason)
{
    if (debug->connect.pauseCallback != STD_NULL) {
        debug->connect.pauseCallback(debug->connect.pauseContext, reason);
    }
}

#if 0
static C_VOID xasm_test(t_debug *debug) {
    type_native_unsigned total = 0; /* diagnostic-only instruction count */
    type_bool flagStop = TYPE_TRUE; /* stop the VM if comparison fails */
    type_native_unsigned i, lenDasm1, lenDasm2, lenAasm;
    type_string_buffer strDasm1, strDasm2;
    type_unsigned_8 ins1[15], ins2[15];
    total++;
    debug->connect.cpuinsReadLinear(debug->connect.cpu->data.cs.base + debug->connect.cpu->data.eip, (type_virtual_address) ins1, 15);
    /* ins1[0] = 0x67;
    ins1[1] = 0xc6;
    ins1[2] = 0x44;
    ins1[3] = 0xf2;
    ins1[4] = 0x05;
    ins1[5] = 0x8e;
    ins1[6] = 0x00;*/
    switch (TYPE_DEREFERENCE_UNSIGNED_8(ins1)) {
    case 0x88:
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
        flagStop = TYPE_FALSE;
        break;
    }
    switch (TYPE_DEREFERENCE_UNSIGNED_8(ins1+1)) {
    case 0x90:
        flagStop = TYPE_FALSE;
        break;
    }
    switch (TYPE_DEREFERENCE_UNSIGNED_16(ins1)) {
    case 0x2e66:
        flagStop = TYPE_FALSE;
        break;
    }
    switch (TYPE_MASK_UNSIGNED_24(TYPE_DEREFERENCE_UNSIGNED_24(ins1))) {
    case 0xb70f66:
        flagStop = TYPE_FALSE;
        break;
    }
    switch (TYPE_MASK_UNSIGNED_24(TYPE_DEREFERENCE_UNSIGNED_24(ins1+1))) {
    case 0xb70f66:
        flagStop = TYPE_FALSE;
        break;
    }
    switch (TYPE_DEREFERENCE_UNSIGNED_32(ins1)) {
    }
    lenDasm1 = core_product_utils_dasm32(strDasm1, ins1, debug->connect.cpu->data.cs.seg.exec.defsize);
    lenAasm  = core_product_utils_aasm32(strDasm1, ins2, debug->connect.cpu->data.cs.seg.exec.defsize);
    lenDasm2 = core_product_utils_dasm32(strDasm2, ins2, debug->connect.cpu->data.cs.seg.exec.defsize);
    if ((flagStop && (lenAasm != lenDasm1 || lenAasm != lenDasm2 || lenDasm1 != lenDasm2 ||
                      STD_MEMCMP((C_VOID *) ins1, (C_VOID *) ins2, lenDasm1))) || STD_STRCMP(strDasm1, strDasm2)) {
        STD_PRINTF("diff at #%d %04X:%08X(L%08X), len(a=%x,d1=%x,d2=%x), CodeSegDefSize=%d\n",
               total, debug->connect.cpu->data.cs.selector, debug->connect.cpu->data.eip, debug->connect.cpu->data.cs.base + debug->connect.cpu->data.eip,
               lenAasm, lenDasm1, lenDasm2, debug->connect.cpu->data.cs.seg.exec.defsize ? 32 : 16);
        for (i = 0; i < lenDasm1; ++i) {
            STD_PRINTF("%02X", ins1[i]);
        }
        STD_PRINTF("\t%s\n", strDasm1);
        for (i = 0; i < lenDasm2; ++i) {
            STD_PRINTF("%02X", ins2[i]);
        }
        STD_PRINTF("\t%s\n", strDasm2);
        debug_request_pause(debug, VM_MACHINE_DEBUG_PAUSE_BREAKPOINT);
    }
}

#endif

C_VOID vm_machine_debug_initialize(t_debug *debug, t_cpu *cpu, t_cpuins *cpuins)
{
    if (debug == STD_NULL) return;
    STD_MEMSET((C_VOID *)debug, TYPE_ZERO_8, sizeof(*debug));
    debug->connect.cpu = cpu;
    debug->connect.cpuins = cpuins;
}
C_VOID vm_machine_debug_reset(t_debug *debug)
{
    if (debug == STD_NULL) return;
    STD_MEMSET((C_VOID *)&debug->data, TYPE_ZERO_8, sizeof(debug->data));
}
#define _expression "cs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s | cs:eip=%04x:%08x(L%08x)"
C_VOID vm_machine_debug_refresh(t_debug *debug) {
    if (debug == STD_NULL || debug->connect.cpu == STD_NULL ||
        debug->connect.cpuins == STD_NULL) return;
    if ((debug->data.flagBreak && debug->connect.cpu->data.cs.selector == debug->data.breakCS && debug->connect.cpu->data.ip == debug->data.breakIP) ||
            (debug->data.flagBreak32 && debug->data.breakCount && (debug->connect.cpu->data.cs.base + debug->connect.cpu->data.eip == debug->data.breakLinear))) {
        debug_request_pause(debug, VM_MACHINE_DEBUG_PAUSE_BREAKPOINT);
    }
    debug->data.breakCount++;
    if (debug->data.flagTrace) {
        if (!debug->data.traceCount) {
            debug->data.flagTrace = TYPE_FALSE;
            debug_request_pause(debug, VM_MACHINE_DEBUG_PAUSE_TRACE);
        } else {
            debug->data.traceCount--;
        }
    }
    /* TODO(Medium): Add a dedicated assembler regression target. */
    /* dump cpu status before execution */
    if (debug->connect.recordFile) {
        type_native_unsigned i;
        type_string_buffer stmt;
        STD_FPRINTF(debug->connect.recordFile, _expression,
                debug->connect.cpu->data.cs.selector, debug->connect.cpu->data.eip, debug->connect.cpu->data.cs.base + debug->connect.cpu->data.eip,
                debug->connect.cpu->data.ss.selector, debug->connect.cpu->data.esp, debug->connect.cpu->data.ss.base + debug->connect.cpu->data.esp,
                debug->connect.cpu->data.eax, debug->connect.cpu->data.ecx, debug->connect.cpu->data.edx, debug->connect.cpu->data.ebx,
                debug->connect.cpu->data.ebp, debug->connect.cpu->data.esi, debug->connect.cpu->data.edi,
                debug->connect.cpu->data.ds.selector, debug->connect.cpu->data.es.selector,
                debug->connect.cpu->data.fs.selector, debug->connect.cpu->data.gs.selector,
                debug->connect.cpu->data.eflags,
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_OF) ? "OF" : "of",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_SF) ? "SF" : "sf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_ZF) ? "ZF" : "zf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_CF) ? "CF" : "cf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_AF) ? "AF" : "af",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_PF) ? "PF" : "pf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_DF) ? "DF" : "df",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_IF) ? "IF" : "if",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_TF) ? "TF" : "tf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_VM) ? "VM" : "vm",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_RF) ? "RF" : "rf",
                TYPE_GET_BIT(debug->connect.cpu->data.eflags, VCPU_EFLAGS_NT) ? "NT" : "nt",
                debug->connect.cpuins->data.reccs, debug->connect.cpuins->data.receip, debug->connect.cpuins->data.linear);

        /* disassemble opcode */
        if (debug->connect.cpuins->data.oplen) {
            debug->connect.cpuins->data.oplen = core_product_utils_dasm32(stmt, debug->connect.cpuins->data.opcodes, debug->connect.cpu->data.cs.seg.exec.defsize);
            for (i = 0; i < STD_STRLEN(stmt); ++i) {
                if (stmt[i] == '\n') {
                    stmt[i] = ' ';
                }
            }
        } else {
            STD_SPRINTF(stmt, "<ERROR>");
        }

        /* print opcode, at least print 8 bytes */
        for (i = 0; i < debug->connect.cpuins->data.oplen; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "%02X", debug->connect.cpuins->data.opcodes[i]);
        }
        for (i = debug->connect.cpuins->data.oplen; i < 8; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "  ");
        }

        /* print assembly, at least 40 char in length */
        STD_FPRINTF(debug->connect.recordFile, "%s ", stmt);
        for (i = STD_STRLEN(stmt); i < 40; ++i) {
            STD_FPRINTF(debug->connect.recordFile, " ");
        }

        /* print memory usage */
        for (i = 0; i < debug->connect.cpuins->data.msize; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "[%c:L%08x/%1d/%016llx] ",
                    debug->connect.cpuins->data.mem[i].flagWrite ? 'W' : 'R', debug->connect.cpuins->data.mem[i].linear,
                    debug->connect.cpuins->data.mem[i].byte, debug->connect.cpuins->data.mem[i].data);
        }

        STD_FPRINTF(debug->connect.recordFile, "\n");
    }
}
C_VOID vm_machine_debug_finalize(t_debug *debug) { (C_VOID)debug; }

C_VOID vm_machine_debug_bind_pause(t_debug *debug,
    vm_machine_debug_pause_callback callback, C_VOID *context)
{
    if (debug == STD_NULL) return;
    debug->connect.pauseCallback = callback;
    debug->connect.pauseContext = context;
}

C_VOID vm_machine_debug_set_breakpoint_real(t_debug *debug, uint16_t segment,
    uint16_t offset) {
    if (debug == STD_NULL) return;
    debug->data.breakCS = segment;
    debug->data.breakIP = offset;
    debug->data.flagBreak = TYPE_TRUE;
}
C_VOID vm_machine_debug_clear_breakpoint_real(t_debug *debug) {
    if (debug == STD_NULL) return;
    debug->data.flagBreak = TYPE_FALSE;
}
C_VOID vm_machine_debug_set_breakpoint_linear(t_debug *debug, uint32_t linear) {
    if (debug == STD_NULL) return;
    debug->data.breakLinear = linear;
    debug->data.flagBreak32 = TYPE_TRUE;
    debug->data.breakCount = 0;
}
C_VOID vm_machine_debug_clear_breakpoint_linear(t_debug *debug) {
    if (debug == STD_NULL) return;
    debug->data.flagBreak32 = TYPE_FALSE;
}
STD_SIZE_T vm_machine_debug_get_breakpoint_count(const t_debug *debug) {
    if (debug == STD_NULL) return 0u;
    return debug->data.breakCount;
}
C_VOID vm_machine_debug_set_trace(t_debug *debug, STD_SIZE_T instruction_count) {
    if (debug == STD_NULL) return;
    debug->data.traceCount = instruction_count;
    debug->data.flagTrace = TYPE_TRUE;
}
C_VOID vm_machine_debug_clear_trace(t_debug *debug) {
    if (debug == STD_NULL) return;
    debug->data.flagTrace = TYPE_FALSE;
}
C_VOID vm_machine_debug_record_start(t_debug *debug, const C_CHAR *file_name) {
    if (debug == STD_NULL) return;
    if (debug->connect.recordFile) {
        STD_FCLOSE(debug->connect.recordFile);
    }
    debug->connect.recordFile = STD_FOPEN(file_name, "w");
    if (!debug->connect.recordFile) {
        STD_PRINTF("ERROR:\tcannot write dump file.\n");
    } else {
        STD_PRINTF("Record started.\n");
    }
}
C_VOID vm_machine_debug_record_stop(t_debug *debug) {
    if (debug == STD_NULL) return;
    if (!debug->connect.recordFile) {
        STD_PRINTF("ERROR:\trecorder not turned on.\n");
    } else {
        STD_PRINTF("Record finished.\n");
        STD_FCLOSE(debug->connect.recordFile);
        debug->connect.recordFile = (STD_FILE *) STD_NULL;
    }
}
