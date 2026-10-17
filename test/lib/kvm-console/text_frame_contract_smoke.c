#include "lib/types/test.h"
#include "lib/kvm-console/frame_interface.h"
int main(void)
{
    kvm_console_text_frame text = {0};
    lib_test_assert(sizeof(kvm_console_text_frame) == 17116);
    /* Character presentation has no bitmap font-storage bound. */
    text.base = (kvm_text_frame){.text_columns = 80, .text_rows = 50,
        .font_height = 33, .cursor_mode = KVM_CURSOR_HOST,
        .cursor_column = -1, .cursor_row = 500};
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    text.base.text_columns++;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_UNSUPPORTED);
    text.base.text_columns = 0u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.base.text_columns = 1u;
    text.characters.primary[255] = 0xd800u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters.primary[255] = 0xd7ffu;
    text.characters.secondary[0] = 0xdfffu;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_INVALID_ARGUMENT);
    text.characters.secondary[0] = 0xe000u;
    lib_test_assert(kvm_console_text_frame_validate(&text) == LIB_STATUS_OK);
    return 0;
}
