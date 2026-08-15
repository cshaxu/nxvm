# T374 S9: Media Address-Mark Contract

## Delivered Contract

S9 extends the profile-neutral media provider with one explicit capability,
`ADDRESS_MARKS`, and two logical-sector operations: query and set a sector's
address-mark class.  The only admitted classes are normal data and deleted
data.  The registry validates frozen binding, present medium, known geometry,
logical-sector range, advertised capability and callback availability before
dispatching either operation.  A provider that does not advertise the
capability, or advertises it without the relevant callback, returns the
existing `UNSUPPORTED` media result.  No byte pattern, absent callback or
FDC-local state is interpreted as a normal or deleted mark.  A provider that
returns a value outside the two admitted classes is rejected as `PERMANENT`;
the wrapper does not expose it to a consumer.

The contract is owned by `core/machine/media_interface`; its mutable mark
storage remains provider-owned.  The positive in-memory provider increments
its medium generation after a successful mutation.  This makes mark mutation
observable through the existing medium lifecycle without creating a second
generation or FDC-local cache.

## Provider Migration

Every `core_machine_media_provider` initializer was swept and migrated.

| Provider class | Disposition |
| --- | --- |
| Generic media-provider smoke fixture | Advertises the capability, stores one class per logical sector, supports both operations, preserves payload separation and advances generation on set. |
| VM FDD raw-image provider | Does not advertise the capability and supplies no callbacks.  Its byte-only raw image has no admitted persistent address-mark representation. |
| VM HDD raw-image provider | Does not advertise the capability and supplies no callbacks.  It remains unrelated to FDC Deleted Data semantics. |
| FDC core, topology and media-change fixtures | Explicitly supply no callbacks and do not advertise the capability because their existing behavior does not test Deleted Data. |
| HDC and mantle-shape fixtures | Explicitly supply no callbacks and do not advertise the capability. |
| Negative generic fixture | Advertises the capability while omitting callbacks, proving registry rejection is still `UNSUPPORTED`, rather than an implicit normal-data fallback. |
| Invalid-mark generic fixture | Advertises a callback that returns an out-of-contract class, proving the shared wrapper rejects it as `PERMANENT`. |

There are no other tracked production provider initializers.  The exact sweep
query was `rg -n "core_machine_media_provider" --glob '!build/**' src tests
CMakeLists.txt`.

## Focused Proof

`core-machine-media-provider-smoke` proves normal-data query, mutation to
deleted-data, generation advance, subsequent query and the no-callback
unsupported route.  `vm-media-provider-smoke` proves both raw FDD and raw HDD
omit the capability and return `UNSUPPORTED` from the corresponding registry
operations.  Fresh Git-Bash CMake/Ninja replay produced:

```text
M5:T270:S2:MEDIA-PROVIDER:OK
M5:T374:S9:MEDIA-ADDRESS-MARK:OK
M5:T272:S2:VM-MEDIA-PROVIDER:OK
M5:T280:S2:ATOMIC-MEDIA:OK
M5:T283:S6:ATOMIC-SAVE:OK
M5:T283:S2:CORE-FDC-MEDIA:OK
M5:T347:S2:FDC-SERVICE:OK
M5:T290:S1:FDC:PORT:OK
M5:T291:S1:FDC:PORT:OK
M5:T286:S1:ATA-NIEN:PORT:OK
M5:T283:S2:CORE-HDC-MEDIA:OK
M5:T347:S3:ATA-SERVICE:OK
M5:T274:S2:MANTLE-SHAPE:OK
M5:T366:S7:MODEL339-FIRMWARE-FDC-TOPOLOGY:OK
```

The post-correction configured `current-gates-gcc` invocation exposed a stale
T344 static inventory: the tracked
`core_machine_planar_parity_nmi_s3_smoke.c` fixture already called
`core_machine_create` but was absent from the retained list.  The gate now
classifies that existing T366 planar-parity fixture explicitly and requires 67
rather than silently ignoring it.  The same full-gate replay reconciled the
T359 receiver-name assertion and T345 retained-owner count with their current,
fully classified ledgers.  `current-gates-gcc` now passes, as do documentation
governance and `git diff --check`.

## Boundary And Transfer

S9 deliberately changes no FDC dispatcher, execution/result state or raw
image persistence.  The next T374 receiver may implement Read Deleted Data,
Write Deleted Data and normal-read Control Mark behavior only against this
contract.  That receiver must retain the raw FDD unsupported boundary unless a
separate owner-approved persistent medium representation is admitted.  Scan,
physical address-mark encoding, index/flux/CRC/gap behavior, controller or
board timing and Model-339 L3 closure remain open.
