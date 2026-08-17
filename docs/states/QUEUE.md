# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline.

Each candidate links to its proposal. The shared admission and program context
is retained separately so this queue remains a short ordering surface.

## M5 Candidates

The [four-profile CPU-completeness program](../proposals/m5-four-profile-cpu-completeness-program.md)
is the shared admission context for this dependency-ordered sequence.

The [80286 closure context](../proposals/m5-80286-protected-mode-closure.md)
and [80386DX closure context](../proposals/m5-80386dx-extended-state-closure.md),
together with the [80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md),
are shared planning references for the corresponding candidates below; they do
not allocate a numeric task.

The [DeskPro 386 Model 40 closure context](../proposals/m5-80386-deskpro-386-l3-baseline.md)
binds its five bounded candidates below without becoming a candidate itself.
Detailed scope, acceptance conditions, and common constraints remain in these
shared proposals and in each candidate's linked proposal.

1. [DeskPro 386 board bus and device timing closure](../proposals/m5-deskpro-386-bus-device-closure.md).
1. [Four-profile CPU physical-timebase closure](../proposals/m5-four-profile-cpu-physical-timebase-closure.md).
1. [DeskPro 386 physical-device and firmware-timing closure](../proposals/m5-deskpro-386-physical-device-firmware-timing-closure.md).
1. [DeskPro 386 model-L3 closure audit](../proposals/m5-deskpro-386-l3-audit.md).
1. [Four-profile CPU instruction-correctness audit and repair program](../proposals/m5-four-profile-cpu-instruction-correctness-audit.md).
1. [Current-product device profile and functional-capability closure](../proposals/m5-current-product-device-profile-capability-closure.md).
1. [32-bit GCC host-build compatibility verification](../proposals/m5-32bit-gcc-host-build-compatibility.md).
1. [8088 CPU-profile admission and timing closure](../proposals/m5-8088-cpu-profile.md).
1. [IBM PC/XT 5160-268 profile and capability-gap audit](../proposals/m5-xt-5160-268-profile-capability-audit.md).
1. [IBM PC/XT 5160-268 selected-device functional closure](../proposals/m5-xt-5160-268-device-functional-closure.md).
1. [IBM PC/XT 5160-268 board and device phase-timing closure](../proposals/m5-8088-5150-xt-l3-baseline.md).
1. [IBM PC/XT 5160-268 final model-L3 audit](../proposals/m5-xt-5160-268-l3-audit.md).
1. [Current-product device-capability L3 closure](../proposals/m5-current-product-device-l3-closure.md).
1. [M5 pre-Windows L3 admission audit](../proposals/m5-pre-windows-l3-admission-audit.md).
1. [Windows 3.1 BYOB lifecycle harness](../proposals/m5-windows31-byob-lifecycle-harness.md).
1. [Windows 3.1 Standard Mode lifecycle](../proposals/m5-windows31-standard-mode-lifecycle.md).
1. [Windows 3.1 386 Enhanced Mode lifecycle](../proposals/m5-windows31-enhanced-mode-lifecycle.md).
1. [M5 final Windows closure audit](../proposals/m5-final-l3-windows-closure-audit.md).

Retained non-candidate proposal index: [VM profile-contract ownership migration](../proposals/m5-vm-profile-contract-ownership.md), [80286 successful-retirement timing closure](../proposals/m5-80286-retirement-timing-closure.md), [cross-mode mechanism coherence](../proposals/m5-cross-mode-mechanism-coherence.md), [instruction-timed execution](../proposals/m5-instruction-timed-execution.md), and [L3 bus-timing convergence](../proposals/m5-l3-bus-timing-convergence.md). Their completed task records are retained in history.

## M6 Candidates

The [M6 mantle experiment program](../proposals/m6-mantle-program.md) applies
to every candidate in this section. These candidates are not eligible until
the M5 final Windows closure audit accepts the selected profile.

1. [Pre-decode transition gateway and mantle probe](../proposals/m6-predecode-transition-gateway.md).
1. [Ordinary-RAM transaction for staged runtime input](../proposals/m6-ordinary-ram-transaction.md).
1. [Transition-local combined commit](../proposals/m6-transition-local-combined-commit.md).
1. [Minimal mantle single-session closure](../proposals/m6-minimal-mantle-single-session.md).
