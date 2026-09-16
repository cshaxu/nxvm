#include "common/debug/debug_interface.h"
#include <assert.h>
#include <string.h>

static lib_u8 memory[0x110000];
static lib_u32 memory_base;
static lib_u16 code_segment, data_segment;
static unsigned reads, writes;
static lib_status acquire(common_machine *m, common_machine_debug_lease *lease)
{ (void)m; *lease = (common_machine_debug_lease){0}; return LIB_STATUS_OK; }
static void cancel(common_machine *m) { (void)m; }
static lib_status execute(common_machine *m, const common_machine_debug_lease *lease,
    const common_machine_debug_request *request, common_machine_debug_result *result)
{
    (void)m; (void)lease;
    *result = (common_machine_debug_result){0};
    if (request->operation == COMMON_MACHINE_DEBUG_READ_REGISTER) {
        if (request->register_id == COMMON_DEBUG_CS) result->value = code_segment;
        if (request->register_id == COMMON_DEBUG_DS) result->value = data_segment;
    } else if (request->operation == COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE) {
        result->value = 1u;
    } else if (request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR ||
               request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR ||
               request->operation == COMMON_MACHINE_DEBUG_READ_REAL ||
               request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL) {
        lib_u32 address = request->operation == COMMON_MACHINE_DEBUG_READ_REAL ||
            request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL ?
            ((lib_u32)request->segment << 4) + request->offset : request->address;
        assert(request->bytes != 0u && request->bytes - 1u <= LIB_UINT32_MAX - address);
        assert(address >= memory_base && address - memory_base < sizeof(memory));
        assert(request->bytes <= sizeof(memory) - (address - memory_base));
        if (request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR ||
            request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL) {
            assert(++writes <= sizeof(memory));
            memcpy(memory + address - memory_base, request->data, request->bytes);
        } else {
            assert(++reads <= sizeof(memory));
            memcpy(result->data, memory + address - memory_base, request->bytes);
        }
    }
    return LIB_STATUS_OK;
}
#define common_machine_debug_acquire acquire
#define common_machine_debug_execute_with_lease execute
#define common_machine_debug_cancel cancel
#include "common/debug/command.c"

static common_debug *debug;
static common_debug_result result;
static const char *submit(const char *line)
{
    reads = writes = 0u;
    assert(common_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
    return result.text;
}

int main(void)
{
    const char *invalid[] = {
        "xm ffffffff 0 2", "xm 0 ffffffff 2", "xc ffffffff 0 2",
        "xc 0 ffffffff 2", "xd ffffffff 2", "xe ffffffff 11 22",
        "xf ffffffff 2 11", "xs ffffffff 2 11"
    };
    assert(common_debug_create(&debug) == LIB_STATUS_OK);
    assert(common_debug_open(debug, (common_machine *)debug) == LIB_STATUS_OK);

    memcpy(memory, "ABCDE", 5);
    assert(strcmp(submit("xm 0 1 4"), "") == 0);
    assert(memcmp(memory, "AABCD", 5) == 0 && reads == 4 && writes == 4);
    memcpy(memory, "ABCDE", 5);
    submit("xm 1 0 4");
    assert(memcmp(memory, "BCDEE", 5) == 0 && reads == 4 && writes == 4);
    submit("xm 0 0 4"); assert(reads == 0 && writes == 0);
    submit("xm 0 10 0"); assert(reads == 0 && writes == 0);
    submit("xm 0 10 4"); assert(memcmp(memory, memory + 0x10, 4) == 0);

    memset(memory, 0x90, sizeof(memory)); /* One-byte NOPs. */
    submit("xu 0 100"); assert(reads == 256 && debug->unassemble_linear == 256);
    submit("xu 0 0"); assert(reads == 0);
    submit("xu 0"); assert(reads == 16);
    submit("xu"); assert(reads == 10);

    memory_base = 0x10000u;
    memcpy(memory, "AAAA", 4);
    assert(strcmp(submit("xs 10000 3 41 41"), "L00010000\nL00010001\n") == 0);
    assert(reads == 2);
    assert(strcmp(submit("xs 10000 1 41 41"), "") == 0 && reads == 0);
    assert(strcmp(submit("xs 10000 0 41"), "") == 0 && reads == 0);
    assert(strcmp(submit("xs 10000 2 41 41"), "L00010000\n") == 0 && reads == 1);

    for (lib_size i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        assert(strstr(submit(invalid[i]), "^ Error") != NULL);
        assert(reads == 0 && writes == 0);
    }
    memory_base = 0xfffff000u;
    memset(memory, 0x90, sizeof(memory));
    submit("xu ffffffff 100");
    assert(reads == 1 && debug->unassemble_linear == LIB_UINT32_MAX);
    assert(strstr(result.text, "<ERROR>") == NULL);
    memory[4095] = 0x0f; /* Incomplete instruction cannot be padded into a decode. */
    assert(strstr(submit("xu ffffffff 2"), "<ERROR>") != NULL && reads == 1);
    submit("xd ffffffff 1"); assert(reads == 1 && debug->dump_linear == LIB_UINT32_MAX);
    submit("xe ffffffff 41"); assert(writes == 1 && memory[4095] == 0x41);
    assert(strcmp(submit("xs ffffffff 1 41"), "LFFFFFFFF\n") == 0 && reads == 1);
    submit("xf fffffffe 2 42"); assert(writes == 2);
    submit("xm fffffffe fffffffd 2"); assert(reads == 2 && writes == 2);
    submit("xc fffffffe fffffffe 2"); assert(reads == 4);
    submit("xa ffffffff");
    assert(strstr(submit("mov eax,12345678"), "^ Error") != NULL && writes == 0);
    submit("xa ffffffff"); submit("nop");
    assert(writes == 1 && debug->assemble_linear == LIB_UINT32_MAX);
    assert(strcmp(result.prompt, "-") == 0);

    /* DOS list syntax is fully validated before any E/F/XE/XF write. */
    memory_base = 0u;
    const char *partial[] = { "e 0:0 11 zz", "f 0:0 1 11 zz",
        "xe 0 11 zz", "xf 0 2 11 zz", "f 0 0 11 zz", "xf 0 1 11 zz" };
    for (lib_size i = 0; i < sizeof(partial) / sizeof(partial[0]); ++i) {
        memset(memory, 0, sizeof(memory));
        assert(strstr(submit(partial[i]), "^ Error") != NULL);
        assert(writes == 0 && memory[0] == 0 && memory[1] == 0);
    }
    const char *bad_ranges[] = { "m 20 10 30", "c 20 10 30", "s 20 10 41",
        "f 20 10 41", "d 20 10", "u 20 10", "f ffff l2 41",
        "d 1 l0", "u 0 l", "e :10 41", "e 1: 41", "e 1:2:3 41",
        "e 0 \"unterminated", "xe 0 'bad'zz" };
    for (lib_size i = 0; i < sizeof(bad_ranges) / sizeof(bad_ranges[0]); ++i) {
        assert(strstr(submit(bad_ranges[i]), "^ Error"));
        assert(reads == 0 && writes == 0);
    }
    submit("E0:100 \"Ab C\" 21 'D''E' \"\"\"F\"");
    assert(strcmp(result.text, "") == 0);
    assert(memcmp(memory + 0x100, "Ab C!D'E\"F", 10) == 0 && writes == 10);
    submit("xe 120 \"Ab C\" 21 'D''E' \"\"\"F\"");
    assert(memcmp(memory + 0x100, memory + 0x120, 10) == 0);
    submit("f 200 l 5 'Ab'"); assert(memcmp(memory + 0x200, "AbAbA", 5) == 0);
    submit("xf 220 5 'Ab'"); assert(memcmp(memory + 0x220, "AbAbA", 5) == 0);
    submit("c 200 l5 220"); assert(strcmp(result.text, "") == 0 && reads == 10);
    submit("m 200 l5 201"); assert(memcmp(memory + 0x200, "AAbAbA", 6) == 0);
    submit("m 201 205 200"); assert(memcmp(memory + 0x200, "AbAbA", 5) == 0);
    submit("f 300 l4 'A'");
    assert(strcmp(submit("s 300 303 'AA'"),
        "0000:0300  \n0000:0301  \n0000:0302  \n") == 0);
    assert(strcmp(submit("s 300 l1 'AA'"), "") == 0 && reads == 0);
    assert(strcmp(submit("xs 300 1 'AA'"), "") == 0 && reads == 0);
    submit("s 300 l2 'AA'"); assert(reads == 2);
    submit("f 0  ffff 90"); assert(writes == 65536);
    submit("s 0 ffff 41"); assert(reads == 65536);
    submit("f ffff l1 41"); assert(writes == 1 && memory[65535] == 0x41);
    submit("e ffff 'AB'"); assert(memory[65535] == 'A' && memory[0] == 'B');
    submit("d ffff"); assert(reads == 1 && debug->dump_offset == 0);
    submit("d ffff l1"); assert(reads == 1);
    submit("d ffff:10 l1"); assert(reads == 1); /* No artificial 1 MiB clamp. */
    memset(memory, 0x90, sizeof(memory));
    submit("u 0 l20"); assert(debug->unassemble_offset == 32);
    submit("u ffff l1"); assert(debug->unassemble_offset == 0);
    memory[65535] = 0xb8; memory[0] = 0x34; memory[1] = 0x12;
    submit("u ffff l1"); /* Whole instruction crosses the segment offset. */
    assert(strstr(result.text, "1234") && debug->unassemble_offset == 2);
    submit("u ffff:10 l1"); assert(strstr(result.text, "<ERROR>") == NULL);
    submit("d 0"); assert(reads == 128);
    submit("u 0"); assert(debug->unassemble_offset >= 32);
    code_segment = 0x100u; data_segment = 0x200u;
    assert(strncmp(submit("u 0 0"), "0100:0000", 9) == 0);
    assert(strncmp(submit("u 0 l1"), "0100:0000", 9) == 0);
    assert(strncmp(submit("d 0 l1"), "0200:0000", 9) == 0);
    submit("e DS:10 41"); assert(memory[0x2010] == 0x41);
    code_segment = data_segment = 0u;
    submit("f 0 l0 42"); assert(writes == 65536);
    submit("m 0 l0 1000:0"); assert(reads == 65536 && writes == 65536);
    assert(memory[0x10000] == 0x42 && memory[0x1ffff] == 0x42);
    submit("c 0 l0 1000:0"); assert(reads == 131072 && strcmp(result.text, "") == 0);
    submit("f 0 l2 41"); submit("m 0 l2 ffff");
    assert(memory[0xffff] == 0x41 && memory[0] == 0x41);
    assert(strstr(submit("s ffff l1 41"), "0000:FFFF"));
    assert(reads == 1);
    assert(strcmp(submit(""), "") == 0 && reads == 0 && writes == 0);
    common_debug_destroy(debug);
    return 0;
}
