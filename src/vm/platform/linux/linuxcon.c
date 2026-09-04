/* Copyright 2012-2014 Neko. */

/* LINUXCON provides linux terminal interface. */

#include "type.h"

#include "core/platform/wait_interface.h"

#include <curses.h>

#include <pthread.h>


#include "core/platform/display_frame.h"

#include "vm/platform/platform_internal.h"


#include "vm/platform/linux/linuxcon.h"

static C_INT vm_platform_linuxcon_minimum(C_INT left, C_INT right)
{
    return left < right ? left : right;
}

static C_INT vm_platform_linuxcon_terminal_initialize()
{
    STD_SIZE_T i, j;
    if (initscr() == STD_NULL) return 0;
    if (raw() == ERR || nodelay(stdscr, TRUE) == ERR ||
        keypad(stdscr, TRUE) == ERR || noecho() == ERR) {
        endwin();
        return 0;
    }
    if (has_colors()) start_color();
    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
            if (has_colors()) init_pair(i * 8 + j, i, j);
        }
    }
    return 1;
}

static C_VOID vm_platform_linuxcon_terminal_finalize()
{
    noraw();
    nodelay(stdscr, FALSE);
    keypad(stdscr, FALSE);
    endwin();
}

static type_unsigned_8 vm_platform_linuxcon_reverse_color(type_unsigned_8 value)
{
    value &= 0x07;
    switch (value) {
    case COLOR_BLACK:
        return COLOR_WHITE;
    case COLOR_BLUE:
        return COLOR_YELLOW;
    case COLOR_GREEN:
        return COLOR_RED;
    case COLOR_CYAN:
        return COLOR_MAGENTA;
    case COLOR_RED:
        return COLOR_GREEN;
    case COLOR_MAGENTA:
        return COLOR_CYAN;
    case COLOR_YELLOW:
        return COLOR_BLUE;
    case COLOR_WHITE:
        return COLOR_BLACK;
    }
    return COLOR_BLACK;
}

static type_unsigned_8 vm_platform_linuxcon_attribute_color(type_unsigned_8 value)
{
    value &= 0x07;
    switch (value) {
    case 0x00:
        return COLOR_BLACK;
    case 0x01:
        return COLOR_BLUE;
    case 0x02:
        return COLOR_GREEN;
    case 0x03:
        return COLOR_CYAN;
    case 0x04:
        return COLOR_RED;
    case 0x05:
        return COLOR_MAGENTA;
    case 0x06:
        return COLOR_YELLOW;
    case 0x07:
        return COLOR_WHITE;
    }
    return COLOR_BLACK;
}

static type_unsigned_8 vm_platform_linuxcon_color_pair(type_unsigned_8 prop)
{
    type_unsigned_8 fore0, back0, fore1, back1;
    fore0 = prop & 0x0f;
    back0 = ((prop & 0x70) >> 4);
    fore1 = vm_platform_linuxcon_attribute_color(fore0);
    back1 = vm_platform_linuxcon_attribute_color(back0);
    if (fore0 != back0 && fore1 == back1) {
        fore1 = vm_platform_linuxcon_reverse_color(fore1);
    }
    return (fore1 * 8 + back1);
}

static const type_unsigned_8 vm_platform_linuxcon_ascii_to_print[][2] = {
    {0x00, ' ' }, {0x01, '*' }, {0x02, '*' }, {0x03, '*' },
    {0x04, '*' }, {0x05, '*' }, {0x06, '*' }, {0x07, 0x07},
    {0x08, 0x08}, {0x09, 0x09}, {0x0a, 0x0a}, {0x0b, 0x0b},
    {0x0c, 0x0c}, {0x0d, 0x0d}, {0x0e, '*' }, {0x0f, '*' },
    {0x10, '>' }, {0x11, '<' }, {0x12, '|' }, {0x13, '!' },
    {0x14, 'T' }, {0x15, '$' }, {0x16, '_' }, {0x17, '|' },
    {0x18, '^' }, {0x19, 'v' }, {0x1a, '>' }, {0x1b, '<' },
    {0x1c, 'L' }, {0x1d, '=' }, {0x1e, '^' }, {0x1f, 'v' },
    {0x20, ' '}, {0x21, '!'}, {0x22,'\"'}, {0x23, '#'},
    {0x24, '$'}, {0x25, '%'}, {0x26, '&'}, {0x27,'\''},
    {0x28, '('}, {0x29, ')'}, {0x2a, '*'}, {0x2b, '+'},
    {0x2c, ','}, {0x2d, '-'}, {0x2e, '.'}, {0x2f, '/'},
    {0x30, '0'}, {0x31, '1'}, {0x32, '2'}, {0x33, '3'},
    {0x34, '4'}, {0x35, '5'}, {0x36, '6'}, {0x37, '7'},
    {0x38, '8'}, {0x39, '9'}, {0x3a, ':'}, {0x3b, ';'},
    {0x3c, '<'}, {0x3d, '='}, {0x3e, '>'}, {0x3f, '?'},
    {0x40, '@'}, {0x41, 'A'}, {0x42, 'B'}, {0x43, 'C'},
    {0x44, 'D'}, {0x45, 'E'}, {0x46, 'F'}, {0x47, 'G'},
    {0x48, 'H'}, {0x49, 'I'}, {0x4a, 'J'}, {0x4b, 'K'},
    {0x4c, 'L'}, {0x4d, 'M'}, {0x4e, 'N'}, {0x4f, 'O'},
    {0x50, 'P'}, {0x51, 'Q'}, {0x52, 'R'}, {0x53, 'S'},
    {0x54, 'T'}, {0x55, 'U'}, {0x56, 'V'}, {0x57, 'W'},
    {0x58, 'X'}, {0x59, 'Y'}, {0x5a, 'Z'}, {0x5b, '['},
    {0x5c,'\\'}, {0x5d, ']'}, {0x5e, '^'}, {0x5f, '_'},
    {0x60, '`'}, {0x61, 'a'}, {0x62, 'b'}, {0x63, 'c'},
    {0x64, 'd'}, {0x65, 'e'}, {0x66, 'f'}, {0x67, 'g'},
    {0x68, 'h'}, {0x69, 'i'}, {0x6a, 'j'}, {0x6b, 'k'},
    {0x6c, 'l'}, {0x6d, 'm'}, {0x6e, 'n'}, {0x6f, 'o'},
    {0x70, 'p'}, {0x71, 'q'}, {0x72, 'r'}, {0x73, 's'},
    {0x74, 't'}, {0x75, 'u'}, {0x76, 'v'}, {0x77, 'w'},
    {0x78, 'x'}, {0x79, 'y'}, {0x7a, 'z'}, {0x7b, '{'},
    {0x7c, '|'}, {0x7d, '}'}, {0x7e, '~'}, {0x7f,0x7f},
    {0x80, 'C'}, {0x81, 'u'}, {0x82, 'e'}, {0x83, 'a'},
    {0x84, 'a'}, {0x85, 'a'}, {0x86, 'a'}, {0x87, 'c'},
    {0x88, 'e'}, {0x89, 'e'}, {0x8a, 'e'}, {0x8b, 'i'},
    {0x8c, 'i'}, {0x8d, 'i'}, {0x8e, 'A'}, {0x8f, 'A'},
    {0x90, 'E'}, {0x91, 'a'}, {0x92, 'A'}, {0x93, 'o'},
    {0x94, 'o'}, {0x95, 'o'}, {0x96, 'u'}, {0x97, 'u'},
    {0x98, 'y'}, {0x99, 'O'}, {0x9a, 'U'}, {0x9b, 'C'},
    {0x9c, 'L'}, {0x9d, 'Y'}, {0x9e, 'P'}, {0x9f, 'f'},
    {0xa0, 'a'}, {0xa1, 'i'}, {0xa2, 'o'}, {0xa3, 'u'},
    {0xa4, 'n'}, {0xa5, 'N'}, {0xa6, 'a'}, {0xa7, 'o'},
    {0xa8, '?'}, {0xa9, '+'}, {0xaa, '+'}, {0xab, '/'},
    {0xac, '/'}, {0xad, 'i'}, {0xae, '<'}, {0xaf, '>'},
    {0xb0, '*'}, {0xb1, '*'}, {0xb2, '*'}, {0xb3, '|'},
    {0xb4, '|'}, {0xb5, '|'}, {0xb6, '|'}, {0xb7, '+'},
    {0xb8, '+'}, {0xb9, '|'}, {0xba, '|'}, {0xbb, '+'},
    {0xbc, '+'}, {0xbd, '+'}, {0xbe, '+'}, {0xbf, '+'},
    {0xc0, '+'}, {0xc1, '+'}, {0xc2, '+'}, {0xc3, '|'},
    {0xc4, '-'}, {0xc5, '+'}, {0xc6, '|'}, {0xc7, '|'},
    {0xc8, '+'}, {0xc9, '+'}, {0xca, '+'}, {0xcb, '+'},
    {0xcc, '|'}, {0xcd, '='}, {0xce, '+'}, {0xcf, '+'},
    {0xd0, '+'}, {0xd1, '+'}, {0xd2, '+'}, {0xd3, '+'},
    {0xd4, '+'}, {0xd5, '+'}, {0xd6, '+'}, {0xd7, '+'},
    {0xd8, '+'}, {0xd9, '+'}, {0xda, '+'}, {0xdb, '#'},
    {0xdc, '#'}, {0xdd, '#'}, {0xde, '#'}, {0xdf, '#'},
    {0xe0, 'a'}, {0xe1, 'b'}, {0xe2, 'r'}, {0xe3, '*'},
    {0xe4, 'S'}, {0xe5, 's'}, {0xe6, 'u'}, {0xe7, 'T'},
    {0xe8, '*'}, {0xe9, '*'}, {0xea, '*'}, {0xeb, 'd'},
    {0xec, '*'}, {0xed, '*'}, {0xee, '*'}, {0xef, '*'},
    {0xf0, '='}, {0xf1, '*'}, {0xf2, '*'}, {0xf3, '*'},
    {0xf4, '*'}, {0xf5, '*'}, {0xf6, '*'}, {0xf7, '~'},
    {0xf8, '.'}, {0xf9, '.'}, {0xfa, '.'}, {0xfb, '*'},
    {0xfc, 'n'}, {0xfd, '2'}, {0xfe, '#'}, {0xff, ' '}
};

static vm_platform_host_surface_lease linux_terminal_lease = {
    ATOMIC_VAR_INIT(0)
};

typedef struct linuxcon_run_handle linuxcon_run_handle;

static C_VOID vm_platform_linuxcon_process_keyboard(linuxcon_run_handle *handle);

static C_VOID vm_platform_linuxcon_paint(vm_platform_run_context *context,
    type_unsigned_8 force)
{
    C_INT ref;
    type_unsigned_8 p, c;
    C_INT i, j, sizeRow, sizeCol, curX, curY;
    core_platform_display_frame frame;

    if (context == STD_NULL || core_platform_presentation_mailbox_capture(
            context->presentation, &frame) != TYPE_STATUS_OK) return;
    if (frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT) return;
    sizeRow = vm_platform_linuxcon_minimum(COLS, frame.columns);
    sizeCol = vm_platform_linuxcon_minimum(LINES, frame.rows);
    ref = 0;
    if (force || (frame.generation != context->terminal_displayed_generation && frame.buffer_changed)) {
        clear();
        for (i = 0; i < sizeCol; ++i) {
            for (j = 0; j < sizeRow; ++j) {
                c = frame.characters[i * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + j];
                p = frame.attributes[i * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + j] & 0x7f;
                c = vm_platform_linuxcon_ascii_to_print[c][1];
                move(i, j);
                addch(c | COLOR_PAIR(vm_platform_linuxcon_color_pair(p)));
            }
        }
        ref = 1;
    }
    if (force || (frame.generation != context->terminal_displayed_generation && frame.cursor_changed)) {
        curX = frame.cursor_x;
        curY = frame.cursor_y;
        if (curX < sizeCol && curY < sizeRow) {
            move(frame.cursor_y, frame.cursor_x);
        } else {
            move(0, 0);
        }
        ref = 1;
    }
    if (ref) {
        refresh();
    }
    context->terminal_displayed_generation = frame.generation;
}

struct linuxcon_run_handle {
    vm_platform_run_handle *owner;
    const vm_platform_run_context *platform;
    pthread_t kernel_thread;
    pthread_t display_thread;
    C_INT kernel_started;
    C_INT display_started;
    STD_ATOMIC_BOOL display_ready;
    STD_ATOMIC_BOOL display_failed;
};

static C_INT linuxcon_display_wait_cancelled(C_VOID *context)
{
    const linuxcon_run_handle *handle = context;

    return handle == STD_NULL || !vm_platform_execution_is_running_for(
        handle->platform->execution);
}

static C_INT linuxcon_display_ready_wait_cancelled(C_VOID *context)
{
    const linuxcon_run_handle *handle = context;

    return handle == STD_NULL || STD_ATOMIC_LOAD(&handle->display_ready) ||
        STD_ATOMIC_LOAD(&handle->display_failed);
}

static C_VOID *linuxcon_display_thread(C_VOID *arg) {
    linuxcon_run_handle *handle = arg;
    vm_platform_run_context *context = (vm_platform_run_context *)handle->platform;

    if (!vm_platform_linuxcon_terminal_initialize()) {
        STD_ATOMIC_STORE(&handle->display_failed, TYPE_TRUE);
        vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
        return STD_NULL;
    }
    STD_ATOMIC_STORE(&handle->display_ready, TYPE_TRUE);
    vm_platform_linuxcon_paint(context, 1);
    while (vm_platform_execution_is_running_for(context->execution)) {
        vm_platform_linuxcon_paint(context, 0);
        vm_platform_linuxcon_process_keyboard(handle);
        (C_VOID)core_platform_wait_milliseconds(20u,
            linuxcon_display_wait_cancelled, handle);
    }
    vm_platform_linuxcon_terminal_finalize();
    return 0;
}

static C_VOID *linuxcon_kernel_thread(C_VOID *arg) {
    linuxcon_run_handle *handle = arg;

    vm_platform_execution_start_for(handle->platform->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    return 0;
}

static C_VOID vm_platform_linuxcon_send_key(
    const vm_platform_run_context *context, C_UCHAR scan, C_UCHAR key)
{
    core_platform_input_event event;

    if (context == STD_NULL) return;
    event.kind = CORE_PLATFORM_INPUT_KEY;
    event.data.key.scan_code = scan;
    event.data.key.virtual_key = key;
    event.data.key.pressed = TYPE_TRUE;
    (C_VOID)vm_platform_host_input_sink_submit(&context->input_sink, &event);
}

static C_VOID vm_platform_linuxcon_make_key(linuxcon_run_handle *handle,
    C_INT keyvalue)
{
    const vm_platform_run_context *context = handle->platform;

    if (keyvalue < 0x001b) {
        switch (keyvalue) {
        case 0x000a:
            /* ENTER */
            vm_platform_linuxcon_send_key(context, 0x1cu, 0x000du);
            break;
        default:
            /* CTRL + LETTER */
            vm_platform_linuxcon_send_key(context, 0x1du, 0u);
            vm_platform_linuxcon_send_key(context, 0u, keyvalue + 0x60);
            break;
        }
    } else if (keyvalue < 0x0020) {
        switch (keyvalue) {
        case 0x001b:
            keyvalue = getch();
            if (keyvalue == ERR) {
                /* ESCAPE*/
                vm_platform_linuxcon_send_key(context, 0x01u, 0x001bu);
            } else {
                /* ALT */
                vm_platform_linuxcon_send_key(context, 0x38u, 0u);
                vm_platform_linuxcon_send_key(context, 0u, keyvalue);
            }
            break;
        default:
            /* CTRL + NUMBER */
            break;
        }
    } else if (keyvalue < 0x0100) {
        switch (keyvalue) {
        default:
            vm_platform_linuxcon_send_key(context, 0u, keyvalue);
            break;
        }
    } else if (keyvalue > KEY_F0 && keyvalue <= KEY_F(12)) {
        switch (keyvalue) {
        default:
            vm_platform_linuxcon_send_key(context, keyvalue - KEY_F0 + 0x3a,
                0u);
            break;
        }
    } else {
        /* get special keys */
        switch (keyvalue) {
        case KEY_DOWN:
            vm_platform_linuxcon_send_key(context, 0x50u, 0u);
            break;
        case KEY_UP:
            vm_platform_linuxcon_send_key(context, 0x48u, 0u);
            break;
        case KEY_LEFT:
            vm_platform_linuxcon_send_key(context, 0x4bu, 0u);
            break;
        case KEY_RIGHT:
            vm_platform_linuxcon_send_key(context, 0x4du, 0u);
            break;
        case KEY_HOME:
            vm_platform_linuxcon_send_key(context, 0x47u, 0u);
            break;
        case KEY_BACKSPACE:
            vm_platform_linuxcon_send_key(context, 0x0eu, 0x0008u);
            break;
        case KEY_ENTER:
            vm_platform_linuxcon_send_key(context, 0x1cu, 0x000du);
            break;
        case KEY_NPAGE:
            vm_platform_linuxcon_send_key(context, 0x51u, 0u);
            break;
        case KEY_PPAGE:
            vm_platform_linuxcon_send_key(context, 0x49u, 0u);
            break;
        case KEY_END:
            vm_platform_linuxcon_send_key(context, 0x4fu, 0u);
            break;
        case 0x014a:
            /* DELETE */
            vm_platform_linuxcon_send_key(context, 0x53u, 0u);
            break;
        case 0x014b:
            /* INSERT */
            vm_platform_linuxcon_send_key(context, 0x52u, 0u);
            break;
        default:
            return;
        }
    }
}

static C_VOID vm_platform_linuxcon_process_keyboard(linuxcon_run_handle *handle)
{
    C_INT keyvalue = getch();
    if (keyvalue != ERR) {
        vm_platform_linuxcon_make_key(handle, keyvalue);
    }
}

C_VOID vm_platform_linuxcon_display_set_screen(
    const vm_platform_run_context *context)
{
    (C_VOID)context;
}

C_VOID vm_platform_linuxcon_display_paint(
    const vm_platform_run_context *context)
{
    vm_platform_linuxcon_paint((vm_platform_run_context *)context, 1);
}

type_status vm_platform_linuxcon_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner) {
    linuxcon_run_handle *handle;
    C_INT old_flip;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->input_sink.submit == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (vm_platform_host_surface_lease_acquire(&linux_terminal_lease,
            context) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    handle = (linuxcon_run_handle *)STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) {
        vm_platform_host_surface_lease_release(&linux_terminal_lease, context);
        return TYPE_STATUS_NO_MEMORY;
    }
    handle->owner = owner;
    handle->platform = context;
    STD_ATOMIC_INIT(&handle->display_ready, TYPE_FALSE);
    STD_ATOMIC_INIT(&handle->display_failed, TYPE_FALSE);
    owner->context = context;
    owner->backend = handle;
    owner->window_display = 0;
    owner->active = 1;
    old_flip = vm_platform_execution_get_flip_for(context->execution);
    if (pthread_create(&handle->kernel_thread, STD_NULL, linuxcon_kernel_thread,
            handle) != 0) {
        vm_platform_linuxcon_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->kernel_started = 1;
    if (!vm_platform_execution_wait_for_flip_for(context->execution, old_flip,
            VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS)) {
        vm_platform_linuxcon_run_handle_request_stop(owner);
        vm_platform_linuxcon_run_handle_join(owner);
        vm_platform_linuxcon_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (pthread_create(&handle->display_thread, STD_NULL, linuxcon_display_thread,
            handle) != 0) {
        vm_platform_linuxcon_run_handle_request_stop(owner);
        vm_platform_linuxcon_run_handle_join(owner);
        vm_platform_linuxcon_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->display_started = 1;
    (C_VOID)core_platform_wait_milliseconds(5000u,
        linuxcon_display_ready_wait_cancelled, handle);
    if (!STD_ATOMIC_LOAD(&handle->display_ready)) {
        vm_platform_linuxcon_run_handle_request_stop(owner);
        vm_platform_linuxcon_run_handle_join(owner);
        vm_platform_linuxcon_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_linuxcon_run_handle_request_stop(vm_platform_run_handle *owner) {
    linuxcon_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle != STD_NULL) vm_platform_execution_stop_for(handle->platform->execution);
}

C_VOID vm_platform_linuxcon_run_handle_join(vm_platform_run_handle *owner) {
    linuxcon_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_started) pthread_join(handle->kernel_thread, STD_NULL);
    if (handle->display_started) pthread_join(handle->display_thread, STD_NULL);
    owner->active = 0;
}

C_VOID vm_platform_linuxcon_run_handle_finalize(vm_platform_run_handle *owner) {
    linuxcon_run_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    vm_platform_host_surface_lease_release(&linux_terminal_lease,
        handle->platform);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
