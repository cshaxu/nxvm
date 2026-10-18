#ifndef KVM_WINDOW_FRAME_INTERFACE_H
#define KVM_WINDOW_FRAME_INTERFACE_H

#include "lib/kvm-base/frame_interface.h"
#include "lib/kvm-base/frame_exchange_interface.h"

#define KVM_WINDOW_GRAPHICS_MAX_WIDTH 1280u
#define KVM_WINDOW_GRAPHICS_MAX_HEIGHT 768u
#define KVM_WINDOW_GRAPHICS_MAX_PIXELS (KVM_WINDOW_GRAPHICS_MAX_WIDTH * KVM_WINDOW_GRAPHICS_MAX_HEIGHT)
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
    lib_u32 pixels[KVM_WINDOW_GRAPHICS_MAX_PIXELS];
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
        kvm_window_text_frame text;
        kvm_window_graphics_frame graphics;
    } data;
} kvm_window_frame_storage;

static inline kvm_window_text_frame *kvm_window_frame_begin_text(
    kvm_window_frame_storage *storage)
{
    storage->frame.text = &storage->data.text;
    storage->frame.graphics = LIB_NULL;
    return &storage->data.text;
}

static inline kvm_window_graphics_frame *kvm_window_frame_begin_graphics(
    kvm_window_frame_storage *storage)
{
    storage->frame.text = LIB_NULL;
    storage->frame.graphics = &storage->data.graphics;
    return &storage->data.graphics;
}

static inline lib_status kvm_window_frame_validate(const kvm_window_frame *frame)
{
    lib_status status;
    if (frame == LIB_NULL || (frame->text == LIB_NULL) == (frame->graphics == LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->graphics != LIB_NULL) {
        if (frame->graphics->width == 0u || frame->graphics->height == 0u ||
            frame->graphics->stride < frame->graphics->width)
            return LIB_STATUS_INVALID_ARGUMENT;
        if (frame->graphics->width > KVM_WINDOW_GRAPHICS_MAX_WIDTH ||
            frame->graphics->height > KVM_WINDOW_GRAPHICS_MAX_HEIGHT ||
            frame->graphics->stride > KVM_WINDOW_GRAPHICS_MAX_WIDTH ||
            frame->graphics->height > KVM_WINDOW_GRAPHICS_MAX_PIXELS / frame->graphics->stride)
            return LIB_STATUS_UNSUPPORTED;
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
        kvm_window_graphics_frame *image = kvm_window_frame_begin_graphics(destination);
        lib_memory_copy(image, source->graphics,
            lib_offsetof(kvm_window_graphics_frame, pixels) +
            (lib_size)source->graphics->stride * source->graphics->height * sizeof(lib_u32));
    } else {
        lib_memory_copy(kvm_window_frame_begin_text(destination), source->text,
            sizeof(*source->text));
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
