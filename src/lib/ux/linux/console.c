#include "lib/base/base.h"
#include "lib/ux/linux/console.h"
#include "lib/ux/linux/input.h"
#include "lib/ux/linux/mailbox.h"

#if !defined(_WIN32)
#include <curses.h>
#include <poll.h>
#include <unistd.h>

static atomic_flag ux_linux_console_lease = ATOMIC_FLAG_INIT;

static lib_bool ux_linux_console_acquire(void)
{
    return !atomic_flag_test_and_set_explicit(&ux_linux_console_lease,
        memory_order_acquire);
}

static void ux_linux_console_release(void)
{
    atomic_flag_clear_explicit(&ux_linux_console_lease, memory_order_release);
}

static lib_u32 ux_linux_action_key(int key)
{
    if (key >= KEY_F(1) && key <= KEY_F(12))
        return UX_KEY_F1 + (lib_u32)(key - KEY_F(1));
    return (lib_u32)key;
}

static int ux_linux_console_key_event(int key, ux_event *event)
{
    if (key == '\n' || key == '\r' || key == KEY_ENTER)
        return ux_linux_key_to_event(UX_LINUX_KEY_ENTER, event);
    if (key == KEY_BACKSPACE || key == 0x08)
        return ux_linux_key_to_event(UX_LINUX_KEY_BACKSPACE, event);
    if (key >= KEY_F(1) && key <= KEY_F(12))
        return ux_linux_key_to_event((ux_linux_key)(UX_LINUX_KEY_F1 +
            key - KEY_F(1)), event);
    switch (key) {
    case KEY_UP: return ux_linux_key_to_event(UX_LINUX_KEY_UP, event);
    case KEY_DOWN: return ux_linux_key_to_event(UX_LINUX_KEY_DOWN, event);
    case KEY_LEFT: return ux_linux_key_to_event(UX_LINUX_KEY_LEFT, event);
    case KEY_RIGHT: return ux_linux_key_to_event(UX_LINUX_KEY_RIGHT, event);
    case KEY_HOME: return ux_linux_key_to_event(UX_LINUX_KEY_HOME, event);
    case KEY_END: return ux_linux_key_to_event(UX_LINUX_KEY_END, event);
    case KEY_PPAGE: return ux_linux_key_to_event(UX_LINUX_KEY_PAGE_UP, event);
    case KEY_NPAGE: return ux_linux_key_to_event(UX_LINUX_KEY_PAGE_DOWN, event);
    case KEY_IC: return ux_linux_key_to_event(UX_LINUX_KEY_INSERT, event);
    case KEY_DC: return ux_linux_key_to_event(UX_LINUX_KEY_DELETE, event);
    default: return LIB_FALSE;
    }
}

static int ux_linux_console_color_pair(lib_u16 attribute)
{
    lib_u16 foreground = attribute & 0x07u;
    lib_u16 background = (attribute >> 4u) & 0x07u;

    return (int)(foreground * 8u + background);
}

static void ux_linux_console_paint(const ux_frame *frame)
{
    lib_u32 row;

    if (frame == LIB_NULL || frame->valid == 0u || frame->graphics != 0u)
        return;
    erase();
    if (frame->text_columns == 0u || frame->text_rows == 0u ||
        frame->text_columns > UX_TEXT_COLUMNS ||
        frame->text_rows > UX_TEXT_ROWS) return;
    for (row = 0u; row < frame->text_rows && row < (lib_u32)LINES; ++row) {
        lib_u32 column;

        for (column = 0u; column < frame->text_columns &&
            column < (lib_u32)COLS; ++column) {
            lib_u32 offset = row * UX_TEXT_COLUMNS + column;
            lib_u8 character = frame->text[offset];

            if (has_colors()) attrset(COLOR_PAIR(ux_linux_console_color_pair(
                frame->attributes[offset])));
            mvaddch((int)row, (int)column,
                character >= 0x20u && character < 0x7fu ? character : ' ');
        }
    }
    if (frame->cursor_visible != 0u && frame->cursor_phase != 0u &&
        frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
        frame->cursor_column < (lib_i32)frame->text_columns &&
        frame->cursor_row < (lib_i32)frame->text_rows &&
        frame->cursor_column < COLS && frame->cursor_row < LINES)
        move(frame->cursor_row, frame->cursor_column);
    refresh();
}

static ux_run_result ux_linux_console_key(const ux_binding *binding, int key)
{
    ux_action action;
    ux_event event = { 0 };

    if (key == ERR) return UX_RUN_CONTINUE;
    action = ux_actions_match(binding->actions, ux_linux_action_key(key), 0u);
    if (action != UX_ACTION_NONE)
        return ux_binding_invoke_action(binding, action);
    if (key >= 0x20 && key <= 0xff) {
        event.type = UX_EVENT_TEXT;
        event.data.text.scalar = (lib_u32)key;
    } else if (!ux_linux_console_key_event(key, &event)) return UX_RUN_CONTINUE;
    (void)binding->input_sink(binding->context, &event);
    return UX_RUN_CONTINUE;
}

ux_run_result ux_linux_run_console(const ux_binding *binding)
{
    ux_frame *frame;
    lib_u32 displayed_generation = 0u;
    ux_run_result result = UX_RUN_STOPPED_RESULT;

    if (ux_binding_validate(binding) != LIB_STATUS_OK ||
        !ux_linux_console_acquire()) return UX_RUN_ERROR_RESULT;
    if (initscr() == LIB_NULL) {
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    if (raw() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR ||
        nodelay(stdscr, FALSE) == ERR) {
        endwin();
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    if (has_colors()) {
        lib_u32 foreground;

        (void)start_color();
        for (foreground = 0u; foreground < 8u; ++foreground) {
            lib_u32 background;

            for (background = 0u; background < 8u; ++background)
                (void)init_pair((short)(foreground * 8u + background),
                    (short)foreground, (short)background);
        }
    }
    frame = calloc(1u, sizeof(*frame));
    if (frame == LIB_NULL) {
        endwin();
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    while (binding->get_state(binding->context) == UX_RUN_RUNNING) {
        struct pollfd waits[2] = {
            { STDIN_FILENO, POLLIN, 0 },
            { ux_linux_mailbox_wait_fd(binding->mailbox), POLLIN, 0 }
        };
        int ready = poll(waits, 2u, -1);

        if (ready < 0) {
            result = UX_RUN_ERROR_RESULT;
            break;
        }
        if ((waits[1].revents & POLLIN) != 0) {
            ux_linux_mailbox_consume(binding->mailbox);
        }
        if ((waits[1].revents & POLLIN) != 0 &&
            ux_mailbox_generation(binding->mailbox) != displayed_generation &&
            ux_mailbox_capture(binding->mailbox, frame) == LIB_STATUS_OK) {
            if (ux_router_target(binding->router) == UX_TARGET_WINDOW) {
                result = UX_RUN_SWITCH_WINDOW;
                break;
            }
            ux_linux_console_paint(frame);
            displayed_generation = frame->sequence;
        }
        if ((waits[0].revents & POLLIN) != 0) {
            result = ux_linux_console_key(binding, getch());
            if (result != UX_RUN_CONTINUE) break;
        }
    }
    if (result == UX_RUN_STOPPED_RESULT &&
        binding->get_state(binding->context) == UX_RUN_PAUSED)
        result = UX_RUN_PAUSED_RESULT;
    free(frame);
    endwin();
    ux_linux_console_release();
    return result;
}
#endif
