# M5 T12 S9 VM Linux Console Host Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/platform/linux/linuxcon.c` and `linuxcon.h`.
- Current destination: `src/vm/platform/linux/linuxcon.c`
  `DC5CFD3AB57E9A32982D7191BA97A04D418BB7852480CE203CC4823F909FAB82` and
  `src/vm/platform/linux/linuxcon.h`
  `755DB004BF280C431ED2A5E6D72DEA266E3A20F0B60605EA3F3D0F009F8AB2E2`.
- Ownership: its VM run/display loops and device state access make it VM-only.
- Changes: direct include paths only; Linux remains excluded from Windows GCC.
- Verification: `docs/verification/m5-t12-s9-linux-console-host-move.md`.
