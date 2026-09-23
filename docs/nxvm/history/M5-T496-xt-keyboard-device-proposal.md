# M5 T496 IBM 5160 XT Keyboard Device

This unit separates the IBM XT keyboard device from the completed 8255/PPI
unit. The device may own only keyboard state actually supported by the source
ledger; it is expected to own scan production, serial transport and the
keyboard-side buffer. The PPI retains its existing sole ownership of ports
`60h`--`63h`, byte latch and IRQ1 publication. The only device-to-PPI
operation is completed-byte delivery.

S1 freezes the visually checked IBM March 1986 keyboard source and bounded
external corroboration. S2 maps the complete keyboard function/timing list to
current owners and records every gap. S3 implements the complete approved
batch in one device owner with no second queue, scheduler, VM state or
PPI-owned keyboard state. S4 validates XT BYOB boot and transfers any next
earliest owner. Manual ranges are retained as source facts; a deterministic
event point inside one is explicitly macro/L2 integration, not a physical-time
claim.
