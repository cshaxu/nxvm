#include "vm/product/recorder.h"
#include "lib/storage/file_interface.h"

static int recorder_text_contains(const char *text, const char *fragment)
{
    lib_size text_bytes;
    lib_size fragment_bytes;
    lib_size index;

    if (text == LIB_NULL || fragment == LIB_NULL) return LIB_FALSE;
    text_bytes = lib_text_length(text);
    fragment_bytes = lib_text_length(fragment);
    if (fragment_bytes > text_bytes) return LIB_FALSE;
    for (index = 0u; index <= text_bytes - fragment_bytes; ++index) {
        if (lib_memory_compare(text + index, fragment, fragment_bytes) == 0)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

int main(void)
{
    const char *path = "vm_product_recorder_smoke.log";
    vm_product_recorder *recorder = LIB_NULL;
    vm_machine_debug_observation observation = {
        .cs = 0xf000u, .ss = 0u, .cs_base = 0xf0000u,
        .eip = 0xfff0u, .instruction_cs = 0xf000u,
        .instruction_eip = 0xfff0u, .instruction_linear = 0xffff0u,
        .instruction_bytes = { 0x90u }, .instruction_byte_count = 1u
    };
    void *text = LIB_NULL;
    lib_size text_bytes = 0u;

    if (vm_product_recorder_create(&recorder) != TYPE_STATUS_OK ||
        vm_product_recorder_start(recorder, path) != TYPE_STATUS_OK)
        return 1;
    vm_product_recorder_observe(recorder, &observation);
    if (vm_product_recorder_stop(recorder) != TYPE_STATUS_OK) return 1;
    if (lib_storage_file_read_owned(path, 1023u, &text, &text_bytes) !=
            LIB_STATUS_OK || text_bytes == 0u) {
        lib_release(text);
        vm_product_recorder_destroy(recorder);
        return 1;
    }
    vm_product_recorder_destroy(recorder);
    if (!recorder_text_contains(text, "cs:eip=f000:0000fff0") ||
        !recorder_text_contains(text, "90")) {
        lib_release(text);
        return 1;
    }
    lib_release(text);
    return 0;
}
