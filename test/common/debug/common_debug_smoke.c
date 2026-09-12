#include "common/debug/debug_interface.h"
#include "lib/storage/file_interface.h"

typedef struct debug_fixture {
    lib_u32 registers[COMMON_DEBUG_REGISTER_COUNT];
    lib_u8 memory[4096];
    common_machine_debug_execution_plan_kind execution_kind;
    lib_u32 execution_address;
    lib_u64 execution_count;
    lib_bool watches[3];
    lib_u32 watch_addresses[3];
} debug_fixture;

static void debug_fixture_consume(void *context,
    const common_machine_request *request)
{ (void)context; (void)request; }

static lib_bool debug_fixture_paused(void *context)
{
    return context != LIB_NULL ? LIB_TRUE : LIB_FALSE;
}

static lib_status debug_fixture_execute(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *result)
{
    debug_fixture *fixture = context;

    if (fixture == LIB_NULL || request == LIB_NULL || result == LIB_NULL ||
        request->bytes > COMMON_MACHINE_DEBUG_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(result, 0, sizeof(*result));
    switch (request->operation) {
    case COMMON_MACHINE_DEBUG_READ_REGISTER:
        if (request->register_id >= COMMON_DEBUG_REGISTER_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
        result->value = fixture->registers[request->register_id];
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_WRITE_REGISTER:
        if (request->register_id >= COMMON_DEBUG_REGISTER_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
        fixture->registers[request->register_id] = request->address;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_READ_REAL:
    case COMMON_MACHINE_DEBUG_WRITE_REAL: {
        lib_u32 address = ((lib_u32)request->segment << 4u) + request->offset;
        if (address + request->bytes > sizeof(fixture->memory)) return LIB_STATUS_INVALID_ARGUMENT;
        if (request->operation == COMMON_MACHINE_DEBUG_READ_REAL)
            lib_memory_copy(result->data, fixture->memory + address, request->bytes);
        else lib_memory_copy(fixture->memory + address, request->data, request->bytes);
        result->bytes = request->bytes;
        return LIB_STATUS_OK;
    }
    case COMMON_MACHINE_DEBUG_READ_LINEAR:
    case COMMON_MACHINE_DEBUG_WRITE_LINEAR:
        if (request->address + request->bytes > sizeof(fixture->memory))
            return LIB_STATUS_INVALID_ARGUMENT;
        if (request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR)
            lib_memory_copy(result->data, fixture->memory + request->address,
                request->bytes);
        else
            lib_memory_copy(fixture->memory + request->address, request->data,
                request->bytes);
        result->bytes = request->bytes;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_GET_CPU_SNAPSHOT:
        result->cpu.es = (common_machine_debug_segment_snapshot) {
            .selector = 0u, .base = 0u, .limit = 0xffffu,
            .writable = LIB_TRUE };
        result->cpu.cs = (common_machine_debug_segment_snapshot) {
            .selector = 0u, .base = 0u, .limit = 0xffffu,
            .executable = LIB_TRUE, .readable = LIB_TRUE };
        result->cpu.ss = result->cpu.ds = result->cpu.fs = result->cpu.gs =
            result->cpu.es;
        result->cpu.cr0 = 0x00000011u;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE:
        result->value = LIB_FALSE;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_SET_WATCH:
        if (request->watch_kind > COMMON_MACHINE_DEBUG_WATCH_EXECUTE)
            return LIB_STATUS_INVALID_ARGUMENT;
        fixture->watches[request->watch_kind] = LIB_TRUE;
        fixture->watch_addresses[request->watch_kind] = request->address;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_CLEAR_WATCH:
        if (request->watch_kind > COMMON_MACHINE_DEBUG_WATCH_EXECUTE)
            return LIB_STATUS_INVALID_ARGUMENT;
        fixture->watches[request->watch_kind] = LIB_FALSE;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_GET_WATCH:
        if (request->watch_kind > COMMON_MACHINE_DEBUG_WATCH_EXECUTE)
            return LIB_STATUS_INVALID_ARGUMENT;
        result->enabled = fixture->watches[request->watch_kind];
        result->value = fixture->watch_addresses[request->watch_kind];
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN:
        fixture->execution_kind = request->execution_kind;
        fixture->execution_address = request->address;
        fixture->execution_count = request->instruction_count;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_CLEAR_EXECUTION_PLAN:
        fixture->execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_NONE;
        fixture->execution_address = 0u;
        fixture->execution_count = 0u;
        return LIB_STATUS_OK;
    case COMMON_MACHINE_DEBUG_GET_EXECUTION_RESULT:
        result->enabled = fixture->execution_kind !=
            COMMON_MACHINE_DEBUG_EXECUTION_NONE;
        result->value = 1u;
        fixture->execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_NONE;
        return LIB_STATUS_OK;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
}

static int debug_text_contains(const char *text, const char *fragment)
{
    lib_size text_bytes;
    lib_size fragment_bytes;
    lib_size index;

    if (text == LIB_NULL || fragment == LIB_NULL) return LIB_FALSE;
    text_bytes = lib_text_length(text);
    fragment_bytes = lib_text_length(fragment);
    if (fragment_bytes == 0u) return LIB_TRUE;
    if (fragment_bytes > text_bytes) return LIB_FALSE;
    for (index = 0u; index <= text_bytes - fragment_bytes; ++index) {
        if (lib_memory_compare(text + index, fragment, fragment_bytes) == 0)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

static int debug_expect(common_debug *debug, const char *line,
    const char *expected_text, const char *expected_prompt)
{
    common_debug_result result;

    if (common_debug_submit_line(debug, line, &result) != LIB_STATUS_OK ||
        (expected_text != LIB_NULL && !debug_text_contains(result.text, expected_text)) ||
        (expected_prompt != LIB_NULL && (!result.prompt_ready ||
            lib_text_compare(result.prompt, expected_prompt) != 0))) {
        return 0;
    }
    return 1;
}

static int debug_expect_machine(common_debug *debug,
    common_debug_machine_state state, common_debug_lifecycle_request request,
    const char *expected_text)
{
    common_debug_result result;

    if (common_debug_observe_machine(debug, state, LIB_STATUS_OK, &result) !=
            LIB_STATUS_OK || result.lifecycle_request != request ||
        (expected_text != LIB_NULL && !debug_text_contains(result.text,
            expected_text))) {
        return 0;
    }
    return 1;
}

int main(void)
{
    common_machine *machine = LIB_NULL;
    common_debug *debug = LIB_NULL;
    debug_fixture fixture = {0};
    const char *file_name = "common_debug_smoke.bin";
    const unsigned char file_input[] = { 0x51u, 0x52u };
    void *file_output = LIB_NULL;
    lib_size file_output_bytes = 0u;
    lib_storage_file_writer *writer = LIB_NULL;
    common_machine_driver driver = {
        debug_fixture_consume,
        debug_fixture_paused,
        debug_fixture_execute,
        &fixture
    };

    fixture.registers[COMMON_DEBUG_EAX] = 0x12345678u;
    fixture.registers[COMMON_DEBUG_DS] = 0u;
    fixture.memory[0] = 0x41u;
    if (lib_storage_file_writer_open(file_name, LIB_STORAGE_FILE_WRITER_TRUNCATE,
            &writer) != LIB_STATUS_OK ||
        lib_storage_file_writer_write(writer, file_input, sizeof(file_input)) !=
            LIB_STATUS_OK || lib_storage_file_writer_close(writer) != LIB_STATUS_OK) {
        return 1;
    }
    if (common_machine_create(&machine) != LIB_STATUS_OK ||
        common_machine_bind_driver(machine, &driver) != LIB_STATUS_OK ||
        common_debug_create(&debug) != LIB_STATUS_OK ||
        common_debug_open(debug, machine) != LIB_STATUS_OK ||
        !debug_expect(debug, "r", "AX=5678", "-") ||
        !debug_expect(debug, "r ax", "AX 5678", ":") ||
        !debug_expect(debug, "3040", "", "-") ||
        fixture.registers[COMMON_DEBUG_EAX] != 0x00003040u ||
        !debug_expect(debug, "xr eax", "EAX 00003040", ":") ||
        !debug_expect(debug, "11223344", "", "-") ||
        fixture.registers[COMMON_DEBUG_EAX] != 0x11223344u ||
        !debug_expect(debug, "x sreg",
            "ES=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Data, e, RW, big\n"
            "CS=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Code, c, Rw, 16\n"
            "SS=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Data, e, RW, big\n"
            "DS=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Data, e, RW, big\n"
            "FS=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Data, e, RW, big\n"
            "GS=0000, Base=00000000, Limit=0000FFFF, DPL=0, a, Data, e, RW, big\n"
            "TR  =0000, Base=00000000, Limit=00000000, DPL=0, Type=0000\n"
            "LDTR=0000, Base=00000000, Limit=00000000, DPL=0, Type=0000\n"
            "GDTR Base=00000000, Limit=0000\nIDTR Base=00000000, Limit=0000\n", "-") ||
        !debug_expect(debug, "x creg",
            "CR0=00000011: pg ET ts em mp PE\nCR2=PFLR=00000000\nCR3=PDBR=00000000\n", "-") ||
        !debug_expect(debug, "d 0:0 1", "0000:0000  41", "-") ||
        !debug_expect(debug, "e 0:1", "", "enter> ") ||
        !debug_expect(debug, "42", "0000:0001", "-") ||
        fixture.memory[1] != 0x42u ||
        !debug_expect(debug, "", "", LIB_NULL) ||
        !debug_expect(debug, "a 0:2", "", "assemble> ") ||
        !debug_expect(debug, "nop", "", "assemble> ") ||
        fixture.memory[2] != 0x90u ||
        !debug_expect(debug, "", "", LIB_NULL) ||
        !debug_expect(debug, "xa 3", "", "-") ||
        !debug_expect(debug, "nop", "", "-") ||
        fixture.memory[3] != 0x90u ||
        !debug_expect(debug, "", "", LIB_NULL) ||
        !debug_expect(debug, "xe 4", "", "-") ||
        !debug_expect(debug, "43", "L00000004", "-") ||
        fixture.memory[4] != 0x43u ||
        !debug_expect(debug, "v", ":", "-") ||
        !debug_expect(debug, "ab", "61 62", "-") ||
        !debug_expect(debug, "n common_debug_smoke.bin", "", "-") ||
        !debug_expect(debug, "l 0:10", "", "-") ||
        fixture.memory[0x10] != 0x51u || fixture.memory[0x11] != 0x52u ||
        !debug_expect(debug, "w 0:10", "", "-") ||
        !debug_expect(debug, "xw r 12ab", "", "-") ||
        !debug_expect(debug, "xw", "Watch-read point: Lin=000012ab", "-") ||
        !debug_expect(debug, "xw r", "Watch-read point removed.", "-") ||
        !debug_expect(debug, "xw", LIB_NULL, "-") ||
        fixture.watches[COMMON_MACHINE_DEBUG_WATCH_READ] ||
        !debug_expect(debug, "t 2", "", "-") ||
        fixture.execution_kind != COMMON_MACHINE_DEBUG_EXECUTION_TRACE ||
        fixture.execution_count != 1u ||
        (common_debug_observe_instruction(debug,
            &(common_debug_instruction_observation){
                .memory_access_count = 0u }),
            0) ||
        !debug_expect_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
            COMMON_DEBUG_LIFECYCLE_RESUME, "AX=3344") ||
        fixture.execution_kind != COMMON_MACHINE_DEBUG_EXECUTION_TRACE ||
        fixture.execution_count != 1u ||
        (common_debug_observe_instruction(debug,
            &(common_debug_instruction_observation){
                .memory_access_count = 0u }),
            0) ||
        !debug_expect_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
            COMMON_DEBUG_LIFECYCLE_NONE, "AX=3344") ||
        !debug_expect(debug, "xt 1", "", "-") ||
        (common_debug_observe_instruction(debug,
            &(common_debug_instruction_observation){
                .memory_accesses = {{ LIB_FALSE, 0x1234u, 1u, 0x5au }},
                .memory_access_count = 1u }),
            0) ||
        !debug_expect_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
            COMMON_DEBUG_LIFECYCLE_NONE, "Read: Lin=00001234") ||
        !debug_expect(debug, "g 0:2", "", "-") ||
        fixture.execution_kind != COMMON_MACHINE_DEBUG_EXECUTION_BREAK_LINEAR ||
        fixture.execution_address != 2u ||
        lib_storage_file_read_owned(file_name, sizeof(file_input), &file_output,
            &file_output_bytes) != LIB_STATUS_OK ||
        file_output_bytes != sizeof(file_input) ||
        lib_memory_compare(file_input, file_output, sizeof(file_input)) != 0) {
        lib_release(file_output);
        common_debug_destroy(debug);
        common_machine_destroy(machine);
        return 1;
    }
    lib_release(file_output);
    common_debug_destroy(debug);
    common_machine_destroy(machine);
    return 0;
}
