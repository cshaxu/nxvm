# T561 S6 Address Decode And Construction Boundary

## Ownership And Minimal Contract

Baseline is accepted S5 `b23431527`; this consumes I3 P7/P8, not the physical
Machine move or complete T561 acceptance. Architecture governance assigns PC
A20 policy to Board before coding governance repairs the actual receivers.

The two existing policies remain the same values (0/1) at Board-Base. Board
preparation alone translates them into a copied inclusive first/last window
and reset_clear_mask. Global A20 uses 0--UINT32_MAX; the other policy uses
0x100000--0x1fffff; both suppress bit20 at reset before the existing KBC reset
output is applied. The 5170's enabled reset output remains its real KBC wiring,
not an executor special case. DeskPro's one configuration constant follows its
Board owner without a value change.

Neutral executor construction consumes that declarative value and validates
first <= last before allocating/publishing. Its zero configuration is identity
decoding, not an implicit PC gate. One existing live flag becomes a clear-mask
value; immutable range/reset data replaces the old policy enum. No additional
live flag, manager, map registry, allocation, address callback or compatibility
API is added. Inclusive 32-bit endpoints represent the full physical space on
both host widths without overflowing a 32-bit length.

The existing paused setter, executor-thread signal and copied query become
clear-mask operations, retaining their prior guards and status propagation.
Board owns the Boolean A20 interpretation. Its KBC and port callbacks now
receive their real Board context and translate the pin into the neutral mask.
The Board's copied Boolean query serves the actual port and boot diagnostic;
it does not expose the Core layout or store another gate flag. No unused PC
setter facade is added: paused tests use the neutral setter's explicit mask.

## Existing Route Evaluation And Preserved Ordering

The prior mechanism already has one routed access path, raw reset overlays,
selected RAM aliases, replacement/ordinary/overlay/fallback providers and cache
observation. Expanding four fixed backing aliases to express every global A20
mirror would duplicate decode rules and introduce arbitrary capacity changes.
A callback on every address would likewise add indirection without an owner
benefit. A direct range/mask operation replaces the existing direct A20 helper
at precisely its existing call sites instead.

Raw reset overlays still resolve first at the CPU's raw physical address.
Selected RAM aliases retain their current priority; ordinary routes consume the
decoded address. UNSUPPORTED still declines/falls through and a real failure
still terminates that access. Cache lookup/fill probes, real-mode accesses,
inspection and write snooping use the same route and decoder, not a second
memory implementation. Private pre_a20 spelling becomes raw_decode without
changing provider flags or ordering. Core reset restores the copied reset mask
before Board reset applies its actual hardware outputs.

## P8 Construction Audit

The existing firmware binding, immutable ROM provider and entry-plan mechanisms
remain neutral and are retained. Firmware binding owns its mapping boundary,
configure failure rollback and borrowed provider/context; the existing active
operation guard excludes recursive run and inappropriate state changes. Reset
and after-run failures propagate through that owner. ROM bytes, image/alias
choices and PC initialization remain Board/App responsibilities.

Entry-plan preparation validates routes/ranges/overlap before CPU entry
publication, cancels an invalid prepared entry and reports real write failures;
it does not invent rollback of accepted device effects. This remains an explicit
construction capability, not a BIOS shortcut. Firmware/entry source files and
CPU/FPU implementation are unchanged. Existing firmware-capability, entry-plan,
reset-ROM, route-transaction and failure tests retain their original assertions.
No host firmware interception or second reset path is restored.

## Test And Similar-Issue Sweep

The existing neutral configuration test adds a real 2 MiB identity instance
and a small instance suppressing bit4 only inside 0x100--0x1ff. It proves
inside/outside addresses, live/paused changes, reset restoration and invalid
range rejection without publication. This is a memory independence check, not
the complete non-PC execution proof reserved for S7.

The existing Board qualification test covers both actual PC policies: their
translated ranges, different 3 MiB behavior, shared 1 MiB wrap, port92 enable,
Board query, reset and invalid-policy rejection. Existing DeskPro D4 map,
compatibility and A20-reset tests and the 5170 composition test retain their
real hardware assertions. Cache inspection supplies its previously implicit
bit20 decode explicitly. Direct RAM tests likewise declare their tested decode.

Every old setter/signal call supplies the equivalent explicit mask (enabled
maps to 0, disabled to bit20). Boolean query consumers use either the real
Board query or the neutral mask. The test boot snapshot captures that mask;
the two App diagnostic outputs retain the same Boolean A20 text. Source/test
searches find old policy/API/private names only in negative-check vocabulary.
CPU's actual architectural A20M vocabulary remains untouched.

The A20/fallback checker retains KBC/private-memory rejection and provider
ownership, requires the new real Board/neutral wiring, and rejects PC policy
in the neutral decoder. The ROM checker follows only the raw-overlay name.
Neither gate is disabled. Source/test structures remain with their current
owners until the admitted extraction migrates each whole owner and fixture.

The actual sweep reads the changed production and test bodies, then searches
`src/core`, all four PC App roots and their matching test roots for
`core_machine_(set_a20|signal_a20|observe_a20)`, `flagA20`, `pre_a20` and
`CORE_MACHINE_A20_WRAP_`; no obsolete receiver remains. The family checker's
negative vocabulary is intentionally retained outside those implementations.
Firmware/entry files are compared byte-for-byte with S5. The preserved-route
comparison normalizes only the declared helper/member/comment substitutions;
it does not normalize algorithms, branches or return values.

The scope-preservation diff against `b23431527` is empty for Lib, Emulator,
all five imported source/test pairs, X86 Core CPU/FPU, MyNES, root README and
all tracked INI files. Existing higher-level white-box fixtures remain an
explicit S7/S8 migration obligation, not a new permanent cross-package
contract. In particular, a directory move cannot authorize Board tests to
borrow the new neutral Machine's private layout or CPU-private headers.

## Actual Change Accounting

Count tracked C/H and changed family CMake files with
`git diff --numstat b23431527`, excluding manifests, documentation and artifacts.
No file relocation occurs in this prerequisite.

| Surface | Files | Added | Deleted | Net |
| --- | ---: | ---: | ---: | ---: |
| Production C/H | 12 | 110 | 89 | +21 |
| Test C/H | 19 | 174 | 43 | +131 |
| Family CMake | 3 | 19 | 4 | +15 |

Production growth expresses the explicit range/reset data and real Board pin
conversion instead of a hidden PC enum. There is no new live state owner:
the former gate flag is replaced by its neutral mask, with copied immutable
range/reset data. One Board Boolean query replaces its former Core-specific
interpretation for real consumers. The private policy setter is retired.
New tests cover the two decoder owners; existing suites are not duplicated.

[Frozen file review](t561-s6-file-review.tsv) accounts for all 36 changed
source/test/build/manifest paths. Independent comparisons against S5 prove
the actual route-resolution/read/write/cache bodies, alias installation,
DMA transaction, ROM provider and firmware/entry source unchanged after the
declared helper/flag/comment substitutions. This supplements actual diff
review and direct tests; it is not a replacement for receiving qualification.

## Frozen Delivery Verification

X64's initial focused group passes 13/13 in 7.94 seconds: configuration, RAM,
device registration, inspection/cache, entry plan, reset alias, instance,
checked memory, ROM transaction, firmware capability and three DeskPro D4
cases. The added two-policy Board test and 5170 composition pass 2/2 in 1.32
seconds. Logs use `build/t561-s6-x64-focus-*.log` and `*-policy-*.log`.
Core manifests/corpus and the affected direct ownership checks pass.

The final suite includes the later inclusive-window-endpoint and rejected
Board-policy atomicity assertions. Both original Ninja/ccache graphs retain
their existing budgets, build -j8/test -j4 and persistent cache. X64 passes
627/627 complete repository units in 256.74 seconds; x86 passes 627/627 in
69.33 seconds. Both complete specialized gate targets pass, including the
513-row direct-compilation matrix (510 strict, three retained exceptions).
Exception-free compilation is not claimed.

Each width passes 48 receiving integration executions:

| Selected profile | X64 passed / seconds | X86 passed / seconds |
| --- | --- | --- |
| Default | 35/35 / 55.89 | 35/35 / 69.58 |
| XT | 3/3 / 31.16 | 3/3 / 35.83 |
| AT | 5/5 / 47.10 | 5/5 / 50.63 |
| DeskPro | 5/5 / 141.15 | 5/5 / 178.89 |

The default selection includes the MyNES receiving integrations. Both graphs
are restored to default and pass the additional real INI identity test
(x64 0.55 seconds, x86 0.44 seconds). Every deployed PC EXE passes the actual
published CLI/INI checks. The qualification driver's terminal exit is zero
with `S6-QUALIFICATION:PASS`; no observation timeout is treated as a result or
used to restart that process. Logs are `build/t561-s6-<width>-<profile>-integration.log`,
`build/t561-s6-<width>-final-*.log` and each published CLI log. The final artifact
inventory is `build/t561-s6-final-artifacts.json`, with retained hashes below.

Each width also passes three automated desktop-labelled unit cases and the
default suite's two automated desktop-labelled integrations. These are not
manual RDP, physical display/timing or full interactive Windows GUI acceptance.
PC110 and new CPU qualification are not claimed. Lib/Emulator/imported packages,
CPU/FPU, MyNES and owner INI/ROM/media inputs remain unchanged. This delivers
P7/P8 for coordinator actual-change acceptance, not I3 extraction, I4/I5 or
complete T561 acceptance.

### Published Artifact Identity

All eight are optimized, stripped Release EXEs, with embedded 0.5.0561,
verified PE width and exactly one dual-width pair per existing App asset root.

| Path | PE machine | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `assets/nxvm/nxvm_0_5_0561_x64.exe` | 0x8664 | 1511941 | `c8124219f6c44c58dab90f73cf180aacf9bf9e2fa234e4dccf52c08b163c8991` |
| `assets/nxvm/nxvm_0_5_0561_x86.exe` | 0x014C | 1658052 | `559de2c2d1953598b3cd7d4363ac2ae0d88c02328e005c35676e792dcfb6a080` |
| `assets/my5160/my5160_0_5_0561_x64.exe` | 0x8664 | 1500407 | `be756182fb410a814c386526f6bf68c425c7badc561aaf5e0d649747a2d29632` |
| `assets/my5160/my5160_0_5_0561_x86.exe` | 0x014C | 1645499 | `e0bf594338840db6bf32cbd0bee511f8d8ca6eba1fb87b7b872455fcb6f32128` |
| `assets/my5170/my5170_0_5_0561_x64.exe` | 0x8664 | 1512771 | `6543bc97acacba6b6f55b708e45a17d3198e2687a57da108021453e7d3f324a5` |
| `assets/my5170/my5170_0_5_0561_x86.exe` | 0x014C | 1658754 | `bf3ae51a7c527f772620f3480f34a5403fbd736b6968ef46d7301aaa4fd2bb54` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x64.exe` | 0x8664 | 1500011 | `298185dd452e144025fab9d7644e4083fef7224eb7ab5d810215cfee00900602` |
| `assets/mydeskpro386/mydeskpro386_0_5_0561_x86.exe` | 0x014C | 1646001 | `893d367da24b956ce10649b863cd643c092851b65e449efa1dbf30d6e1f52ad7` |
