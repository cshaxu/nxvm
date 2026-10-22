#ifndef KVM_BASE_FRAME_EXCHANGE_INTERFACE_H
#define KVM_BASE_FRAME_EXCHANGE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct kvm_frame_exchange kvm_frame_exchange;
typedef void (*kvm_frame_exchange_latest_observer)(void *context,
    const void *frame);
/* Leaf-owned payload retirement. Called only for an unheld slot. Release its
 * payload if its specification differs from latest; null latest releases all.
 * Must not reenter the exchange or perform host output. */
typedef void (*kvm_frame_exchange_retire)(void *slot, const void *latest);

/* One producer and one consumer, with four reusable frames:
 * producing, pending, processing and displayed.
 * Contents belong to the caller.
 * The producer alone writes writer(), outside the exchange lock. Publication
 * transfers that storage and supplies another writer without waiting for the
 * consumer. Create allocates only caller-sized slot metadata. Payloads are
 * prepared by their leaf on the writer, never copied by the exchange.
 * Destroy requires both participants and all observers to have stopped. */
lib_status kvm_frame_exchange_create(kvm_frame_exchange **out_exchange,
    lib_size frame_size, kvm_frame_exchange_retire retire);
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
/* Observe the latest publication while the exchange protects it. The callback
 * must not retain frame or call host/UI operations. False means no publication
 * was available; true means observer ran exactly once. */
lib_bool kvm_frame_exchange_observe_latest(kvm_frame_exchange *exchange,
    kvm_frame_exchange_latest_observer observer, void *context);
/* Producer-side generation invalidation; never mutates a consumer's input. */
void kvm_frame_exchange_invalidate(kvm_frame_exchange *exchange);

#endif
