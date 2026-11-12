# M5 T587 S2 — selectable CMD build entry

Owner admitted S2 on 2026-11-11. SoftPC tools/build.cmd was inspected as a
workflow reference: thin CMD, CMake-owned build, visible result and pause.
Reference is project-owned MIT SoftPC `tools/Build.cmd` at
`51d0bc15ba65e12b1792c74780b51c89d75075b9`; no SoftPC repository was changed.
This implementation is adapted to the repository's seven products rather than
copying SoftPC identity, presets or assets.

Audit: existing PC/MyNES executable post-link hooks publish, but do not restore
an absent asset when linking is up to date. MyRaidenII has no automatic release
copy. Product targets do not require PowerShell; independent test/governance
targets do. Stable release trees and optional ccache already exist.

Plan: CMD selects a product and invokes a CMake driver; CMake configures the
existing release trees and builds only the selected publish target. Versions
come from existing executable declarations. A common repository publication
recipe always copies the built executable, including on a no-op build.
Expected implementation/test growth is roughly 200 lines. No production C,
shared package contract, runtime configuration or firmware changes are planned.

## Implemented design and first validation

`tools/build.cmd` owns only menu/arguments, CMake availability, exit status and
pause. `cmake/BuildProducts.cmake` maps seven product choices to the existing
profile selection and invokes x86 then x64 in the persistent release trees.
ccache is optional and its location stays owned by BuildEnvironment.cmake.
`PublishProducts.cmake` registers Release-only targets; `PublishProduct.cmake`
copies the EXE byte-for-byte; stripping belongs to product linking. MyRaidenII
now strips debug information when linking Release, matching the PC approach.
Existing product names
and versions are consumed, not parsed or duplicated in the menu. Existing
post-link hooks remain for direct build users; explicit publication additionally
works without a relink. App source/runtime paths are unchanged.

The actual `all --no-pause` invocation published all fourteen EXEs successfully.
A redirected menu selection 1 built both NXVM widths and reached the final
pause; selection 0 exited successfully. An invalid product returned nonzero.
Generated Ninja command closures for the four available publication targets
(default PC, PC110, MyNES, MyRaidenII) on both widths contain no PowerShell.
The three other PC selections use the same publication function; their actual
build log also contains no PowerShell. Fourteen PE headers have correct widths.

The publication regression uses an existing built binary, not user assets:
built/published identity, repeat hash, deleted-output restoration, missing
input failure with retained output, and CMD invalid-product propagation.

The first validation rejected a publication defect: stripping a second time
changed built-target identity. Both PC110 artifact integration checks failed on
both widths, and the x64 deleted-output test detected changed bytes. Initial
unit results were x86 716/716, x64 715/716; integration was 46/48 on each. The
corrected publisher only copies; no existing identity assertion was relaxed.

## Review boundaries

The ordinary executable post-link hooks remain to preserve direct-build behavior.
The explicit repository publish target additionally owns no-op re-publication
and uses one recipe across all seven products. Shared package source/test trees
are unchanged; no manifest regeneration is needed. Tools, root configuration and
repository tests form the Shared commit; App declaration/artifact changes are
separate MyNES/MyRaidenII commits; NXVM owns its records and PC artifacts.

The CMD menu was exercised through redirected input, not a desktop double-click.
It was run from this Windows host with the existing BYOB archive and toolchains.
Cold builds on a new PC, absent-tool installation and game-play acceptance are
not claimed. Optional ccache off/on is covered by the existing build-environment
unit test, not by requiring ccache for users.

## Final validation and artifacts

Both corrected publication tests passed (0.93/0.94 seconds); both PC110
artifact checks passed on both widths. Together with the full runs above,
all 716 unit and 48 integration cases per width have passing evidence. This
is a full run plus focused correction/retest, not a second full-suite run.
Background CTest included the registered native/desktop cases; no additional
interactive desktop double-click or game-play acceptance was performed.

All fourteen EXEs were republished with the real CMD driver. The final
MyNES/PC110 hashes equal HEAD, so these four files need no binary commit.
The other ten are retained in product-scoped commits. Existing versions
remain receiving baselines; no version or runtime change was admitted.
Generated publish command closures on both widths and the all-products log
contain no PowerShell. Repeating publication on both widths performed no
compilation/linking. Documentation governance, LF and diff whitespace pass.

| Published asset / verified PE suffix | SHA-256 |
| --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | 919bc33f2e8921af223785fb9533baca7e45c04fa7e5b89dbb3991dd723979ef |
| assets/nxvm/nxvm_0_5_0582_x86.exe | d2d64e5b35fe9956302c4f3bb02fa6b44eb52ce362064d5266d0f010b0ece03c |
| assets/my5160/my5160_0_5_0582_x64.exe | 0a40350c27d272c639795eff0a93cec1cb39baf4ba33f3bc19d5d9f2cfb0aed7 |
| assets/my5160/my5160_0_5_0582_x86.exe | 9778df55d4015ade52944951468c7053fef82bfae2e964ea5f67d29e562cb447 |
| assets/my5170/my5170_0_5_0582_x64.exe | 5248635bd26a6089d254f48b805030613d8ce3481751ab367d213792c24a99e8 |
| assets/my5170/my5170_0_5_0582_x86.exe | b0f33ddd34f41aa2748b2a0ee2ef9e7dc3269cfaf7d7cffb782fa2b7c94450c2 |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | e48fefd425429fe46ace9bf0d2a78fa238ee15da512bf1d5857772adee9b0a1f |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | 52bea6aa375651419327d6199d6c4b4b976081e183496713497c918f0b6dec18 |
| assets/mypc110/mypc110_x64.exe | c1fa066cc9e775ab80073e533a35c03ae3ae8a12da94dabdc5b425ad09a277cc |
| assets/mypc110/mypc110_x86.exe | 42757f431d5f867aa1ef4753ca45511b00ff342d7d7a70d69c75bb814648f4eb |
| assets/mynes/mynes_0_0_0044_x64.exe | ef41bb7ca2a57e03957c99c64000b5fb2cd7454ff2fbc3eb777b40fb5d04d136 |
| assets/mynes/mynes_0_0_0044_x86.exe | bd76f447efd072ca3879c74e4b4dd833fe51ee45b9ff3c416055c7890178bf3b |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | 798ea1e5d6846929166a213a7edf606b9f500ec66587cd23b9f908ad41b47a94 |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | 12ee31d81fe2210748183c3f1d1ca5d7f01c699c5d4fa75a1c7165c4ab01c8d0 |

## Coordinator actual-change review

Reviewed all changed/new build scripts, declarations, fixture, documentation
and deployed artifact hashes against the admitted packet. No shared src/test
package, INI, media or root README changed. Executable selection, versions and
architecture names reuse existing declarations. The sole product-link change
is Release debug stripping for MyRaidenII; publication itself is copy-only.
Scripts/build/test changes: +213/-0 lines.
No C source or public API changed. S2 is delivered pending owner acceptance.

Temporary log summaries are recorded above; raw task-owned logs will be
removed after their processes have exited. SHA-256 evidence index:

| Log | SHA-256 |
| --- | --- |
| t587-s2-build.log | e51faa8c34209c427dfc076724418c792b49a3e7d608b0a6dca19ad872f5c48f |
| t587-s2-doc.log | f75713948db59703b8e3f357eca11ec78c144e6b39d888a89c73128b6030b709 |
| t587-s2-final-build.log | a9b16f3c42e6e20a330a3e3de14d438a1636b7fbcdb1e9014f78c22d5ce27a07 |
| t587-s2-final-x64.log | ad47824985d1b7ac68bc46c46e73b53cbaa7d372f7c478ebdca31cea0dd11f7f |
| t587-s2-final-x86.log | b74bbefb8387bb79b430e43fb62b53f71b8059edb38b3b45317998b1313587d2 |
| t587-s2-incremental-x86.log | 8d5e5703817b09b07a273232682380845475c0bb03bdf75c91f1542c82c39487 |
| t587-s2-incremental.log | 3f05935f5d418cc0eef6369605cdefb88692ff976f2afa3f6e631a4af8838412 |
| t587-s2-integration-x64.log | d6d35bef2ecb18a0f201d85b29db93e09d9448c770f68b11e072b11476b741b0 |
| t587-s2-integration-x86.log | f98602f1ad0632c8331cce3e99cd2db79d979be7460dfea56a022dd83be9c7be |
| t587-s2-menu.log | c6b9ddf81a4e62e405bac1fc982ace6a8539f8153910ad1522b1e5a2d2868ab4 |
| t587-s2-x64.log | 2476486ba305dc7ead53e5610b6170192f0f07f14d1f6837ec36ac5add758532 |
| t587-s2-x86.log | 3327b86b6f2fe704eaa7c5178129ac318a1e10aa6b4ad9d8e046d0ae5ca5a988 |

Delivery commits: Shared tooling 799dde021, MyNES registration b8204b19c,
MyRaidenII declaration/artifacts f5610c439; the following NXVM P records
this evidence and the eight changed PC EXEs. Final source includes these
commits and the unchanged receiving App baselines.

## Owner acceptance and closure

On 2026-11-11 the owner accepted S2 and explicitly approved closure.
Delivery through 544e51f5c satisfies the admitted build/publication request.
S2 is closed; T587 remains open with no active S. This closure changes only
documentation, so no new compilation, runtime test or artifact is required.

### Archived admission packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: M5 T587 S2 after accepted S1. |
| Admission And Approval | Owner requested a double-click tools/build.cmd product menu, both widths and publication, with no PowerShell in the build chain; reference SoftPC approach (2026-11-11). |
| Objective | One selectable CMD entry with cached Ninja/GCC dual-width publication for all seven products. |
| Non-goals | No emulator behavior, version, INI/media change or replacement of independent governance/test tools. |
| Reference Baseline | 09ebe521e; inspected SoftPC tools/build.cmd as workflow reference only. |
| Candidate Proposal | [S2 build entry](M5-T587-build-entry.md). |
| Files And ABI Surface | Shared tools, root CMake and repository tests; MyNES/MyRaidenII publication declarations; NXVM records and receiving EXEs only if bytes change. No public C API. |
| Applicable Rules | Execution, Documentation, Architecture, Coding; NXVM/MyNES/MyRaidenII design and layout. |
| Verification | Real CMD menu and noninteractive calls; seven products x86/x64, incremental reuse, errors and absent output recreation; inspect target commands for PowerShell; existing unit suites and documentation/LF checks. |
| Expected Markers | Successful publication and nonzero failures, existing version and PE architecture for all fourteen outputs. |
| Asset Needs | Existing BYOB paths and stable .ninja/.ccache trees; never overwrite INI or media. |
| Reporting Requirements | Audit findings, actual diff, command usage, test and release evidence; owner acceptance before closure. |
| Stop Conditions | Do not hide missing tools/assets or stale outputs as success. |
| Exit Criteria | Menu and direct invocation verified; dual-width products published; checks, review, commits and push completed. |
| Original Owner Request | Select an App by double-clicking tools/build.cmd and build/publish 32/64-bit without PowerShell, similarly to SoftPC. |
| Similar-Issue Sweep | All seven products, version naming, no-op publication, tool lookup, caller cwd and failure propagation. |
