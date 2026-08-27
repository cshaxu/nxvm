# T491 S1 8255A And IBM 5160 Original Source Ledger

`M5:T491:S1:8255A-SOURCE-LEDGER:ACCEPTED-PENDING-GOVERNANCE`

| Source | Provenance and verification | Scope admitted |
| --- | --- | --- |
| External owner-managed archive: `controllers/intel/8255A-5_Programmable_Peripheral_Interface.pdf` | Public scan of Intel document 231308-004, September 1993; 24 pages; SHA-256 `1F2DF669139B8F51B7B6CD7EB6B854A0F5F37A8A59943A02F2141B32E3ECC5C4`. `pdftotext` yields no usable body text, so all claims require visual-page citation. Cover/page 1 visually confirms 8255A-5, 24 I/O pins, Modes 0/1/2, BSR, reset/control inputs and block diagram. | Chip registers, control-word/BSR semantics, port direction/mode behavior, handshake and documented electrical timing. |
| External owner-managed archive: `controllers/ibm/IBM_5160_Technical_Reference_APR83.pdf` | IBM 5160 Technical Reference, SHA-256 `18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8`. Board-specific pages will be visually cited in List 1. | XT port decoding and the PPI-to-keyboard/parity/I/O-check/NMI board wiring only. |

The chip source is a 1993 Intel 8255A-5 revision rather than an 8042 manual;
it is suitable for NMOS-compatible PPI register semantics, but not evidence of
IBM-specific board wiring. The IBM reference is the sole board authority.
S2 must separately mark any unavailable board electrical/edge timing as L1/L2,
not infer it from the chip manual or an AT 8042.
