# M1 T3: Board Bus, Interrupts And Seibu COP

Admitted on 2026-10-19 after accepted M1 T2 at `18eb3fd6d`, under the owner's
standing MyRaidenII implementation objective. This is MyRaidenII's third
numeric task, not an NXVM task. The [proposal](../proposals/m1-board-cop.md)
defines its bounded board/COP outcome; Current owns the active S packet.

## Scope And Exit

T3 owns the selected RAIDEN II board's deterministic Core bus assembly: V30
ROM/RAM/bank mapping, board interrupt/input/sound-latch wiring, and every COP
operation actually required by the frozen selected set. It uses the single
qualified X86 Core Machine timeline and real bus callbacks. It does not add a
second CPU runner, generic arcade-device framework, game bypass, ROM patch,
video renderer, audio synthesizer, Product host integration or gameplay claim.

T3 closes only when its finite board/COP ledger has a disposition for every
mapped range, lane/bank edge, interrupt/acknowledgement route, input edge,
sound latch operation and required COP/DMA operation; ROM-free vectors prove
their real bus routes; and owner-ROM headless startup checkpoints show the
board prerequisites are genuinely sufficient. Video, sound and Product/game
receivers remain explicitly separate tasks.

## Planned Batches

S1 freezes the implementation ledger: actual selected-set map, byte-lane/bank,
interrupt/vector, input/latch and COP/DMA operation universe; it maps each to
the qualified public X86 Core Machine seam and names source grade/proof owner.
It introduces no production source, ROM data, Shared API or executable.

Later batches are evidence-led, but start with: Core construction plus ROM/RAM
bus lanes; board interrupt/input/sound-latch/device deadlines; COP/DMA models;
then real-set headless startup receiving proof. A later S cannot turn an
unresolved COP operation into a scripted result or transfer it to video/sound
to make this task appear complete.

## Boundary

MAME is L2 research only. No MAME source, decryption/protection tables or ROM
bytes are copied, linked or committed. Existing external ROMs remain read-only
and their owner-provided identities are verified only in integration tests.
All Core behavior is App-owned under `src/myraidenii/core`; Shared X86 Core is
changed only after a real public-contract gap is discovered, separately audited
and approved.

## S1 Contract-Freeze Delivery

S1 records the finite B01--B09 board/COP ledger in
[its evidence](../etc/evidence/t3-s1-board-contract.md). It independently
rechecks the public X86 Core memory, immutable-ROM, attachment and DMA
interfaces rather than assuming the earlier V30 task made them usable.
The decisive composition result is that static program ROM can use the existing
immutable mapper, while the runtime-selectable bank is one App-owned memory
provider over immutable App-owned backing. Thus no Shared API, parallel bus,
second timeline or generic arcade layer is required for the first board batch.

The pinned MAME reference is fetched into one owned temporary directory, its
existing recorded SHA-256 matches, its L2 map/behavior observations are
classified without source/table import, and the 184646-byte temporary payload
is deleted using the governed cleanup tool. Actual tracked source/test/CMake/
manifest/artifact delta is zero; task records/evidence establish the follow-on
S2--S5 boundaries. Documentation governance and whitespace checks pass.

## S1 Coordinator Acceptance

The sequential coordinator inspected actual committed P1 `e83f186a8`, the
complete B01--B09 ledger, the public X86 Core seams and the cleanup record; it
did not rely only on the executor summary. The diff is limited to MyRaidenII
roadmap/status/history/evidence records: it contains no production source,
build registration, test target, asset bytes or external source/table import.
The static upper-ROM versus dynamic-bank distinction is correctly assigned to
the existing immutable mapper and one App-owned memory route respectively.
Every later board category has one named owner and a receiving proof, while
video, sound, Product and gameplay remain explicit non-goals.

Before S2, the coordinator also audits the current `x86-core` test corpus for
test bloat. It finds no exact duplicate C tests; the earlier fixture
consolidation removed 213 net lines. The corpus is large because it owns CPU
form/timing matrices, not because B01--B03 are already covered. S2 may add
only one focused ROM-free board test family for new board routing. It must not
retest CPU execution, generic immutable-ROM mapping, generic RAM access or
generic Machine lifecycle; those retain their existing test owners.

## S2 Coordinator Acceptance

The owner accepted S2 on 2026-10-19. The coordinator reviewed P1--P3
(`dd2611e1b`, `9f941ae69`, `f2c30a1e6`) against the B01--B03/B09 packet and
its evidence. One MyRaidenII board owns the V30 construction, copied program
lanes, dynamic bank and control write, while the existing immutable mapper owns
the static upper image. The focused board route test covers only new board
address/state behavior; it does not duplicate CPU, generic RAM, immutable-ROM
or lifecycle tests. Required x64/x86 focused checks and the repository unit
suite passed before delivery. B04--B06 remain explicitly unimplemented and are
now the admitted S3 receiver.

## S3 Coordinator Acceptance

The coordinator independently reviewed delivered P1 `4d340ce41`, rather than
relying on its implementation summary. The only production change is the
MyRaidenII-local Board's one pending/acknowledged main interrupt, active-low
input words and low-byte main/sound latches. It uses the existing X86 Core
Machine attachment and replacement memory routes; no Shared interface, second
clock, sound worker, external-ROM byte or imported MAME code/table was added.
The extended Board smoke owns only the new Board-facing bus behavior and a
real V30 interrupt route, leaving generic CPU, attachment, RAM and immutable
ROM coverage with their existing owners. Focused Board tests and the complete
repository unit suite pass on both x64 and x86 (`687/687` each); documentation
governance and whitespace checks pass. No runnable MyRaidenII executable is
claimed because B04--B06 are a ROM-free Core delivery. S3 is accepted and
closed; B07--B08 remain the next unadmitted T3 receivers.

## S4 Coordinator Acceptance

The coordinator independently reviewed P1 `da6d9d3a5`, its source-policy
limits, the actual documentation diff and its focused dual-width Board proof.
S4 changes no production source, shared contract, test, asset or third-party
payload.  The owner-ROM probe used the existing observable transaction surface,
observed only neutral address/category facts, and removed its two owned
temporary roots after recording their hashes and sizes.  It reached multiple
COP configuration, register, command and DMA-configuration families through
the real V30 bus, proving that their present ordinary-RAM fallback is a missing
device boundary.  It did not falsely claim a boot, a COP result read, a DMA
trigger or a hardware-exact MAME-derived model.  The finite receiving split is
accepted: the next bounded S establishes explicit local COP register ownership;
decoded command/DMA semantics remain a later S.

## S5 Coordinator Acceptance

The coordinator independently reviewed the committed implementation P1
`e284fb386` against the admitted S5 packet, S4 B07 evidence and the actual
source/test diff. One private MyRaidenII Core COP owner now installs every S4
observed configuration/register family and the observed command family as
replacement routes. The Board remains the sole composer; no Shared contract,
second bus, clock, scheduler, ROM payload, external source/table or generic
device framework was introduced.

Configuration writes accept the Core's existing byte/word dispatch and remain
private COP state. Reads and the unmodelled command range explicitly return
`LIB_STATUS_INVALID_STATE`, rather than `LIB_STATUS_UNSUPPORTED`, so a known
COP address cannot silently decline and fall back to ordinary RAM. The
expanded existing Board unit proves each configured route, byte/word dispatch,
the explicit command failure, an adjacent RAM boundary and reset route
retention without creating test-only readback or a parallel harness. Final
repository unit qualification passed `687/687` on x64 and x86; MyRaidenII
documentation governance and whitespace checks pass. S5 is accepted and
closed. COP command/result/DMA semantics remain the finite S6 receiver, and
the observed video/bank families remain outside the COP owner.

## S6 Coordinator Acceptance

The coordinator independently reviewed evidence P1 `38e778ea8`, its exact
temporary-root cleanup and its source-policy boundary. The probe intentionally
uses accepted pre-S5 `77110cf04` only as a disposable old-fallback baseline;
it changes no tracked source and cannot restore that behavior to the product.
It verifies both extracted lane hashes, retains only neutral aggregate access
counts and sequences, and deletes the entire 58,710,039-byte probe root after
review. No ROM byte, command value, raw trace, MAME source/table or production
model is committed.

Both fixed budgets reach the same finite fact: command writes occur, while
result reads and DMA triggers do not. The delivery correctly records that this
does not identify a command, result, timing or stall cause. S6 is accepted and
closed. S7 receives the complete finite B07 operation ledger and may implement
only operation behavior supported by a separate source-grade model and direct
receiver proof; otherwise it must transfer the missing evidence explicitly.

## S7 Coordinator Acceptance

The coordinator independently reviewed P1 `4cb7776f1`, the exact external
reference identities, the source-policy boundary and the final tracked diff.
S7 does not add production source, tests, ROM data, generated payloads or a
Shared contract. Its evidence correctly classifies the inspected MAME
materials as L2 only: their own incomplete/guesswork annotations prevent them
from proving a RAIDEN II COP command, result, DMA or timing implementation.

The complete unresolved S6 receiver is now explicit. T3 retains ownership of
any eventual COP command/result/DMA bus behavior, but may not invent it. The
periodic event and video-owned DMA discovery transfer to the later M1 Video
task, where one deterministic Video-owned event contract must be established
without fabricating a COP result. The task-owned three-file, 245,735-byte
research root was removed through `Clear-Build.ps1`; only neutral identities
and conclusions remain. Documentation governance and whitespace checks pass.
S7 is accepted and closed. T3 remains open pending owner-approved ordering of
the Video receiver or source-grade COP evidence; it has no active S.

## S8 Coordinator Acceptance

The owner approved advancing Video's periodic-event prerequisite within T3
and delegated routine S ordering on 2026-10-19. The coordinator independently
reviewed pushed P1 `34e22a100`: the complete source/header/CMake/test diff,
the packet and the result/provenance/cleanup evidence. The changed surface
matches B04's formerly missing periodic receiver. B07/B08 and the original
real-set startup exits remain open rather than being reassigned or waived.

One private Video phase now produces an explicitly L2 VBlank event on the
existing Core timeline; Board's existing IRQ latch and acknowledgement remain
the sole interrupt route. The nominal 16 MHz axis stays macro-proportional,
with no physical-time promotion. The local test build dependency is repaired
so the aggregate compiles the actual updated test before executing it. Source
and tests change only MyRaidenII, with +71/-2 production and +112/-1 test lines
including local CMake. No Shared API, copied reference implementation, ROM,
host timer, second execution path or receiving App artifact is added.

Both final complete repository unit runs pass 687/687 with zero failures/skips:
x64 128.82 seconds, x86 125.44 seconds. Earlier sandbox and stale-test attempts
are retained distinctly in evidence, not relabelled as final qualification.
Follow-up builds have no work; documentation governance and whitespace checks
pass. Six temporary files totalling 775,694 bytes are removed after every
owned session is terminal, with hashes and summaries retained. S8 is accepted
and closed. T3 remains open; the next batch returns to the finite B07/B08 COP
operation/evidence receiver. Full Video rendering and real-set/gameplay proof
remain their separately recorded requirements.

## S9 Coordinator Acceptance

The coordinator independently reviewed P1 `456cde1fb`, the actual three-file
documentation diff, bounded diagnostic method and retained input/probe/output
identities. No tracked production, test, build, ABI or artifact change exists.
The probe preserves the current command rejection and runs the configured
Board/V30 route with S8 Video advancement; it neither scripts completion nor
restores the old fallback. Ten temporary files, including both verified ROM
lane copies, are removed after their process is terminal (1788292 bytes).

The evidence adds a complete finite initial-upload inventory: 176 distinct
upload addresses, 22 trigger identifiers and one actual rejected `0x5205`
attempt after 68254 retired instructions. CPU IF is clear and no IRQ is
acknowledged despite advancing Video time. The record correctly distinguishes
these facts from a model of COP operation effects or S6's later stall cause.
All original B07/B08 families remain accounted for; the 22 uploaded IDs are
not relabelled the full game's execution universe. The next S10 receiver is
the whole upload/register protocol and three-member memory-transfer family,
with a safe Core transaction service point and explicit L2 functional grade.

Documentation governance and whitespace checks pass. S8's unchanged exact
production/test baseline retains its 687/687 dual-width qualification; S9
does not claim a new full unit, integration, EXE or game pass. S9 is accepted
and closed. T3 remains open and ordinary next-S admission needs no new owner
approval under the recorded delegation.

## S10 Coordinator Acceptance

Actual-change review accepts Shared P1 f2ca9ae01, NXVM receiving P2 990ff6eda
and MyRaidenII P3 af1649f60. S10 implements the entire admitted upload/register
and three-command L2 transfer batch, using existing public Core transactions
at the one Board service point. Unknown/incomplete/busy commands still refuse;
reset invalidates pending/upload state; two reads precede writes and a failed
write retains its accepted prefix. It is not a generic COP interpreter.

The owner's additional approval repairs the reproduced Core callback-fault
publication mechanism at exact instruction/tick budgets. One public run
boundary reports existing fault state consistently; no second executor,
public API, Shared persistent state, Lib/Emulator/MyNES change is added.
Production changes MyRaidenII +165/-7/net158 and Shared +28/-7/net21; existing
owned tests change +160/-2/net158 and +74/-1/net73 respectively. Two manifests
change +3/-3. The positive code owns transfer state and explicit boundary proof.

Both final full unit runs pass687/687 with no failures/skips (x64 135.95s,
x86 152.11s); an earlier x64 aggregate deadline remains an incomplete attempt,
not a pass. Eight0564 PC EXEs are rebuilt/deployed/hash/PE verified;14 selected
receiving integrations pass per width, with unchanged predicates/budgets and
zero CPU recompilation during profile switches. MyNES,33 protected tracked
inputs and six selected media-role hashes remain unchanged.

The [review](../etc/evidence/t3-s10-cop-transfer-review.md) and
[results](../etc/evidence/t3-s10-results.json) retain actual scope, model/source
grade, failure variants, release identities, complete archived packet and
cleanup proof. All owned processes exit before preview/apply cleanup of37
files/55430 bytes; only persistent caches remain. Documentation gates and
whitespace checks pass. S10 is accepted and closed. Other complete B07/B08
families, selected-set startup and all gameplay remain unqualified; T3 stays
open and no next S is admitted by this governance-only acceptance.

## S11 Coordinator Acceptance

Actual-change review accepts P1 b977bfa48. The complete0205/0905/2a05 family
is reconciled: two explicit L2 fixed-point position/velocity updates are
implemented;2a05 remains non-eligible until the same-T semantic receiver
establishes its output width/neighbour preservation. It still rejects after
a complete synthetic upload. This is not full-family or game qualification.

One bounded COP memory helper replaces old copy-only loops and is shared by
all five admitted macros; one private request-slot field preserves command
address meaning. Source changes +62/-18/net44; the existing Board unit changes
+131/-1/net130 with41 data-driven actual guest cases and independent literal
oracles. Source ownership, modular arithmetic, accepted-prefix failures and
neighbour-word preservation are reviewed, with no Shared/public API change,
copied reference body/table, new test executable or alternate path.

Both full repository unit runs pass687/687, zero failures/skips: x64 106.12s,
x86 126.27s. All ten release hashes,33 tracked guards and six selected media
roles remain unchanged. No disposable output is persisted; only both caches
remain. The directly stale partial-S49 architecture statement is corrected
to accepted T2, not used to rewrite historical evidence. The
[S11 record](../etc/evidence/t3-s11-cop-state-update.md) preserves source/model
identities, packet, complete family dispositions, actual delta and proof.
Documentation governance/whitespace checks pass. S11 is accepted and closed;
remaining COP, DMA/result and selected-set startup requirements keep T3 open.
