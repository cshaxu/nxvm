#ifndef COMMON_MACHINE_INPUT_QUEUE_H
#define COMMON_MACHINE_INPUT_QUEUE_H

#include "lib/kvm-base/event_interface.h"

typedef struct common_machine_input_queue common_machine_input_queue;

/* SoftPC runtime coordination: KVM producers publish copied KVM events, while
 * the sole machine executor consumes them.  This is deliberately product
 * runtime ownership, not a second KVM implementation. */
lib_status common_machine_input_queue_create(common_machine_input_queue **out_queue);
void common_machine_input_queue_destroy(common_machine_input_queue *queue);
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
