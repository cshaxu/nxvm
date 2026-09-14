# M5 T531 S20 - VM-Machine Responsibility Layout

## Result

The former `src/vm/machine/runtime/` directory was only a naming shell around
NXVM's one machine driver. Its 28 implementation/interface files now reside
directly in `src/vm/machine/`, alongside the existing paused-Debug adapter.
The public `vm_machine_*` contract is unchanged.

`src/vm/machine/media/` remains a distinct child. It is the sole adapter from
Core FDD/HDD providers to Lib storage leases; it is not a SoftPC-style
compatibility layer and has not gained a second owner or media path.

## Mechanical sweep

- Updated all direct production, test, CMake, static-gate and dependency-DAG
  paths to `vm/machine/...`.
- Relocated the executor-state unit smoke from the matching retired test
  directory to `test/vm/machine/`.
- Kept historical evidence unchanged where it truthfully records a prior
  layout. No live source, test, CMake path or include references the retired
  directory.
- The change is relocation plus include-path spelling only: no Core behavior,
  profile behavior, media semantics, public ABI, Common/Lib corpus or
  compatibility route changed.

## Verification

- `verify_vm_machine_owner`, `verify_vm_machine_lifecycle`,
  `verify_vm_provider_composition`, `verify_debugger_capability`, and
  `verify_core_controller_authority`: passed.
- Complete repository-only x64 unit suite: **325/325 passed** in 64.02
  seconds. The initially parallel run exposed an existing native-window test
  resource collision; `library.kvm_window_modal` passed alone and the required
  complete serial run passed without modification.
- x64 and x86 Release builds passed their architecture checks. Their stripped
  task artifacts were refreshed in both required locations:
  - x64: `30A576E1D18ABFBC851C5951E7165E78F5D917AD1609FB1F5E1B258E192B3616`
  - x86: `CE57E2826398986B4EA7358C263FCBDC25705ED9C94B3C476DBD44AEBE1185B6`

## Size and boundary

The tracked diff is mechanical path reconciliation: 28 machine files and one
unit test relocate; all remaining source/test edits are direct include-path
updates. The sole target remains `vm-machine`; Core owns machine behavior,
VM owns its product adapter, and `media/` owns only host-storage adaptation.
