# T386 S22: D4 Memory Parity Diagnostic And IOCHK

`M5:T386:S22:D4-PARITY-DIAGNOSTIC:OK`

`M5:T386:S22:D4-IOCHK-CLEAR:OK`

`M5:T386:S22:MEMORY-PARITY-OWNER:OK`

## Scope And Source Boundary

S22 implements only the selected DeskPro Model 40 D4 memory-parity diagnostic
and logical IOCHK path. The primary Compaq D3PE Processor Descriptions
(1987-01-05), reviewed transiently under the source policy, defines the
D4 diagnostic/control byte at `80C00000h`: bits 0--3 expose individual parity
byte status, bits 4--7 expose the selected memory/jumper topology, a control
write clears diagnostic parity status, and the first covered ordinary memory
write clears the IOCHK condition. This record does not import source, firmware,
media, paths, hashes, extracted text, physical timing or a new product
asset/dependency.

## Owner Decision

Core already owns ordinary memory storage, parity backing, route resolution and
configuration lifetime. It now publishes a location-bearing parity-fault
callback at that existing owner and retains write-observer publication only
when a write finally reaches ordinary backing RAM. The Model 40 D4 carrier owns
its board-local four-lane diagnostic latch and consumes Core's generic event;
the existing D4 platform owner publishes IOCHK/NMI. This preserves the IBM
planar parity route as a distinct consumer of the same Core event mechanism.

No test-only production API was added. The focused test corrupts the retained
same-module parity backing only to establish a fault, then uses the normal
memory-read, port, diagnostic-register and ordinary-memory-write paths.

## Regression And Sweep

- `vm-model40-d4-parity-s22-smoke` proves normal 1 MiB RAM parity faulting,
  lane-1 diagnostic status (`BFh` to `BDh`), masked IOCHK, NMI release,
  port-`61h` publication, first ordinary covered-memory-write IOCHK clear,
  control-write diagnostic clear and reset restoration.
- `vm-model40-d4-map-s16-smoke` preserves D4 replacement/control mapping.
- `core-machine-planar-parity-nmi-s3-smoke` preserves IBM planar parity
  behavior and proves that its existing callback consumer remains isolated.
- The callback/configuration sweep covers Core parity enablement, its planar
  callback, Model 40 composition, D4 control/reset and the ordinary-memory
  write publication boundary. There is one Core parity mechanism and no D4
  memory/port ownership fork.

## Retained Transfer

D4-SKEY A20 interception, shutdown/program/power reset arbitration, physical
reset pulses, memory-cycle waits and board timing remain outside S22. T386
still retains the selected 1.2 MB FDC/media, fitted 40 MB controller/startup
media and CECG functional receivers before its board/device timing work. This
S makes no firmware-execution, physical-media, generic-variant or L3 claim.

## Verification And Acceptance

The three focused regressions pass. `verify-current-specialized-gates` and the serial `run-current-smokes` gate pass with 276 registered current tests. `Verify-DocumentationGovernance.ps1` passes. The rebuilt current developer artifact is `vm-0-5-0389`, `build/output/nxvm_0_5_0389.exe`, SHA-256 `B7DC400F3E3A8857C4699EEB6349893F2B72C70607FC29BF2C02F6E1952B5D70`. Coordinator actual-change review and the compact CURRENT/history acceptance follow P1.