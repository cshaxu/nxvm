# T394 S6 Descriptor-Selection Non-Admission

## Decision

The complete T390 C0 81-key set is not installed as a Model-40 physical
retirement descriptor. This is a whole-batch non-admission, not a partial
selection or a key/capture defect.

## Complete Route Sweep

Both `vm_session_create_model40_byob` and `vm_session_create_model40_private`
construct the same selected 80386DX-16 Core configuration with deterministic
retirement and a neutral 1:1 plan for every registered clock domain. Their
existing private/public composition smokes assert deterministic retirement.
No Model-40 route currently supplies a distinct physical clock plan or a
configuration surface that could enable physical retirement.

T387 S1 records why that is required: the 1:1 Model-40 plan is neutral and
cannot support a physical board-clock or L3 claim. CPU-to-PIT calibration,
DCLK conversion, bus availability and controller service timing remain
unselected pending a source-backed conversion contract. Installing the S5
keys in physical mode before that contract would publish successful retirement
into an invented board-time domain.

## Preserved Core Safety

S4's copied descriptor remains available and unchanged: an absent or
unallocated successful retirement faults before elapsed/timeline/provider
publication in physical mode. S5's 81-key C0 mapping remains complete (80
one-to-one keys plus one recorded prefix-normalized equivalence). Neither fact
establishes a board clock conversion; therefore neither authorizes a physical
Model-40 route.

## Transfer

The full 81-key batch transfers intact to the first source-backed DeskPro
Model-40 board clock-plan/consumer contract. That receiver must define CPU
source-to-project-tick conversion, reset phase and the first consumer boundary,
then may separately admit a Model-40 descriptor-selection proof. It must not
invent a ratio from the neutral plan or select only a subset of C0 keys.

## Verification

Static route sweep of both Model-40 constructors and their deterministic
composition smokes confirms the non-admission. Documentation governance and
the current gate suite remain required for closure. No external asset was
imported or recorded.