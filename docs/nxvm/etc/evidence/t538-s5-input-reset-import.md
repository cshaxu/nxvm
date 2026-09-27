# T538 S5: Reviewed Input Reset Import

## Admission And Source

Owner admitted continuation S5 on 2026-09-27; S4 was already accepted and
closed. Baseline is 52e5da766. Source is clean SoftPC
40da7d0059c97e7f3a5026d16c128417e48e74b7, compared with b79769c1.
The following twenty units were initially imported at the same relative paths;
the approved test-only correction below supersedes the initial equality claim:

- `src/common/MANIFEST.sha256`
- `src/common/session/control.c`
- `src/lib/MANIFEST.sha256`
- `src/lib/console-broker/console.c`
- `src/lib/console-broker/win32/console.c`
- `src/lib/kvm-base/component.c`
- `src/lib/kvm-base/event_interface.h`
- `src/lib/kvm-base/worker_interface.h`
- `src/lib/kvm-console/console.c`
- `src/lib/kvm-window/win32/component.c`
- `src/lib/types/win32/console.h`
- `test/common/CMakeLists.txt`
- `test/common/MANIFEST.sha256`
- `test/common/physical_key_identity_smoke.c`
- `test/common/window_input_reset_smoke.c`
- `test/lib/MANIFEST.sha256`
- `test/lib/console_broker_smoke.c`
- `test/lib/kvm_console_retirement_barrier_smoke.c`
- `test/lib/kvm_input_admission_smoke.c`
- `test/lib/kvm_window_capture_contract_smoke.c`

These are the existing owner-authorized neutral shared corpus under the root
MIT grant, not recovered SoftPC/MVDM source. The diff introduces no independent
copyright/license notice, firmware, media or sibling runtime dependency.
Sibling source and build files are read-only; the pin, not later upstream work,
defines the import.

## Architecture And Code Review

Lib records a source losing balanced physical input. Its KVM owner discards
withheld hotkey prefixes and emits INPUT_RESET without retiring the source.
Common Session consumes that fact and releases only keys in its source ledger.
The driver receives ordinary key breaks, not a new native or reset API. There
is no extra queue, pressed-key authority or App-specific shared branch.

Window loss/deactivation/freeze and Console focus/handoff/destruction use this
contract. Native readers are quiescent before broker cutover reset. A rejected
Window reset marks the component failed and stops admission; it is not silently
treated as a successful delivery. Console emits a zero-button mouse record
before reset; the matcher forwards mouse without replaying withheld keys.
Committed frames, frame extent and Console viewport behavior are unchanged.

The public event enum gains INPUT_RESET and the KVM-leaf worker contract gains
reset_input. These are in-process values, not serialized ABI; all receivers
must rebuild together. No product handles the numeric enum directly.
NXVM already consumes Common-generated breaks through its one keyboard mapper.
MyNES already consumes them through its source bindings and separately clears
guest buttons at paused/stopped lifecycle boundaries. No adapter change is
required. This import does not claim a new paused guest-input contract.

Review includes all changed production lines and tests. New Common regression
uses a substituted Window context to exercise native loss notifications through
the production Session ledger without a real focus race. However it includes
Lib's private kvm-window/window.h and win32/component.c from test/common. This
cross-owner white-box dependency conflicts with the declared public-boundary
and same-owner test discipline. Existing static gates do not detect this test
dependency. The owner subsequently approved fixing this boundary here, with
SoftPC to import the correction. No production divergence is authorized. Existing tests cover
freeze, rejected reset, repeated reset, paused ledger cleanup, source isolation,
Console handoff and mouse neutralization. No external ROM or physical audio
endpoint is introduced into unit inputs.

Initial imported C/header changes: production +92/-11; tests +248/-22,
including the subsequently removed 127-line test. These are historical import
counts; final reviewed counts are recorded with verification below.
Manifests, documentation and EXEs are excluded. Most growth is regression proof;
production adds a source-loss fact to existing owners rather than a parallel
input mechanism. The broker retains two explicit fail-closed cleanup branches
for deactivation and reset failure; no new resource owner is created.

## Finite Similar-Issue Batch

Queries: `rg -n 'INPUT_RESET|SOURCE_RETIRED|reset_input|release_source'
src/lib src/common src/app-nxvm src/app-mynes test/lib test/common` and
`rg -n 'KVM_EVENT_|input_reset_requested' src/app-nxvm/machine src/app-mynes/core`.

- Window focus loss, app deactivation and freeze: fixed by the imported reset
  producer; duplicate facts are harmless, rejected delivery is terminal.
- Raw Console focus loss and broker replacement/destruction: fixed by the
  imported producer; old reader is quiesced before binding replacement.
- Common running ledger: source-specific breaks; paused/stopped ledger:
  cleanup only, preserving the existing no-guest-input contract.
- Source retirement: distinct permanent fact retains its existing path.
- Both product drivers: unchanged ordinary key-break adapters; no native
  input source owns guest state. MyNES lifecycle reset remains product-owned.
- Linux Window is not an admitted runnable backend here; no runtime Linux
  proof is claimed. Existing explicit backend capability remains unchanged.

This resolves the admitted import batch, not T538's repeated deployed-boot
matrix. Existing cooked-history rollback debt remains separately recorded.

## Equality And Verification

Initial path and byte comparison passed for 228 files: src/lib 109, src/common 23,
src/x86 14, test/lib 51, test/common 21, test/x86 10. All six manifests pass;
test/register.cmake also matches. The component DAG check passes. The x86
pair is unchanged. The approved test correction supersedes initial test-root equality.
Final verification below supersedes the interrupted run; closure follows the
separate scoped deliveries and coordinator actual-change review.

## Review Hold And Approved Resolution

The initial review required owner disposition before acceptance. The import
and documentation were retained uncommitted. The full verification and receiver-build
scripts were stopped at this review boundary after identifying their process
trees. NXVM x64 build completed and its unit run was partial, with no complete
suite claim; some NXVM receiving EXEs were rebuilt but are not accepted release
evidence. Retain those task-owned changes for continuation and rebuild/verify
all receivers after the decision. INI hashes still match S4; MyNES configuration
and snapshot are unchanged. No sibling file was modified.

The owner's subsequent instruction is to repair here for SoftPC to import.
The 127-line cross-owner fixture and its registration are removed. Existing
Lib input-admission coverage now checks native focus loss/deactivation, repeated
reset, source identity and withheld-prefix discard. Existing Common ledger
coverage checks source-local breaks, repeated reset, paused cleanup and fresh
input after reset. Its negative verifier now rejects private Lib includes in
Common test fixtures. No test-only API or replacement production path is added.
The two updated test manifests enumerate the correction; all production files
and test/x86 remain identical to the source pin. Full verification is restarted
after the correction, rather than crediting the interrupted run.

The first relocated fixture run incorrectly left ordinary A pressed before
expecting Ctrl to be withheld as a hotkey prefix. The corrected stimulus sends
the balanced A make/break before Ctrl; no matcher behavior was changed.
Full desktop verification then exposed an upstream retirement-fixture omission:
scenario 17 freezes the Window and legitimately emits INPUT_RESET, but the sink
assumed every other event was SOURCE_RETIRED. The fixture now counts reset
separately, validates source identity and requires a reset in that scenario.
All original one-retirement/failure/lifetime assertions remain. The corrected
desktop regression passes; final complete suites are rerun after both changes.

The event-sink sweep covers `test/lib` and `test/common` SOURCE_RETIRED/type
assertions. Console shutdown and retirement-barrier fixtures explicitly inject
only their declared lifecycle/key inputs, without a native focus or broker
cutover producer; their narrow sink assertions remain valid. Modal Window
already accepts reset without counting it as retirement. Capture and admission
fixtures handle reset explicitly. Common observes only normalized public values.

## SoftPC Return Delta

Relative to 40da7d00 (also unchanged in clean 19e853c0), production roots and
test/x86 are identical. The approved return delta contains eight test paths:

- test/common/CMakeLists.txt: remove the cross-owner fixture registration.
- test/common/window_input_reset_smoke.c: delete the cross-owner fixture.
- test/common/physical_key_identity_smoke.c: prove fresh input after reset.
- test/common/verify_negative.cmake: reject private Lib includes in Common tests.
- test/common/MANIFEST.sha256: reflect the test changes and deleted fixture.
- test/lib/kvm_input_admission_smoke.c: native loss, duplicates and source identity.
- test/lib/kvm_window_retirement_smoke.c: admit reset independently of retirement.
- test/lib/MANIFEST.sha256: reflect the two local test updates.

This is a portable test correction, not a product-specific shared fork. SoftPC
must import it before six-root equality can be claimed again; no sibling writes
are performed here.
Synchronize the two complete test roots, including removals, from the delivered
NXVM revision. The removed Common fixture never existed in NXVM's committed
baseline, so cherry-picking its import commit alone would not delete that file
from SoftPC.

## Size And Ownership Review

Against the NXVM S4 baseline, `git diff --numstat` over tracked C, headers and
CMake scripts counts 16 paths, +274/-33, net +241. Production C/headers remain
the imported +92/-11; remaining growth is regression proof and the boundary
check, not another runtime path. Manifests, documents and binaries are excluded.
Against the imported SoftPC tests, the local repair is +62/-129, net -67 over
six code/build paths, excluding the two manifests. It removes the duplicate
127-line cross-owner harness while preserving the behavior at each owner's
contract. Lib owns source reset/matcher state; Common owns delivered keys.

## Receiver Artifacts

All eight NXVM optimized stripped Release targets are built from the S4 product
source plus the production import pinned above. PE inspection verifies x64/x86
and no debug sections. The product identity remains 0.5.0538. Existing owner
INIs are unchanged. Product-target builds, not test executables, supply:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_at_0_5_0538_x64.exe | 9A02CF7269AA43EE8FD8206975957E1E126E55BD0BA0C6D92DCD934897AE4D02 |
| nxvm_at_0_5_0538_x86.exe | 1CEF176539AAB401E541C5096E9583836A45AE2C5C5E3641B99AD7F994A387E1 |
| nxvm_xt_0_5_0538_x64.exe | E1DE1BBCE67B1B9D3C1CCEBC52BF47C784D759F6D6BFD395208C10047620246C |
| nxvm_xt_0_5_0538_x86.exe | 38A0C20AEA13DB14F016A59D2692B5FBC0BAA621924257D569C1141EDBFB8DD9 |
| nxvm_default_0_5_0538_x64.exe | 3A3C702E161E9E5984F320624CDAF18836049B2162C2D530915DC3E3E97E6602 |
| nxvm_default_0_5_0538_x86.exe | 1B1F32D8BEB93390FAF39F2697EA34759796F27309C4AC52A6F7F73190D4C78D |
| nxvm_model40_0_5_0538_x64.exe | 026BF9AFC13E768D9E3AA24BAA8CA2D27D0D8199C53C02977E8833A94F7C5006 |
| nxvm_model40_0_5_0538_x86.exe | 4F87262BAB23CADC5EF1ECAF8EEA4D4AEFF65F41CDBCF845062DFAFA4D60BB5E |

Both MyNES receivers remain 0043, with their product-local source and INI/snapshot
unchanged. [Receiver evidence](../../../mynes/etc/evidence/t538-s5-input-reset-receiver.md)
records hashes and verification. Ten EXEs remain the complete deployment set.

## Final Verification

- NXVM x64 and x86: 335/335 full unit cases each, including three serial
  desktop tests, plus 21/21 static checks each. The non-desktop invocation
  contains 353 cases (332 units plus 21 checks), followed by three desktop cases.
- NXVM x64 external integration: 20/20, with unchanged configured inputs.
- MyNES x64 and x86: 132/132 each, including five serial desktop cases.
- All six manifests, Types/boundary/DAG checks, both documentation gates and
  `git diff --check` pass. The final corpus has 227 files; the only upstream
  differences are the eight approved test paths listed above.
- Eight NXVM 0538 and two MyNES 0043 Release targets build successfully;
  PE architecture and absent debug sections are verified for all ten.
  NXVM binaries contain the 0.5.0538 identity; MyNES target/output identity is
  mynes-0-0-0043 in its unchanged product CMake definition.
- Owner INI byte hashes and MyNES snapshot hash still match S4. No product
  source, test, media or configuration was changed. No sibling file was written.

Commands: build `t41-s8-nxvm-{x64,x86}`, then CTest excluding desktop/integration
and a separate serial desktop pass; x64 CTest integration pass. Build each
`t535-s4-{at,xt,default,model40}-release-{x64,x86}` target vm-0-5-0538.
Build `mynes-gcc-{x64,x86}-release`, then CTest excluding desktop and a separate
serial desktop pass. Build trees remain needed for the immediately following
T538 deployed-boot qualification. The temporary negative probe contains no
retained generated files.

Executor self-review inspected actual production and test diffs, all removed
fixture assertions against replacement coverage, manifest contents and receiver
inputs. No second input owner, test-only public API or product shared fork is
introduced. T538's three fresh launches for each deployed EXE/INI pair remain
pending; this S does not substitute unit/integration counts for that gate.

Shared implementation is 0c71110b0714fffee3ef8c40bb352ee3dd71ee40, pushed to
origin/master. It identifies the final source used by all ten receivers.
