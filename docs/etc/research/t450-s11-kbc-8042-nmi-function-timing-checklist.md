# T450 S11 KBC 8042/NMI Original Function And Timing Checklist

## Scope And Source Key

This complete KBC source universe transfers unchanged to T450 S12. `UPI`
means Intel *UPI-41A/41AH/42/42AH User's Manual*, order 231318-006 (Oct.
1993); page references are printed manual pages. `AT` means IBM *Personal
Computer AT Technical Reference*, 1502243 (Mar. 1984), system-board pages.
The UPI manual specifies a programmable controller substrate, while AT
specifies the selected 8042 firmware-visible host interface, keyboard serial
handling and board wiring. Keyboard command protocol is included only where AT
assigns it to this selected controller boundary.

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
