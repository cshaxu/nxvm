#include "lib/types/test.h"
#include "lib/kvm-base/frame_exchange_interface.h"
#include "lib/base/sync_interface.h"

static lib_size allocations, releases, last_bytes, live;
static const void *held_payload[2];
static lib_bool fail_allocation;
static void *allocate_payload(lib_size count, lib_size bytes)
{
    last_bytes = count * bytes;
    if (fail_allocation) return LIB_NULL;
    void *result = lib_allocate_zero(count, bytes);
    lib_test_assert(result != LIB_NULL);
    ++allocations; ++live;
    return result;
}
static void release_payload(void *pointer)
{
    if (pointer != LIB_NULL) {
        lib_test_assert(pointer != held_payload[0] && pointer != held_payload[1]);
        ++releases; --live;
    }
    lib_release(pointer);
}
/* Observe real leaf allocations, not a test-only production API. Exchange
 * metadata remains separately allocated by its normal implementation. */
#define lib_allocate_zero allocate_payload
#define lib_release release_payload
#include "lib/kvm-window/frame_interface.h"
#undef lib_allocate_zero
#undef lib_release

static void check_copy(void)
{
    kvm_window_frame_storage source = {0}, copy = {0};
    lib_test_assert(kvm_window_frame_validate(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    const lib_u32 sizes[][2] = {{80,25}, {80,50}, {132,43}, {100,60}, {193,77}};
    for (lib_size i = 0u; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        lib_u32 columns = sizes[i][0], rows = sizes[i][1], stride = columns + 3u;
        lib_test_assert(kvm_window_frame_prepare_text(&source, columns, rows, stride) == LIB_STATUS_OK);
        lib_test_assert(last_bytes == sizeof(kvm_window_text_frame) + (lib_size)stride * rows * sizeof(kvm_text_cell));
        source.frame.sequence = (lib_u32)i + 1u;
        source.data.text->base.cells[(rows - 1u) * stride + columns - 1u] =
            (kvm_text_cell){65, 1, 3, 4, 1};
        source.data.text->secondary_font[65 * KVM_WINDOW_FONT_HEIGHT] = 0xa5;
        source.data.text->base.cursor[1] = (kvm_text_cursor_span){12, 2};
        lib_test_assert(kvm_window_frame_copy(&copy, &source.frame));
        lib_test_assert(copy.frame.sequence == source.frame.sequence &&
            copy.data.text != source.data.text && copy.data.text->base.cells != source.data.text->base.cells);
        lib_test_assert(lib_memory_compare(copy.data.text->base.cells, source.data.text->base.cells,
            (lib_size)stride * rows * sizeof(kvm_text_cell)) == 0);
        lib_test_assert(copy.data.text->secondary_font[65 * KVM_WINDOW_FONT_HEIGHT] == 0xa5 &&
            copy.data.text->base.cursor[1].height == 2);
        lib_size before = allocations;
        lib_test_assert(kvm_window_frame_prepare_text(&source, columns, rows, stride) == LIB_STATUS_OK);
        lib_test_assert(kvm_window_frame_copy(&copy, &source.frame) && allocations == before);
        source.data.text->base.capacity--;
        lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
        source.data.text->base.capacity++;
        source.data.text->base.font_height = KVM_WINDOW_FONT_HEIGHT + 1u;
        lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_UNSUPPORTED);
        source.data.text->base.font_height = 0;
    }
    lib_test_assert(kvm_window_frame_prepare_graphics(&source, 3, 5, 7) == LIB_STATUS_OK);
    source.frame.text = (const kvm_window_text_frame *)source.data.graphics;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    source.frame.text = LIB_NULL;
    --source.data.graphics->capacity;
    lib_test_assert(kvm_window_frame_validate(&source.frame) == LIB_STATUS_INVALID_ARGUMENT);
    ++source.data.graphics->capacity;
    lib_test_assert(last_bytes == sizeof(kvm_window_graphics_frame) + 35 * sizeof(lib_u32));
    for (lib_size i = 0; i < 35; ++i) source.data.graphics->pixels[i] = (lib_u32)i;
    lib_test_assert(kvm_window_frame_copy(&copy, &source.frame));
    lib_test_assert(copy.data.graphics->pixels != source.data.graphics->pixels &&
        lib_memory_compare(copy.data.graphics->pixels, source.data.graphics->pixels, 35*sizeof(lib_u32)) == 0);
    lib_test_assert(kvm_window_frame_copy(&source, &source.frame));
    source.data.graphics->pixels[0] = 0xff000000u;
    lib_test_assert(!kvm_window_frame_copy(&copy, &source.frame));
    lib_test_assert(copy.data.graphics->pixels[0] == 0);
    lib_test_assert(kvm_window_frame_prepare_graphics(&source, 0, 1, 1) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_prepare_graphics(&source, 5, 1, 4) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_prepare_graphics(&source, LIB_UINT32_MAX,
        LIB_UINT32_MAX, LIB_UINT32_MAX) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(kvm_window_frame_prepare_text(&source, LIB_UINT32_MAX,
        LIB_UINT32_MAX, LIB_UINT32_MAX) == LIB_STATUS_INVALID_ARGUMENT);
    kvm_window_frame_release(&source);
    kvm_window_frame_release(&copy);
}

static void replace_frames(void *opaque, const base_sync_task *task)
{
    kvm_frame_exchange *frames = opaque;
    (void)task;
    for (lib_u32 i = 0; i < 20; ++i) {
        kvm_window_frame_storage *writer = kvm_frame_exchange_writer(frames);
        if (i % 2u == 0u) {
            lib_test_assert(kvm_window_frame_prepare_text(writer, 80+i,25,100) == LIB_STATUS_OK);
        } else {
            lib_test_assert(kvm_window_frame_prepare_graphics(writer, 31+i,17,64) == LIB_STATUS_OK);
            writer->data.graphics->pixels[0] = i;
        }
        lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
        lib_test_assert(live <= 4u);
    }
}

static void check_exchange(void)
{
    kvm_frame_exchange *frames = LIB_NULL;
    lib_size before = allocations;
    lib_test_assert(kvm_frame_exchange_create(&frames, sizeof(kvm_window_frame_storage),
        kvm_window_frame_retire) == LIB_STATUS_OK && allocations == before);
    kvm_window_frame_storage *writer = kvm_frame_exchange_writer(frames);
    lib_test_assert(writer->frame.text == LIB_NULL && writer->frame.graphics == LIB_NULL);
    lib_test_assert(kvm_window_frame_prepare_text(writer, 80,25,80) == LIB_STATUS_OK);
    lib_test_assert(allocations == before + 1);
    writer->data.text->base.cells[0].glyph_index = 'A';
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
    const kvm_window_frame *old = kvm_frame_exchange_take(frames, LIB_NULL);
    kvm_frame_exchange_finish(frames, LIB_TRUE);
    writer = kvm_frame_exchange_writer(frames);
    lib_test_assert(kvm_window_frame_prepare_text(writer, 80,25,80) == LIB_STATUS_OK);
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
    const void *previous;
    const kvm_window_frame *processing = kvm_frame_exchange_take(frames, &previous);
    lib_test_assert(previous == old && processing != old);
    base_sync_task *producer;
    held_payload[0] = old->text; held_payload[1] = processing->text;
    lib_test_assert(base_sync_task_create(replace_frames, frames, &producer) == LIB_STATUS_OK);
    /* The consumer owns both old payloads throughout concurrent replacement.
     * Only the producer mutates allocation counters until join establishes
     * quiescence; no sleep or unsynchronized counter observation is needed. */
    lib_test_assert(old->text->base.cells[0].glyph_index == 'A' && processing->text);
    lib_test_assert(base_sync_task_destroy(producer) == LIB_STATUS_OK);
    lib_test_assert(old->text->base.cells[0].glyph_index == 'A' && processing->text);
    held_payload[0] = held_payload[1] = LIB_NULL;
    kvm_frame_exchange_finish(frames, LIB_TRUE);
    lib_test_assert(live == 2); /* Old text freed; mismatched writer is empty. */
    const kvm_window_frame *latest = kvm_frame_exchange_take(frames, &previous);
    lib_test_assert(latest->graphics->pixels[0] == 19 && previous == processing);
    kvm_frame_exchange_finish(frames, LIB_TRUE);
    lib_test_assert(live == 1); /* Only the newest graphics payload remains. */
    kvm_frame_exchange_redraw(frames);
    lib_test_assert(kvm_frame_exchange_take(frames, LIB_NULL) == latest);
    held_payload[0] = latest->graphics;
    lib_test_assert(base_sync_task_create(replace_frames, frames, &producer) == LIB_STATUS_OK);
    lib_test_assert(latest->graphics->pixels[0] == 19);
    lib_test_assert(base_sync_task_destroy(producer) == LIB_STATUS_OK);
    lib_test_assert(latest->graphics->pixels[0] == 19);
    held_payload[0] = LIB_NULL;
    kvm_frame_exchange_finish(frames, LIB_TRUE);
    latest = kvm_frame_exchange_take(frames, LIB_NULL);
    lib_test_assert(latest != LIB_NULL && latest->graphics->pixels[0] == 19);
    kvm_frame_exchange_finish(frames, LIB_TRUE);
    fail_allocation = LIB_TRUE;
    writer = kvm_frame_exchange_writer(frames);
    lib_test_assert(kvm_window_frame_prepare_text(writer,132,43,132) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(latest->graphics->pixels[0] == 19);
    fail_allocation = LIB_FALSE;
    for (lib_u32 i = 0; i < 100; ++i) {
        writer = kvm_frame_exchange_writer(frames);
        lib_test_assert(kvm_window_frame_prepare_text(writer,100+i%2,60,101) == LIB_STATUS_OK);
        lib_test_assert(kvm_window_frame_commit(frames) == LIB_STATUS_OK);
        lib_test_assert(kvm_frame_exchange_take(frames, LIB_NULL) != LIB_NULL);
        kvm_frame_exchange_finish(frames, LIB_TRUE);
        lib_test_assert(live <= 2);
    }
    kvm_frame_exchange_destroy(frames);
}

int main(void)
{
    check_copy();
    lib_test_assert(live == 0 && allocations == releases);
    check_exchange();
    lib_test_assert(live == 0 && allocations == releases);
    return 0;
}
