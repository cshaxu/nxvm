#include "lib/base/process_interface.h"

int main(void)
{
    char directory[1024];
    char unchanged[] = "unchanged";

    if (base_process_executable_directory(directory, sizeof(directory)) != LIB_STATUS_OK ||
        directory[0] == '\0') return 1;
    if (base_process_executable_directory(unchanged, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_text_compare(unchanged, "unchanged") != 0) return 1;
    return 0;
}
