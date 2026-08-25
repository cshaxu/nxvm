# T467 S6 CGA Firmware, Profile And Guest-Path Proof

The default profile has one CGA control path. `qdcga.c` resets mode 3 by
writing `3D8h`, then programs the retained 6845 port pair; its BDA writes
record firmware service state only. They are not read by VADP. The selected
`INT 10h` writers use the same firmware port capability: mode 06 writes the
CGA mode and colour ports, and mode 03 returns through the mode/CRTC route.
Guest fixtures issue those same ports and write B8000h; Core memory routes the
aperture to VADP, which alone publishes a copied display snapshot for VM
presentation. No profile, firmware, VM or renderer state mirrors mode, VRAM
or a frame.

The profile's sparse port registration admits the retained CRTC index/data,
mode, colour and status leaves. The Model-339 topology smoke independently
confirms the selected CGA-only composition. This is the same VADP owner used
by reset, firmware and guest code; no fallback path was found.

After the configured Debug build, the following system tests passed 4/4:

```text
vm-ibm-5170-model-339-cga-topology-smoke
vm-cga-graphics-system-smoke
vm-cga-640-system-smoke
vm-cga-graphics-dos-smoke
```

The 320 fixture proves a guest `3D8h`/`3D9h` write and B8000h even/odd pixels;
the 640 fixture proves `INT 10h` mode 06 and return to text mode 03; the DOS
fixture boots a supplied test image, reaches a prompt, injects a tiny guest
program and observes the same 320 snapshot. The checks use copied snapshots;
they do not inspect a VM-side display mirror.

This S changes no runtime source or ABI. It corrects the active contract to
the actual S6 proof and preserves the existing single-owner implementation.
The absent IBM-CGA light-pen input lifecycle, incomplete 160x100 programming
tuple, physical/composite monitor output and unqualified default timing values
remain the exact L2 boundaries recorded in Checklists 1 and 2.
