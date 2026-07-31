# M4 T2 S1 Firmware Contract Verification

- Reviewed the baseline's mutable `vbiosAddPost` and `vbiosAddInt` tables and
  its reset-time ROM/BDA assembly path.
- Firmware V1 defines session ownership, composition-only registration,
  reset-time freezing, typed POST/ROM/interrupt identities, failure behavior,
  abstract host capability use, and trace events.
- The contract explicitly prohibits firmware access to host paths, UI handles,
  Console handles, or mutable Machine pointers.
- No runtime source, BIOS behavior, host integration, DOS service, or product
  CLI changed.
