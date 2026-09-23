# T484 S19 XT PPI Parity And I/O-Check NMI

`M5:T484:S19:XT-PPI-PARITY:OK`

## Source Boundary

The rendered IBM *PC/XT 5155/5160 Technical Reference* system-board 8255A
I/O bit map, already qualified by T484 S7, is normative.  It assigns PPI-B
`61h` bit 4 to active-low RAM-parity checking enable and bit 5 to active-low
I/O-channel-check enable; PPI-C `62h` bit 6 is positive I/O-check and bit 7
is positive RAM parity.  The source gives the logical lines and their control
relation, but not a synthetic fault generator or a physical error duration.

Read-only PCjs `machines/pcx86/modules/v2/chipset.js` corroborates the same
four masks and the normal `99h` PPI mode.  IBM remains the authority; neither
PCjs behavior nor source text enters NXVM.

## One Owner And Route

`core_machine_xt_ppi_keyboard` remains the sole XT `60h`--`63h` owner.  It
owns the two current PC6/PC7 input levels, PPI-B latch, active-low eligibility
test and one outstanding request condition.  A board/device source calls the
single typed `core_machine_set_xt_ppi_fault_input` entry; it cannot write a
port latch, CPU state or NMI flag.  The PPI calls its bound request operation
only when an asserted source is enabled.  Core alone observes its NMI mask and
sets the CPU's existing pending-NMI flag.  Unmasking asks the PPI owner to
retry its still-pending input; no board parity state, PC/AT alias or mirrored
fault latch exists.

Mode `99h` exposes the source inputs through PPI-C's upper input nibble.  A
disabled source remains visible at `62h` but cannot request NMI.  Reset clears
both source inputs and the PPI request condition.  This is logical L3 for the
manual-defined port relation and real external input endpoint.  Physical RAM
parity detection, I/O adapter fault origin, line duration and phase remain
unimplemented transfers; no L2 timing estimate was added.

## Focused Proof And Sweep

`core-machine-xt-ppi-keyboard-smoke` proves:

- PC7 parity status survives disabled PB4 and requests NMI only when PB4 is
  enabled;
- PC6 I/O-check status survives the global Core NMI mask and requests NMI
  after unmask; and
- reset clears both source inputs, while the retained XT keyboard and default
  8042 paths still behave as before.

The defect-class sweep used:

```text
rg -n -C 2 "CORE_MACHINE_XT_PPI_FAULT|set_xt_ppi_fault|io_check_asserted|ram_parity_asserted|xt_ppi.*nmi" src tests
rg -n -i -g "*.c" -g "*.cpp" -g "*.h" "DISABLE.*PARITY|IO.?CHECK|RAM.*PARITY" <external>/86Box/src
rg -n -i "port.?61|io.?check|parity.*check" <external>/pcjs/machines/pcx86/modules
```

All NXVM production hits are the single XT PPI owner, its narrow Core request
edge and the existing generic NMI refresh point.  PC/AT planar and DeskPro D4
parity owners were intentionally retained as distinct boards and passed their
focused regressions.

Focused build and tests passed on the new executable:

```text
cmake --build build/mingw-gcc-x64 --target core-machine-xt-ppi-keyboard-smoke -j 1
ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R "core-machine-xt-ppi-keyboard-smoke|core-machine-planar-parity-nmi-s3-smoke|core-machine-d4-platform-s4-smoke"
```

All three selected current-gate tests passed.
