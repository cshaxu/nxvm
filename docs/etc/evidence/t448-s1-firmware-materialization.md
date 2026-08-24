# T448 S1 Firmware Materialization

The six over-limit generated firmware sources are `VCMOS_INT_SOFT_TIMER_1A`,
`VBIOS_INT_HARD_KEYBOARD_09`, `VBIOS_INT_SOFT_KEYBOARD_16`,
`VHDC_INT_SOFT_HDD_13`, `VFDC_INT_SOFT_FDD_40`, and
`VBIOS_INT_SOFT_VIDEO_10`. Their old and chunk-rejoined source streams have
identical character counts and byte-for-byte content; the chunks are all at
most 3,496 characters.

`vm_session_profile_firmware_materialize` is the only new path: it validates
the aggregate capacity, allocates once, copies each chunk in order, then hands
the unchanged assembled statement to the existing assembler. The FDC segment
substitution retains this same buffer and assembler route. `vm_session_read_u16`
had no caller and was deleted.

UCRT GCC 16.1.0 built `vm-0-5-0448` and the focused ROM, timer and PC/AT
smokes in 114 steps. ROM materialization and PC/AT apply passed. The timer
smoke fails identically in two pre-T448 binaries, so it is not attributed to
this change. The copied developer artifact SHA-256 is
`A89C1901B3D631DFD7482432E74C4CB799BB2A549FA5C6BE93F7205A0BB83677`.
