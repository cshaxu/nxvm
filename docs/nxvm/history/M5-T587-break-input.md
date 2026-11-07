# M5 T587 — Break input support

Status: S1 closed after owner acceptance on 2026-11-06; T587 remains open.
[Proposal](M5-T587-break-input-proposal.md) records the bounded import.

Source revision: `6bc8e71d7` (Shared P1).

## Actual change and boundary review

The source is NTVDM64 MIT commit
`70e829d880cf897a340af6791c1343b9238748af`. Three selected files compare exactly
with that commit after only `lib-nxvm/` include-prefix normalization. Production
adds four lines: the neutral Break key, VK_CANCEL Types alias, Base mapping and
one contract comment. No state, function, struct layout or existing enum value
changes. Existing Window resizing and newer Lib implementation remain intact.

Window supplies the physical scan and extended bit in its Base transition
record; Console Broker supplies the same facts in its combined record. Both
use the existing Base decoder and KVM submission path. X86-NXVM Machine passes
the complete KVM event to its existing keyboard mapper and board receiver.
Scan sets 1 and 2 already consume E0/46 correctly, so no PC production change
is needed. Pause remains a separate identity and sequence.

Existing tests add 21 lines: one Lib table row exercises make/release,
physical scan, extended identity and repeat normalization; twenty PC test lines
exercise Break E0 46 / E0 C6 in set 1 and E0 7E / E0 F0 7E in set 2. No new
fixture, test target, framework or duplicate production decoder is introduced.
The three affected manifests were regenerated and independently hash checked.

## Verification

- Cached x64 and x86 complete default-profile builds passed, including all
  configured shared and App targets. PC110 product targets were also explicitly
  built (they are excluded from the default build).
- x86 full unit suite: 715/715, 137.23 seconds.
- x64 full unit suite: 715/715, 518.99 seconds.
- x64 registered integration suite: 48/48, 212.96 seconds.
- x86 registered integration suite: 48/48, 247.95 seconds.
- Both integration suites include the native MIDI endpoint check, MyNES native
  Console/Window fixtures, default DOS/Windows checks and PC110 real firmware
  cold/resume boot. This run needed no native-MIDI exception.
- Selected-profile integration also passed on both widths: My5160 3/3,
  My5170 5/5, DeskPro386 6/6. x64 elapsed times were 27.36, 39.58,
  116.66 seconds; x86 times were 31.63, 52.00, 166.02 seconds.
- Both reusable build trees were restored to the default profile and rebuilt.
- All three product documentation governance checks, LF and whitespace checks
  passed. Manifest, Types and dependency gates passed within full units.
- Coordinator reviewed the actual eight-file Shared diff after execution:
  source +4/-0, tests +21/-0, manifests +5/-5; total +30/-5, net +25.
  No outward dependency, extra execution path or platform vocabulary leak was
  introduced. All tracked non-EXE receiver assets remain unchanged.

These are automated native fixtures and synthetic input records, not a claim
of physical Break-key or RDP keyboard acceptance. Zero-scan host records and
scan-set-3 Break semantics are not newly qualified by the added test. MyRaidenII
external game-play qualification is not a registered integration test and is
not claimed by the suite results. User INI/media and sibling repositories remain
unchanged. Receiving Apps retain their current version under the Shared-change
artifact rule.

## Release inventory

All fourteen receiving assets were rebuilt from this S1 source and retain
their current product versions. PE machine widths were checked independently.
MyRaidenII uses the cached linked binary with debug symbols stripped for release.
The PC deploy hooks and current INI-identity integrations verify receiving paths.

| Artifact | SHA-256 |
| --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | `342dcda32da9941ab66af08118e1b9267f399d4bfdc81e3e7ce0c19ecae40732` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | `fb5f83a07ee031b0e7fc549ee56cd83735ba0010ef4ac5ff543f9df68be3b9c2` |
| assets/my5160/my5160_0_5_0582_x64.exe | `2067e5f96a9a83c91fdaa4341e6a7eac8d776180c4af08a33dfd8db3908436b7` |
| assets/my5160/my5160_0_5_0582_x86.exe | `94f24e38d425ca44edff0be0eeeb146e3de87274afe3ce6fbdc42204f7091257` |
| assets/my5170/my5170_0_5_0582_x64.exe | `94966ae60725884493ea0178c6270315cee459039903ab2ef7dcbc5d9ff23083` |
| assets/my5170/my5170_0_5_0582_x86.exe | `dae8e4c1409c1ae592fb644c266cca5e2da09478a48b2fb702090afa158bad5d` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | `21bf6b91e59d33401981a1afaf64b6032122df205ccd81ce8b3e125fc3222ca4` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | `addc8866230305a60f82361fabe174c0d68afbfb7e4f76d272d8231823fa245c` |
| assets/mypc110/mypc110_x64.exe | `c1fa066cc9e775ab80073e533a35c03ae3ae8a12da94dabdc5b425ad09a277cc` |
| assets/mypc110/mypc110_x86.exe | `42757f431d5f867aa1ef4753ca45511b00ff342d7d7a70d69c75bb814648f4eb` |
| assets/mynes/mynes_0_0_0044_x64.exe | `ef41bb7ca2a57e03957c99c64000b5fb2cd7454ff2fbc3eb777b40fb5d04d136` |
| assets/mynes/mynes_0_0_0044_x86.exe | `bd76f447efd072ca3879c74e4b4dd833fe51ee45b9ff3c416055c7890178bf3b` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | `27c0ce9704282e217ea9cbbb1173b4e1545fd53c365dcbece136cfedab02cc8f` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | `d3942c0a063ec6a5c368c48002f8d08e6a050432927f99b63b7bc76e1f197a95` |

## Owner acceptance and closure

The owner approved S1 closure and commit/push after delivery `7794e25f9`.
Closure review matched all fourteen deployed hashes to this record and checked
the delivery commits and scope. The prior automated qualification remains
valid; this documentation-only closure does not rebuild unchanged executables
or repeat runtime tests. Physical/RDP limitations above remain unchanged.
Documentation governance, LF and diff checks validate the closure update.
T587 stays open with no active S.

## Archived S1 admission packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New: M5 T587 S1, following owner-closed T586. |
| Admission And Approval | Owner requested minimal Break import, PC wiring tests, compilation and push on 2026-11-06. |
| Objective | Preserve Break identity through the existing neutral input and PC keyboard paths. |
| Non-goals | No wholesale Lib import, Window changes, new input path or product-version advance. |
| Reference Baseline | NXVM b1fd613e1; NTVDM64 70e829d880cf897a340af6791c1343b9238748af. |
| Candidate Proposal | [Break import](M5-T587-break-input-proposal.md). |
| Files And ABI Surface | Three Lib source headers/implementation, existing Lib and X86-NXVM tests and manifests. Add a key constant, no layout or function change. All seven Apps receive rebuilt artifacts. |
| Applicable Rules | NXVM guide, Execution, Documentation, Architecture, Coding, NXVM layout/design and source policy. |
| Verification | Existing x86/x64 cached builds; full unit and integration suites; profile-specific integration; manifest, dependency, LF and documentation gates; 14 PE artifacts and hashes. |
| Expected Markers | HOST-KEY-TO-NATIVE-SCAN-SETS:OK and native decoder exit zero; exact suite outcomes recorded. |
| Asset Needs | Retain current receiver versions, INI and media. Publish seven dual-width pairs. Use existing .ninja and .ccache only. |
| Reporting Requirements | Production/test line counts, source provenance, exact results, artifact hashes and commits; disclose unavailable physical/native tests. |
| Stop Conditions | No widening production changes without a demonstrated defect; never count unavailable desktop hardware as passed. |
| Exit Criteria | Minimal import reviewed, tests and releases verified, evidence complete, commits pushed; await owner acceptance. |
| Original Owner Request | Import NTVDM64 Break with minimal changes, verify PC wiring and necessary tests, build/test/commit/push. |
| Similar-Issue Sweep | Pause versus Break; make/break and extended identity through shared decoder and PC scan sets; retain newer Window behavior. |
