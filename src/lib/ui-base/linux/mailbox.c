#include "lib/types/types_interface.h"
#include "lib/ui-base/mailbox_wake.h"
#include "lib/ui-base/linux/mailbox_wake.h"

#include <stdlib.h>

struct ui_mailbox_wake {
    int read_fd;
    int write_fd;
};

static int ui_linui_mailbox_make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);

    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

ui_mailbox_wake *ui_mailbox_wake_create(void)
{
    ui_mailbox_wake *wake = calloc(1u, sizeof(*wake));
    int fds[2];

    if (wake == NULL || pipe(fds) != 0) {
        free(wake);
        return NULL;
    }
    wake->read_fd = fds[0];
    wake->write_fd = fds[1];
    if (!ui_linui_mailbox_make_nonblocking(wake->read_fd) ||
        !ui_linui_mailbox_make_nonblocking(wake->write_fd)) {
        (void)close(wake->read_fd);
        (void)close(wake->write_fd);
        free(wake);
        return NULL;
    }
    return wake;
}

void ui_mailbox_wake_destroy(ui_mailbox_wake *wake)
{
    if (wake == NULL) return;
    (void)close(wake->read_fd);
    (void)close(wake->write_fd);
    free(wake);
}

void ui_mailbox_wake_signal(ui_mailbox_wake *wake)
{
    static const char wake = 1;

    if (wake != NULL)
        (void)write(wake->write_fd, &wake, sizeof(wake));
}

int ui_linui_mailbox_wait_fd(const ui_mailbox_wake *wake)
{
    return wake == NULL ? -1 : wake->read_fd;
}

void ui_linui_mailbox_consume(const ui_mailbox_wake *wake)
{
    char bytes[64];
    int fd = ui_linui_mailbox_wait_fd(wake);

    while (fd >= 0 && read(fd, bytes, sizeof(bytes)) > 0) {}
}
