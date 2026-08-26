# M5 T480 VADP IBM VGA L3 Capability

T480 admits the queued IBM VGA controller capability without claiming that an
existing IBM 5170 or DeskPro profile shipped with VGA. It retains VADP as the
only owner of guest video state, ports, mapped memory and copied snapshots.

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: the finite IBM primary/cross-model ledger classifies controller state, attribute access, chain-4, aperture, latches, DAC, Mode 13h, CRTC mapping, timing and profile binding. It records PCjs/QEMU chain-4 limits and leaves every current profile unselected. |
| S2 | Accepted: one VADP owns all current video state and one copied snapshot reaches VM presentation. The row-complete code-gap ledger assigns EGA-common work to S3, VGA-only DAC/aperture/chain-4/256-colour work to S4, and preserves an unselected profile boundary for S5. |
| S3 | In progress: verify and consolidate the source-qualified EGA/VGA-common controller mechanism inside the sole VADP state before VGA-only additions. |
