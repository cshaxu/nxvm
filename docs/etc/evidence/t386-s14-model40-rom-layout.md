# T386 S14: Model 40 Rev-E ROM Carrier And Reset Alias

`M5:T386:S14:MODEL40-ROM-LAYOUT:OK`

`M5:T386:S14:MODEL40-ROM-RESET:OK`

## Delivered Boundary

S14 corrects the private Model 40 firmware carrier introduced by S7.  The
primary Compaq DeskPro 386-16 Technical Specification (1986), D3PE processor
board and D4-SROMA descriptions was consulted transiently under the source
policy.  It describes the standard high ROM bank as two 16 KiB odd/even chips
and its 64 KiB system-ROM presentation.  No scan, ROM, firmware, guest media,
local path, hash, binary, third-party source, or source text is retained in
Git.

The private carrier now accepts exactly two non-null 16 KiB chip spans.  VM
interleaves even and odd bytes into a 32 KiB logical image, mirrors that image
to fill the 64 KiB F0000h--FFFFFh ROM window, and supplies copied immutable
bytes through the existing core firmware interface.  It also supplies the
Model-40 high-reset alias at FFEF0000h--FFEFFFFFh.  The unchanged generic A20
route maps the 80386 reset fetch at FFFFFFF0h to that profile-owned alias.

This is VM composition: chip organization, board window and reset alias are
Model-40 facts.  Core has no S14 source change and continues to own its generic
copied immutable-ROM and A20 behavior.  S7's historical record remains intact;
its old contiguous 128 KiB carrier is superseded for current source by this
S14 correction.

## Proof And Similar-Issue Sweep

`vm-model40-rom-layout-s14-smoke` uses project-owned synthetic bytes only.  It
rejects a non-16-KiB pair, verifies odd/even interleaving at F0000h/F0001h,
verifies the mirrored half at F8000h, verifies the canonical top-window reads
including FFFFFFF0h, rejects writes to both immutable aliases, and executes a
synthetic HLT at the 80386 reset vector.

The sweep searched all tracked VM Model-40 carrier declarations, provider
calls and test initializers for the retired contiguous symbols and every direct
carrier initializer.  It found the private composition route and S7--S13
controls only; all are migrated atomically to the two-chip contract.  Other
`.bytes` and `byte_count` members belong to unrelated CPU, DMA and media data
structures and are not carrier hits.  No other profile consumes this private
Model-40 carrier.

## Verification

- Focused Model-40 S14 plus retained S7--S13 private controls passed.
- The two initially affected controls, `vm-fdc-dma-boundary-smoke` and
  `vm-default-pc-at-rom-materialization-smoke`, pass after Core was restored
  and the VM-owned alias was used.
- Serial `ctest -L current-gate --parallel 1` passed 269/269 tests.
- `run-current-smokes`, VM-composition, strict T345 ownership and
  documentation-governance gates are recorded at delivery closure.
- The runnable developer artifact is `vm-0-5-0386`,
  `build/output/nxvm_0_5_0386.exe`, SHA-256
  `254054B0700F9A9F286C13AD16D7214C7946C4BF4D916D75FD5257728B21E437`.
  It prints `Neko's x86 Virtual Machine [0.5.0386]` and exits cleanly from the
  zero-session Console with `exit`.

## Transfers

S14 is private selected-machine functional progress only.  It is neither a
public ROM loader nor a real-firmware execution claim, and it does not model
ROM shadowing, relocation, flash/update behavior, physical board decode,
selected-device timing or Model-40 L3.  Those remain explicit receivers in the
T386 functional and board/L3 proposals and `TODO.md`.

## Coordinator Acceptance

P1 `f8257a5d` is accepted after independent actual-diff review.  The review
confirms that the revision changes no Core source: generic immutable mapping
and A20 semantics remain there, while the Model-40 chip organization, system
ROM window and reset alias remain in VM.  The carrier migration covers every
private initializer and retained S7--S13 control; the new focused smoke proves
interleaving, mirror, immutability and canonical reset fetch.  The reviewed
verification record is 269/269 serial current-gate, successful current-smoke
aggregate, VM-composition, strict T345 ownership and documentation governance.
No protected asset is tracked.  S14 is accepted as private selected-device
functional progress only; T386, public firmware admission, board behavior,
timing and Model-40 L3 remain open.
