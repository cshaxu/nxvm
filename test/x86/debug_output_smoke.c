#include "x86/debug/debug_interface.h"
#include <assert.h>

static lib_bool fail_allocation;
static lib_u32 allocations;
static void *allocate(lib_size count, lib_size size)
{ ++allocations; return lib_allocate_zero(count, size); }
static void *grow(void *memory, lib_size bytes)
{ return fail_allocation ? NULL : lib_reallocate(memory, bytes); }
static void cancel(common_machine *machine) { (void)machine; }
#define lib_reallocate grow
#define lib_allocate_zero allocate
#define common_machine_debug_cancel cancel
#include "x86/debug/command.c"

int main(void)
{
    x86_debug *command;
    x86_debug_result result;
    assert(x86_debug_create(&command) == LIB_STATUS_OK);
    assert(x86_debug_open(command, (common_machine *)command) == LIB_STATUS_OK);
    assert(allocations == 1u); /* Only the debugger itself, not an argument table. */
    /* These original CLI operations do not access a machine at all. */
    fail_allocation = LIB_TRUE;
    assert(x86_debug_submit_line(command, "?", &result) == LIB_STATUS_NO_MEMORY);
    fail_allocation = LIB_FALSE;
    assert(x86_debug_submit_line(command, "?", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "assemble") && lib_text_compare(result.prompt, "-") == 0);
    assert(x86_debug_submit_line(command, "x?", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "XA [address]"));
    assert(x86_debug_submit_line(command, "v", &result) == LIB_STATUS_OK);
    assert(lib_text_compare(result.text, "") == 0 && lib_text_compare(result.prompt, ":") == 0);
    assert(x86_debug_submit_line(command, "Hello", &result) == LIB_STATUS_OK);
    assert(lib_text_compare(result.text, "48 65 6C 6C 6F \n") == 0);
    /* Fault after some output is already accepted must still return failure. */
    command_begin_output(command, &result);
    assert(command_printf(command, "prefix") == 6);
    fail_allocation = LIB_TRUE;
    assert(command_printf(command, "%10000s", "tail") == -1);
    assert(command_end_output(command) == LIB_STATUS_NO_MEMORY);
    fail_allocation = LIB_FALSE;
    assert(x86_debug_submit_line(command, "", &result) == LIB_STATUS_OK);
    assert(lib_text_compare(result.text, "") == 0 && lib_text_compare(result.prompt, "-") == 0);
    /* A maximum-length line cannot fill the 256-pointer table, including the
       split command prefix and trailing sentinel. Parsing stays original. */
    command->command_buffer[0] = 'x';
    command->command_buffer[1] = '0';
    for (lib_u32 i = 2u; i < sizeof(command->command_buffer) - 1u; ++i)
        command->command_buffer[i] = i % 2u ? 'a' : ' ';
    command->command_buffer[255] = '\0';
    parse(command);
    assert(command->argument_count == 128u && command->arguments[128] == NULL);
    assert(sizeof(command->arguments) == DEBUG_MAXNARG * sizeof(char *));
    assert(x86_debug_submit_line(command, "v", &result) == LIB_STATUS_OK);
    x86_debug_close(command);
    x86_debug_close(command);
    assert(x86_debug_submit_line(command, "?", &result) == LIB_STATUS_INVALID_ARGUMENT);
    assert(x86_debug_open(command, (common_machine *)command) == LIB_STATUS_OK);
    assert(allocations == 1u && command->argument_count == 0u);
    for (lib_u32 i = 0u; i < DEBUG_MAXNARG; ++i) assert(command->arguments[i] == NULL);
    assert(x86_debug_submit_line(command, "?", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "assemble") && lib_text_compare(result.prompt, "-") == 0);
    x86_debug_destroy(command);
    return 0;
}
