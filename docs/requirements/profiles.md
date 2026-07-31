# Profile And Firmware Provider Requirements

## Purpose

Profiles select a complete, bounded machine shape without making `core` know a
product, model name, BIOS vendor, DOS policy, or host OS. A profile is
session-owned, selected before reset, frozen before execution, and destroyed
with its runtime session.

## Profile Families

### NXVM Machine Profiles

An NXVM machine profile describes a bootable computer. Its registry key uses
`nxvm.machine.<name>`, beginning with `nxvm.machine.pc_at_builtin`.

Each descriptor declares:

- CPU class/features and required core capabilities;
- RAM, ROM, port, IRQ, DMA, and memory-map topology;
- enabled devices and their connection graph;
- firmware provider and required service/device identities;
- boot/media policy and presentation capabilities; and
- reset-vector, trace checkpoints, and profile-specific acceptance probes.

`nxvm.full_pc` is the M3 baseline-adapter descriptor only. It remains an
evidence-preserving compatibility alias while M5 migrates the current path, but
it is not the canonical identifier for new code. M5 introduces only
`nxvm.machine.pc_at_builtin`; it preserves the current M1 behavior through
that profile.

Future examples are `nxvm.machine.compaq_deskpro_386` and
`nxvm.machine.ibm_pc110`. They require their own owner-approved design gate,
device inventory, ROM/provider decision, legal review, fixture policy, and
implementation breakdown. PCjs and PC110-emu may be behavior and architecture
research references only unless a separate source review authorizes copying.

### NTVDM64 Execution Profiles

An ntvdm64 execution profile describes a non-booting DOS capability set. Its
key uses `ntvdm64.execution.<name>`, beginning with
`ntvdm64.execution.dos_minimal`.

Each descriptor declares:

- selected CPU and core capability subset;
- optional device/services subset, including any BIOS-compatible provider;
- owned DOS loader/backend configuration and supported ABI level; and
- host capability and product-policy requirements.

It never selects POST, a ROM boot chain, a full-machine BIOS, or a real hardware
model by implication. Future execution profiles such as text, graphics, XMS,
or EMS compatibility sets are admitted only through their owning design
milestone and corpus evidence.

## Registry Contract

`runtime` owns two composition registries: one for machine profiles and one
for execution profiles. A descriptor has a stable id, ABI version, owner,
capability requirements, composition callback, teardown rule, and verification
schema. Product code requests a profile by id; runtime rejects a mismatched
product family, missing required capability, duplicate id, unsupported ABI, or
late profile change.

```c
typedef enum {
    NXVM_PROFILE_MACHINE,
    NXVM_PROFILE_EXECUTION
} nxvm_profile_family;

typedef struct {
    const char *id;
    unsigned int abi_version;
    nxvm_profile_family family;
    const char *owner;
} nxvm_profile_descriptor_v1;
```

The public descriptor is metadata. Composition callbacks are private to the
owning module and receive only abstract Machine, registry, and host-capability
contracts. A machine profile cannot compose DOS; an execution profile cannot
implicitly compose a boot ROM or product-specific whole-machine topology.

## Firmware Providers

Firmware is selected by a machine profile through a provider identity, not by
global BIOS code. M5 implements `firmware.provider.pc_at_builtin`, which is the
project-owned firmware described in `firmware-nxvm.md`.

Future provider kinds are:

- `builtin`: project-owned firmware that may ship with the product;
- `external_rom_bundle`: user-supplied immutable ROM files described by a
  validated manifest; and
- `absent`: no firmware, permitted only for an execution profile that has an
  explicitly specified BIOS-compatible service subset.

Providers declare compatible machine-profile ids, CPU requirements, memory-map
regions, reset vector, required devices, and an expected verification schema.
They do not select host paths, create a user interface, or open media directly.

## External ROM Bundle Manifest

An external bundle is a local, user-provided JSON manifest with schema version
1. It is not a downloaded catalog, installer, repository asset, or release
dependency. The implementation must reject a malformed document, unknown key,
relative escape, duplicate region, overlapping mapping, wrong hash/length,
incompatible profile, unavailable device, or reset vector outside a declared
read-only ROM region.

```json
{
  "schema": 1,
  "bundle_id": "user.example.pc386",
  "machine_profile": "nxvm.machine.example_386",
  "redistributable": false,
  "roms": [
    {
      "role": "system_bios",
      "path": "bios.bin",
      "sha256": "lowercase-hex-sha256",
      "load_address": "0x000f0000",
      "length": 65536,
      "read_only": true
    }
  ],
  "reset_vector": "0x000ffff0",
  "required_devices": ["core.pic", "core.pit"]
}
```

The manifest may name provenance and license metadata for local diagnostics but
does not make a ROM redistributable. Award, Phoenix, IBM, Compaq, and other
third-party ROMs remain BYOR material: no binary, hash catalog, vendor code,
or derived code is committed or shipped. M5 does not implement external ROM
loading; a future owner-approved profile design task must specify the parser,
path-containment strategy, mapping behavior, and validation probes.

## Admission And Verification

Every new machine profile requires a dedicated design gate before code:

1. record hardware/device and firmware-provider provenance;
2. define CPU, mapping, device, boot, and presentation contracts;
3. define ROM/media legal and fixture boundaries;
4. choose bounded reset/POST/boot trace checkpoints; and
5. produce the implementation breakdown and corpus/regression plan.

Every new execution profile similarly requires an owned DOS/backend design
gate. Neither profile family is admitted by a product flag alone.
