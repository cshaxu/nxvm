# T533 S5 Product-Convergence Ledger

This ledger freezes the T533 closure universe. A row is accepted only with the
named configuration, executable, INI and test evidence. A skipped or
unavailable owner-provided asset is a disposition, not a pass.

## Fixed Product Builds

| Product profile | CPU | Media topology | Required release artifacts | Current disposition |
| --- | --- | --- | --- | --- |
| `ibm-5160-model-268-360k` | 8088 | 360 KiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Both Release architectures reached `installer-running` |
| `ibm-5170-model-339-1200k` | 80286 | 1.2 MiB FDD | x64 and x86 EXE plus adjacent `NXVM.ini` | Both Release architectures reached `installer-running` |
| `compaq-deskpro-386-model-40-1200k` | 80386 | 1.2 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Both Release architectures reached `installer-running` |
| `default-pc-at-80386-1440k-hdd` | 80386 | 1.44 MiB FDD and HDC | x64 and x86 EXE plus adjacent `NXVM.ini` | Both Release architectures reached `dos-prompt` |

Every artifact row must validate the same profile manifest selected at CMake
configure time. The checked-in root INI remains the canonical template. The
two deployed copies adjust only the relative `nxvm-assets` prefix for their
one-level-deeper product directory; media remains relative to the adjacent
INI, not to a CMake build directory.

## Construction And Test Universe

| Surface | Required disposition |
| --- | --- |
| Production construction | One CMake-selected profile binding, one App INI parser, no YAML/catalog fallback. |
| Integration construction | Each registered scenario uses the matching profile INI and external BYOB assets directly through readonly/overlay storage; no copied-media route. |
| Repository-only tests | Complete unit suite passes without INI, firmware, CMOS, font or media files. |
| Product integration | Each profile's registered boot row reaches its declared terminal in the optimized product build; Debug keeps repository-only unit coverage but is not a production-speed qualification runner. |
| Release deployment | Eight optimized, stripped x64/x86 artifacts exist with their matching INI companions in the required product-output locations, without overwriting another product's configuration. |
| Cleanup | No user-selectable CPU/profile/firmware route, product YAML loader, catalog, or duplicate construction/reset/media path remains. |

## Known First Failure

The Model 40 Debug boot row reaches the timeout while a valid FDC `E6` transfer
is still progressing. Replays with the pre-S4 YAML probe and with the current
INI both demonstrate that this is not an INI/configuration regression. The
optimized Release x64 product reaches `installer-running` within 60 seconds.
Accordingly, Debug-timeout output is diagnostic-only; S5 validates product
bootability with the optimized build, then must replay every required product
and architecture before it can close.

## Completed S5 Evidence

- The fixed-product deployment no longer writes a shared `NXVM.ini`.
  Each Release product deploys only to
  `build/output/<profile>/` and `assets/sessions/<profile>/`; both receive its
  matching executable and an adjacent INI whose external-asset relative prefix
  is correct from that directory.
- The prior fixed-product implementation ran **336/336** repository-only
  cases.  This S5 repair removes the unconsumed Core guest-input-source and
  its isolated smoke, leaving **335** repository-only cases; the remaining
  host ingress coverage exercises the actual Machine/Common route.  The suite
  still consumes no INI, ROM, CMOS, font or guest-media file.  The revised
  **335/335** suite passed at four-way parallelism in 22.82 seconds.
- All eight stripped Release artifacts were built with their native x64 or
  x86 toolchain and passed the CMake PE-architecture check.  Each has an
  adjacent generated INI in both required product directories.  The four x64
  and four x86 boot probes reached the terminal recorded in the table above.

## Input And HDD Gate Repair

The former 17/20 integration result exposed two shared test/product boundary
errors rather than three profile-specific failures:

- `vm_machine_submit_host_input()` bypassed Common's executor FIFO and wrote
  KBC state from a host thread while the Core runner was active.  Composed
  machines now enqueue the copied `kvm_input_event` through Common; only an
  explicitly uncomposed deterministic Core loop retains direct owner-local
  delivery.  The redundant Core guest-input-source transport and its lone
  smoke were removed.
- The HDD-only probe used a canonical INI that correctly includes a normal
  floppy, but then incorrectly expected HDD VBR handoff.  It now removes the
  declared floppy through the existing media owner and resets before executing;
  it does not introduce a second INI, profile, firmware route, or boot-order
  override.
- Runner pause acknowledgement now follows final-frame publication, so a
  paused observer cannot read the preceding frame.

Focused replay of keyboard, HDD-only, Windows checkpoint and memory-fault
input tests passed serially.  The complete optimized default integration suite
then passed **20/20** serially and **20/20** at four-way parallelism (12.21
seconds for the final parallel replay).

## Revised Artifact Evidence

All four product Release build directories were rebuilt after the repair for
both architectures and redeployed only to their product-specific
`build/output/<profile>/` and `assets/sessions/<profile>/` directories.  PE
inspection confirms every x64 artifact is machine `8664h` and every x86
artifact is machine `014Ch` in both locations; no shared root INI was written.
