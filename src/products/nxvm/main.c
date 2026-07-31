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
} nxvm_product_console_config;

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
    puts("HELP");
    puts("INFO");
    puts("DEVICE fdd insert <file>");
    puts("DEVICE hdd connect <file>");
    puts("SET boot fdd|hdd");
    puts("RECORD start <file>");
    puts("START");
    puts("EXIT");
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

static void print_info(const nxvm_product_console_config *config)
{
    printf("profile: nxvm.machine.pc_at_builtin\n");
    printf("firmware: firmware.provider.pc_at_builtin\n");
    printf("boot: %s\n", config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD ? "hdd" : "fdd");
    printf("fdd: %s\n", config->fdd_path[0] == '\0' ? "none" : config->fdd_path);
    printf("hdd: %s\n", config->hdd_path[0] == '\0' ? "none" : config->hdd_path);
    printf("record: %s\n", config->record_path[0] == '\0' ? "off" : config->record_path);
}

static int start_session(const nxvm_product_console_config *config)
{
    nxvm_product_nxvm_session_config session_config;
    nxvm_product_nxvm_session session;
    nxvm_core_status status;

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
    status = nxvm_product_nxvm_session_create(&session, &session_config);
    if (status != NXVM_CORE_STATUS_OK) {
        printf("Could not create PC/AT session (%d).\n", (int)status);
        return 0;
    }
    if (config->record_path[0] != '\0' &&
        nxvm_product_nxvm_pc_at_record_start(&session.pc_at,
            config->record_path) != NXVM_CORE_STATUS_OK) {
        puts("Could not start instruction recording.");
        nxvm_product_nxvm_session_destroy(&session);
        return 0;
    }
    nxvm_product_nxvm_pc_at_run(&session.pc_at);
    nxvm_product_nxvm_pc_at_record_stop(&session.pc_at);
    nxvm_product_nxvm_session_destroy(&session);
    return 1;
}

int main(void)
{
    char line[1024];
    char command[32];
    char item[32];
    char value[NXVM_PRODUCT_PATH_CAPACITY];
    nxvm_product_console_config config = { { 0 }, { 0 }, { 0 }, NXVM_PRODUCT_NXVM_BOOT_FDD };

    print_banner();
    puts("Please enter 'HELP' for information.");
    while (1) {
        fputs("\nConsole> ", stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        command[0] = '\0';
        item[0] = '\0';
        value[0] = '\0';
        if (sscanf(line, "%31s %31s %511[^\r\n]", command, item, value) == 0) continue;
        if (strcmp(command, "help") == 0 || strcmp(command, "HELP") == 0) {
            print_help();
        } else if (strcmp(command, "info") == 0 || strcmp(command, "INFO") == 0) {
            print_info(&config);
        } else if (strcmp(command, "exit") == 0 || strcmp(command, "EXIT") == 0) {
            break;
        } else if ((strcmp(command, "set") == 0 || strcmp(command, "SET") == 0) &&
                   (strcmp(item, "boot") == 0 || strcmp(item, "BOOT") == 0)) {
            if (strcmp(value, "fdd") == 0 || strcmp(value, "FDD") == 0) {
                config.boot_target = NXVM_PRODUCT_NXVM_BOOT_FDD;
            } else if (strcmp(value, "hdd") == 0 || strcmp(value, "HDD") == 0) {
                config.boot_target = NXVM_PRODUCT_NXVM_BOOT_HDD;
            } else puts("Boot target must be fdd or hdd.");
        } else if ((strcmp(command, "record") == 0 || strcmp(command, "RECORD") == 0) &&
                   (strcmp(item, "start") == 0 || strcmp(item, "START") == 0)) {
            if (copy_path(config.record_path, value)) puts("Instruction recording configured.");
            else puts("Usage: RECORD start <file>");
        } else if ((strcmp(command, "device") == 0 || strcmp(command, "DEVICE") == 0) &&
                   (strcmp(item, "fdd") == 0 || strcmp(item, "FDD") == 0)) {
            char action[32];
            char path[NXVM_PRODUCT_PATH_CAPACITY];
            if (sscanf(value, "%31s %511[^\r\n]", action, path) == 2 &&
                (strcmp(action, "insert") == 0 || strcmp(action, "INSERT") == 0) &&
                copy_path(config.fdd_path, path)) puts("Floppy disk configured.");
            else puts("Usage: DEVICE fdd insert <file>");
        } else if ((strcmp(command, "device") == 0 || strcmp(command, "DEVICE") == 0) &&
                   (strcmp(item, "hdd") == 0 || strcmp(item, "HDD") == 0)) {
            char action[32];
            char path[NXVM_PRODUCT_PATH_CAPACITY];
            if (sscanf(value, "%31s %511[^\r\n]", action, path) == 2 &&
                (strcmp(action, "connect") == 0 || strcmp(action, "CONNECT") == 0) &&
                copy_path(config.hdd_path, path)) puts("Hard disk configured.");
            else puts("Usage: DEVICE hdd connect <file>");
        } else if (strcmp(command, "start") == 0 || strcmp(command, "START") == 0) {
            (void)start_session(&config);
        } else {
            puts("Illegal command. Type HELP for available commands.");
        }
    }
    return 0;
}
