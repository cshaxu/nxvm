#ifndef UX_LINUX_CONSOLE_H
#define UX_LINUX_CONSOLE_H

#include "lib/ux/presenter.h"

#if !defined(_WIN32)
ux_run_result ux_linux_run_console(const ux_binding *binding);
#endif

#endif
