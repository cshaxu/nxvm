/* Product-neutral display-mode notification bound by root composition. */
#ifndef NTVDM64_CORE_MACHINE_DISPLAY_H
#define NTVDM64_CORE_MACHINE_DISPLAY_H

typedef void (*core_machine_display_mode_notifier)(void *context);

void core_machine_display_bind(void *context,
    core_machine_display_mode_notifier mode_notifier);
void core_machine_display_notify_mode_changed(void);

#endif
