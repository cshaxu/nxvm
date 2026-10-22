#ifndef KVM_WINDOW_FRAME_INTERFACE_H
#define KVM_WINDOW_FRAME_INTERFACE_H

#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/frame_exchange_interface.h"

#define KVM_WINDOW_FONT_HEIGHT 32u
#define KVM_WINDOW_FONT_GLYPHS 256u

typedef struct kvm_window_text_frame {
    kvm_text_frame base;
    /* Fixed slot stride; active height (zero defaults to 16) is in base. */
    lib_u8 font[KVM_WINDOW_FONT_GLYPHS * KVM_WINDOW_FONT_HEIGHT];
    lib_u8 secondary_font[KVM_WINDOW_FONT_GLYPHS * KVM_WINDOW_FONT_HEIGHT];
} kvm_window_text_frame;

typedef struct kvm_window_graphics_frame {
    lib_u32 width;
    lib_u32 height;
    lib_u32 stride; /* Pixels, including uninterpreted row padding. */
    /* Complete image, numeric 0x00RRGGBB (no alpha); visible high bytes must
     * be zero. Damage is derived by the receiving Window. */
    lib_size capacity; /* Pixels available at pixels. */
    lib_u32 *pixels;
} kvm_window_graphics_frame;

/* Exactly one payload pointer is non-null. Published payloads remain immutable
 * for the complete lifetime of their exchange slot. */
typedef struct kvm_window_frame {
    lib_u32 sequence;
    const kvm_window_text_frame *text;
    const kvm_window_graphics_frame *graphics;
} kvm_window_frame;

/* Producer-owned backing for one reusable slot, not another transport schema.
 * Only the active payload occupies storage. Never shallow-copy this object. */
typedef struct kvm_window_frame_storage {
    kvm_window_frame frame;
    union {
        kvm_window_text_frame *text;
        kvm_window_graphics_frame *graphics;
    } data;
} kvm_window_frame_storage;

static inline void kvm_window_frame_release(kvm_window_frame_storage *storage)
{
    lib_release(storage->data.text); /* Each variant is one owned allocation. */
    *storage = (kvm_window_frame_storage){0};
}

static inline lib_bool kvm_window_frame_same_spec(const kvm_window_frame *a,
    const kvm_window_frame *b)
{
    if (a->text && b->text)
        return a->text->base.text_columns == b->text->base.text_columns &&
            a->text->base.text_rows == b->text->base.text_rows &&
            a->text->base.stride == b->text->base.stride;
    return a->graphics && b->graphics &&
        a->graphics->width == b->graphics->width &&
        a->graphics->height == b->graphics->height &&
        a->graphics->stride == b->graphics->stride;
}

static inline void kvm_window_frame_retire(void *slot, const void *latest)
{
    kvm_window_frame_storage *storage = slot;
    if (latest == LIB_NULL || !kvm_window_frame_same_spec(&storage->frame, latest))
        kvm_window_frame_release(storage);
}

/* Only call on an unheld writer, initially zero-initialized. Equal
 * specifications reuse their allocation. Invalid arguments leave it unchanged;
 * allocation failure empties the writer, never a held publication. */
static inline lib_status kvm_window_frame_prepare_text(
    kvm_window_frame_storage *storage, lib_u32 columns, lib_u32 rows, lib_u32 stride)
{
    lib_size bytes;
    kvm_window_text_frame *text;
    if (storage == LIB_NULL || kvm_frame_extent(columns, rows, stride,
            sizeof(kvm_text_cell), &bytes) != LIB_STATUS_OK ||
        bytes > LIB_SIZE_MAX - sizeof(*text)) return LIB_STATUS_INVALID_ARGUMENT;
    if (storage->frame.text && storage->frame.text->base.text_columns == columns &&
        storage->frame.text->base.text_rows == rows && storage->frame.text->base.stride == stride)
        return LIB_STATUS_OK;
    kvm_window_frame_release(storage);
    text = lib_allocate_zero(1u, sizeof(*text) + bytes);
    if (text == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    text->base.text_columns = columns;
    text->base.text_rows = rows;
    text->base.stride = stride;
    text->base.capacity = bytes / sizeof(kvm_text_cell);
    text->base.cells = (kvm_text_cell *)(text + 1);
    storage->data.text = text;
    storage->frame.text = text;
    return LIB_STATUS_OK;
}

static inline lib_status kvm_window_frame_prepare_graphics(
    kvm_window_frame_storage *storage, lib_u32 width, lib_u32 height, lib_u32 stride)
{
    lib_size bytes;
    kvm_window_graphics_frame *image;
    if (storage == LIB_NULL || kvm_frame_extent(width, height, stride,
            sizeof(lib_u32), &bytes) != LIB_STATUS_OK ||
        bytes > LIB_SIZE_MAX - sizeof(*image)) return LIB_STATUS_INVALID_ARGUMENT;
    if (storage->frame.graphics && storage->frame.graphics->width == width &&
        storage->frame.graphics->height == height && storage->frame.graphics->stride == stride)
        return LIB_STATUS_OK;
    kvm_window_frame_release(storage);
    image = lib_allocate_zero(1u, sizeof(*image) + bytes);
    if (image == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    image->width = width; image->height = height; image->stride = stride;
    image->capacity = bytes / sizeof(lib_u32);
    image->pixels = (lib_u32 *)(image + 1);
    storage->data.graphics = image;
    storage->frame.graphics = image;
    return LIB_STATUS_OK;
}

static inline lib_status kvm_window_frame_validate(const kvm_window_frame *frame)
{
    lib_status status;
    if (frame == LIB_NULL || (frame->text == LIB_NULL) == (frame->graphics == LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->graphics != LIB_NULL) {
        lib_size bytes;
        if (frame->graphics->pixels == LIB_NULL ||
            kvm_frame_extent(frame->graphics->width, frame->graphics->height,
                frame->graphics->stride, sizeof(lib_u32), &bytes) != LIB_STATUS_OK ||
            frame->graphics->capacity < bytes / sizeof(lib_u32))
            return LIB_STATUS_INVALID_ARGUMENT;
        for (lib_u32 row = 0u; row < frame->graphics->height; ++row)
            for (lib_u32 column = 0u; column < frame->graphics->width; ++column)
                if (frame->graphics->pixels[(lib_size)row * frame->graphics->stride + column] & 0xff000000u)
                    return LIB_STATUS_INVALID_ARGUMENT;
        return LIB_STATUS_OK;
    }
    status = kvm_text_frame_validate(&frame->text->base);
    if (status != LIB_STATUS_OK) return status;
    return frame->text->base.font_height > KVM_WINDOW_FONT_HEIGHT ?
        LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}

/* Explicit diagnostic/fixture copy only, never presentation transport.
 * Destination owns its backing; source pointers must not survive in it. */
static inline lib_bool kvm_window_frame_copy(kvm_window_frame_storage *destination,
    const kvm_window_frame *source)
{
    if (destination == LIB_NULL || kvm_window_frame_validate(source) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (&destination->frame == source) return LIB_TRUE;
    if (source->graphics != LIB_NULL) {
        const kvm_window_graphics_frame *image = source->graphics;
        if (kvm_window_frame_prepare_graphics(destination, image->width,
                image->height, image->stride) != LIB_STATUS_OK) return LIB_FALSE;
        lib_memory_copy(destination->data.graphics->pixels, image->pixels,
            (lib_size)image->stride * image->height * sizeof(lib_u32));
    } else {
        const kvm_text_frame *text = &source->text->base;
        kvm_text_cell *cells;
        if (kvm_window_frame_prepare_text(destination, text->text_columns,
                text->text_rows, text->stride) != LIB_STATUS_OK) return LIB_FALSE;
        cells = destination->data.text->base.cells;
        *destination->data.text = *source->text;
        destination->data.text->base.cells = cells;
        destination->data.text->base.capacity = (lib_size)text->stride * text->text_rows;
        lib_memory_copy(cells, text->cells, destination->data.text->base.capacity * sizeof(*cells));
    }
    destination->frame.sequence = source->sequence;
    return LIB_TRUE;
}

/* Producer commit: validate before replacing the last complete publication. */
static inline lib_status kvm_window_frame_commit(kvm_frame_exchange *frames)
{
    lib_status status;
    if (frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = kvm_window_frame_validate(kvm_frame_exchange_writer(frames));
    if (status == LIB_STATUS_OK) kvm_frame_exchange_publish(frames);
    return status;
}

#endif
