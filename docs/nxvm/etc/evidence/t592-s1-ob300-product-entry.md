# T592 S1 — MyOB300 Product entry

The `myob300` target links the existing MyOB300 Machine and X86-NXVM Product.
It selects the Am386SXLV without an FPU, uses the existing PC INI parser and
starts with Turbo selected. The existing board binding requires 2 MiB. INI,
System Card and relative media resolution use the executable directory.

The supported System 1.0 ABA image is external. Machine preparation reads one
16 MiB image. The App validates its full SHA-256 on the owned bytes and releases
the prepared configuration on a mismatch. The digest routine is a private
adaptation of the repository's MyRaidenII ROM hash implementation. Neither
App links the other, and the Shared APIs are unchanged. The sample INI keeps
owner C media external and uses overlay access by default.

| Verification | x64 | x86 |
| --- | --- | --- |
| Cached Release candidate build | Pass | Pass |
| MyOB300 board unit test | 1/1 | 1/1 |
| Actual EXE rejects missing, short and same-size wrong-content cards from a foreign working directory | 3/3 | 3/3 |
| Real App entry and Machine preparation reach the presentation boundary with correct card, 2 MiB, Turbo and the relative read-only C image | Pass | Pass |

The preparation probe supplied a deliberately invalid INI in the working
directory and a valid INI with a 512-byte C fixture beside the executable. The
probe wrapped only the final `x86_ibmpc_product_run` presentation call, checked
the actual prepared Machine through its Product callbacks and destroyed it.
The negative cases ran the unwrapped candidates. The valid unwrapped candidate
opened the monitor in both builds. The hidden Console input probe did not
receive the expected `info` result before its helper timeout. Native command
delivery, desktop Window, and this Product's complete DOS startup remain for
the publication and qualification receiver. The prior standalone Machine DOS
evidence still applies to that earlier entry only.

| Build-only candidate | SHA-256 |
| --- | --- |
| x64 `myob300.exe` | `287a36a81e0510a5c2525e818383f8b0f0eeB2a7f1f5ae43c30538dc9a604c16` |
| x86 `myob300.exe` | `5202198acea69515daeeB182fa937acffbf436e9eaf5305443de60eecffbdb1` |

No EXEs were deployed. Test cards were extracted into transient storage and
removed without changing the owner's source archive or the external assets
directory.
