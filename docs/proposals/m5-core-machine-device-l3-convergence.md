# M5 Core-Machine Device Parity And L3 Timing Convergence

## Purpose

Prepare the NXVM core machine for evidence-led Windows 3.x research by
converging the devices, buses, ports, interrupt paths, and deterministic timing
boundaries that compose a PC/AT-class machine.  This is a hardware/system
program, not another CPU instruction sweep and not an x87 program.

The program begins with a whole-machine audit because an apparently local
device change can otherwise create inconsistent interrupt visibility, DMA
ownership, reset behavior, or wall-clock dependence.  It must replace neither
the CPU profile closure nor the later Windows readiness map; it supplies the
machine capability ledger that those decisions consume.

## Reference Discipline

Primary hardware documentation and project-owned probes define requirements.
The local Bochs 2.6 compatibility tree and PCjs tree are read-only design and
behavior references: they may identify state, register, queue, clock-domain,
or ordering questions, but their source, build, global manager structure, host
integration, firmware, and trace outputs are not imported, transliterated, or
made dependencies.  Every later implementation records its independent
hardware/probe evidence.

## L3 Target

For this program, L3 means deterministic event-and-bus timing, not
cycle-exact hardware emulation.  A future L3-ready core must provide:

1. one deterministic monotonic machine-time basis and explicit clock-domain
   conversion;
2. scheduled device events with cancellation/reset semantics and defined
   visibility points;
3. CPU retirement and I/O/memory transactions that expose a bounded bus owner,
   duration, and ordering rule where the device contract requires one;
4. deterministic IRQ/DMA/refresh/device ordering at equal timestamps; and
5. testable traces or checkpoints that prove ordering without host-clock,
   thread, renderer, or wall-time dependence.

L3 explicitly does not claim CPU prefetch queues, analog display physics,
sub-cycle pin behavior, or a universal cycle-exact PC.  Those remain separate
profile/device decisions after an evidence need appears.

## Ordered Work

### S1 - Whole-Machine Capability, Reference, And L3 Audit

Inventory every core-machine device, bus, port range, memory mapping, IRQ/DMA
route, clock source, reset path, and current focused proof.  Compare each
family structurally with the read-only Bochs and PCjs reference trees and with
its named hardware contract where available.  Produce a single capability
ledger with implemented behavior, missing/uncertain behavior, current timing
model, Windows relevance, owner, source/test evidence, and an exact receiver:
later S, Queue candidate, or TODO.  Define the measured L3 gap and dependency
graph.  No runtime behavior changes occur in S1.

### S2 - Deterministic Time, Event, And Bus Foundation

Only after S1 proves the common mechanism, introduce or reconcile the one
machine-time/event/bus transaction owner needed for L3.  Preserve existing
device observable behavior until each device adopts the new contract through
its own bounded proof.  Do not create a parallel scheduler or host-time path.

### S3 - Interrupt, Timer, DMA, And Storage Ordering

Converge PIC/PIT/RTC/NMI/DMA/FDC/ATA event visibility, arbitration, service
time, and reset/abort semantics on the S2 foundation.  Split a device family
only where it has a true hardware contract difference; use DOS/Windows-relevant
corpora and deterministic ordering probes.

### S4 - Input, Display, And Peripheral Timing

Converge KBC/AUX, speaker if present, CGA/EGA/VADP, CMOS, and host-presentation
boundaries.  Keep guest timing/state in core and host rendering/input policy in
adapters.  Do not claim VGA/VBE, composite simulation, or host wall-clock
fidelity unless separately admitted.

### S5 - PC/AT L3 Closure And Windows Handoff

Reconcile the machine ledger, prove each admitted L3 contract, transfer every
unsupported hardware feature precisely, and update the Windows 3.x readiness
map with only evidence-backed blockers.  This is not a Windows installation
task and does not close device features outside the selected PC/AT corpus.

## Completion Standard

The task closes only after every inventoried core-machine path is either
evidence-backed under the defined L3 contract or transferred once with an
owner, risk, and admission trigger.  Passing a device smoke or copying a
reference behavior does not establish parity.  x87 execution and broad 80287/
80387 work are explicitly excluded.
