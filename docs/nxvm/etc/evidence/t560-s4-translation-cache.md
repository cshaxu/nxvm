# T560 S4 Translation Cache Investigation

## Baseline And Delivery Boundary

Admission baseline: bbc8b85c6, clean after accepted S3 cb52586a9.
CURRENT owns the active S4 packet. This record contains inspected sources,
the first implementation and focused proof, not completed C06 qualification
or a pushed S4 delivery. C06 remains open.

Target is NXVM only. Shared-six, MyNES, owner INIs and external ROM/media are
excluded. The eight S3 0560 binaries remain the latest verified release.

## Original Source Inspection

The pinned originals and their identities are in
[T558 source availability](t558-s2-source-availability.md#acquired-intel-originals):

- Intel 486 PRM: sections 5.3.5 and 10.6.1--10.6.3, printed pages
  5-22 and 10-8--10-11; INVLPG at 26-172.
- Intel 273021-001 developer manual: sections 6.4.5--6.4.6, appendix B.3,
  printed B-8--B-12; cache replacement discussion in 7.5 is a comparison,
  not permission to equate the cache and TLB allocation policies.

Read the complete relevant TLB text, not only search matches. Rendered and
visually inspected PRM PDF page 260 (Figure 10-4/Table 10-4) and developer
PDF page 432 (Figure B-6/Table B-4). Ignored renderings are under
`build/t560-s4-research/`; no original PDF or extracted manual is committed.

Verified hardware-visible facts:

1. Eight sets, four ways, 32 entries. Linear bits 14:12 select the set;
   bits 31:15 form the tag. Physical bits 31:12 form the translated page.
2. Each entry has V, D, U/S and R/W, with PCD/PWT in its data block.
   Each set has three pseudo-LRU bits. This is actual chip state, not a
   product optimization cache or a second copy of the page tables.
3. Developer B.3.1 explicitly says TLB replacement can replace a valid
   entry even with an invalid entry in the set. Do not add an empty-slot-first
   rule merely because that is appropriate for the on-chip cache.
4. TR6 bits 11:5 are V, D, D#, U, U#, W, W# respectively; bit 0 selects
   write (0) or lookup (1). The complementary lookup pairs mean never
   match (00), match clear (01), match set (10), or match either (11).
   Writes only define 01 and 10. Duplicate tags and other undefined test
   inputs must not be invented as architecturally successful extra commands.
5. TR7 bits 11/10 are PCD/PWT; bits 9:7 report replacement state before
   the lookup. Bit 4 selects explicit replacement for writes and reports
   hit for lookups. Bits 3:2 select/report the way. Both test writes and
   successful lookups update replacement state. Paging may remain enabled.
6. Loading CR3 empties valid entries, including task loading. INVLPG
   invalidates the matching page without reading its memory operand;
   a register encoding is illegal, and privilege restrictions apply.
7. Present page-table edits need explicit invalidation to become effective.
   Not-present translations are not cached. A permission check on a cached
   translation still depends on the current access privilege and CR0.WP.

The PRM and developer source give architecture and test bit fields directly.
The source/model reconciliation below distinguishes documented architectural
fields from model-level internal choices. The on-chip cache's Figure 7-2
alone does not prove every TLB detail or complete page-walk bus timing.

### Source And Model Reconciliation

- Geometry, tag/attribute/test bit positions, complement matching, explicit
  replacement, lookup-before-update readback, invalidation and legal
  instruction/privilege effects follow the inspected original manuals.
- The implemented three-bit replacement tree uses the developer manual's
  Figure 7-2 bit order and update rule, with B.3.1's explicit TLB difference:
  do not prefer invalid slots. The exact internal TLB-bit correspondence is
  an L2 model choice, not a claim that the manuals directly measure those
  silicon bits. Reset chooses zero for that private replacement state;
  undefined reset/tag and duplicate-tag results are not qualification cases.
- Dirty-clear writes rewalk at the ordinary translation owner after checking
  cached protection. The local Bochs source corroborates this model-level
  mechanism; a later clean-to-dirty walk replaces the cached entry only after
  successful page-table publication. No stored host/PTE pointer is added.
  This is L2 internal modeling with direct CPU data/fault proof, not a claim
  of microcycle-exact dirty-update circuitry.
- INVLPG resolves the existing effective address, adds the selected segment
  base and invalidates its page without an operand read or page-table walk.
  PRM 26-172 lists register-form UD and privilege GP, not an operand read
  fault. Read-only inspection of the local Bochs `paging.cc` INVLPG handler
  corroborates effective-address plus segment-base calculation without a
  data-access check. The 86Box source's additional segment check is not
  imported as an undocumented NXVM exception. Valid segment/size forms have
  direct proof; do not infer a later x86-64 canonical-address contract.
- The earlier PDE/PTE A/D preparation/publication mechanism is retained.
  Developer 6.4.5's second-read/locked page-table update and complete external
  bus-cycle fidelity are not discharged by adding a TLB. C04/C16 remain open
  for that physical-bus reconciliation in T560; cache/bus work must consume
  it before full CPU admission. No test pass or C06 state proof closes that
  existing wider obligation, and no CPU requirement leaves this T.

These are upgrades from absent TLB hardware, not downgrades of an accepted
486 L3 translation-cache model. No accuracy grade is inferred from boot or
from a test reproducing the implementation's chosen replacement tree.

## Actual Current Owner And Caller Sweep

Inspected the real flat CPU directory; there is no `cpu_memory.c`,
`cpu_execution.h` or standalone instruction-metadata file to migrate.

| Existing path | Current implementation and S4 obligation |
| --- | --- |
| `cpu_instructions.c`, `_kma_prepare_physical_linear` | Direct PDE/PTE reads and permission checks, no TLB lookup. Keep this the preparation owner and introduce cached translation here, not in the bus provider or Board. |
| `_kma_commit_physical_linear` | Publishes A/D after checked access preparation; split callers prepare both pages before both commits. Cache publication must respect this existing failure boundary. |
| `_s_write_cr0`, `_s_write_cr3` | Guest control writes invalidate prefetch, not translation state. Task32 completion already calls the same CR3 writer. Add translation effects at these owners. |
| `MOV_TR_R32`, `MOV_R32_TR`, `_d_modrm_treg` | TR6/TR7 are passive storage; TR3--5 reject. Implement C06 commands on the real TLB while leaving C07 cache test ownership explicit. |
| `INS_0F_01`, extension 7 | Currently rejects INVLPG. Add source-qualified memory/address and privilege handling without operand read or a new opcode dispatcher. |
| `core_machine_cpu_instruction_lexeme_modrm_form_valid` | Independently rejects `0F 01 /7` and TR indices below 6. The real handler fix must include this lexical form owner; do not claim decode completeness from execution alone. TR3--5 remain C07 rather than being accepted without effects. |
| `cpu.c`, context initialization/state reset | Architectural CPU and private executor state reset through existing lifecycle. New translation storage must be initialized and invalidated by these paths. |
| `core_machine_cpu_debug_patch_registers` | Uses a candidate context/CPU, validates segments, then publishes CPU registers. Successful CR changes must invalidate translation state at publication; rejection must preserve the live cache. Candidate segment validation must use the candidate translation context rather than stale live mapping. |
| `core_machine_cpu_execution_preview_lexeme` | Copies context/CPU/instructions and sets preview mode. Preview must observe applicable cached mappings without mutating the live cache or writing A/D, and must not publish external cycles. |
| `cpu_test_register_smoke.c` | Proves retained 386 transfers and illegal forms/privilege/fetch. Its passive transfer cases are deliberately not 486 hardware proof; preserve them. |

Queries used `rg` for prepare/commit, control writers, TR6/TR7,
preview/reset/debug mutation and INVLPG across `src/core/chips/cpu` and
direct CPU tests. The absence of an INVLPG spelling is not itself proof of
every metadata path; dispatch/lexeme and observation still need full form audit.

### Read-only Reference Cross-check

Rehashed both pinned Intel originals; identities match the T558 record.
The local 86Box reference is clean at
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`. Locating its protected-mode
INVLPG implementation is only a research lead, not a source import or
evidence that its generic translation cache matches 486 TR6/TR7 hardware.

Inspected the dirty-access design comment in the non-Git local Bochs 2.6
reference `cpu/paging.cc`, SHA-256
`084bc92ee4d88b3396a9c9278574ea473ba5cc80f3f27b3c93edd5c8b0375731`.
It corroborates a page rewalk when a first write must update dirty state.
Its optimized permission representation also depends on CR0.WP and forces
cache flushing when WP changes. That optimization is not adopted: NXVM's
planned entries retain architectural U/S and R/W and evaluate current WP
at the existing permission owner. This avoids turning an optimization's
invalidation requirement into a guessed hardware rule. No source is copied
or transliterated; remaining 486 dirty/test-injected cases still need direct
source reconciliation and actual instruction proof.

## Intended Single-Owner Design And Required Direct Proof

The private execution context owns a bounded 8-by-4 TLB and eight replacement
states. Entries store real translated pages and attributes, not mutable
external page-table pointers. Ordinary translation and TR commands consume
this same storage. Existing paging permission and fault helpers remain the
validation/publication boundary; no public TLB object or new Shared API.

Before production edits, finalize the remaining source decisions and refine
the packet's preliminary estimates: production +250--500/-50--150,
tests +350--600/-30--80, excluding docs/manifests/artifacts. These are
estimates, not a justification for dropping necessary hardware behavior.

Required execution matrix:

- Every set/way, explicit and internal replacement, hit/miss and conflicting
  tags; reads report pre-lookup LRU and real page/attribute contents.
- Test injection drives actual translated CPU memory; ordinary page walking
  creates entries visible through test lookup. Paging-enabled tests use
  stable code mappings, not direct state edits standing in for MOV TR.
- Page-table change remains stale until CR3/INVLPG; present/permission/WP,
  A/D and dirty transition effects, split-page preflight and failed commits.
- INVLPG register/memory, address/code size, segment override, real/protected
  CPL0--3/VM86, LOCK, failed fetch, no matching entry, unchanged flags and
  absence of target-data reads.
- Guest/task/reset/debug invalidation and failed-debug atomicity; preview
  retains live entries/replacement/A-D exactly while fetching correct bytes.
- Retained 386 and older-family tests remain separate applicability proof.

## First Implementation And Focused Proof

The original CPU translation owner now checks a private 8-by-4 TLB and
publishes entries at its existing translation commit boundary. TR6 commands
operate on that same state; TR7 provides command data/readback. INVLPG is
connected in the existing group handler and lexical/metadata form checks.
Guest/task CR3 loads, paging changes, reset and successful debug CR writes
invalidate the relevant private state. Debug candidate CR0/CR3 values are
prepared before segment validation, so enum ordering cannot validate a
segment through the old translation context. Failed patches do not publish
that candidate cache.

The first implementation uses three-bit tree replacement without a
free-slot-first preference, and page rewalking for the first dirty-clear
write. These model choices remain subject to the explicit source
reconciliation above; focused tests prove the implemented mechanism, not
unmeasured silicon-specific bit encoding or all bus-cycle ordering.

Actual guest-command proof currently covers:

- All 32 set/way locations, including paged actual data access through
  a test-written mapping rather than the backing page-table mapping.
- All eight D/U/W attribute combinations against all 64 lookup-pair
  combinations (512 matching cases), including forced hit/miss pairs.
- Explicit/internal replacement, pre-lookup replacement-state readback,
  matching and no-entry INVLPG, unaffected other entries and CR3 flushing.
- Ordinary translation visible through TR lookup, stale page edits until
  INVLPG, cached instruction preview with unchanged live context/A-D,
  successful debug CR3 invalidation and reset invalidation.
- First write publishes Dirty; subsequent already-dirty hit performs only
  the data write and does not resurrect a software-cleared memory D bit
  without invalidation.

The existing control paged-table regression changed a present PTE without
invalidation. Its new guest INVLPG preserves the original readonly/Accessed
fault assertions instead of weakening them to fit stale cached permissions.
An initial new fixture fault was due to its null/short protected data segment;
the fixture now supplies a valid selector and flat data limit, retaining
ordinary CPU segment checks. Initial test compilation errors used guessed
Types spellings; corrected to the existing `lib_c_fprintf`, stream macros
and `LIB_UINT32_MAX`, with no Types changes.

Both original Ninja/ccache graphs build the affected CPU targets. The same
five selected CTest cases pass 5/5 per width: cpu_tlb, cpu_test_register,
cpu_control_state, cpu_rmw_access and cpu_alignment_access. Core source/test
manifests and whitespace checks pass. This is focused proof only.

Current counted changes, excluding docs/manifests/artifacts: production
three files +237/-9, net +228; tests include the new 272-line CPU test plus
seven control-fixture lines and one registration, +280/-0. These are
intermediate counts, not final S4 delivery statistics.

Remaining before delivery: complete INVLPG code/address/segment/mode/LOCK/
failed-fetch matrix with direct no-target-read proof; complete positive TR
prefix/mode forms; task/debug-failure invalidation and page-walk commit-failure
proof; source reconciliation for every model choice and similar-path review;
full repository units, applicable integrations, eight rebuilt deployments
and actual-change/coordinator review. No desktop/RDP or Windows GUI result
is claimed. No S4 commit or artifact replacement has occurred.

## Complete Matrix And Repository Unit Qualification

The later matrix extends the same CPU-owned test, not a new test framework:

- 96 combinations of code size, operand/address overrides, real/protected
  CPL0 and selected segment; 32 positive TR register/prefix/mode forms.
- All 24 memory ModRM forms per address size, with every SIB byte for all
  three memory modes: 813 effective-address cases, including negative
  displacements, no-base SIB and default SS/DS selection. Target data-read
  counters stay zero during INVLPG.
- Real UD for register/LOCK/old-profile cases; valid GP receivers for
  protected CPL1--3 and VM86, with unchanged translation state.
- Every byte of the prefixed INVLPG stream can fail instruction fetch without
  invalidation or architectural register mutation. Existing TR fetch-failure
  tests now cover both 386 and 486 directions/registers.
- Page-table commit failure does not publish the target translation or read
  target data. Rejected combined debug CR3/CS patch leaves CPU, TLB and LRU
  unchanged; accepted CR3 patch invalidates at publication.
- The existing 256 PDE/PTE/CPL/WP/direction combinations now run both cold
  and after a real supervisor read populated the TLB, giving 512 cases with
  the original fault/CR2/frame/memory assertions.
- Five actual task-transfer routes seed a real TR-written translation before
  the existing 486 completion-flags matrix; every implicit CR3 load clears
  it. Real MOV CR0 paging enable and state reset have direct invalidation proof.

Whole repository unit qualification is complete for the current source:
x64 600/600 in 260.20 seconds, x86 600/600 in 69.20 seconds. Logs are
`build/t560-s4-x64-unit.log` and `build/t560-s4-x86-unit.log`. This includes
Lib, Emulator, x86, Core and all five Apps, with three automated desktop
labelled unit cases per width. It is not manual RDP or Windows GUI proof.

Both four-profile deployment/integration loops are running on the same
persistent graphs. Their final results, hashes, actual diff and acceptance
remain pending; the preceding intermediate count/result paragraphs retain
the investigation chronology and are not final S4 closure claims.

## Final Delivery Qualification And Review

All eight final profile/host-width integration groups passed, each run once:

| Profile | x64 | x86 |
| --- | --- | --- |
| Default | 35/35, 59.24 seconds | 35/35, 69.71 seconds |
| XT | 15/15, 34.55 seconds | 15/15, 39.89 seconds |
| AT | 17/17, 51.65 seconds | 17/17, 59.07 seconds |
| DeskPro | 17/17, 151.68 seconds | 17/17, 179.66 seconds |

Logs use `build/t560-s4-<width>-<default|xt|at|deskpro>-integration.log`
and the corresponding `-integration-details.log`. Aggregate counts include
the same 12 MyNES integration cases in every profile, not distinct repeated
coverage. Two automated native desktop-labelled cases per group pass; manual
RDP interaction is unexecuted. The Windows31 checkpoint remains DOS C-drive,
directory and BDA proof, not execution of the Windows 3.1 GUI.

At final review, added one test-only assertion that an initialized invalid
tag can be queried with V=0 after INVLPG. Both current cpu_tlb test binaries
were rebuilt and passed (x64 0.13 seconds, x86 0.10 seconds); details are
`build/t560-s4-<width>-tlb-final.log`. Production and every other unit input
remain unchanged, so the passing 600-case full runs retain their proof for
those unchanged cases. This test-only addition does not require rebuilding
the unchanged production artifacts a second time.

Counted C/H/CMake paths: production cpu.c, cpu_instructions.c and
cpu_instructions.h; tests cpu_tlb_smoke.c (new), cpu_control_state_smoke.c,
cpu_task_switch32_state_smoke.c, cpu_test_register_smoke.c and x86_tests.cmake.
Tracked `git diff --numstat` plus the new test's 606 lines gives production
+237/-9, net +228; tests +671/-2, net +669. Documentation, manifests and
binary files are excluded. The positive increase implements real missing
32-entry hardware state and its complete owner-local execution matrix;
it does not introduce a device framework, thread, queue or public API.

Six necessary operations own invalidation, lookup, replacement selection,
replacement update, translation publication and test commands. State is
one private CPU TLB plus its replacement bits. The existing temporary
translation record carries the checked page until commit; it is not another
persistent mapping. Retained 386 passive test-register transfers have live
old-profile tests and remain an explicit older-family contract. TR3--5/cache,
SL/VME/SMM, complete bus/timing and production admission remain T560 work.

Executor actual-diff review read all production changes, the complete new
CPU test and three extended CPU tests/registration, manifests, active packet
and evidence. It confirmed no Shared-six/MyNES source or asset change,
no App/Board TLB state, no target-memory read for INVLPG, and atomic debug
TLB invalidation at successful candidate publication. Core manifests,
dependency and documentation gates and whitespace checks pass.

Both original persistent Ninja/ccache graphs are restored to default after
deployment; shared targets were reused between profiles. Eight verified
stripped Release version-0560 artifacts, with PE machines 8664/014C:

| Artifact under assets/ | SHA-256 |
| --- | --- |
| my5160/my5160_0_5_0560_x64.exe | 1D8D02B4F56BA0AB84992E08BB70FC16B6D45A89F5655737B12444A6AEF35135 |
| my5160/my5160_0_5_0560_x86.exe | 6A532FF697865F19FC68B983CF8A326EB0043C995120C913302FE11FBC9808D6 |
| my5170/my5170_0_5_0560_x64.exe | 167F3F3E83EF9436FC25FFAD77C3DA5D1B3F981F4D59539CD3A376EFC4F057D8 |
| my5170/my5170_0_5_0560_x86.exe | 04A72D74580528A9859241682A64DC5AF85AD159BAF99D513138E74180CCCB8E |
| mydeskpro386/mydeskpro386_0_5_0560_x64.exe | 470148772A8255740436EADFFAF273EB30CAB6E9F86D2AA9A708A8476585BDF8 |
| mydeskpro386/mydeskpro386_0_5_0560_x86.exe | 86F89721A0C0362467D518056C985A2678F5859BF9D0E8D6BDCB74E5CBA6409C |
| nxvm/nxvm_0_5_0560_x64.exe | C9C65918FCF165CBE9B7FFADFD28E92DBE139970349DC2BCAE7E84AAEDB031FA |
| nxvm/nxvm_0_5_0560_x86.exe | 9AA7E5D0514CF9CFF00595D03675044356C0A97841048E32BF316EE1E96F266E |

Owner INIs remain unchanged; no ROM/media/snapshot or MyNES artifact is
edited. MyNES still matches its accepted 0044 hashes. No whole CPU, PC110,
microcycle-exact TLB, manual RDP or Windows GUI qualification is inferred.
Complete implementation P push and coordinator actual-change acceptance
remain required before S4 closes.
