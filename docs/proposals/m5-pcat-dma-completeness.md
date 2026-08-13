# M5 PC/AT 8237A DMA Completeness

## Purpose

Complete the PC/AT dual-8237A contract after T347 established real FDC and ATA
DRQ service states. The task owns the complete machine-level mechanism rather
than a series of isolated channel repairs: controller topology, ports and page
registers, primary/secondary cascade, byte/word addressing, request/mask/
priority arbitration, transfer/terminal-count semantics, reset, and one
deterministic bus-visibility boundary.

## Dependencies

T346 owns the reset-safe due-event timeline and equal-tick arbitration order.
T347 supplies FDC/ATA states that can legitimately assert or retire requests.
The primary hardware authority is the Intel 8237A programming contract together
with the PC/AT dual-controller port, page-register, and channel-4 cascade
wiring. Reference emulators may guide a probe but do not define the contract
or become a dependency.

## Planned Execution Shape

### S1 - Dual-Controller Contract And Gap Ledger

Create the exact source-to-port-to-channel matrix before modifying behavior.
It classifies every primary/secondary register, page port, cascade route,
transfer type/mode, request/mask/priority/reset path, memory address rule, and
timeline/arbitration publication point as implemented, defective, or explicitly
out of scope. Reproduce each claimed defect with an owner smoke or bounded
probe. The output defines the finite later S slices and gives every unselected
bit an exact Queue/TODO receiver.

### Later S Units

The coordinator admits only the mechanism slices proven necessary by S1. The
expected dependency order is: PC/AT port/page representation plus primary and
secondary byte/word transfer, terminal-count, and auto-init semantics; then
request, mask, priority, cascade, and software/hardware EOP behavior; then a
final FDC/ATA/PIC/timeline reconciliation. These headings are not allocated S
identifiers and do not authorize partial delivery.

## Completion Standard

Every adopted 8237A/PC-AT behavior has one validation-to-commit owner and one
focused proof. Primary channels 0--3 and secondary channels 5--7, with channel
4 cascade, must retain their real addressing and port distinctions. A request
cannot publish a memory/device/terminal effect outside the deterministic DMA
arbitration boundary. Reset/cancel/mask/terminal paths cannot leave stale
request, callback, or channel state. The closure ledger must distinguish
implemented hardware behavior from unsupported but named modes.

## Non-goals And Transfers

Do not add host-device shortcuts, a second scheduler, host-clock timing,
generic cycle-exact bus phases, or broad peripheral behavior. Compressed
timing, undocumented clone behavior, host DMA passthrough, cache policy,
controller-duration constants, and general CPU memory/I/O wait states require
separate evidence-led admission. T348 does not make a Windows compatibility or
whole-board L3 claim; later PIC, platform-signal, peripheral, and bus-timing
candidates consume its bounded result.
