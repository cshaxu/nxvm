#include "type.h"

#include "app/ini_interface.h"

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
    vm_session_request request;

    if (!parse(valid, &request) || request.memory_bytes != 640u * 1024u ||
        STD_STRCMP(request.display, "window") || !request.console_control ||
        request.floppy_count != 2u || request.fixed_disk_count != 1u ||
        STD_STRCMP(request.floppy[0u], "unit-root/boot.img") ||
        STD_STRCMP(request.floppy[1u], "C:/owner/second.img") ||
        request.floppy_mode[0u] != LIB_STORAGE_MEDIUM_OVERLAY ||
        request.floppy_mode[1u] != LIB_STORAGE_MEDIUM_READONLY ||
        request.fixed_disk_mode[0u] != LIB_STORAGE_MEDIUM_DIRECT) return 1;
    if (parse(duplicate, &request) || parse(sparse, &request) || parse(forbidden, &request) || parse(bad_mode, &request))
        return 1;
    STD_PRINTF("M5:T533:S4:NXVM-INI:OK\n");
    return 0;
}
