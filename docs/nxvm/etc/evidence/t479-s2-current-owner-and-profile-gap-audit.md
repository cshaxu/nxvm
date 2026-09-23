# T479 S2 Current Owner And Profile-Gap Audit

`M5:T479:S2:HDC-OWNER-AUDIT:OK`

The sweep covers `controller_interface.h`, `hdc.[ch]`, `machine_board.c`,
`machine_plan.c`, default-PC/AT profile declaration/materialization, Model-40
composition, VM HDD media and every HDC smoke target.  No firmware `1F0h`
direct port path or second mutable HDC/media cache was found.

| Route | Current mutable owner and construction path | Test corpus | Disposition |
| --- | --- | --- | --- |
| Default-AT ATA | `vm_session_machine_devices_materialize_hdc` copies `profile->hdc_pio` into `core_machine_hdc_config`; `core_machine_plan_configure_hdc` copies it into one `core_machine_hdc`. VM HDD owns media bytes. | `core-machine-hdc-smoke`, `vm-hdc-port-smoke`, `vm-hdc-hdd-boot-smoke`, PC/AT composition smokes. | Retain behavior, but replace the ATA-named profile payload and implicit zero-valued protocol with the S3 immutable personality. |
| DeskPro Model 40 | `vm_session_model40_materialize_controllers` makes the same plan call with `COMPAQ_WD_40MB`; Core alone owns command/phase/IRQ. The FDC remains the bit-7 owner on the shared `3F7h` read. | `core-machine-compaq-hdc-s5-smoke`, `core-machine-compaq-hdc-machine-s5-smoke`, `vm-model40-hdc-s26-smoke`, Model-40 integration smoke. | Retain the one-owner implementation. S4 deletes only duplicated configuration spelling, not the distinct shared-port rule. |
| IBM 5170 Model 339 | Resolver/profile has no HDC and rejects HDD by design. | IBM 5170 root/composition profile tests. | Preserve exactly. S5 must add a distinct fixed-disk descriptor, never flip this descriptor or grant a runtime HDD option. |
| IBM 5160-268 | No XT descriptor/composition or HDC binding exists. The existing generic materializer assumes a PC/AT task-file payload, so it cannot receive the `320h` XT controller. | No current XT fixed-disk test exists. | S6 must consume the later approved XT profile receiver and use the S3 discriminated Core input. It cannot route an XT controller through `hdc_pio`. |
| ESDI | No ESDI enum, port map, command path or profile binding exists. | No ESDI test exists. | S7 owns the source-bounded personality and tests. It must not alias ATA merely because both use `1F0h` task-file addresses. |

## Exact implementation seam

`core_machine_hdc` already provides a useful single mutable state object and
one installed Core port provider.  Its defect is narrower: the public
`core_machine_hdc_config` is an ATA-shaped flat record, while the generic VM
route duplicates its fields in `vm_profile_default_pc_at_hdc_pio`; the ATA
protocol is also selected accidentally by zero initialization.  Model-40
duplicates a literal configuration because its profile composition predates the
resolver route.

S3 therefore has one replacement seam:

1. define a copied, validated discriminated HDC personality in Core;
2. give each host grammar only its required static port/configuration fields;
3. make every profile materializer supply that one declaration to the existing
   plan path; and
4. retain one `core_machine_hdc`, one media registry and one IRQ publication
   path.

This deletes the `hdc_pio`-as-controller-identity duplication and prevents a
future XT or ESDI personality from inheriting invalid ATA fields.  It does not
extract a virtual controller framework or duplicate command state.

## Time and media boundary

`core_machine_hdc_advance` supplies the current causal command completion
path. It has no source-qualified service deadline; S2 records no deadline
claim and does not derive one from host I/O or an emulator delay.  The VM HDD
provider validates geometry and owns image bytes/persistence only.  These are
the retained Core-time and VM-media owner boundaries for S3--S8.
