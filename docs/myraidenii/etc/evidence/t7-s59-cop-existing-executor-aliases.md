# T7 S59: COP Existing-Executor Alias Completion

## Scope

S58 retained every command whose complete local contract was unknown.  This
receiver instead admits only four finite aliases for which the local state,
input layout, output publication and failure behavior already belong to an
existing executor.  It does not add a COP path, table, device, public
interface or Shared dependency.

## L2 Basis And Boundary

The behavior comparison used `rtl/raiden2_cop_cmd.sv` from the local
`Arcade-Raiden2_MiSTer-l2` checkout at revision
`27021027c5c3db226c5e77a27c5390022b8aa1f2` (GPL-3.0).  It is L2 behavioral
evidence only.  No external source, table, literal implementation or
protected media data was copied or transliterated.

The reference shows four aliases that map to existing local contracts:

- one subtractive state update using the existing secondary object layout;
- one direct-vector result form that also stores the existing angle byte;
- one distance form that publishes the existing aggregate result only; and
- one distance form that publishes to the alternate existing object field.

The classifier directs each alias to the existing update or numeric executor,
so checked-memory failure, pending completion, reset and result-publication
semantics remain single-owned.

## Deliberate Exclusions

The observed collision-mirror forms require persistent direction/flip state
that the local collision contract does not own.  They remain unsupported.
Previously documented historical unknown forms likewise remain explicit
rejections.  This change does not infer selected-program command identity or
claim hardware-exact behavior.

## Verification

The existing Board smoke was extended through the production CPU/Board route
for subtractive update, direct-vector stored-result, no-store distance and
alternate-store distance behavior.  Its focused target passed on x64 and
x86.  The complete MyRaidenII repository-only unit set then passed on both
widths: 11 executables on x64 and 11 on x86.

No owner-ROM run, gameplay qualification, desktop verification, artifact
publication or release claim is made by this S.
