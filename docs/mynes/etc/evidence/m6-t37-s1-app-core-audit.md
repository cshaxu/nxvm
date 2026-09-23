# M6 T37 S1 App/Core Quality Audit

## Scope And Method

Reviewed only `src/app`, `src/core` and direct App/Core/integration tests. The
review traced command-to-completion control flow, Core state ownership and every
serialization, allocation and forwarding boundary. Lib/Common source was used
only as the published contract required to interpret a caller; it was neither
audited nor changed.

## Findings

| ID | Severity | Evidence | Disposition |
| --- | --- | --- | --- |
| AQ-1 | P1 | `core_snapshot_write/read` serialize byte ranges beginning at `offsetof(core_machine, apu)` and `offsetof(core_cartridge, mapper)`. This writes compiler layout/padding and the APU sample FIFO, rather than the declared typed guest image. | Repair in S2 with explicit little-endian sections, bounded field codecs, boolean/enum/counter validation, and staged commit. |
| AQ-2 | P2 | `snapshot.c` and `command.c` bypass the project allocation/memory vocabulary with C runtime allocation and byte routines. The snapshot instance is the relevant production occurrence. | Repair in S2/S3 by using `lib_allocate`/`lib_reallocate`/`lib_release` and `lib_memory_*`; no shared-code change is needed. |
| AQ-3 | P2 | The App snapshot accumulator grows until native allocation fails. Its only writer is trusted Core, but the product command boundary still has no stated image bound. | Repair in S3 with one owner-local maximum accepted by App before allocation and a rejection test. |
| AQ-4 | P2 | `app/config.c` includes `windows.h` and calls `GetModuleFileNameA`; the source layout reserves SDK declarations for Lib platform implementations. No existing neutral Lib process-image-path capability is available. | Record as a Lib transfer; do not create a local App platform facade or weaken executable-adjacent INI behavior. |
| AQ-5 | P2 | The byte-reader callback has no length/EOF observation. Core can reject a malformed declared section but cannot prove there are no physical trailing bytes before committing a candidate. | Record as a Common/Lib stream-contract transfer; do not add an App preflight that creates a second decoder or a post-commit rejection path. |

## Simplification Decisions

The initial review treated MyNes's synchronous Common wait as sufficient for
the direct `save`/`load` path. The owner rejected that divergence: MyNes must
use SoftPC's command-effect and runtime-completion shape as well as its UX.
S3 therefore replaces the direct path with a pending snapshot result published
only from the paused completion fact.

The two Driver snapshot callback adapters remain. They translate between the
neutral Common stream types and Core-owned callbacks at the real machine
adapter boundary; making Core consume Common types would reverse the allowed
dependency direction.

`command.c` remains one product-command owner. Its debug grammar shares the
same command context, Common lease and output result. Splitting it now would
add an internal interface without removing state or behavior. Likewise,
`driver.c` is the concrete App/Common-to-NES adapter; the input, frame, audio
and pacing helpers directly protect one private driver state rather than form
independent components. No forwarding-only production layer was found there.

## Repair Order

S2 first audits the four imported shared roots against SoftPC. S3 replaces raw
snapshot layout persistence and validates the direct Core failure/continuation
paths. S4 adopts SoftPC's completion model and bounds App snapshot buffering.
S5 performs the full App/Core sweep, removes obsolete code and records the two
shared-boundary transfers without modifying their sources.

## Test Coverage Gap

The existing `snapshot_smoke` proves round-trip, magic/truncation/cartridge
rejection and rollback. It cannot prove portable bytes, scalar rejection,
absent host-audio FIFO or physical trailing-byte rejection. S2 adds the
first four categories; the final category remains the transferred stream
contract gap.
