# T540 S74: firmware publication boundary

Baseline: accepted S73 P2 `aec7d3b76`. Coordinator admits the entire binding
transaction and automatic board reset-alias completion. Executor confirms
the sixteen-field Current packet.

## Owner decision

Core owns provider/context, ephemeral callback guard, immutable ROM registry,
routes and checkpoints. Board owns the PC F0000h window and CPU-family high
reset alias. Move binding to existing `machine_firmware.c`; one optional
synchronous construction completion follows the existing attachment pattern.
Board uses copied CPU profile plus neutral range-presence and atomic reset
window operations, never mappings, images, RAM pointers or registry snapshots.

Window publication intersects existing mappings in source order and preserves
reset-overlay priority. Failure restores its alias checkpoint; outer binding
failure additionally restores all provider-configured ROMs. Prior mappings and
retry remain intact. No second provider/context or retained policy is created.

Preserve source-presence width 15 and target-presence width 16, sparse/clipped
windows and explicit high mapping precedence. Structural extraction does not
silently change their historical asymmetry. PC constants remain board-owned.

## Sweep and verification plan

Search `src/app-nxvm`, `test/app-nxvm`, `cmake/nxvm` for binding definitions,
`firmware_provider`, `firmware_context`, `immutable_rom_mapping` and rollback.
Core implementations/same-owner tests remain legitimate; remove/gate board
registry/provider/context hits. Profile ephemeral-capability configure callbacks
and the sole production lifecycle binder caller remain unchanged.

Extend existing ROM/reset/neutral fixtures with synthetic owned bytes; full
x64/x86 units/gates, independent actual-source linkage, eight Release products
and one headless overlay boot each. This proves neither GUI acceptance nor
intermittent-frequency bounds nor completed physical Shared relocation.

## Implementation review

One private completion slot is installed before fallible board initialization
and cleared by its sole finalizer. It is synchronous, absent for standalone
Core, and invoked only after a successful configure callback has released the
ephemeral firmware guard. It returns status into the existing binding rollback;
there is no new lifecycle, execution loop, provider context or scheduling path.

All board firmware/provider/context and ROM-registry borrowing is removed.
Remaining private hits are only the neutral ROM owner, firmware callback owner,
Core reset/after-run/destruction and the memory-registration capability guard.
Profile plan/provider values are composition inputs, not another Core registry.
The sole production binder caller remains `machine/lifecycle.c`. Same-owner
tests deliberately inspect rollback counts; test relocation is not completed.

The new window operation validates both 32-bit ranges without addition overflow,
scans only mappings present at entry, and publishes their clipped subranges in
existing order. It does not recursively alias newly appended mappings. Its
checkpoint owns only new aliases; outer binding owns configure plus completion.
The controller-authority gate rejects board private firmware/ROM borrowing and
requires the binder definition/completion/rollback in the neutral owner.

P1 `git diff --numstat` counts eight tracked source/test/gate paths: +261/-94,
net +167. The P2 gate correction adds +10/-7, making the final total +264/-94,
net +170. Production alone is +106/-91, net +15; synthetic regressions are
+135/-3, net +132; the existing static gate is +23. Positive production growth
provides an independently reusable bounded publication operation and atomic
failure behavior while deleting the board's registry traversal and transaction.

## Completed local verification

- Independent actual-source linkage, firmware capability, ROM transaction and
  reset-ROM tests pass their existing markers on x64. The neutral fixture now
  actually binds a firmware provider without any board archive or attachment.
- Full units: x64 470/470 in 269.09 seconds; x86 470/470 in 76.70 seconds.
  Native targets were incrementally built first, then complete suites ran
  serially between widths with `ctest -L '^unit$' -j 8 --output-on-failure`.
- Both `verify-current-specialized-gates` builds return zero. Direct strict
  compilation retains 402 rows, 377 strict and 25 declared deferred owners.
- Documentation governance and `git diff --check` pass. An initial manual
  authority-script invocation used a relative project root and failed before
  inspection; the corrected absolute-root invocation passes. Registered gates
  use their configured absolute root and both complete runs pass.

## Product and boot verification

Eight Release product/probe/independent-Core builds returned zero using the
existing `t535-s4-<machine>-release-<width>` trees and targets
`vm-0-5-0540`, `vm-profile-floppy-boot-matrix`,
`core-machine-neutral-link-smoke`. All eight independent probes emit
`M5:T540:S69:NEUTRAL-LINK:OK`.

The matching probe consumes `assets/nxvm <profile>/NXVM.ini 180000` once per
product: default x64/x86 reach `dos-prompt`; XT, AT and Model40 x64/x86 reach
`installer-running`. All eight return zero; no repeats or synthetic guest
key injection are used. This is a headless firmware/media boundary check,
not native presenter/audio/manual-GUI acceptance or a complete T integration
claim. INI content, external master media and firmware inputs remain unchanged.

Build input is `aec7d3b76` plus this complete S74 implementation diff; the
pushed P1 identifies that source delivery. Product revision remains 0.5.0540.
All eight deployed files pass `objdump -f` PE-width and `objdump -h` absence
of `.debug` checks; product build checks confirm optimized Release. Values
below are full SHA-256. Each file remains beside its unchanged owner INI under
the existing `assets/nxvm/<profile>/` directory.

| Product file | SHA-256 |
| --- | --- |
| `nxvm_default_0_5_0540_x64.exe` | `13F1A3C90A3B0763626E05F12D1D127ADBCE3F9ED808A73D26FA999342C0C4C0` |
| `nxvm_default_0_5_0540_x86.exe` | `4E5B37429C7771198B8AA0E97851249807130904F82B0738E08AA0E7AB76B1DE` |
| `nxvm_xt_0_5_0540_x64.exe` | `9D46249401DA72148C8E89F95CFFCC359865E5EC29D234D61B070C0705A9B227` |
| `nxvm_xt_0_5_0540_x86.exe` | `4343BE0D98A591CB4CFC9D25069F7D4C7BD16921BA7D3E65B8A4F9B4A09728CE` |
| `nxvm_at_0_5_0540_x64.exe` | `F3515AA2A6C53B41CC0849C4D1EA1742FD8BB9653161B5CBC8DD368D2B734E10` |
| `nxvm_at_0_5_0540_x86.exe` | `8F2C5FA210FEA9539603202729241C1A394E9190CC7D9AEC1FBFB613BB66078B` |
| `nxvm_model40_0_5_0540_x64.exe` | `925732A05EA3F451128746312FC6A887274BD708052401D0EC34A6015CC43A96` |
| `nxvm_model40_0_5_0540_x86.exe` | `A467B0833E906644A342E3CA1BE7F80D79D794D8FC05A5C9B484144EA94C7BCE` |

Shared six corpora and MyNES source/tests/0043 artifacts have no diff. No raw
ROM, media master or generated byte source is committed. Existing build trees
and ignored logs are retained for the immediately next pre-relocation receiver's
incremental regression comparison, not as distributable artifacts.

Executor P1 self-review confirms the admitted runtime boundary and original
behavior. Coordinator actual review of pushed `7997202a6` accepts those source,
test and artifact changes, but rejects closure for one static-coverage gap:
the new gate inspected only the original board source. The consolidated P2
brief requires the existing whole-production scan to enforce the private-ROM/
firmware owner boundary, including the operation guard. Executor confirms it.
No runtime input changes; P1 artifacts/boots remain the acceptance baseline.
Corrective proof passes: the existing whole-production scan rejects each of
five forbidden accesses injected into a different board source, while the
unchanged positive fixture passes. The generated copy is removed after proof.
Marker: `S74 P2 entire-production firmware boundary proof: OK`.
Corrective complete units pass x64 470/470 in 64.67 seconds and x86 470/470
in 69.37 seconds. Both complete specialized gates return zero, retaining
402 strict-matrix rows, 377 strict and 25 declared deferred owners.
No executable input changed: all eight deployed hashes still match P1, so no
product rebuild or repeated boot is required. Actual P2 review and governance
closure remain pending.
T540 still requires attachment ownership, test classification and physical
Core/IBM-PC source relocation. These proofs do not replace those exits.
