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

## M4: Optional Microsoft NTVDM Guest Research

**Goal:** investigate validated, user-supplied NTVDM guest components.

**Scope:** component discovery, hash/profile validation, binary loading, BOP
trap/dispatch, startup traces, and an XP SP3 x86 research profile.

**Non-goals:** redistribution, default-backend dependency, or a blanket
compatibility promise.

**Demo and exit:** staged M4A first BOP, M4B `COMMAND.COM` startup, M4C prompt
with `DIR`, `TYPE`, and `EXIT`. Tests use metadata and permitted local inputs.
Risk: undocumented protocol and licensing. Dependencies: M1-M3 and BYOB policy.

## M5: Invasive Windows Integration Research

**Goal:** evaluate optional system integration outside the CLI path.

**Scope:** documents, risk analysis, reversible prototypes, installer and
uninstaller design, and Windows-version compatibility evaluation.

**Non-goals:** default build inclusion, automatic system changes, release
dependency, or security-policy changes.

**Demo and exit:** reviewed design and rollback prototype only. Tests validate
reversibility where a prototype is approved. Risk: security and distribution.
Dependency: explicit owner decision.

## M6: Win16 Research

**Goal:** select a viable Win16 route without blocking DOS delivery.

**Scope:** WineVDM external backend, Microsoft WOW research, NE loading,
unified launcher options, API thunking, process/filesystem model, license,
distribution, ARM64 feasibility, and relation to Microsoft guest mode.

**Non-goals:** a promise of complete Win16 support.

**Demo and exit:** architecture options, dependency map, license analysis,
proof of concept, and recommended path. Risk: large API and licensing surface.
Dependency: M3 and an owner-approved direction.
