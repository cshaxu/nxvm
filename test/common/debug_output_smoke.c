#include "common/debug/debug_interface.h"
#include <assert.h>
#include <string.h>

static lib_bool fail_allocation;
static void *grow(void *memory, lib_size bytes)
{ return fail_allocation ? NULL : lib_reallocate(memory, bytes); }
static void cancel(common_machine *machine) { (void)machine; }
#define lib_reallocate grow
#define common_machine_debug_cancel cancel
#include "common/debug/command.c"

int main(void)
{
    common_debug *command;
    common_debug_result result;
    assert(common_debug_create(&command) == LIB_STATUS_OK);
    assert(common_debug_open(command, (common_machine *)command) == LIB_STATUS_OK);
    /* These original CLI operations do not access a machine at all. */
    fail_allocation = LIB_TRUE;
    assert(common_debug_submit_line(command, "?", &result) == LIB_STATUS_NO_MEMORY);
    fail_allocation = LIB_FALSE;
    assert(common_debug_submit_line(command, "?", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "assemble") && strcmp(result.prompt, "-") == 0);
    assert(common_debug_submit_line(command, "x?", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "XA [address]"));
    assert(common_debug_submit_line(command, "v", &result) == LIB_STATUS_OK);
    assert(strcmp(result.text, "") == 0 && strcmp(result.prompt, ":") == 0);
    assert(common_debug_submit_line(command, "Hello", &result) == LIB_STATUS_OK);
    assert(strcmp(result.text, "48 65 6C 6C 6F \n") == 0);
    /* Fault after some output is already accepted must still return failure. */
    command_begin_output(command, &result);
    assert(command_printf(command, "prefix") == 6);
    fail_allocation = LIB_TRUE;
    assert(command_printf(command, "%10000s", "tail") == -1);
    assert(command_end_output(command) == LIB_STATUS_NO_MEMORY);
    fail_allocation = LIB_FALSE;
    assert(common_debug_submit_line(command, "", &result) == LIB_STATUS_OK);
    assert(strcmp(result.text, "") == 0 && strcmp(result.prompt, "-") == 0);
    common_debug_destroy(command);
    return 0;
}
