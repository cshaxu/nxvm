# M5 Shared Product-To-X86 Normalization

## Goal

Rename the portable x86 Product corpus from `product` to `x86`, and rename its
former `surface` subsystem to `product`.  The resulting shared corpus is:

```text
src/{lib,emulator,x86}
test/{lib,emulator,x86}
```

`x86` owns portable x86 tooling and IBM-PC product behavior; `x86/product`
owns the former surface behavior.  `core/x86` remains NXVM-private and is not
an alias or substitute for this shared component.

## Scope

- Move the source and test trees, manifests, CMake identities and all test
  registration names.
- Rename the portable component symbols consistently: `product_*` becomes
  `x86_*`; former `product_surface_*` becomes `x86_product_*`.
- Update every consuming Shared, Core and App include, target, test and
  documentation reference without compatibility aliases.
- Retain behavior, public contracts and dependency direction
  `Lib -> Emulator -> x86`; this is an identity migration, not a feature
  change.

## Non-Goals

- Do not rename or move NXVM-private `core/x86`.
- Do not alter command, debugger, hotkey, machine, UI, firmware, INI, media or
  artifact behavior.
- Do not import unrelated SoftPC worktree changes or create a second
  compatibility component.

## Subtasks

| S | Scope | Exit condition |
| --- | --- | --- |
| S1 | Shared, NXVM, MyNES | Move the six-tree Product source/test corpus to `x86`, rename symbols/targets/tests/manifests and every receiver reference, delete the old paths with no aliases, and pass focused x64/x86 configure/build/unit and static component gates. |
| S2 | Shared, NXVM, MyNES | Run the complete required dual-width qualification, rebuild and PE-verify all ten receiving App artifacts, update task evidence and close only after an actual-diff audit proves no old live identity remains. |

## Completion Standard

No live `src/product`, `test/product`, `product_surface_*`, or portable
`product_*` component identity remains.  Every surviving public and internal
name, CMake target, CTest identity, manifest entry, document and consumer uses
the `x86` / `x86_product` vocabulary.  All six shared packages retain their
manifest, Types, dependency and test-boundary proof; all receiving apps compile
and retain their behavior.
