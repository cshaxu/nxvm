# T450 S11 KBC 8042/NMI Original Function And Timing Checklist

> S19 direct PDF check: scan with OCR text layer (not born-digital); rendered
> pages govern pin/timing tables. Chip facts are sufficient; keyboard and AT
> board phases remain L2. No external emulator is primary.

## Scope And Source Key

This complete KBC source universe transfers unchanged to T450 S12. `UPI`
means Intel *UPI-41A/41AH/42/42AH User's Manual*, order 231318-006 (Oct.
1993); page references are printed manual pages. `AT` means IBM *Personal
Computer AT Technical Reference*, 1502243 (Mar. 1984), system-board pages.
The UPI manual specifies a programmable controller substrate, while AT
specifies the selected 8042 firmware-visible host interface, keyboard serial
handling and board wiring. Keyboard command protocol is included only where AT
assigns it to this selected controller boundary.

## T464 S1 Cross-Validation

The rendered local PDFs remain normative: Intel defines the programmable
UPI-42 substrate and IBM defines the selected 5170 wiring and firmware-visible
contract.  The PDFs are OCR scans, so their pin/timing tables were checked on
rendered pages.  No emulator replaces a manual statement.  The following
read-only comparison was completed on 2026-08-25: 86Box `4fef696`
(`src/device/kbc_at.c` and `keyboard_at.c`), local PCjs `c7f21b4fa`
(`machines/pcx86/modules/v2/keyboard.js` plus its 8042 material), the local
Bochs 2.6 compatibility tree (`iodev/keyboard.cc`), current MAME
`src/devices/machine/at_keybc.cpp`, and current QEMU `hw/input/pckbd.c` /
`ps2.c`.  MAME is especially useful corroboration because it runs the selected
8042 ROM through an emulated UPI-41 core; QEMU, 86Box and Bochs instead carry
behavioural compatibility models.  PCjs is a useful AT integration model, not
a source for chip-level timing.

| Checklist rows | Manual conclusion | Cross-check conclusion | Final tier |
| --- | --- | --- | --- |
| KBC-R1--R2 | UPI host buffers and IBM 0060h/0064h status are specified. | 86Box, Bochs, MAME and QEMU all retain the single host command/data and output/status interface. | Manual L3 for register semantics; exact firmware service delay requires board L3 or falls back to L2. |
| KBC-R3--R4 | IBM specifies the selected commands, command byte and ports. | 86Box/Bochs/QEMU implement 20h/60h/AAh/ABh/AC/ADh/AEh/C0h/D0h/D1h/E0h/F0h--FFh; MAME obtains the selected behaviour from ROM. PS/2/AUX commands are later-machine extensions. | Manual L3 for selected commands whose state is defined by the selected board. AC's RAM/PSW scan-code dump requires the selected controller firmware, so it falls back to L2; extensions are Other L3 only when an explicitly selected profile admits them. |
| KBC-F1 | UPI defines internal instruction/interrupt relations but not IBM's ROM schedule in Core ticks. | MAME confirms that instruction cadence belongs to the UPI/ROM model; the behavioural models intentionally do not agree on a universal delay. | Manual L3 relation; board L3 input or fallback to L2 for selected cadence. |
| KBC-F2--F3 | IBM defines the AT serial frame, error/timeout rules and keyboard-visible protocol boundary. | 86Box, Bochs, MAME and QEMU all model a distinct keyboard endpoint; their concrete delay choices differ. PCjs deliberately abstracts host keyboard delivery. | Manual L3 for frame/error/protocol semantics; board L3 input or fallback to L2 for clock-derived delivery and exact selected-keyboard coverage. |
| KBC-F4--F5 | IBM defines IRQ1 and reset/A20 output-port bindings. | All five references retain separate output-buffer/IRQ and A20/reset paths; QEMU and Bochs explicitly reduce the pulse to reset rather than claim a universal pulse model. | Manual L3 for logical publication/binding; board L3 input or fallback to L2 for PIC/CPU visibility. The 6-us electrical pulse is L4. |
| KBC-F6--F7 | IBM separates NMI board logic from the KBC; UPI electrical tables are not Core tick contracts. | The reference models likewise keep NMI outside the KBC and do not derive Core time from AC limits. | Manual L3 negative boundary; L4 electrical limits excluded. |
| KBC-T1--T4 | IBM fixes 5170 topology and owner separation. | 86Box/Bochs/MAME/QEMU agree on 0060h/0064h, IRQ1 and A20/reset; AUX/IRQ12 is an explicit PS/2/later extension. | Manual L3 for 5170 topology and separation; Other L3 only for selected extension profiles. |
| KBC-T5 | No source supplies a selected Core tick conversion. | Behavioural models use incompatible schedulers, confirming that none is a substitute for a board contract. | board L3 when a selected profile supplies it; otherwise fallback to L2. |

Every row below uses this final tier. `Manual L3` means a directly selected
Intel/IBM semantic relation; `Other L3` is reserved for an explicitly selected
later-model profile corroborated by a named emulator; `fallback to L2` means
the source provides neither a selected Core-tick relation nor a board value.

## Host Interface, Controller And Port Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| KBC-R1 | UPI pp. 4, 20--22, 56--61 | DBBIN, DBBOUT and STATUS provide one host data/command input, one output and status flags. A0 selects command versus data; host reads clear OBF and writes set IBF. | RESET clears status/flags and disables internal interrupts; port pins return to their reset state. | UPI host WR/CS produces the IBF request; host RD clears OBF. | L3 generic host-buffer function. |
| KBC-R2 | AT pp. 1-37--1-40 | AT maps output buffer read and input buffer write to 0060h and status/command to 0064h. Status reports OBF, IBF, system, command/data, inhibit, transmit timeout, receive timeout and parity. | 0060h output read clears OBF; power-on clears the system flag. | Status may be read at any time; input is not a second output path. | L3 selected host interface. |
| KBC-R3 | AT pp. 1-40--1-42 | Commands 20h/60h read/write command byte; AA/AB test controller/interface; AC dump; AD/AE disable/enable keyboard; C0/D0 read input/output port; D1 writes output port; E0 reads test inputs; F0h--FFh pulses output-port bits. | Data after 60h/D1 is a command parameter; output-buffer precondition applies to C0/D0; tests produce their stated result bytes. | Pulse command holds selected low output bits about 6 microseconds. | L3 command/result function; pulse duration L4 unless selected by board timing. |
| KBC-R4 | AT pp. 1-40--1-44 | Command byte controls PC compatibility/translation, keyboard enable, inhibit override, system flag and output-buffer-full interrupt; input/output/test ports carry the stated board inputs and reset/A20/keyboard lines. | Bit 4 disables keyboard clock/data interface; reserved command-byte bits are written zero. | Port bits are board signals, not Core-owned CPU or NMI storage. | L3 selected register/port function. |

## Keyboard, Interrupt, Reset And Timing Universe

| ID | Source | Finite function | Reset/cancellation | Timing or signal relation | Disposition |
| --- | --- | --- | --- | --- | --- |
| KBC-F1 | UPI pp. 14--21, 56--61 | The UPI oscillator/state/cycle counters execute controller firmware; DBBIN IBF and timer overflow are its two internal interrupt sources. | RESET clears PC/stack/status/timer and disables interrupts. | One instruction cycle has five states; UPI interrupt latency is four to seven clocks. | L3 chip relation; selected controller clock/firmware cadence is L2. |
| KBC-F2 | AT pp. 1-31--1-43 | AT 8042 receives keyboard serial frames, checks parity, translates scan codes, supplies output bytes and accepts keyboard-bound data. | Parity failure or receive timeout places FFh and status error; controller disables receive interface until host accepts a received byte. | Keyboard frames have start, eight data, odd-parity and stop bits; receive timeout is two milliseconds. | L3 selected keyboard-controller behavior. |
| KBC-F3 | AT pp. 1-36--1-37, 4-5--4-15 | Transmit path adds parity and serializes host data; keyboard acknowledgement/response is ordered after each prior byte. Keyboard reset, resend, defaults, scanning, typematic and scan-code commands are keyboard protocol at the controller's serial edge. | Start-clock timeout (15 ms), transmission-complete timeout (2 ms) and response timeout (25 ms) produce FEh and documented status errors; no automatic retries for transmission errors. | Keyboard clock supplies serial synchronization; protocol response timing is selected keyboard behavior, not an arbitrary host callback cadence. | L3 controller-visible serial behavior; exact keyboard device command coverage is L2. |
| KBC-F4 | AT pp. 1-31--1-32, 1-37--1-40 | Controller makes OBF visible to the system and, when enabled by command byte, asserts keyboard IRQ1. | Host output-buffer read releases OBF; disabling keyboard/interrupt control stops the corresponding publication. | AT assigns keyboard OBF to IRQ1. | L3 logical route; PIC/CPU visibility phase is L2. |
| KBC-F5 | AT pp. 1-42--1-44 | Output-port bit 0 is system reset and bit 1 is Gate A20; D1 writes the output port and F0h--FFh can pulse bits 0--3 low. | Bit 0 should not be written low; pulsing bit 0 resets the processor. | Pulse width is approximately 6 microseconds; A20/reset delivery is a board/CPU relation. | L3 selected logical binding; pulse and CPU visibility phase are L2/L4 as applicable. |
| KBC-F6 | AT pp. 1-28, 1-31--1-44 | NMI masking and parity/I/O-channel-check are board mechanisms distinct from the 8042 command/output port. | At power-on NMI is masked; POST later enables it after board setup. | No AT source makes KBC output port an NMI mask or NMI producer. | L3 negative ownership boundary. |
| KBC-F7 | UPI pp. 14--19, AT pp. 1-31--1-44 | UPI oscillator, reset, host bus and serial electrical limits define chip operation. | UPI RESET minimum is eight instruction cycles for an external reset; power-on hold recommendation is 10 ms. | Oscillator range, bus/serial waveforms and voltage/AC tables do not by themselves define Core ticks. | L4 electrical limits excluded; selected timing inputs remain L2. |

## Selected AT Binding And Ownership Universe

| ID | Source | Rule | Reset/cancellation and signal relation | Disposition |
| --- | --- | --- | --- | --- |
| KBC-T1 | AT pp. 1-28, 1-31--1-44 | AT assigns 0060h--006Fh to the 8042 keyboard controller, specifically 0060h data and 0064h status/command; the controller owns keyboard serial buffering/translation. | Host status/data access uses the single controller buffer pair. | L3 topology. |
| KBC-T2 | AT pp. 1-10, 1-31--1-40 | Keyboard Output Buffer Full is IRQ1; the secondary controller's IRQ12/AUX route is not an IBM 5170 keyboard-controller fact and requires its own selected source. | PIC acknowledgement stays PIC-owned. | L3 IRQ1 binding; AUX is L2/unselected. |
| KBC-T3 | AT pp. 1-42--1-44 | 8042 output port routes system reset and Gate A20; memory mapping and CPU reset consumption stay their Core owners. | D1/pulse commands produce the board signals; one reset path must cancel pending controller work deterministically. | L3 board binding; L2 CPU/memory phase. |
| KBC-T4 | AT pp. 1-28--1-29, 1-31--1-44 | NMI mask belongs to the RTC/board I/O path, while parity/I/O-channel-check is board logic; it must not be attached to KBC output-port state. | Power-on NMI mask and POST enable are board/firmware policy. | L3 separation; firmware policy L2. |
| KBC-T5 | T433 S6/S7; T449 | The sole Core timeline, reset, arbitration and observation owners supply selected controller/serial cadence; KBC publishes only through its existing port/IRQ/A20/reset endpoints. | No selected 8042 firmware instruction schedule, keyboard clock rate or output-port-to-CPU phase is yet admitted. | L2 receiver: queued Core KBC 8042/NMI phase contract. |

## Completeness And S12 Transfer

The finite universe is `KBC-R1`--`KBC-R4`, `KBC-F1`--`KBC-F7` and
`KBC-T1`--`KBC-T5`. It covers UPI host buffering and internal timing, selected
AT status/commands/ports, keyboard serial error and command boundary, IRQ1,
A20/reset, NMI separation, electrical exclusions and Core ownership. S12 must
retain every identifier and add only current-code/test disposition; each
nonconforming or unallocated row transfers once to the queued Core KBC
8042/NMI phase contract.
