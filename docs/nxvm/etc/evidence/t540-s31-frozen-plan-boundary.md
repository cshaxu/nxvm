# M5 T540 S31 Frozen Plan Boundary

## Actual change and owner review

The product profile plan is the construction source for the selected machine.
`core_machine_plan_create` copies its configuration once into the sole board
construction plan; the timing-rule setter copies the rules into that same
plan. The VM session no longer mirrors either complete value. On later memory
reconfiguration, Core owns the actual RAM capacity and the VM retains its
runtime request; the frozen construction plan is not rewritten.

The board plan validates topology, keyboard wiring, controller timing rules,
declaration uniqueness, seam and timing dispositions before allocation. The
neutral transaction contract is validated by `core_machine_create_internal`
before its allocation, not redundantly by board-plan validation. The existing
invalid-contract test now proves both sides of that boundary: board validation
accepts the board plan, but `create_from_plan` rejects the neutral contract,
publishes no machine, and allocates no successful result. Constructor effects
and board topology application remain the bounded S32 and S33 receivers.

The source/test diff deletes 33 lines and adds 29 (net -4); it introduces no
type, getter, adapter, second parser, profile-name branch, runtime controller
state or alternative constructor. The four profile factories still supply
their original configuration, timing and topology values. The product profile
plan retains asset/firmware lifetime; the board construction plan retains
topology and declarations; Core copies only its own runtime facts. These are
distinct lifetimes, not parallel mutable machine states.

## Verification

- Full repository-only units: x64 **469/469**, x86 **469/469**.
- Specialized NXVM/Shared boundary target: **121/121** build/gates passed.
- Existing focused plan, invalid-transaction, composition and reconfiguration
  tests are included in both complete unit runs.
- External fixed-profile boot checkpoint, once per width: default, 5160 XT,
  5170 AT and DeskPro Model 40 all passed, **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` confirms four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `5A0F66985B58CF1F204F09A4B0E61C7590457A24FFFF4E43CBABC28663ED857C`, `090D51DE07D9E87ED1ABB43A880519882D2CB2ECF86407B65BC9CDEA609E9AD5`.
  - Default: `E6B6E99B21ADB3BECE123BDBE226FC305F561531874FBCA901BAFC03573C47D0`, `9B5B54FCD9DE5C4689BD1AA09F87EC53E91B8792663C848DB21E055BFA118EA1`.
  - XT: `0C4C88A4ACB7E855227A0254CAE2B5E879AE2C96AB814DF0ADC07AFB502F385A`, `B43A06ECBE79B5670210C734F53C9CBE97961DF7706B233DD927CF631CC2DDB8`.
  - 5170: `67D7DAB23D74ECDA5B7138AF3AED603A4A9311D5959F4CD1A326A5D21AC1B777`, `8FE86350121CE378CC1D93A9F328CFFD85609345A9349FED5557AD72ABA9299F`.

No INI, protected asset, firmware input, Shared source/test or MyNES file is
changed. This S makes no timing-grade or behavior upgrade. S32 receives the
still-mixed constructor preflight/allocation; S33 receives controller creation
and plan topology application. T540 remains open.
