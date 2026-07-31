#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "products/nxvm/session.h"

#define NXVM_PRODUCT_PATH_CAPACITY 512u

#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m5t7"
#endif

typedef struct nxvm_product_console_config {
    char fdd_path[NXVM_PRODUCT_PATH_CAPACITY];
    char hdd_path[NXVM_PRODUCT_PATH_CAPACITY];
    char record_path[NXVM_PRODUCT_PATH_CAPACITY];
    nxvm_product_nxvm_boot_target boot_target;
    int window_display;
} nxvm_product_console_config;

typedef struct nxvm_product_console_runtime {
    nxvm_product_nxvm_session session;
    int session_active;
    int running;
    int debugger_paused;
    int recording_active;
} nxvm_product_console_runtime;

static const nxvm_product_nxvm_media_identity unverified_fdd = {
    "user-fdd", 1u, "user-supplied"
};
static const nxvm_product_nxvm_media_identity unverified_hdd = {
    "user-hdd", 1u, "user-supplied"
};

static void print_banner(void)
{
    puts("Neko's x86 Virtual Machine [0.4.015d." NXVM_BUILD_SUFFIX "]");
    puts("Copyright (c) 2012-2014 Neko.");
    puts("");
}

static void print_help(void)
{
    puts("HELP [command]");
    puts("INFO");
    puts("DEVICE ram <KB>");
    puts("DEVICE display console|window");
    puts("DEVICE fdd insert <file>|remove");
    puts("DEVICE hdd connect <file>|disconnect");
    puts("SET boot fdd|hdd");
    puts("RECORD start <file>|stop");
    puts("START");
    puts("RESET");
    puts("STOP");
    puts("RESUME");
    puts("DEBUG");
    puts("EXIT");
}

static int equal_word(const char *left, const char *right)
{
    size_t index;

    if (left == NULL || right == NULL) return 0;
    for (index = 0u; left[index] != '\0' && right[index] != '\0'; ++index) {
        if (toupper((unsigned char)left[index]) != toupper((unsigned char)right[index])) {
            return 0;
        }
    }
    return left[index] == '\0' && right[index] == '\0';
}

static int copy_path(char *destination, const char *source)
{
    size_t length;

    if (source == NULL || source[0] == '\0') return 0;
    length = strlen(source);
    if (length >= NXVM_PRODUCT_PATH_CAPACITY) return 0;
    memcpy(destination, source, length + 1u);
    return 1;
}

static void print_info(const nxvm_product_console_config *config,
                       const nxvm_product_console_runtime *runtime)
{
    printf("profile: nxvm.machine.pc_at_builtin\n");
    printf("firmware: firmware.provider.pc_at_builtin\n");
    printf("boot: %s\n", config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD ? "hdd" : "fdd");
    printf("display: %s\n", config->window_display ? "window" : "console");
    printf("fdd: %s\n", config->fdd_path[0] == '\0' ? "none" : config->fdd_path);
    printf("hdd: %s\n", config->hdd_path[0] == '\0' ? "none" : config->hdd_path);
    printf("record: %s\n", config->record_path[0] == '\0' ? "off" : config->record_path);
    printf("lifecycle: %s\n", runtime->running ? "running" :
           runtime->debugger_paused ? "debugger-paused" : "ready");
}

static void destroy_session(nxvm_product_console_runtime *runtime)
{
    if (!runtime->session_active) return;
    if (runtime->recording_active) {
        nxvm_product_nxvm_pc_at_record_stop(&runtime->session.pc_at);
    }
    nxvm_product_nxvm_session_destroy(&runtime->session);
    memset(runtime, 0, sizeof(*runtime));
}

static int create_session(const nxvm_product_console_config *config,
                          nxvm_product_console_runtime *runtime)
{
    nxvm_product_nxvm_session_config session_config;
    nxvm_core_status status;

    if (runtime->session_active) return 1;
    if ((config->boot_target == NXVM_PRODUCT_NXVM_BOOT_FDD && config->fdd_path[0] == '\0') ||
        (config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD && config->hdd_path[0] == '\0')) {
        puts("Selected boot media is not configured.");
        return 0;
    }
    session_config.fdd_path = config->fdd_path[0] == '\0' ? NULL : config->fdd_path;
    session_config.fdd_identity = session_config.fdd_path == NULL ? NULL : &unverified_fdd;
    session_config.hdd_path = config->hdd_path[0] == '\0' ? NULL : config->hdd_path;
    session_config.hdd_identity = session_config.hdd_path == NULL ? NULL : &unverified_hdd;
    session_config.boot_target = config->boot_target;
    status = nxvm_product_nxvm_session_create(&runtime->session, &session_config);
    if (status != NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_pc_at_set_window_display(&runtime->session.pc_at,
            config->window_display) != NXVM_CORE_STATUS_OK) {
        puts("Could not create PC/AT session.");
        destroy_session(runtime);
        return 0;
    }
    runtime->session_active = 1;
    return 1;
}

static void refresh_running(nxvm_product_console_runtime *runtime)
{
    int running;

    if (!runtime->session_active ||
        nxvm_product_nxvm_pc_at_is_running(&runtime->session.pc_at, &running) !=
            NXVM_CORE_STATUS_OK) return;
    runtime->running = running;
}

static void start_session(const nxvm_product_console_config *config,
                          nxvm_product_console_runtime *runtime)
{
    if (runtime->running) {
        puts("Guest is already running.");
        return;
    }
    if (!create_session(config, runtime)) return;
    if (config->record_path[0] != '\0' &&
        !runtime->recording_active &&
        nxvm_product_nxvm_pc_at_record_start(&runtime->session.pc_at,
            config->record_path) != NXVM_CORE_STATUS_OK) {
        puts("Could not start instruction recording.");
        return;
    }
    if (config->record_path[0] != '\0') runtime->recording_active = 1;
    runtime->debugger_paused = 0;
    nxvm_product_nxvm_pc_at_run(&runtime->session.pc_at);
    refresh_running(runtime);
}

static void reset_session(const nxvm_product_console_config *config,
                          nxvm_product_console_runtime *runtime)
{
    if (runtime->running) {
        puts("Cannot reset while the guest is running.");
        return;
    }
    destroy_session(runtime);
    if (create_session(config, runtime) &&
        nxvm_product_nxvm_pc_at_reset(&runtime->session.pc_at) == NXVM_CORE_STATUS_OK) {
        puts("PC/AT session reset.");
    } else {
        puts("Could not reset PC/AT session.");
    }
}

static void stop_session(nxvm_product_console_runtime *runtime)
{
    if (!runtime->session_active || !runtime->running) {
        puts("No active guest is running.");
        return;
    }
    nxvm_product_nxvm_pc_at_request_stop(&runtime->session.pc_at);
    refresh_running(runtime);
    if (runtime->running) puts("Stop request is pending.");
    else puts("Guest stopped.");
}

static void resume_session(nxvm_product_console_runtime *runtime)
{
    if (!runtime->session_active || !runtime->debugger_paused) {
        puts("Resume requires a debugger-paused session.");
        return;
    }
    runtime->debugger_paused = 0;
    nxvm_product_nxvm_pc_at_resume(&runtime->session.pc_at);
    refresh_running(runtime);
}

static void debug_session(const nxvm_product_console_config *config,
                          nxvm_product_console_runtime *runtime)
{
    if (runtime->running) {
        stop_session(runtime);
        if (runtime->running) return;
    }
    if (!create_session(config, runtime)) return;
    puts("Entering NXVM hardware debugger. Use Q to return to the product Console.");
    if (nxvm_product_nxvm_pc_at_debug(&runtime->session.pc_at) == NXVM_CORE_STATUS_OK) {
        runtime->debugger_paused = 1;
    } else {
        puts("Could not enter the debugger.");
    }
}

static void reconfigure(nxvm_product_console_runtime *runtime)
{
    if (!runtime->running) destroy_session(runtime);
}

int main(void)
{
    char line[1024];
    char command[32];
    char item[32];
    char value[NXVM_PRODUCT_PATH_CAPACITY];
    nxvm_product_console_config config = { { 0 }, { 0 }, { 0 },
        NXVM_PRODUCT_NXVM_BOOT_FDD, 0 };
    nxvm_product_console_runtime runtime = { 0 };

    print_banner();
    puts("Please enter 'HELP' for information.");
    while (1) {
        fputs("\nConsole> ", stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        command[0] = '\0';
        item[0] = '\0';
        value[0] = '\0';
        if (sscanf(line, "%31s %31s %511[^\r\n]", command, item, value) == 0) continue;
        refresh_running(&runtime);
        if (equal_word(command, "help")) {
            print_help();
        } else if (equal_word(command, "info")) {
            print_info(&config, &runtime);
        } else if (equal_word(command, "exit")) {
            if (runtime.running) puts("Please stop NXVM before exit.");
            else break;
        } else if (equal_word(command, "start")) {
            start_session(&config, &runtime);
        } else if (equal_word(command, "reset")) {
            reset_session(&config, &runtime);
        } else if (equal_word(command, "stop")) {
            stop_session(&runtime);
        } else if (equal_word(command, "resume")) {
            resume_session(&runtime);
        } else if (equal_word(command, "debug")) {
            debug_session(&config, &runtime);
        } else if (equal_word(command, "set") && equal_word(item, "boot")) {
            if (runtime.running) puts("Cannot change boot target while the guest is running.");
            else if (equal_word(value, "fdd")) {
                reconfigure(&runtime); config.boot_target = NXVM_PRODUCT_NXVM_BOOT_FDD;
            } else if (equal_word(value, "hdd")) {
                reconfigure(&runtime); config.boot_target = NXVM_PRODUCT_NXVM_BOOT_HDD;
            } else puts("Boot target must be fdd or hdd.");
        } else if (equal_word(command, "record") && equal_word(item, "start")) {
            if (runtime.running) puts("Cannot change recording while the guest is running.");
            else if (copy_path(config.record_path, value)) {
                reconfigure(&runtime); puts("Instruction recording configured.");
            } else puts("Usage: RECORD start <file>");
        } else if (equal_word(command, "record") && equal_word(item, "stop")) {
            if (runtime.running) puts("Cannot change recording while the guest is running.");
            else {
                if (runtime.session_active && runtime.recording_active) {
                    nxvm_product_nxvm_pc_at_record_stop(&runtime.session.pc_at);
                    runtime.recording_active = 0;
                }
                config.record_path[0] = '\0';
                puts("Instruction recording disabled.");
            }
        } else if (equal_word(command, "device") && equal_word(item, "display")) {
            if (runtime.running) puts("Cannot change display while the guest is running.");
            else if (equal_word(value, "console")) { reconfigure(&runtime); config.window_display = 0; }
            else if (equal_word(value, "window")) { reconfigure(&runtime); config.window_display = 1; }
            else puts("Display must be console or window.");
        } else if (equal_word(command, "device") && equal_word(item, "fdd")) {
            char action[32] = { 0 };
            char path[NXVM_PRODUCT_PATH_CAPACITY] = { 0 };
            if (runtime.running) puts("Cannot change media while the guest is running.");
            else if (sscanf(value, "%31s %511[^\r\n]", action, path) == 2 &&
                     equal_word(action, "insert") && copy_path(config.fdd_path, path)) {
                reconfigure(&runtime); puts("Floppy disk configured.");
            } else if (equal_word(value, "remove")) {
                reconfigure(&runtime); config.fdd_path[0] = '\0'; puts("Floppy disk removed.");
            } else puts("Usage: DEVICE fdd insert <file>|remove");
        } else if (equal_word(command, "device") && equal_word(item, "hdd")) {
            char action[32] = { 0 };
            char path[NXVM_PRODUCT_PATH_CAPACITY] = { 0 };
            if (runtime.running) puts("Cannot change media while the guest is running.");
            else if (sscanf(value, "%31s %511[^\r\n]", action, path) == 2 &&
                     equal_word(action, "connect") && copy_path(config.hdd_path, path)) {
                reconfigure(&runtime); puts("Hard disk configured.");
            } else if (equal_word(value, "disconnect")) {
                reconfigure(&runtime); config.hdd_path[0] = '\0'; puts("Hard disk disconnected.");
            } else puts("Usage: DEVICE hdd connect <file>|disconnect");
        } else if (equal_word(command, "device") && equal_word(item, "ram")) {
            puts("RAM configuration remains fixed by nxvm.machine.pc_at_builtin.");
        } else {
            puts("Illegal command. Type HELP for available commands.");
        }
    }
    destroy_session(&runtime);
    return 0;
}
