#ifndef UX_LINUX_PRESENTER_WAKE_H
#define UX_LINUX_PRESENTER_WAKE_H

#include "lib/ux/presenter.h"

#if !defined(_WIN32)
int ux_linux_presenter_wait_fd(const ux_presenter *presenter);
void ux_linux_presenter_consume(const ux_presenter *presenter);
#endif

#endif
