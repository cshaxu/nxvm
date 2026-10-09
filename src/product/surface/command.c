#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "product/surface/command_interface.h"

static const char HELP_COMMANDS[] =
    "Control your virtual machine:\r\n"
    "  start          cold-reset and run the machine\r\n"
    "  resume         continue a paused machine\r\n"
    "  pause          request machine pause\r\n"
    "  stop           stop execution\r\n"
    "  reset          cold-reset and pause at firmware entry\r\n"
    "  help           show this help\r\n"
    "  debug          enter debugger (q returns to monitor)\r\n"
    "  exit           quit\r\n";
static const char HELP_HOTKEYS[] =
    "While the guest is running:\r\n"
    "  Ctrl+Alt+P     pause or resume\r\n"
    "  Ctrl+Alt+D     send Ctrl+Alt+Del to the guest\r\n"
    "  Ctrl+Alt+F     send Alt+Enter to the guest\r\n"
    "  Ctrl+Alt+T     send Alt+Tab to the guest\r\n"
    "  Ctrl+Alt+M     release captured mouse\r\n";

static void clear(product_surface_command_effect *e) { lib_memory_set(e, 0, sizeof(*e)); }
static void text(product_surface_command_effect *e, const char *s) { (void)lib_c_snprintf(e->text, sizeof(e->text), "%s", s); }
static void message(char *out, lib_size capacity, const char *value)
{
    (void)lib_c_snprintf(out, capacity, "%s\r\n\r\n", value);
}
static void help(const product_surface_command_session *s, product_surface_command_effect *e)
{
    (void)s;
    (void)lib_c_snprintf(e->text, sizeof(e->text), "%s\r\n", HELP_COMMANDS);
}
/* Product readiness remains true until a command reserves a transition.
 * Emulator alone admits one pending line; callbacks never consume readiness. */
static void prompt(product_surface_command_session *s) { s->prompt_due = 1; }
static void outcome(product_surface_command_session *s, const char *value)
{
    message(s->pending_monitor_text, sizeof(s->pending_monitor_text), value);
    prompt(s);
}
static void reject(product_surface_command_session *s, product_surface_command_effect *e, const char *value)
{
    message(e->text, sizeof(e->text), value);
    prompt(s);
}
static lib_bool whitespace(char value)
{
    return value == ' ' || value == '\t' || value == '\n' ||
           value == '\r' || value == '\f' || value == '\v';
}
static char lowercase(char value)
{
    return value >= 'A' && value <= 'Z' ?
        (char)(value + ('a' - 'A')) : value;
}
static char *trim(char *s)
{
    char *e;
    while (*s && whitespace(*s))
        ++s;
    e = s + lib_text_length(s);
    while (e != s && whitespace(e[-1]))
        --e;
    *e = 0;
    return s;
}
static void lower(char *s)
{
    while (*s)
    {
        *s = lowercase(*s);
        ++s;
    }
}
static void accept(product_surface_command_session *session,
                   app_lifecycle_request request)
{
    session->pending_request = request;
    session->dispatch_pending = 1;
    session->transition_pending = 1;
    session->prompt_due = 0;
}

static void lifecycle(product_surface_command_session *s, app_monitor_state state,
                      const char *c, product_surface_command_effect *e)
{
    if (s->transition_pending || s->dispatch_pending)
    {
        reject(s, e, "Machine state transition is in progress.");
        return;
    }
    if (!lib_text_compare(c, "start"))
    {
        if (state == APP_MONITOR_STOPPED)
        {
            accept(s, APP_LIFECYCLE_REQUEST_START);
        }
        else
            reject(s, e, state == APP_MONITOR_PAUSED ? "Machine is paused; use resume, reset, or stop." : "Machine is already running; use pause, reset, or stop.");
    }
    else if (!lib_text_compare(c, "pause"))
    {
        if (state == APP_MONITOR_RUNNING)
            accept(s, APP_LIFECYCLE_REQUEST_PAUSE);
        else
            reject(s, e, state == APP_MONITOR_PAUSED ? "Machine is paused; use resume, reset, or stop." : "Machine is stopped; use start or reset.");
    }
    else if (!lib_text_compare(c, "resume"))
    {
        if (state == APP_MONITOR_PAUSED)
            accept(s, APP_LIFECYCLE_REQUEST_RESUME);
        else
            reject(s, e, state == APP_MONITOR_RUNNING ? "Machine is already running; use pause, reset, or stop." : "Machine is stopped; use start or reset.");
    }
    else if (!lib_text_compare(c, "reset"))
    {
        accept(s, APP_LIFECYCLE_REQUEST_RESET);
    }
    else if (!lib_text_compare(c, "stop"))
    {
        if (state == APP_MONITOR_RUNNING || state == APP_MONITOR_PAUSED)
        {
            accept(s, APP_LIFECYCLE_REQUEST_STOP);
        }
        else
            reject(s, e, "Machine is stopped; use start or reset.");
    }
    else {
        e->unrecognized = LIB_TRUE;
        reject(s, e, "Unknown command.");
    }
}

void product_surface_command_session_initialize(product_surface_command_session *s, emulator_session_display display)
{
    lib_memory_set(s, 0, sizeof(*s));
    s->display = display;
}
const char *product_surface_command_hotkey_help(void) { return HELP_HOTKEYS; }
void product_surface_command_session_open(product_surface_command_session *s, product_surface_command_effect *e)
{
    clear(e);
    help(s, e);
    prompt(s);
}
void product_surface_command_session_reject_line(product_surface_command_session *s, product_surface_command_effect *e)
{
    clear(e);
    reject(s, e, "Command is too long.");
}
void product_surface_command_session_submit_line(product_surface_command_session *s, app_monitor_state state,
                                     const char *line, product_surface_command_effect *e)
{
    char b[PRODUCT_SURFACE_COMMAND_TEXT_CAPACITY], *c, *a;
    lib_size n;
    clear(e);
    if (!line || (n = lib_text_length(line)) >= sizeof(b))
    {
        reject(s, e, line ? "Command is too long." : "Unknown command.");
        return;
    }
    lib_memory_copy(b, line, n + 1);
    c = trim(b);
    a = c;
    while (*a && !whitespace(*a))
        ++a;
    if (*a)
        *a++ = 0;
    a = trim(a);
    lower(c);
    if (!*c)
    {
        prompt(s);
        return;
    }
    if (!lib_text_compare(c, "help"))
    {
        help(s, e);
        e->action = PRODUCT_SURFACE_COMMAND_ACTION_HELP;
        prompt(s);
        return;
    }
    if (!lib_text_compare(c, "exit"))
    {
        e->exit_requested = 1;
        return;
    }
    if (!lib_text_compare(c, "debug") && !*a)
    {
        e->action = PRODUCT_SURFACE_COMMAND_ACTION_DEBUG;
        prompt(s);
        return;
    }
    /* The parser does not own machine state.  It receives control's current
       stable fact for the one command validation below. */
    if (state == APP_MONITOR_ERROR)
    {
        reject(s, e, "Machine has failed; exit and restart the program.");
        return;
    }
    lifecycle(s, state, c, e);
}
app_lifecycle_request product_surface_command_session_take_request(product_surface_command_session *s)
{
    app_lifecycle_request request;
    if (s == NULL || !s->dispatch_pending)
        return APP_LIFECYCLE_REQUEST_NONE;
    request = s->pending_request;
    s->pending_request = APP_LIFECYCLE_REQUEST_NONE;
    s->dispatch_pending = 0;
    return request;
}
int product_surface_command_session_begin_external(product_surface_command_session *s,
                                       app_monitor_state state, app_lifecycle_request request)
{
    if (s == NULL || s->transition_pending ||
        request == APP_LIFECYCLE_REQUEST_NONE)
        return 0;
    if ((request == APP_LIFECYCLE_REQUEST_PAUSE &&
         state != APP_MONITOR_RUNNING) ||
        (request == APP_LIFECYCLE_REQUEST_RESUME &&
         state != APP_MONITOR_PAUSED) ||
        (request == APP_LIFECYCLE_REQUEST_STOP &&
         state != APP_MONITOR_RUNNING && state != APP_MONITOR_PAUSED))
        return 0;
    s->transition_pending = 1;
    s->prompt_due = 0;
    return 1;
}
void product_surface_command_session_note_runtime(product_surface_command_session *s,
                                      app_monitor_state prior, emulator_machine_state state, product_surface_command_effect *e)
{
    if (s == NULL || e == NULL)
        return;
    clear(e);
    if (state == EMULATOR_MACHINE_RESET_COMPLETED)
    {
        s->transition_pending = 0;
        outcome(s, "Machine reset and paused.");
        return;
    }
    if (state == EMULATOR_MACHINE_PAUSED && prior != APP_MONITOR_PAUSED)
    {
        s->transition_pending = 0;
        outcome(s, "Machine paused.");
    }
    else if (state == EMULATOR_MACHINE_RUNNING)
    {
        s->transition_pending = 0;
        if (prior == APP_MONITOR_STOPPED)
            outcome(s, "Machine started.");
        else if (prior == APP_MONITOR_PAUSED)
            outcome(s, "Machine resumed.");
        else if (s->display == EMULATOR_SESSION_DISPLAY_WINDOW)
            prompt(s);
    }
    else if (state == EMULATOR_MACHINE_STOPPED &&
             prior != APP_MONITOR_STOPPED)
    {
        s->transition_pending = 0;
        outcome(s, "Machine stopped.");
    }
    else if (state == EMULATOR_MACHINE_ERROR)
    {
        s->transition_pending = 0;
        outcome(s, "Machine error.");
    }
}

void product_surface_command_session_note_broker(product_surface_command_session *s,
                                     app_monitor_state state, int vm,
                                     int monitor_running_surface)
{
    if (s == NULL)
        return;
    if (vm && state == APP_MONITOR_RUNNING)
    {
        s->prompt_due = 0;
        s->pending_monitor_text[0] = '\0';
    }
    else if (!vm && state == APP_MONITOR_RUNNING &&
             monitor_running_surface)
        prompt(s);
}

void product_surface_command_session_note_monitor_current(product_surface_command_session *s,
                                              int current, product_surface_command_effect *e)
{
    if (s == NULL || e == NULL)
        return;
    clear(e);
    if (!s->prompt_due || !current)
        return;
    text(e, s->pending_monitor_text);
    s->pending_monitor_text[0] = '\0';
    e->arm_prompt = 1;
}

_Static_assert(EMULATOR_SESSION_PROMPT_CAPACITY >= PRODUCT_DEBUG_PROMPT_CAPACITY,
               "Session prompt must hold debugger continuation prompts");

/* product/ owns product command policy. emulator/session owns its neutral copied
 * completion facts; convert explicitly at this one composition boundary.
 * The numeric enum values are deliberately not a cross-component contract. */
static emulator_machine_state app_machine_completed_state(
    emulator_session_machine_state state)
{
    switch (state)
    {
    case EMULATOR_SESSION_MACHINE_RUNNING:
        return EMULATOR_MACHINE_RUNNING;
    case EMULATOR_SESSION_MACHINE_PAUSED:
        return EMULATOR_MACHINE_PAUSED;
    case EMULATOR_SESSION_MACHINE_ERROR:
        return EMULATOR_MACHINE_ERROR;
    case EMULATOR_SESSION_MACHINE_RESET_COMPLETED:
        return EMULATOR_MACHINE_RESET_COMPLETED;
    default:
        return EMULATOR_MACHINE_STOPPED;
    }
}

static app_monitor_state product_surface_command_state(emulator_session_machine_state state)
{
    switch (state)
    {
    case EMULATOR_SESSION_MACHINE_RUNNING:
        return APP_MONITOR_RUNNING;
    case EMULATOR_SESSION_MACHINE_PAUSED:
        return APP_MONITOR_PAUSED;
    case EMULATOR_SESSION_MACHINE_ERROR:
        return APP_MONITOR_ERROR;
    default:
        return APP_MONITOR_STOPPED;
    }
}

static emulator_session_request app_session_request(app_lifecycle_request request)
{
    switch (request)
    {
    case APP_LIFECYCLE_REQUEST_START:
        return EMULATOR_SESSION_REQUEST_START;
    case APP_LIFECYCLE_REQUEST_RESUME:
        return EMULATOR_SESSION_REQUEST_RESUME;
    case APP_LIFECYCLE_REQUEST_PAUSE:
        return EMULATOR_SESSION_REQUEST_PAUSE;
    case APP_LIFECYCLE_REQUEST_STOP:
        return EMULATOR_SESSION_REQUEST_STOP;
    case APP_LIFECYCLE_REQUEST_RESET:
        return EMULATOR_SESSION_REQUEST_RESET;
    default:
        return EMULATOR_SESSION_REQUEST_NONE;
    }
}

static app_lifecycle_request app_lifecycle_request_from_session(
    emulator_session_request request)
{
    switch (request)
    {
    case EMULATOR_SESSION_REQUEST_START:
        return APP_LIFECYCLE_REQUEST_START;
    case EMULATOR_SESSION_REQUEST_RESUME:
        return APP_LIFECYCLE_REQUEST_RESUME;
    case EMULATOR_SESSION_REQUEST_PAUSE:
        return APP_LIFECYCLE_REQUEST_PAUSE;
    case EMULATOR_SESSION_REQUEST_STOP:
        return APP_LIFECYCLE_REQUEST_STOP;
    case EMULATOR_SESSION_REQUEST_RESET:
        return APP_LIFECYCLE_REQUEST_RESET;
    default:
        return APP_LIFECYCLE_REQUEST_NONE;
    }
}

static void product_surface_command_set_prompt(const product_surface_command_context *command,
                                   emulator_session_command_result *out)
{
    if (command == NULL || out == NULL)
        return;
    (void)lib_c_snprintf(out->prompt, sizeof(out->prompt), "%s",
        command->debug_active ? command->debug_prompt : "> ");
}

static void product_surface_command_copy_effect(product_surface_command_context *command,
                                    emulator_session_command_result *out, const product_surface_command_effect *effect)
{
    if (command == NULL || out == NULL || effect == NULL)
        return;
    *out = (emulator_session_command_result){0};
    (void)lib_c_snprintf(out->text, sizeof(out->text), "%s", effect->text);
    out->exit_requested = effect->exit_requested != 0;
    out->arm_prompt = effect->arm_prompt != 0;
    product_surface_command_set_prompt(command, out);
}

static void product_surface_command_append_help(const product_surface_command_context *command,
    emulator_session_command_result *out)
{
    lib_size used;

    if (command == LIB_NULL || out == LIB_NULL) return;
    used = lib_text_length((const char *)out->text);
    if (used >= sizeof(out->text)) return;
    if (command->extensions.help_text != LIB_NULL) {
        (void)lib_c_snprintf((char *)out->text + used, sizeof(out->text) - used,
            "%s", command->extensions.help_text);
        used = lib_text_length((const char *)out->text);
    }
    if (used < sizeof(out->text))
        (void)lib_c_snprintf((char *)out->text + used, sizeof(out->text) - used,
            "\r\n%s\r\n", HELP_HOTKEYS);
}

void product_surface_command_provider_open(void *opaque,
                               emulator_session_command_result *out)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_effect effect = {0};
    product_surface_command_session_open(&command->session, &effect);
    product_surface_command_copy_effect(command, out, &effect);
    product_surface_command_append_help(command, out);
}

void product_surface_command_provider_reject_line(void *opaque,
                                      emulator_session_command_result *out)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_effect effect = {0};
    product_surface_command_session_reject_line(&command->session, &effect);
    product_surface_command_copy_effect(command, out, &effect);
}

static void product_surface_command_copy_debug(product_surface_command_context *command,
                                   emulator_session_machine_state state, const product_debug_result *result,
                                   emulator_session_command_result *out)
{
    out->detail = result->text;
    (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "%s", result->prompt);
    if (!result->keep_active)
    {
        product_debug_close(command->debug);
        command->debug_active = LIB_FALSE;
        command->debug_completed_pending = LIB_FALSE;
    }
    if (result->lifecycle_request == PRODUCT_DEBUG_LIFECYCLE_RESUME &&
        product_surface_command_session_begin_external(&command->session,
                                           product_surface_command_state(state), APP_LIFECYCLE_REQUEST_RESUME))
        out->request = EMULATOR_SESSION_REQUEST_RESUME;
    else if (result->lifecycle_request != PRODUCT_DEBUG_LIFECYCLE_NONE)
        (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug lifecycle request is not applicable.\r\n\r\n");
}

void product_surface_command_provider_submit_line(void *opaque,
                                      emulator_session_machine_state state, const char *line,
                                      emulator_session_command_result *out)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_effect effect = {0};
    if (command->debug_active)
    {
        product_debug_result result = {0};
        command->debug_completed_pending = LIB_FALSE;
        lib_status status = product_debug_submit_line(command->debug, line, &result);
        *out = (emulator_session_command_result){0};
        if (status != LIB_STATUS_OK)
        {
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug command failed.\r\n\r\n");
            emulator_machine_debug_cancel(command->machine);
            (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
        }
        else
        {
            product_surface_command_copy_debug(command, state, &result, out);
        }
        command->session.prompt_due = out->request == EMULATOR_SESSION_REQUEST_NONE;
        product_surface_command_set_prompt(command, out);
        return;
    }
    product_surface_command_session_submit_line(&command->session, product_surface_command_state(state), line,
                                    &effect);
    if (effect.unrecognized && command->extensions.submit != LIB_NULL) {
        *out = (emulator_session_command_result){0};
        if (command->extensions.submit(command->extensions.context,
                command->machine, state, line, out)) {
            product_surface_command_set_prompt(command, out);
            return;
        }
    }
    if (effect.action == PRODUCT_SURFACE_COMMAND_ACTION_DEBUG)
    {
        if (product_debug_open(command->debug, command->machine) == LIB_STATUS_OK)
        {
            command->debug_active = LIB_TRUE;
            (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
            effect.text[0] = '\0';
        }
        else
            (void)lib_c_snprintf(effect.text, sizeof(effect.text), "Cannot open debugger.\r\n\r\n");
    }
    else if (effect.action == PRODUCT_SURFACE_COMMAND_ACTION_HELP)
    {
        product_surface_command_copy_effect(command, out, &effect);
        product_surface_command_append_help(command, out);
        return;
    }
    product_surface_command_copy_effect(command, out, &effect);
    out->request = app_session_request(
        product_surface_command_session_take_request(&command->session));
}

lib_bool product_surface_command_provider_begin_external(void *opaque,
                                             emulator_session_machine_state state, emulator_session_request request)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    return product_surface_command_session_begin_external(&command->session,
                                              product_surface_command_state(state), app_lifecycle_request_from_session(request)) != 0;
}

void product_surface_command_provider_note_runtime(void *opaque,
                                       emulator_session_machine_state prior, emulator_session_machine_state completed,
                                       emulator_session_command_result *out)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_effect effect = {0};
    product_surface_command_session_note_runtime(&command->session, product_surface_command_state(prior),
                                     app_machine_completed_state(completed), &effect);
    product_surface_command_copy_effect(command, out, &effect);
    if (command->debug_active)
    {
        command->debug_completed_pending = LIB_FALSE;
        product_debug_machine_state state = completed == EMULATOR_SESSION_MACHINE_PAUSED ? PRODUCT_DEBUG_MACHINE_PAUSED : completed == EMULATOR_SESSION_MACHINE_RUNNING ? PRODUCT_DEBUG_MACHINE_RUNNING
                                                                                                                                                                  : PRODUCT_DEBUG_MACHINE_STOPPED;
        product_debug_result result = {0};
        if (product_debug_observe_machine(command->debug, state, LIB_STATUS_OK, &result) != LIB_STATUS_OK)
        {
            (void)lib_c_snprintf(out->text, sizeof(out->text), "Debug command failed.\r\n\r\n");
            emulator_machine_debug_cancel(command->machine);
            (void)lib_c_snprintf(command->debug_prompt, sizeof(command->debug_prompt), "-");
        }
        else if (result.prompt_ready)
        {
            command->debug_completed = result;
            command->debug_completed_pending = LIB_TRUE;
        }
    }
}

void product_surface_command_provider_note_broker(void *opaque,
                                      emulator_session_machine_state state, lib_bool vm_console_current,
                                      lib_bool monitor_running_surface)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_session_note_broker(&command->session, product_surface_command_state(state),
                                    vm_console_current != 0, monitor_running_surface != 0);
}

void product_surface_command_provider_note_monitor_current(void *opaque,
                                               lib_bool current, emulator_session_command_result *out)
{
    product_surface_command_context *command = (product_surface_command_context *)opaque;
    product_surface_command_effect effect = {0};
    product_surface_command_session_note_monitor_current(&command->session, current != 0,
                                             &effect);
    product_surface_command_copy_effect(command, out, &effect);
    if (current && command->debug_completed_pending)
    {
        command->debug_completed_pending = LIB_FALSE;
        product_surface_command_copy_debug(command, EMULATOR_SESSION_MACHINE_PAUSED,
                               &command->debug_completed, out);
        out->arm_prompt = out->request == EMULATOR_SESSION_REQUEST_NONE;
    }
    product_surface_command_set_prompt(command, out);
}

lib_status product_surface_command_initialize(product_surface_command_context *command,
                                  emulator_machine *machine, emulator_session_display display,
                                  const product_surface_command_extensions *extensions)
{
    if (command == NULL || machine == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(command, 0, sizeof(*command));
    command->machine = machine;
    if (extensions != LIB_NULL) command->extensions = *extensions;
    product_surface_command_session_initialize(&command->session, display);
    return product_debug_create(&command->debug);
}

void product_surface_command_dispose(product_surface_command_context *command)
{
    if (command != NULL)
        product_debug_destroy(command->debug);
}
