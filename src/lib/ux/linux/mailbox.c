#include "lib/base/base.h"
#include "lib/ux/internal/mailbox_native.h"
#include "mailbox.h"

struct ux_mailbox_native {
    int read_fd;
    int write_fd;
};

static int ux_linux_mailbox_make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);

    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

ux_mailbox_native *ux_mailbox_native_create(void)
{
    ux_mailbox_native *native_mailbox = calloc(1u, sizeof(*native_mailbox));
    int fds[2];

    if (native_mailbox == NULL || pipe(fds) != 0) {
        free(native_mailbox);
        return NULL;
    }
    native_mailbox->read_fd = fds[0];
    native_mailbox->write_fd = fds[1];
    if (!ux_linux_mailbox_make_nonblocking(native_mailbox->read_fd) ||
        !ux_linux_mailbox_make_nonblocking(native_mailbox->write_fd)) {
        (void)close(native_mailbox->read_fd);
        (void)close(native_mailbox->write_fd);
        free(native_mailbox);
        return NULL;
    }
    return native_mailbox;
}

void ux_mailbox_native_destroy(ux_mailbox_native *native_mailbox)
{
    if (native_mailbox == NULL) return;
    (void)close(native_mailbox->read_fd);
    (void)close(native_mailbox->write_fd);
    free(native_mailbox);
}

void ux_mailbox_native_signal(ux_mailbox_native *native_mailbox)
{
    static const char wake = 1;

    if (native_mailbox != NULL)
        (void)write(native_mailbox->write_fd, &wake, sizeof(wake));
}

int ux_linux_mailbox_wait_fd(const ux_mailbox *mailbox)
{
    ux_mailbox_native *native_mailbox = ux_mailbox_native_for_mailbox(mailbox);

    return native_mailbox == NULL ? -1 : native_mailbox->read_fd;
}

void ux_linux_mailbox_consume(const ux_mailbox *mailbox)
{
    char bytes[64];
    int fd = ux_linux_mailbox_wait_fd(mailbox);

    while (fd >= 0 && read(fd, bytes, sizeof(bytes)) > 0) {}
}
