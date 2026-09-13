#ifndef COMMON_UI_INTERFACE_H
#define COMMON_UI_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/ui-base/event_interface.h"
#include "lib/ui-base/frame_interface.h"
#include "lib/ui-base/hotkey_interface.h"

typedef struct common_ui common_ui;

typedef enum common_ui_target {
    COMMON_UI_TARGET_NONE,
    COMMON_UI_TARGET_CONSOLE,
    COMMON_UI_TARGET_WINDOW
} common_ui_target;

/* A presentation policy asks for one mechanical operation.  Common UI reports
 * its completed surface facts only after the Lib operation returns. */
typedef enum common_ui_action_kind {
    COMMON_UI_ACTION_CREATE_WINDOW,
    COMMON_UI_ACTION_DESTROY_WINDOW,
    COMMON_UI_ACTION_CREATE_RAW_CONSOLE,
    COMMON_UI_ACTION_DESTROY_RAW_CONSOLE,
    COMMON_UI_ACTION_BIND_RAW_CONSOLE,
    COMMON_UI_ACTION_BIND_MONITOR_CONSOLE,
    COMMON_UI_ACTION_SET_WINDOW_TITLE,
    COMMON_UI_ACTION_SET_MOUSE_CAPTURABLE,
    COMMON_UI_ACTION_RELEASE_MOUSE
} common_ui_action_kind;

typedef struct common_ui_action {
    common_ui_action_kind kind;
    union {
        const char *title;
        lib_bool mouse_capturable;
    } value;
} common_ui_action;

typedef struct common_ui_surface_facts {
    lib_bool window_exists;
    lib_bool raw_console_exists;
    lib_bool raw_console_current;
} common_ui_surface_facts;

typedef struct common_ui_completion {
    common_ui_action_kind action;
    common_ui_surface_facts facts;
} common_ui_completion;

typedef lib_status (*common_ui_input_sink)(void *context,
    const ui_input_event *event);
typedef lib_status (*common_ui_console_line_sink)(void *context,
    const char *line);
typedef void (*common_ui_failure_sink)(void *context, lib_u64 source_identity,
    lib_status status);

typedef struct common_ui_options {
    void *input_context;
    common_ui_input_sink input_sink;
    void *console_line_context;
    common_ui_console_line_sink console_line_sink;
    void *failure_context;
    common_ui_failure_sink failure_sink;
    ui_hotkey_registry hotkeys;
    const char *initial_window_title;
} common_ui_options;

typedef struct common_ui_plan {
    lib_bool target_changed;
    common_ui_target target;
    lib_bool mouse_capturable_changed;
    lib_bool mouse_capturable;
    lib_bool release_mouse;
    lib_bool frame_ready;
    ui_frame frame;
} common_ui_plan;

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options);
void common_ui_destroy(common_ui *ui);
lib_status common_ui_request_console_line(common_ui *ui);
lib_status common_ui_write_console(common_ui *ui, const char *text);
lib_status common_ui_apply_action(common_ui *ui, const common_ui_action *action,
    common_ui_completion *out_completion);
common_ui_surface_facts common_ui_get_surface_facts(const common_ui *ui);
/* Legacy target helpers only translate their single-surface request into the
 * action protocol. Session policy migrates to independent surface facts in S4. */
lib_status common_ui_set_target(common_ui *ui, common_ui_target target);
common_ui_target common_ui_get_target(const common_ui *ui);
lib_status common_ui_apply(common_ui *ui, const common_ui_plan *plan);
lib_status common_ui_set_window_title(common_ui *ui, const char *title);
lib_status common_ui_set_mouse_capturable(common_ui *ui, lib_bool capturable);
lib_status common_ui_release_mouse(common_ui *ui);

#endif
