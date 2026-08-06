# M5 T245: Generic Immutable ROM Mapping

## Status

**S1 active.** This subtask defines the mapping contract only. It introduces
no PC/AT address, BIOS byte image, ROM file path, media policy, or NXVM startup
change.

## Objective

Add one generic core-owned immutable byte-image mapping that uses the existing
frozen device-memory route. ROM image selection, generation, POST, and
firmware services remain VM profile responsibilities.

## Frozen Contract

* `core_machine` owns a private copied image and its mapping record. The caller
  retains neither image storage nor a provider callback after registration.
* Registration is valid only while `INITIALIZED`, takes a nonzero physical
  start, nonzero byte count, and copied source bytes, and uses the existing
  non-overlap route registration. It rejects 32-bit range overflow and any
  provider overlap.
* Read and CPU physical fetch return copied ROM bytes through the ordinary core
  route. Write always returns `TYPE_STATUS_FAULT`. A checked range query returns
  `PROVIDER` for READ and `TYPE_STATUS_FAULT` for WRITE.
* Mappings freeze with execution providers. Cold reset clears mutable RAM and
  devices but retains the immutable core image and mapping. Destroy frees core
  image storage after provider teardown.
* No raw pointer, mutable provider owner, alternate firmware shortcut, or
  profile-specific identifier enters the public interface. The mapping is not a
  host file service and does not expose an image enumeration API.

## Implementation And Verification Plan

S2 adds `core/machine/rom_mapping_interface.h`, private core storage, and a
read-only provider registered through `memory.c`. Focused smoke must prove
read/fetch, checked route classification, write rejection, overlap rejection,
freeze rejection, reset persistence, and no partial registration on allocation
or validation failure. Existing default-ROM boot must remain unchanged because
T245 admits no default profile migration.

S3 runs the current GCC/CTest matrix and records
`build/output/nxvm_0_5_0245.exe` plus SHA-256.

## Stop Conditions

Stop for owner direction if implementation needs a second CPU/debugger memory
path, a ROM pointer returned to profile/product, a file/path policy in core,
or a BIOS/POST/startup behavior change.
