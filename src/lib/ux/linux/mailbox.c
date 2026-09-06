#include "lib/base/base.h"
#include "lib/ux/mailbox_native.h"
#include "mailbox.h"

struct ux_mailbox_native {
    int read_fd;
    int write_fd;
};

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
    (void)fcntl(native_mailbox->read_fd, F_SETFL,
        fcntl(native_mailbox->read_fd, F_GETFL) | O_NONBLOCK);
    (void)fcntl(native_mailbox->write_fd, F_SETFL,
        fcntl(native_mailbox->write_fd, F_GETFL) | O_NONBLOCK);
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

void *ux_mailbox_native_wait_handle(const ux_mailbox_native *native_mailbox)
{
    return native_mailbox == NULL ? NULL :
        (void *)(intptr_t)(native_mailbox->read_fd + 1);
}

int ux_linux_mailbox_wait_fd(const ux_mailbox *mailbox)
{
    return (int)(intptr_t)ux_mailbox_native_wait_handle_for_mailbox(mailbox) - 1;
}

void ux_linux_mailbox_consume(const ux_mailbox *mailbox)
{
    char bytes[64];
    int fd = ux_linux_mailbox_wait_fd(mailbox);

    while (fd >= 0 && read(fd, bytes, sizeof(bytes)) > 0) {}
}
