# Roadmap

Each milestone requires a runnable demonstration, focused automated tests,
recorded evidence, and preservation of established baselines.

## M1: Validate The NXVM Machine Foundation

**Goal:** prove that an NXVM-derived 80386 machine can reliably execute the
real-mode subset required by the first DOS path.

**Scope:** CPU/memory, IVT, minimal BIOS service or startup path, PIC/PIT,
basic keyboard and text-display paths, debugger, assembler/disassembler,
single stepping, register and memory inspection.

**Non-goals:** complete DOS, VGA, sound, networking, full disk controller, or
full PC boot chain. The existing 80386 CPU is retained; M1 does not replace it
with a new 8086 implementation.

**Demo and exit:** load a small real-mode binary, execute instructions and an
interrupt, and verify expected registers and memory. Tests cover the selected
instruction and interrupt boundaries. Risk: inherited global state and device
coupling. Dependency: a provenance-reviewed NXVM import.

## M2: Add The Owned DOS Backend

**Goal:** run a simple COM program using DOS interrupts.

**Scope:** COM loader, PSP, environment, INT 20h, high-ROI INT 21h services,
text and keyboard I/O, exit, basic files, memory services, and DOS errors.

**Non-goals:** complete DOS API, XMS/EMS/DPMI, Microsoft guest components, and
shell integration.

**Demo and exit:** `hello.com` prints and exits; a file probe opens, reads,
prints, closes, and exits. Generated probes cover each new service. Risk:
machine/DOS ABI leakage. Dependency: M1 machine contract.

## M3: Non-Invasive Windows Platform Integration

**Goal:** make the default backend usable through `ntvdm run` on a clean
64-bit Windows system.

**Scope:** CLI, arguments, current directory, environment, host filesystem,
keyboard/mouse, text and basic graphics paths, exit code, logging, debug mode,
configuration, Ctrl+C, and adapted NXVM input/display components.

**Non-goals:** global file association, loader replacement, injection, drivers,
or registry-dependent operation.

**Demo and exit:** `ntvdm run hello.com` and an argument-bearing DOS program
work without system changes. Tests cover paths, arguments, host I/O, and exit
status. Risk: platform adapters becoming DOS internals. Dependency: M2.

## M4: Real DOS Program Compatibility

**Goal:** grow from probes to representative DOS utilities and real programs.

**Scope:** add MZ loading, EXEC, MCB, FCB, wildcard and 8.3 semantics,
attributes, date/time, device names, selected XMS/EMS, graphics, and mouse only
when a corpus requirement justifies it.

**Non-goals:** theoretical API completeness or speculative Microsoft ABI.

**Demo and exit:** each corpus program has a reproducible result. Risk:
unbounded compatibility scope. Dependency: M3.

## M5: Invasive Windows Integration Research

**Goal:** evaluate optional system integration outside the CLI path.

**Scope:** documents, risk analysis, reversible prototypes, installer and
uninstaller design, and Windows-version compatibility evaluation.

**Non-goals:** default build inclusion, automatic system changes, release
dependency, or security-policy changes.

**Demo and exit:** reviewed design and rollback prototype only. Tests validate
reversibility where a prototype is approved. Risk: security and distribution.
Dependency: explicit owner decision. Its host-ABI findings are an M6 entry
condition.

## M6: Microsoft NTVDM Component Research

**Goal:** after M5, evaluate the complete Microsoft component system and decide
whether any bounded, non-invasive, high-value integration is feasible.

**Scope:** inventory NTVDM host files, guest DOS files, ROMs, redirectors,
debuggers, configuration, WOW, and host facilities; map startup dependencies,
private ABI, BYOB usability, and interface feasibility with NXVM and Platform.

**Non-goals:** a component loader, BOP dispatcher, or implementation promise.

**Demo and exit:** component matrix, dependency graph, ABI findings, feasibility
report, and a formal Go/No-go decision. Risk: coupled private host architecture.
Dependency: M5.

## M7: Win16 Research

**Goal:** select a viable Win16 route without blocking DOS delivery.

**Scope:** WineVDM, Microsoft WOW, NE loading, KERNEL/USER/GDI, thunking,
unified launcher options, ARM64, licensing, distribution, and relation to M6.

**Non-goals:** a promise of complete Win16 support.

**Demo and exit:** architecture options, dependency map, proof of concept where
approved, and recommended path. Risk: large API surface. Dependency: M3 and
owner approval.
