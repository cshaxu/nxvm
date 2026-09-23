# T514 S4: Win32 RDP Frontend Closure

The owner exercised the rebuilt `nxvm_0_5_0514.exe` through Remote Desktop and
reported that the soft keyboard now works.  This is the required real frontend
observation; it is not substituted by a BIOS, text, or KBC shortcut.

The retained route is one Core-platform Win32 normalizer, thin Console/Window
packet adapters, one VM input-sink binding and the existing Core-owned KBC
route.  The VM F9 product command remains the sole distinct policy.  No guest
text injection, profile route, second queue/state owner, source import or
`build/output` YAML mutation was introduced.

T514 task gates passed on 2026-09-01:

| Gate | Result |
| --- | --- |
| Focused packet matrix | Core normalizer, VM ingress and scan tests pass. |
| Complete repository-only unit | 317/317 at `ctest -L unit -j 4`. |
| Owner-managed integration | 40/40 at `ctest -L integration -j 8`. |
| Documentation governance | Passes for `vm-0-5-0514`. |
| Developer artifact | Stripped Release `nxvm_0_5_0514.exe`, SHA-256 `DB7971DD1B0218C8D437A6EA77D0A9C701EC53BEE732E80EEE6890AB7687279A`. |

Relative to `f07edd0b`, T514 changes eight tracked source/test files by
350 added and 9 removed lines (`git diff --numstat f07edd0b..HEAD -- src test`).
The positive net is the bounded reusable normalizer and its owner-local tests;
it replaces the duplicated VM event construction and Window-owned duplicate
state rather than adding a parallel keyboard route.
