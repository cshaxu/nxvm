#include "vm/platform/display_frame.h"

#include <stdatomic.h>
#include <string.h>

static atomic_flag vmPlatformDisplayLock = ATOMIC_FLAG_INIT;
static vm_platform_display_frame vmPlatformDisplayFrame;

static void vm_platform_display_lock(void)
{
    while (atomic_flag_test_and_set_explicit(&vmPlatformDisplayLock,
                                             memory_order_acquire)) {
    }
}

static void vm_platform_display_unlock(void)
{
    atomic_flag_clear_explicit(&vmPlatformDisplayLock, memory_order_release);
}

void vm_platform_display_initialize(void)
{
    vm_platform_display_lock();
    memset(&vmPlatformDisplayFrame, 0, sizeof(vmPlatformDisplayFrame));
    vmPlatformDisplayFrame.columns = VM_PLATFORM_DISPLAY_MAX_COLUMNS;
    vmPlatformDisplayFrame.rows = VM_PLATFORM_DISPLAY_MAX_ROWS;
    vm_platform_display_unlock();
}

void vm_platform_display_publish(const vm_platform_display_frame *frame)
{
    if (frame == NULL) return;
    vm_platform_display_lock();
    vmPlatformDisplayFrame = *frame;
    vm_platform_display_unlock();
}

void vm_platform_display_capture(vm_platform_display_frame *out_frame)
{
    if (out_frame == NULL) return;
    vm_platform_display_lock();
    *out_frame = vmPlatformDisplayFrame;
    vm_platform_display_unlock();
}
