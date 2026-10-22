#include "lib/types/test.h"
#include "lib/types/types_interface.h"
static lib_size allocations, releases, bytes_requested;
static lib_bool fail_allocation;
static void *allocate_payload(lib_size count, lib_size size)
{
    bytes_requested = count * size;
    if (fail_allocation) return LIB_NULL;
    void *memory = lib_allocate_zero(count, size);
    lib_test_assert(memory != LIB_NULL);
    ++allocations;
    return memory;
}
static void release_payload(void *memory)
{
    if (memory) ++releases;
    lib_release(memory);
}
#define lib_allocate_zero allocate_payload
#define lib_release release_payload
#include "lib/kvm-console/frame_interface.h"
#undef lib_allocate_zero
#undef lib_release

static void check_storage(void)
{
    kvm_frame_exchange *exchange;
    lib_test_assert(kvm_frame_exchange_create(&exchange, sizeof(kvm_console_text_frame),
        kvm_console_frame_retire) == LIB_STATUS_OK);
    lib_size start = allocations;
    const lib_u32 sizes[][2] = {{80,25}, {80,50}, {132,43}, {100,60}, {1,1}};
    const kvm_console_text_frame *displayed = LIB_NULL;
    for (lib_size size = 0; size < sizeof(sizes)/sizeof(sizes[0]); ++size) {
        kvm_console_text_frame *writer = kvm_frame_exchange_writer(exchange);
        lib_test_assert(writer->characters == LIB_NULL);
        lib_test_assert(kvm_console_frame_prepare(writer, sizes[size][0], sizes[size][1],
            sizes[size][0]+2u) == LIB_STATUS_OK);
        lib_test_assert(bytes_requested == sizeof(kvm_console_character_map) +
            (lib_size)(sizes[size][0]+2u)*sizes[size][1]*sizeof(kvm_text_cell));
        lib_size count = allocations;
        lib_test_assert(kvm_console_frame_prepare(writer, sizes[size][0], sizes[size][1],
            sizes[size][0]+2u) == LIB_STATUS_OK && allocations == count);
        writer->base.cells[0].glyph_index = (lib_u8)size;
        lib_test_assert(kvm_console_frame_commit(exchange) == LIB_STATUS_OK);
        if (displayed) lib_test_assert(displayed->base.cells[0].glyph_index == size-1u);
        displayed = kvm_frame_exchange_take(exchange, LIB_NULL);
        lib_test_assert(displayed != LIB_NULL);
        kvm_frame_exchange_finish(exchange, LIB_TRUE);
        lib_test_assert(allocations-releases == 1u);
    }
    lib_test_assert(allocations == start+5u);
    kvm_console_text_frame *writer = kvm_frame_exchange_writer(exchange);
    fail_allocation = LIB_TRUE;
    lib_test_assert(kvm_console_frame_prepare(writer, 2, 2, 2) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(kvm_console_frame_commit(exchange) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(displayed->base.cells[0].glyph_index == 4u);
    fail_allocation = LIB_FALSE;
    kvm_frame_exchange_destroy(exchange);
    lib_test_assert(allocations == releases);
}
int main(void)
{
    kvm_console_text_frame text = {0};
    lib_test_assert(sizeof(kvm_console_text_frame) == sizeof(text.base) + sizeof(text.characters));
    /* Character presentation has no bitmap font-storage bound. */
    lib_test_assert(kvm_console_frame_prepare(&text, 132, 60, 140) == LIB_STATUS_OK);
    text.base.font_height = 33; text.base.cursor_mode = KVM_CURSOR_HOST;
    text.base.cursor_column = -1; text.base.cursor_row = 500;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    text.base.text_columns = 141;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.base.text_columns = 0u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.base.text_columns = 1u;
    text.characters->primary[255] = 0xd800u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters->primary[255] = 0xd7ffu;
    text.characters->secondary[0] = 0xdfffu;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters->secondary[0] = 0xe000u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    kvm_console_frame_release(&text);
    check_storage();
    return 0;
}
