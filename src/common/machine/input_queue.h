#ifndef COMMON_MACHINE_INPUT_QUEUE_H
#define COMMON_MACHINE_INPUT_QUEUE_H

#include "lib/kvm-base/event_interface.h"
#include "lib/base/sync_interface.h"

#define COMMON_MACHINE_INPUT_QUEUE_CAPACITY 256u

typedef struct common_machine_input_queue {
    base_sync_mutex *lock;
    kvm_input_event entries[COMMON_MACHINE_INPUT_QUEUE_CAPACITY];
    lib_size head;
    lib_size tail;
} common_machine_input_queue;

/* KVM producers publish copied events; the sole machine executor consumes them.
 * Queue ownership belongs to Machine, not to a second KVM implementation. */
lib_status common_machine_input_queue_initialize(common_machine_input_queue *queue);
void common_machine_input_queue_dispose(common_machine_input_queue *queue);
lib_bool common_machine_input_queue_push(common_machine_input_queue *queue,
    const kvm_input_event *event);
lib_bool common_machine_input_queue_pop(common_machine_input_queue *queue,
    kvm_input_event *event);
lib_bool common_machine_input_queue_pending(common_machine_input_queue *queue);
/* A new cold VM run has no guest-input history.  This atomically discards
   records accepted for an earlier run; it does not affect the host monitor
   or its independent control queue. */
void common_machine_input_queue_clear(common_machine_input_queue *queue);

#endif
