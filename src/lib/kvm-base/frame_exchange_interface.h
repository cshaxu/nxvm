#ifndef KVM_BASE_FRAME_EXCHANGE_INTERFACE_H
#define KVM_BASE_FRAME_EXCHANGE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct kvm_frame_exchange kvm_frame_exchange;

/* One producer and one consumer, with four reusable frames:
 * producing, pending, processing and displayed.
 * Contents belong to the caller.
 * The producer alone writes writer(), outside the exchange lock. Publication
 * transfers that storage and supplies another writer without waiting for the
 * consumer. No allocation or payload copy occurs after create.
 * Destroy requires both participants and all observers to have stopped. */
lib_status kvm_frame_exchange_create(kvm_frame_exchange **out_exchange,
    lib_size frame_size);
void kvm_frame_exchange_destroy(kvm_frame_exchange *exchange);
void *kvm_frame_exchange_writer(kvm_frame_exchange *exchange);
void kvm_frame_exchange_publish(kvm_frame_exchange *exchange);
/* One outstanding take. Both the returned input and optional previous displayed
 * input remain immutable until finish. Successful finish replaces displayed;
 * failure preserves it and requeues only an unsuperseded input.
 * No host drawing/output may hold the exchange lock. */
const void *kvm_frame_exchange_take(kvm_frame_exchange *exchange, const void **previous);
void kvm_frame_exchange_finish(kvm_frame_exchange *exchange, lib_bool success);
/* Requeue the consumer-held input only when no newer frame is pending.
 * Safe during output; a repeated request coalesces in the existing pending
 * role. Generation invalidation suppresses stale redraw requests. */
void kvm_frame_exchange_redraw(kvm_frame_exchange *exchange);
/* After consumer quiescence, return its input and allow a replacement consumer
 * to present the latest complete publication, without copying it. */
void kvm_frame_exchange_detach(kvm_frame_exchange *exchange);
/* A bounded observation, not another frame owner. Always unlock, even when
 * NULL is returned. Never retain the pointer or call host/UI operations while
 * locked. Used for publication metadata and explicit diagnostic copies. */
const void *kvm_frame_exchange_lock_latest(kvm_frame_exchange *exchange);
void kvm_frame_exchange_unlock(kvm_frame_exchange *exchange);
/* Producer-side generation invalidation; never mutates a consumer's input. */
void kvm_frame_exchange_invalidate(kvm_frame_exchange *exchange);

#endif
