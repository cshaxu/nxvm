#include "lib/types/test.h"
#include "lib/console-broker/console_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/storage/medium_interface.h"
#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-window/window_interface.h"


int main(void)
{
    void *bytes = (void *)1;
    lib_size byte_count = 7u;
    lib_storage_file_writer *writer = (lib_storage_file_writer *)1;
    lib_storage_medium *medium = (lib_storage_medium *)1;
    base_sync_task *task = (base_sync_task *)1;
    console_broker *broker = (console_broker *)1;
    kvm_console *console = (kvm_console *)1;
    kvm_window *window = (kvm_window *)1;

    lib_test_assert(lib_storage_file_read_owned(LIB_NULL, 1u, &bytes, &byte_count) ==
        LIB_STATUS_INVALID_ARGUMENT && bytes == LIB_NULL);
    lib_test_assert(lib_storage_file_writer_open(LIB_NULL,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_INVALID_ARGUMENT &&
        writer == LIB_NULL);
    lib_test_assert(lib_storage_medium_open(LIB_NULL, LIB_STORAGE_MEDIUM_READONLY, &medium) ==
        LIB_STATUS_INVALID_ARGUMENT && medium == LIB_NULL);
    medium = (lib_storage_medium *)1;
    lib_test_assert(lib_storage_medium_create_overlay(LIB_NULL, 1u, &medium) ==
        LIB_STATUS_INVALID_ARGUMENT && medium == LIB_NULL);
    lib_test_assert(base_sync_task_create(LIB_NULL, LIB_NULL, &task) ==
        LIB_STATUS_INVALID_ARGUMENT && task == LIB_NULL);
    lib_test_assert(console_broker_create(&broker, LIB_NULL, CONSOLE_BROKER_RAW_EVENTS) ==
        LIB_STATUS_INVALID_ARGUMENT && broker == LIB_NULL);
    lib_test_assert(kvm_console_create(&console, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT &&
        console == LIB_NULL);
    lib_test_assert(kvm_window_create(&window, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT &&
        window == LIB_NULL);
    return 0;
}
