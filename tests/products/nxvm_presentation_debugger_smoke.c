#include <stdio.h>

#include "vm/product/session_debugger.h"
#include "vm/product/presentation.h"

typedef struct input_fixture {
    uint16_t values[2];
    unsigned count;
} input_fixture;

static ntvdm64_status consume_input(void *context, uint16_t scan_code)
{
    input_fixture *fixture = (input_fixture *)context;
    if (fixture->count == 2u) return NTVDM64_STATUS_NO_MEMORY;
    fixture->values[fixture->count++] = scan_code;
    return NTVDM64_STATUS_OK;
}

int main(void)
{
    vm_product_presentation presentation;
    core_machine_text_snapshot guest_text;
    core_machine_text_snapshot captured_text;
    input_fixture input = { { 0u, 0u }, 0u };
    core_machine *machine = NULL;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL, 0u
    };
    vm_product_debugger debugger;
    core_machine_cpu_state cpu;
    core_machine_run_result result;

    vm_product_presentation_initialize(&presentation);
    core_machine_text_snapshot_initialize(&guest_text);
    if (vm_product_presentation_enqueue_input(&presentation, 0x1eu) != NTVDM64_STATUS_OK ||
        vm_product_presentation_enqueue_input(&presentation, 0x30u) != NTVDM64_STATUS_OK ||
        vm_product_presentation_apply_input(&presentation, consume_input,
            &input) != NTVDM64_STATUS_INVALID_STATE ||
        vm_product_presentation_open_command_boundary(&presentation) != NTVDM64_STATUS_OK ||
        vm_product_presentation_apply_input(&presentation, consume_input,
            &input) != NTVDM64_STATUS_OK || input.count != 2u ||
        input.values[0] != 0x1eu || input.values[1] != 0x30u ||
        core_machine_text_snapshot_write(&guest_text, 0u, 0u, 'A', 7u) != NTVDM64_STATUS_OK ||
        vm_product_presentation_publish_text(&presentation,
            &guest_text) != NTVDM64_STATUS_OK ||
        vm_product_presentation_capture_text(&presentation,
            &captured_text) != NTVDM64_STATUS_OK || captured_text.characters[0] != 'A') return 1;
    vm_product_presentation_close_command_boundary(&presentation);
    if (vm_product_presentation_publish_text(&presentation,
            &guest_text) != NTVDM64_STATUS_INVALID_STATE ||
        core_machine_create(&config, &machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(machine) != NTVDM64_STATUS_OK ||
        vm_product_debugger_initialize(&debugger, machine) != NTVDM64_STATUS_OK ||
        vm_product_debugger_read_cpu(&debugger, &cpu) != NTVDM64_STATUS_INVALID_STATE ||
        vm_product_debugger_open_command_boundary(&debugger) != NTVDM64_STATUS_OK ||
        vm_product_debugger_read_cpu(&debugger, &cpu) != NTVDM64_STATUS_OK ||
        cpu.cs != 0xf000u ||
        vm_product_debugger_step(&debugger, &result) != NTVDM64_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }
    vm_product_debugger_close_command_boundary(&debugger);
    core_machine_destroy(machine);
    puts("M5:T6:S2:NXVM-PRESENTATION-DEBUG:OK");
    return 0;
}
