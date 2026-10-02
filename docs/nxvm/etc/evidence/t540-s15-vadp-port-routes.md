# M5 T540 S15 VADP Port Routes

Initial CGA and the staged generic EGA, Compaq enhanced-color and VGA port
sets now use the existing Core-owned typed route batch. The video chip remains
VADP's sole state owner. VADP no longer stores or registers through `t_port`;
the old per-port registration and external checkpoint are deleted. The route
tables retain their original address order and read/write directions.

## Failure boundary and actual diff

- Initial CGA creation publishes its eight routes as one batch. Allocation
  failure at each position leaves a previously occupied port intact and no
  CGA route, and a subsequent creation succeeds.
- Staged configuration first prepares the complete candidate chip and its
  existing memory attachment. Only then does one Core batch publish all
  selected ports: 20 generic EGA, 27 generic EGA/VGA or 27 Compaq EGA routes.
  Every allocation position was injected. Core rolls back the batch; VADP
  removes the candidate memory owner and destroys the candidate chip. The
  original CGA chip and routes, unrelated port and memory owners remain.
  Retrying succeeds. A collision on the last Compaq port (`0xfc6`) likewise
  leaves no earlier Compaq route and preserves the occupying reader.
- On success, the candidate replaces the prior video chip only after the
  batch publishes. Teardown removes VADP-owned routes through the Core owner
  operation. The existing memory route remains a separate S16 receiver.
- The actual tracked production diff in `devices/{machine,vadp}.c` and
  `vadp.h` adds 46 and removes 42 lines (net +4). Ten owner-local test files
  add 399 and remove 349 (net +50), mainly to construct through a Core
  machine and to exhaust the failure matrix. The NXVM CMake/preset/verifier
  maintenance adds 21 and removes 15 tracked lines (net +6), plus one new
  36-line VADP gate. No Shared, MyNES, firmware, INI or media file changed.

## Receiving proof

The `vadp.c`/`vadp.h` sweep finds no `t_port`, raw port add or registration
checkpoint. `verify-vadp-port-routes` guards the sole CGA and staged batch,
Compaq/generic selection, VGA extension, candidate memory rollback and
owner-scoped removal. Relocating `vadp.c` from `core-machine-executor` into
the existing strict `core-machine` runtime removes its reverse static-library
dependency; no second video path or forwarding library was introduced.

The full specialized gate run also exposed stale verification metadata from
earlier source moves. NXVM-only updates now resolve T332 CPU fixtures under
`test/x86/chips`, remove one obsolete dependency allowlist edge, point three
T388 checks to current test files, name the real `x86-cpu-shared` build target
in the strict matrix, remove VADP's obsolete deferred-compilation row, check
the current Core batch in T296, and name the current 0540 artifact in the
GCC presets. The resulting specialized suite passes 69/69. These edits change
no production behavior.

## Verification and artifacts

- Final repository-only unit suites: x64 467/467 and x86 467/467.
- Focused CGA/EGA/Compaq/display tests: 14/14 per width. External EGA planar
  DOS checkpoint: 1/1 per width. The T540 full four-machine external matrix
  remains a T-level exit requirement, not an S15 claim.
- Documentation governance, the complete specialized gate suite and
  `git diff --check` pass. Four x64 and four x86 0540 profile products were
  rebuilt as optimized Release executables. `objdump` confirms each PE
  architecture and the absence of `.debug` sections.

| Product executable under `assets/nxvm/` | SHA-256 |
| --- | --- |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | `362832B9302C9B0384CAC8DA9337933F3E8FE04CBF89C5A20874C77FF1001062` |
| `compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | `E06942244FCF8FC3AFCD763A03B28A3D9A8263E73A8A7BCCD3B995C80700CA1E` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | `E11A095827B9BB3B81745D4388CD766B4F97F11AD7549D78EEAF428939C55847` |
| `default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | `31675018437AE30A02081FDC9F9848E203E01E78943C211A9A3450FF0AF4386A` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | `59F7B5E42C5317C4F7AE7109D47D5E66A39F3D57E0480CD642CFE642C701053C` |
| `ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | `9C7D0F37CDB8BE7370DA12D771356C1662D3D263AD950C0149293FF199280C03` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | `E83155E9AC4AEF21D0B9AF0F3384EF982F1F1F801143A30B3F1746B70B20B04A` |
| `ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | `214D4DDAD36B8C265D1B341454096AD4A28F14DFB33C851F2F9E9D8105E23DF7` |

No new timing-grade claim is made by this structural cut.
