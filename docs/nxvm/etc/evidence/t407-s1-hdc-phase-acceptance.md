# T407 S1 HDC Generic-AT Phase Acceptance

`M5:T407:S1:HDC-PHASE:OK`

Audit finds the requested generic-AT phase mechanism already implemented at the sole Core HDC owner. Command capture sets `PENDING_COMMAND` and `BSY`; the existing readiness tick invokes `core_machine_hdc_advance`, which alone publishes data DRQ and IRQ14. Read/write sector continuation similarly uses pending sector phases. Reset cancels the pending state and IRQ. Existing `core_machine_hdc_smoke` and Compaq HDC smoke cover these transitions.

No runtime source is changed. This accepts only the generic-AT logical virtual-time phase contract. No original Compaq timing scalar, physical controller latency, media behavior beyond the frozen protocol, or L3 claim is accepted.