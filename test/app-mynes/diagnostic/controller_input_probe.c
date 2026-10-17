#include "machine/driver.h"
#include "core/machine.h"
#include "lib/kvm-base/input_interface.h"
#include "lib/types/file.h"

/* Explicit owner-ROM diagnostic, never an automatic CTest. No native window,
 * pacing, audio endpoint or saved-media mutation is involved. The bounded
 * output contains controller observations, not cartridge bytes. */
static lib_bool deliver(void *context, const kvm_input_event *event)
{
    machine_driver_deliver_input(context, event);
    return LIB_TRUE;
}

static lib_status probe(const char *path, lib_bool character)
{
    machine_driver *driver = LIB_NULL;
    lib_status status = machine_driver_create(&driver,
        &(machine_driver_options){0});
    lib_u32 latches = 0u, nonzero = 0u, close_pairs = 0u;
    lib_u32 reads = 0u, writes = 0u;
    lib_u64 previous_cycle = 0u;
    lib_bool injected = LIB_FALSE, released = LIB_FALSE;
    kvm_input_event key = {.type = KVM_EVENT_KEY, .source_identity = 1u,
        .data.key = {.key = LIB_KEY_ENTER, .pressed = LIB_TRUE}};

    if (status != LIB_STATUS_OK) return status;
    if (!machine_driver_set_media(driver, path, LIB_STORAGE_MEDIUM_READONLY)) {
        status = LIB_STATUS_IO_ERROR;
        goto done;
    }
    lib_c_printf("mode=%s\n", character ? "character-pulse" : "held-enter");
    for (lib_u32 step = 0u; step < 10000000u; ++step) {
        core_machine *machine = driver->machine;
        core_run_result result;
        lib_bool strobe = machine->controller.strobe;

        if (!injected && machine->ppu.frame_revision >= 180u) {
            if (character) {
                base_keyboard_decoder normalizer = {0};
                const base_keyboard_record record = {
                    .kind = BASE_KEYBOARD_CHARACTER, .utf16 = '\r',
                    .repeat_count = 1u};
                if (kvm_keyboard_submit_record(&normalizer, LIB_NULL,
                        driver, deliver, &record) != BASE_KEYBOARD_ACCEPTED) {
                    status = LIB_STATUS_INTERNAL_ERROR;
                    goto done;
                }
            } else machine_driver_deliver_input(driver, &key);
            injected = LIB_TRUE;
        }
        if (!character && !released && machine->ppu.frame_revision >= 190u) {
            key.data.key.pressed = LIB_FALSE;
            machine_driver_deliver_input(driver, &key);
            released = LIB_TRUE;
        }
        status = core_machine_run(machine, 1u, 32u, &result);
        if (status != LIB_STATUS_OK || machine->trap.trap_valid) {
            if (status == LIB_STATUS_OK) status = LIB_STATUS_UNSUPPORTED;
            goto done;
        }
        for (lib_u32 i = 0u; i < machine->trace_count; ++i) {
            const core_bus_transfer *transfer = &machine->trace[i];
            if (injected && transfer->kind == CORE_BUS_TRANSFER_READ &&
                transfer->address == 0x4016u) ++reads;
            if (reads >= 16u && machine->ppu.frame_revision == 180u &&
                transfer->kind == CORE_BUS_TRANSFER_WRITE &&
                transfer->address < 0x100u && writes < 24u) {
                lib_c_printf("post-poll pc=%04x ram=%02x value=%02x\n",
                    machine->pc, transfer->address, transfer->value);
                ++writes;
            }
            if (transfer->kind != CORE_BUS_TRANSFER_WRITE ||
                transfer->address != 0x4016u) continue;
            if (strobe && (transfer->value & 1u) == 0u && injected) {
                lib_u64 gap = machine->cycles - previous_cycle;
                if (latches != 0u && gap < 1000u) ++close_pairs;
                if (machine->controller.captured != 0u) ++nonzero;
                if (latches < 12u)
                    lib_c_printf("latch=%u frame=%u gap_cycles=%llu buttons=%02x\n",
                        latches, machine->ppu.frame_revision,
                        (unsigned long long)gap, machine->controller.captured);
                previous_cycle = machine->cycles;
                ++latches;
            }
            strobe = (transfer->value & 1u) != 0u;
        }
        if (machine->ppu.frame_revision >= 200u) {
            lib_c_printf("latches=%u nonzero=%u close_pairs=%u\n",
                latches, nonzero, close_pairs);
            status = latches == 0u ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK;
            goto done;
        }
    }
    status = LIB_STATUS_LIMIT_EXCEEDED;
done:
    {
        lib_status cleanup = machine_driver_destroy(driver);
        if (status == LIB_STATUS_OK) status = cleanup;
    }
    return status;
}

int main(int argc, char **argv)
{
    lib_status status;
    if (argc != 2) return 2;
    status = probe(argv[1], LIB_FALSE);
    if (status == LIB_STATUS_OK) status = probe(argv[1], LIB_TRUE);
    if (status != LIB_STATUS_OK) lib_c_printf("probe_status=%u\n", status);
    return status == LIB_STATUS_OK ? 0 : 1;
}
