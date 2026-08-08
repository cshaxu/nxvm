/* Copyright 2012-2014 Neko. */

/*
 * VDEBUG provides hardware debug device which collects
 * device status and stops device thread at breakpoints.
 */

#include "type.h"

#include "core/machine/cpu_instructions.h"


#include "vm/machine/debug.h"

static C_VOID debug_request_pause(t_debug *debug,
    vm_machine_debug_pause_reason reason)
{
    if (debug->connect.pauseCallback != STD_NULL) {
        debug->connect.pauseCallback(debug->connect.pauseContext, reason);
    }
}

C_VOID vm_machine_debug_initialize(t_debug *debug)
{
    if (debug == STD_NULL) return;
    STD_MEMSET((C_VOID *)debug, TYPE_ZERO_8, sizeof(*debug));
}
C_VOID vm_machine_debug_reset(t_debug *debug)
{
    if (debug == STD_NULL) return;
    STD_MEMSET((C_VOID *)&debug->data, TYPE_ZERO_8, sizeof(debug->data));
}
#define _expression "cs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s | cs:eip=%04x:%08x(L%08x)"
C_VOID vm_machine_debug_refresh(t_debug *debug,
    const core_machine_debug_instruction_observation *observation) {
    if (debug == STD_NULL || observation == STD_NULL) return;
    debug->connect.observation = *observation;
    debug->connect.observation_valid = TYPE_TRUE;
    if ((debug->data.flagBreak && debug->connect.observation.cpu.data.cs.selector == debug->data.breakCS && debug->connect.observation.cpu.data.ip == debug->data.breakIP) ||
            (debug->data.flagBreak32 && debug->data.breakCount && (debug->connect.observation.cpu.data.cs.base + debug->connect.observation.cpu.data.eip == debug->data.breakLinear))) {
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
    /* dump cpu status before execution */
    if (debug->connect.recordFile) {
        type_native_unsigned i;
        type_string_buffer stmt;
        STD_FPRINTF(debug->connect.recordFile, _expression,
                debug->connect.observation.cpu.data.cs.selector, debug->connect.observation.cpu.data.eip, debug->connect.observation.cpu.data.cs.base + debug->connect.observation.cpu.data.eip,
                debug->connect.observation.cpu.data.ss.selector, debug->connect.observation.cpu.data.esp, debug->connect.observation.cpu.data.ss.base + debug->connect.observation.cpu.data.esp,
                debug->connect.observation.cpu.data.eax, debug->connect.observation.cpu.data.ecx, debug->connect.observation.cpu.data.edx, debug->connect.observation.cpu.data.ebx,
                debug->connect.observation.cpu.data.ebp, debug->connect.observation.cpu.data.esi, debug->connect.observation.cpu.data.edi,
                debug->connect.observation.cpu.data.ds.selector, debug->connect.observation.cpu.data.es.selector,
                debug->connect.observation.cpu.data.fs.selector, debug->connect.observation.cpu.data.gs.selector,
                debug->connect.observation.cpu.data.eflags,
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_OF) ? "OF" : "of",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_SF) ? "SF" : "sf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_ZF) ? "ZF" : "zf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_CF) ? "CF" : "cf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_AF) ? "AF" : "af",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_PF) ? "PF" : "pf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_DF) ? "DF" : "df",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_IF) ? "IF" : "if",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_TF) ? "TF" : "tf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_VM) ? "VM" : "vm",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_RF) ? "RF" : "rf",
                TYPE_GET_BIT(debug->connect.observation.cpu.data.eflags, VCPU_EFLAGS_NT) ? "NT" : "nt",
                debug->connect.observation.instructions.data.reccs, debug->connect.observation.instructions.data.receip, debug->connect.observation.instructions.data.linear);

        /* disassemble opcode */
        if (debug->connect.observation.instructions.data.oplen) {
            debug->connect.observation.instructions.data.oplen = debug->connect.disassembleProvider == STD_NULL ?
                0u : debug->connect.disassembleProvider(
                    debug->connect.disassembleContext, stmt,
                    debug->connect.observation.instructions.data.opcodes,
                    debug->connect.observation.cpu.data.cs.seg.exec.defsize);
            for (i = 0; i < STD_STRLEN(stmt); ++i) {
                if (stmt[i] == '\n') {
                    stmt[i] = ' ';
                }
            }
        } else {
            (C_VOID)STD_SNPRINTF(stmt, sizeof(stmt), "<ERROR>");
        }

        /* print opcode, at least print 8 bytes */
        for (i = 0; i < debug->connect.observation.instructions.data.oplen; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "%02X", debug->connect.observation.instructions.data.opcodes[i]);
        }
        for (i = debug->connect.observation.instructions.data.oplen; i < 8; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "  ");
        }

        /* print assembly, at least 40 char in length */
        STD_FPRINTF(debug->connect.recordFile, "%s ", stmt);
        for (i = STD_STRLEN(stmt); i < 40; ++i) {
            STD_FPRINTF(debug->connect.recordFile, " ");
        }

        /* print memory usage */
        for (i = 0; i < debug->connect.observation.instructions.data.msize; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "[%c:L%08x/%1d/%016llx] ",
                    debug->connect.observation.instructions.data.mem[i].flagWrite ? 'W' : 'R', debug->connect.observation.instructions.data.mem[i].linear,
                    debug->connect.observation.instructions.data.mem[i].byte, debug->connect.observation.instructions.data.mem[i].data);
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

C_VOID vm_machine_debug_bind_disassembler(t_debug *debug,
    vm_machine_debug_disassemble_provider provider, C_VOID *context)
{
    if (debug == STD_NULL) return;
    debug->connect.disassembleProvider = provider;
    debug->connect.disassembleContext = context;
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
