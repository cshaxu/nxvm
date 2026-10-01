# M5 T539 S99 final CPU path cleanup

## Corrected boundary

S97 initially left `cpu_idt_privilege_entry_smoke.c` in NXVM because it
included `device_support.h`. S99 audited the actual symbols and found that the
only use was `CORE_MACHINE_BIT_IS_SET`, a generic bit-test macro. The receiver
therefore has no App/board dependency and moves to the Shared CPU corpus.

Its local replacement is the equivalent expression:

```c
(eflags & flag) != 0u
```

No API, fixture, timing or instruction semantic changes are introduced.

## Final fixture ownership

`machine_idt_privilege_pic_board_smoke.c` is the retained NXVM board receiver.
It now directly includes the canonical Shared CPU fixture. The obsolete NXVM
forwarding header is deleted, so there is no second fixture path.

## Verification

- Shared IDT CPU and retained PIC-board receiver: x64 **2/2**, x86 **2/2**.
- T317 test-type vocabulary, CPU/PIC authority, documentation governance,
  Shared manifest, Shared corpus and `git diff --check`: passed.
- Detached repository-only full units: x64 **466/466**, x86 **466/466**.

No executable rebuild is required for this test/CMake/documentation-only
ownership cleanup.
