#include "type.h"

#include "app-nxvm/product/ini_interface.h"
#include "app-nxvm/product/config.h"
#include "app-nxvm/product/startup.h"

static C_INT parse(C_CHAR *text, vm_session_request *request)
{ return vm_app_ini_parse("unit-root", "NXVM.ini", text, request) == TYPE_STATUS_OK; }

C_INT main(C_VOID)
{
    static C_CHAR valid[] =
        "[machine]\nmemory_kib=640\n[presentation]\ndisplay=window\n"
        "console_control=true\n[media]\nfloppy0=boot.img|overlay\n"
        "floppy1=C:/owner/second.img|readonly\nfixed_disk0=disk.img|direct\n";
    static C_CHAR duplicate[] = "[media]\nfloppy0=one.img|overlay\nfloppy0=two.img|overlay\n";
    static C_CHAR sparse[] = "[media]\nfloppy1=two.img|overlay\n";
    static C_CHAR forbidden[] = "[machine]\nprofile=ibm-5170-model-339\n";
    static C_CHAR bad_mode[] = "[media]\nfixed_disk0=disk.img|transient\n";
    static C_CHAR root[] = "[media]\nfloppy0=boot.img|overlay\n";
    vm_session_request request;
    vm_machine_config config;
    vm_session_request cleared_request = {0};
    vm_machine_config cleared_config = {0};
    C_CHAR executable_ini_path[1024];
    C_CHAR rejected_path[] = "unchanged";
    STD_SIZE_T executable_ini_length;

    STD_MEMSET(&request, 0xff, sizeof(request));
    if (vm_app_ini_load(STD_NULL, &request) != TYPE_STATUS_INVALID_ARGUMENT ||
        STD_MEMCMP(&request, &cleared_request, sizeof(request)) ||
        vm_app_ini_load("NXVM.ini", STD_NULL) != TYPE_STATUS_INVALID_ARGUMENT ||
        !parse(valid, &request) || request.memory_bytes != 640u * 1024u ||
        STD_STRCMP(request.display, "window") || !request.console_control ||
        request.floppy_count != 2u || request.fixed_disk_count != 1u ||
        STD_STRCMP(request.floppy[0u], "unit-root/boot.img") ||
        STD_STRCMP(request.floppy[1u], "C:/owner/second.img") ||
        request.floppy_mode[0u] != LIB_STORAGE_MEDIUM_OVERLAY ||
        request.floppy_mode[1u] != LIB_STORAGE_MEDIUM_READONLY ||
        request.fixed_disk_mode[0u] != LIB_STORAGE_MEDIUM_DIRECT) return 1;
    STD_MEMSET(&request, 0xff, sizeof(request));
    if (vm_app_ini_load("", &request) != TYPE_STATUS_FAULT ||
        STD_MEMCMP(&request, &cleared_request, sizeof(request))) return 1;
    STD_MEMSET(&request, 0xff, sizeof(request));
    if (parse(duplicate, &request) ||
        STD_MEMCMP(&request, &cleared_request, sizeof(request)) ||
        parse(sparse, &request) || parse(forbidden, &request) || parse(bad_mode, &request))
        return 1;
    if (vm_app_ini_parse("\\", "\\NXVM.ini", root, &request) != TYPE_STATUS_OK ||
        STD_STRCMP(request.floppy[0u], "\\boot.img")) return 1;
    if (vm_app_ini_executable_path(executable_ini_path, sizeof(executable_ini_path)) !=
            TYPE_STATUS_OK) return 1;
    executable_ini_length = STD_STRLEN(executable_ini_path);
    if (executable_ini_length < sizeof("NXVM.ini") ||
        STD_STRCMP(executable_ini_path + executable_ini_length - sizeof("NXVM.ini") + 1u,
            "NXVM.ini")) return 1;
    if (vm_app_ini_executable_path(rejected_path, sizeof("NXVM.ini")) !=
            TYPE_STATUS_INVALID_ARGUMENT || STD_STRCMP(rejected_path, "unchanged")) return 1;
    STD_MEMSET(&config, 0xff, sizeof(config));
    if (vm_app_configure_machine(STD_NULL, &config) != TYPE_STATUS_INVALID_ARGUMENT ||
        STD_MEMCMP(&config, &cleared_config, sizeof(config))) return 1;
    STD_PRINTF("M5:T533:S4:NXVM-INI:OK\n");
    return 0;
}
