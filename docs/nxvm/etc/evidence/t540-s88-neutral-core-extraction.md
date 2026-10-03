# T540 S88 Neutral Core Extraction

## Accepted Core Delivery

Baseline is accepted S87 P2 `07f62906f`. S88 receives the entire neutral
source/test/build batch in the S42 ownership ledger: 16 C units and 19 headers
now live in `src/x86/core`. The former App implementations are deleted, and
NXVM links the sole Shared implementation. This completes the assigned Core
receiver, not T540: the flat common/AT/XT board receivers remain outstanding.
Coordinator acceptance is recorded in Current and task history after actual
pushed-change review of Shared P1 `8d3c57df3` and NXVM P2 `89f9416b2`.

## Ownership And Actual Change Review

- `x86-core` owns execution, guest timeline, CPU/FPU lifetime, bus transactions,
  RAM/port/ROM routes and bounded debug operations. It depends only on Types,
  CPU and FPU, never App assembly, PC chip wiring, Common or native services.
- `x86-core-observable` compiles the same sources for trace-contract tests.
  A consumer links one variant; Release keeps development trace disabled.
  NXVM retains board/display attachments and links Core, rather than compiling
  its source again. The duplicate proof-only OBJECT target is deleted.
- All 35 production moves were compared to the baseline after exact include
  substitution and LF normalization. Thirty-three are mechanically identical;
  only `machine.h` and `trace_interface.h` additionally relocate the existing
  trace writer declaration and Release no-op. No new function, field or
  algorithm is introduced.
- All 260 retained App C/header reference changes were compared by the same
  method. Four board sources additionally remove private Core includes:
  deadline/plan/KBC never needed them; advance uses the existing public trace
  writer. Algorithms and original assertions remain verbatim.
- Seven existing neutral tests plus independent-link proof move to
  `test/x86/core`. Their fixture uses genuine neutral construction and an
  explicit synthetic 386 reset alias. The 92h/8042 instance, checked-memory
  and firmware-capability fixtures retain their actual NXVM board constructor.
  The original App fixture remains unchanged; these are distinct synthetic
  setup responsibilities, not a second production constructor.
- Actual build review found duplicate root/Shared CTest registration. The root
  now checks the Shared directory's actual test names, not a hard-coded skip
  set; the registration verifier rejects duplicate routes before deduplication.
- The moved scheduler/bus initially escaped legacy gate scans. Their actual
  owner paths are now scanned; FDC imports/private phases and CPU private
  mutation still fail their original negatives. RAM backing authority now
  excludes only its two genuine implementation owners, not a whole directory.
- No live production/build include points to the old Core closure. The one
  remaining old CPU-bus header string is an intentional forbidden include
  injected by the negative test. App private-Core imports and restoration of
  an old Core source are both guarded and negatively exercised.

The line-count method is `git diff --cached --numstat -M` over the complete
S88 delta before target-separated commits, excluding documents, manifests,
READMEs and generated/artifact files. 354 source/test/build/tool paths add
753 and remove 728 lines (net +25). The narrower C/header
source/test count is 299 paths, +548/-515 (net +33).
The small positive delta supplies the neutral synthetic fixture and enforceable
ownership/registration guards; no additional production execution path exists.

## Requirement-to-Proof Record

| Required outcome | Final authoritative proof |
| --- | --- |
| Independently usable real Core | Standalone Release with X86_BUILD_TOOLS=OFF builds the actual x86-core target; standalone suite 121/121, 19.75 seconds. Eight original neutral cases remain in this suite. |
| NXVM uses one implementation | Actual CMake source-list/link diff; old App Core files absent; executor closure gate and copied-source duplicate negative pass. |
| No reverse or private dependency | Shared corpus/DAG/negative tests, App private-header gate, 34 electrical and ten construction/private-header negatives plus duplicate-source negative all pass; restored positives pass. |
| Original full unit coverage | Final x86 470/470 in 67.81 seconds; final x64 470/470 in 264.57 seconds under concurrent product builds. Both runs follow the last private-header cleanup. |
| Strict and specialized checks | Both width jobs exit 0 after their specialized targets; strict inventory has 402 rows, 380 strict and 22 previously deferred. |
| Complete shared identity | All six source/test manifests pass on final files; source/test x86 inventories cover 94/137 files. |
| Preserved products | Eight fresh optimized stripped 0540 products; architecture/banner/sections/freshness/hash inspection passes. Original unchanged-INI boot checkpoints below pass once on final inputs. |
| Unaffected MyNES and owner inputs | Actual MyNES build graph has no x86 dependency; no MyNES build or file diff; no INI or external-master changes. |
| Truthful governance | NXVM documentation governance and staged diff checks pass; no T540 or flat-board completion claim. |

Logs and temporary mutation/review harnesses are owned ignored diagnostics
under `build/s88-*`. The two final full-unit jobs and both product build jobs
have terminal success. Initial default checks predated the last declaration
move; the final-refresh job rebuilt affected early products and the final
default checks verified those changed inputs. This is not a three-retry
acceptance threshold.

## Final Boot Checkpoints

| Product | x64 | x86 |
| --- | --- | --- |
| default PC/AT | dos-prompt | dos-prompt |
| IBM 5160 Model 268 | installer-running | installer-running |
| IBM 5170 Model 339 | installer-running | installer-running |
| DeskPro 386 Model 40 | installer-running | installer-running |

The neutral-link executable also passes in each of the eight product trees.
Each integration command uses the original deployed profile/NXVM.ini and a
180-second containment budget; success is the observed terminal, not timeout
or retirement count. This proves the preserved finite boot checkpoints, not
indefinite absence of intermittent firmware faults.

## Final Embedded Products

The products were built from the final S88 source tree, to be fixed by its
Shared/NXVM implementation receipts. All have banner 0.5.0540, correct PE
architecture and no compiler-debug sections. Original external firmware
inputs and embedding policy remain unchanged.

| Artifact | Bytes | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1335049 | `65B030FEC73D65D64DE8B5E513DA2034408E58B644AB8F3AAEB74A0BFFC790E0` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505881 | `DA8DF969CFE4C0EF58A106443BD2BF861888020E3F54CD028F67274E3FD32C72` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1351366 | `161DC4A5887E5B8CCD9183CCE98E98C7ACEEC8747AA38772DC0615A309C4ABE5` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1522196 | `3322144B9AA08228709B75930EAD5E57E10D27AD70F8E33C7D9DA18C5EC74381` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1351334 | `3B24A2BAC10DBB6B8FD2CC8E3ABEC532E53479F481F6DD324FB842EF14CB3C3E` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1522163 | `90A6E0ADED20CBB4E7CCD47BABE112FAD5541EC4773EBEC031CC6B36E2771441` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1351400 | `C2AD9A629767DBC99DDE72B096EEB5FBFEF3FA46A6BE8B8956023995FB534BFE` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1522231 | `8690A9245413A92C7F6058BF7081275F3109FAA033D742F833CAF70243AED826` |

## Remaining T540 Boundary

Shared implementation receipt P1 is `8d3c57df3` and NXVM P2 is `89f9416b2`,
both immediately pushed to origin/master. The coordinator reviews their
combined actual delta from S87: 52 Shared-only paths and 365 NXVM-only paths,
unchanged verified blobs, no MyNES/INI delta and exact artifact hashes.
Post-commit comparison confirms 33 mechanical production moves, the two
reviewed declaration moves and 259 changed retained App C/header paths with
only four private-header removals beyond include substitution. The unchanged
App fixture is the 260th reviewed working candidate. Both post-commit 81-check
specialized jobs exit successfully; a sandbox run which never launched child
tasks was explicitly terminated and the same checks completed outside it.
Governance P3 accepts S88 only, not the remaining board extraction.

Core has no named PC-board pointer and owns one copied attachment binding;
board construction retains its actual local state and required callbacks.
`app-nxvm/devices` still contains real IBM-PC board adapters, including
display, chip wiring and composition. They are the next complete flat
`x86/ibmpc-common`, `x86/ibmpc-at` and `x86/ibmpc-xt` receivers. This S
does not invent per-profile copies, a generic device framework, a new host
worker or timing-grade claims, and does not close that outstanding scope.
