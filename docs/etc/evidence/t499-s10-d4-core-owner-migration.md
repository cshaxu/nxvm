# T499 S10: Model-40 D4 Core-Owner Migration

`M5:T499:S10:D4-CORE-OWNER:OK`

## Result

The selected D4 controller now has one mutable owner: `core_machine_d4_memory`.
Core owns the compatibility backing, D4 diagnostic/control/setup state, parity
mask, physical decode, address callbacks and reset defaults. The Model-40
profile contributes only immutable even/odd ROM lanes and frozen reset values
while composing the Core plan; Core copies the lanes during machine creation.

The removed route is:

```text
profile D4 state -> profile callbacks/observer -> generic Core dispatch
provider lifecycle -> profile D4 reset
Core plan -> profile-owned parity-mask pointer
```

The resulting route is:

```text
frozen Model-40 construction value -> Core D4 construction -> Core state/decode/reset
```

There is no profile forwarding callback, duplicate D4 backing store or second
reset path. The generic Core memory-device capability remains for unrelated
devices; the selected D4 controller no longer uses a profile-owned instance of
it.

## Implemented Admitted Relations

| Relation | Disposition |
| --- | --- |
| D4 compatibility, replacement and high alias decode | Manual-L3 selected D3PE relation, implemented by Core. |
| D4 control write-protect and ROM replacement selection | Manual-L3 selected D3PE relation, implemented by Core. |
| D4 diagnostic/control/setup readback and reset defaults | Manual-L3 where specified; the selected setup reset byte is External-L2 cross-validated configuration data. |
| D4 parity fault and ordinary-memory-write clear condition | Existing selected D4 behavior, now owned by Core. |
| Setup-byte dynamic remapping | The selected two-MiB setup-low `1` disconnects extension RAM below `F00000h`; Core exposes it as an External-L2 open-bus receiver. Other selections remain unsupported. |

## Reset Boundary

Core reset restores D4 control, parity and mutable setup state from the frozen
construction defaults. This was required because the superseded profile reset
had previously performed that controller reset. ROM backing is immutable after
Core construction and is not regenerated on reset.

## Focused Evidence

The following freshly built focused executables pass:

```text
vm-model40-d4-map-s16-smoke
vm-model40-d4-parity-s22-smoke
vm-model40-private-composition-s7-smoke
vm-model40-integration-s8-smoke
```

Together they cover D4 aliases, A20 interaction, replacement/write-protect,
reset defaults, parity/IOCHK, private composition absence and the selected
Model-40 controller composition. S11 still requires scheduler/lifecycle replay
and the Model-40 semantic boot replay; S10 alone does not claim that result.
