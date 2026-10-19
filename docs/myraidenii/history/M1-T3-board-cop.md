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
