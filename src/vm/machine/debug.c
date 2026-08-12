/* Copyright 2012-2014 Neko. */

/*
 * VDEBUG provides hardware debug device which collects
 * device status and stops device thread at breakpoints.
 */

#include "type.h"

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
    if ((debug->data.flagBreak && debug->connect.observation.cs ==
            debug->data.breakCS && debug->connect.observation.eip ==
            debug->data.breakIP) || (debug->data.flagBreak32 &&
            debug->data.breakCount && (debug->connect.observation.cs_base +
            debug->connect.observation.eip == debug->data.breakLinear))) {
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
                debug->connect.observation.cs, debug->connect.observation.eip,
                debug->connect.observation.cs_base + debug->connect.observation.eip,
                debug->connect.observation.ss, debug->connect.observation.esp,
                debug->connect.observation.ss_base + debug->connect.observation.esp,
                debug->connect.observation.eax, debug->connect.observation.ecx,
                debug->connect.observation.edx, debug->connect.observation.ebx,
                debug->connect.observation.ebp, debug->connect.observation.esi,
                debug->connect.observation.edi, debug->connect.observation.ds,
                debug->connect.observation.es, debug->connect.observation.fs,
                debug->connect.observation.gs, debug->connect.observation.eflags,
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_OF) ? "OF" : "of",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_SF) ? "SF" : "sf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_ZF) ? "ZF" : "zf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_CF) ? "CF" : "cf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_AF) ? "AF" : "af",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_PF) ? "PF" : "pf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_DF) ? "DF" : "df",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_IF) ? "IF" : "if",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_TF) ? "TF" : "tf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_VM) ? "VM" : "vm",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_RF) ? "RF" : "rf",
                (debug->connect.observation.eflags & CORE_MACHINE_DEBUG_EFLAGS_NT) ? "NT" : "nt",
                debug->connect.observation.instruction_cs,
                debug->connect.observation.instruction_eip,
                debug->connect.observation.instruction_linear);

        /* disassemble opcode */
        if (debug->connect.observation.instruction_byte_count) {
            debug->connect.observation.instruction_byte_count = debug->connect.disassembleProvider == STD_NULL ?
                0u : debug->connect.disassembleProvider(
                    debug->connect.disassembleContext, stmt,
                    debug->connect.observation.instruction_bytes,
                    debug->connect.observation.code_default_size);
            for (i = 0; i < STD_STRLEN(stmt); ++i) {
                if (stmt[i] == '\n') {
                    stmt[i] = ' ';
                }
            }
        } else {
            (C_VOID)STD_SNPRINTF(stmt, sizeof(stmt), "<ERROR>");
        }

        /* print opcode, at least print 8 bytes */
        for (i = 0; i < debug->connect.observation.instruction_byte_count; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "%02X", debug->connect.observation.instruction_bytes[i]);
        }
        for (i = debug->connect.observation.instruction_byte_count; i < 8; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "  ");
        }

        /* print assembly, at least 40 char in length */
        STD_FPRINTF(debug->connect.recordFile, "%s ", stmt);
        for (i = STD_STRLEN(stmt); i < 40; ++i) {
            STD_FPRINTF(debug->connect.recordFile, " ");
        }

        /* print memory usage */
        for (i = 0; i < debug->connect.observation.memory_access_count; ++i) {
            STD_FPRINTF(debug->connect.recordFile, "[%c:L%08x/%1d/%016llx] ",
                    debug->connect.observation.memory_accesses[i].write ? 'W' : 'R',
                    debug->connect.observation.memory_accesses[i].linear,
                    debug->connect.observation.memory_accesses[i].bytes,
                    debug->connect.observation.memory_accesses[i].data);
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

C_VOID vm_machine_debug_set_breakpoint_real(t_debug *debug, type_unsigned_16 segment,
    type_unsigned_16 offset) {
    if (debug == STD_NULL) return;
    debug->data.breakCS = segment;
    debug->data.breakIP = offset;
    debug->data.flagBreak = TYPE_TRUE;
}
C_VOID vm_machine_debug_clear_breakpoint_real(t_debug *debug) {
    if (debug == STD_NULL) return;
    debug->data.flagBreak = TYPE_FALSE;
}
C_VOID vm_machine_debug_set_breakpoint_linear(t_debug *debug, type_unsigned_32 linear) {
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
