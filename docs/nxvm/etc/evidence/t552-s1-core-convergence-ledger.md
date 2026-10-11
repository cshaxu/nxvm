# T552 S1 Core Convergence Ledger

## Baseline And Method

- Frozen source baseline: NXVM `5ce73c840` before T552 admission.
- Exact file inventory authority: `src/core/MANIFEST.sha256` and
  `test/core/MANIFEST.sha256`. The manifests retain the file-level inventory;
  this ledger records the owner-level audit disposition rather than duplicating
  their hashes.
- Reviewed surfaces: every Core production owner, its CMake target/link edge,
  its matching test owner, Core manifest/corpus verifier and direct caller
  boundary. `src/core` contains 197 tracked files and `test/core` contains 466.
- Static baseline: `verify_manifest.cmake` and `verify_corpus.cmake` pass.
  The latter confirms that production Core has no App dependency and permits
  only the inward `lib`, `emulator` and portable `x86` corpora.

## Owner Inventory And Initial Disposition

| Owner | Production role / target | Test owner | S1 disposition |
| --- | --- | --- | --- |
| `chips/pit825x`, `rtc146818`, `pic8259`, `dma8237`, `fdc8272`, `hdc`, `fpu`, `video`, `ps2mouse`, `keyboard`, `ppi8255`, `xtkeyboard` | One chip per static `core-chip-*` target. | Matching `test/core/chips/*` units. | Inventory complete; no S1-confirmed defect. Failure-return candidates remain in S7 or later evidence. |
| `chips/kbc8042` | 8042 controller, keyboard serial arbitration, reply queue and IRQ origin. | `test/core/chips/kbc8042/controller_contract_smoke.c`. | Two confirmed mechanism defects. S2 owns reply delay/poll progress; S3 owns executable deadline agreement. |
| `chips/cpu` | Sole CPU decode/execution/timing owner, linked through `core-chip-cpu`. | CPU and timing-manifest test families. | Existing `-w` exception on `cpu.c` and `cpu_instructions.c` is a confirmed build-audit receiver for S5; correctness/timing recipes transfer under S9. |
| `x86` | Neutral CPU bus, memory, ports, timeline, scheduler, firmware, trace and debug integration. | `test/core/x86/*`. | No reverse App dependency found. Scheduler is a KBC deadline consumer, not the owner of either confirmed KBC defect. Cross-owner proof is reserved for S4. |
| `board-base` | Common board composition, ROM/media/display, PIT/PIC/DMA/FDC/HDC/video wiring and clock/deadline conversion. | `test/core/board-base/*`. | `board_deadline.c` faithfully turns a KBC zero deadline into immediate work; do not patch around S3's chip defect. Compatibility/source questions transfer to S8. |
| `board-at` | AT KBC/parity/wiring. | `test/core/board-at/*`. | Owns board-facing KBC wiring. Use only for S4 boundary proof; no board-specific BIOS workaround. |
| `board-xt` | XT PPI/keyboard wiring. | `test/core/board-xt/*`. | Inventory complete; no S1-confirmed finding. |
| `machine` | Emulator-machine adaptation, lifecycle, runner, waiting, media and frame/input conversion. | `test/core/machine/*`. | Lifecycle/result and media-close candidates require reproduction before S7 repairs. |
| `product` | Shared PC configuration / product-local policy below the App roots. | `test/core/product/*`. | Inventory complete; no S1-confirmed finding. |
| `test/core/setup` | Shared Core setup and qualification fixtures. | Used by Core unit/integration/diagnostic routes. | `model40_profile.h` observes a fixed DeskPro profile through App-private headers only for Model40 qualification. It is not a production dependency; S6 must decide its final test owner and predicate boundary. |

## Confirmed 8042 Mechanisms

### Delay And Status-Poll Circular Wait

`x86_kbc8042_schedule_response()` records both `response_remaining_ticks` and
`response_status_polls_remaining`. `x86_kbc8042_read_status()` decrements the
latter only after the former is zero. `x86_kbc8042_advance()` decrements the
former only after the latter is zero. With both configured nonzero, neither
state can change.

The owner-local baseline test separately proves two status polls with zero
delay and a two-tick response with zero polls; it cannot observe their joint
deadlock. The direct caller chain is:

`board-at/kbc port read -> x86_kbc8042_read_status -> board-base deadline ->
x86 scheduler / machine waiting`.

S2 design: elapsed time always reduces a pending reply delay. After that delay
reaches zero, status reads consume the profile-owned visibility polls. A reply
may publish only after both gates and existing serial/FIFO publication rules
allow it. No new state, queue, public API, profile condition or scheduler is
needed.

### False Immediate Deadline While Serial Blocks Publication

`x86_kbc8042_ticks_until_event()` currently selects a zero-tick delayed reply
after only checking empty FIFO and zero visibility polls. Publication in
`x86_kbc8042_advance()` additionally requires that keyboard serial delivery
will not take precedence. Consequently a serial-delivery delay can block the
reply while the query advertises zero, even though `advance(0)` cannot make
the reply observable.

S3 design: a zero deadline means the next `advance(0)` can make actual
observable progress. If a serial delay is the next blocker, report that serial
deadline; if output consumption is the blocker, do not fabricate time. This
keeps the existing board clock and scheduler contract intact.

## Reproduction And Baseline Evidence

The existing focused KBC controller, serial-cadence and board-controller CTest
routes pass in x64 and x86. That result is expected: they prove isolated
contracts but do not contain the two cross-condition regressions above.

S2 will add delay-plus-poll owner tests. S3 will add serial/FIFO query/advance
agreement tests. S4 will add at most one board/scheduler boundary assertion;
it will not mirror chip logic at a higher layer.

## Planned Receivers

The approved linear S1-S9 plan is recorded in the task proposal. Initial
candidate rows are deliberately not defects until their contract is reproduced:

| Receiver | Current evidence | Assigned S |
| --- | --- | --- |
| CPU `-w` and debug warning exceptions | Explicit build exception. | S5 |
| Model40 test setup/App observation edge | Test-only fixed-profile support edge. | S6 |
| Runner result state, media replacement and close/discard errors | Static leads only. | S7 |
| Compaq CECG alias and PC/AT bounded-L1 policy | Existing compatibility behavior without S1 source conclusion. | S8 |
| CPU compatibility timing recipes | CPU-owned correctness/timing scope. | S9 transfer/reconciliation |

No Shared, MyNES, App production, firmware, media asset or INI change is
admitted by S1.
