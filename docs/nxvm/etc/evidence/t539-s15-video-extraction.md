# T539 S15: Independent Video Family Evidence

Baseline: 8831f05c5. The [boundary and chronological review](../architecture/t539-s15-video-extraction.md)
records the admitted scope and intermediate findings. Current owns acceptance;
this evidence does not close T539 or promote a hardware timing grade.

## Ownership And Original-Case Disposition

The sole register/latch/VRAM/raster/frame owner is x86/devices/video. Its static
target depends only on Types. NXVM vadp.c retains PC port decode/registration,
physical-memory adaptation, borrowed backing reads and atomic construction.
No old video algorithm or mutable chip layout remains in the App. The live
machine display-provider slot and copied guest frame stay with their board and
driver consumers. The unused presentation_interface.c/h and isolated smoke
are removed after whole-repository caller review; no production caller existed.

| Original case family | Independent owner proof | Retained receiver responsibility |
| --- | --- | --- |
| VADP text/status | video_text, video_text_status | machine snapshot and profile glyph wiring |
| CGA 320/640 | cga_graphics, cga_640 | actual ports, physical memory and system capture |
| EGA sequencer/controller/external | ega_sequencer, ega_controller, ega_external | RAM fallback, write observation, port selection and mapped chain-4 |
| CRTC/mode10 | ega_crtc_boundary_port, ega_mode10 | existing profile/system composition |
| Planar | ega_planar | physical aperture, fallback RAM byte and copied frame |
| Compaq CECG S9/S10/S12/S13 | compaq_cecg_s9/s10/s12/s13 | nineteen combined original personality-port assertions; S11 gate/reset isolation |
| Compaq EGA S6/CECG S28 | compaq_ega_s6, compaq_cecg_s28 | page/alias mapping and Model40 system cases |
| Allocation/public contract | video_allocation, video_contract | whole-display retry, foreign route preservation and teardown |

Chip-only assertions move with their original expected register/pixel values.
Mixed tests keep board assertions and add independent owner counterparts.
UNSUPPORTED is the chip's rejected-cycle result; only the receiver tests the
resulting ordinary RAM fallback. Fixtures never copy the port dispatcher or
mirror live VRAM. The final suite has 367 tests rather than the intermediate
368 solely because the dead presentation helper's isolated test is retired.

## Actual Source Review

Function comparison against the baseline accounted for 118 old and 124 new
functions. Ninety matched after declared name/type/register-selector transport
and constant relocation. Twenty-three matching functions required explicit
before/after review: typed CGA/planar buffers and plane allocation, aperture
predicates, memory observer, light-pen selector, configuration, reset/finalize,
advance and four graphics captures. Typed pointers replace integerized host
pointers; raster arithmetic, masks, pixel loops and failure results remain.
The CGA preset selector still decodes 03DCh; mono/color Compaq latch routing is
unchanged. The borrowed reader replaces physical RAM access, not its algorithm.

The two renamed capture entry points preserve mode selection and text wrap,
geometry/glyph/cursor/dirty behavior. Three old registration/initialization
functions are resolved into board routing and opaque creation. Nine additional
functions provide the single register dispatcher, create/destroy, three-field
copied diagnostic observation, bounded memory operations and write notification.
The new dispatcher was reviewed against all old port registrations, including
overlapping Compaq/VGA selectors; the App alone assigns PC addresses.

Configuration is construction-only. The board prepares the candidate before
publishing routes, replaces the old chip only on complete success, and removes
only candidate-owned memory registrations plus the new port suffix on failure.
Memory compaction preserves unrelated route order; port removal preserves
unrelated entries. Teardown unbinds before freeing the callback owner. Tests
cover all eight initial CGA, twenty EGA and twenty-seven VGA allocation positions,
provider/observer exhaustion, overlay priority, retry and frozen-map teardown.
The optional VGA route omitted by the initial adapter was restored within the
same transaction, not as a second runtime configuration path.

The three-field bus observation replaces six private reads at two real boot
diagnostic sites; its same-owner test proves no register/latch/frame mutation.
The scheduler retains board clock conversion and advances the one shared chip.
Its static gate needed the new symbol name, not a timing relaxation.

## Verification

- Final complete units: x86 367/367 in 48.27 s; x64 367/367 in 215.88 s.
- Default external integrations: x86 20/20 in 24.21 s; x64 20/20 in 21.60 s.
- Tools-off standalone x86 package: 43/43, including corpus/negative/manifests.
- Complete NXVM specialized aggregate passes, including strict direct compile;
  five video boundary gates also pass individually.
- Six manifests have exact membership and matching SHA-256: source Lib/Common/
  x86 108/22/47 files and test Lib/Common/x86 50/19/50 files.
- MyNES source and product/core CMake dependency review finds no x86 chip link;
  Lib/Common and MyNES inputs are unchanged. Its 0043 pair needs no rebuild.

Each vendor/profile/width boot ran once through the real adjacent INI and
embedded firmware, with external overlay media and no synthetic F1 or probe
mode flags. The process containment was 110 seconds; time limits were not
success criteria. Every row reached BOOT-PROBE=installer-ready and DOS 5 text:

| Machine | x86 | x64 |
| --- | --- | --- |
| XT | installer-ready; host duration not recorded | installer-ready; 33.05 s |
| AT | installer-ready; 63.09 s | installer-ready; 54.55 s |
| Model40 | installer-ready; 86.06 s | installer-ready; 83.11 s |

The diagnostic FDC failed-terminal counter is not an installer failure predicate;
the record does not claim every guest-issued command succeeded. Acceptance is
the existing installer marker, not a new blanket device-correctness claim.
Runtime suites/boots were serialized. Both reusable build trees were restored
to default afterward; only the profile embedding/link step changed. Owner INIs
and external master assets have no content changes. The three build/t539-s3
trees remain needed for the immediately following chip extraction.

## Size And Retained Paths

Count: git diff --numstat plus line counts of all new source/test files,
against 8831f05c5; manifests, documents and binaries excluded. Source: 27 paths,
+3055/-2673, net +382. Tests: 55 paths, +2269/-1212, net +1057. Build/gates:
seven paths, +38/-72, net -34. Combined net +1405 is predominantly independent
chip tests and explicit public/board boundaries, not a second implementation.
The production increase buys opaque typed access, complete route rollback and
dispatch separation. The physical-memory adapter and machine presentation
conversion remain because they cross real, different contracts; no forwarding
facade preserves the deleted chip API.

## Deployed Artifacts

All are optimized stripped Release 0.5.0539 products, with runtime debugger
retained, under assets/nxvm/<profile>. PE machine values were checked: x86
014C, x64 8664. Build-selected commercial firmware remains external BYOB input;
the owner explicitly requires these embedded-ROM EXEs to be committed/pushed.
Final default hashes reflect restoration of the reusable build trees after
vendor qualification, with unchanged tested source and default inputs.

| Artifact | SHA-256 |
| --- | --- |
| nxvm_default_0_5_0539_x86.exe | EC57880F2C5D3BB98ABCA254E7AD95A83A1CF0751A56D8B5825F0F9E3CD63896 |
| nxvm_default_0_5_0539_x64.exe | 6F9A85DDAC4D95BED857D36C70265516149E016D918236815BA40FEE1531F93F |
| nxvm_xt_0_5_0539_x86.exe | B4BD3C2AB0C985655F1A86E52F7F1B1EB8B91A28F641A96619287F5683A33F31 |
| nxvm_xt_0_5_0539_x64.exe | 0BD7167890032BD154B4F7C60312C7A1BE19B7C913F2FA5055983CFFC4D122BA |
| nxvm_at_0_5_0539_x86.exe | B1337CBEAD222DE57326FA9DADB1FEAB05AF4D68488C757A35FD5047357C1607 |
| nxvm_at_0_5_0539_x64.exe | 14516A4D1427174C7C90F29268C5DCA3FF3D82FA2616DFEB708DB3D127ACB64A |
| nxvm_model40_0_5_0539_x86.exe | 242F7D7FB553BA22B7F12076072FFD152281078477786DFB34DF3A3825D8FDE0 |
| nxvm_model40_0_5_0539_x64.exe | 296A24FBB991297A912F174C9A7780F52DA6A15CAC0C8A015D0D3CC811CD070F |

Source identity is Shared 522d0b27f plus NXVM 88ae417ff. Coordinator review of
the actual commits accepts this bounded batch; the post-delivery tools-off
suite passes 43/43 and artifact hashes remain as above. T539 is not complete:
CPU/FPU and the remaining finite ledger stay in its original scope.
