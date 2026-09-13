#include "lib/ui-window/geometry.h"

int ui_window_display_rect(int client_width, int client_height,
    lib_u32 source_width, lib_u32 source_height, ui_window_rect *display)
{
    if (display == LIB_NULL || source_width == 0u || source_height == 0u ||
        client_width <= 0 || client_height <= 0) return 0;
    display->left = 0;
    display->top = 0;
    display->right = client_width;
    display->bottom = client_height;
    return 1;
}

void ui_window_map_dirty_rect(const ui_window_rect *source, const ui_window_rect *display,
    lib_u32 source_width, lib_u32 source_height, ui_window_rect *target)
{
    int width;
    int height;

    if (source == LIB_NULL || display == LIB_NULL || target == LIB_NULL ||
        source_width == 0u || source_height == 0u) return;
    width = display->right - display->left;
    height = display->bottom - display->top;
    target->left = display->left + source->left * width / (int)source_width;
    target->top = display->top + source->top * height / (int)source_height;
    target->right = display->left + (source->right * width +
        (int)source_width - 1) / (int)source_width;
    target->bottom = display->top + (source->bottom * height +
        (int)source_height - 1) / (int)source_height;
    if (target->right <= target->left) target->right = target->left + 1;
    if (target->bottom <= target->top) target->bottom = target->top + 1;
}

int ui_window_fit_outer_rect(const ui_window_rect *work_area, int desired_width,
    int desired_height, ui_window_rect *fitted)
{
    int available_width;
    int available_height;
    int width;
    int height;

    if (work_area == LIB_NULL || fitted == LIB_NULL || desired_width <= 0 ||
        desired_height <= 0) return 0;
    available_width = work_area->right - work_area->left;
    available_height = work_area->bottom - work_area->top;
    if (available_width <= 0 || available_height <= 0) return 0;
    width = desired_width;
    height = desired_height;
    if (width > available_width || height > available_height) {
        if ((lib_u64)available_width * (lib_u64)desired_height <=
            (lib_u64)available_height * (lib_u64)desired_width) {
            width = available_width;
            height = (int)((lib_u64)width * (lib_u64)desired_height /
                (lib_u64)desired_width);
        } else {
            height = available_height;
            width = (int)((lib_u64)height * (lib_u64)desired_width /
                (lib_u64)desired_height);
        }
        if (width <= 0 || height <= 0) return 0;
    }
    fitted->left = work_area->left + (available_width - width) / 2;
    fitted->top = work_area->top + (available_height - height) / 2;
    fitted->right = fitted->left + width;
    fitted->bottom = fitted->top + height;
    return 1;
}

int ui_window_fit_client_size(const ui_window_rect *work_area, int decoration_width,
    int decoration_height, int desired_width, int desired_height,
    int *fitted_width, int *fitted_height)
{
    int available_width;
    int available_height;

    if (work_area == LIB_NULL || fitted_width == LIB_NULL || fitted_height == LIB_NULL ||
        decoration_width < 0 || decoration_height < 0 || desired_width <= 0 ||
        desired_height <= 0) return 0;
    available_width = (work_area->right - work_area->left) - decoration_width;
    available_height = (work_area->bottom - work_area->top) - decoration_height;
    if (available_width <= 0 || available_height <= 0) return 0;
    if (desired_width <= available_width && desired_height <= available_height) {
        *fitted_width = desired_width;
        *fitted_height = desired_height;
        return 1;
    }
    return ui_window_fit_aspect_size(available_width, available_height,
        (lib_u32)desired_width, (lib_u32)desired_height, fitted_width,
        fitted_height);
}

int ui_window_fit_aspect_size(int available_width, int available_height,
    lib_u32 source_width, lib_u32 source_height, int *fitted_width,
    int *fitted_height)
{
    int width;
    int height;

    if (available_width <= 0 || available_height <= 0 || source_width == 0u ||
        source_height == 0u || fitted_width == LIB_NULL || fitted_height == LIB_NULL)
        return 0;
    if ((lib_u64)available_width * source_height <=
        (lib_u64)available_height * source_width) {
        width = available_width;
        height = (int)((lib_u64)width * source_height / source_width);
    } else {
        height = available_height;
        width = (int)((lib_u64)height * source_width / source_height);
    }
    if (width <= 0 || height <= 0) return 0;
    *fitted_width = width;
    *fitted_height = height;
    return 1;
}

int ui_window_cursor_rect(const ui_frame *frame, const ui_window_rect *display,
    ui_window_rect *cursor)
{
    int width, height, cell_top, cell_bottom;
    lib_u32 top, bottom;
    if (!ui_frame_is_valid(frame) || !display || !cursor || frame->graphics ||
        !frame->cursor_visible || frame->cursor_column < 0 || frame->cursor_row < 0 ||
        frame->cursor_column >= frame->text_columns || frame->cursor_row >= frame->text_rows)
        return 0;
    width = display->right - display->left;
    height = display->bottom - display->top;
    if (width <= 0 || height <= 0) return 0;
    cell_top = (int)((lib_i64)frame->cursor_row*height/frame->text_rows);
    cell_bottom = (int)((lib_i64)(frame->cursor_row+1)*height/frame->text_rows);
    cursor->left = display->left+(lib_i32)((lib_i64)frame->cursor_column*width/frame->text_columns);
    cursor->right = display->left+(lib_i32)((lib_i64)(frame->cursor_column+1)*width/frame->text_columns);
    cursor->top = display->top+cell_top;
    cursor->bottom = display->top+cell_bottom;
    if (frame->font_height && frame->cursor_bottom >= frame->cursor_top) {
        top = frame->cursor_top;
        if (top >= frame->font_height) return 0;
        bottom = (lib_u32)frame->cursor_bottom + 1u;
        if (bottom > frame->font_height) bottom = frame->font_height;
        cursor->top = display->top+cell_top+(lib_i32)(
            (lib_i64)(cell_bottom-cell_top)*top/frame->font_height);
        cursor->bottom = display->top+cell_top+(lib_i32)(
            ((lib_i64)(cell_bottom-cell_top)*bottom+frame->font_height-1u)/frame->font_height);
    }
    return cursor->right > cursor->left && cursor->bottom > cursor->top;
}


void ui_window_constrain_sizing(ui_window_rect *outer, ui_window_edge edge,
    int frame_width, int frame_height, lib_u32 source_width, lib_u32 source_height)
{
    int client_width, client_height, target_width, target_height;
    if (!outer || !source_width || !source_height ||
        frame_width < 0 || frame_height < 0) return;
    target_width = outer->right - outer->left;
    target_height = outer->bottom - outer->top;
    client_width = target_width - frame_width;
    client_height = target_height - frame_height;
    if (client_width <= 0 || client_height <= 0) return;
    if (edge == UI_WINDOW_EDGE_LEFT || edge == UI_WINDOW_EDGE_RIGHT) {
        client_height = (int)((lib_u64)client_width * source_height /
            source_width);
    } else if (edge == UI_WINDOW_EDGE_TOP || edge == UI_WINDOW_EDGE_BOTTOM) {
        client_width = (int)((lib_u64)client_height * source_width /
            source_height);
    } else if ((lib_u64)client_width * source_height >=
        (lib_u64)client_height * source_width) {
        client_height = (int)((lib_u64)client_width * source_height /
            source_width);
    } else {
        client_width = (int)((lib_u64)client_height * source_width /
            source_height);
    }
    target_width = client_width + frame_width;
    target_height = client_height + frame_height;
    if (edge == UI_WINDOW_EDGE_LEFT || edge == UI_WINDOW_EDGE_TOPLEFT || edge == UI_WINDOW_EDGE_BOTTOMLEFT)
        outer->left = outer->right - target_width;
    else
        outer->right = outer->left + target_width;
    if (edge == UI_WINDOW_EDGE_TOP || edge == UI_WINDOW_EDGE_TOPLEFT || edge == UI_WINDOW_EDGE_TOPRIGHT)
        outer->top = outer->bottom - target_height;
    else
        outer->bottom = outer->top + target_height;
}
