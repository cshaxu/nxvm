#include "lib/types/test.h"
#include "lib/kvm-window/frame_interface.h"
int main(void)
{
    kvm_window_frame *frame = lib_allocate_zero(1u, sizeof(*frame));
    lib_test_assert(frame != LIB_NULL);
    frame->valid = LIB_TRUE;
    frame->graphics = 0u;
    frame->text.base.text_columns = 80u;
    frame->text.base.text_rows = 25u;
    frame->text.base.cells[0].glyph_index = 'X';
    lib_test_assert(kvm_window_frame_validate(frame) == LIB_STATUS_OK);

    lib_release(frame);
    return 0;
}
