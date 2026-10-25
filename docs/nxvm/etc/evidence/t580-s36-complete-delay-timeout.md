# T580 S36 — Complete BIOS Delay Routine: Durable Timeout

## Scope

S36 repeated S34's `F000:53D0h`--`F000:5405h` observation with isolated,
durably logged x64 and x86 processes. The temporary diagnostic records entry,
return and first caller only if those events occur.

## Observations

- Both processes remained live for approximately 40 wall-clock minutes.
- Their CPU time remained within one second throughout (about 2,307 seconds
  at containment), so there was no host-width split, crash, fault report or
  early exit.
- Neither returned before S-local wall-time containment. The diagnostic emits
  its result only after its run loop exits, so neither log supplies an
  entry/return/caller record.
- The temporary Scheduled Tasks, processes, batch launchers and output logs
  were removed after containment. The temporary diagnostic mode was removed.

## Disposition

This is not evidence that `F000:53D0h` does not return, nor evidence for a
receiver, POST, Setup, boot or guest compatibility claim. It establishes only
that the whole routine is too expensive for a practical diagnostic boundary in
the current instruction-level runtime. S33 remains the retained dynamic fact:
the real `EA38h` outer delay returns to `F000:53DDh` on both widths.

No production source, public ABI, firmware/media, asset or artifact changed.
Further PC110 work must begin with a source-backed guest-visible contract, not
a longer undifferentiated run budget.
