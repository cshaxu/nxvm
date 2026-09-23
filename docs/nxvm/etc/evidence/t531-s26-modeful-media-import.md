# M5 T531 S26 modeful-media Common import

Frozen source: SoftPC `95c467a73e448ca7953f8d1b776843b26316651b`.

- `src/lib`, `src/common`, `test/lib`, and `test/common` compare byte-for-byte
  with the frozen source after the import.
- YAML `media.floppy[]` and `media.fixed_disk[]` entries accept `mode:
  direct|readonly|overlay`; omitted mode is `overlay`. App carries the one
  selected Lib storage mode through the construction config. VM-machine is the
  sole FDD/HDD opener; runtime removable FDD replacement uses Common's same
  modeful driver contract.
- The former three FDD/HDD mode wrapper families were collapsed into one
  mode-parameterized owner API per device. No YAML output/session file changed.
- Focused catalog/default/error and all FDD/HDD mode tests pass. Full
  repository-only unit suite: 336/336 pass. Documentation governance and
  `git diff --check` pass.
- x64 and x86 stripped Release artifacts were built and copied identically to
  `build/output` and `assets/sessions`:
  - x64 `85DCE01B1BFE67C598B542100A1E666063B64D2BD510E42FD6E9F7A372B4079F`
  - x86 `DFCC7BAA63184508BE99E1D6679AE52CE51623D62F33F525506EF0E0D9D19436`
- External integration first completed 41/42: the Model 40 1.2MB boot row
  timed out at 180.07 seconds in FDC READ TRACK. Its immediate isolated replay
  passed at 175.57 seconds. This is recorded as a non-deterministic existing
  boot-chain observation, not accepted as a single clean 42/42 run; T531
  remains open for its broader acceptance gate.
