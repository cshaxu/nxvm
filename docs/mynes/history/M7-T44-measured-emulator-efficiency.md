# M7 T44 Measured Emulator Efficiency

Owner admits the queued Nesticle-inspired performance study on 2026-10-06
against fc0d31623, after closed MyNES T43. Task closes on owner acceptance after
S3 reconciliation on 2026-10-06. Historical packets are frozen in their commits; the
[proposal](M7-T44-measured-emulator-efficiency-proposal.md) preserves the
full goal. No performance gain or new product is qualified at admission.

## Frozen Coverage And Disposition

The universe is the retained NROM/MMC1/UxROM/CNROM/MMC3 support and six owner ROMs,
code-owned blank/repeated/raster-color/emphasis/dense/scrolling fixtures,
Window graphics and Console text, x64/x86, reset/pause/resume/input and
snapshot continuation. Source-native guest cycles, completed frames, PCM and
mapper/interrupt events must survive every accepted optimization. Record
source/build/input identity and observe host display separately from guest
production; frameskip/coalescing is not a computational gain.

| Batch | Required proof | Disposition |
| --- | --- | --- |
| S1 baseline and palette conversion | Repeatable bounded before/after execution and conversion measures; exact palette ordering/RGB duplicates/overflow, text/publish behavior and bounded memory. Distinguish synthetic, game and paced native routes. | Accepted and closed after immutable-diff review of ae15f9129 / b2c973f69: both suites 56/56, exact paired identities and qualified 0044 pair. [Evidence](../etc/evidence/m7-t44-s1-palette-performance.md). |
| S2 Mapper candidate | Only mapping-register-derived address computation. Preserve bus/A12/IRQ and reset/snapshot reconstruction. | Accepted and closed after actual-diff review of b22a3bb89: selected simplifications and CNROM bounds repair, both suites 57/57, paired proof and updated 0044 pair. [Evidence](../etc/evidence/m7-t44-s2-mapper-performance.md). |
| S3 integrated qualification | Full coverage reconciliation, rejected-candidate dispositions, both output modes/widths, required units/integration and paired artifacts. | Accepted after actual-diff review of f1a1ffa34; exact accepted inputs/results reused, candidate/debt/scope dispositions reconciled, proposal archived. S3 and T44 closed. [Audit](../etc/evidence/m7-t44-s3-closure.md). |

Accepted means repeatable whole-frame benefit with unchanged applicable bytes,
guest timing/state/events and no material regression. Rejected candidates keep
their evidence, not a hidden mode-specific workaround. An unmeasured stage,
unsupported reference license, or needed Shared capability is explicit; do
not invent a timing attribution or silently change the universe. Original
Task completion requires every row's direct disposition and final qualification.

## S1 Initial Source And Boundary Review

driver.c still owns pacing, audio adaptation and both output conversions.
Its immutable 512-entry RGB table already avoids repeated emphasis arithmetic;
the graphics path instead linearly searches the accumulating palette for each
of 61,440 pixels, then makes a second pass on overflow. Preserve raster-order
indexing and equal-RGB deduplication. PPU dot/fetch/mapper costs remain to
measure before prescribing tile/dirty caches. At admission no source had changed;
the completed S1 changes only frame-local graphics palette lookup.

Primary reference metadata identifies athros/NESticle as historical leaked
Bloodlust source, not an MIT grant. Remote HEAD/master is
7ede7dab54bc0cef85774b424025a7d6b8133d28. Only neutral architecture lessons are
admitted; no copying/transliteration, source/binary import or exact x.xx speed
claim. The sibling MySMB study is read-only background and not MyNES proof.

## S1 Coordinator Review

Actual review covered the pushed P1 root preset diff and every P2 source,
test, build, tool, document and artifact change, not just the timing report.
The packet is frozen in b2c973f69:docs/mynes/states/CURRENT.md.
Owner outcomes map to a pre-edit baseline, repeatable frame-conversion benefit,
exact pixels/palette plus game state/PCM proof, bounded local ownership,
full dual-width tests and latest-pair artifacts. Driver net +9 is justified by
the direct lookup, not an added layer; full counted source/test net +327 includes
the reusable measurement and independent regression. RGB aliases, index 255,
ignored sample high bits and overflow preserve original behavior. No persistent
state, extra producer, runtime API, shared production or NXVM change exists.
INI/snapshot/ROM identities are preserved; old EXEs are recoverable in Git.
The sole Shared diff is the separately approved pair of MyNES preset targets.
No universal FPS/whole-emulator gain or native timing is accepted. S1 consumes
the whole palette batch; Mapper remains a separately admitted next candidate.
T44 remains open, and no S2 implementation has begun.

## S2 Coverage Clarification

The initial shorthand omitted CNROM, although mapper 3 has an existing parser,
production path and owned test. S2 includes it in the disposition universe;
this is not new mapper support. Its non-power-of-two CHR capacity and snapshot
validation must not be overlooked when applying a common address optimization.

## S2 Coordinator Review

Reviewed every immutable b22a3bb89 source/header, test, tool, document and
artifact change against the approved packet frozen in that commit's Current.
Constructor, relevant register writes and validated staged snapshot commit
share one mapping builder; it uses actual-capacity modulo, not a guessed mask.
Only the fixed admitted MMC1/UxROM domains use masks. NROM is unchanged; CNROM
keeps its modulo semantics and gains restored-bank rejection. v3 stores no
derived offsets, and failed loads preserve live state. Bus/A12/IRQ and reset
code is unchanged. Code-owned full capacity/value/mode proof and both complete
57/57 suites agree with exact external before/after identities and both PE/hash
artifacts. Production net +3 and 48-byte offset arrays are bounded; test growth
provides the full affected mapping/snapshot batch rather than a second engine.
Whole-leaf CPU measurements and noisy wall-clock observations have separate
scopes; no universal speed/FPS claim is accepted. No Shared or adjacent App,
INI, ROM or owner snapshot change exists. P1 was pushed immediately; subsequent
pure governance P records S2 closure. T44 stays open for its S3 reconciliation,
which has not started and awaits owner direction.

## S3 Final Acceptance And T Closure

Owner accepts S2 and requests closure. S3 P1 f1a1ffa34 completes the planned
task audit; coordinator reviews its actual documentation/archive diff and the
unchanged source/test/tool/configuration/artifact surface against b22a3bb89.
Existing final logs contain 57 passes and zero failures per width; both EXE,
INI and six-ROM identities match accepted evidence. No redundant full rebuild
or test is run for this documentation-only step. Local T44 probe binaries/CSV
directories are disposed after recording summaries; warm build caches remain.
All candidate batches have direct dispositions, no new in-scope defect remains,
and five pre-existing unrelated TODO entries retain their original owners.
No Shared source import, adjacent App change, native FPS/per-device timing or
compounded speed claim is accepted. The proposal is archived beside this record.
Pure governance P records acceptance and closes S3/T44; Queue is empty and no
next T/S is admitted. Current 0044 pair remains the S2 owner-tested build.

## S5 Corrective Closure

The owner later reopened the latest closed T44 for the narrow receiving repair
only: declare IBM PC Machine's already-used `kvm-base` dependency and add a
private close-failure injection proof for FDD/HDD removal. Shared P
`56b81532c` contains that correction. The IBM PC corpus gate and normal plus
injected media-removal tests pass on x64 and x86; receiving artifacts were
rebuilt and pushed only where their hashes changed. This corrective work does
not revise the accepted MyNES performance claim or modify MyNES runtime
source, configuration, snapshot, ROM, or media behavior. Owner acceptance
closes T44 again.
