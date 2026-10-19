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
