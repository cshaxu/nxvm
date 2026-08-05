# M5 NXVM PC/AT Hardware Convergence

## Authority And Scope

This is the authoritative, ROI-ordered implementation queue for the remaining
M5 NXVM/shared-core PC/AT work. It supersedes the broad hardware priority list
in `TODO.md`; `TODO.md` remains the long-term risk ledger. It does not start a
later milestone or define VDM completion.

The target is a trustworthy bootable NXVM PC/AT subset that boots real DOS
images and runs text-mode DOS software through owned CPU, bus, device,
firmware, and profile behavior. It must not use BIOS hacks, host shortcuts, or
application-specific paths to manufacture guest-observable behavior.

The completed baseline is M5 T216 S5: the sole ordinary software-interrupt path
is CPU `INT` -> IVT -> ROM/guest handler; the core PIC owns live IRQ delivery;
and the retained FDD `EDIT.COM` launch regression passes. Active sources are
PIT IRQ0, KBC IRQ1, FDC IRQ6, and HDC IRQ14. RTC IRQ8 and AUX IRQ12 are future
routes, not T216 claims.

Each admitted task uses the hardware-device verification template: S1 contract
and port/instruction probe, S2 implementation at the real owner, and S3 focused
plus DOS/system-image regression. Every behavior-changing task records one
verified `build/output/nxvm_0_5_NNNN.exe` artifact, hash, source commit, and
retained Console/debugger/FDD/HDD evidence. Optional Bochs/PCjs comparison is
bounded differential evidence only, never copied code, a runtime dependency,
or an acceptance substitute.

## Time And Real-Mode Foundation

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T217 | T216 | Define one core-owned monotonic `elapsed_ticks` contract: run budget plus reset, stop, pause, and step semantics; frozen profile clock parameters; host pacing cannot advance guest time. Cold reset returns to one deterministic time origin. |
| T218 | T217 | Establish the ROM/DOS/device real-mode 8086/80186 instruction and tick-attribution corpus. Fix only reproducible real-mode defects. Every completed architectural instruction receives a deterministic non-zero coarse tick; prefixes do not independently advance time. This explicitly does not claim protected mode, paging, or cycle accuracy. |
| T219 | T217, T218 | Establish one core elapsed-tick scheduler with fixed post-instruction device order, delta-tick delivery, IRQ visibility, and reset/stop behavior. PIT, VADP, and DMA advance there; the VM outer loop only calls `core_machine_run()`. |
| T220 | T219 | Replace the T216 minimal deterministic `3DAh` phase with a core VADP text-raster contract: display-enable, vertical retrace, and read-only bit 0/bit 3 observations derived from elapsed ticks. Platform consumes copied snapshots only. |
| T221 | T217--T220 | Record deterministic clock/CGA checkpoints and fixed profile ratios. Optional bounded PCjs/Bochs status-sequence comparison may calibrate evidence. It does not claim final PIT waveform timing. |

## High-ROI Input, Timer, Display, And DMA

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T222 | T216, T219, T221 | Implement PIT elapsed-tick waveform and GATE semantics: modes 0--5, count zero, BCD, read-back boundaries, and IRQ0 lifecycle. No host timer or one-instruction fake timer behavior remains. |
| T223 | T216, T219 | Implement KBC phase 1: set-1 break/E0/E1, FIFO, rapid typeahead, IRQ1, and ROM `INT 09h`/`INT 16h` ordering. Host input remains profile mapping -> KBC ingress. |
| T224 | T223, T219 | Admit and verify KBC phase 2 scan-set selection, translation, LED, typematic, resend/error, and controller timing. Delays consume core elapsed ticks, never host sleep. |
| T225 | T219, T220 | Implement the bounded CGA `320x200x4` VADP subset: VRAM layout, 3D8h/3D9h mode/color/palette, raster state, and copied pixel snapshot. EGA/VGA remains excluded. |
| T226 | T216, T223, T224 | Add 8042 AUX mouse packet/controller path and IRQ12 through PIC. Host mouse enters only through profile mapping; it never writes DOS APIs or guest memory directly. |
| T227 | T216, T219 | Implement 8237 DMA request/mask/mode/page/address/count lifecycle and the channel-2 FDC contract. DMA memory access uses the core memory contract; devices cannot directly copy guest RAM. |

## Storage And RTC

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T228 | T216, T219, T227 | Implement the VM-owned FDC/FDD command/result state machine: media change, motor, rate, errors, non-DMA, format, transfer timing, DMA2, and IRQ6. Image files are backends only; boot alone is insufficient evidence. |
| T229 | T216, T219 | Define and implement CMOS/RTC register, periodic/update/alarm IRQ8, NVRAM, and deterministic-time semantics. Host time is only a provider and cannot bypass guest register state. |
| T230 | T213, T216, T219 | Extend the existing VM-owned ATA PIO controller through an explicit feature matrix: LBA, slave/secondary channel, reset and status/error timing. DMA is not admitted; file images never substitute for controller behavior. |

## EGA/VGA And Deferred CPU/FPU

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T231 | T219, T225 | Add only EGA/VGA memory windows, mapping, and sequencer families with per-family port and memory-map probes. |
| T232 | T231 | Add admitted graphics- and attribute-controller families with planar-access fixtures; deferred families remain explicitly unsupported. |
| T233 | T219, T232 | Add bounded DAC, planar VRAM, latch, and raster subfamilies with copied frame snapshots. This is not a single unbounded VGA task. |
| T234 | Adequate real-mode device baseline | 286 descriptors, exceptions, and protected-mode control transfer, each with instruction probes and bounded differential evidence. |
| T235 | T234 | 386 paging, CRx, CPL/IOPL, and TSS I/O map with focused probes and bounded differential evidence. |
| T236 | T235 | Task switching and remaining admitted 286/386 instruction families. |
| T237 | T234--T236 as applicable | Present FPU state, operations, exceptions, and `FWAIT`. Existing FPU-none ESC consumption is not present-FPU support. |

CPU/FPU work is intentionally lower ROI than the real-mode hardware route. The
DOS `MEM`/`FNINIT` history remains a regression sample, not evidence that 80386
or FPU compatibility is complete.

## Execution Constraints

- Keep one active subtask. Start each task with its S1 record, contract, probe,
  explicit owner, deferred behavior, and stop condition.
- `core/machine` owns generic guest state and elapsed time; `vm/machine` owns
  VM-only devices; profiles provide frozen topology and ROM/firmware contents;
  platform never mutates guest state.
- Do not add a second executor, machine/session, device-state mirror,
  VM-side instruction loop, host-clock guest shortcut, global/TLS current-object
  facade, or an unapproved NXVM Console/debugger/startup/boot behavior change.
- A code task commits only after its stated evidence is recorded. Push the
  verified commit when the configured remote is authorized; external transport
  is never evidence in place of local verification.
