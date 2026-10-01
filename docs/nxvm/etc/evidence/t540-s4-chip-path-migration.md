# M5 T540 S4 Chip Path Migration

S4 moves the independent Shared chip corpus from `src/x86/devices` and
`test/x86/devices` to `src/x86/chips` and `test/x86/chips`. NXVM source,
build, test and tool references now use the sole new path. No chip algorithm,
public symbol, board wiring, firmware or INI semantics changed. MyNES does not
link x86 and received no change.

## Changes and ownership

- Shared P2 `c4d2fc29d`: 173 files; 450 added and 450 removed lines, net zero.
  The source and test moves are Git renames; the remaining edits are path
  references, manifests and static gates.
- NXVM P3 `e8ea2ddc8`: 108 files; 200 added and 200 removed lines, net zero.
  It reconnects product includes, tests, tools and build inventories and
  advances the executable revision to `0.5.0540`.
- `rg -n -F 'x86/devices' src test cmake tools CMakeLists.txt` finds no live
  reference. No forwarding header or parallel old source remains.

## Verification

- Shared x86 standalone test suites: x64 **119/119**, x86 **119/119**.
- Complete repository-only unit suites: x64 **467/467**, x86 **467/467**.
- `src/x86` and `test/x86` manifest checks, corpus boundary and negative
  probe: pass. `git diff --check`: pass.
- Four fixed NXVM profiles each built as optimized Release for x64 and x86;
  the artifact gate accepted the corresponding PE architecture. These are
  product builds, not unit-test substitutes. The external BYOB root supplied
  existing selected ROM inputs only; no asset was imported into source.
- MyNES source and artifact paths remained unchanged.

## Deployed 0540 artifacts

| Profile | Architecture | SHA-256 |
| --- | --- | --- |
| XT 5160 | x64 | `F324FE2DE1E78C270BF69FA30B25F95EDFFF7F5A8A16E42257D38E8162C0460F` |
| XT 5160 | x86 | `6FCC3DA414C9CF6756E1AB795E480AF7B5B5E4F2950F85B6361C0D9EED5EF7F1` |
| AT 5170 | x64 | `D2042CB54A0800159A2444ADA41A99177020B3F1869ED963926747E31A595A9D` |
| AT 5170 | x86 | `605EDB3196952CCA604460FB65C0A9010703E2183998EF82095C3F21F4FA1B92` |
| Model 40 | x64 | `A04F2F624BC4453B0C1267372FAC4F41FC4C4A456E11563A56366ACB8E9695E3` |
| Model 40 | x86 | `A7BA009F29BE69552B5359B73789C4C1779A08AADEF9879AE14E85EC722BD4BD` |
| Default PC/AT | x64 | `DAA8BD55EA0B817C75D37BAEECFC1830D8A9EE4608E95D08045C680DC09759B4` |
| Default PC/AT | x86 | `5F4E7AA0D3248B212EE66A6C926C1A6149953CDAAC91CDA88B0AE19F50AC7596` |

Each profile directory retains only its verified 0540 pair and its unchanged
adjacent `NXVM.ini`. The superseded 0539 binaries remain recoverable in Git
history. S4 is a structural path migration, not a new runtime behavior claim;
T540's full external integration gate remains for task closure.
