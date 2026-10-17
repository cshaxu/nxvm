# T1 S2: Repeatable Inventory And Receiving Contracts

## Delivered Boundary

Baseline: accepted S1 `c34b5b2ad`. This batch changes MyRaidenII tools,
owned tests and documentation only. Shared production/tests, root build
configuration, existing App inputs, INIs and released executables are unchanged.
No MyRaidenII runtime or playable EXE exists yet.

The read-only inventory verifier checks the reviewed archive fingerprint,
member identities, lengths, SHA-1 and SHA-256 using one retained input handle.
It rejects duplicate/case-colliding names, paths, malformed inventory and
oversized or expanding members. It does not extract or acquire ROMs. Exact ZIP
identity is an archival audit contract, not the future runtime loading API.

New tool/test code is 313 lines: verifier 135, synthetic unit 167 and standalone
CMake registration 11. The estimate was 300--400 added lines; deleted tool/test
code is zero. README and research/governance changes are separate prose.

## Verification

- Existing x64 repo-only unit suite: 655/655 pass, 64.34 seconds.
- Existing x86 repo-only unit suite: 655/655 pass, 66.03 seconds.
- New owned CTest unit: 1/1 pass, containing 18 independent synthetic cases.
- The same 18 cases pass in native x86 Windows PowerShell.
- Actual owner archive verifies in both PowerShell host widths: 13 runtime
  images and one extra non-runtime JED, with the S1 container fingerprint.
- Synthetic malformed central-directory metadata proves the stream length
  bound, not merely the declared ZIP entry length check.
- Failure cases also prove missing images, wrong lengths/content/hash,
  duplicates, case mismatch, traversal and inventory/container limits.

Existing compiled C/CMake inputs did not change; their current build graphs
were reused. No executable rebuild is necessary for this offline tool batch.
The ten published executable hashes match the accepted artifact baseline.
No desktop, RDP, physical audio, real-game execution or new runtime integration
was performed. Unit success does not qualify unimplemented game behavior.

During development, explicit PS5 compression-assembly loading and the test's
local hash helper resolved host/module differences. Final tests require no
system configuration change or downloaded module.

## Sources And Design Review

The original OKI 1994 databook resolves F06 source acquisition. Its MSM6295
section was inspected, including rendered command/timing pages; its hash and
page range are recorded in the research index. Source inspection is not a
chip-accuracy claim. The publisher flyer supplies no exhaustive stage list.

The integration freeze records the existing neutral Core Machine as sole
execution/time owner, native and 8080 V30 obligations, public bus/IRQ routes,
source-graded sound/video boundaries and G01--G12 semantic proof receivers.
The owner approved only the specified V30/NEC/timing enum expansion and CPU
integration. Additional public fields/functions require separate review.
No such API or Shared source was changed in S2.

The full-colour Lib contract remains owner-declined High TODO. No palette
quantization, second presenter, alternate CPU or protection bypass is added.
Exact stage-ID/input/signature coverage remains a prerequisite of the named
gameplay qualification receiver; it is not invented from the operator manual.

## Actual-Change Review

Review covers the complete verifier and synthetic test, CMake registration,
tool/test usage, source hashes, receiving contracts, ledger and active packet.
One archive input and one metadata authority remain; no runtime compatibility
loader or new framework is introduced. Generated fixtures clean up their own
fresh directory, and all production input handles are released on failure.
The similar-issue sweep distinguishes file size from mapped region size and
PAL/JED metadata from runtime payloads. T1 freeze and M1 runtime qualification
remain separate claims; S2 does not declare the persistent product goal done.
