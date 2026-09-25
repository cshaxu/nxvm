#include "lib/kvm-window/geometry.h"

/* Ceiling preserves the limiting axis when integer bounds are fitted again. */
static lib_i32 kvm_window_scale_extent(lib_i32 extent, lib_u32 numerator, lib_u32 denominator)
{
    return (lib_i32)(((lib_u64)extent * numerator + denominator - 1u) / denominator);
}

void kvm_window_map_dirty_rect(const kvm_window_rect *source, const kvm_window_rect *display,
    lib_u32 source_width, lib_u32 source_height, kvm_window_rect *target)
{
    lib_i32 width;
    lib_i32 height;

    if (source == LIB_NULL || display == LIB_NULL || target == LIB_NULL ||
        source_width == 0u || source_height == 0u) return;
    width = display->right - display->left;
    height = display->bottom - display->top;
    target->left = display->left + source->left * width / (lib_i32)source_width;
    target->top = display->top + source->top * height / (lib_i32)source_height;
    target->right = display->left + (source->right * width +
        (lib_i32)source_width - 1) / (lib_i32)source_width;
    target->bottom = display->top + (source->bottom * height +
        (lib_i32)source_height - 1) / (lib_i32)source_height;
    if (target->right <= target->left) target->right = target->left + 1;
    if (target->bottom <= target->top) target->bottom = target->top + 1;
}

lib_bool kvm_window_fit_outer_rect(const kvm_window_rect *work_area, lib_i32 desired_width,
    lib_i32 desired_height, kvm_window_rect *fitted)
{
    lib_i32 available_width;
    lib_i32 available_height;
    lib_i32 width;
    lib_i32 height;

    if (work_area == LIB_NULL || fitted == LIB_NULL || desired_width <= 0 ||
        desired_height <= 0) return LIB_FALSE;
    available_width = work_area->right - work_area->left;
    available_height = work_area->bottom - work_area->top;
    if (available_width <= 0 || available_height <= 0) return LIB_FALSE;
    width = desired_width;
    height = desired_height;
    if (width > available_width || height > available_height) {
        if (!kvm_window_fit_aspect_size(available_width, available_height,
                (lib_u32)desired_width, (lib_u32)desired_height, &width, &height)) return LIB_FALSE;
    }
    fitted->left = work_area->left + (available_width - width) / 2;
    fitted->top = work_area->top + (available_height - height) / 2;
    fitted->right = fitted->left + width;
    fitted->bottom = fitted->top + height;
    return LIB_TRUE;
}

lib_bool kvm_window_fit_client_size(const kvm_window_rect *work_area, lib_i32 decoration_width,
    lib_i32 decoration_height, lib_i32 desired_width, lib_i32 desired_height,
    lib_i32 *fitted_width, lib_i32 *fitted_height)
{
    lib_i32 available_width;
    lib_i32 available_height;

    if (work_area == LIB_NULL || fitted_width == LIB_NULL || fitted_height == LIB_NULL ||
        decoration_width < 0 || decoration_height < 0 || desired_width <= 0 ||
        desired_height <= 0) return LIB_FALSE;
    available_width = (work_area->right - work_area->left) - decoration_width;
    available_height = (work_area->bottom - work_area->top) - decoration_height;
    if (available_width <= 0 || available_height <= 0) return LIB_FALSE;
    if (desired_width <= available_width && desired_height <= available_height) {
        *fitted_width = desired_width;
        *fitted_height = desired_height;
        return LIB_TRUE;
    }
    return kvm_window_fit_aspect_size(available_width, available_height,
        (lib_u32)desired_width, (lib_u32)desired_height, fitted_width,
        fitted_height);
}

lib_bool kvm_window_fit_aspect_size(lib_i32 available_width, lib_i32 available_height,
    lib_u32 source_width, lib_u32 source_height, lib_i32 *fitted_width,
    lib_i32 *fitted_height)
{
    lib_i32 width;
    lib_i32 height;

    if (available_width <= 0 || available_height <= 0 || source_width == 0u ||
        source_height == 0u || fitted_width == LIB_NULL || fitted_height == LIB_NULL)
        return LIB_FALSE;
    if ((lib_u64)available_width * source_height <=
        (lib_u64)available_height * source_width) {
        width = available_width;
        height = kvm_window_scale_extent(width, source_height, source_width);
    } else {
        height = available_height;
        width = kvm_window_scale_extent(height, source_width, source_height);
    }
    if (width <= 0 || height <= 0) return LIB_FALSE;
    *fitted_width = width;
    *fitted_height = height;
    return LIB_TRUE;
}

lib_bool kvm_window_cursor_rect(const kvm_window_frame *frame, const kvm_window_rect *display,
    kvm_window_rect *cursor)
{
    lib_i32 width, height, cell_top, cell_bottom;
    lib_u32 top, bottom, font_height;
    if (kvm_window_frame_validate(frame) != LIB_STATUS_OK || !display || !cursor || frame->graphics ||
        !frame->text.base.cursor_visible || frame->text.base.cursor_column < 0 || frame->text.base.cursor_row < 0 ||
        frame->text.base.cursor_column >= frame->text.base.text_columns || frame->text.base.cursor_row >= frame->text.base.text_rows)
        return LIB_FALSE;
    width = display->right - display->left;
    height = display->bottom - display->top;
    if (width <= 0 || height <= 0) return LIB_FALSE;
    cell_top = (lib_i32)((lib_i64)frame->text.base.cursor_row*height/frame->text.base.text_rows);
    cell_bottom = (lib_i32)((lib_i64)(frame->text.base.cursor_row+1)*height/frame->text.base.text_rows);
    cursor->left = display->left+(lib_i32)((lib_i64)frame->text.base.cursor_column*width/frame->text.base.text_columns);
    cursor->right = display->left+(lib_i32)((lib_i64)(frame->text.base.cursor_column+1)*width/frame->text.base.text_columns);
    cursor->top = display->top+cell_top;
    cursor->bottom = display->top+cell_bottom;
    font_height = frame->text.base.font_height ? frame->text.base.font_height : KVM_WINDOW_FONT_HEIGHT;
    if (frame->text.base.cursor_bottom >= frame->text.base.cursor_top) {
        top = frame->text.base.cursor_top;
        if (top >= font_height) return LIB_FALSE;
        bottom = (lib_u32)frame->text.base.cursor_bottom + 1u;
        if (bottom > font_height) bottom = font_height;
        cursor->top = display->top+cell_top+(lib_i32)(
            (lib_i64)(cell_bottom-cell_top)*top/font_height);
        cursor->bottom = display->top+cell_top+(lib_i32)(
            ((lib_i64)(cell_bottom-cell_top)*bottom+font_height-1u)/font_height);
    }
    return cursor->right > cursor->left && cursor->bottom > cursor->top;
}


void kvm_window_constrain_sizing(kvm_window_rect *outer, kvm_window_edge edge,
    lib_i32 frame_width, lib_i32 frame_height, lib_u32 source_width, lib_u32 source_height)
{
    lib_i32 client_width, client_height, target_width, target_height;
    if (!outer || !source_width || !source_height ||
        frame_width < 0 || frame_height < 0) return;
    target_width = outer->right - outer->left;
    target_height = outer->bottom - outer->top;
    client_width = target_width - frame_width;
    client_height = target_height - frame_height;
    if (client_width <= 0 || client_height <= 0) return;
    if (edge == KVM_WINDOW_EDGE_LEFT || edge == KVM_WINDOW_EDGE_RIGHT) {
        client_height = kvm_window_scale_extent(client_width, source_height, source_width);
    } else if (edge == KVM_WINDOW_EDGE_TOP || edge == KVM_WINDOW_EDGE_BOTTOM) {
        client_width = kvm_window_scale_extent(client_height, source_width, source_height);
    } else if ((lib_u64)client_width * source_height >=
        (lib_u64)client_height * source_width) {
        client_height = kvm_window_scale_extent(client_width, source_height, source_width);
    } else {
        client_width = kvm_window_scale_extent(client_height, source_width, source_height);
    }
    target_width = client_width + frame_width;
    target_height = client_height + frame_height;
    if (edge == KVM_WINDOW_EDGE_LEFT || edge == KVM_WINDOW_EDGE_TOPLEFT || edge == KVM_WINDOW_EDGE_BOTTOMLEFT)
        outer->left = outer->right - target_width;
    else
        outer->right = outer->left + target_width;
    if (edge == KVM_WINDOW_EDGE_TOP || edge == KVM_WINDOW_EDGE_TOPLEFT || edge == KVM_WINDOW_EDGE_TOPRIGHT)
        outer->top = outer->bottom - target_height;
    else
        outer->bottom = outer->top + target_height;
}
