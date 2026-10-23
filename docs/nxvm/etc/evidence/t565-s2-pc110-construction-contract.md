# T565 S2 — PC110 Construction, Reset And Decode Contract

## Scope

This is a source-and-evidence contract for the next implementation batches. It
does not add a PC110 App, chipset, firmware payload, executable or guest boot
claim. The contract is deliberately smaller than a board model: it freezes
ownership and the conditions under which a later receiver may add behavior.

## Existing construction boundary

| Existing surface | Proven responsibility | PC110 disposition |
| --- | --- | --- |
| `x86_core_machine_firmware_provider` | Core synchronously accepts a configuration-time provider, validates immutable ROM mappings, and keeps CPU/executor ownership. | Reuse unchanged. A PC110 App may supply its own typed provider/context only through this neutral surface. |
| `x86_nxvm_machine_construction_begin/finish` | Owns the candidate lifetime and publishes one prepared machine construction. | Reuse only if the candidate fields describe an independently proven PC110 topology. It must not manufacture AT defaults. |
| `x86_nxvm_machine_pc_at_construction_create` | Prepares an AT profile snapshot and fixed AT firmware roles. | Explicitly excluded. Its AT slot model is not a PC110 firmware contract. |
| Current App `machine_plan.c` files | Each fixed product owns its model-specific preparation and transfers a single construction to the shared Machine adapter. | Pattern to follow in a future `app-mypc110`; no App-to-App dependency is permitted. |

The legal flow is:

```text
app-mypc110 machine plan
  -> typed PC110 board/firmware binding
  -> x86-nxvm neutral construction publication
  -> x86-core sole executor, reset and immutable-ROM owner
```

There is no runtime model switch, second executor, BIOS interception or reuse
of an AT descriptor as a PC110 disguise. DSTN and TFT remain separate
build-fixed firmware pairings; their release admission is separately blocked
on lawful external asset/provenance approval.

## Reset and decode disposition

| Row | Owner for a later implementation | Evidence grade | Contract now | Direct falsification test | Blocker |
| --- | --- | --- | --- | --- | --- |
| BRD-1 population/construction | `app-mypc110` plan plus one PC110 board binding | Source-backed topology; exact revision unresolved | One plan declares only the fitted controller population it can prove. | Construct/destroy rejects an omitted or duplicate attachment atomically. | Selected board revision and fitted-option record. |
| BRD-2 cold/warm reset, RAM and ROM/shadow | PC110 board binding; Core remains reset executor | Reconstructed/reference only | Reset reasons may be represented only after every initialized field has a source or an explicitly labelled L2 rule. No POST dump becomes a reset default. | Cold/warm reset tests assert only source-backed fields and reject conflicting maps before publication. | VL82C420 reset/shadow register values. |
| BRD-3 A20 and standard IRQ/DMA/PIT/RTC routes | PC110 board binding wires existing controllers | Existing Core route support, PC110 route incomplete | Each standard route has one attachment owner; the board does not mirror controller state. | Composition test proves one handler per claimed route and no reverse App dependency. | PC110-specific decode and gate facts. |
| SIO-1 configuration entry/exit | PC110 board binding owns the Super-I/O configuration decode | Component documentation for 3F0h sequence | One configuration state owner; endpoint receivers cannot install overlapping configuration handlers. | Unlock/index/data route test rejects overlapping port registration. | Exact board enable policy. |
| SIO-2 FDC/IDE mode routing | Storage receiver, attached only after SIO ownership | Component/reference evidence; fitted route unresolved | FDC/ATA endpoints are absent until the Board establishes the selected mode. | Mode selection proves only one endpoint owns each conflicted port. | Exact PC110 routing/revision behavior. |
| SIO-3 reset defaults/preservation | PC110 board binding | Documentation conflict/unresolved | No blanket-zero or blanket-preserve initialization. | Future test must name each source-backed reset bit. | Selected SMC revision/default columns. |

## Firmware contract

The S1 candidate hashes remain research identities, not repository inputs or
release masters. A later Firmware receiver must provide an owner-approved
external asset record for every compiled pairing, retain master/derived hashes
outside the repository and prove the selected panel/BIOS relationship. The
future App passes bytes by a typed PC110 firmware binding; it does not select a
slot in the AT helper, infer a firmware from an INI, or fall back across panel
variants.

## Implementation-ready versus blocked work

| Status | Work |
| --- | --- |
| Implementation-ready | Create the `app-mypc110` source/test root only when the first concrete board binding and its source-backed population are selected; use the existing neutral Core firmware provider and one construction lifetime. |
| Implementation-ready | Add independent negative construction tests for duplicate/missing attachment ownership once a PC110 plan exists. |
| Blocked | Chipset reset tables, ROM/shadow behavior, SIO reset defaults, panel-specific boot and all firmware embedding remain blocked on the rows named above. |
| Deferred | Display/font, PCMCIA/flash/storage, keyboard/pointer/power and audio/aux retain their existing receiver proposals. They may not create an alternate construction or decode owner. |

## Verification and limits

This contract was checked against the actual four current App plans,
`x86-nxvm` preparation surfaces and `x86-core` firmware interface. It adds no
runtime validation because it adds no runtime behavior. Documentation
governance and whitespace checks are the applicable S2 checks. No desktop,
guest, firmware or hardware validation is claimed.
