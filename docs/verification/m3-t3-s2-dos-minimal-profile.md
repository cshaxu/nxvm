# M3 T3 S2 DOS-Minimal Profile Verification

- GCC built the no-media runtime composition with strict project-owned
  warnings; its smoke printed `M3:T3:S2:DOS-MINIMAL-PROFILE:OK`.
- The profile excludes disk boot, BIOS, FDD, HDD, and VADP while retaining CPU,
  RAM, ports, PIC, PIT, keyboard, and text display declarations.
- The smoke verified deterministic reset, PIT ticks, keyboard IRQ assertion and
  port delivery, and copied 80x25 text snapshots. It admitted no DOS ABI,
  firmware, host filesystem, graphics, or platform UI behavior.
- Existing trace and M1 baseline targets remained buildable. The imported
  baseline source has no diff and no local artifact was produced.
