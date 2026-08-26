# T484 S21 XT BYOB Session Route

`M5:T484:S21:XT-B6-BYOB-SESSION:OK`

The selected `ibm-5160-model-268` profile now reaches Core only through the
existing typed `SESSION OPEN` request, session factory and session creator.
The product request supplies one transient, owner-provided base-system-ROM
manifest; composition validates its path, exact 64 KiB size and SHA-256 before
Core construction.  It may likewise validate one optional 8 KiB Xebec option
ROM.  Core receives only the corresponding immutable mappings at `F0000h` and
`C8000h`; it receives no file path, provenance or mutable manifest.

The former binary firmware selection is replaced by one finite
profile-selected choice: default-PC/AT synthesized firmware, Model-40 BYOB,
or XT BYOB.  The XT choice consumes the copied resolved topology, its external
firmware provider and the existing Core-owned FDC/HDC/media routes.  It does
not bind synthesized AT firmware, create an ATA alias or add a second media or
ROM state owner.  The fixed 360 KiB floppy and Type-2 Xebec image are accepted
only through the session media registry already owned by Core.

All BYOB blobs now use one neutral local validator/loader.  It owns only
byte-size and SHA-256 verification; profile code still owns slot layout,
provenance, mapping and ROM materialization.  Model-40 retains its distinct
even/odd interleave mapping but no longer retains a duplicate SHA-256 reader.
XT ROM storage is allocated only for an XT session and freed with that session;
the VM session object no longer carries an unconditional 72 KiB XT buffer.

Focused verification passed:

- `vm-xt-5160-268-profile-smoke`: required/optional ROM validation, immutable
  reset-vector route, rejected bad hash and CPU mutation, 360 KiB FDD and
  Type-2 media insertion, and typed product request construction.
- `vm-session-profile-smoke` and `vm-product-session-catalog-smoke`: retained
  session/profile and strict catalog behavior.
- complete Debug `current-gate` rebuild and test run: passed.

The current stripped Release artifact is
`build/output/nxvm_0_5_0484.exe`, SHA-256
`57FC91744B3C0F0F123D51D35E34423D271A965DA1D4A3BDC62AFDF66914B280`.

No ROM, firmware, guest media, local path, third-party code, board timing or
new command grammar is committed.  Xebec option-ROM execution, physical drive
service timing and XT board timing remain their existing later receivers.
