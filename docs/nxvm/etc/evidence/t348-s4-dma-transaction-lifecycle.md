# T348 S4 DMA Transaction And Lifecycle Reconciliation

## Validation Before Publication

`dma_physical_address` owns ordinary byte/word physical address formation;
`dma_memory_route_is_valid` uses the frozen checked-memory route query. Ordinary
device-to-memory and memory-to-device transfers query their single target
before calling a device provider, reading/writing the latch, or advancing
count/address. Memory-to-memory queries both source-read and destination-write
routes before reading either route or changing its temporary latch, channel-1
count, current addresses, software request, or terminal state.

The query is existing memory metadata infrastructure: it calls no provider
data callback and does not create a second memory path. Actual data access
continues exclusively through the checked physical read/write owner after that
preflight. A rejected route clears the transient selected-service state while
leaving the request pending for the next valid configuration; it does not
fabricate terminal count or consume a provider observation.

## Terminal And Lifecycle Ownership

`dma_complete_transfer` is the single terminal publication owner. Ordinary
channels clear their software request, clear service, call only their owner
terminal callback, and either restore auto-init state without TC/masking or
set TC and mask. Memory-to-memory is explicitly two-channel: channel 1 owns
the count/TC result; both channel 0 and 1 software requests clear; both current
register pairs are independently restored/masked according to their actual
auto-init bit; and any bound terminal callbacks receive their own channel
completion. This replaces the old construction that applied one ordinary
channel's completion rules to a two-register mechanism.

An active channel-0 binding can signal external EOP between M2M primitives.
The already committed byte remains published, channel 1 alone records TC, and
the unexecuted source/destination byte remains untouched.

Reset clears controller transient data while retaining frozen binding ownership;
the existing owner smoke repeats reset/program/request paths and confirms those
bindings remain usable. `core_machine_dma_finalize` remains ownership-neutral:
it neither frees nor mutates a binding owned by the machine lifecycle.

## Permanent Owner-Smoke Matrix

`core-machine-dma-channel-smoke` proves:

| Boundary | Proof |
| --- | --- |
| Device-to-memory failed write route | A two-megabyte RAM allocation rejects page `20h`; read provider count, terminal callback, current address/count, request, mask, and selected-service state remain unchanged or clear as applicable. |
| Memory-to-device failed read route | The same invalid source route invokes no write provider and leaves request/current state unchanged with no selected service retained. |
| M2M preflight | Independently invalid source and destination routes leave both address pairs, channel-1 count, channel-0 software request, temporary latch, and selected-service state unchanged or clear as applicable. |
| M2M terminal | Channel 1 owns terminal count; the retained M2M proof observes its TC and no stale request/service state. |
| M2M auto-init | One transfer restores both channel-0 and channel-1 current address/count pairs, retains masks clear and TC clear, and invokes the bound channel-1 terminal callback. |
| M2M external EOP | A channel-0 opaque binding terminates after exactly one committed primitive; channel 1 records TC and the second destination byte remains untouched. |
| Reset/binding lifecycle | Reset clears request/status/service/latch/EOP transient data and retains the original bound provider, which completes a later request without rebinding. |
| Consumer/timeline continuity | The same owner smoke retains all S2/S3 port/page/request proofs; `core-machine-fdc-smoke`, `core-machine-rtc-storage-s4-smoke`, and `core-machine-arbitration-s3-smoke` revalidate the FDC binding, T347 service owner, PIC visibility, and T346 one-grant timeline boundary. `vm-fdc-dma-boundary-smoke` invokes default INT 13h through a two-sector 64KiB-crossing destination; it proves both fragments while DMA channel 2 retains its page. `vm-ata-pio-dos-smoke` preserves the consumer boot path. |

The owner marker is `M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK`.

## Retained Boundary

This closes logical DMA transaction and lifecycle state, not physical signal
edges, READY/compressed transfer duration, DREQ/DACK/EOP waveform polarity, or
generic CPU/memory wait states. Those require the later L3 bus-timing admission
and remain outside T348.

## FDC 64KiB Consumer Boundary

The 8237A current address wraps within its page register; page carry is not a
DMA-controller behavior. The default FDC INT 40h service is therefore placed
in its owned `f000:0c00` through `f000:dfff` fixed ROM interval rather than the
sequential vector-code stream. The materializer validates the fixed-slot
boundary and refuses any sequential interrupt, POST, or boot stream that would
enter that interval. The service routes
crossing reads through the existing reserved `9fc0:0000` bounce page, copying
one DMA-safe sector at a time while preserving the caller DS and CHS registers
between nested reads. The crossing owner smoke verifies the BDA conventional
memory report remains `639 KiB`, excluding `9fc00h--9ffffh`, and that channel 2
finishes on the bounce page `09h`; no DMA page carry or guest-visible allocation
is introduced.

## Similar-Issue Sweep

Every production DMA physical read/write is in `Transmission` or the M2M branch
of `Execute`; both now preflight through the same checked-memory query. All
terminal writes of request/mask/TC/current state route through
`dma_complete_transfer`; reset routes through `doReset`; finalization contains
no state mutation. Machine-level request callers are the opaque binding
operations and FDC bridge in `machine.c`; FDC's only request paths are its
assert/deassert helpers. T346's sole DMA advance call remains before readiness
service, and T347's FDC/ATA service owner remains after it.

## References

- [Intel 8237A data sheet](https://www.pcjs.org/documents/datasheets/intel/INTEL_8237A_DMA.pdf): M2M channel-0 request / channel-1 count, EOP, terminal count,
  and auto-initialize semantics.
- [IBM PC/AT Technical Reference](https://minuszerodegrees.net/manuals/IBM_5170_Technical_Reference_6280070_SEP85.pdf): PC/AT dual-controller integration.
