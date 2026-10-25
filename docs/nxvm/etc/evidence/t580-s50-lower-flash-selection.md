# T580 S50 - Guest-owned lower Flash selection

## Baseline and contract

Baseline is pushed `d347ce8b6`, after the accepted integrated-FDC receiver.
The batch implements the independently identified lower Flash selection in
[S48](t580-s48-post-initialization-route.md); it does not attribute every later
POST delay to Flash. The unchanged TFT BIOS, external source pins and hashes
remain those recorded there. No third-party source or binary bytes are imported.

The real firmware's RAM trampoline toggles planar port 98h bit 2, makes E/F
external, copies the two banks and restores its saved configuration. Together
with the external four-bank description, that is an L2 functional decoder
contract, not an original chipset truth table or physical timing formula.
Zero cold/reset value, preservation across processor-only reset, and a fixed
upper-bank high reset alias are explicitly L2 model choices. Writable Flash,
VPP, power effects and unrelated planar-control bits are not qualified.

## One production owner

The existing App firmware binding owns one byte of planar-control state.
Its existing port-route callback accepts byte writes at 98h and reads back
the latch. A bit-2 transition invalidates the existing E/F memory decode view.
The existing external Flash decoder selects original banks 0/1 rather than
2/3; internal shadow selection is unchanged, as is the high reset alias.
All other latch bits are preserved for read/modify/write, without assigning
them invented hardware effects. Unsupported write widths are rejected.

The guest, not the host, copies bytes into RAM. There is no decompressor,
firmware redirect, synthetic service, second memory path, clock or loader.
Lib, Emulator and every Shared source/test tree are unchanged; no manifest
needs regeneration for this App-local change.

## Existing-fixture proof

The existing firmware-binding test covers both selected low banks and every
16 KiB access field, ignored external writes, shadow independence, upper-bank
restoration, the high reset view, processor-only retention and complete reset.
The configuration-route test executes guest IN/XOR/OUT and memory instructions:
the CPU selects lower Flash, copies a byte from E000 into ordinary RAM, and
restores the upper selection. It does not inject a host copy or claim that
this small fixture executes the actual compressed Setup image.

Both final App unit selections pass 11/11 (x64 5.92 s, x86 0.87 s).
All four current preview targets build in the original stable Ninja/ccache
graphs. Production adds 20 lines; existing tests add 54 lines, with no removed
lines, new helper, public Shared API, test target or framework.

| Path | Added | Removed | Net |
| --- | ---: | ---: | ---: |
| src/app-mypc110/pc110_config.c | 14 | 0 | 14 |
| src/app-mypc110/pc110_firmware.c | 5 | 0 | 5 |
| src/app-mypc110/pc110_firmware_interface.h | 1 | 0 | 1 |
| test/app-mypc110/unit/config_route_smoke.c | 16 | 0 | 16 |
| test/app-mypc110/unit/firmware_binding_smoke.c | 38 | 0 | 38 |
| Total | 74 | 0 | 74 |

The preview files stay in each original build graph, not assets. Their PE
machine fields and SHA-256 values are:

| Width/variant | PE | SHA-256 |
| --- | --- | --- |
| x64/TFT | 8664 | `B8327D98D8D7E180F969EE0006D324769B798231F6AD23CB2EFAA12F51E7F9AF` |
| x64/derived DSTN | 8664 | `611847F5325D93BB058E862AFAFA5C5B79FAD7BB5459E055CF5D342C29823C52` |
| x86/TFT | 014C | `EEE22F767028EA68439CFAFC8250E069FEBE0FD30BDEEBDFEF898606C5619EFC` |
| x86/derived DSTN | 014C | `B75A65D21F872DC1142D4EC6B2ACCDCCAD33356EB51D53DEA33497D7196DD030` |

## Actual unchanged-firmware disposition

The x64 normal-route observation executes 32,000,607 instructions in about
153 seconds, with timer pass/delta 166C, initialized handler, ASMI/RSM and
three staged processor resets. Status is OK with BUDGET containment, no
terminal fault, and no observed INT 19h, Setup call, lower-bank toggle or
loader entry. Four previously recorded protected-memory GP probe deliveries
remain visible; this is not an exception-free POST claim.

The final PC 392BC is inside the copied SMM module's 64 KiB checksum loop
(LODSW, two byte additions, LOOP with initial CX=8000h). A sample there does
not prove a deadlock, broken checksum, missing wake or successful boot.
The temporary route sampler is removed; no test-only instrumentation remains.

The established actual-BIOS timer/SMI/FDC integration still passes: x64
61.65 seconds and x86 79.91 seconds. Both report DOR assertion/release,
IRQ-wait return and all four reset SENSE results, with the same timer delta
166C and 11,612,848 executed instructions. These are prefix regressions, not
qualification of subsequent Setup, boot-sector transfer or guest execution.

## Remaining qualification

The lower-bank receiver has direct fixture proof; the real BIOS bulk
copy/checksum/loader consumer has not executed in this bounded normal-route
sample. Actual Setup/boot, selected storage/guest, original-DSTN provenance,
panel/input/power, live Product lifecycle and release remain T580 gates.
Neither a preview build nor the retirement ceiling authorizes PC110
publication. Previously published Apps, owner INI, ROM/font masters, disks
and snapshots remain unchanged.

## Complete unit regression and review

Both original cached graphs build the complete `build-unit-tests` target.
The full repository-only unit selection executes all 712 cases per width:

| Run | Result |
| --- | --- |
| Initial x64 unit suite | 709 pass, three unchanged static checks time out; 434.23 s. |
| Initial x86 unit suite | 708 pass, four unchanged static checks time out; 216.94 s. |
| x64 failed-item rerun, one at a time | All three pass; 159.87 s. Types layout self-test 147.74 s, Debug corpus negative 6.08 s, Core verifier negative 5.94 s. |
| x86 failed-item rerun, one at a time | All four pass; 165.01 s. Types layout self-test 137.28 s, Debug corpus negative 8.50 s, IBM PC corpus negative 6.62 s, Core verifier negative 12.50 s. |

Thus each of the 712 current unit cases has passing evidence on each width;
neither initial full invocation was wholly green. No source, fixture,
assertion, registration or timeout changes between those runs. The initial
architecture runs overlap; the limited reruns reduce competing test jobs.
These observations do not establish a production defect or prove the precise
host scheduling reason for the initial timeouts. No new retry mechanism or
test-budget exception is introduced.

App/test-support boundaries, documentation governance and `git diff --check`
pass. Unchanged preview builds report `ninja: no work to do` in both stable
graphs. The actual-change review checks all three changed App production
files, both existing test extensions and documentation against the packet.
The sole retained path is port route -> App latch -> existing firmware
decoder. The additional state represents a real guest-visible control byte;
the test growth supplies the missing bank/reset/copy combinations rather
than introducing another harness.

The complete external integration corpus, owner desktop/RDP, physical panel,
actual Setup and guest boot are not executed or qualified by this S. Its
actual firmware integrations cover only the established initialized/SMI/
timer/FDC prefix. No unrelated App binary refresh is necessary because none
links this App-owned production change.

## Task cleanup record

All owned test/observation sessions have exited. The seven task logs total
250,232 bytes; after retaining these facts and hashes, the official cleanup
tool previews and removes `build/t580-s50-flash`. Preview reports no deletion;
Apply removes exactly seven files/250,232 bytes. No cache, owner file or
external source is removed.

| Temporary log | Bytes | SHA-256 |
| --- | ---: | --- |
| x64-fdc-regression.log | 1876 | `265BA93258B5269186CF537313F21EC6E96840D697ADEABBA6397CA06D34F285` |
| x64-route.log | 763 | `2D7423EB63E62794FF2E10F8D5AF90C5D4B97365F70A4C74F8393401F02F0878` |
| x64-unit-rerun.log | 679 | `A0C3A71A89D44B845B20E3D0CC2AD686007118F560C1E7A6C001B66DD3AE1614` |
| x64-units.log | 122041 | `B7136B533C55CA8095DC524E7268C28FEA9882B3057D2126008320750A2C6B80` |
| x86-fdc-regression.log | 1876 | `47941303AC0E96ED90C2FDBBF76AA26941A01CC1875E6B8A6BF6110274B9A082` |
| x86-unit-rerun.log | 834 | `C5130A91A82D0039A215FB552B36E477D7A5DB0FD45D0940B7163389E8BBE332` |
| x86-units.log | 122163 | `F0CEAAF86E7ADF9A6A07D507FA7C791B818D3D78A057CCBF87040E159BDE39E4` |
