#ifndef UX_WIN32_RUNNER_H
#define UX_WIN32_RUNNER_H

#include "lib/ux/presenter.h"

#ifdef _WIN32
ux_run_result ux_win32_run(const ux_binding *binding);
#endif

#endif
