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

## Sequence Correction

T223 was a completed urgent keyboard compatibility repair. T225 is the
approved time/PIT closure task; the KBC work formerly shown at T224 is deferred
to T226. This correction does not rewrite pushed historical commits or turn
T225 into KBC work. The later queue is rebaselined by T225 S6 before any new
device implementation begins.

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
| T222 | T216, T219, T221 | Complete. PIT elapsed-tick waveform/GATE semantics are recorded under the true T222 identity; its final artifact revision is `0.5.0224`. |
| T223 | T216, T219 | Complete urgent keyboard compatibility repair only. It restored `EDIT.COM` interaction but did not claim either planned KBC protocol phase. |
| T224 | T223, T219 | Historical completed task identity. Its record remains historical; it is not a license to relabel later planned work. |
| T225 | T219, T220 | Complete. S7 locks reset-remainder, paused-session, one-step, and repeated-period evidence around the elapsed-tick/PIT/IRQ0 -> ROM -> BDA -> INT 1Ah path. |
| T226 | T216, T219, T225 | Complete: set-1 break/E0/E1, FIFO, rapid typeahead, IRQ1, and ROM `INT 09h`/`INT 16h` ordering. Host input remains profile mapping -> KBC ingress. Its historical `0.5.0226` worktree artifact is withdrawn as non-reproducible evidence. |
| T227 | T216, T226 | **Complete.** Set-1 selection/query, translation-bit observation, LED, command-state typematic, ACK/RESEND, and controller timing are complete. S4 disables uncalibrated default auto-repeat while retaining explicit nonzero profile deadlines; its next monotonic artifact revision is `0.5.0237`. |
| T228 | T219, T220 | Complete. VADP owns the bounded CGA `320x200x4` B8000h layout, `3D8h`/`3D9h` subset, palette, raster state, and copied indexed snapshot. `console`/`window`/`auto` presentation is composition policy; 62 current smoke cases and GCC gates pass. EGA/VGA remains excluded. |
| T229 | T216, T226, T227 | **Complete.** Bounded 8042 AUX packet/controller path, IRQ12, host -> profile mapper -> KBC ingress, port/IRQ12 probe, and ordinary guest-IVT fixture are verified. Host mouse never writes DOS APIs, BDA, or guest memory directly; advanced AUX and guest mouse API work remain deferred. |
| T230 | T216, T219 | **Complete.** Core-owned 8237 request/mask/mode/page/address/count lifecycle is exposed through frozen device bindings; the default FDC is bound to DMA2 and cannot directly copy guest RAM. T231 retains FDC behavior expansion. |

## Storage And RTC

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T231 | T216, T219, T230 | **Complete.** VM-owned bounded FDC/FDD command/result state machine: media change, DOR motor/select state, 500/250 kbit CCR admission, errors, non-DMA, DMA format IDs, DMA2 byte cadence, and IRQ6/SENSE-INTERRUPT completion. Image files remain validated backends; multi-drive mechanics, rotation latency, deleted/scan/read-track commands, and arbitrary FDC configuration remain deferred. |
| T232 | T216, T219 | **Complete.** VM-owned MC146818 RTC/NVRAM subset with profile-bound 70h/71h + IRQ8, deterministic elapsed-tick progression, BCD/binary and 12/24-hour conversion, SET, periodic/update/exact-alarm flags, and read-C acknowledgement. Host time cannot bypass guest register state; persistent battery files, square-wave output, don't-care alarms, and broader RTC services remain deferred. |
| T233 | T213, T216, T219 | **Complete.** Primary-master ATA PIO admits LBA28 and count-zero=256 with bounded backend access; profile-declared slave and secondary absence return controlled non-aliasing behavior. SRST and status/error/IRQ14 semantics are covered. IDE DMA, ATAPI, LBA48, cache, and host-clock timing remain deferred. |

## EGA/VGA, Real-Mode Compatibility, And Deferred CPU/FPU

| Task | Dependencies | Deliverable and exit condition |
| --- | --- | --- |
| T234 | M5 boundary audit | **Complete.** `core/utils` now owns the injected wait scope; VM peers no longer import `core/product`; composition owns BIOS text-to-bytes assembly; and the source/target boundary gate plus GCC smoke matrix pass. |
| T235 | T219, T228 | **Complete.** Core VADP owns the profile-bound A0000h aperture semantics and bounded `3C4h`/`3C5h` sequencer subset; core RAM remains the one byte backing. Port and frozen-session probes plus 69/69 current gates pass. No EGA/VGA frame or BIOS-mode claim is made. |
| T236 | T235 | **Complete.** Core VADP owns bounded graphics/attribute register families, status-reset flip-flop, and A0000h/B0000h/B8000h map-select classification with explicit untransformed planar-access fixtures. Profile reset values are normalized through the same masks as port writes. 71/71 current gates pass. Planar/latch/raster/frame behavior remains T238-only; programmable DAC remains deferred. |
| T238 | T219, T236 | **Complete.** Direct-port `EGA-320x200x16-direct` now uses VADP-owned non-mirrored planes/latches through frozen core memory routing and publishes a copied 16-entry fixed RGBI frame. Mode activation requires the bounded guest port sequence; core/system/DOS fixture evidence and the full current matrix pass. DAC, BIOS modes, and all other EGA/VGA families remain excluded. |
| T239 | T238 | **Complete.** Default-profile ROM admits only `INT 10h AH=00h, AL=0Dh` for the T238 EGA direct surface and `AL=03h` to exit to text. Firmware drives the real VADP port/memory path; BDA `40:49` is query-only state. System/DOS fixtures and 76/76 current gates pass; see [T239 record](m5-t239-rom-ega-int10.md). |
| T240 | T216--T239 | **Complete.** Established a bounded real-mode **8086** trust baseline: reset-vector entry, segment override, `REP`/direction strings, `INT`/IVT/`IRET`, immediate/DX I/O transactions, and retained `#UD` outcome. Existing 80186+ forms remain profile boundaries; 77/77 current CTest cases passed. This is not an exhaustive opcode-completeness claim. See [T240 record](m5-t240-real-mode-cpu-trust.md). |
| T241 | T229, T226, T227 | Admit the minimum guest mouse-driver corpus through the existing AUX/KBC/IRQ12 path. Advanced AUX controller commands are separately probed only when the corpus requires them; host input never writes DOS APIs or guest memory. |
| T242 | T230, T231 | Extend FDC/FDD only through failing real media or driver corpus: multi-drive mechanics, rotation/motor timing, additional commands, media formats, and error paths. Boot alone remains insufficient evidence. |
| T243 | T213, T233 | Extend ATA/IDE only through a declared feature matrix and failing corpus: selected channel/device topology, timing/error behavior, or other admitted PIO features. IDE DMA, ATAPI, LBA48, and cache remain separate admissions. |
| T244 | T219, T220, T228 | Complete digital CGA through bounded `640x200x2`, remaining text modes, and 6845-visible start/page/cursor/scanline behavior. Composite artifact color remains an optional renderer/profile capability. |
| T245 | M5 profile boundary | Define machine-profile admission: immutable topology/capability declarations, optional user-provided ROM manifests, clock/port/IRQ/DMA/media contracts, fixture corpus, and provenance policy before PC110, DeskPro, Award, or Phoenix profiles. |
| T246 | T217--T225 | Admit the next timing-fidelity level only when a real corpus requires it: profile-bound rational frequency/phase/rounding/event ordering for CPU, PIT, CGA, DMA, and RTC. Instruction, bus, and cycle timing remain separate work. |
| T247 | Adequate real-mode device baseline | Add 286 descriptors, exceptions, and protected-mode control transfer with instruction probes and bounded differential evidence. |
| T248 | T247 | Add 386 paging, CRx, CPL/IOPL, and TSS I/O map with focused probes and bounded differential evidence. |
| T249 | T248 | Add task switching and remaining admitted 286/386 instruction families. |
| T250 | T247--T249 as applicable | Add present FPU state, operations, exceptions, and `FWAIT`. Existing FPU-none ESC consumption is not present-FPU support. |

CPU/FPU work is intentionally lower ROI than the real-mode hardware, firmware,
profile, and timing route. The
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
