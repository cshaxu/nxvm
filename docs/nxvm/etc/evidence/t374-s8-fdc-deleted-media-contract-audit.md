# T374 S8: FDC Deleted-Data Media Contract Audit

## Decision

The Intel 8272A has distinct `Read Deleted Data` and `Write Deleted Data`
commands.  Its command descriptions state that a successful deleted-data read
identifies a deleted-data address mark, while a normal-data read that encounters
one reports the ST2 Control Mark condition.  Conversely, Write Deleted Data
writes a deleted-data address mark, rather than an ordinary sector write with a
special byte pattern.  The selected source therefore makes address-mark class
sector metadata: it is independent of the sector payload and must be visible to
both normal and deleted command routes.  The primary command descriptions and
result semantics are recorded in the
[Intel 8272A command reference](https://cpctech.cpcwiki.de/docs/i8272/8272sp.htm).

The current `core_machine_media_provider` contract contains only query,
byte/sector read, byte/sector write, sector-format and flush operations.  It
has no capability or operation for a sector address-mark class.  The FDC
therefore cannot truthfully implement either Deleted command: byte I/O cannot
say whether the selected ID field precedes a normal or deleted data address
mark, and an ordinary write cannot change that class.

S9 is the selected repair receiver.  It must add a profile-neutral,
logical-sector address-mark capability and query/set operations, including an
explicit unsupported result for providers that cannot represent the metadata.
The operation must identify at least normal-data and deleted-data classes; it
must not expose filler bytes as a substitute for physical media structure.  A
later FDC receiver may use the contract to implement Read/Write Deleted and
the normal-read Control Mark result.  It must not implement Scan, flux, index,
CRC, gap, physical address-ID search or timing as part of this bounded chain.

## Provider And Consumer Sweep

| Surface | Current truth | Required S9/S10 disposition |
| --- | --- | --- |
| `media_interface.{h,c}` registry and byte/sector/format wrappers | One generic logical-sector contract; no address-mark capability, callback, wrapper or result distinction. | Own the capability, mark enum and validated query/set wrappers.  A metadata mutation advances the medium generation exactly as other guest-visible media mutations do. |
| VM FDD raw-image provider | Retains only raw payload bytes plus geometry; insertion/removal/format advance generation and removal persists only the raw image bytes. | It cannot claim persistent Deleted Data support merely from a `.img` file.  S9 must classify it explicitly; a later admitted representation/persistence design is required before it advertises mutable address-mark capability. |
| VM HDD raw-image provider | Same byte/geometry abstraction, but it is not an FDC floppy medium. | Keep address-mark capability absent; no ATA/HDC semantic change. |
| FDC fixture providers (`core_machine_fdc_smoke`, topology and media-change) | Mutable in-memory payload fixtures used to prove FDC routes. | Migrate to the new ABI and provide deterministic mark metadata for Deleted-command tests, or explicitly remain unsupported in tests that do not exercise that route. |
| Generic media-provider and mantle-shape fixtures | Exercise registry validation and composition, not FDC address-mark semantics. | Migrate every initializer; add a negative unsupported-provider proof so no caller treats a missing callback as normal data. |
| HDC fixture | Logical disk fixture for a separate controller. | Migrate ABI with address-mark capability absent; retain its current controller scope. |
| Current FDC transfer and format routes | `Read Data`, `Write Data`, `Read Track` and `Format` issue byte/sector operations only. | S10 must query the mark before read selection, return the documented Control Mark status where required, set deleted metadata after a successful Write Deleted transfer, and set normal metadata after a successful format.  FDC reset changes controller state only; it must not rewrite medium metadata. |

The initializer sweep covers the two production providers and these six test
providers: FDC media-change, FDC core, FDC topology, HDC, generic
media-provider and mantle fixtures.  No other production media provider is
registered by the current VM composition.

## Lifecycle And Persistence Boundary

Address-mark metadata belongs to the inserted medium, not to the FDC command
or drive state.  Therefore controller reset, DOR reset, seek and recalibrate
must preserve it.  A successful mark mutation must be observable as a media
generation change, so the existing disk-change observer cannot cache a stale
mark.  Removal discards the inserted medium state; re-insertion may restore it
only when the provider's admitted on-disk representation restores it.

The current raw floppy image save path writes exactly the payload image.  It
has no place for deleted-data address-mark metadata.  S9 must not silently add
an untracked sidecar or advertise a transient implementation as persistent
guest media.  If raw-image support is retained for Deleted Data, its later
receiver needs a documented, atomic representation and save/remove lifecycle;
otherwise the provider returns unsupported and the product-support receiver
must decide the exposed command boundary.  That decision is deliberately
outside this audit.

## Focused Baseline Proof

Git-Bash invoked the locally installed CMake 3.30.5 and its configured Ninja
generator.  The unchanged provider/FDC/Model-339 replay produced:

```text
M5:T270:S2:MEDIA-PROVIDER:OK
M5:T272:S2:VM-MEDIA-PROVIDER:OK
M5:T280:S2:ATOMIC-MEDIA:OK
M5:T283:S6:ATOMIC-SAVE:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T291:S1:FDC:PORT:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

## Transfer

S9 owns only the shared media-contract and provider-ABI migration, with
explicit unsupported behavior and fixture proof.  S10 may then implement the
FDC command/result routes against that contract.  Physical address-mark
encoding, index/flux/CRC/gaps, drive/controller service time, DMA competition,
board timing, Scan commands and Model-339 L3 closure remain separate work.
This audit makes no functional-completion or L3 claim.
