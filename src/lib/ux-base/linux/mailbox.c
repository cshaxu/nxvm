#include "lib/base/base_interface.h"
#include "lib/ux-base/mailbox_wake.h"
#include "lib/ux-base/linux/mailbox_wake.h"

#include <stdlib.h>

struct ux_mailbox_wake {
    int read_fd;
    int write_fd;
};

static int ux_linux_mailbox_make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);

    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

ux_mailbox_wake *ux_mailbox_wake_create(void)
{
    ux_mailbox_wake *wake = calloc(1u, sizeof(*wake));
    int fds[2];

    if (wake == NULL || pipe(fds) != 0) {
        free(wake);
        return NULL;
    }
    wake->read_fd = fds[0];
    wake->write_fd = fds[1];
    if (!ux_linux_mailbox_make_nonblocking(wake->read_fd) ||
        !ux_linux_mailbox_make_nonblocking(wake->write_fd)) {
        (void)close(wake->read_fd);
        (void)close(wake->write_fd);
        free(wake);
        return NULL;
    }
    return wake;
}

void ux_mailbox_wake_destroy(ux_mailbox_wake *wake)
{
    if (wake == NULL) return;
    (void)close(wake->read_fd);
    (void)close(wake->write_fd);
    free(wake);
}

void ux_mailbox_wake_signal(ux_mailbox_wake *wake)
{
    static const char wake = 1;

    if (wake != NULL)
        (void)write(wake->write_fd, &wake, sizeof(wake));
}

int ux_linux_mailbox_wait_fd(const ux_mailbox_wake *wake)
{
    return wake == NULL ? -1 : wake->read_fd;
}

void ux_linux_mailbox_consume(const ux_mailbox_wake *wake)
{
    char bytes[64];
    int fd = ux_linux_mailbox_wait_fd(wake);

    while (fd >= 0 && read(fd, bytes, sizeof(bytes)) > 0) {}
}
