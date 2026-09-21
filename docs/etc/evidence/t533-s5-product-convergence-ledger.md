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
one deployed copy adjusts the relative `nxvm-assets` prefix for its
product-specific directory; media remains relative to the adjacent INI, not to
a CMake build directory.

## Construction And Test Universe

| Surface | Required disposition |
| --- | --- |
| Production construction | One CMake-selected profile binding, one App INI parser, no YAML/catalog fallback. |
| Integration construction | Each registered scenario uses the matching profile INI and external BYOB assets directly through readonly/overlay storage; no copied-media route. |
| Repository-only tests | Complete unit suite passes without INI, firmware, CMOS, font or media files. |
| Product integration | Each profile's registered boot row reaches its declared terminal in the optimized product build; Debug keeps repository-only unit coverage but is not a production-speed qualification runner. |
| Release deployment | Eight optimized, stripped x64/x86 artifacts exist with their matching INI companions only in `assets/binary/<profile>/`, without overwriting another product's configuration. |
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

- The fixed-product deployment no longer writes a shared `NXVM.ini` or a
  second product copy. Each Release product deploys only to
  `assets/binary/<profile>/`, with its matching executable and an
  adjacent INI whose external-asset relative prefix is correct from that
  directory.
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

All four product Release artifacts were redeployed after the repair for both
architectures, only to `assets/binary/<profile>/`. PE inspection
confirms every x64 artifact is machine `8664h` and every x86 artifact is
machine `014Ch`; no shared root INI or second product copy was written.

| Product | x64 SHA-256 | x86 SHA-256 |
| --- | --- | --- |
| `ibm-5160-model-268-360k` | `4E0C81B7E883498665AADF22D80D222A88BF905D25A54F4C886CF1F801E7E077` | `4771804A21D9890998EBC5A797E23E7257BD2D37BBC99A1B604A29A7E1D008B2` |
| `ibm-5170-model-339-1200k` | `EA03D4E567F949104EB7B196FC4253E7CB0265BA124B38B0ACF64B941A26CA63` | `450A34A71A428333AE7E0F44CCA2E63A843C5E22C2A4F970799025FBB244DFA4` |
| `compaq-deskpro-386-model-40-1200k` | `B2BD2C62767A3A8E125EECD82DFE449FB8E0E3F129F88D7DA65DF81D1E07A97B` | `7D45098771F9F603BABE47A40E031D57BA7622280B85E76C6654FEA1C91B8200` |
| `default-pc-at-80386-1440k-hdd` | `AF249AFD0858B868185AA6C9E4582AD8268B04E98435FCB00C19AA45BD26F9ED` | `5AA2B10EF866F031798FB74EB0ECB5E2B50B4B9EFB2AC769657C44BBCAE8C64F` |
