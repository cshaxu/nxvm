#include "lib/ux/linux/console.h"
#include "lib/ux/linux/input.h"

#if !defined(_WIN32)
#include <curses.h>

static STD_ATOMIC_FLAG ux_linux_console_lease = ATOMIC_FLAG_INIT;

static type_bool ux_linux_console_acquire(C_VOID)
{
    return !STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&ux_linux_console_lease,
        STD_MEMORY_ORDER_ACQUIRE);
}

static C_VOID ux_linux_console_release(C_VOID)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&ux_linux_console_lease,
        STD_MEMORY_ORDER_RELEASE);
}

static type_unsigned_32 ux_linux_action_key(C_INT key)
{
    if (key >= KEY_F(1) && key <= KEY_F(12))
        return UX_KEY_F1 + (type_unsigned_32)(key - KEY_F(1));
    return (type_unsigned_32)key;
}

static C_INT ux_linux_console_guest_key(C_INT key, ux_event *event)
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
    default: return TYPE_FALSE;
    }
}

static C_INT ux_linux_console_color_pair(type_unsigned_16 attribute)
{
    type_unsigned_16 foreground = attribute & 0x07u;
    type_unsigned_16 background = (attribute >> 4u) & 0x07u;

    return (C_INT)(foreground * 8u + background);
}

static C_VOID ux_linux_console_paint(const ux_frame *frame)
{
    type_unsigned_32 row;

    if (frame == STD_NULL || frame->valid == 0u || frame->graphics != 0u)
        return;
    erase();
    for (row = 0u; row < UX_TEXT_ROWS && row < (type_unsigned_32)LINES; ++row) {
        type_unsigned_32 column;

        for (column = 0u; column < UX_TEXT_COLUMNS &&
            column < (type_unsigned_32)COLS; ++column) {
            type_unsigned_32 offset = row * UX_TEXT_COLUMNS + column;
            type_unsigned_8 character = frame->text[offset];

            if (has_colors()) attrset(COLOR_PAIR(ux_linux_console_color_pair(
                frame->attributes[offset])));
            mvaddch((C_INT)row, (C_INT)column,
                character >= 0x20u && character < 0x7fu ? character : ' ');
        }
    }
    if (frame->cursor_column >= 0 && frame->cursor_row >= 0 &&
        frame->cursor_column < COLS && frame->cursor_row < LINES)
        move(frame->cursor_row, frame->cursor_column);
    refresh();
}

static ux_run_result ux_linux_console_key(const ux_binding *binding, C_INT key)
{
    ux_action action;
    ux_event event = { 0 };

    if (key == ERR) return UX_RUN_CONTINUE;
    action = ux_actions_match(binding->actions, ux_linux_action_key(key), 0u);
    if (action != UX_ACTION_NONE)
        return binding->handle_action(binding->context, action,
            binding->input_sink);
    if (key >= 0x20 && key <= 0xff) {
        event.type = UX_EVENT_TEXT;
        event.data.text.scalar = (type_unsigned_32)key;
    } else if (!ux_linux_console_guest_key(key, &event)) return UX_RUN_CONTINUE;
    (C_VOID)binding->input_sink(binding->context, &event);
    return UX_RUN_CONTINUE;
}

ux_run_result ux_linux_run_console(const ux_binding *binding)
{
    ux_frame *frame;
    type_unsigned_32 displayed_generation = 0u;
    ux_run_result result = UX_RUN_STOPPED_RESULT;

    if (ux_binding_validate(binding) != TYPE_STATUS_OK ||
        !ux_linux_console_acquire()) return UX_RUN_ERROR_RESULT;
    if (initscr() == STD_NULL) {
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    if (raw() == ERR || noecho() == ERR || keypad(stdscr, TRUE) == ERR ||
        nodelay(stdscr, TRUE) == ERR) {
        endwin();
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    if (has_colors()) {
        type_unsigned_32 foreground;

        (C_VOID)start_color();
        for (foreground = 0u; foreground < 8u; ++foreground) {
            type_unsigned_32 background;

            for (background = 0u; background < 8u; ++background)
                (C_VOID)init_pair((short)(foreground * 8u + background),
                    (short)foreground, (short)background);
        }
    }
    frame = STD_CALLOC(1u, sizeof(*frame));
    if (frame == STD_NULL) {
        endwin();
        ux_linux_console_release();
        return UX_RUN_ERROR_RESULT;
    }
    while (binding->get_state(binding->context) == UX_RUN_RUNNING) {
        if (ux_mailbox_generation(binding->mailbox) != displayed_generation &&
            ux_mailbox_capture(binding->mailbox, frame) == TYPE_STATUS_OK) {
            if (ux_router_observe(binding->router, frame) == UX_TARGET_WINDOW) {
                result = UX_RUN_SWITCH_WINDOW;
                break;
            }
            ux_linux_console_paint(frame);
            displayed_generation = frame->sequence;
        }
        result = ux_linux_console_key(binding, getch());
        if (result != UX_RUN_CONTINUE) break;
        napms(16);
    }
    if (result == UX_RUN_STOPPED_RESULT &&
        binding->get_state(binding->context) == UX_RUN_PAUSED)
        result = UX_RUN_PAUSED_RESULT;
    STD_FREE(frame);
    endwin();
    ux_linux_console_release();
    return result;
}
#endif
