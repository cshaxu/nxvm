# T498 S5 Release Turbo Matrix Replay

`M5:T498:S5:RELEASE-TURBO-MATRIX-REPLAY:RECORDED`

## Method

The project Release BYOB probe ran each available supported row in Turbo mode
with retirement observation disabled.  A result is accepted only at a semantic
terminal; elapsed execution is containment evidence, not success.  ROM and
DOS inputs remained owner-managed and read-only.  This record intentionally
retains no input path, digest, byte, or firmware-derived trace.

## Current Replay Results

| Profile | CPU | Media | Terminal or disposition | Result |
| --- | --- | --- | --- | --- |
| IBM 5160 Model 268 | 8088 | 360 KB | `installer-ready` | Accepted.  The FDC completed 161 observed terminal operations with no failed terminal. |
| IBM 5170 Model 339 | 80286 | 360 KB | `installer-ready` | Accepted.  The FDC completed 173 observed terminal operations with no failed terminal. |
| IBM 5170 Model 339 | 80286 | 1.2 MB | external-input boundary | Retains S2's observed active pre-80386-incompatible driver component; this is not a DOS or Core decoder repair. |
| DeskPro 386 Model 40 | 80386 | 1.2 MB | no semantic terminal | The four-minute Turbo replay remains in ROM memory diagnostic with no CPU exception and no FDC command.  [S3](t498-s3-model40-startup-chain-ledger.md) owns this owner-boundary disposition. |
| `default-at` | 8086 | 360 KB | `installer-ready` | Accepted.  The FDC completed 180 observed terminal operations with no failed terminal. |
| `default-at` | 80186 | 720 KB | `date-input` | Accepted.  The FDC completed 29 observed terminal operations with no failed terminal. |
| `default-at` | 80286 | 360 KB | `installer-ready` | Accepted.  The FDC completed 191 observed terminal operations with no failed terminal. |
| `default-at` | 80386 | 1.2 MB | `dos-prompt` | Accepted.  The FDC completed 242 observed terminal operations with no failed terminal. |
| `default-at` | 80386 | 1.44 MB | `dos-prompt` | Accepted.  The FDC completed 250 observed terminal operations with no failed terminal. |

## Disposition

Every available non-Model-40 supported row reaches its required semantic
terminal in the current Release Turbo replay.  The remaining Model-40 result
is neither a CPU exception nor an FDC failure.  Its observed predecessor is
the Core-wide recurring tick maintenance mechanism recorded in `TODO(High)`;
the current T498 packet forbids a scheduler repair, so this S transfers that
mechanism rather than adding a machine-specific workaround.
