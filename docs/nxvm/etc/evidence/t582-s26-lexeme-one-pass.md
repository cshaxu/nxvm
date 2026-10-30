# T582 S26 — lexical prefix scan

The existing lexical decoder first scans recognized prefixes, then previously
scanned the same bytes again on 32-bit profiles to detect `66` and `67` size
prefixes. It now records those flags in the existing first loop. The existing
prefix predicate accepts `66` and `67` as prefixes only for 32-bit profiles;
V30 and older Intel profiles retain their previous admission behavior. Repeated
size prefixes still set a Boolean rather than toggling it. No new state,
interface, decoder or guest timing path is introduced. Production code is
+4/-12 lines, net -8; instruction length is bounded, so no asymptotic
complexity improvement is claimed.

A task-only baseline/candidate differential ran 1,400,000 deterministic
random prefix-heavy byte-sequence/profile/code-size cases on each host width.
All four results produced digest `e04c4ce7b2535a98`. It is a sampled comparison
of public lexical scan results, not an exhaustive proof or a substitute for
the existing real-execution, decoder and preview tests.

At the 10,000,000-instruction private-media startup checkpoint, ordered
baseline/candidate/candidate/baseline times were 19,330/19,158/18,606/18,735
ms on x64 and 23,448/23,999/23,247/23,784 ms on x86. All runs reported
79,822,202 ticks and PC `00039119`. The differences are comparable with host
variation and show no reliable speedup or stable regression. This checkpoint
does not prove complete state equivalence, actual Game speed, or desktop/RDP
behavior.

Cached x64 and x86 builds and all 712 unit tests per architecture passed.
Default-profile integration passed 48 tests per architecture; the 5160,
5170 and DeskPro profile integrations passed 3, 5 and 6 tests respectively
per architecture. MyRaidenII selected-set startup and product-console probes
passed on both architectures. Fourteen released executable PE widths and
hashes were checked in `t582-s26-lexeme-one-pass-artifacts.json`; MyNES
does not depend on this change and its binaries were unchanged. The X86 Core
manifest, documentation governance, artifact roots and `git diff --check`
passed. No new desktop or RDP interaction was performed for this change.
T582 S26 stays open for the larger PC110 usability objective.
