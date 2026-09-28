# T539 S7: DMA Extraction Boundary

Baseline: 0ceb739f6. This is the implementation brief, not evidence that the
extraction is complete. Current owns admission and acceptance.

## Single Controller

`src/x86/devices/dma8237` owns one controller's address/count registers,
flip-flop, command/mode/mask/request/status, priority, temporary byte and
service phases. It depends only on Types. Its opaque instance has no page
register, peer controller, machine pointer, host resource or board clock.

The bounded interface provides construction/destruction/reset, register-byte
access, request/EOP inputs, eligible-request and active-channel observations,
priority selection, grant/release and one-input-clock service advancement.
Undefined register reads preserve the caller's bus byte. No full-state getter
is added to preserve private tests.

During advancement a borrowed cycle provider receives the local channel,
16-bit address and transfer kind. Normal device transfers use the board's
byte/word latch; memory-to-memory cycles exchange only a copied temporary byte.
Terminal notification follows existing completion ordering. A provider may
change an input synchronously but cannot recursively advance or destroy the
controller. Failed transfers must not commit address/count progress; this
does not promise rollback of irreversible external device effects.

## Board Responsibilities

NXVM retains page/spare latches, byte versus word expansion, physical address
decode, the existing provider binding tokens, device/RAM validation and
transaction order. The board also connects the AT controller pair, grants the
secondary cascade channel and supplies elapsed input clocks. XT must not need
a second dummy shared controller.

Preserve S6's first-service phase rule on both controllers. Do not move the
test-only accelerated transfer loop into the shared API. Move isolated chip
assertions to the chip's own tests; exercise board behavior through ports,
transfers and connected signals rather than exposing private register arrays.

The old reset handler clears page/spare latches together with chip state.
Preserve that existing board-visible behavior explicitly in board reset/master
clear routing during this structural extraction; it is not newly certified as
an Intel chip behavior. Priority rotation and cascade release must retain
their existing observable ordering; do not casually move them between grant
and transfer to make the API shorter.

The grant input explicitly distinguishes a board-delegated cascade slot from
a local transfer. Delegated grants rotate priority at grant, as the old board
did; ordinary transfers rotate at execution. No second execution loop or
private priority setter is exposed. The obsolete secondary software-request-0
clear is removed: the board excludes that software slot from arbitration and
derives cascade eligibility solely from the primary controller, so it never
caused a transfer or a readable status effect.

## Construction And Proof

Create the actual configured controller count before publishing port routes.
Propagate allocation/registration failure and roll routes back before freeing
their contexts. Destruction occurs after execution stops and while connected
recipients still exist. Add construction-failure regression coverage.

Preserve every current test scenario, including demand/single/block, software
requests, mask/priority, normal/compressed timing, autoinit, EOP, memory-to-memory,
page/lane behavior, stale bindings, FDC/Xebec and Model-40 integration. S6's
126 first-service matrix remains board-level evidence. Independent chip tests
must neither import NXVM headers nor reproduce a second PC DMA board.

Update the finite ledger only after this complete cutover is verified. The
remaining controller batches are not part of S7 acceptance.
