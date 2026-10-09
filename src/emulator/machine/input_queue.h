#ifndef EMULATOR_MACHINE_INPUT_QUEUE_H
#define EMULATOR_MACHINE_INPUT_QUEUE_H

#include "lib/kvm-base/event_interface.h"
#include "lib/base/sync_interface.h"

#define EMULATOR_MACHINE_INPUT_QUEUE_CAPACITY 256u

typedef struct emulator_machine_input_queue {
    base_sync_mutex *lock;
    kvm_input_event entries[EMULATOR_MACHINE_INPUT_QUEUE_CAPACITY];
    lib_size head;
    lib_size tail;
} emulator_machine_input_queue;

/* KVM producers publish copied events; the sole machine executor consumes them.
 * Queue ownership belongs to Machine, not to a second KVM implementation. */
lib_status emulator_machine_input_queue_initialize(emulator_machine_input_queue *queue);
void emulator_machine_input_queue_dispose(emulator_machine_input_queue *queue);
lib_bool emulator_machine_input_queue_push(emulator_machine_input_queue *queue,
    const kvm_input_event *event);
lib_bool emulator_machine_input_queue_pop(emulator_machine_input_queue *queue,
    kvm_input_event *event);
lib_bool emulator_machine_input_queue_pending(emulator_machine_input_queue *queue);
/* A new cold VM run has no guest-input history.  This atomically discards
   records accepted for an earlier run; it does not affect the host monitor
   or its independent control queue. */
void emulator_machine_input_queue_clear(emulator_machine_input_queue *queue);

#endif
