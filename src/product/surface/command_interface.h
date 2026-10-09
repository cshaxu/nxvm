#ifndef PRODUCT_SURFACE_COMMAND_H
#define PRODUCT_SURFACE_COMMAND_H

#include "emulator/product/monitor_interface.h"
#include "emulator/session/session_interface.h"
#include "emulator/machine/machine_interface.h"
#include "product/debug/debug_interface.h"

#define PRODUCT_SURFACE_COMMAND_TEXT_CAPACITY 2048u
#define PRODUCT_SURFACE_COMMAND_PATH_CAPACITY 1024u

/* Optional App commands are deliberately outside the shared command grammar.
 * Session owns monitor admission, prompt timing and lifecycle dispatch; an
 * App extension can only recognize and complete its own command through the
 * copied Emulator result. */
typedef lib_bool (*product_surface_command_extension_submit)(void *context,
    emulator_machine *machine, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out);

typedef struct product_surface_command_extensions {
    void *context;
    product_surface_command_extension_submit submit;
    /* Command rows only: Product supplies the title, canonical rows, blank
     * separators and shared hotkey section around this App-owned middle. */
    const char *help_text;
} product_surface_command_extensions;

/* Product CLI state and callbacks; composition installs these directly. */
/* The app chooses its CLI. Session continues owning dispatch and Console I/O. */
typedef struct product_surface_command_context {
    emulator_machine *machine;
    product_debug *debug;
    lib_bool debug_active;
    char debug_prompt[PRODUCT_DEBUG_PROMPT_CAPACITY];
    emulator_product_monitor_provider monitor;
    product_surface_command_extensions extensions;
} product_surface_command_context;

lib_status product_surface_command_initialize(product_surface_command_context *, emulator_machine *,
    emulator_session_display,
    const product_surface_command_extensions *);
void product_surface_command_dispose(product_surface_command_context *);
void product_surface_command_provider_submit_line(void *, emulator_session_machine_state,
    const char *, emulator_session_command_result *);
void product_surface_command_provider_note_runtime(void *, emulator_session_machine_state,
    emulator_session_machine_state, emulator_session_command_result *);

#endif
