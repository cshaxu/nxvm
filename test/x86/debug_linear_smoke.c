#include "x86/debug/debug_interface.h"
#include <assert.h>

static lib_u8 memory[0x110000];
static lib_u32 memory_base;
static lib_u16 code_segment, data_segment;
static lib_u32 reads, writes;
static lib_bool short_response;
static lib_status acquire(common_machine *m, common_machine_debug_lease *lease)
{ (void)m; *lease = (common_machine_debug_lease){0}; return LIB_STATUS_OK; }
static void cancel(common_machine *m) { (void)m; }
static lib_status execute_x86(common_machine *m, const common_machine_debug_lease *lease,
    const x86_debug_request *request, x86_debug_response *result)
{
    (void)m; (void)lease;
    *result = (x86_debug_response){0};
    if (request->operation == X86_DEBUG_READ_REGISTER) {
        if (request->register_id == X86_DEBUG_CS) result->value = code_segment;
        if (request->register_id == X86_DEBUG_DS) result->value = data_segment;
    } else if (request->operation == X86_DEBUG_GET_CODE_DEFAULT_SIZE) {
        result->value = 1u;
    } else if (request->operation == X86_DEBUG_READ_LINEAR ||
               request->operation == X86_DEBUG_WRITE_LINEAR ||
               request->operation == X86_DEBUG_READ_REAL ||
               request->operation == X86_DEBUG_WRITE_REAL) {
        lib_u32 address = request->operation == X86_DEBUG_READ_REAL ||
            request->operation == X86_DEBUG_WRITE_REAL ?
            ((lib_u32)request->segment << 4) + request->offset : request->address;
        assert(request->bytes != 0u && request->bytes - 1u <= LIB_UINT32_MAX - address);
        assert(address >= memory_base && address - memory_base < sizeof(memory));
        assert(request->bytes <= sizeof(memory) - (address - memory_base));
        if (request->operation == X86_DEBUG_WRITE_LINEAR ||
            request->operation == X86_DEBUG_WRITE_REAL) {
            assert(++writes <= sizeof(memory));
            lib_memory_copy(memory + address - memory_base, request->data, request->bytes);
        } else {
            assert(++reads <= sizeof(memory));
            lib_memory_copy(result->data, memory + address - memory_base, request->bytes);
        }
    }
    return LIB_STATUS_OK;
}
static lib_status execute(common_machine *m, const common_machine_debug_lease *lease,
    const void *bytes, lib_size size, void *response, lib_size capacity, lib_size *response_size)
{
    x86_debug_request request;
    x86_debug_response result;
    assert(size == sizeof(request) && capacity == sizeof(result));
    lib_memory_copy(&request, bytes, size);
    lib_status status = execute_x86(m, lease, &request, &result);
    lib_memory_copy(response, &result, sizeof(result));
    *response_size = short_response ? sizeof(result) - 1u : sizeof(result);
    return status;
}
#define common_machine_debug_acquire acquire
#define common_machine_debug_execute_with_lease execute
#define common_machine_debug_cancel cancel
#include "x86/debug/command.c"

static x86_debug *debug;
static x86_debug_result result;
static const char *submit(const char *line)
{
    reads = writes = 0u;
    assert(x86_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
    return result.text;
}

int main(void)
{
    const char *invalid[] = {
        "xm ffffffff 0 2", "xm 0 ffffffff 2", "xc ffffffff 0 2",
        "xc 0 ffffffff 2", "xd ffffffff 2", "xe ffffffff 11 22",
        "xf ffffffff 2 11", "xs ffffffff 2 11"
    };
    assert(x86_debug_create(&debug) == LIB_STATUS_OK);
    assert(x86_debug_open(debug, (common_machine *)debug) == LIB_STATUS_OK);

    x86_debug_response response;
    short_response = LIB_TRUE;
    assert(command_execute(debug, &(x86_debug_request){
        .operation = X86_DEBUG_GET_CODE_DEFAULT_SIZE }, &response) != 0);
    assert(debug->access_status == LIB_STATUS_IO_ERROR && response.value == 0u);
    short_response = LIB_FALSE;
    assert(x86_debug_open(debug, (common_machine *)debug) == LIB_STATUS_OK);

    lib_memory_copy(memory, "ABCDE", 5);
    assert(lib_text_compare(submit("xm 0 1 4"), "") == 0);
    assert(lib_memory_compare(memory, "AABCD", 5) == 0 && reads == 4 && writes == 4);
    lib_memory_copy(memory, "ABCDE", 5);
    submit("xm 1 0 4");
    assert(lib_memory_compare(memory, "BCDEE", 5) == 0 && reads == 4 && writes == 4);
    submit("xm 0 0 4"); assert(reads == 0 && writes == 0);
    submit("xm 0 10 0"); assert(reads == 0 && writes == 0);
    submit("xm 0 10 4"); assert(lib_memory_compare(memory, memory + 0x10, 4) == 0);

    lib_memory_set(memory, 0x90, sizeof(memory)); /* One-byte NOPs. */
    submit("xu 0 100"); assert(reads == 256 && debug->unassemble_linear == 256);
    submit("xu 0 0"); assert(reads == 0);
    submit("xu 0"); assert(reads == 16);
    submit("xu"); assert(reads == 10);

    /* Both debugger entry points stop on an overlong instruction, then recover. */
    lib_memory_set(memory, 0x66, 15);
    assert(lib_text_find_substring(submit("u 0 l1"), "<ERROR>") != NULL && reads == 1);
    assert(lib_text_find_substring(submit("xu 0 1"), "<ERROR>") != NULL && reads == 1);
    memory[14] = 0x90;
    assert(lib_text_find_substring(submit("u 0 l1"), "NOP") != NULL);
    assert(debug->unassemble_offset == 15);
    assert(lib_text_find_substring(submit("xu 0 1"), "NOP") != NULL);
    assert(debug->unassemble_linear == 15);

    memory_base = 0x10000u;
    lib_memory_copy(memory, "AAAA", 4);
    assert(lib_text_compare(submit("xs 10000 3 41 41"), "L00010000\nL00010001\n") == 0);
    assert(reads == 2);
    assert(lib_text_compare(submit("xs 10000 1 41 41"), "") == 0 && reads == 0);
    assert(lib_text_compare(submit("xs 10000 0 41"), "") == 0 && reads == 0);
    assert(lib_text_compare(submit("xs 10000 2 41 41"), "L00010000\n") == 0 && reads == 1);

    for (lib_size i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        assert(lib_text_find_substring(submit(invalid[i]), "^ Error") != NULL);
        assert(reads == 0 && writes == 0);
    }
    memory_base = 0xfffff000u;
    lib_memory_set(memory, 0x90, sizeof(memory));
    submit("xu ffffffff 100");
    assert(reads == 1 && debug->unassemble_linear == LIB_UINT32_MAX);
    assert(lib_text_find_substring(result.text, "<ERROR>") == NULL);
    memory[4095] = 0x0f; /* Incomplete instruction cannot be padded into a decode. */
    assert(lib_text_find_substring(submit("xu ffffffff 2"), "<ERROR>") != NULL && reads == 1);
    submit("xd ffffffff 1"); assert(reads == 1 && debug->dump_linear == LIB_UINT32_MAX);
    submit("xe ffffffff 41"); assert(writes == 1 && memory[4095] == 0x41);
    assert(lib_text_compare(submit("xs ffffffff 1 41"), "LFFFFFFFF\n") == 0 && reads == 1);
    submit("xf fffffffe 2 42"); assert(writes == 2);
    submit("xm fffffffe fffffffd 2"); assert(reads == 2 && writes == 2);
    submit("xc fffffffe fffffffe 2"); assert(reads == 4);
    submit("xa ffffffff");
    assert(lib_text_find_substring(submit("mov eax,12345678"), "^ Error") != NULL && writes == 0);
    submit("xa ffffffff"); submit("nop");
    assert(writes == 1 && debug->assemble_linear == LIB_UINT32_MAX);
    assert(lib_text_compare(result.prompt, "-") == 0);

    /* DOS list syntax is fully validated before any E/F/XE/XF write. */
    memory_base = 0u;
    const char *partial[] = { "e 0:0 11 zz", "f 0:0 1 11 zz",
        "xe 0 11 zz", "xf 0 2 11 zz", "f 0 0 11 zz", "xf 0 1 11 zz" };
    for (lib_size i = 0; i < sizeof(partial) / sizeof(partial[0]); ++i) {
        lib_memory_set(memory, 0, sizeof(memory));
        assert(lib_text_find_substring(submit(partial[i]), "^ Error") != NULL);
        assert(writes == 0 && memory[0] == 0 && memory[1] == 0);
    }
    const char *bad_ranges[] = { "m 20 10 30", "c 20 10 30", "s 20 10 41",
        "f 20 10 41", "d 20 10", "u 20 10", "f ffff l2 41",
        "d 1 l0", "u 0 l", "e :10 41", "e 1: 41", "e 1:2:3 41",
        "e 0 \"unterminated", "xe 0 'bad'zz" };
    for (lib_size i = 0; i < sizeof(bad_ranges) / sizeof(bad_ranges[0]); ++i) {
        assert(lib_text_find_substring(submit(bad_ranges[i]), "^ Error"));
        assert(reads == 0 && writes == 0);
    }
    submit("E0:100 \"Ab C\" 21 'D''E' \"\"\"F\"");
    assert(lib_text_compare(result.text, "") == 0);
    assert(lib_memory_compare(memory + 0x100, "Ab C!D'E\"F", 10) == 0 && writes == 10);
    submit("xe 120 \"Ab C\" 21 'D''E' \"\"\"F\"");
    assert(lib_memory_compare(memory + 0x100, memory + 0x120, 10) == 0);
    submit("f 200 l 5 'Ab'"); assert(lib_memory_compare(memory + 0x200, "AbAbA", 5) == 0);
    submit("xf 220 5 'Ab'"); assert(lib_memory_compare(memory + 0x220, "AbAbA", 5) == 0);
    submit("c 200 l5 220"); assert(lib_text_compare(result.text, "") == 0 && reads == 10);
    submit("m 200 l5 201"); assert(lib_memory_compare(memory + 0x200, "AAbAbA", 6) == 0);
    submit("m 201 205 200"); assert(lib_memory_compare(memory + 0x200, "AbAbA", 5) == 0);
    submit("f 300 l4 'A'");
    assert(lib_text_compare(submit("s 300 303 'AA'"),
        "0000:0300  \n0000:0301  \n0000:0302  \n") == 0);
    assert(lib_text_compare(submit("s 300 l1 'AA'"), "") == 0 && reads == 0);
    assert(lib_text_compare(submit("xs 300 1 'AA'"), "") == 0 && reads == 0);
    submit("s 300 l2 'AA'"); assert(reads == 2);
    submit("f 0  ffff 90"); assert(writes == 65536);
    submit("s 0 ffff 41"); assert(reads == 65536);
    submit("f ffff l1 41"); assert(writes == 1 && memory[65535] == 0x41);
    submit("e ffff 'AB'"); assert(memory[65535] == 'A' && memory[0] == 'B');
    submit("d ffff"); assert(reads == 1 && debug->dump_offset == 0);
    submit("d ffff l1"); assert(reads == 1);
    submit("d ffff:10 l1"); assert(reads == 1); /* No artificial 1 MiB clamp. */
    lib_memory_set(memory, 0x90, sizeof(memory));
    submit("u 0 l20"); assert(debug->unassemble_offset == 32);
    submit("u ffff l1"); assert(debug->unassemble_offset == 0);
    memory[65535] = 0xb8; memory[0] = 0x34; memory[1] = 0x12;
    submit("u ffff l1"); /* Whole instruction crosses the segment offset. */
    assert(lib_text_find_substring(result.text, "1234") && debug->unassemble_offset == 2);
    submit("u ffff:10 l1"); assert(lib_text_find_substring(result.text, "<ERROR>") == NULL);
    submit("d 0"); assert(reads == 128);
    submit("u 0"); assert(debug->unassemble_offset >= 32);
    code_segment = 0x100u; data_segment = 0x200u;
    assert(lib_text_compare_n(submit("u 0 0"), "0100:0000", 9) == 0);
    assert(lib_text_compare_n(submit("u 0 l1"), "0100:0000", 9) == 0);
    assert(lib_text_compare_n(submit("d 0 l1"), "0200:0000", 9) == 0);
    submit("e DS:10 41"); assert(memory[0x2010] == 0x41);
    code_segment = data_segment = 0u;
    submit("f 0 l0 42"); assert(writes == 65536);
    submit("m 0 l0 1000:0"); assert(reads == 65536 && writes == 65536);
    assert(memory[0x10000] == 0x42 && memory[0x1ffff] == 0x42);
    submit("c 0 l0 1000:0"); assert(reads == 131072 && lib_text_compare(result.text, "") == 0);
    submit("f 0 l2 41"); submit("m 0 l2 ffff");
    assert(memory[0xffff] == 0x41 && memory[0] == 0x41);
    assert(lib_text_find_substring(submit("s ffff l1 41"), "0000:FFFF"));
    assert(reads == 1);
    assert(lib_text_compare(submit(""), "") == 0 && reads == 0 && writes == 0);
    x86_debug_destroy(debug);
    return 0;
}
