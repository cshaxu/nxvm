# M5 T517: Dual-Architecture Developer Artifacts

The [archived proposal](M5-T517-dual-architecture-developer-artifacts-proposal.md)
defines the owner-approved dual-output contract.

## Task Record

| Subtask | Scope | State |
| --- | --- | --- |
| S1 | Establish architecture-aware presets, one artifact route and PE validation. | Closed |
| S2 | Build and verify both stripped Release artifacts. | Closed |
| S3 | Run primary-host regression suites, x86 smoke and close the task. | Closed |

## Delivery

`vm-0-5-0517` remains the sole product target.  CMake derives its artifact
suffix from the configured compiler pointer width and rejects a preset whose
declared architecture disagrees.  It emits and verifies:

| Artifact | PE identity | SHA-256 |
| --- | --- | --- |
| `nxvm_0_5_0517_x64.exe` | PE32+ x64 | `7CF353BAB01EE59DF8C67081B6C9D8036CCCE926DC4A4A72ABC922BAEE4ECA07` |
| `nxvm_0_5_0517_x86.exe` | PE32 i386 | `C0170DC664D7E666091878AF38A81A96DC05D8B0BA0C984C3E2EDEC012ECAB70` |

Both are optimized, stripped Release builds.  The x86 artifact ran the native
repository-only `core-machine-bit-test-smoke` successfully.  Full regression
remains one primary x64 host run because host pointer width does not define a
separate guest-emulation path.

## Code Shape

Tracked build/governance code changed by +83/-10 lines (net +73), excluding
documentation and generated artifacts: the one 33-line PE verifier replaces
no product path, while the remaining changes select its input and remove the
unsuffixed artifact route. The retained owner is the existing single CMake
product target; compiler configuration is the only architecture variation.

## Closure Verification

- x64 repository-only unit suite: 302/302 passed in 16.71 seconds.
- x64 external-ROM/media integration suite: 44/44 passed; its log contains no
  failed or unrun entry.
- x64 and x86 PE identity verification passed; both Release policy checks
  passed.
