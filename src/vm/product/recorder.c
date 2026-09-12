#include "vm/product/recorder.h"

#include "common/xasm32/xasm32_interface.h"
#include "lib/storage/file_interface.h"

struct vm_product_recorder {
    lib_storage_file_writer *writer;
};

static type_status recorder_status(lib_status status)
{
    switch (status) {
    case LIB_STATUS_OK: return TYPE_STATUS_OK;
    case LIB_STATUS_INVALID_ARGUMENT: return TYPE_STATUS_INVALID_ARGUMENT;
    case LIB_STATUS_INVALID_STATE: return TYPE_STATUS_INVALID_STATE;
    case LIB_STATUS_NO_MEMORY: return TYPE_STATUS_NO_MEMORY;
    default: return TYPE_STATUS_FAULT;
    }
}

static C_INT recorder_write(vm_product_recorder *recorder,
    const C_CHAR *format, ...)
{
    C_CHAR text[4096];
    STD_VA_LIST arguments;
    C_INT written;

    if (recorder == STD_NULL || recorder->writer == STD_NULL) return 0;
    va_start(arguments, format);
    written = vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    return written >= 0 && (STD_SIZE_T)written < sizeof(text) &&
        lib_storage_file_writer_write(recorder->writer, text,
            (lib_size)written) == LIB_STATUS_OK;
}

type_status vm_product_recorder_create(vm_product_recorder **out_recorder)
{
    vm_product_recorder *recorder;

    if (out_recorder == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_recorder = STD_NULL;
    recorder = STD_CALLOC(1u, sizeof(*recorder));
    if (recorder == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_recorder = recorder;
    return TYPE_STATUS_OK;
}

void vm_product_recorder_destroy(vm_product_recorder *recorder)
{
    if (recorder == STD_NULL) return;
    (void)vm_product_recorder_stop(recorder);
    STD_FREE(recorder);
}

type_status vm_product_recorder_start(vm_product_recorder *recorder,
    const C_CHAR *path)
{
    if (recorder == STD_NULL || path == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    (void)vm_product_recorder_stop(recorder);
    return recorder_status(lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &recorder->writer));
}

type_status vm_product_recorder_stop(vm_product_recorder *recorder)
{
    lib_status status;

    if (recorder == STD_NULL || recorder->writer == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    status = lib_storage_file_writer_close(recorder->writer);
    recorder->writer = STD_NULL;
    return recorder_status(status);
}

void vm_product_recorder_observe(void *context,
    const vm_machine_debug_observation *observation)
{
    vm_product_recorder *recorder = context;
    C_CHAR statement[256] = "<ERROR>";
    STD_SIZE_T decoded_bytes = 0u;
    STD_SIZE_T index;

    if (recorder == STD_NULL || recorder->writer == STD_NULL ||
        observation == STD_NULL) return;
    (void)common_xasm32_disassemble(observation->instruction_bytes,
        observation->instruction_byte_count, statement, sizeof(statement),
        &decoded_bytes, observation->code_default_size);
    for (index = 0u; statement[index] != '\0'; ++index)
        if (statement[index] == '\n') statement[index] = ' ';
    if (!recorder_write(recorder,
            "cs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) "
            "eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x "
            "ds=%04x es=%04x fs=%04x gs=%04x eflags=%08x | ",
            observation->cs, observation->eip,
            observation->cs_base + observation->eip, observation->ss,
            observation->esp, observation->ss_base + observation->esp,
            observation->eax, observation->ecx, observation->edx,
            observation->ebx, observation->ebp, observation->esi,
            observation->edi, observation->ds, observation->es,
            observation->fs, observation->gs, observation->eflags)) return;
    for (index = 0u; index < observation->instruction_byte_count; ++index)
        if (!recorder_write(recorder, "%02X", observation->instruction_bytes[index])) return;
    if (!recorder_write(recorder, " %-40s", statement)) return;
    for (index = 0u; index < observation->memory_access_count; ++index)
        if (!recorder_write(recorder, " [%c:L%08x/%u/%016llx]",
                observation->memory_accesses[index].write ? 'W' : 'R',
                observation->memory_accesses[index].linear,
                observation->memory_accesses[index].bytes,
                (unsigned long long)observation->memory_accesses[index].data)) return;
    (void)recorder_write(recorder, "\n");
}
