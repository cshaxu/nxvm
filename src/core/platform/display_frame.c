#include "core/platform/display_frame.h"

#include <stdatomic.h>
#include <string.h>

static atomic_flag vmPlatformDisplayLock = ATOMIC_FLAG_INIT;
static core_platform_display_frame vmPlatformDisplayFrame;

static void core_platform_display_lock(void)
{
    while (atomic_flag_test_and_set_explicit(&vmPlatformDisplayLock,
                                             memory_order_acquire)) {
    }
}

static void core_platform_display_unlock(void)
{
    atomic_flag_clear_explicit(&vmPlatformDisplayLock, memory_order_release);
}

void core_platform_display_initialize(void)
{
    core_platform_display_lock();
    memset(&vmPlatformDisplayFrame, 0, sizeof(vmPlatformDisplayFrame));
    vmPlatformDisplayFrame.columns = CORE_PLATFORM_DISPLAY_MAX_COLUMNS;
    vmPlatformDisplayFrame.rows = CORE_PLATFORM_DISPLAY_MAX_ROWS;
    core_platform_display_unlock();
}

void core_platform_display_publish(const core_platform_display_frame *frame)
{
    if (frame == NULL) return;
    core_platform_display_lock();
    vmPlatformDisplayFrame = *frame;
    core_platform_display_unlock();
}

void core_platform_display_capture(core_platform_display_frame *out_frame)
{
    if (out_frame == NULL) return;
    core_platform_display_lock();
    *out_frame = vmPlatformDisplayFrame;
    core_platform_display_unlock();
}
