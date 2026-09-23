# T348 S3 DMA Request, Cascade, Priority, And EOP Contract

## One Logical Selection Owner

`dma_pending_requests` is the sole pre-grant projection for one 8237A. It
combines unmasked held hardware DREQ state with valid software-request state.
The latter is intentionally admitted only for block mode (and the channel-0
memory-to-memory initiation case), as required by the 8237A request-register
contract. A software request persists until terminal count, external EOP, or
reset; a held bound DREQ persists until its binding deasserts it.

At the PC/AT pair boundary, only a nonempty, enabled primary projection
derives the secondary controller's reserved local channel 0. That is the
channel-4 cascade path. Secondary software request bit 0 cannot synthesize
primary service, and channel 4 cannot receive a device binding. The secondary
controller can still grant its local channels 5--7 while the primary controller
is disabled; either controller's own disable gate prevents grants within its
actual ownership domain.

Fixed and rotating selection share `GetRegTopId`; each real controller updates
its own rotation position. Selecting the cascade channel advances only the
secondary cascade priority state, then lets the selected primary controller
advance its own state. No second scheduler or peer-owned request state was
introduced.

## External EOP Boundary

`core_machine_dma_request_terminate` is an opaque-binding operation. It
validates the same token, DMA pair, and channel ownership as assert/deassert,
and records EOP only when that exact channel is in service. Consequently a
device callback can terminate its active transfer without exposing a raw
controller pointer, terminating another channel, or manufacturing a second
truth source.

The ordinary-channel terminal path clears the software request, clears
in-service state, calls the owner terminal callback, restores base
address/count without setting TC or masking for auto-initialize, and otherwise
sets the channel TC and mask. This follows the ordinary 8237A EOP contract.
Memory-to-memory's dual-channel terminal and auto-initialize semantics remain
with S4's physical transaction/rollback owner rather than being conflated with
the one-bound-channel operation. Physical EOP pin voltage/pulse timing,
DREQ/DACK electrical polarity, READY wait states, and cycle duration remain
unimplemented electrical/bus observations for the later L3 bus-timing task;
the current binding API is a logical request contract, not a raw pin facade.

## Permanent Owner-Smoke Matrix

`core-machine-dma-channel-smoke` now proves:

| Contract | Permanent proof |
| --- | --- |
| Held hardware request | Demand and single modes receive one deterministic grant per tick while a binding remains asserted; masking and deassertion prevent publication. |
| Software request | A block-mode channel runs through the derived cascade from one primary software request and clears that request at terminal count; demand-mode software request has no transfer. Memory-to-memory starts from channel-0 software request alone. |
| Fixed/rotating priority | Concurrent primary channels 1/2 select 1 first in fixed mode and 1 then 2 in rotating mode; concurrent secondary channels 5/6 select 5 then 6 under secondary rotation. |
| Cascade isolation | Channel 4 binding is rejected and its secondary software-request bit cannot fabricate primary activity. |
| Controller disable | Primary disable blocks its cascaded service but not secondary channels 5--7; secondary disable blocks the whole paired arbitration path. |
| External EOP | A channel-3 provider requests termination during its own active callback. Exactly its first byte publishes; non-auto mode sets TC/mask and auto-init reloads current state without TC/masking. |

The same smoke retains S2's page/port/byte/word matrix and T269/T230 transfer
evidence. The S3 owner marker is `M5:T348:S3:DMA-REQUEST-CASCADE:OK`.

## Similar-Issue Sweep

All reads and writes of DMA request, status, mask, ISR, priority, command,
mode, `flagEOP`, binding token, primary/secondary selection, and channel-4
derivation are in `src/core/machine/dma.c`. All current production callers of
the opaque request operations are `machine.c` and FDC-owned paths; they retain
logical assert/deassert behavior and require no ABI rewrite. The machine
timeline invokes only `core_machine_dma_advance`, retaining T346's one-grant
boundary. No second DMA producer or scheduler was found.

## References

- [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf): DREQ, EOP, block/demand/cascade, software request,
  auto-initialize, fixed and rotating priority, and status semantics.
- [IBM PC/AT Technical Reference](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf): dual-controller channel-4 cascade and
  PC/AT wiring context.
