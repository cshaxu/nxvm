# MyPC110: startup performance and guest clocks

Transfer from T580 by owner direction on 2026-10-29. Diagnose the long startup and slow World Clock using actual product, guest clock, RTC and timer observations. Determine the correct owner and implement a measured, minimal repair without replacing guest time with an unexamined host-time shortcut. Preserve the owner's exclusion of elapsed host-off compensation. Distinguish L1/L2 models from manual-defined facts.

Acceptance requires a before/after representative startup measurement, guest clock and RTC evidence, both host architectures, appropriate regression and product artifacts. Do not claim hardware accuracy or root cause from a single observation. See [S89](../etc/evidence/t580-s89-clock-rate.md), [S88](../etc/evidence/t580-s88-clock-persistence.md) and [S90](../etc/evidence/t580-s90-acceptance-failure.md).
