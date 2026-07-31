# M4 T3 S1 NXVM Console Verification

- Reviewed the baseline's documented command grammar, case-normalizing parser,
  device/media commands, lifecycle commands, and Console exit behavior.
- Reviewed the baseline Win32 console/window split and recorded the M5 target:
  product commands and host events cross only the runtime command boundary.
- `docs/requirements/firmware-nxvm.md` defines the retained Console surface,
  state/presentation rules, debug boundary, and no-process-CLI decision.
- No runtime source, command behavior, DOS behavior, or ntvdm64 policy changed.
