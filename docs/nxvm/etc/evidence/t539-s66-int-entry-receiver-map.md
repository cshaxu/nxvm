# T539 S66 INT-entry receiver map

`machine_interrupt_entry_smoke.c` is the sole Core-machine receiver for the
shared interrupt-entry setup.  It constructs GDT/IDT images, protected gate
state, PIC IRQ and NMI delivery, and public fault observation; these are board
and Core responsibilities, not CPU-local fixture state.

`core_machine_software_int_s50_smoke.c` includes that receiver for S66's real
and protected software-INT forms.  `core_machine_hardware_delivery_s3_smoke.c`
receives only the required mechanical include rename: its VM86 and hardware
delivery cases remain exclusively assigned to S67.  No second descriptor, IDT,
PIC or interrupt-frame setup path is introduced.

The receiver emits `M5:T539:S66:INT-ENTRY:OK` alongside its retained markers.
Focused x64/x86 receiver and both includers pass.  Complete repository-only
unit suites pass 426/426 on each width.  On both widths the T332 lifecycle,
T344 registration and historical-shape, VM lifecycle, and Core CPU/PIC
authority gates pass; documentation governance and `git diff --check` pass.

The scoped code/test/build paths add nine lines and remove eight lines; the
separate receiver-map evidence is 24 lines.  This is test/CMake/documentation
work only: no production/API, Shared, firmware, asset, INI or EXE input changed,
so no EXE rebuild is required.  The similar-issue sweep covers every direct
include of the renamed receiver; S67 retains the different VM86 group.
