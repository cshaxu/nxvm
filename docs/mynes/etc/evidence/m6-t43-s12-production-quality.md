# M6 T43 S12 MyNES Production Quality Repair

## Admission And Boundary

Owner admits the read-only audit findings on 2026-09-25, explicitly forbids
changes to the six Shared components, and approves retaining atomic file
replacement as MyNES TODO debt. Reference: 1ee811710. Only MyNES source, tests,
documents and the current 0043 x64/x86 artifacts change. No INI, media, NXVM
or Shared source/test/manifest is changed. T43 remains open.

## Complete Audit Batch

| Finding | Disposition and owning mechanism |
| --- | --- |
| Battery save errors swallowed | Core save returns successful no-work for absent/non-battery/clean RAM; clears dirty only after successful close. Composition checks save before replacement; failure retains old media/path, prints an error and rejects replacement. Joined-executor shutdown reports save failure with exit 1. Startup and interactive attachment now share the same path preparation. |
| Destructive file truncation | Owner-deferred P1 in TODO for both battery and snapshot destinations. No atomicity claim, native bypass, or Shared API modification. Existing load policy remains best-effort for absent/unreadable battery files; this S does not introduce a file-existence oracle. |
| PPU snapshot omission | v3 explicitly saves sprite_evaluation_byte. Old incomplete v2 images are rejected rather than synthesizing the missing state. A mid-evaluation restore into a machine with different history resumes to byte-identical state. |
| Candidate validation | PPU sprite counts are bounded by eight and paired counts agree; fine-X/evaluation-byte ranges are checked. The same-class sweep adds APU phase/counter, MMC1 shift/register, DMA phase/index and immutable cartridge-kind checks. All checks precede the existing candidate commit; failed loads preserve live state. |
| Implicit scalar layout | All grouped scalar spans in cartridge, APU, PPU, controller, CPU and trap state become explicit one-byte field operations. True byte arrays and existing endian helpers remain. A product-local static gate rejects the old scalar-span shape. No serializer framework or second codec. |
| Boolean vocabulary | Config boolean returns and copied frame flags use LIB_TRUE/LIB_FALSE or boolean expressions. Wire-format 0/1 bytes, counters, atomic integers and process exit codes remain numeric. INI grammar is unchanged. |

## Schema And Similar-Issue Sweep

MNS1 retains its little-endian header and section order. Version is now 3;
the PPU section gains exactly one byte immediately after sprite evaluation
index. All other explicit scalar expansions preserve byte order and length.
The generated NROM fixture is 136007 bytes. v1/v2 rejection is deliberate;
there is no compatibility path. APU delivery samples remain transient as before.
Cartridge identity and immutable RAM-kind facts must match the loaded ROM.

Reviewed every scalar span in `core/snapshot.c` against the corresponding
private layouts and production uses. Searches included
`rg -n 'read\(|write\(|SNAPSHOT_STEP' src/app-mynes/core/snapshot.c`,
`rg -n 'save_battery_ram|load_battery_ram|WRITER_TRUNCATE' src/app-mynes`, and
`rg -n 'return [01];|valid = [01]|graphics =' src/app-mynes`.
The complete span batch is fixed. Battery save callers are checked; snapshot
save already reports failure but shares the deferred truncation risk. Atomic
replacement cannot be implemented through medium_replace, which transfers an
in-memory lease only. No change is made to read-only Shared/NXVM owners.

Regression coverage includes invalid scalar matrices, malformed boolean/magic,
old version, truncation, cartridge mismatch, unchanged live state on rejection,
continued execution after restore, failed disk open retaining dirty RAM,
save-before-replace ordering, and actual product exit status on save failure.
Tests generate their own bytes; no external assets or physical audio observation.

## Verification And Delivery

Both existing Release trees build with strict production warnings. Verification
commands are `cmake --build build/mynes-gcc-<arch>-release --parallel 8` and
`ctest --test-dir build/mynes-gcc-<arch>-release --output-on-failure -j 4`.
Final complete configured suites pass x64 132/132 (120.29 seconds) and x86
132/132 (125.03 seconds), including all registered Shared tests/manifests and
repository-generated MyNES integration scenarios. No case is excluded.

The initial sandboxed Ninja invocation stalled without compiler output. Only
its identified MyNES CMake/Ninja processes were stopped; rebuilding outside
the sandbox succeeded. No sibling process was terminated. One newly added
test initially supplied an unsupported battery-backed NROM fixture; it failed
before reaching shutdown. The fixture now uses supported NROM and substitutes
the existing battery-save result at the composition boundary, with the exact
boolean session callback signature. No production behavior or assertion was
weakened; the real MMC3 battery test separately exercises disk failure and dirty
RAM preservation.

Code-size method: staged `git diff --numstat -- src/app-mynes test/app-mynes`,
including new tests and their CMake/static check, excluding docs/artifacts:
+430/-95, net +335. Production is +181/-61; tests are +249/-34. Explicit field
serialization accounts for most production growth; this removes hidden layout
coupling rather than adding an abstraction. New tests replace compressed
snapshot assertions with readable malformed-state/continued-execution checks.
There is one codec, one candidate commit and one checked composition save path.

The MyNES documentation gate, whitespace check and six-root unchanged check
pass. The EXEs are optimized stripped Release developer artifacts; objdump
reports x64 PE and x86 PE respectively and no `.debug` sections. Their source
is this S12 implementation P based on 1ee811710. Product identity and target
revision remain MyNES 0043; existing Running/Paused titles and INI stay intact.
Build trees are retained for the still-open T43. No external-ROM qualification
or T closure is claimed.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| assets/mynes/mynes_0_0_0043_x64.exe | 248334 | 8C8516C27E2BA6A653D98EF3859DA90455B5F002C2E14953A5AC67993007FED7 |
| assets/mynes/mynes_0_0_0043_x86.exe | 242190 | A3DE2C05A6CEADA3A7B4125D34FE537F49E29ACB8DB789CED1D46DDBE0CE05AC |

Implementation P1 d92bccd7d is committed and pushed. The coordinator then
reviewed its actual source/test/document/artifact diff, verified the five-item
disposition against the original request and confirmed no Shared/NXVM changes.
S12 is accepted and closed; this governance-only closure changes no executable
input. Owner gameplay verification is pending and T43 remains open.
