# T585 S12 — PC110 PC Card I/O ownership audit

Current source has one board-owned ATA PIO controller directly registered on
1F0h–1F7h and 3F6h in `mypc110_machine_hdc`. It has booted the selected DOS
and Personaware media. `x86_rb5c396_decode_io()` independently decodes two
programmable windows per socket, but no production caller uses that decoder:
the App registers only the Ricoh index/data and memory routes. Consequently
the guest can change or disable a PC Card I/O window without changing ATA
port dispatch. Boot success does not establish PC Card I/O mapping.

Open-Source-PC110 `Discovery/Storage/readme.md` describes internal CF/PC Card
storage through the Ricoh controller, and its LBA-BIOS-Patch README identifies
16-bit window setup at F000:D752. This is useful L2 reference evidence, not
an imported implementation or a physical measurement performed here.
PC110-EMU uses a 3E0h/3E1h placeholder; PC110-QEMU has indexed PCIC register
storage. Neither source establishes a reusable routed I/O implementation.

Core's existing exact-port registration rejects duplicate owners and is
frozen before execution. It cannot safely follow guest-programmed ranges by
removing and reinstalling live routes. The bounded receiver should retain
one ATA controller and one bus: an optional dynamic port-window receiver
must resolve mapping and conflicts within the existing Core dispatch, and
the PC110 adapter must translate enabled Ricoh windows to the existing ATA
provider. Static-device precedence, open-bus behavior, access width, reset,
IRQ ownership and change visibility need explicit tests. No all-ports table,
second ATA device, host-side BIOS interception or direct-path fallback is
justified. Receiver design and implementation are the next numeric S tasks.

This audit is static; no new firmware trace or physical-card result is
claimed. S11's final dual-width 712/712 unit and 48/48 integration suites,
profile rows and product qualification remain the code baseline because this
S changes documentation only.
