#ifndef X86_PRODUCT_COMMAND_H
#define X86_PRODUCT_COMMAND_H

#include "emulator/product/monitor_interface.h"
#include "emulator/session/session_interface.h"
#include "emulator/machine/machine_interface.h"
#include "x86/debug/debug_interface.h"

#define X86_PRODUCT_COMMAND_TEXT_CAPACITY 2048u
#define X86_PRODUCT_COMMAND_PATH_CAPACITY 1024u

/* Optional App commands are deliberately outside the shared command grammar.
 * Session owns monitor admission, prompt timing and lifecycle dispatch; an
 * App extension can only recognize and complete its own command through the
 * copied Emulator result. */
typedef lib_bool (*x86_product_command_extension_submit)(void *context,
    emulator_machine *machine, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out);

/* Emulator Product recognizes the shared snapshot grammar. The selected App
 * owns the machine image and supplies its already-parsed SAVE/LOAD operation. */
typedef lib_bool (*x86_product_command_snapshot_submit)(void *context,
    emulator_machine *machine, emulator_product_monitor_command command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out);

typedef struct x86_product_command_extensions {
    void *context;
    x86_product_command_extension_submit submit;
    x86_product_command_snapshot_submit submit_snapshot;
    /* Command rows only: Product supplies the title, canonical rows, blank
     * separators and shared hotkey section around this App-owned middle. */
    emulator_product_help_map help;
} x86_product_command_extensions;

/* Product CLI state and callbacks; composition installs these directly. */
/* The app chooses its CLI. Session continues owning dispatch and Console I/O. */
typedef struct x86_product_command_context {
    emulator_machine *machine;
    x86_debug *debug;
    lib_bool debug_active;
    char debug_prompt[X86_DEBUG_PROMPT_CAPACITY];
    emulator_product_monitor_provider monitor;
    x86_product_command_extensions extensions;
} x86_product_command_context;

lib_status x86_product_command_initialize(x86_product_command_context *, emulator_machine *,
    emulator_session_display,
    const x86_product_command_extensions *);
void x86_product_command_dispose(x86_product_command_context *);
void x86_product_command_provider_submit_line(void *, emulator_session_machine_state,
    const char *, emulator_session_command_result *);
void x86_product_command_provider_note_runtime(void *, emulator_session_machine_state,
    emulator_session_machine_state, emulator_session_command_result *);

#endif
