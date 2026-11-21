# T593 S3 shared execution and working-set audit

Baseline [168f253bc](https://github.com/cshaxu/nxvm/commit/168f253bc).
S3 admission is [fc958526d](https://github.com/cshaxu/nxvm/commit/fc958526d).
The source inventory contains 55 X86 Core files and 163 X86 NXVM files.
It includes 84 C and 119 header files. The audit scanned directory
responsibilities, loops, allocations and copies, then inspected CPU timing,
memory and port routing, guest-time publication, chip advancement, graphics
capture and storage. This inventory is not a dynamic profile of every App.

The [controlled baseline](t593-s1-ob300-execution-baseline.md) puts about
4.32 of 5.68 sampled x64 DIR Core seconds in CPU refresh and 1.48 in elapsed
time publication. Corresponding x86 estimates are 6.86 and 2.52 of 8.79 s.
Nested samples overlap. The source review prioritizes shared work that
scales with instruction, tick, pixel, or sector counts.

## X86 Core

| Order | Candidate | Complexity or cost | Required proof |
| --- | --- | --- | --- |
| 1 | Consolidate [timing evaluator selection](../../../../src/x86-core/cpu/cpu_timing.c#L647) by profile and decoded form, reusing the existing result and formula owners. | Profile sampling measured 5.65 evaluator attempts per cold retirement, with about 82 percent rejected. Grouping candidates can eliminate repeated selection work. The earlier guard-only prototype missed the end-to-end requirement. | Use existing decoded facts. Preserve evaluator precedence, repeat-state clears, attribution, exception and prefix outcomes. Compare sampled form mixes on both widths before retaining a dispatch table. |
| 2 | Reduce [physical routing](../../../../src/x86-core/machine/memory.c#L82) work using registration-time geometry and original provider order. | A B-byte access may perform approximately B additional full scans over P providers. An immutable geometry filter can reduce candidates before query callbacks. | Raw-before-decoded priority, A20 aliases, selected RAM, overlapping partial claims, terminal errors and unsupported fallback. Index only frozen geometry, with ordinal-preserving order. Keep byte routing where coverage is uncertain. Two narrowed scan prototypes remain unselected. |
| 3 | Remove unused [trace PC preparation](../../../../src/x86-core/machine/machine_scheduler.c#L345) and use existing [timeline count](../../../../src/x86-core/machine/timeline.c#L177). | The CPU retirement trace prepares a PC even when no provider consumes it. Timeline pending count scans 32 entries even though heap size is maintained. The first saves per-retirement work and the second replaces O(capacity) with O(1). | A trace-enabled run must produce the identical fields and order. Compare timeline count after cancellation, callback reentry, and reset. Keep existing registration and trace delivery. |
| 4 | Fold [parity](../../../../src/x86-core/machine/memory.c#L301) in fixed work and pack stored bits. | Replace up to eight shifts per byte with a fixed fold. A packed shadow uses ceil(N/8) bytes in place of N. Relevant to parity-backed profiles such as My5170. MyOB300 configures port 61h without a parity shadow. | All 256 values, odd offsets, partial final byte, aliases, read fault address and observation-only reads. Verify no write changes adjacent parity bits; measure time and memory separately. |
| 5 | Investigate the full CPU restart-state copy at [ExecBoundary](../../../../src/x86-core/cpu/cpu_instructions.c#L18953). | Previous instrumentation saw about 380 bytes copied at many instruction boundaries. Cost of that copy alone is unknown. | Identify actual readers and distinguish pre-instruction observations from rollback. Prove protected exceptions, faults with write prefixes, task switches, SMM and Debug before making the snapshot smaller. Avoid a second rollback mechanism. |
| Later | [Timeline slot allocation](../../../../src/x86-core/machine/timeline.c#L97), [port lookup](../../../../src/x86-core/machine/port.c#L21), [physical retirement qualification](../../../../src/x86-core/machine/machine.c#L85), and [486 line-fill routing](../../../../src/x86-core/machine/memory.c#L896). | The 32-slot calendar scans for a free slot. Port buckets use linked entries. The physical qualification table scans up to 128 exact keys. Cache-fill proof can resolve 16 byte routes. | Favor a slot bitmap, compact frozen buckets, sorted keys or a range proof only after a workload shows the corresponding frequency. Preserve stale-token rejection, wired-OR and port widths, exact eligibility, and MMIO exclusions. A dense 64 Ki port map or general replacement scheduler needs stronger evidence. |

The actual x64 DIR trace counted 32.88 million memory route resolutions,
including 18.91 million tagged normal instruction fetch and 11.21 million
preview resolutions. The earlier separate geometric sweep reduced the
number of calls but did not consistently improve elapsed time. A new
prototype integrated geometry into existing selection and passed seven
existing route contracts per width. One x64 replay reached DOS in 75.383 s
versus 75.005 s stock and completed DIR in 7.555 s versus 6.616 s stock.
It is not selected. Byte-key locality also does not justify a second decoder
or bypassing real fetch. The earlier bytes-only diagnostic measured about
0.11 s of direct scan work within about 1 s of inclusive preview work.

## X86 NXVM

| Order | Candidate | Complexity or cost | Required proof |
| --- | --- | --- | --- |
| 1 | Simplify exact [RTC advancement](../../../../src/x86-nxvm/chips/rtc146818/rtc.c#L333). | Reuse elapsed quotient/remainder for both phases. Each phase sum is below twice the denominator, so remainder normalization is a conditional subtraction. While PF is already latched, periodic edge detection need not recompute divisions, but the phase still advances. | Register C clear-on-read, periodic/update/alarm flags, IRQ transitions, SET, mode/divider reprogramming, and large intervals. Compare stock and candidate chip states and ordered output. |
| 1 | Eliminate idle work in [board advance](../../../../src/x86-nxvm/board-base/board_advance.c), [deadline observation](../../../../src/x86-nxvm/board-base/board_deadline.c#L50) and [Turbo pacing](../../../../src/x86-nxvm/machine/waiting.c#L119). | Do not advance an uninstalled auxiliary PIT clock or ask unconfigured disk controllers for a deadline. One PIT tick is the lowest successful positive output deadline, so further PIT counters cannot lower it. Turbo does not use the host pacing observation. | Verify installed and uninstalled states, disk readiness flags, observable clocks and callback ordering. Early exit is local to the PIT minimum. Other devices can still contribute immediate/blocked information. |
| 2 | Advance the [PIT](../../../../src/x86-nxvm/chips/pit825x/pit.c#L587) to its next state-changing boundary. | Current advance scans all three counters for every input tick. For long silent intervals this can become proportional to boundaries and emitted edges. Small advances already constrained by the scheduler may benefit little. | Start with proved silent spans and retain the existing edge path. Match modes 0–5, binary/BCD counts, latches, Gate/OUT, reload/restart and counter ordering when a callback mutates another counter. |
| 2 | Remove unnecessary whole-frame memory work in [Machine display](../../../../src/x86-nxvm/machine/display.c#L40) and [C&T VGA capture](../../../../src/x86-nxvm/chips/ct-vga/ct_vga.c#L183). | A video snapshot contains about 385 KiB of array payload, including 360 KiB of pixels. Capture can clear full capacity and then copy full capacity into the retained snapshot even for text. Aim for active-payload work or direct capture only under a transactional capture contract. | Preserve last good frame on failure, force/first publication, cursor-only updates, mode switches and exact valid extents. Keep any copy required across thread/ownership boundaries. Measure text and planar Win3 graphics separately. |
| 3 | Share stable [EGA raster geometry](../../../../src/x86-nxvm/chips/video/video.c#L916) between advance, deadline and register observations, and write planar scanout in natural row order. | Raster geometry is decoded repeatedly from rarely changed CRTC state. C&T planar output emits eight per-pixel combinations per plane-byte group. Local geometry reuse or a compact expansion table may help graphics. | Identify every geometry update, including reset and mode transitions, before materializing anything. Keep per-pixel work asymptotically linear. Test exact pixels, offsets, split raster, IRQ and blink. C&T and EGA have different controller semantics. |
| 3 | Byte-wise [HDC ECC](../../../../src/x86-nxvm/chips/hdc/hdc.c#L29). | Replace eight bit recurrence steps with one table step. Two 256-entry 32-bit tables cost 2 KiB total. Ordinary non-ATA reads/writes can use it. The ATA path and existing correction search retain their semantics. | Keep both polynomials, MSB-first input, appended checksum/zero bytes, corrupt-record detection and correction boundaries. Retain only if a sector-stream benchmark justifies the table or a smaller nibble table. |
| Later | Xebec track interleave free-slot search, HDD explicit ECC record list, RTC long-span alarm/calendar arithmetic. | Format-track can scan already used positions, record-list invalidation scans sparse metadata, and alarm search already skips nonmatching hours and minutes. | Benchmark format, raw-error media or long simulated time first. Keep sparse records sparse, preserve controller ordering and prove DST/wildcard boundaries before a calendar shortcut. |

The board profile observed 3.42 million deadline queries during x64 DIR,
with 94.53 percent repeating the same absolute minimum. This does not
establish a valid deadline cache. Guest I/O, DMA and IRQ state, profile
owners and clock phase can change which work is due. Do not add another
scheduler. The small optional-device prototype had one x64 sample
(80.825 s to DOS, 5.496 s DIR), compared with 75.005 and 6.616 s stock.
The opposite movements leave wall-time benefit unresolved. The subsequent
x86 replay was intentionally stopped after the owner requested faster local
benchmarking. Task-owned guest media copies were removed by the replay.

## Focused benchmark and acceptance plan

Verify identical endpoint outputs before collecting timing. Run the same
Release benchmark against baseline and candidate objects in alternating
order, with fixed work and medians per width. Use existing functional tests
for coverage and add no wall-time assertion to CTest. The relevant benchmark
groups are CPU timing/forms and fixed instruction streams, memory spans and
aliases, board timer/IRQ publication, frame conversion, and sector-stream ECC.
Exercise at least two existing receiver configurations where an optimization
applies. Use actual DOS once after collecting accepted changes for
end-to-end regression, along with repository qualification when production
code changes.

The preliminary independent math probe passed all 256 parity values,
131,072 ECC byte-recurrence vectors spanning both polynomials, and 100,000
RTC phase/edge vectors. These are mathematical equivalence checks. They
do not execute changed chip code or measure performance. No production
source, runtime contract, manifest or executable changed in this audit.
Window/RDP and original Windows performance were not measured here.
