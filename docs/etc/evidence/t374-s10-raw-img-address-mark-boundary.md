# T374 S10: Raw-IMG Address-Mark Support Boundary

## Owner Decision

The owner selects ordinary raw `.img` as the only admitted floppy-image
exchange and persistence format. It remains a sector-by-sector payload image:
its selected 1.44 MB geometry and payload are preserved, but it carries no
per-sector data-address-mark class. NXVM will not introduce a private
container, implicit sidecar, hidden host-path association, or a byte-pattern
heuristic to represent that missing information.

Accordingly, the FDC's `Read Deleted Data`, `Write Deleted Data` and normal-read
ST2 Control Mark distinction remain unimplemented deferred 765 functionality.
The separately unimplemented Scan family remains deferred with the rest of the
FDC command-capability work; this decision does not assert that Scan needs
deleted-address-mark persistence. The raw FDD provider continues not to advertise
`CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS`; its address-mark operations
return `UNSUPPORTED`. This is an explicit current support boundary, not an
assertion that ordinary raw sectors are deleted-data sectors, nor a claim that
the 8272A command surface or Model-339 functional closure is complete.

## Lifecycle Sweep

The read-only lifecycle sweep found one raw payload representation and no
metadata identity that could survive its paths.

| Route | Current behavior | Address-mark disposition |
| --- | --- | --- |
| FDD create/insert | `vm_machine_fdd_insert_for()` reads the fixed-geometry raw payload into the inserted medium. | No mark metadata is present or inferred. |
| Guest byte/sector write and format | The VM provider mutates raw payload bytes and advances the existing medium generation. | Ordinary payload behavior remains supported; it does not create a deleted-data mark. |
| Save/remove | `vm_machine_fdd_remove_for()` writes only the raw payload through the single-file atomic-save helper, then clears inserted state. | There is no second file or in-memory metadata to persist. |
| Reinsert | A later insertion reloads the selected raw payload path. | No deleted-data identity can be restored, so capability remains unsupported. |
| Read-only/failure/collision | The existing helper owns one-file temporary-name, replace and failure behavior for the payload path. | A paired sidecar would need an unadmitted multi-file identity and atomic-commit contract; none is introduced. |
| Session/console bindings and fixtures | Callers select/pass a raw image path; FDC and VM media fixtures exercise payload/geometry and the shared address-mark negative result. | No caller attaches mark state to an image name, generation, drive or FDC cache. |

The exact production sweep covered `src/vm/machine/fdd.{h,c}`, the shared
`media_save.*` helper, VM session/console FDD binding, the raw FDD provider,
and the FDC, VM-media and Model-339 topology fixtures. The search terms were
`fdd_image_path`, `vm_machine_fdd_`, `media_save`, `ADDRESS_MARKS`,
`get_address_mark`, and `set_address_mark` over tracked `src`, `tests`, CMake
and task records. No additional raw-FDD persistence route or metadata consumer
was found.

## Reference-Implementation Check

This decision matches the interoperability boundary used by reference tools:
[86Box documents raw sector image formats separately from its 86F
surface-format support](https://86box.readthedocs.io/en/stable/hardware/diskimages.html),
[MAME represents a floppy internally as magnetic cells and relies on image
handlers for format conversion](https://docs.mamedev.org/techspecs/floppy.html),
and [PCjs describes IMG as a sector-by-sector binary
dump](https://www.pcjs.org/blog/2016/02/17/). These are behavior and format
references only; no external source, firmware, media or code was copied.

## Focused Baseline

The unchanged Git-Bash CMake/Ninja replay passed:

```text
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

The new TODO admission records the deferred work. The Deleted/Control-Mark
part may be reopened only by an owner-approved fidelity format and lifecycle
contract that defines stable medium identity, versioning, import/export,
single- or multi-file failure atomicity, removal/reinsert behavior, read-only
behavior, and the supported physical-media scope. It must then implement and
test those FDC command/result routes against the already accepted provider
contract. A future Scan receiver needs its own command/transfer contract.
Until then, a later T374 audit must treat both boundaries as intentionally
unsupported and must not use ordinary `.img` success to claim full 765 or
Model-339 closure.
