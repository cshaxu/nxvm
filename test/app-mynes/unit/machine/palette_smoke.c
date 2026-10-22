#include "machine/driver.h"
#include "core/machine.h"
#include "lib/types/test.h"

/* Exact RGB delivery includes scenes which formerly exhausted the indexed
 * palette and entered colour quantization. No second palette is defined. */
int main(void)
{
    core_machine *machine = lib_allocate_zero(1u, sizeof(*machine));
    kvm_window_frame_storage *actual = lib_allocate_zero(1u, sizeof(*actual));
    kvm_console_text_frame console = {0};
    lib_bool produced;
    machine_driver driver = { .machine = machine };
    lib_u32 rgb[512];
    lib_u32 random = 1u;
    lib_test_assert(machine != LIB_NULL && actual != LIB_NULL);
    machine->ppu.frame_ready = LIB_TRUE;
    for (lib_u32 sample = 0u; sample < 512u; ++sample) {
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index)
            machine->ppu.completed[index] = (lib_u16)sample;
        ++machine->ppu.frame_revision;
        lib_test_assert(machine_driver_copy_frame(&driver, actual, &console, &produced) == LIB_STATUS_OK);
        lib_test_assert(produced && actual->frame.graphics == actual->data.graphics);
        rgb[sample] = actual->data.graphics->pixels[0];
    }
    for (lib_u32 scene = 0u; scene < 8u; ++scene) {
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index) {
            random = random * 1664525u + 1013904223u;
            machine->ppu.completed[index] = scene == 0u ? 0x20u :
                scene == 1u ? (lib_u16)((index / 8u) & 63u) :
                scene == 2u ? (lib_u16)(index & 511u) :
                scene == 3u ? (lib_u16)(0x800du | ((index & 7u) << 6u)) :
                scene == 4u ? (lib_u16)(511u - (index & 511u)) :
                scene == 5u ? (lib_u16)((index & 255u) | 0xfe00u) :
                scene == 6u ? 0u : (lib_u16)(random >> 16u);
        }
        ++machine->ppu.frame_revision;
        lib_test_assert(machine_driver_copy_frame(&driver, actual, &console, &produced) == LIB_STATUS_OK);
        lib_test_assert(produced && actual->frame.graphics &&
            actual->data.graphics->width == CORE_PPU_WIDTH &&
            actual->data.graphics->height == CORE_PPU_HEIGHT &&
            actual->data.graphics->stride == CORE_PPU_WIDTH);
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index)
            lib_test_assert(actual->data.graphics->pixels[index] ==
                rgb[machine->ppu.completed[index] & 511u]);
        lib_test_assert(machine_driver_copy_frame(&driver, actual, &console, &produced) == LIB_STATUS_OK);
        lib_test_assert(!produced);
    }
    kvm_window_frame_release(actual);
    lib_release(actual);
    lib_release(machine);
    return 0;
}
