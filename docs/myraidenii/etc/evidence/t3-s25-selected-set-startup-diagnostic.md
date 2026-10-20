# T3 S25: Selected-Set Startup Diagnostic

## Scope

S25 adds an explicit, opt-in selected-set startup probe. It verifies the
owner-supplied archive through the existing inventory verifier, stages only
the two V30 program lanes in a fresh disposable child directory, then runs the
real MyRaidenII Board and V30 path. It does not add a CTest case, modify an
archive, retain staged ROM bytes, print instruction bytes, print registers, or
claim a boot checkpoint.

The probe is a diagnostic tool, not an alternative machine or execution path.
Its fixed two-million-instruction limit is containment only.

## Observation

Both release widths produce the same bounded terminal result:

| Width | Stop | CPU fault code | Executed | Elapsed ticks |
| --- | --- | ---: | ---: | ---: |
| x64 | internal CPU case fault | `0x43e` | 65,587 | 1,049,717 |
| x86 | internal CPU case fault | `0x43e` | 65,587 | 1,049,717 |

A disposable Debug-only transaction observation classified the faulting
`0x43e` access as a CPU memory write. It retained no values, program counter,
instruction bytes, register state, raw trace, or staged input.

The existing S4 retained address summary covered a longer five-million-
instruction run and lists `0x432--0x43c`, not `0x43e`. The already-pinned
reference mapping used only as L2 navigation likewise does not assign a COP
register at `0x43e`. Those facts do not establish the physical behavior of
that address, but they rule out declaring it a known COP configuration register
from this one terminal observation.

## Disposition

No Board, COP, Shared CPU, or public-interface change is made in S25. The
Board must not widen the `0x432--0x43d` register route merely to conceal this
CPU fault. The observed cross-width-stable write is recorded as a Shared X86
Core V30 execution prerequisite: reconcile the effective-address/instruction
path that emits the write against the selected-set run before admitting a
Board mapping change. This preserves S16's explicit control-window ownership
instead of reverting unknown accesses to RAM or a no-op fallback.

The selected-set startup and remaining COP/Video obligations remain open in
T3. This diagnostic is neither a game run nor an integration qualification.

## Validation

The release x64 and x86 probe targets both built through the existing
persistent Ninja graphs. The wrapper completed the bounded diagnostic on both
widths, producing the observation above. The full repository unit aggregates
then passed on both release widths; the repository cleanup safety test required
the normal junction-capable test environment and passed there. No product
binary was rebuilt or published because this S changes only MyRaidenII test,
tool, and evidence files.
