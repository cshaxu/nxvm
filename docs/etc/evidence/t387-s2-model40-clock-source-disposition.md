# T387 S2: Model-40 Clock Source Disposition

`M5:T387:S2:MODEL40-CLOCK-SOURCE-DISPOSITION:OK`

## Result

The primary corpus establishes independent facts, not one common conversion:
D3PE selects a 16 MHz 80386 phase-related to a 32 MHz oscillator, 4 MHz DCLK
with one wait per DMA cycle, and approximately 1.19318 MHz PIT input. It also
only bounds the resynchronized expansion bus as averaging below 8 MHz. Neither
Compaq primary material retained by T384 nor Intel CPU timing material supplies
a mapping from successful-retirement ticks to PIT or DCLK ticks, nor an
observable board phase from which one may be inferred.

The only admissible equation currently available is PIT frequency divided by a
*chosen* CPU clock. It cannot be applied to NXVM because `elapsed_ticks` are
successful-retirement timing units, not documented 16 MHz oscillator cycles.
Likewise, `4 MHz / 16 MHz = 1/4` describes oscillator domains but cannot make
one Core DMA arbitration callback into one DCLK cycle or place its required
wait state. The bus statement has no exact scalar at all.

## Current Owner Sweep

Core's sole publisher advances its deterministic three-phase timeline from
successful retirement. `core_machine_clock_domain` can represent a rational
ratio, but Model-40's private construction still declares neutral domains.
The existing Model-339 PIT ratio is a different IBM source-backed contract and
cannot be borrowed. No Core/VM route currently observes a real Model-40 PIT
counter-1 pulse, DMA DACK, READY, HRQ/HLDA or bus phase.

PCjs, 86Box and MAME can locate a configuration or cross-check a separately
qualified capture; they provide no primary-constrained CPU-to-PIT or DCLK
measurement contract here. S2 therefore admits no timing scalar and no code.

## Next Receiver

A later T387 S must define an owner-approved bridge observation: a real
Model-40/D4 measurement or a primary-constrained reference capture with clock
source, trigger, checkpoints, instruction/time/no-progress budget, masks and
cleanup. It must measure a named observable such as PIT counter-1 through its
board consumer or a DMA request-to-DACK lifecycle, then prove conversion and
reset semantics before any Core clock-plan publication. Without that, the
proper outcome is retained non-admission, not a fabricated L3 value.