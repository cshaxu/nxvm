# MyRaidenII Status

## Current Work

M1 T4 remains open after S44 P1 `1b34c4155`: Video now converts the four
selected sprite image lanes into two 32-bit little-endian word-lane regions.
The result is available to the later one-owner decrypt/draw path without
implementing either behavior.
The owner has admitted published MAME/other emulator implementations as L2
layout/behavior evidence; source, tables, ROM data, and decryption material
remain prohibited from import or transliteration.

S43 was accepted after actual-change review of `6b5def499`: two owner-local
helpers read only safe CRTC word pairs and apply them to the existing layer
draw path. Synthetic regression proves enable, per-layer X/Y scroll and
modulo-512 source behavior; Board regression retains its public byte-route and
reset proof. Both cached widths passed all three MyRaidenII unit entries and
documentation governance. No external media, artifact, desktop or playable
game claim is made.

## Archived Packet — M1 T4 S44

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S44 follows coordinator-accepted S43 P2 `9468f4cca`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly directs continued implementation using external emulator behavior/layout as L2 without further authority churn, 2026-10-20. |
| Objective | Correct the one Video graphics ingress so four immutable 2 MiB sprite lanes become their selected 32-bit little-endian 8 MiB stream for a later sprite owner. |
| Non-goals | No sprite decryption, object decode/draw, Board object semantics, priority/blending, Sprite flags interpretation, Shared change, real media read, or external source/table/ROM import. |
| Reference Baseline | Video currently copies the four sprite inputs consecutively into an 8 MiB allocation, while inspected L2 board layout establishes a four-lane 32-bit little-endian sprite region. No current reader depends on their incorrect contiguous-per-file arrangement. |
| Candidate Proposal | Retain one graphics allocation and one loader; interleave the four owned input lanes into the existing sprite-region capacity, name the stream layout explicitly, and extend the existing synthetic graphics smoke with a finite lane-order proof. |
| Files And ABI Surface | `src/myraidenii/core/{video.c,video.h}`, `test/myraidenii/unit/core/graphics_smoke.c`, L2 evidence and task record only; no external function/type ABI change. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation and source policy; execution rules. Video retains sole graphics-copy ownership; synthetic tests use no external payload; L2 records only the finite lane organization. |
| Verification | Passed focused graphics/Board smoke on x64/x86; then passed all three cached MyRaidenII unit entries on each width, documentation governance and `git diff --check`. |
| Expected Markers | Every four-byte synthetic stream group preserves lane order and repeated source indices; character/background layouts remain unchanged; loader failure/release behavior remains one allocation/owner. |
| Asset Needs | None; tests allocate synthetic bytes only and do not access external ROM/media. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, the finite L2 lane fact, verification per width, and deferred decrypt/object/render work. |
| Stop Conditions | Any needed decryption table/algorithm, actual sprite object protocol, Board API, Shared change, or external payload is deferred rather than guessed. |
| Exit Criteria | Met: the only sprite ingress is explicitly lane-interleaved; owning Video regression rejects the former file-concatenation layout; Board route regression, prior graphics ownership and all unit entries remain intact. |
| Original Owner Request | Continue implementing MyRaidenII toward RAIDEN II using external implementations as L2 rather than treating absent physical L3 sources as a repeated blocker. |
| Similar-Issue Sweep | Recheck all seven graphics-role offsets, allocation size/overflow, loader/release failures and consumers for duplicate sprite ingress or accidental interpretation. |

## Archived Packet — M1 T4 S43

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S43 follows coordinator-accepted S42 P3 `b1dd46f0f`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly directed continued MyRaidenII implementation under external emulator L2 evidence without back-and-forth on 2026-10-20. |
| Objective | Make the existing Video composition path consume the finite CRTC active-low Background/Midground/Foreground/Text enables and three tile-layer scroll pairs. |
| Non-goals | No sprite decode/decryption/drawing, base-scroll behavior, text scroll, blend/priority behavior, Machine publication, Shared change, ROM/media input, or copied/transliterated source/table. |
| Reference Baseline | S42 P2 `37cba797a` establishes correct static map/palette/order ownership, while the existing CRTC register file is routed through Board but has no frame effect. |
| Candidate Proposal | Add owner-local CRTC word and enabled helpers; conditionally draw the four existing layers and apply modulo-512 source coordinates for the three 16 by 16 tile layers. Reuse the sole compositor, Board CRTC route, and existing smoke files. |
| Files And ABI Surface | `src/myraidenii/core/video.c`, its public comment only if required, `test/myraidenii/unit/core/{graphics_smoke.c,board_bus_smoke.c}`, L2 evidence, manifests and task record; no function/type ABI change. |
| Applicable Rules | MyRaidenII guide, architecture, coding, documentation and source policy; execution rules. Video remains the single raster/composition owner, Board remains the one CRTC route, and external evidence supplies only finite L2 behavior/layout facts. |
| Verification | Run focused graphics and Board tests on x64/x86; then cached x64/x86 MyRaidenII unit selections, manifests/dependency gates, documentation governance and `git diff --check`. |
| Expected Markers | Active-low layer masking leaves the frame clear when all tiles are disabled; each three scroll pairs shifts only its own layer; a Board bus write changes the same sole Video frame output. |
| Asset Needs | None; all regressions use synthetic buffers and do not read or modify external ROM/media. |
| Reporting Requirements | Report source/test/doc added/removed/net lines, the exact adopted L2 facts, verification per width, and deferred sprite/base-scroll/text-scroll/Machine work. |
| Stop Conditions | Any needed sprite/decryption/base-scroll contract, Shared API, new renderer, external data payload, or behavior beyond the named finite CRTC fields is transferred rather than guessed. |
| Exit Criteria | Met: one Video owner consumes the named enables/scroll pairs, Board retains the only public CRTC route, and synthetic regression proves the previous inert-register behavior cannot recur. Evidence is updated; this product has no file manifest. |
| Original Owner Request | Continue implementation toward RAIDEN II with external implementations admitted as L2, without treating the lack of a physical L3 source as a blocker. |
| Similar-Issue Sweep | Recheck all existing CRTC frame-affecting fields, Video/Board ownership, source-coordinate bounds, layer order/transparency, and public API changes for a second route or unapproved behavior. |

## Archived Packet — M1 T4 S42

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective continuation: M1 T4 S42 P2 corrects the unaccepted P1 tilemap/palette L2 mapping; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved MAME/other emulator implementations as L2 evidence and explicitly directed continued use without further back-and-forth on 2026-10-20. |
| Objective | Correct the four non-sprite tile-layer RAM ordering and palette bases in the sole MyRaidenII Video composition path, and make its owning smoke reject the former swapped mapping. |
| Non-goals | No sprite decoding/decryption, scroll/enable behavior, blending, rotation, Machine frame publication, external ROM data, copied reference source/table, or Shared change. |
| Reference Baseline | S42 P1 `6944777f4` publishes an initial 320 by 240 frame, but its Mid/Foreground tilemap offsets and palette bases disagree with the recorded L2 source layout. |
| Candidate Proposal | Retain one Video compositor; draw Background from 000h/400h palette, Midground from 800h/420h palette, Foreground from 400h/410h palette, and Text from c00h/700h palette. Strengthen the existing synthetic smoke with distinct entries/colours and transparent peel-down checks. |
| Files And ABI Surface | `src/myraidenii/core/video.c`, `test/myraidenii/unit/core/graphics_smoke.c`, S42 evidence and task record only; no public API/ABI changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. The L2 implementation is independent, records revision/path facts only, and does not import or transliterate source, tables, ROM data or decryption material. |
| Verification | Run the owning Core graphics and Board tests x64/x86, static/manifest checks, then the required repository unit selections under the documented junction execution context. |
| Expected Markers | Distinct foreground/midground offsets and palette groups are exercised; transparent layers reveal the correct lower layer in the correct order; no alternative composition owner exists. |
| Asset Needs | None; tests use synthetic buffers only and external ROM/media remain untouched. |
| Reporting Requirements | Report source/test added/removed/net lines, exact L2 limitation, verification by width, and all deferred sprite/scroll/frame-publication work. |
| Stop Conditions | Any needed sprite decrypt source/table, ROM extraction, Shared API change, or behavior beyond finite L2 layer mapping is transferred rather than guessed. |
| Exit Criteria | Met: the sole tile composition path conforms to the recorded L2 map/order/palette facts; the regression rejects the former swap; evidence and manifests are updated. |
| Original Owner Request | Continue MyRaidenII implementation with external emulator behavior/layout as approved L2, without repeatedly requesting that authority. |
| Similar-Issue Sweep | Recheck all four tile layers for one owner, source/test mapping agreement, bounds, transparent-pen behavior and absence of copied source/table/decryption material. |

## Archived Packet — M1 T4 S41

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S41 follows accepted S40 P2 `91f7f76fa`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly authorizes MAME/other emulator implementations as L2 references. Source policy forbids copying/transliterating their source, tables, ROM data or decryption material. |
| Objective | Derive and independently implement the first bounded Core graphics decode/composition contract from recorded L2 source facts, then qualify it with synthetic ROM-free vectors. |
| Files And ABI Surface | MyRaidenII Core video/graphics, its owning unit test, evidence, manifest and task records only. No Shared, Product, external asset or ROM change. |
| Verification | Record source revision/path and adopted finite facts; run x64/x86 focused Core video tests plus the complete repository-only unit suite before S closure. |
| Exit Criteria | Met: the independently implemented character/background decoder passes synthetic regression and records its L2 scope; no reference source/table/ROM payload entered the repository. |

## Archived Packet — M1 T4 S40

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S40 follows accepted S39 P2 `9add13000`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved bounded L2 behavioral modeling with MAME as a black-box reference. MAME's public Lua documentation identifies `emu.pause()` and video frame updates as session control; no reference implementation/source is read. |
| Objective | Determine whether a paused external reference can render two synthetic graphics states without CPU/frame animation, yielding a finite repeatable element coordinate/value observation. |
| Non-goals | No MAME/FPGA source or debugger-memory inspection, no ROM/RAM/register/pixel payload retention, screenshot, trace, decoder/render/API/test/artifact change, decryption claim or playable-game claim. |
| Reference Baseline | S40's initial paused repaint produced no byte effect; its advancing mutation/control pair proves the earlier S37 screen difference was time advancement. S39 leaves no original layout source. MAME documents public pause/session and screen/video Lua capabilities. |
| Candidate Proposal | At a bounded post-startup frame, pause the reference, replace only its external tile region with a synthetic state, call the public video update, hold the baseline only in process, toggle one synthetic byte, call video update again, and print only dimensions, difference count/bounds and a checksum. |
| Files And ABI Surface | Documentation/evidence only; one ignored temporary script under a unique `build/` path. No repository production/test/manifest/API/shared/artifact or external media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME stays black-box only; all mutations occur in its process and no protected payload leaves it. |
| Verification | Run two identical paused-mutation trials and one no-mutation control; prove the callback/termination remains bounded; retain only declared neutral summary; delete all temporary script/output; run documentation governance and `git diff --check`. |
| Expected Markers | Either a finite stable element coordinate/value fact or an exact documented reason pause/frame-update cannot yield one under containment. |
| Asset Needs | Existing owner-supplied external archive and S35 external PALs, read-only; generated MAME configuration remains only under `nxvm-assets/media-myraidenii/raiden2/mame-config/`. |
| Reporting Requirements | Record MAME version/API class, pause/update outcome, neutral dimensions/checksum/bounds, repeatability and cleanup only. No raw pixel, ROM, RAM, register, trace or implementation content. |
| Stop Conditions | Pause/update needs debugger/source inspection, a saved state, protected payload retention, MAME modification/plugin, archive change, unbounded run, or non-repeatable output. |
| Exit Criteria | A complete bounded element contract is documented for a decoder S, or a truthful blocked field set is transferred without any production source claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no temporary script/log/NVRAM/snapshot persists and no second graphics owner, decoder implementation, MAME-derived source/table or protected payload reaches repository state. |

## Archived Packet — M1 T4 S39

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S39 follows accepted S38 P2 `b32ce2ed9`; one session performs coordinator and executor roles. |
| Admission And Approval | The product source policy permits original NEC/Zilog/Yamaha/OKI manuals and board observations as research. This packet does not import, download, commit or derive executable source from any reference. |
| Objective | Locate original/vendor board documentation or independently recorded physical measurement that specifies a finite Raiden II graphics-element layout/attribute contract suitable for an L2 Core decoder admission. |
| Non-goals | No MAME/FPGA/reference-source inspection, no decoder/render implementation, no third-party code import, no ROM/manual payload retention, no source/test/API/artifact change and no playable-game claim. |
| Reference Baseline | S38 proves the existing no-payload MAME black-box method cannot isolate a finite element coordinate result. S40 later corrects S37's unpaired causal attribution. |
| Candidate Proposal | Search only primary/archival metadata and public original-board documentation indexes. Retain URLs, publisher/holder, document identity, license/access facts and only the finite hardware facts directly needed for a later proposal. |
| Files And ABI Surface | `docs/myraidenii/{states,etc/evidence}` only. No product, Core, shared, asset, test, API, manifest or external-media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. Original documentation remains external research unless an independent acquisition/usage decision is later approved. |
| Verification | Recheck provenance and whether each result is original/vendor material rather than derived emulator material; record positive evidence or the exact evidence gap; run documentation governance and `git diff --check`. |
| Expected Markers | A finite provenance-qualified graphics contract, or a documented lack of one and explicit transfer boundary. |
| Asset Needs | None. Existing external game/PAL inputs are out of scope and untouched. |
| Reporting Requirements | Record only source identity, ownership/provenance, URL/access, permitted factual scope and limitations. Do not retain a copyrighted manual, screenshots, ROM content, MAME/FPGA implementation detail or raw game data. |
| Stop Conditions | A candidate is derivative/emulator-source based, access requires acquisition without policy approval, the document lacks element-layout facts, or a result would require source/table/transliteration. |
| Exit Criteria | A finite independently sourced contract is ready for a later decoder S, or the unresolved evidence field is transferred without a production claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no local research payload, external tool configuration, MAME artifact, decoder implementation, second graphics owner or non-approved reference-derived content is introduced. |

## Archived Packet — M1 T4 S38

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S38 follows accepted S37 P2 `6eb554da4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized M1/T4 L2 behavioural modeling and external MAME as a black-box research reference. This is a same-T bounded continuation; existing source-policy prohibitions remain in force. |
| Objective | Determine whether controlled synthetic tilemap, palette and graphics mutations can establish a finite coordinate/value graphics-element oracle sufficient to specify one initial Core decoder contract. |
| Non-goals | No MAME source/table/handler import, reading/dumping/retaining ROM or memory content, screenshots, raw traces, production/test/API/artifact change, layer/sprite composition, decryption or playable-game claim. |
| Reference Baseline | S31 fixed the seven image roles; S32 established L2 RGB555 lookup; S36 fixes 320×240; S37 proves a graphics-region mutation changes later visible output reproducibly. |
| Candidate Proposal | In a transient MAME Lua run, install only synthetic in-process graphics state and one known graphics-bit pattern, then retain only a bounded set of pixel coordinates/colours or an all-screen neutral summary. Repeat each admitted observation twice and delete all script/output files. MAME uses the external `nxvm-assets/media-myraidenii/raiden2/mame-config/` directory explicitly; it must not create repository-root configuration. |
| Files And ABI Surface | Documentation/evidence only; one ignored temporary script under a unique `build/` path. No repository production/test/manifest/API/shared/artifact or external media changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME is black-box only; all mutations occur in its process and no protected bytes or pixel payload leave it. |
| Verification | Prove synthetic isolation by repeatable observations with and without the selected mutation; verify only declared coordinates/colours/checksums are printed; delete all temporary data; run documentation governance and `git diff --check`. |
| Expected Markers | Either a finite replicated element-layout fact table, or an exact reason a coordinate oracle cannot be obtained under containment rules. |
| Asset Needs | Existing owner-supplied external archive and S35 external PALs, read-only; generated external MAME configuration is stored only under `nxvm-assets/media-myraidenii/raiden2/mame-config/`. |
| Reporting Requirements | Record only MAME version/API class, synthetic state classes, neutral coordinates/colours/checksums, repeatability and cleanup. No raw address-to-content dump, image, ROM byte, memory dump or MAME implementation detail. |
| Stop Conditions | The experiment needs source inspection, protected payload retention, an archive change, an unbounded observation, a MAME modification/plugin or non-repeatable output. |
| Exit Criteria | A complete bounded element contract is documented for a decoder S, or a truthful blocked field set is transferred without any production source claim. |
| Original Owner Request | Continue MyRaidenII under the approved L2 model and external BYOB assets until RAIDEN II can be run, rather than stopping after a planning-only result. |
| Similar-Issue Sweep | Confirm no temporary script/log/NVRAM/snapshot persists and no second graphics owner, decoder implementation, MAME-derived source/table or protected payload reaches repository state. |
| Later Correction | S40's repeated no-mutation control proves that S37's later checksum difference was ordinary reference-frame advancement, not established graphics-region sensitivity. |

## Archived Packet — M1 T4 S37

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S37 follows accepted S36 P2 `b6b80ccda`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved continued M1/T4 implementation under L2 behavioural evidence, MAME as a black-box reference only, and external BYOB PAL inputs. |
| Objective | Derive a bounded reproducible L2 graphics-element contract from controlled external MAME runtime mutation, or record the exact missing element without guessing a decoder. |
| Non-goals | No MAME source/table/handler import, ROM/image dump or retained payload, screenshot, raw trace, source/test/API/artifact change, renderer/frame publication, decryption claim or playable-game claim. |
| Reference Baseline | S31 owns fixed-role image ingress; S32 owns RGB555 lookup; S36 fixes the external 320×240 screen checkpoint. MAME's published Lua interface exposes memory-region writing and screen observation. |
| Candidate Proposal | Mutate only synthetic in-process bytes in an external reference and retain only neutral dimensions/checksums; use the result to decide whether a decoder contract is actually established. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` plus a transient ignored build script only. No source, test, manifest, shared component, asset or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME remains research-only, external inputs read-only, and protected payload is never retained. |
| Verification | Run two identical bounded trials, verify official Lua API capability, remove the transient script/output, run documentation governance and `git diff --check`. |
| Expected Markers | A finite L2 fact table or a precise missing-contract transfer; no production-behaviour claim. |
| Asset Needs | Existing owner archive and S35's external PAL files, read-only and untracked. |
| Reporting Requirements | Record tool/API class, synthetic mutation class, neutral repeatability and cleanup; never record raw memory/pixel/ROM content. |
| Stop Conditions | Source inspection, retained protected data, archive mutation, non-repeatability, unbounded trace, or a claim stronger than L2 is required. |
| Exit Criteria | Evidence establishes a complete bounded graphics-element contract, or truthfully records the exact unresolved element with all transient data removed. |
| Original Owner Request | Continue MyRaidenII under owner-approved L2 evidence and external BYOB PAL setup toward running RAIDEN II. |
| Similar-Issue Sweep | Confirm no MAME configuration/NVRAM/snapshot/log/script survives and no second graphics owner, renderer or source-derived layout enters the repository. |

## Archived Packet — M1 T4 S36

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S36 follows accepted S35 P2 `4744a53e4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner approved continuing implementation under L2 behavioral evidence and explicitly authorized external BYOB PAL inputs. This packet uses MAME's documented Lua black-box API only; it imports no source, table, handler or protected payload. |
| Objective | Establish one reproducible, neutral selected-set screen checkpoint: visible dimensions plus a non-reversible byte-stream checksum at a declared emulated frame. |
| Non-goals | No screenshot, pixel dump, raw trace, ROM/memory/register capture, decoder/layout/decryption implementation, renderer/frame, shared change, product artifact or playable-game claim. |
| Reference Baseline | S35 makes current MAME 0.289 accept the owner archive plus the two external PAL files. MAME documents `-autoboot_script`, `screen:pixels()` and `machine:exit()` as public scripting facilities. |
| Candidate Proposal | Create one transient Lua file outside the repository, wait for a fixed video-frame count, compute Adler-32 over the returned visible-pixel bytes, print only tag/width/height/checksum/frame count, schedule exit, retain those neutral values and delete the script. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source, test, manifest, API, asset, archive or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. The probe is black-box, bounded and deletes all transient output; BYOB PALs remain external/untracked. |
| Verification | Run the probe twice with identical current-MAME options and external paths; require same dimensions/checksum and a bounded clean exit each time. Confirm the Lua file/output are deleted; run documentation governance and `git diff --check`. |
| Expected Markers | A stable selected screen tag, dimensions and checksum at the exact declared frame; no retained pixel data and no repository asset changes. |
| Asset Needs | Existing owner archive plus S35's owner-approved external PAL files, all read-only. |
| Reporting Requirements | Record MAME version, command class, frame number, neutral metadata/checksum, repeatability, cleanup and limitations. Do not retain payload, pixels, screenshots or memory state. |
| Stop Conditions | Lua API cannot yield a stable visible frame, output differs between identical runs, probe needs protected content capture, or any proposed decoder/layout claim would exceed neutral black-box evidence. |
| Exit Criteria | A repeatable, cleaned-up neutral screen checkpoint or an exact documented failure with no code changes; any decoder work remains a separate reviewed S. |
| Original Owner Request | Continue MyRaidenII after PAL acquisition and connect it through compilation/runtime evidence. |
| Similar-Issue Sweep | Confirm no MAME config/NVRAM/snapshot/log or script remains in repository/assets; verify MAME reads both external roots only and no raw data enters task evidence. |

## Archived Packet — M1 T4 S35

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S35 follows accepted S34 P2 `bce13d2b8`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner explicitly authorizes downloading the two required PAL images as external BYOB inputs, placing them only under `O:\repos.hobby\nxvm-assets`, without Git/manifest/EXE inclusion. This is a recorded exception to the default MyRaidenII source-policy acquisition restriction. |
| Objective | Acquire, hash-verify and stage only the PAL images required by current MAME's Raiden II verifier, then prove the current black-box runner accepts the existing owner archive plus those external inputs. |
| Non-goals | No source import, decoder/renderer/frame implementation, ROM/archive commit, asset manifest inclusion, EXE embedding, modification of existing owner archive, screenshot/trace retention, or playable-game claim. |
| Reference Baseline | Current MAME 0.289 requests `jj4b02__ami18cv8-15.u0342` (341 bytes, SHA-1 `8b46f6673ddf11efbc3394ae423ec89d4a1283bf`) and `jj4b01__mmipal16l8bcn.u0341` (279 bytes, SHA-1 `95ce9cfbfb280dfc6a326e378684eff3c6f54701`). S34 established that historical MAME can execute without the first dump. |
| Candidate Proposal | Download a published split-set archive to a temporary external staging path, extract exactly the two named PAL files, verify their size/SHA-1, delete the temporary archive, then run current MAME verification and one bounded headless launch with the existing archive plus the staged PAL directory. |
| Files And ABI Surface | External BYOB assets under `O:\repos.hobby\nxvm-assets\media-myraidenii\raiden2\roms\raiden2`; repository documentation/evidence only. No source, test, manifest, API, asset archive or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy, with the owner-approved external BYOB acquisition exception recorded above. Inputs remain external and untracked. |
| Verification | Verify exact filenames, sizes and SHA-1; confirm the staging archive is deleted; run current MAME `-verifyroms` and one bounded headless launch against combined external paths; run documentation governance and `git diff --check`. |
| Expected Markers | Both exact hashes match; current MAME no longer reports missing PALs; headless process terminates within budget; Git has no PAL/ROM additions. |
| Asset Needs | Owner-approved external PAL inputs only; the existing archive stays read-only and unmodified. |
| Reporting Requirements | Record source URL, exact hashes, external location, verifier/launch result and cleanup. Do not retain or report protected payload content. |
| Stop Conditions | Hash/size mismatch, extra extracted media, need to modify the owner archive, a request to commit/embed inputs, or evidence that current MAME cannot combine the staged files with the archive. |
| Exit Criteria | Current MAME accepts the two external PALs and completes its bounded black-box launch, or the exact combined-path limitation is documented without a repository workaround. |
| Original Owner Request | Download the PAL into nxvm-assets as BYOB, then compile/connect it. |
| Similar-Issue Sweep | Verify no temporary archive remains, no PAL/ROM is staged under the repository, no existing asset is overwritten and no protected input is added to Git. |

## Archived Packet — M1 T4 S34

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S34 follows accepted S33 P2 `3e708f2af`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner asked to source the missing PAL from MAME and continue. Source policy forbids downloading protected game/PAL data, so this packet uses only an official historical MAME binary as a transient black-box research tool. |
| Objective | Establish whether the existing owner archive can supply a lawful executable black-box baseline without obtaining either missing PAL dump. |
| Non-goals | No PAL/ROM download, archive mutation, source/table/handler import, graphics decoder, renderer/frame, shared change, screenshot/trace retention, playable-game claim or product artifact. |
| Reference Baseline | S33 used current MAME 0.289, whose strict verifier rejected two PAL blobs. The owner archive remains read-only at its recorded identity. |
| Candidate Proposal | Download the official MAME 0.182 Windows binary outside the repository; verify its identity, run `-verifyroms` and one bounded headless launch against the existing archive; retain only neutral status/speed observations. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source, test, manifest, API, asset or executable changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. MAME is research-only; protected inputs are read-only and no output payload is retained. |
| Verification | Verify the downloaded file is a PE, record its SHA-256, run official MAME `-verifyroms` and one ten-second headless launch, then remove the external tool directory after retaining neutral evidence. Run documentation governance and `git diff --check`. |
| Expected Markers | Historical verifier calls the set `best available` despite the no-good PAL; headless process terminates within its ten-second budget; no PAL is acquired. |
| Asset Needs | Existing owner-supplied external archive only; no new asset is accepted or stored. |
| Reporting Requirements | Record source, tool hash, commands, neutral outcome, cleanup and precise limit: executable behavior does not establish a decoder/layout contract. |
| Stop Conditions | Any need to download or preserve PAL/ROM data, inspect/copy third-party implementation, retain protected video/audio output, make decode claims from a source table, or modify production code without a separate contract. |
| Exit Criteria | A reproducible lawful black-box baseline or documented failure; archive S34 with the correction and transfer only the still-needed layout/behavior contract to later Video work. |
| Original Owner Request | Search/download missing PAL into external assets and compile/connect it. |
| Similar-Issue Sweep | Check that no tool, temporary configuration, archive output, screenshot, trace or downloaded protected media remains under the repository or external asset root. |

## Archived Packet — M1 T4 S33

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S33 follows accepted S32 P2 `fb7c3f5a4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized L2 behavior modeling when L3 primary hardware detail is unavailable. This packet permits an external emulator only as a transient black-box reference; it does not authorize source/table/handler import. |
| Objective | Establish whether a lawful, reproducible behavioral-oracle workflow can qualify later graphics layout/decryption work without exposing protected game bytes or copying third-party implementation. |
| Non-goals | No MyRaidenII production/test change, graphics decode, decryption, renderer/frame, shared change, ROM commit, screenshot/trace retention or playable-game claim. |
| Candidate Proposal | Obtain one official upstream emulator binary outside the repository, pin its release identity/hash, run it only against the owner-supplied archive with a bounded controlled launch, and retain only neutral observations/checkpoints. If its behavior cannot produce an independent layout contract, record that exact stop rather than invent a decoder. |
| Files And ABI Surface | `docs/myraidenii/{states/CURRENT.md,etc/evidence/}` only. No source/API/manifest/asset/product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source policy. External emulator remains research-only, all protected media read-only, and neither bytes nor derived decode tables are retained. |
| Verification | Verify source identity/hash, bounded-process cleanup, no retained protected outputs, documentation governance and `git diff --check`. No claim that reference execution proves hardware accuracy. |
| Stop Conditions | Need to inspect/copy/provide third-party implementation or ROM bytes, make a decoder claim from a source table, commit the external tool/output, or change production code without a separately reviewed contract. |
| Exit Criteria | A recorded, reproducible black-box workflow with a clear next contract or a documented insufficiency that leaves production unchanged. |
| Original Owner Request | Continue MyRaidenII under the authorized L2 model boundary rather than require unavailable L3 proof. |
| Similar-Issue Sweep | Check output retention, archive write access, process/temporary cleanup, output paths, license/source identity and whether any proposed fact is actually independent behavior rather than a copied source assertion. |

## Archived Packet — M1 T4 S32

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S32 follows accepted S31 P2 `97361495e`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continued MyRaidenII implementation and L2 models where primary hardware detail is unavailable. This packet changes only MyRaidenII Core/tests/documentation. |
| Objective | Give the existing 2048-word private Video palette one bounded RGB32 interpretation for a later native-frame owner. |
| Non-goals | No graphics ROM decode, tile/sprite layout, decryption, brightness, priority, scroll, frame publication, Machine/Product route, Shared change or playable-game claim. |
| Reference Baseline | S25 admits a complete 2048-word palette image without channel interpretation. Shared accepts 0x00RRGGBB frames, but MyRaidenII has no native frame yet. |
| Candidate Proposal | Add one Video-local checked palette lookup that interprets the stored 15 low bits as L2 RGB555, expands each 5-bit component to 8 bits and returns 0x00RRGGBB. Keep raw palette ownership and failed-load preservation unchanged. |
| Files And ABI Surface | `src/myraidenii/core/{video.c,video.h}`, existing Board smoke, CURRENT and one evidence record. No Shared/public component API, asset, ROM, Product or Machine change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The mapping remains L2, one Video owner and no imported source/table/algorithm. |
| Verification | Focused x64/x86 Board smoke with synthetic palette vectors and bounds/argument failure; existing selected-set probe; documentation governance and `git diff --check`. |
| Expected Markers | Zero, individual primary extremes and a mixed synthetic RGB555 word produce exact RGB32 values; invalid index/pointer leaves caller output unchanged; no frame is published. |
| Asset Needs | None. Tests use synthetic palette words only; no external archive access, extraction or content observation. |
| Reporting Requirements | Record the L2 source/limits, exact bit/expansion rule, failed-input behavior, actual diff and verification; state that this does not establish selected-board palette hardware accuracy. |
| Stop Conditions | Need to import a reference layout/table/handler, infer brightness/alpha, touch Shared, create a renderer/frame path, inspect protected media or claim exact board colour. |
| Exit Criteria | One reviewed MyRaidenII P with bounded conversion proof, dual-width focused qualification, evidence and cleanup; otherwise revert the whole interpretation rather than add alternate color paths. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check raw palette load/reset, all color extraction shifts, output alpha, index bounds, caller output preservation and existing RGB32 frame conventions so no duplicate palette converter or format appears. |

## Archived Packet — M1 T4 S31

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S31 follows accepted S30 P2 `0860cdeee`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continued MyRaidenII implementation and bounded L2 models without source import. This packet changes only MyRaidenII Core and its owned unit tests/documentation. |
| Objective | Establish the one Board-to-Video ingress for the selected set's immutable character, background and sprite image roles, with exact owner-supplied inventory sizes and lifetime rules. |
| Non-goals | No ROM extraction, protected media commit, decoder/decryption algorithm, tile/sprite semantics, frame publication, Machine/Product implementation, Shared change, renderer, or playable-game claim. |
| Reference Baseline | S30 reaches its containment budget with no remaining selected control-window fault. The audited inventory identifies one 128 KiB character image, two 2 MiB background images and four 2 MiB sprite images; Core currently accepts only program lanes. |
| Candidate Proposal | Extend the existing Board creation config with one fixed-role immutable image bundle. Validate a supplied bundle is complete; Video privately copies the roles so the caller may release staging data after creation, matching the existing program-lane lifetime. |
| Files And ABI Surface | `src/myraidenii/core/{board.c,board_interface.h,graphics_interface.h,video.c,video.h}`, existing Core Board smoke, CURRENT and one evidence record. This is an App-owned Core API extension only; no Shared API, asset, external archive or product artifact changes. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Preserve Core's inward dependencies, one Video asset owner, copied/lifetime-safe public input and `lib_status` failures. |
| Verification | x64/x86 Board smoke for exact input validation and copy/lifetime; existing selected-set probe to prove no program/control regression; documentation governance and `git diff --check`. |
| Expected Markers | Missing, mismatched or partial image roles reject creation without a Board; accepted synthetic role images remain owned by Video after caller staging is changed/released; no frame, decoder or external media is retained. |
| Asset Needs | Existing inventory metadata only. Tests create synthetic in-memory role images and never open, extract, retain or inspect the owner-supplied archive. |
| Reporting Requirements | Record the role/size/lifetime contract, one-owner proof, model grade/limits, actual diff and verification; explicitly state that asset ingress is not graphics decoding or output. |
| Stop Conditions | Need to inspect/copy protected ROM content, infer graphics layout/decryption, add a generic asset registry, create a second state owner, touch Shared, or publish an incomplete frame. |
| Exit Criteria | One reviewed MyRaidenII P with exact role validation and copied-lifetime proof, dual-width focused qualification, evidence and cleanup; otherwise abandon the whole ingress change without placeholder compatibility. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Board creation rollback, program/image role validation, Video reset/destroy, caller lifetime, every role's exact size and test fixtures were checked; no raw external asset is retained or another owner is introduced. |

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | Finite Video/Sprite convergence ledger; no invented receiver admitted without a lawful flag-family consumer contract. | P1 `ac0872270`; coordinator review: documentation governance and `git diff --check` passed. |
| S2 | Lawful-source disposition: existing sources cannot establish the sprite-control consumer contract, so code remains correctly blocked. | P1 `f67876e8e`; coordinator review: documentation governance and `git diff --check` passed. |
| S3 | Public FPGA implementation provenance review; it is MAME-derived/GPL and supplies no independent Video/Sprite contract. | P1 `ec71c2227`; coordinator review: documentation governance and `git diff --check` passed. |
| S4 | Owner-authorized L2 sprite-flags latch from reference-observed behavior; no imported implementation or claimed decode/render effect. | P1 `2276bbe2c`; selected-set x64/x86 both advance past the prior flags stop; complete repository units pass 669/669 on x64 and x86. |
| S5 | Selected-set-observed `0x696--0x697` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `bc67b7bbc`; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s5-selected-control-hole-l2.md). |
| S6 | Selected-set-observed `0x69c--0x69d` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `e317a10b8` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s6-selected-control-hole-l2.md). |
| S7 | Selected-set-observed `0x692--0x693` Board-local write-only L2 hole, independently bounded from Video/Sprite state. | P1 `f4ebdcf4f` accepted by coordinator review; x64/x86 Board and selected-set probes pass; post-change repository-only units pass 642/642 per width; [L2 evidence](../etc/evidence/t4-s7-selected-control-hole-l2.md). |
| S8 | Exact selected-set-observed `0x69a--0x69b` Board-local write-only L2 hole. | P1 `fa34720df` accepted by coordinator review; x64/x86 focused Board and selected-set probes advance to the independent next stop `0x690`; [L2 evidence](../etc/evidence/t4-s8-selected-control-hole-l2.md). |
| S9 | Exact selected-set-observed `0x690--0x691` Board-local write-only L2 hole. | P1 `b576fcae5` accepted by coordinator review; x64/x86 focused Board and selected-set probes reach the separate COP-owned `INVALID_STATE` boundary; [L2 evidence](../etc/evidence/t4-s9-selected-control-hole-l2.md). |
| S10 | Bounded Board-owned COP failure attribution for the selected set. | P1 `6a79f1102`; x64/x86 probes both identify command `0x5205`, first failed DMA address `0x6a0`, and existing `INVALID_STATE`; complete repository-only units pass 642/642 on each width. [Evidence](../etc/evidence/t4-s10-cop-failure-attribution.md). |
| S11 | L2 sprite-protection upload aperture from independently observed reference address ownership. | P1 `a9c048fd9`; x64/x86 focused Board and selected-set probes reach the separate Video/CRTC receiver at `0x600`; complete repository units pass 642/642 per width. [Evidence](../etc/evidence/t4-s11-sprite-protection-upload-l2.md). |
| S12 | L2 Video-owned CRTC register aperture from independently observed reference address ownership. | P1 `9042cdd35`; x64/x86 Board and selected-set probes advance from `0x600` to `0x640`; complete repository units pass 642/642 per width. [Evidence](../etc/evidence/t4-s12-crtc-register-aperture-l2.md). |
| S13 | L2 CRTC write-only extension at the exact next selected-set stop. | P1 `22d47c4b0`; P2 `24546816b`; x64/x86 focused Board tests and selected-set probes advance from `0x640` to the next independent stop `0x658`; complete repository-only units pass 669/669 on each width. [Evidence](../etc/evidence/t4-s13-crtc-write-extension-l2.md). |
| S14 | Bounded attribution of the selected set's first unowned control-window access at `0x658`. | P1 `f8fc5f867` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes identify the same one-byte write while preserving `INVALID_STATE`; all 642 repository unit cases pass per width when the known temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s14-control-fallback-attribution.md). |
| S15 | Exact selected-set word-write receiver at `0x658--0x659`, separately owned from CRTC. | P1 `0392b97e2` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only the complete word span, then reach the same independent next byte-write stop at `0x65a`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s15-object-write-sink-l2.md). |
| S16 | Exact selected-set word-write receiver at `0x65a--0x65b`, separately owned from CRTC and S15. | P1 `84f3023fb` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only either exact word span, reject the cross-pair without RAM fallback, then reach the same independent next byte-write stop at `0x65c`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s16-object-write-sink-l2.md). |
| S17 | Exact selected-set word-write receiver at `0x65c--0x65d`, separately owned from CRTC and S15--S16. | P1 `a815a6a0a` accepted by coordinator review: x64/x86 focused Board tests and selected-set probes accept only the three exact word pairs, reject cross-pairs without RAM fallback, then reach the same independent next byte-write stop at `0x65e`; all 642 repository unit cases pass per width when the temporary-output-sensitive DAG negative case is executed separately from the other 641 cases. [Evidence](../etc/evidence/t4-s17-object-write-sink-l2.md). |
| S18 | Source-bounded L2 replacement of the incremental pairs with the complete unowned object write aperture `0x650--0x69f`. | P1 `cdd761ac0`; x64/x86 Board smoke and selected-set probes pass; complete repository unit suites pass 642/642 per width when the temporary-output-sensitive DAG negative case is executed separately. [Evidence](../etc/evidence/t4-s18-object-aperture-l2.md). |
| S19 | Selected-set-observed tile-bank-2 Board register at `0x470--0x471`. | P1 `7258833b8`; x64/x86 Board smoke passes, both selected-set probes advance to `0x6cc`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s19-tile-bank-2-l2.md). |
| S20 | Selected-set-observed tile-bank-01 latch at `0x6cc` plus adjacent `0x6cd` no-op. | P1 `229b564bb`; P3 `8ad61e876` corrects the overwide latch. x64/x86 Board smoke passes, both selected-set probes advance to `0x6c4`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s20-tile-bank-01-l2.md). |
| S21 | Selected-set-observed sprite-protection constant write span at `0x6c4--0x6c5`. | P1 `448691874`; x64/x86 Board smoke passes, both selected-set probes advance to `0x6c0`, and the full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s21-sprite-protection-constant-l2.md). |
| S22 | Bounded sprite-protection register group at `0x6c0--0x6c3` and `0x6c6--0x6c7`. | P1 `6d0246f46`; x64/x86 Board smokes and selected-set probes pass; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s22-sprite-protection-registers-l2.md). |
| S23 | Attribute the selected-set's rejected COP DMA trigger before considering any new DMA mode. | P1 `bcbaeeeac`; x64/x86 probes identify rejected raw mode `0x0015` at `0x6fc`; final complete units pass 642/642 per width. [Evidence](../etc/evidence/t4-s23-cop-dma-trigger-attribution.md). |
| S24 | Record the selected DMA context in the existing COP failure observation. | P1 `bf3e3f396`; x64/x86 probes identify source `0x07c0`, size `0x00ff` and destination `0x0000` for rejected mode `0x0015`; final complete units pass 642/642 per width. [Evidence](../etc/evidence/t4-s24-cop-dma-context-attribution.md). |
| S25 | L2 fixed mode-`0x0015` palette receiver from permitted research and the bounded owner-set descriptor. | P1 `f96557a77`; x64/x86 probes advance to separate mode `0x0014`; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s25-palette-dma-l2.md). |
| S26 | Capture the selected mode-`0x0014` relative-address setup in the existing COP failure observation. | P1 `0c692e3b5`; x64/x86 probes identify `relative=0`; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s26-tilemap-dma-context-attribution.md). |
| S27 | L2 fixed mode-`0x0014` tilemap receiver from its selected bounded descriptor. | P1 `63b7c68bc`; x64/x86 Board smokes and owner-set probes advance to an independent Board-control stop; complete repository unit suites pass 642/642 per width. [Evidence](../etc/evidence/t4-s27-tilemap-dma-l2.md). |
| S28 | L2 exact selected-set `0x6ca` write-only no-op. | P1 `d62e0deee`; P2 `7efca76c0`; x64/x86 Board smoke and selected-set probes advance to `0x6d8`; full 642-case unit qualification passes per width. [Evidence](../etc/evidence/t4-s28-control-noop-l2.md). |
| S29 | L2 Sprite-protection X/Y/max-X/source endpoint group. | P1 `6ebe6cd9c`; x64/x86 Board smoke and selected-set probes pass the former `0x6d8` stop and reach `0x762`; full 668-case qualification and the separately run dependency-DAG negative pass per width. [Evidence](../etc/evidence/t4-s29-sprite-protection-latches-l2.md). |
| S30 | L2 Sprite-protection destination read endpoint. | P1 `532ef5a8f`; x64/x86 Board smoke and selected-set probes pass the former `0x762` stop; probes end only at their existing containment budget, and full 668-case qualification plus the separately run dependency-DAG negative pass per width. [Evidence](../etc/evidence/t4-s30-sprite-protection-destination-output-l2.md). |
| S31 | Fixed-role graphics image ingress, lifetime and creation validation. | P1 `197ec9b3c`; x64/x86 Board smoke and selected-set startup probes pass. [Evidence](../etc/evidence/t4-s31-graphics-image-ingress.md). |
| S32 | Bounded Video-owned RGB555 to `0x00RRGGBB` palette lookup. | P1 `822f76ba5`; x64/x86 Board smoke and selected-set startup probes pass. [Evidence](../etc/evidence/t4-s32-palette-rgb555-l2.md). |
| S33 | Recorded the current-MAME verifier limitation without source import or retained media output. | P1 `9b179c83d`; current MAME rejects the supplied archive for PAL blobs. [Evidence](../etc/evidence/t4-s33-black-box-oracle-disposition.md). |
| S34 | Corrected the black-box baseline with historical official MAME; no PAL/ROM was acquired. | P1 `c895eaca0`; the bounded historical run executes the supplied archive with the absent PAL marked no-good-dump. [Evidence](../etc/evidence/t4-s34-historical-mame-oracle.md). |
| S35 | Connected owner-approved external PAL BYOB inputs to current-MAME verification. | P1 `2ea80ac66`; both hashes match and current MAME verifies/runs the combined inputs. [Evidence](../etc/evidence/t4-s35-external-pal-byob.md). |
| S36 | Established a repeatable, neutral current-MAME screen checkpoint. | P1 `ded2d0a81`; two bounded runs agree on 320×240 at frame 600 without retained pixel output. [Evidence](../etc/evidence/t4-s36-mame-screen-checkpoint.md). |
| S41 | Owner-authorized L2 character/background graphics decoder primitive. | P1 `b15bf1768`; x64/x86 focused and MyRaidenII unit groups pass. Both full 643-case repository unit selections complete; the controlled junction-only cleanup case requires the authorized execution context and passes separately on both widths. [Evidence](../etc/evidence/t4-s41-l2-graphics-layout.md). |

## Archived Packet — M1 T4 S30

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S30 follows accepted S29 P2 `6e1b1b5ee`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continuing MyRaidenII and L2 models derived from external emulator research without source import; this packet changes only MyRaidenII. |
| Objective | Receive exactly the selected program's read pair `0x762--0x763` as a Board-owned L2 Sprite-protection destination-output endpoint, so it no longer falls through the unmapped control window. |
| Non-goals | No Sprite-protection algorithm, decryption, object transfer, new output state, write access, timing claim, rendering/frame effect, Shared/public API, ROM inspection, asset change or playable-game claim. |
| Reference Baseline | S29's dual-width probes next stop at a one-word read `0x762`. Permitted L2 map research identifies the pair as a Sprite-protection destination output. Existing Board route `0x6c6--0x6c7` is the sole private destination latch in the same bounded mechanism. |
| Candidate Proposal | Add one exact read-only replacement route that returns the existing reset-zero destination latch; reuse its current byte/word lane callbacks, retain the `0x6c6` write-only ingress and add no storage or helper. This is a declared L2 mirror, not a claim that all protection semantics are modeled. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, existing selected-set startup probe, CURRENT and one L2 evidence record. No Shared/public header, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The model is L2 only; one Board state owner, `lib_bool`/LIB_NULL/lib_status and inward dependencies remain required. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact pair reads the existing destination latch through both byte lanes; writes remain rejected; reset returns zero; both probes pass the former `0x762` boundary without a widened fallback. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient program-lane staging only, no content dump, raw trace or screenshot retention. |
| Reporting Requirements | Record L2 basis/limit, single-owner/no-second-state proof, exact access contract, verification result, diff size, next stop and remaining non-goals. |
| Stop Conditions | Any need to infer a protection algorithm, attach a Sprite/Video consumer, add new mutable state or Shared API, accept write access, inspect protected ROM content, diverge by width or retain media. |
| Exit Criteria | One reviewed MyRaidenII P with exact endpoint proof/evidence, dual-width qualification and cleanup; otherwise transfer/refuse the whole endpoint without widening a fallback. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check all existing Sprite-protection queries and routes, byte/word splitting, read/write directions and reset so the new read endpoint neither adds state nor changes existing route ownership. |

## Archived Packet — M1 T4 S29

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S29 follows accepted S28 P3 `f871e1185`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner authorized continuing MyRaidenII and standing L2 models using external emulator research while source-policy restrictions remain in force; this packet changes only MyRaidenII. |
| Objective | Receive the complete selected Sprite-protection endpoint group `0x6d8--0x6df` with its map-established byte/word access directions, so the selected program can pass its current `0x6d8` write without a generic control fallback. |
| Non-goals | No sprite decryption, object transfer, frame/render effect, protection algorithm, register readback beyond the mapped max-X pair, timing claim, Shared/public API, ROM inspection, asset change or playable-game claim. |
| Reference Baseline | S28's dual-width selected-set probes stop at one-byte write `0x6d8`. Permitted L2 map research identifies pairs `0x6d8/0x6da/0x6dc/0x6de` as Sprite-protection X, Y, max-X and source respectively; only max-X is mapped readable. |
| Candidate Proposal | Extend the existing private Board Sprite-protection latch owner with four reset-zero `lib_u16` pairs. Preserve existing Core byte splitting and little-endian lanes; X, Y and source are write-only, max-X is readable/writeable. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, existing selected-set startup probe, CURRENT and one L2 evidence record. No Shared/public header, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The model is L2 only; `lib_bool`/LIB_NULL/lib_status and inward dependencies remain required. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | All four pairs accept contained byte/word writes; only max-X read/inspect succeeds; reset clears readable max-X; both probes advance beyond `0x6d8` with no retained protected payload. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient program-lane staging only, no content dump, raw trace or screenshot retention. |
| Reporting Requirements | Record source grade, exact access contract, local state/no-effect boundary, verification result, diff size, next stop and remaining non-goals. |
| Stop Conditions | Any need to infer protection/render/decryption semantics, add a generic register framework or Shared API, accept a different access contract, inspect protected ROM content, diverge by width or retain media. |
| Exit Criteria | One reviewed MyRaidenII P with exact endpoint proof/evidence, dual-width qualification and cleanup; otherwise transfer/refuse the whole group without widening a fallback. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check existing Sprite-protection latch fields, byte/word lane handling, query/read/write directions, reset and nearby `0x6ce` flags/`0x6fc` DMA routes so no duplicate state owner or widened control path remains. |

## Archived Packet — M1 T4 S28

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S28 follows accepted S27 P2 `58a038ff4`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner's standing L2 authorization permits this bounded address-owner model; the source policy continues to forbid importing code, tables, algorithms or protected media. |
| Objective | Receive only the selected program's exact one-byte write at `0x6ca` as a write-only no-op. |
| Non-goals | No `0x6cb` ROM bank, readback, retained register state, generic control window, tile/sprite semantics, Shared change, ROM inspection or playable-game claim. |
| Reference Baseline | S27's x64/x86 probes agree on a byte write at `0x6ca`. Permitted L2 map research distinguishes that byte as a no-op from the adjacent `0x6cb` Raiden II program-ROM bank endpoint. |
| Candidate Proposal | Add one exact Board replacement route that reuses the existing selected-control no-op owner; read and inspect remain refused. A wider access is left to the existing Core byte splitter, whose adjacent `0x6cb` lane already has its independent bank owner. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, startup probe, CURRENT and one evidence record. No Shared/public API, media, asset or product artifact change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address model is L2 only. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | `0x6ca` write succeeds without state/readback; adjacent `0x6cb` remains owned by its existing bank route; both probes advance to the next independent stop. |
| Stop Conditions | Any need to combine the byte with `0x6cb`, retain behavior, add a generic route, change Shared, inspect ROM payload or diverge by width. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the boundary cannot stay exact. |
| Original Owner Request | Continue MyRaidenII under permitted L2 evidence without fabricating a full hardware model. |
| Similar-Issue Sweep | Check existing no-op route boundaries, wider-write splitting, sprite-object overlap and reset so this byte cannot silently widen a neighboring owner. |

S28 is closed. The Board receives only the exact selected one-byte `0x6ca`
write as a state-free, write-only no-op. The existing Core byte splitter keeps
the adjacent `0x6cb` bank route independent. The old unsupported-DMA Board
smoke row now uses still-unimplemented `0x0016`: S27 made `0x0014` a legal
tilemap mode, so retaining it as the failure row would test stale behavior.
The dual-width focused Board smokes, selected-set probes and complete 642-case
unit qualifications pass; the dependency-DAG negative case is executed
separately because its probe creates an isolated temporary tree. No Shared
source/API, ROM/media payload or product artifact changed.

## Active Packet — M1 T4 S23

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S23 follows accepted S22 P2 `c39d84c04`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded diagnostic and L2 work under the MyRaidenII source policy. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Record the existing COP DMA trigger rejection through the existing bounded failure observation, including its raw mode, so the selected-set probe can attribute `0x6fc` without guessing a receiver. |
| Non-goals | No DMA mode acceptance, Video/Palette implementation, new public API, new failure type, generic trace system, Shared change, ROM inspection, or playable-game claim. |
| Reference Baseline | S22's x64/x86 probe reaches a terminal CPU fault at `0x6fc`; `cop.c` already identifies that exact pair as the DMA trigger and rejects unsupported raw modes. The existing failure summary only records failures after an accepted pending operation. |
| Candidate Proposal | Reuse the existing private COP failure observation: before returning the existing invalid-state result for an unsupported DMA mode, fill its existing operation/value/physical/status fields once. Remove Board's redundant snapshot so its existing getter reads the COP owner directly. Existing reset clears it; the probe already prints the existing status fields. |
| Files And ABI Surface | MyRaidenII `core/cop.c`, `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No Shared component, public interface, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Attribution is not a DMA implementation or a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Unsupported mode leaves no pending operation, yields existing invalid-state behavior, records only existing bounded fields, clears on reset and produces identical x64/x86 probe attribution. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record the raw mode, source boundary, unchanged refusal behavior, next ownership decision and remaining non-goals. |
| Stop Conditions | The selected trigger is not the documented DMA trigger, the diagnostic requires a new public/shared contract, host widths diverge, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded attribution fails. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check every existing early DMA-trigger refusal, accepted DMA failure, pending-state guard and reset path so attribution does not overwrite an earlier failure or change acceptance. |

S23 is closed. COP is the sole owner of its bounded failure observation, and
Board's existing getter now reads that owner directly. Both selected-set probes
report the same rejected DMA mode `0x0015` at `0x6fc`, while all accepted modes
and the unimplemented Video-DMA modes retain their pre-existing behavior. The
complete x64/x86 unit qualification passes 642/642 per width. No Shared
component, public API, ROM/media payload, product artifact or playable-game
claim changed.

## Active Packet — M1 T4 S24

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S24 follows S23 P3 `282ba4a29`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded diagnostic work under the MyRaidenII source policy. No Shared component, external code, protected payload or mode implementation is admitted. |
| Objective | Capture the already-selected DMA context's source, size and destination when the existing trigger rejection occurs, so the selected-set probe can establish a finite descriptor without violating the write-only context contract. |
| Non-goals | No mode acceptance, DMA transfer, Video/Palette implementation, register readback, generic trace, new function/callback, Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S23 identifies raw mode `0x0015` at `0x6fc`; its context registers `0x478--0x47d` are intentionally write-only and cannot lawfully be read back through the Board bus. |
| Candidate Proposal | Extend the existing bounded COP failure observation with the three reset-zero `lib_u16` values from the already-selected private DMA context. Populate them only with the first rejected DMA trigger alongside its existing fields; reset clears all fields. The existing Board getter and selected-set probe consume that observation without adding a second diagnostic route. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, existing Board smoke, existing selected-set probe, CURRENT and one evidence record. The existing diagnostic record gains three fields; no Shared component, new function, callback, platform code, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. This is a bounded runtime observation, not a source of hardware timing or palette semantics. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Rejected mode preserves existing invalid-state behavior and no pending operation; observation contains the exact selected descriptor on both widths; reset clears it; the write-only bus reads remain refused. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient staging only, with no retained payload, raw trace or screenshot. |
| Reporting Requirements | Record descriptor values, source boundary, reset/failure behavior, dual-width probe result and remaining mode-15/Video non-goals. |
| Stop Conditions | Capturing the context requires exposing a write-only bus read, a new generic trace/API, host widths diverge, protected data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if bounded observation cannot be made. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check each existing DMA-trigger refusal, reset, accepted DMA advance failure and Board diagnostic getter so one first-failure owner remains. |

S24 is closed. The existing COP-owned failure record now supplies the selected
DMA context at an immediate rejection without creating bus readback or a second
diagnostic path. Both owner-set probes agree on mode `0x0015`, source `0x07c0`,
size `0x00ff` and destination `0x0000`; those values do not establish palette
semantics or authorize DMA acceptance. The complete x64/x86 unit qualification
passes 642/642 per width. No Shared component, product artifact, ROM/media
payload or playable-game claim changed.

## Active Packet — M1 T4 S25

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S25 follows S24 P2 `6e322a604`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner permits an L2 model informed by other emulators where L3 material is unavailable. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Admit only the selected mode-`0x0015` fixed palette DMA receiver, using the S24 descriptor as its exact eligibility gate. |
| Non-goals | No RGB conversion, renderer/frame, tile/sprite DMA, brightness mode, generic DMA framework, new Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S24 observes mode `0x0015`, source `0x07c0`, size `0x00ff`, destination `0`; permitted L2 research identifies the corresponding Raiden II palette aperture as 2048 16-bit entries at `0x1f000--0x1ffff`. |
| Candidate Proposal | Board constructs the existing private Video object before COP. COP accepts exactly that descriptor at its existing safe point, reads 2048 words through the existing checked Core-memory owner, then atomically passes those raw words to Video. Video retains reset-zero palette words only. Any descriptor or source-read failure retains the old palette. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, `core/video.h`, `core/video.c`, `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No Shared component or external-code import. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The palette receiver is L2 and does not claim original board timing or colour interpretation. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact descriptor accepts and publishes all 2048 words atomically; altered source/size/destination refuses without publication; source failure preserves the prior palette; reset clears it; both selected-set probes advance past the former rejection. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload, raw trace or screenshot. |
| Reporting Requirements | Record the precise L2 eligibility contract, the source-policy boundary, failure/reset semantics, next probe stop and dual-width proof. |
| Stop Conditions | The implementation requires a generic DMA/Video API, source readback, a Shared change, protected-data exposure, host-width divergence or an unbounded receiver. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the finite receiver cannot be made at the stated owners. |
| Similar-Issue Sweep | Check existing DMA source failure, pending clearing, reset, supported modes and every mode-`0x0015` descriptor field so no alternate path accepts the palette receiver. |

S25 is closed.  The precise selected descriptor now reaches a private,
reset-zero 2048-word Video palette through the existing COP safe point and
checked Core memory path.  It has no RGB decoder, renderer, public readback or
general DMA rule.  Altered descriptor fields and partial Video input leave the
previous palette intact.  The owner-set probes agree on the next separate
mode-`0x0014` rejection; it is not admitted by this receiver.  Complete
repository unit suites pass 642/642 on each width.  No Shared component,
product artifact, ROM/media payload or playable-game claim changed.

## Active Packet — M1 T4 S26

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S26 follows S25 P2 `f6269721e`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner's standing approval permits bounded diagnostics and L2 work under the MyRaidenII source policy. No mode implementation, Shared change or protected-data capture is admitted. |
| Objective | Capture the selected rejected mode-`0x0014` DMA context's existing relative-address word alongside the S24 descriptor fields. |
| Non-goals | No tilemap transfer, destination interpretation, renderer, generic trace, new function/callback, Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S25 advances to mode `0x0014` with source `0x033f`, size `0x027b` and destination `0xfffe`; existing register `0x476` is part of the selected DMA configuration but has not been retained in the failure observation. |
| Candidate Proposal | Add one reset-zero `lib_u16` field to the existing COP-owned first-failure record. Populate it only at the existing immediate rejected-DMA trigger from the already-selected `0x476` configuration word. Existing Board getter and startup probe consume it. |
| Files And ABI Surface | MyRaidenII `core/cop.h`, `core/cop.c`, existing Board smoke, existing startup probe, CURRENT and one evidence record. The existing internal diagnostic record gains one field; no new function, callback, Shared component, ROM or asset change. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Rejected mode remains rejected/no-pending; the captured relative word agrees across widths and reset clears it; the existing mode-`0x0015` palette route remains unchanged. |
| Stop Conditions | Capturing it needs bus readback, a generic trace/API, a Shared change, protected-data exposure or host-width divergence. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if a bounded observation cannot be made. |
| Similar-Issue Sweep | Check each immediate unsupported-DMA rejection, accepted-DMA failure and reset path so the first-failure record remains single-owner and no rejected request gains a pending operation. |

S26 is closed.  The existing COP-owned record now captures the reset-zero
relative-address word at rejected DMA admission without adding readback or a
second diagnostic route.  The selected descriptor has `relative=0` on both
host widths and remains rejected with no pending operation.  No mode behavior,
Shared component, product artifact, ROM/media payload or playable-game claim
changed.

## Active Packet — M1 T4 S27

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S27 follows S26 P2 `5fbf826b9`; one session performs coordinator and executor roles. |
| Admission And Approval | The owner permits an L2 model based on bounded selected-set observation and permitted external-emulator research. The source policy still forbids importing code, tables, algorithms or protected media. |
| Objective | Admit only the selected mode-`0x0014` fixed private tilemap DMA receiver. |
| Non-goals | No tile format/decode, scroll/bank behavior, renderer/frame, destination generalization, generic DMA framework, new Shared API, ROM inspection, artifact or playable-game claim. |
| Reference Baseline | S26 fixes the descriptor at `source=0x033f`, `size=0x027b`, `destination=0xfffe`, `relative=0`; permitted L2 research identifies a private 0x2800-byte tilemap transfer for mode `0x0014`. |
| Candidate Proposal | COP accepts exactly that descriptor at its existing safe point, reads 0x1400 words at `0x0cfc0` through the existing checked Core-memory owner, then atomically passes them to Video. Video retains reset-zero raw tilemap words only. |
| Files And ABI Surface | MyRaidenII `core/cop.c`, `core/video.h`, `core/video.c`, existing Board smoke, startup probe, CURRENT and one evidence record. No Shared component, new generic interface, ROM or asset change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. This is L2 and does not claim original tile/pixel semantics or timing. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | The exact descriptor atomically publishes 0x1400 words; altered fields and partial Video input retain prior data; reset clears it; both probes advance past the prior mode-`0x0014` refusal. |
| Stop Conditions | The implementation needs a generic DMA/Video API, source readback, Shared change, protected-data exposure, host-width divergence or an unbounded receiver. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer if the finite receiver cannot be made at the stated owners. |
| Similar-Issue Sweep | Check the existing palette path, mode selection, source failure, reset and pending clearing so neither exact receiver becomes a generic DMA fallback. |

S27 is closed. Its exact raw receiver retains one trigger-time relative value
so a later register write cannot alter an accepted pending transfer, and uses
the existing checked memory route for all source words before one Video commit.
Both widths advance through mode `0x0014` to a new independent Board-control
write at `0x6ca`; complete repository unit qualification passes 642/642 on
each width, documentation governance passes, and transient probe staging was
removed. No decode, frame or generic DMA behavior was admitted.

## Active Packet — M1 T4 S22

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S22 follows S21 P2 `e007e826d`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where physical L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Establish complete Board ownership for the adjacent sprite-protection register group: reset-zero read/write word latches at `0x6c0--0x6c1` and `0x6c2--0x6c3`, plus a reset-zero write-only word latch at `0x6c6--0x6c7`. |
| Non-goals | No protection algorithm, sprite decryption, destination interpretation, frame effect, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S21's x64/x86 probes stop on a byte write at `0x6c0`. The selected Raiden II inherited map gives named read/write ownership to `0x6c0--0x6c3`, write-only ownership to `0x6c6--0x6c7`, and independently assigns the intervening `0x6c4--0x6c5` no-op to S21. |
| Candidate Proposal | Add three exact Board routes and three `lib_u16` reset-zero latches. Byte and contained word lanes use explicit little-endian handling. Readable latches expose stored bytes; the destination latch rejects read/inspect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership and local latching are L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Read/write latch lane round-trips and reset are verified; destination writes and reads fail as mapped; S21 no-op remains independent; a wider access keeps Core's established byte-route splitting; both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record the endpoint group, L2 limits, lane/reset behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Required protection behavior beyond latching, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test every group endpoint, byte/word lanes, readable versus write-only distinction, reset, adjacent S21 no-op, wider accesses through Core's byte-route split and duplicate owner paths. |

## Retained Packet — M1 T4 S21

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S21 follows S20 P4 `91c97c436`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed `0x6c4--0x6c5` sprite-protection constant write span as a stateless Board-owned no-op. |
| Non-goals | No readback, constant interpretation, sprite-protection implementation, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S20's x64/x86 probes stop on a one-byte write at `0x6c4`. The selected Raiden II inherited map marks `0x6c4--0x6c5` write-only and states that a constant is written alongside the separate `0x6c0` receiver. |
| Candidate Proposal | Add the exact two-byte span to the existing Board no-op owner. It accepts contained writes only, retains no state and leaves reads/inspection on the existing refusal path. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Contained byte and word writes are accepted, reads/inspection remain refused, a crossing write is rejected, and both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, stateless behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Required protection behavior, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Check only the exact pair, byte/word containment, read/inspect refusal, cross-pair behavior and duplicate owner paths. |

S21 is closed.  The stateless constant sink preserves the explicit boundary
between it and the `0x6c0--0x6c3` sprite-protection read/write group.  Both
probes now stop at the same `0x6c0` write; that stateful group remains a
separate ownership obligation.  No Shared component, public API, ROM/media
payload, desktop/RDP validation, or playable-game claim changed.

## Retained Packet — M1 T4 S20

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S20 follows S19 P2 `ee5a5c514`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external address-map evidence where L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed two-byte tile-bank-01 write endpoint at `0x6cc--0x6cd` as reset-zero Board state. |
| Non-goals | No readback, renderer, layer selection, palette, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S19's x64/x86 probes first stop on a one-byte write at `0x6cc`; the temporary bounded receiver advances both to a one-byte `0x6cd` write. The selected Raiden II reference map assigns the write-only tile-bank-01 endpoint over `0x6cc--0x6cd`; S28 already records this range as a distinct established owner. |
| Candidate Proposal | Add one Board-owned two-byte replacement route, one `lib_u16` reset-zero latch and no read route. The latch retains both admitted byte lanes for a later evidenced Video consumer but has no current frame effect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership and retention are L2 behavior, not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Byte writes at both lanes and a complete word write succeed, reads/inspection remain refused, nearby `0x6ca` and `0x6ce` retain their existing refusal, and both probes report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, reset/write-only behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Need for tile/render semantics, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test the exact byte ownership, read/inspect refusal, adjacent fallback ownership and reset lifecycle; verify no second tile-bank-01 or generic register path exists. |

S20 P2's combined-latch conclusion was reopened by the map recheck.  S20 P3
narrows `0x6cc` to its actual tile-bank latch and retains `0x6cd` as the
separate mapped no-op; the repeated complete dual-width qualification passes.
S20 is closed.  Neither endpoint has readback or a frame effect.  The next
stop remains the unowned `0x6c4` write.  No Shared component, public API,
ROM/media payload, desktop/RDP validation, or playable-game claim changed.

## Retained Packet — M1 T4 S19

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S19 follows coordinator-accepted S18 P1 `cdd761ac0`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's standing approval permits bounded L2 behavior from external emulator evidence where physical L3 material is unavailable; source policy forbids importing code, tables, algorithms, or protected media. |
| Objective | Model the selected-set-observed Board tile-bank-2 endpoint at `0x470--0x471` as one reset-zero, byte-addressable 16-bit local register. |
| Non-goals | No tile renderer, layer selection, palette, CRTC change, generic register framework, public API, Shared change, reference implementation translation, ROM inspection, or playable-game claim. |
| Reference Baseline | S18 probes reach a one-byte Board read at `0x470` on both widths. The selected Raiden II reference map declares a read/write `0x470--0x471` tile-bank endpoint; earlier internal B07 inventory assigns tile-bank behavior to Board/Video, not COP. |
| Candidate Proposal | Add one Board-owned two-byte replacement route and one `lib_u16` reset-zero field. Reads, inspection and byte/word writes use explicit little-endian lane handling; it retains values locally only for this endpoint. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset, or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. L2 address/behavior evidence is not a hardware-accuracy claim. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe; complete repository dual-width unit suites; documentation governance and `git diff --check`. |
| Expected Markers | Initial read returns zero; byte lanes and complete word round-trip; reset clears value; neighbouring access remains refused; both probes pass `0x470` and report the same next terminal. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; transient startup staging only, with no retained payload or raw trace. |
| Reporting Requirements | Record L2 source boundary, exact lane/reset behavior, next probe stop, dual-width result and remaining non-goals. |
| Stop Conditions | Need for tile/render semantics, conflicting endpoint evidence, Shared/public API change, divergent host widths, protected-data exposure, or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Original Owner Request | Continue implementing MyRaidenII under the repository governance and permitted L2 source boundary. |
| Similar-Issue Sweep | Test full/byte lanes, initial/reset state, read/inspect, neighbours and control-failure ownership; verify no duplicate tile-bank or generic register path exists. |

S19 is closed.  The local L2 register accepts only `0x470--0x471`, preserves
its two byte lanes, and resets to zero without claiming a tile-rendering
effect.  Both selected-set probes advance to the identical next unowned Board
write at `0x6cc`; that endpoint remains a separate ownership obligation.
No Shared component, public API, ROM/media payload, desktop/RDP validation, or
playable-game claim changed.

## Retained Packet — M1 T4 S18

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S18 follows coordinator-accepted S17 P1 `a815a6a0a`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes behavioral L2 models from external emulator behavior when L3 is unavailable and explicitly asks to avoid inefficient one-address-at-a-time work. The source-policy boundary remains: no implementation, table, algorithm or ROM bytes are imported. |
| Objective | Replace S15--S17's incremental adjacent pairs with one stateless, write-only Board aperture at the complete currently unowned portion of the externally documented OBJ range: `0x650--0x69f`. |
| Reference Baseline | `42b5e6a54`; both widths reach the next byte write at `0x65e`. MAME's address map identifies `0x640--0x6bf` as an OBJ interface but leaves it commented; this repository already has CRTC ownership through `0x64f` and protection-upload ownership from `0x6a0` through `0x6bf`. Thus `0x650--0x69f` is the exact remaining contiguous aperture. |
| Candidate Proposal | One existing Board route covers `0x650--0x69f`; contained writes are accepted as stateless L2 ownership except mixed-width accesses that straddle an existing selected-control receiver. Reads and inspection remain refused, source bytes are not retained, and no object/decryption/rendering semantics are added. Existing CRTC, protection and selected-control receivers retain their ranges and Core's existing byte-splitting behavior. |
| Non-goals | No generic RAM, object register meanings, readback, renderer, decryption, palette behavior, physical claim, Shared change, external source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board/CRTC/protection tests, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Writes at both aperture bounds and representative former gaps are accepted; reads/inspection remain refused; CRTC/protection boundary tests continue to own their ranges; both probes pass the previous `0x65e` stop and report one identical next stop. |
| Stop Conditions | Required object behavior beyond write acceptance, evidence contradicting aperture ownership, Shared change, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Removed stale “unmodelled” expectations in the aperture, retained CRTC/protection tests as their own owners, tested lower/upper bounds/read/inspect/reset, and proved no object data persists. |

S18 is closed.  The source-bounded aperture advances both selected-set probes
past the prior `0x65e` stop to the same next unmodeled Board write at `0x470`.
That address remains a separate T4 ownership/evidence obligation.  No
playable-game, desktop/RDP, renderer, sound, or physical-hardware claim is
implied.

## Retained Packet — M1 T4 S17

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S17 follows coordinator-accepted S16 P1 `84f3023fb`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes a bounded behavioral L2 model from external emulator behavior when L3 material is unavailable. The exact two-byte span is inferred only from existing Core byte/word routing and the first post-S16 selected-set byte write; no external code, table or algorithm is imported. |
| Objective | Establish the smallest write-only receiver for the selected set's adjacent word span at `0x65c--0x65d`, separate from Video CRTC and prior object pairs. |
| Non-goals | No object register semantics, readback, generic RAM, rendering, decryption, palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `84f3023fb`; both widths accept the two prior word pairs and then report first unowned access as a one-byte write at `0x65c`. FBNeo's CRTC path ends at `0x64f`; MAME's commented OBJ aperture surrounds the area without defining contents or readback. |
| Candidate Proposal | Extend the existing Board-owned object write route over exactly the new adjacent pair and permit that pair only through the existing stateless callback. The route continues to prevent unsupported within-window accesses from falling through to ordinary RAM. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Each exact byte lane and word pair are accepted; reads, inspection, cross-pair writes and immediate neighbours remain refused; both probes advance past `0x65d` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | State L2 limitation; report exact accepted/rejected boundaries, dual-width result, next stop and no playable-game claim. |
| Stop Conditions | Need for access outside the observed span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Check `0x65c--0x65d`, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC or prior object pairs; no fallback widening or duplicate Video state. |

S17 is closed. The bounded L2 receiver owns only the three proven pairs and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65e`. M1 T4 remains open; further ownership needs a separately admitted
packet or a new bounded diagnostic strategy.

## Retained Packet — M1 T4 S16

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S16 follows coordinator-accepted S15 P1 `0392b97e2`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes a bounded behavioral L2 model from external emulator behavior when L3 material is unavailable. The exact two-byte span is inferred only from the existing Core byte-split word route and the first post-S15 selected-set byte write; no external code, table or algorithm is imported. |
| Objective | Establish the smallest write-only receiver for the selected set's next adjacent word span at `0x65a--0x65b`, separate from Video CRTC and S15 ownership. |
| Non-goals | No object register semantics, readback, generic RAM, rendering, decryption, palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `0392b97e2`; both widths accept `0x658--0x659` and then report the first unowned access as a one-byte write at `0x65a`. FBNeo's CRTC path ends at `0x64f`; MAME's commented OBJ aperture surrounds this region, without defining contents or readback. |
| Candidate Proposal | One stateless Board-owned two-byte write-only L2 sink at exact `0x65a--0x65b`. One Board route owns the four-byte evidence window so an unsupported cross-pair write cannot fall back to ordinary RAM; its write callback accepts only contained byte lanes and the complete word for either exact pair. Reads, inspection, cross-pair writes and neighbours remain refused; no getter or frame effect exists. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. Address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board route test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | Exact byte lanes and complete word are accepted; read/inspect, cross-pair writes and immediate neighbours remain refused; both probes advance past `0x65b` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; the probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | State L2 limitation; report exact accepted/rejected boundaries, dual-width result, next stop and no playable-game claim. |
| Stop Conditions | Need for access outside the observed span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host widths, protected-data exposure or cleanup failure. |
| Exit Criteria | Focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or refusal/transfer because bounded assumptions fail. |
| Similar-Issue Sweep | Check `0x65a--0x65b`, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC or S15; no fallback widening or duplicate Video state. |

S16 is closed. The bounded L2 receiver owns only the proven adjacent pair and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65c`. M1 T4 remains open; any receiver for that new stop requires a
separately admitted packet.

## Retained Packet — M1 T4 S15

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S15 follows coordinator-accepted S14 P1 `f8fc5f867`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorizes an external-emulator behavioral L2 model when physical L3 material is unavailable. This model uses only independently stated address ownership and the selected-set's bounded runtime access; no implementation/source/table import is permitted. |
| Objective | Establish the smallest write-only receiver for the exact selected-set word write at `0x658--0x659`, separate from Video CRTC ownership. |
| Non-goals | No full object/sprite interface, readback, generic RAM, rendering/decryption/palette behavior, physical-hardware claim, Shared change, source translation, protected-ROM inspection, raw trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `f8fc5f867`; both widths report first fallback write `0x658`, one byte. FBNeo routes only `0x600--0x64f` writes to its CRTC helper; MAME documents a commented OBJ aperture `0x640--0x6bf`. Neither source establishes object contents or readback. |
| Candidate Proposal | One stateless Board-owned two-byte, write-only L2 object sink at exact `0x658--0x659`. It accepts either contained byte lane or the contained word through the existing Core route, refuses reads/inspection/mixed widths/neighbours, exposes no getter and has no frame effect. |
| Files And ABI Surface | MyRaidenII `core/board.c`, existing Board smoke, selected-set integration probe, CURRENT and one evidence record. No public interface, Shared component, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address ownership is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test through the real Core route; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Expected Markers | The exact byte lanes and contained word are accepted; read/inspect, mixed accesses and immediate neighbours remain refused; both probes advance past `0x659` to the same independent next stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; the probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record runtime write fact and external ownership separately; state the L2 limitation; report exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Need for access beyond the observed two-byte word span, readback, algorithmic behavior, Shared change, contradictory runtime evidence, divergent host-width result, protected-data exposure or cleanup failure. |
| Exit Criteria | The exact receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or it is refused/transferred because the bounded assumptions fail. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check both `0x658--0x659` lanes, immediate neighbours, read/inspect, widths, reset and no overlap with CRTC; ensure no fallback widening or duplicate Video state. |

S15 is closed. The bounded L2 receiver accepts only the observed word span and
the selected-set probe's next independent unowned access is a one-byte write
at `0x65a`. M1 T4 remains open; that next stop requires a separately admitted
packet rather than widening this receiver.

## Retained Packet — M1 T4 S14

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S14 follows coordinator-accepted S13 P2 `24546816b`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner permits a bounded L2 model from external behavior when physical L3 material is unavailable, but requires it to be labeled L2 and not derived from external source implementation. This S admitted only neutral runtime attribution, not a receiver model. |
| Objective | Determine whether the selected set's exact next stop at `0x658` reaches the Board fallback as a read or write and with what byte width, without accepting or altering that access. |
| Non-goals | No receiver, physical control-hardware claim, fallback widening, generic RAM, new executor, Shared change, source translation, protected-ROM inspection, PC/register/payload capture, raw trace retention, desktop qualification or playable-game claim. |
| Candidate Proposal | Board keeps one reset-cleared first-failure observation containing only valid, physical address, byte width and Core access direction. Existing fallback callbacks record it once then return their unchanged `LIB_STATUS_INVALID_STATE`; they do not inspect source/destination data. |
| Verification | Focused x64/x86 Board test through the real Core memory route; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; documentation-governance and `git diff --check`. |
| Exit Criteria | The bounded observation has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or the access does not reach a lawful Board fallback and is transferred. |

## Retained Packet — M1 T4 S13

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S13 follows accepted S12 P1 `9042cdd35`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits an external-emulator behavioral L2 model when physical L3 material is absent. The owner additionally directs use of other emulator behavior as clearly labeled L2 when L3 is unavailable. It permits behavior only; no source, table, ROM, trace or algorithm import. |
| Objective | Establish the smallest Video-owned receiver for the selected set's exact next stop at `0x640`, preserving the existing CRTC state owner without treating the Video area as generic Board RAM. |
| Non-goals | No physical CRTC timing claim, rendering/decryption/palette implementation, generic control RAM, unproven extension readback, new executor, Shared/API change, source translation, protected-ROM inspection, trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `9042cdd35`; both selected-set probes advance through the S12 CRTC read/write aperture then fault at `0x640`. MAME establishes read/write ownership only through `0x63f`; an independent FBNeo driver routes CRTC writes through `0x64f`. The two sources do not establish extension read behavior. |
| Candidate Proposal | Extend the reset-cleared Video CRTC storage from 64 to 80 bytes. Board retains `0x600--0x63f` read/write behavior and accepts only writes at `0x640--0x64f`; reads/inspection in the extension and all accesses at immediate neighbours remain refused. |
| Files And ABI Surface | MyRaidenII `core/video.[ch]`, `core/board.c`, existing Board smoke, CURRENT and one evidence record. No Shared component, public cross-component ABI, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address/ownership fact is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; manifest, documentation-governance and `git diff --check`. |
| Expected Markers | The original aperture preserves read/write/reset behavior; the exact extension preserves byte write order/reset, refuses read/inspect and immediate neighbours, and both widths advance past the prior `0x640` stop to one same next independently owned boundary. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record local stop and external ownership separately; state the L2 limitation; report exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Required algorithmic behavior, non-byte width semantics that Core cannot lawfully split, contradictory reference/runtime evidence, Shared change, protected-data exposure, divergent host-width result or cleanup failure. |
| Exit Criteria | The bounded Video receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or a conflicting contract is refused and transferred. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check all `0x600--0x64f` bytes, surrounding `0x5ff`/`0x650`, original versus extension read ownership, reset, and ensure Board has no duplicate Video state or generic fallback acceptance. |

## Retained Packet — M1 T4 S12

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S12 follows accepted S11 P1 `a9c048fd9`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits an external-emulator behavioral L2 model when physical L3 material is absent. It permits behavior only; no source, table, ROM, trace or algorithm import. |
| Objective | Establish the smallest Video-owned receiver for the selected set's next exact stop at `0x600`, and prove bounded forward progress without treating the Video area as generic Board RAM. |
| Non-goals | No physical CRTC timing claim, rendering/decryption/palette implementation, generic control RAM, new executor, Shared/API change, source translation, protected-ROM inspection, trace retention, desktop qualification or playable-game claim. |
| Reference Baseline | `a9c048fd9`; both selected-set probes advance through the S11 COP upload aperture and fault at `0x600`. The pinned external reference map assigns `0x600--0x63f` as read/write CRTC ownership. |
| Candidate Proposal | One reset-cleared 64-byte Video CRTC register aperture. It preserves byte reads/writes only within `0x600--0x63f`; Board translates physical addresses to existing Video ownership and retains all neighbouring refusal. |
| Files And ABI Surface | MyRaidenII `core/video.[ch]`, `core/board.c`, existing Board smoke, CURRENT and one evidence record. No Shared component, public cross-component ABI, product, ROM, asset or external-code change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. The address/ownership fact is L2 behavioral context only. |
| Verification | Focused x64/x86 Board test; x64/x86 selected-set startup probe with transient staging/cleanup; complete repository dual-width unit suites after production change; manifest, documentation-governance and `git diff --check`. |
| Expected Markers | The aperture preserves byte order and reset, accepts only `0x600--0x63f`, rejects immediate neighbours, and both widths advance past the prior `0x600` stop to one same next independently owned boundary. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; probe stages only transient program lanes and retains no payload, screenshot or raw trace. |
| Reporting Requirements | Record local stop and external ownership separately; state the L2 limitation; report exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Required algorithmic behavior, non-byte width semantics that Core cannot lawfully split, contradictory reference/runtime evidence, Shared change, protected-data exposure, divergent host-width result or cleanup failure. |
| Exit Criteria | The bounded Video receiver has focused/full dual-width proof, evidence and one reviewed MyRaidenII P, or a conflicting contract is refused and transferred. |
| Original Owner Request | Do not require physical-level material; use other emulator observable behavior as clearly labeled L2 where L3 is unavailable. |
| Similar-Issue Sweep | Check all `0x600--0x63f` bytes, surrounding `0x5ff`/`0x640`, reset/readback ownership, and ensure Board has no duplicate Video state or generic fallback acceptance. |

## Retained Packet — M1 T4 S11

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S11 follows accepted S10 P1 `6a79f1102`; one session performs coordinator and executor roles. |
| Objective | Admit the smallest L2 Board receiver for the selected set's COP `0x5205` destination `0x6a0`, based on independently observed emulator address ownership, and prove bounded forward progress without importing external implementation. |
| Non-goals | No emulation of external sprite-protection algorithms, CRTC/Video behavior, generic control RAM, Shared/API change, protected ROM inspection, trace capture, playable-game claim, or physical-hardware-accuracy claim. |
| Candidate | One reset-cleared, write-only, Board-owned `0x6a0--0x6bf` sprite-protection upload aperture. It accepts only contained writes, rejects reads and inspection, and preserves no public readback contract. |
| Source Policy | MAME address ownership is behavioral L2 context only. No source code, tables, ROM content, payload, PC/register data or external algorithm is imported. |
| Verification | Focused x64/x86 Board smoke; x64/x86 selected-set startup probe with transient staged program lanes; complete repository dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The aperture's contained-write/read-refusal contract is covered; both widths advance beyond the prior COP stop to the same independent next receiver; reviewed evidence and one MyRaidenII P are committed/pushed. |
| Stop Conditions | The aperture needs readback, algorithmic behavior, an external source translation, a Shared change, ambiguous width semantics, divergent host-width result, protected-data exposure or staging cleanup failure. |

## Retained Packet — M1 T4 S10

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S10 follows accepted S9 P1 `b576fcae5`; one session performs coordinator and executor roles. |
| Objective | Attribute the selected-set COP `INVALID_STATE` to the existing pending operation and first failing Core DMA address without logging protected payload or inventing a receiver. |
| Non-goals | No COP algorithm/mapping acceptance, generic trace recorder, second execution path, Shared/Core API, ROM dump, full instruction trace, Board fallback route, or playable-game claim. |
| Candidate | One reset-cleared, Board-owned copy of the immediate COP failure observation; a read-only MyRaidenII Board getter for the existing bounded integration probe. COP records only operation discriminator, command/mode value, first failed bus address and existing status. |
| Files And ABI Surface | MyRaidenII Core private COP header/source, Board interface/source, existing Board unit and selected-set integration probe only. No Shared ABI change. |
| Source Policy | The observation is neutral runtime metadata under the existing owner-supplied opt-in probe. It must not print or retain ROM bytes, PCs, register dumps or raw trace. |
| Verification | Focused x64/x86 Core Board smoke and selected-set probe; complete dual-width repository unit suites after production change; documentation governance and `git diff --check`. |
| Exit Criteria | The probe emits a bounded, stable fault attribution on both widths, synthetic failure/reset coverage proves no stale diagnostic state, and one reviewed MyRaidenII P is committed/pushed. |
| Stop Conditions | Observation would require protected payload, ambiguous multiple failures, a Shared change, a new execution path, or contradictory dual-width result. |

## Retained Packet — M1 T4 S9

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S9 follows accepted S8 P1 `fa34720df`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x690`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `fa34720df`; after exact routes at `0x692`, `0x696`, `0x69a` and `0x69c`, both widths fault at `0x690`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S9 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x690`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S8

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S8 follows accepted S7 P1 `f4ebdcf4f`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x69a`, current reference-map-gap context and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `f4ebdcf4f`; after exact routes at `0x692`, `0x696` and `0x69c`, both widths fault at `0x69a`. The current reference map leaves the local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S8 evidence only if accepted. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x69a`; otherwise report retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S7

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S7 follows accepted S6 P1 `e317a10b8`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits bounded L2 behavioral models from other emulators when L3 material is unavailable. No external code, tables, ROM content or protected trace may be imported. |
| Objective | Use the exact selected-set stop at `0x692`, the current reference-map gap and actual forward-progress replay to admit only the smallest stateless Board operation that advances the existing selected program. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, Shared/Core API change, source translation, ROM retention, physical-hardware claim or playable-game claim. |
| Reference Baseline | `e317a10b8`; after the exact `0x69c--0x69d` route, both widths fault at `0x692`. The current reference map leaves its local `0x690--0x69f` interval unassigned between named receivers. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback only by the exact smallest route if the replay advances and local reads, inspection, mixed widths and neighbours remain rejected; otherwise retain/refine the refusal. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S7 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide; execution, architecture, coding, documentation and source-policy rules. External behavior is L2 context only. |
| Verification | x64/x86 focused Board test and selected-set probe with existing transient staging/cleanup; complete repository-only dual-width unit suites after a production change; documentation governance and `git diff --check`. |
| Expected Markers | Exact two-byte route accepts only byte lanes or contained word writes, rejects reads/inspection/mixed widths/neighbours, and advances both selected-set probes beyond `0x692`; otherwise report the retained stop. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record local stop and reference-map facts separately, L2 limitation, exact accepted/rejected boundaries, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Contradictory replay, stateful/read access, inaccessible exact width, required new owner/API, protected-data exposure or probe cleanup failure. |
| Exit Criteria | A bounded L2 disposition is complete: accepted Board-only route has focused/full dual-width proof and evidence, or refusal is retained/transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require the owner to supply physical-level detail when a bounded other-emulator L2 model can make correct progress. |
| Similar-Issue Sweep | Recheck existing selected write-only routes and every immediately adjacent byte for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S6

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S6 follows accepted S5 P1 `bc67b7bbc`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner's 2026-10-19 authorization permits a bounded L2 model from other emulators where L3 material is unavailable. It permits behavioral facts only, not source/code/table/ROM import. |
| Objective | Use the exact selected-set fault point at `0x69c`, the current reference-map gap and actual forward-progress replay to admit only the smallest state-free Board operation that makes the existing program advance. |
| Non-goals | No blanket `0x690--0x69f` acceptance, Video/Sprite/COP state, generic RAM, reference implementation translation, Shared/Core API change, ROM retention, or physical-hardware-accuracy claim. |
| Reference Baseline | `bc67b7bbc`; dual-width bounded probe passes `0x696` then faults at `0x69c`. Current MAME RAIDEN II map has a gap between its sprite-buffer trigger at `0x68e--0x68f` and next receiver at `0x6a0`, providing L2 map-disposition context only. |
| Candidate Proposal | Extend the existing selected-control write-only no-state callback by one exact two-byte route only if the selected-set replay advances and all local read/inspection/neighbour/boundary tests retain refusal; otherwise retain refusal and record/transfer the result. |
| Files And ABI Surface | `test/myraidenii/integration/selected_set_startup_probe.c` for bounded diagnostic evidence; if proven, `src/myraidenii/core/board.c`, its existing Board smoke, CURRENT and S6 evidence. No public API or ABI change. |
| Applicable Rules | MyRaidenII guide, execution, architecture, coding, documentation and source-policy rules. The precise reference fact is L2; local program behavior is a bounded external integration observation. |
| Verification | x64/x86 selected-set probe with existing transient staging/cleanup; focused Board x64/x86 test; complete repository-only unit suites per width after production change; documentation governance and `git diff --check`. |
| Expected Markers | Both widths stop at the same named address before the change; a state-free exact route accepts only its two byte lanes and retains reads, inspection, mixed widths and neighbours as errors; the probe advances beyond `0x69c` or the refusal is retained with a justified record. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only; existing probe stages only program lanes transiently and retains no payload, trace or screenshots. |
| Reporting Requirements | Record the selected-set fault fact separately from the reference-map fact, L2 limitation, exact accepted/rejected range, dual-width results, next stop and no playable-game claim. |
| Stop Conditions | Read/stateful access, access width not representable by existing callback contract, reference contradiction, required new owner/API, protected-data exposure, or failed staging cleanup. |
| Exit Criteria | A bounded L2 disposition resolves the operation; any accepted Board-only route has focused and full dual-width proof/evidence, or an incompatible operation is explicitly retained/refused and transferred. One reviewed MyRaidenII P is committed/pushed. |
| Original Owner Request | Do not require physical-level details from the owner; use other emulator observable behavior as an L2 reference when L3 material is absent. |
| Similar-Issue Sweep | Review each existing selected write-only route and the immediate `0x690--0x69f` neighbours for accidental widening, duplicate ownership, readback or generic-RAM fallback. |

## Retained Packet — M1 T4 S5

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M1 T4 S5 follows accepted S4 P1 `2276bbe2c`; one session performs coordinator and executor roles. |
| Admission And Approval | Owner authorized L2 behavioral models from other emulators when L3 material is unavailable on 2026-10-19; no external source import, translation, algorithm/table reuse or protected ROM retention is authorized. |
| Objective | Establish the exact selected-set-observed `0x696--0x697` V30 control write as a Board-local, write-only, no-state L2 hole, then prove that the real program advances past it on both host widths. |
| Non-goals | No Video/Sprite register, RAM, decoder, decryption, palette, renderer, Shared API, Core CPU change, generic control-window acceptance, or claim of original-hardware accuracy. |
| Reference Baseline | `2276bbe2c`; the bounded selected-set probe reaches a terminal fault with code `0x696` only after the completed `0x6ce--0x6cf` sprite-flags route. Current MAME RAIDEN II map is an L2 behavioral reference and does not assign this span a device receiver. |
| Candidate Proposal | Add one two-byte replacement route to the existing selected-control write-only hole owner; retain explicit rejection for all reads, inspection, adjacent bytes and wider mixed accesses. |
| Files And ABI Surface | `src/myraidenii/core/board.c`, its existing Board unit smoke, S5 evidence and CURRENT only. No public API or ABI changes. Expected production +5--10/-0--3; test +12--24/-0--4; refine after actual change review. |
| Applicable Rules | MyRaidenII guide, architecture/coding/source policy, execution and documentation rules; source policy permits L2 behavioral contracts but prohibits external implementation import. |
| Verification | Focused x64/x86 Board smoke; bounded selected-set probe on x64/x86 with transient staged lanes and cleanup; documentation governance, manifest if applicable and `git diff --check`; complete repository units per width before S closure. |
| Expected Markers | Synthetic actual Board writes accept only the exact two bytes and preserve no readback; selected set advances beyond `0x696`; all unrelated control addresses still reject. |
| Asset Needs | Existing owner-supplied `O:\assets\raidenii\raiden2.zip`, read-only and staged transiently by the existing probe; no bytes, screenshots or traces retained. |
| Reporting Requirements | Record exact route, L2 limitation, reference distinction, actual diff, synthetic/selected-set results, cleanup and any next stop without claiming playable output. |
| Stop Conditions | A read, stateful consumer, width other than the bounded word, contradictory reference behavior, required Shared/API change, protected-source exposure or probe cleanup failure. |
| Exit Criteria | Exact Board-only route, tests and evidence are complete; dual-width complete units pass; actual change review and one MyRaidenII P are committed/pushed. |
| Original Owner Request | When no physical-level evidence is available, use other simulators as L2 references rather than blocking progress; do not require the owner to supply more detailed materials. |
| Similar-Issue Sweep | Recheck the complete `0x400--0x7ff` control route table for accidental widening, duplicate device ownership, readback or generic-RAM fallback; classify every hit. |

M1 T3 remains open after accepted S1--S25:

| S | Delivered scope | Receipt |
| --- | --- | --- |
| S1 | B01--B09 board/COP contract and source-grade ledger | `e83f186a8`, `86c634958` |
| S2 | V30 construction, static program lanes, dynamic bank and board RAM/control routes | `dd2611e1b`, `9f941ae69`, `f2c30a1e6` |
| S3 | Main IRQ acknowledgement, active-low inputs and main/sound latches | `4d340ce41`, `d7b2c32b1` |
| S4 | Bounded owner-ROM COP/DMA address observation and receiving ledger | `da6d9d3a5`, `77110cf04` |
| S5 | Local COP configuration/register boundary and explicit unmodelled command failure | `e284fb386`, `5079476c4` |
| S6 | Bounded command/result/DMA receiver ledger and cleanup | `38e778ea8`, `e84ce8020` |
| S7 | COP evidence-grade decision and Video-owner transfer | `4cb7776f1` |
| S8 | Video-owned L2 raster/VBlank receiver, actual IRQ/reset proof and test build wiring | `34e22a100` |
| S9 | Finite initial COP upload/trigger inventory and first strict rejection | `456cde1fb` |
| S10 | Uploaded transfer family and approved Shared callback-fault publication repair; eight receiving releases | `f2ca9ae01`, `990ff6eda`, `af1649f60` |
| S11 | L2 position/velocity updates and common COP memory transport; explicit 2a05 non-eligible disposition | `b977bfa48` |
| S12 | Complete nine-ID geometry contract, result/config gaps and minimal Types review request; no code changes | `87a1ac547` |
| S13 | Complete four-ID L2 collision/result receiver, bus/failure/latch/reset proof and S12 status-prefix correction | `b3098766d` |
| S14 | Complete decimal register family, byte/mode/reset/guest proof, exact old failure-stage preservation and safe pointer offsets | `c28a162d5` |
| S15 | Complete COP-owned register inventory, explicit unmodelled routes and shared stored DMA-mode readback | `fb5e14f7b` |
| S16 | Complete low-control operational refusal, preserved valid/mixed routes and isolated actual-COP failure matrices | `acc9cc10c` |
| S17 | Independent L2 angle-approach memory effect, target/step configuration and true-byte Core DMA transport | `b3a7cc0a6` |
| S18 | Approved optional Types math declaration vocabulary, existing runtime contract and unchanged product linkage | `9b41a82ca` |
| S19 | Complete nine-ID L2 numeric/result chain, common dispatch status, bounded arithmetic and failure-prefix proof | `02a72979a` |
| S20 | Whole residual COP/RNG/DMA contract, existing live timeline capability and cohesive implementation receivers | `d20d65ee0` |
| S21 | Complete nominal L2 RNG aliases, existing timeline observation and actual Board/guest/reset proof | `bede72c2a` |
| S22 | Complete mode-banked DMA/copy/fill/sort receiver, one pending owner, and actual Board proof | `52fc45e98` |
| S23 | Complete Video-DMA/palette/brightness receiving-evidence gate; no invented implementation | `c48f9e02c` |
| S24 | Complete COP 2a05 offset-add receiver and bounded actual-bus proof | `7d37760f7` |
| S25 | Bounded selected-set startup diagnostic and explicit Board/COP-owner transfer | P1 `c75a016d0`; P2 state receipt; P3 owner correction |
| S26 | Selected-set `0x43e` write-only Board hole and bounded next-stop transfer | P1 `42845208d` |
| S27 | Exact `0x6c8--0x6c9` selected write-only span and bounded Sprite/Video transfer | P1 `f792feffa` |
| S28 | Sprite-flags receiver boundary and transfer to queued Video work | P1 `b2a2d00d5`; P2 closure receipt |
| S29 | Complete refusal/evidence boundary for `1905`, `9100` and `9900` | P1 `a5904833b`; P2 closure receipt |

The owner approved advancing Video's prerequisite inside T3 and delegates
routine same-T S planning. B04 now has an actual modeled periodic receiver;
full Video rendering and DMA qualification remain queued work. The next S
receives B07/B08 COP operation contracts/evidence and real-set startup proof.
It may not copy reference code, fabricate results or silently close unresolved
COP obligations. No further approval is needed for ordinary same-T ordering.

S26 admits the selected-set-observed `0x43e--0x43f` write-only hole as a
Board-local L2 route. The previous CPU fault is a correct V30 16-bit
`MOV moffs, AX` write, while the pinned reference map assigns no receiver to
that word. The route stores no state and has no readback; all other unknown
control addresses remain explicitly refused. The same two-width probe now
passes that point and reaches a new `0x6c8` write. That later address is not
absorbed by S26 and remains the next Board/Video map-evidence receiver.

Sequential coordinator actual-change review
accepts S10's Shared P1 f2ca9ae01, NXVM receiving P2 990ff6eda and MyRaidenII
P3 af1649f60. Both full unit suites pass687/687; selected PC receiving
integrations pass14/14 per width. Eight current0564 PC EXEs are rebuilt,
deployed and hash/PE verified; MyNES and protected inputs remain unchanged.
The [S10 review](../etc/evidence/t3-s10-cop-transfer-review.md) archives its
packet, code delta, incomplete first x64 attempt, final proof and cleanup;
[results](../etc/evidence/t3-s10-results.json) retain exact release identities.
Build now contains only the persistent .ccache/.ninja caches.

The complete S10 batch is upload/register admission plus the explicit L2
5205/5a05/f205 transfer family, not a generic programmable COP. Other B07/B08
numeric/geometry/collision/unresolved-trigger/DMA-sort operations and genuine
selected-set startup remain T3 obligations. S9's initialization observation
is not a new S10 ROM run. No complete external suite, desktop/RDP, physical
timing or playable game is claimed. T3 and M1 remain open; ordinary same-T S
planning remains owner-delegated.

Sequential coordinator actual-change review accepts S11 P1 b977bfa48.
0205/0905 now have explicit L2 macro effects and41 new actual guest/failure
cases in the existing executable;2a05 still rejects because its write width
and neighbouring-word effects are not established. Its same-T B07 semantic
reconciliation receiver is mandatory before T3 can close. The original
three-ID batch is not misreported as wholly implemented.

Both final full suites pass687/687 with no failures/skips (x64 106.12s,
x86 126.27s). Ten EXEs and protected inputs remain unchanged; only local
COP source/test and MyRaidenII records changed. Build contains only caches,
with no owned temporary output left. The [S11 record](../etc/evidence/t3-s11-cop-state-update.md)
retains the complete packet, source identities, actual delta, qualification
and explicit unverified boundaries. S11 is closed; its execution qualification remains unchanged by S12.
Other B07/B08 numeric/geometric/collision/DMA/result and real-set startup
obligations remain open. S9's22 uploaded IDs are not the full game's universe.

S12 research/design is accepted after coordinator actual-change review of
P1 87a1ac547. The [complete geometric receiver](../etc/evidence/t3-s12-cop-geometry-contract.md)
archives all nine forms, shared result/configuration dependencies, L2
limitations, independent oracles, exact future file/API estimate and packet.
No production/test/build/manifest or artifact changes occurred. Ten release
identities and protected inputs remain unchanged; document checks pass and
build contains only persistent caches. No new unit or game run is claimed.

The S12 proposal requested explicit owner approval for four optional Lib Types
mathematical names across six Shared files. That specific approval has now
been received and is being executed by S18; general product approval was not
used as a substitute. No workaround, raw CRT consumer access or private FPU
dependency is introduced. The next numerical receiver must consume this complete
numeric/result ledger and its evidence choices, not one first-failing opcode.
T3 and the full playable MyRaidenII goal remain open;
2a05, numeric/unresolved/DMA/result and genuine startup retain their original
required receivers. S12 is a complete contract audit, not implemented geometry.

Sequential coordinator actual-change review accepts S13 P1 b3098766d.
The complete four-ID collision batch now has explicit L2 position/bounds
capture and copied read-only results through the existing Core bus/service
path. The [S13 record](../etc/evidence/t3-s13-cop-collision.md) retains the
original packet, containment revision, actual delta, independent test matrix,
source/test/log hashes and qualification limits. It also corrects the S12
numeric entry-status contract without implementing geometry.

Final complete units pass687/687 per width (x64 146.13s; x86 148.06s);
the earlier503/687 deadline attempt is not counted as passing. Shared, all
other Apps, ten releases and protected inputs remain unchanged. All owned
processes are terminal and build contains only persistent caches. S13 is
closed; T3 and playable MyRaidenII remain open.
The remaining numeric/2a05/unresolved/DMA/result/startup batches keep their
mandatory receiving requirements; the accepted S14 decimal receipt follows.

Sequential coordinator actual-change review accepts S14 P1 c28a162d5.
The full decimal input/mode/result family now has an explicit original L2
implementation, independent literal oracles and actual V30/provider proof.
The [S14 record](../etc/evidence/t3-s14-cop-decimal.md) retains its packet,
failure-stage reconciliation, pointer review, actual delta and exact hashes.

Final frozen-source full units pass687/687 per width (x64 109.49s,
x86 129.18s). Shared, other products, ten release identities and protected
inputs remain unchanged. Owned processes are terminal and no disposable
task root exists; build contains only persistent caches. S14 is closed with
no active S packet at its acceptance. T3 and the complete playable MyRaidenII goal remain open;
numeric/math authorization,2a05, unresolved operations, remaining COP/DMA
decode and selected-set startup retain their required receivers.

Sequential coordinator actual-change review accepts S15 P1 fb5e14f7b.
The [S15 record](../etc/evidence/t3-s15-cop-register-inventory.md) archives the
complete map ownership inventory, packet, failure-stage reconciliation,
source/test/log hashes, actual delta and verification limits. Five missing
COP groups no longer succeed as RAM; unimplemented algorithms still fail
explicitly. The existing DMA mode shares stored GPR readback, with no new
state, public API or execution path.

Both final complete unit suites pass687/687 (x64 119.11s; x86 126.84s).
Shared, other Apps, ten releases and protected inputs remain unchanged.
Owned processes are terminal and build contains only persistent caches.
S15 is closed with no active packet. T3, M1 and playable MyRaidenII remain
open; the full low-control window, numeric/2a05/DMA/unresolved operations
and genuine startup retain their mandatory receivers. The independent
Shared complete-span query discrepancy is recorded in TODO, not repaired.

Sequential coordinator actual-change review accepts S16 P1 acc9cc10c.
The [S16 record](../etc/evidence/t3-s16-board-control-window.md) archives its
packet, one fallback-route design, exact retained13/25/14-row COP matrices,
actual Board CPU transfer/fault proof, frozen identities and qualification.
Every400h--7ffh byte has a device disposition; unknown data accesses no
longer succeed as hidden RAM. Existing valid multiword devices and real
boundary prefixes retain their original Core-owned transfer behavior.

Final strict cached builds and full units pass687/687 per width (x64
122.30s; x86 133.90s). Shared, other Apps, ten releases and protected inputs
remain unchanged. Owned processes are terminal; build contains only caches.
S16 is closed with no active packet. T3/M1 remain open: the Shared wide-query
classification debt, numeric/math authorization,2a05, remaining algorithms
and real-set startup retain their original receiving obligations. No desktop,
RDP, integration-suite or playable-game qualification is inferred.

Sequential coordinator actual-change review accepts S17 P1 b3a7cc0a6.
The [S17 record](../etc/evidence/t3-s17-cop-angle-approach.md) archives the
whole nine-ID dependency disposition, packet, independent L2 circular model,
actual53 command cases, byte/failure-prefix proof and exact run identities.
6200's memory effect and target/step capability now use the existing COP
store, admission and single Core DMA/safe-point path without new state/API.

Strict cached builds pass both widths. Full units pass687/687: x64 117.73s
before one final fixture row, followed by the final whole affected-unit
1/1 pass; x86 final687/687,129.64s includes that row. Evidence does not claim
the earlier x64 full run included it; unchanged686 binaries retain their
full proof. Shared, ten releases and protected inputs remain unchanged.
Owned work is terminal and build contains only persistent caches.
S17 is closed with no active packet; T3/M1 remain open. Common arithmetic
status/results, eight other numeric IDs, the explicit Shared math approval,
2a05/RNG/DMA/unresolved forms and genuine startup retain their full receivers.
There is still no playable MyRaidenII or desktop/RDP qualification.

Sequential coordinator actual-change review accepts S18 Shared P1 9b41a82ca.
The [S18 record](../etc/evidence/t3-s18-types-math.md) retains explicit owner
approval, the six-file scope, archived packet, exact aliases/link ownership,
actual delta and dual-width log identities. The additive optional header
provides no algorithm/state and leaves every existing interface unchanged.

Final full repository units pass687/687 per width, zero failures/skips
(x64 113.60s; x86 126.35s). Strict cached builds and subsequent no-work
incremental builds pass. Ten existing releases,33 tracked asset guards and
six media-role hashes remain unchanged; no existing product includes this
header and no release rebuild is required. Owned runs have terminated and
build contains only persistent caches. S18 is closed with no active packet.
The complete nine-ID numerical/result receiver is now unblocked by the math
vocabulary prerequisite, but remains unimplemented/unqualified as recorded
in S12/S17. T3/M1,2a05/RNG/DMA/unresolved forms and genuine startup remain
open; no desktop/RDP, external integration or playable-game pass is claimed.

S19 executor delivery completes the full nine-ID L2 numeric/result receiver.
The [S19 record](../etc/evidence/t3-s19-cop-numeric.md) retains all dispositions,
actual-bus/failure proof, the x87 truncation correction, frozen identities and
actual production+136/-3, test+333/-2 delta. Final complete units pass687/687
per width (x64 116.99s; x86 130.43s), zero failures/skips. Shared, ten release
identities and protected inputs remain unchanged. Sequential coordinator
actual-change review accepts P1 02a72979a against the complete nine-member
ledger and packet. The common register/dispatch/DMA path, all old literal
matrices and exact frozen source/test identities remain intact. S19 is closed
with no active packet and no owned temporary work left. T3/M1 remain open:
2a05, remaining unidentified/unresolved forms, RNG, sorting/DMA and genuine
selected-set startup retain their mandatory receivers. No rendering, sound,
external integration, desktop/RDP or playable-game qualification is claimed.

S20 executor delivery reconciles the complete residual COP/RNG/DMA batch in
the [residual contract](../etc/evidence/t3-s20-residual-contract.md). Existing
Core timeline observation removes the suspected RNG Shared-interface gap;
mode-banked DMA, Video latches, both2a05 outputs and unknown macro semantics
remain distinct required receivers. No source/test/artifact changes or new
dynamic qualification occurred. Sequential coordinator actual git show
review accepts d20d65ee0's four-file209-line record against the complete
residual ledger. Source/test delta remains zero and S19's exact687/687 dual
qualification is retained, not rerun or relabelled. Ten releases and all
protected guards are unchanged; no temporary work exists. S20 is closed with
no active packet. Ordinary local RNG implementation is unblocked without a
Shared edit; DMA, Video and residual/startup obligations keep T3/M1 open.

S21 executor delivery completes the mapped RNG family using existing Core
timeline observation, with no new state/API or Shared changes. The
[S21 record](../etc/evidence/t3-s21-cop-rng.md) archives the packet, explicit
L2 limit, actual production+18/-1 and test+121/-4 delta, independent raster
oracles and runtime guest MMIO. Final full units pass687/687 per width,
zero failures/skips (x64 387.69s; x86 171.12s). Strict cached and no-work
incremental builds pass. Ten existing releases and all protected guards are
unchanged; no affected existing EXE requires rebuilding. Owned runs are
terminal and build contains only persistent caches. Sequential coordinator
actual git show review accepts bede72c2a's six-file+298/-6 delivery against
the full RNG contract, archived packet and frozen qualification identities.
S21 is closed with no active packet. DMA, Video latches, remaining macros
and real selected-set startup keep T3/M1 open.

Sequential coordinator actual-change review accepts S22 P1 `52fc45e98`.
The COP has one raw-mode-selected 512-bank setup owner and replaces its prior
single pending-command fields with one discriminated pending operation; no
second queue, Core DMA route, public interface, Shared component, ROM, or
artifact path was added. The existing Board executable proves byte lanes,
bank isolation, invalid admission, copy/fill units, overlap, stable signed
sort, unknown sort ordering and reset. Final complete cached units pass
687/687 per width, and protected release/media hashes match after restoring
two unrelated MyNES EXEs rewritten by an aggregate build. The S22 evidence
archives exact hashes, L2 limits and remaining receiver boundaries. S22 is
closed with no active packet; T3/M1 remain open for Video-owned DMA, residual
macros and real selected-set startup.

Sequential coordinator actual-change review accepts S23 P1 `c48f9e02c`.
The receiver-3 evidence gate covers all ten modes (`14`, `15`, `80`--`87`),
their common registers and their private Video ownership without source import,
ROM inspection, special-case address, API or code change.  It correctly blocks
implementation until selected-set descriptor and palette/brightness oracles
exist; no unqualified mode is reported as a no-op or a RAM copy.  Documentation
governance and `git diff --check` pass.  S23 is closed with no active packet;
T3/M1 remain open for residual macros and genuine selected-set startup, while
the queued Video task owns later rendering/DMA qualification.

Sequential coordinator actual-change review accepts S24 after the final frozen
two-width qualification.  `2a05` now uses the one existing uploaded-command,
pending-operation and checked Core DMA route: for each slot it reads the
unsigned delta at `B1+1e+4i`, then writes widened sums at `B0+6+4i` and
`B0+1e+4i` in that order.  The existing Board executable proves all three
slots, carry, input/output ordering, pending/reset, source failure and late
DMA-lane prefix behavior.  Final full repository-only units pass687/687 per
width (x64 106.24s; x86 125.78s).  No Shared/public ABI, external source,
ROM/media, artifact, integration, desktop/RDP or playable-game claim changed.
The [S24 record](../etc/evidence/t3-s24-cop-offset-add.md) retains the
complete bounded model and frozen hashes.  S24 is closed.  T3/M1 remains open
for unresolved `1905`/`9100`/`9900`, Video qualification and genuine
selected-set startup.

S25's [bounded selected-set diagnostic](../etc/evidence/t3-s25-selected-set-startup-diagnostic.md)
uses the verified external program lanes without retaining protected payload.
It stops identically on x64 and x86 at a CPU-internal fault caused by a write
to `0x43e`, before a COP command checkpoint. A bounded Debug stack observation
confirms V30's existing 16-bit `MOV moffs, AX` branch, not a 32-bit CPU path.
This address is not established as a Board/COP register, so S25 does not widen
a route or weaken the explicit control window. Its owner is an open
MyRaidenII Board/COP mapping prerequisite; T3 remains open for that and its
existing COP/Video/startup receivers.

S26 accepts only the narrow selected-set `0x43e--0x43f` write-only Board
route. The existing Board executable proves that it does not supply readback
or weaken any other control-window rejection. The two-width selected-set probe
then advances from the prior stop by two instructions to the same new `0x6c8`
write on both widths. That later address is explicitly not absorbed: its
Board/Video receiver remains open, along with the existing COP, Video, and
startup requirements. See the [S26 record](../etc/evidence/t3-s26-control-write-hole.md).

S27 accepts only the independently evidenced `0x6c8--0x6c9` write-only span;
the real V30 word store reaches its two byte lanes and retains no state or
readback. Both selected-set probes next stop at the named `0x6ce--0x6cf`
Sprite/Video flags receiver. The [S27 record](../etc/evidence/t3-s27-selected-control-noop.md)
retains its exact boundary, cleanup and final 687/687 dual-width qualification.
S27 is closed; T3/M1 remain open for the receiver and genuine selected-set
startup, without any playable-game claim.

Sequential coordinator review accepts S28's source-policy-bounded disposition:
the selected write at `0x6ce--0x6cf` has a named Sprite/Video owner but no
independently qualified state, reset, decryption or frame contract. The
existing `0x6cb` program-bank receiver remains correct despite the older
reference-file comment naming `0x6c9`: the same fixed Raiden II map explicitly
maps the bank byte at `0x6cb` and leaves `0x6c8--0x6c9` unassigned. S28
therefore leaves S27's narrow no-op intact, imports no reference algorithm,
and transfers the flags receiver to the queued Video task. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S28 is
closed with no active packet. T3/M1 remains open for Video, sound, Product and
genuine selected-set startup qualification.

Sequential coordinator review accepts S29's complete three-form disposition.
The local classifier remains the only command admission/advance path; all three
forms still reject before creating a pending operation or result side effect.
No original hardware evidence was found to establish their operand/output,
reset or failure contracts, and source-policy rules prohibit deriving those
contracts from reference comments. The blockers remain T3-owned and are not
misrepresented as aliases, no-ops, Video or Sound work. Documentation-only
checks pass; no source, test, ABI, ROM/media or executable changed. S29 is
closed with no active packet. T3/M1 remains open for the three forms, Video,
sound, Product and genuine selected-set startup qualification.
