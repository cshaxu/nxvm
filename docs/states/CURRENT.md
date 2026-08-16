# Project Status

## Current Work

**T386 open; S1--S19 accepted; S20 active.** S20 makes the accepted private
DeskPro 386/16 Model 40 composition usable only through a constrained, frozen
YAML BYOB profile entry: an owner-supplied two-chip ROM manifest is read and
validated before creation, the fixed backbone selects the reusable 1.2 MB
floppy device profile, and no arbitrary machine construction becomes public.
It is functional composition progress only; firmware execution, remaining
selected-device work, board timing and Model-40 L3 remain open.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T386 S20. |
| Admission And Approval | The owner approved continued DeskPro functional-before-timing work, directed that reusable hardware belong in Core or reusable VM device profiles while machine backbone/allowed variants remain in VM, and approved YAML selection of such constrained machine profiles. The source policy explicitly permits a separately admitted BYOB provider/manifest before machine creation. No exception. |
| Objective | Publish one constrained `compaq-deskpro-386-model-40` YAML backbone through the existing startup-frozen catalog and session-open route. It must require an owner-supplied two-slot Rev-E ROM manifest with exact local path, profile slot, 16-KiB exact size, SHA-256, read-only mapping request and non-empty user provenance; VM validates and copies both files before session creation. The selected Model-40 backbone fixes 80386/no-FPU/1 MiB, its accepted D4/CECG/DMA/HDC composition, and the reusable 5.25-inch 1.2 MB floppy profile. |
| Non-goals | No ROM/media import, repository path/hash/catalog/download mechanism, default firmware dependency, vendor-ROM execution claim, BIOS-service implementation, arbitrary YAML hardware assembly, Model-40 CPU/device/timing/L3 closure, D4 firmware copy/parity, CECG functional expansion, physical storage semantics, Core Model-40 knowledge, or generic SHA/public filesystem ABI. No new Console creation/edit/save operation. |
| Reference Baseline | T384 selected Model-40 capability audit; T386 S14 two-chip carrier, S16 D4 mapping, S17 dual DMA, S18 reusable floppy profile, S19 accepted baseline correction; T381 frozen YAML session catalog; current source at `960cf6fa`. |
| Candidate Proposal | `docs/proposals/m5-deskpro-386-model40-device-functional-closure.md`, with `docs/proposals/m5-80386-deskpro-386-l3-baseline.md` and the source-policy BYOB manifest rule. |
| Files And ABI Surface | `src/vm/product` owns catalog schema/discovery and frozen manifest fields; `src/vm/composition/session` owns option-to-session application and transaction/failure cleanup; `src/vm/profile/model40` owns the Model-40-only local ROM load, SHA-256 check, copied carrier and fixed machine contract; `src/vm/profile/device/floppy` remains the reusable geometry selector. Core machine/session contracts must remain profile-neutral and unchanged unless a demonstrated generic defect requires its own admitted receiver. Tests create only project-owned synthetic ROM files in an owned temporary directory. |
| Applicable Rules | `docs/rules/EXECUTION.md`, `docs/design/ARCHITECTURE.md`, `docs/design/CODING.md`, `docs/rules/ARCHITECTURE.md`, `docs/rules/CODING.md`, `docs/rules/DOCUMENT.md`, `docs/etc/operations/policy/source-policy.md`, and T381/T386 proposal boundaries. Require one mutable-state and failure owner; configuration is frozen at startup/session-open; all valid Model-40 file bytes are copied for session lifetime; no protected asset enters Git. |
| Verification | Add a catalog and public production-route regression using synthetic 16-KiB ROM pairs. Prove accepted manifest parses, wrong/missing slot/path/size/hash/mapping/provenance rejects, pre-create hash validation rejects mutation, valid session has 80386/no-FPU/1 MiB/1.2MB FDD and accepted private composition, then source files may be changed without changing the live copied carrier. Prove atomic failure leaves no session. Retain default/Model-339 catalog and session controls. Sweep all profile parse/selection, firmware-provider, model40-private construction, FDD selection and external-file loads. Run focused tests, dependency/ownership checks, serial current gate, artifact build, documentation governance and actual-diff review. |
| Expected Markers | `M5:T386:S20:MODEL40-BYOB-MANIFEST:OK`, `M5:T386:S20:MODEL40-BYOB-VALIDATION:OK`, and `M5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK`. |
| Asset Needs | Operator-supplied ROMs may be read only from an external local path at runtime and are never committed. Verification uses synthetic project-owned bytes and an expected digest generated within the test; evidence retains neither an owner path nor vendor digest. No guest media or third-party source is needed. |
| Reporting Requirements | Record the exact profile-local manifest schema and refusal behavior, frozen-copy ownership, Core/VM boundary, synthetic proof, source-policy compliance and remaining transfers in indexed evidence. Push one complete implementation P, then perform an independent one-session coordinator actual-change review and push the governance P. |
| Stop Conditions | Stop and return for owner direction if the selected Rev-E hardware needs a different chip geometry/mapping, a vendor asset/catalog/discovery mechanism, a shared public filesystem/hash capability, a generic profile override outside Model-40's allowed contract, or an unproven BIOS/device behavior. Stop if a valid manifest cannot create or rollback atomically without a Core contract change. |
| Exit Criteria | The frozen catalog admits only a complete Model-40 BYOB manifest; invalid values fail before session publication; valid synthetic files produce the fixed Model-40 composition through the ordinary Console/session route and remain valid after source-path mutation because session-owned copies are used; the reusable 1.2-MB profile is selected; default/Model-339 remain valid; no vendor bytes, paths, hashes or catalog enter Git; all stated proof and gates pass. |
| Original Owner Request | Achieve complete DeskPro 386 Model 40 functionality and L3 timing under the approved queue and capability-ledger-first order. Reusable hardware belongs in Core or reusable device profiles, machine backbone/allowed variants belong in VM, and YAML must let users select those constrained backbones/variants. |
| Similar-Issue Sweep | Sweep every session profile identifier/parser/options route, catalog field/path resolver, external-file/firmware materialization, Model-40 private/public constructor, composition failure path, fixed-profile override rejection, FDD device-profile selection and focused catalog/session controls. Classify each as generic Core, reusable VM, Model-40 profile-local, retained consumer, or later device/timing work; repair the actual owner or record a bounded transfer. |

## Current Technical Baseline

- **Current developer artifact:** T386 S18 `vm-0-5-0389` /
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `29A174DBC889A1220B0F4BD17805AAA78B326F01B31E5D68D5688F6B5DEC09E4`.
  Built from the S18 worktree after its 273-test serial current-gate; the
  floppy device-profile proof is retained in
  [S18 evidence](../etc/evidence/t386-s18-model40-floppy-device-profile.md).
- **Model-339 readiness:** T383 S1 accepts the current runnable source graph
  under the frozen deterministic event-and-bus L3 contract. This is only the
  selected Model 339 configuration and preserves every documented physical,
  fixed-disk, generic-PC/AT and later-machine boundary.
- **T285 display implementation:** `INT 10h` mode `10h` /
  `EGA-640x350x16-direct` has a VADP-owned planar frame path and copied-frame
  consumer boundary; mode 0Dh remains a separate retained path.
- **Core boundary:** T243--T246 retain checked physical memory, immutable ROM
  mapping, and validated real-mode entry plans. The obsolete post-`#UD`
  transition has no public or runtime path.
- **Product boundary:** `nxvm.exe` is the retained runnable product. `mantle`,
  `dos`, and `nxvdm.exe` remain future architecture commitments; they are not
  current runtime or release products.

## Recent M5 Closures

| Task | Compact result |
| --- | --- |
| T386 S19 | Accepted: corrected S18 baseline provenance to artifact 0389 and 273/273 serial current gate; no runtime or artifact change. [Evidence](../etc/evidence/t386-s18-model40-floppy-device-profile.md). |
| T385 | Closed: the selected Model-40 80386DX-16 consumes the shared 80386 CPU/state/retirement owners without a DeskPro CPU fork; a clean current-source 251/251 gate passed, while functionality and board timing transfer unchanged. [Closure audit](../etc/evidence/t385-s1-task-closure-audit.md). |
| T384 | Closed: the Model-40 capability audit freezes its original Compaq system board/storage ledger and owner-selected color EGA, keyboard and external-ROM constraint; its shared-VADP IBM/Compaq personality boundary transfers CPU, functional and timing work without making a runnable or L3 claim. [Closure audit](../etc/evidence/t384-s2-task-closure-audit.md). |
| T383 | Closed: independent current-source replay accepts the frozen Model-339 deterministic L3 contract after T380's strict-start repair; 251/251 current gate and focused matrix are retained in [evidence](../etc/evidence/t383-s1-model339-current-source-reaudit.md). |
| T382 | Reclosed at S8: the prior S1--S7 closure is retained historically, while S8 corrects its Console 64-bit transport and final artifact baseline. [Corrective audit](../etc/evidence/t382-s8-reclosure-audit.md). |
| T381 | Closed: S1 P1 `9b6a86c6` delivered frozen YAML profile selection and the true zero-session Console; corrective S2 restored its task-level closure state without changing runtime evidence or the T381 artifact. [Evidence](../etc/evidence/t381-s1-console-yaml-profiles.md). |
| T380 | Closed: strict Model-339 startup, 512 KB FDC/BDA repair, input/FDC current-gate recovery, and the owner-replayed DOS 5/HIMEM no-XMS path are verified. It does not claim DOS completion or 5170 L3 closure. [Evidence](../etc/evidence/t380-s2-model339-start-and-current-gate-recovery.md). |
| T378 | Closed at `42f0402a`: production ingress proof and virtual-time ownership are corrected without a test-only API; Model-339 re-audit remains the sole L3 decision receiver. [Evidence](../etc/evidence/t378-s1-native-ingress-boundary-repair.md). |

## Recent Governance

- **M5 Td S101 P1:** added a compact execution-entry table without creating
  new paths, and made coordinator semantic review explicit after structural
  documentation-gate success.
- **M5 Td S100 P1:** archived the closed T382 proposal, queued Model-339
  current-source re-audit before DeskPro, reconciled T380 S2 history, and made
  host/workspace path detection reject the external-assets form while retaining
  guest DOS-path examples.
- **M5 Td S99 P1:** compacted documentation gates, closure evidence, artifact
  identity, build hygiene, and recorder containment within Execution; design
  authorities remain unchanged.
- **M5 Td S98 P1:** made coordinator-review inputs, exit criteria, and the
  executor-report boundary explicit in Role cycle.
- **M5 Td S97 P1:** added explicit single-/separate-session review navigation
  without changing the lifecycle requirements.
- **M5 Td S96 P1:** compressed duplicate Role cycle prose into references to
  its retained authorities without changing lifecycle requirements.
- **M5 Td S95 P1:** removed duplicate M5 technical narrative from Queue while
  retaining all shared-context, candidate, and required proposal-index links.
- **M5 Td S94 P1:** locks the DeskPro 386 Model 40 and PC/XT 5160-268
  baselines, establishes capability-ledger then functional-before-timing
  closure for all three machines, and retains external ROMs outside Git.