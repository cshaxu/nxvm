#include "core/driver.h"
#include "core/machine.h"
#include "lib/types/test.h"

/* Code-owned frames only. The reference retains raster-order RGB deduplication,
 * independently of the production sample-index shortcut. */
static void reference_frame(const core_machine *machine, const lib_u32 *rgb,
    emulator_machine_frame *frame)
{
    lib_u32 count = 0u;
    lib_bool cube = LIB_FALSE;
    lib_memory_set(frame->window.image.palette, 0, sizeof(frame->window.image.palette));
    for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index) {
        lib_u32 color = rgb[machine->ppu.completed[index] & 511u];
        lib_u32 candidate;
        for (candidate = 0u; candidate < count; ++candidate)
            if (frame->window.image.palette[candidate] == color) break;
        if (candidate == count) {
            if (count == 256u) { cube = LIB_TRUE; break; }
            frame->window.image.palette[count++] = color;
        }
        frame->window.image.pixels[index] = (lib_u8)candidate;
    }
    if (!cube) return;
    for (lib_u32 index = 0u; index < 256u; ++index)
        frame->window.image.palette[index] = ((index / 36u) * 51u << 16u) |
            (((index / 6u) % 6u) * 51u << 8u) | (index % 6u) * 51u;
    for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index) {
        lib_u32 color = rgb[machine->ppu.completed[index] & 511u];
        frame->window.image.pixels[index] = (lib_u8)(
            36u * (((color >> 16u) + 25u) / 51u) +
            6u * ((((color >> 8u) & 255u) + 25u) / 51u) +
            ((color & 255u) + 25u) / 51u);
    }
}

int main(void)
{
    core_machine *machine = lib_allocate_zero(1u, sizeof(*machine));
    emulator_machine_frame *actual = lib_allocate_zero(1u, sizeof(*actual));
    emulator_machine_frame *expected = lib_allocate_zero(1u, sizeof(*expected));
    core_driver driver = { .machine = machine };
    lib_u32 rgb[512];
    lib_u32 random = 1u;
    lib_test_assert(machine != LIB_NULL && actual != LIB_NULL && expected != LIB_NULL);
    machine->ppu.frame_ready = LIB_TRUE;
    /* Learn each unchanged RGB value through the existing production contract;
     * no second palette definition or public test helper is introduced. */
    for (lib_u32 sample = 0u; sample < 512u; ++sample) {
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index)
            machine->ppu.completed[index] = (lib_u16)sample;
        ++machine->ppu.frame_revision;
        lib_test_assert(core_driver_copy_frame(&driver, actual) == LIB_STATUS_OK);
        rgb[sample] = actual->window.image.palette[0];
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
        lib_test_assert(core_driver_copy_frame(&driver, actual) == LIB_STATUS_OK);
        lib_test_assert(actual->window.valid && actual->window.graphics);
        reference_frame(machine, rgb, expected);
        for (lib_u32 index = 0u; index < 256u; ++index)
            lib_test_assert(actual->window.image.palette[index] == expected->window.image.palette[index]);
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index)
            lib_test_assert(actual->window.image.pixels[index] == expected->window.image.pixels[index]);
        lib_test_assert(core_driver_copy_frame(&driver, actual) == LIB_STATUS_OK);
        lib_test_assert(!actual->window.valid);
    }
    lib_release(expected); lib_release(actual); lib_release(machine);
    return 0;
}
