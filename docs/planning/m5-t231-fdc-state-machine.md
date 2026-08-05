# M5 T231: FDC/FDD Controller State Machine

## Scope

T231 replaces the legacy default-profile FDC's implicit `msr`/`rwCount`
protocol with one VM-owned, bounded 8272A-compatible controller state machine.
The FDD remains an in-memory image backend; image bytes never bypass the FDC,
DMA2, or IRQ6 route. This is an admitted 1.44 MB PC/AT slice, not a claim of a
complete 8272A clone.

## S1: Contract And Probe Design

**Status:** Complete.

### Ownership And Lifetime

```text
guest FDC ports 03F2h/03F4h/03F5h/03F7h
  -> vm_machine_fdc command/execution/result state
  -> frozen DMA2 request binding -> core DMA -> guest RAM
  -> VM FDD image backend
  -> FDC result phase -> IRQ6 source -> core PIC -> guest IVT
```

- `vm/machine/fdc` owns DOR/CCR/MSR, command/result FIFOs, selected drive,
  command phase, transfer direction/count, pending interrupt, and the only
  DMA2/IRQ6 request lifecycle.
- `vm/machine/fdd` owns fixed 1.44 MB geometry, inserted/read-only media,
  media generation, validated CHS byte access, and formatted-sector contents.
  It does not own a controller phase, IRQ, DMA request, or guest-RAM access.
- Core owns DMA2 controller registers, byte cadence, guest-memory access, PIC
  delivery, and elapsed ticks. Composition binds the already profile-declared
  IRQ6/DMA2 route once and never drives a transfer.
- The legacy ROM remains an ordinary port client. No BIOS, BDA, portal, or host
  shortcut is admitted.

### Admitted Guest Contract

| Area | T231 behavior |
| --- | --- |
| Phases / MSR | Explicit idle-command, DMA/non-DMA execution, and result phases. `RQM`, `DIO`, `NDM`, and `CB` describe that phase; result reads consume exactly the declared result length. |
| Commands | `SPECIFY`, `SENSE DRIVE STATUS`, `RECALIBRATE`, `SENSE INTERRUPT`, `SEEK`, `VERSION`, `READ ID`, `READ DATA`, `WRITE DATA`, and `FORMAT TRACK`. Flagged read/write opcodes are decoded by their base opcode; unsupported read-track/deleted/scan/configure commands return one invalid-command result and do not start DMA. |
| Media / DOR | Insert/remove advances FDD media generation. DIR bit 7 reports unacknowledged change. DOR reset cancels the transfer and deasserts DMA/IRQ; drive select and motor bits are retained as guest-visible controller state. The existing ROM-compatible no-spin-up-delay behavior remains explicit. |
| Geometry / errors | 80 cylinders, 2 heads, 18 512-byte sectors. No media, write-protect, invalid CHS/size/EOT, wrong selected drive, or unsupported rate produces a bounded result and completion IRQ where the command class requires one. |
| Rate / timing | CCR accepts the default 500/250 kbit profile values but does not invent host-time rotation. Core DMA2 is the sole byte clock: one scheduled DMA service per core elapsed tick. FDC deasserts DRQ when its explicit transfer byte budget completes. |
| Non-DMA | `SPECIFY.ND=1` transfers the same bounded bytes through 03F5h data handshakes. It never accesses RAM directly. |
| Format | In DMA mode, FDC consumes four-byte C/H/R/N ID tuples through DMA2, validates the selected track, then fills each declared sector. In non-DMA mode the same tuples enter through 03F5h. Unsupported ID/geometry ends with an error result. |
| Completion | Transfer/seek/read-ID/format completion is FDC-owned: deassert DRQ, prepare a fixed result FIFO, assert IRQ6 if DOR enables it. `SENSE INTERRUPT` consumes the pending completion and deasserts its IRQ6 source. |

### Required Evidence

1. The port probe programs DOR and FDC commands and validates phase/MSR,
   exact result lengths, no-media result, media-change acknowledgement,
   non-DMA handshake, and format ID transfer. The retained core DMA channel
   probe owns DMA register assertions.
2. The existing owner-supplied FDD system-image regressions exercise the same
   03F5h/DMA2/IRQ6 route. DOS boot, prompt, `EDIT.COM`, Console and debugger
   tests protect the image workflow.
3. The T230 boundary gate remains: FDC has no raw DMA/latch/RAM access. A new
   gate rejects direct FDD pointer arithmetic from FDC and a second FDC
   transfer loop.

### Stop Conditions

Stop and revise the contract before S2 if completion needs a VM-side
per-instruction loop, host sleep/wall clock, direct PIC/CPU/BDA mutation, a
second media or DMA state copy, a raw core-memory borrow, or a change to the
NXVM Console/debugger/startup experience. Multi-drive mechanics, rotational
latency, deleted-data/scan/read-track, and arbitrary FDC configuration are
explicitly deferred.

## S2: Owned Implementation

**Status:** Complete.

- Replaced implicit `rwCount` command/result handling with phase, declared
  command/result lengths, bounded transfer counters, and a fixed result FIFO.
- Added admitted command handling for `SPECIFY`, drive/interrupt sensing,
  seek/recalibrate, version, read-ID, read/write data, and format. Unsupported
  commands return a one-byte invalid-command result without starting DMA.
- FDC owns DRQ/IRQ6 assertion and teardown; its frozen DMA2 binding remains
  the only FDD-to-guest-memory route. Non-DMA uses only 03F5h handshakes.
- FDD is now a validated CHS media backend. The retired FDD cursor/transfer
  helpers could have formed a second controller path and were removed.

## S3: Verification And Closure

**Status:** Complete.

| Evidence | Result |
| --- | --- |
| `vm-fdc-port-smoke` | Emits `M5:T231:S1:FDC-PORT:OK`; validates command/result phase, no-media error, media-change acknowledgement, non-DMA format, and non-DMA sector read. |
| `core-machine-dma-channel-smoke` | Retained core-owned DMA2 register/cascade/terminal-count evidence. |
| `verify-fdc-state-machine-boundary` | Emits `M5:T231:S3:FDC-STATE-MACHINE-BOUNDARY:OK`; rejects FDC image-pointer/RAM/PIC shortcuts and the retired FDD cursor transfer API. |
| Current matrix | 66/66 CTest cases pass, including FDD DOS boot, prompt, `EDIT.COM`, Console/debugger, and the new FDC port smoke. Linux remains static-contract-only; no WSL is used. |

The completed artifact is `build/output/nxvm_0_5_0231.exe`, SHA-256
`42BDBE713ABE5B075C957B652DB27FF881EA3712052E4AAA88B2B32353AC3845`, from
implementation commit `1ac0e0a`. Its runtime identity is `Neko's x86 Virtual
Machine [0.5.0231] Copyright (c) 2012-2026 Neko.`
