# M5 T227: KBC Controller Protocol, Phase 2

## Scope

T227 extends the T226 byte path with a deliberately bounded 8042/keyboard
command contract. It owns no host input queue and adds no AUX mouse/IRQ12.

## Contract

- Keyboard commands admit scan-set query/select, controller translation,
  LED write, typematic write, reset, enable/disable scanning, identify, ACK,
  RESEND, and defined invalid-command behavior.
- The KBC owns pending command parameters, active scan set, translation flag,
  LED state, typematic configuration, and elapsed-tick deadlines. Profile and
  platform never inspect or mutate them.
- Controller and typematic delays advance only when the core elapsed scheduler
  refreshes KBC. Host `Sleep` is a test watchdog only.
- Every command response, error, and delay has a focused owner-local port
  probe. Unimplemented commands respond with the documented RESEND/error value
  rather than impersonating wider compatibility.

## Evidence

S1 defines the precise command table and timing constants with probes. S2
implements state in `core/machine/kbc`. S3 runs command/timing probes, DOS
keyboard corpus, retained FDD prompt/EDIT.COM, Console/window, and current
GCC/CTest gates before recording the T227 artifact/hash.
