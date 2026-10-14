# Project Status

## Current Work

The owner accepts T556 S11 and requests the new PC publication/INI task.
T556's NXVM work is closed. The owner subsequently closed S10's diagnostic
delivery with its unresolved MyNES input repair transferred to
[MyNES TODO](../../mynes/states/TODO.md#rdp-short-input-repair).
`3dc750be2` is not a production fix. No MyNES implementation is
part of T557's original S1; S2's approved receiver exception is recorded below.
Prior S9/S11 packets below are historical delivery evidence.

## Retained Progress — M5 T557

| Task | Status |
| --- | --- |
| T557 S2 | Owner accepted and closed S2 on 2026-10-13. T557 remains open; no active S or next-task admission. |

Accepted delivery: Shared `5f5f43c8c`, MyNES `08bdaf06d`, NXVM `97d3cf948`.
Both widths passed 592 repository units, 36 PC integrations and 12 MyNES
integrations, with all ten EXEs deployed. The [S2 evidence](../etc/evidence/t557-s2-cursor-ownership.md)
retains the contract, review, hashes and verification limits. This closure only
updates governance; production and executable inputs are unchanged, so the
verified binaries remain current without a rebuild. The [proposal](../proposals/m5-pc-publication-ini-identity.md)
remains active for the open T.

## Delivered Task — M5 T557 S1 PC Publication And INI Identity

| Field | Required record |
| --- | --- |
| Identifier Mode | New T557 S1, next numeric NXVM task after accepted T556 S11 and explicit MyNES residual transfer. |
| Admission And Approval | Owner requests four product-specific lowercase EXE/INI identities, explicit App INI injection and full affected qualification. This naming contract supersedes the old machine-key artifact naming in Execution for these four Apps only. |
| Objective | Publish my5160, my5170, mydeskpro386 and nxvm release pairs and load each App's own adjacent lowercase INI through the existing Core parser. |
| Non-goals | No SoftPC access/import, Shared-six changes, MyNES changes, configuration-content/media/ROM/snapshot mutation, fallback names or machine-key renaming. |
| Reference Baseline | ed75cf572, clean after separate MyNES diagnostic publication; accepted PC implementation 6b0cb13e8. |
| Candidate Proposal | [PC publication and INI identity](../proposals/m5-pc-publication-ini-identity.md). |
| Files And ABI Surface | Four App build/main declarations; Core Product startup signature and version; NXVM build/deployment/gates; affected Core/App tests, current documents and eight assets. Root PC preset target version changes use a separate Shared commit, with no MyNES preset change. |
| Applicable Rules | NXVM guide and Architecture/Coding/UI/source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Existing x64/x86 Ninja/ccache graphs; affected startup/config/Console tests, all repository units, four-PC integrations and static/manifest/docs gates. Prove explicit INI selection, no wrong-name fallback, INI-relative media paths, lowercase release identities, PE widths and deployment hashes. |
| Expected Markers | Eight version 0.5.0557 release files with product-specific prefixes; four matching lowercase INIs, byte-identical contents; unchanged display names and internal machine keys. |
| Asset Needs | Read existing external inputs only; create and verify new eight artifacts before removing replaced PC EXEs; preserve MyNES and owner media/configuration bytes. |
| Reporting Requirements | Per-file scope/diff and naming map, actual dual-width results, release identity proof, missing manual/RDP verification; never infer desktop acceptance from unit tests. |
| Stop Conditions | Required Shared implementation, format/behavior change beyond naming, unexplained input mutation or conflict with other work. |
| Exit Criteria | Source/build/runtime/test/docs identities agree, required tests and eight deployments pass, old duplicate releases removed, reviewed target-scoped commits pushed; await owner review. |
| Original Owner Request | Close current S and admit a new T for four PC product-named EXE/INI publication and explicit runtime configuration loading. |
| Similar-Issue Sweep | All four App declarations/callers, integration/setup fixtures, release gates/scripts/presets and current normative documents; historical text stays historical. |

### S1 Delivery

Implementation and qualification are complete: 592/592 repository units and
36/36 four-PC integration executions per width. Eight 0.5.0557 product-named
EXEs are deployed and the replaced eight filenames removed. Four renamed INIs
retain their original hashes. Both specialized gate aggregates, manifests,
registration, INI/publication and documentation checks pass. The
[delivery evidence](../etc/evidence/t557-s1-publication-ini-identity.md) records
actual diff, final hashes and limits. Shared-six and MyNES are unchanged.
S1 is delivered for owner verification; T557 remains open.

## Accepted Task — M5 T556 S11 Default Text Raster

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: next unused S11 after the MyNES S10 admission; do not execute or modify that product's pending work. |
| Admission And Approval | Owner requests default NXVM 80x25 full 8x16 text, DeskPro verification only unless a defect is proven, dual-width qualification and deployment. |
| Objective | Repair the default guest BIOS text-mode register/font geometry so boot and graphics-to-mode03 return produce 640x400, with correct cursor and raster timing; establish whether DeskPro 640x350 is correct. |
| Non-goals | No Shared change, forced Window height, host BIOS interception, external ROM or INI/media mutation; preserve MyNES and other pending changes. |
| Reference Baseline | Current master 7bfefb2fc, plus preserved S10 documentation/diagnostic work; review b2ee99b1c and b25e2ca8d and subsequent firmware changes. |
| Candidate Proposal | [T556 proposal](../proposals/m5-app-component-layout.md), owner-added S11. |
| Files And ABI Surface | Default firmware/video tests and native size diagnostic; owner-added Core CPU retirement selection and existing capture boundary, timing tests/gates and Core manifests. Timing selector becomes CPU-private; no Shared API change. Docs and affected four-PC dual EXEs. |
| Applicable Rules | NXVM guide, architecture, coding, UI and source policy; shared Execution/Architecture/Coding/Document rules; architecture-governance and coding-governance skills. |
| Verification | Real guest execution at boot and return from graphics; complete font halves, cursor, vertical active/retrace/exit; snapshot-to-Window 640x400; actual client dimensions. Both existing Ninja/ccache graphs, full repository units, all four PC profile integrations including default DOS/Windows and required static/manifest/doc gates. Actual RDP DPI proof is separately reported, never inferred from a synthetic frame. |
| Expected Markers | Default 80x25, glyph height 16 and 640x400; coherent CRTC overflow/total/display/blank/retrace and cursor registers. DeskPro retains 14/350 if sources and selected board agree. |
| Asset Needs | Existing external manuals and build inputs read-only; refresh affected four-PC 0556 x64/x86 pairs and validate PE identity after the Core repair. Preserve MyNES, adjacent owner inputs and unrelated EXEs. |
| Reporting Requirements | Record sources and timing grade, before/after reproduction, actual diff, dual-width results and release hashes; distinguish native client, outer frame and untested RDP scaling. |
| Stop Conditions | Required Shared/API change, unsupported EGA register semantics, unproven DeskPro defect, or overlap with unrelated work. Report before extending scope. |
| Exit Criteria | Correct default guest BIOS and regression proof, required qualification/deployment complete, actual-diff review and scoped commits pushed; await owner desktop acceptance, T remains open. |
| Original Owner Request | Investigate/fix default Window text geometry from guest BIOS and verify rather than normalize DeskPro 640x350. |
| Similar-Issue Sweep | Check all default text-mode initialization/restore paths, glyph upload, BDA geometry, cursor and raster counters; inspect selected DeskPro BIOS/board without changing external ROM. |

S11 implementation and final automatic qualification are complete: 592/592
units and 32/32 four-PC profile integration executions per width. All eight
PC EXEs are rebuilt and deployed; MyNES and public six remain unchanged.
The default BIOS now presents all 16 font rows. Its exposed Core TIME fault
is repaired by selecting completed-instruction timing before interrupt entry
reuses decoder state. The actual-Core regression fails against admission
objects and passes with the repair. DeskPro retains its correct 14/350 mode.
Native remote-session observations no longer show the TIME failure, but the
diagnostic pause acknowledgement and owner phone-client DPI/interaction are
not qualified by automation. The owner has accepted this scoped delivery;
S11 is closed. Preserve the separately retained MyNES S10 pending work. See the
[S11 evidence](../etc/evidence/t556-s11-default-text-raster.md).

## Delivered Task — M5 T556 S9 Audit Repairs And Qualification

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: S9 follows completed S8 audit and accepted S1--S7. |
| Admission And Approval | Owner's 2026-10-13 request assigns necessary audit repairs to S9. Targets are NXVM and MyNES, with separate commits. The proposal's existing Shared root-build receiver covers only three PC preset target names changing from 0555 to 0556; public six and shared rules remain excluded. |
| Objective | Flatten DeskPro source and matching unit tests under the owner's S9 amendment; repair A1--A5 under that revised ownership, retain A6's separate Td receiver and complete five-App qualification. |
| Non-goals | No Shared-six or shared-rule edit, unrelated hardware rewrite, new runtime manager, persisted-format change or owner-input mutation. |
| Reference Baseline | S8 audit `a28cac691`; accepted S7 implementation `307cf1bd6`. |
| Candidate Proposal | [Five-App component layout](../proposals/m5-app-component-layout.md). |
| Files And ABI Surface | Core test registration/owner gates; all DeskPro source moved to its App root and units to its unit root, with includes/guards/symbols/build/gates/docs repaired; moved MyNES Machine symbols/callers; corresponding product version/artifact receivers. No Shared API change. S9 actual diff is recorded against the amended flat file map. |
| Applicable Rules | Product guides; shared Execution, Architecture, Coding and Document rules; both product architecture/layout authorities; source policy; architecture-governance and coding-governance skills. |
| Verification | Correct labels/names without reducing cases; reject incomplete D4 attachment with rollback proof; run dual-width full units and every admitted profile/MyNES integration in original Ninja/ccache trees; verify manifests, dependency/doc gates and affected release pairs. |
| Expected Markers | No absolute-path or mislabeled Core tests; no DeskPro chips/machine/product source directories or matching unit nesting; one D4 state owner and one Core attachment owner; missing alias rejects atomically; documents match current source; complete per-route results and ten qualified artifacts. |
| Asset Needs | Existing BYOB inputs read-only; update affected executable pairs only, preserving INIs, media, ROM originals and snapshots. |
| Reporting Requirements | Report per-target actual added/deleted/net lines, retained owners, exact test coverage, artifact identities/hashes, failures and unexecuted desktop checks. |
| Stop Conditions | Required Shared-six/rule change, incompatible saved data, unexplained behavior change or newly expanded hardware scope. Report rather than patch outside this packet. |
| Exit Criteria | A1--A5 repaired with evidence; A6 explicitly transferred; required dual-width builds/tests and artifact checks complete; actual changes reviewed, per-target P commits pushed, clean worktree; T closure still requires final acceptance. |
| Original Owner Request | Admit S8 for comprehensive governance, documentation and code-quality audit; admit S9 for necessary repairs. |
| Similar-Issue Sweep | Search all five App owners and Core plus current authorities for the same stale ownership, registration, vocabulary and contract defects; retain historical terminology as history. |

### S8 Audit Delivery

The [audit ledger](../etc/evidence/t556-s8-quality-audit.md) records six
confirmed classes and their S9 disposition. Both widths pass the 18 selected
static gates; both documentation checks and the production dependency DAG pass.
No new full dynamic qualification is claimed. Production, tests and assets
remain unchanged. Shared-rule vocabulary retains a separate Td receiver.

| Task | Progress |
| --- | --- |
| T556 S9 | Delivered for owner review: 592/592 units and 44 profile/MyNES integration executions per width, both specialized gate targets, documentation/dependency/manifest checks and ten release artifacts. T556 remains open. |

## Retained S9 Delivery

The [S9 delivery report](../etc/evidence/t556-s9-quality-repair-delivery.md)
records exact scope, actual diff, logs, artifact hashes and unperformed manual
acceptance. Four PC Apps retain 0.5.0556 under their flat asset roots; MyNES
retains its independent 0.0.0044 pair. The PC target is `vm-0-5-0556`.
MyNES receiving commits are `7fcd7b1d9` and `c5ca06e85`; the NXVM commit
containing this packet/report identifies its delivered source and eight EXEs.
Root preset names use a separate Shared-target commit; the public six are unchanged.

## Historical Evidence

Prior packets remain in the [retained status archive](../history/M5-T556-retained-status.md).
They are not concurrent active tasks. The [T556 history](../history/M5-T556-app-component-layout.md)
records accepted layout work, S11 closure and the explicit MyNES residual.
