# T7 S58: COP Unsupported Command-Family Audit

## Scope

S57 safely established only that the selected external program reaches the
existing unsupported-command admission branch. It deliberately did not copy
the command value. This audit therefore inventories the complete local command
family surface without attempting to identify that external request.

## Single Local Route

`myraidenii_core_cop_classify()` is the single admission classifier.
`myraidenii_core_cop_advance()` is the single pending-command dispatcher.
Every admitted family below reaches one existing executor through that route;
no family owns a parallel direct execution path.

| Family | Local owner | Contract status | Existing Board coverage |
| --- | --- | --- | --- |
| copy | advance dispatch plus checked memory service | Complete admitted L2 family | transfers, prefix failure and reset/admission cases |
| state update | update executor | Complete admitted L2 family | update vectors and failure cases |
| offset add | offset-add executor | Complete admitted L2 family | output order, carry, overlap, failure and reset cases |
| position capture | position executor | Complete admitted L2 family | collision/position result paths |
| hitbox capture | hitbox executor | Complete admitted L2 family | collision/position result paths |
| angle approach | angle executor | Complete admitted L2 family | angle vectors, admission and failure cases |
| numeric/result | numeric executor | Complete admitted L2 family | vectors, failure, chaining and decimal-result cases |

The supported families have established input, output, reset, accepted-prefix
and publication behavior in the existing Board smoke. Their presence does not
license broadening the classifier: the classifier remains a finite list of
complete contracts.

## Unsupported Disposition

The prior residual ledger and unresolved-boundary review identify three
historical command forms for which reference material does not establish a
complete operand, output, reset, failure-prefix, publication or selected-set
use contract. They remain explicit rejections. S57's safe external category
does not distinguish any one of them from every other unclassified command.

No further command family is admissible from the current evidence. In
particular, this audit does not infer an external command identity, alias an
unresolved form to a supported numeric family, accept unknown requests as
no-ops, or turn a source comment into an execution contract.

## Result And Transfer

The existing unsupported return is correct and necessary. A future receiver
may change it only after independently establishing one complete command
family and extending the existing classifier and advance dispatcher together.
The next productive MyRaidenII receiver remains a separately evidenced
hardware or runtime boundary; no code change is selected by this audit.

## Verification

This is a static, source-policy-limited audit. It cross-checks the sole
classifier, sole advance dispatcher, existing Board family tests, and the
residual/unresolved evidence records. Documentation governance and whitespace
checks are required; no ROM/media execution, build, release artifact or
dynamic behavior is claimed.
