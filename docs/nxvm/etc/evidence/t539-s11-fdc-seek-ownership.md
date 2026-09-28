# T539 S11: Pending FDC Seek Ownership

Baseline 9053d917d. This repairs the pending-operation class identified by the
[S10 decision](../architecture/t539-s10-fdc-boundary.md), without moving FDC
or claiming complete READY/Track0 qualification. Only NXVM is changed.

## Cause And Single-Owner Repair

`advance_at` previously chose SEEK versus RECALIBRATE from the current `cmd`
buffer. A later Sense Drive Status, Specify, SIS or other drive's command
could therefore change the earlier operation's completion. Each admitted
operation now records its recalibration boolean alongside its existing
per-drive target/deadline. No second command queue or duplicate cylinder state
is introduced. Reset's existing whole-state initialization clears this field.

The original four-result storage had an unchecked append, while the command
entry allowed unlimited new seeks without SIS and replaced same-unit active
targets. Entry now rejects non-SIS commands while seek results are undrained,
and rejects a new seek/recalibrate on an already seeking unit with the existing
80h invalid-command result. Existing results, targets and deadlines survive.

The capacity bound follows from admission, not dropped results: at most one
active operation per each of four units; a completion converts one active
operation into one retained result; while any result remains, no new operation
is admitted. SIS removes one result, reset removes all. Thus active plus
retained never exceeds four, including simultaneous completions. There is no
larger queue, overflow overwrite, arbitrary eviction or hidden retry path.

Intel 8272A page 11 explicitly requires SIS after seek/recalibrate completion
and permits parallel seeks on four drives. Same-unit overlap rejection is a
bounded emulator admission choice for an unsupported sequence, not a newly
claimed measured silicon response. The existing logical seek/mechanics result
distinction is retained; its inaccurate Not Ready manual claim is removed.
READY/Track0, no-pending SIS and other S10 qualifications remain cutover gates.

## Before/After And Similar-Issue Sweep

New negative controls fail against the old implementation for:

- intervening command changing SEEK completion on every drive;
- same-unit replacement of the active target/deadline on all four units;
- accepting a fifth request while four completions remain undrained.

The fifth-request negative control stops before its deadline; it does not
exercise undefined out-of-bounds memory writes. Final regressions also cover
mixed SEEK/RECALIBRATE completions on four units, all fifteen non-SIS command
families/invalid input while results are pending, FIFO draining and reset.
The earlier 120-case readiness characterization and all original cases remain.

The sweep reads every writer/reader of pending flags, targets, deadlines,
command input, result count/arrays, IRQ and reset. All writes are local to
FDC; scheduler and diagnostic callers only observe. The existing controller
authority gate now rejects current-command reads in `advance_at`; it checks
that its inspection range exists instead of silently passing after a move.

Production C/H +22/-6 (net +16); test C/H +145/-1 (net +144); existing NXVM
static gate +13/-0. Counted with `git diff --numstat` against baseline. The
small added production state is the previously missing pending-operation
identity, not a copy of another owner's state. Shared source/test/manifests,
MyNES and siblings remain unchanged.

## Verification

Full units pass 347/347 on x64 and x86. The x64 run remains live during a slow
206.35-second execution and completes successfully without restart; x86 takes
32.89 seconds. No timing-performance conclusion is inferred from those host
durations. Default external integration passes 20/20 per width (11.76 seconds
x64, 12.51 seconds x86). External profile boots are run once per width.

Other profile boots pass once each: XT 19.34/24.60 seconds, AT 40.42/45.79,
Model 40 74.33/90.94 (x64/x86). Both reusable trees are restored to the default
profile. The specialized static aggregate, six unchanged manifest hash sets,
documentation governance, local document links and whitespace checks pass.
All eight Release 0539 artifacts are rebuilt; their PE machine IDs are
8664/014C as named and contain no compiler-debug sections. Runtime Debug is
retained. INI contents and external asset masters are unchanged. Current alone
owns coordinator acceptance; passing boots do not close the remaining FDC gates.

## Final Artifact SHA-256

Files remain in their existing profile directories under `assets/nxvm/`.

| Product/width | SHA-256 |
| --- | --- |
| default x64 | `4770846704EADB8DE7D7EA5CC656C7CFF8F002F0761C682A8F65D4E86DD90939` |
| default x86 | `793145C8DF8241D7F6B9D82592EBE437FC08914F12D3E0A05D62D85125A0C1F7` |
| XT x64 | `798EB44323CCFF7A81B46FF71C649EBBDC3338C8DEF043B6368731043FD02B08` |
| XT x86 | `CBC0A310DFF38CDFAD0667E10B347271BDF8257BB99F690A04DFDEE8A0AE404C` |
| AT x64 | `59CCC0BFEC2882AF35312ED12423018BC532C04C166075E94AA8283DF2BBD7B5` |
| AT x86 | `693A72E6990F781B0E4C8FFE9A115A8F2039331474E3A04946BE081A53695892` |
| Model 40 x64 | `8DF121073022EB0211C904F5BEDDF66FA7C35DF08FF3369863FB2BD8111EA30A` |
| Model 40 x86 | `1E51C7A476EE8D9FE9D860C615AEEB1A2ECCF243533D5C207EAEC2B49C1B802D` |
