#include <stdio.h>
#include <string.h>

#include "vm/product/recorder.h"

int main(void)
{
    const char *path = "vm_product_recorder_smoke.log";
    vm_product_recorder *recorder = NULL;
    vm_machine_debug_observation observation = {
        .cs = 0xf000u, .ss = 0u, .cs_base = 0xf0000u,
        .eip = 0xfff0u, .instruction_cs = 0xf000u,
        .instruction_eip = 0xfff0u, .instruction_linear = 0xffff0u,
        .instruction_bytes = { 0x90u }, .instruction_byte_count = 1u
    };
    FILE *file;
    char text[1024] = {0};

    (void)remove(path);
    if (vm_product_recorder_create(&recorder) != TYPE_STATUS_OK ||
        vm_product_recorder_start(recorder, path) != TYPE_STATUS_OK)
        return 1;
    vm_product_recorder_observe(recorder, &observation);
    if (vm_product_recorder_stop(recorder) != TYPE_STATUS_OK) return 1;
    file = fopen(path, "rb");
    if (file == NULL || fread(text, 1u, sizeof(text) - 1u, file) == 0u) {
        if (file != NULL) fclose(file);
        return 1;
    }
    fclose(file);
    (void)remove(path);
    vm_product_recorder_destroy(recorder);
    if (strstr(text, "cs:eip=f000:0000fff0") == NULL ||
        strstr(text, "90") == NULL) return 1;
    puts("M5:T527:S5:VM-PRODUCT-RECORDER:OK");
    return 0;
}
