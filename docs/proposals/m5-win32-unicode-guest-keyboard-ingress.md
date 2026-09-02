# M5 Win32 Unicode Guest-Keyboard Ingress

## Purpose

Make NXVM accept the key events delivered by a local Windows console/window
and by Remote Desktop touch keyboards through one normalized physical-key
ingress.  In particular, a Win32 Unicode character packet with no usable PC
scan code must no longer disappear before it reaches the existing guest
keyboard path.

## Evidence And Scope

The initial ledger covers `KEY_EVENT_RECORD` from the Console and `WM_KEY*`,
`WM_CHAR`, `WM_UNICHAR`, and UTF-16 surrogate input from the Window path.  It
records each event's physical-key information, Unicode payload, duplicate
relationship, and disposition.  SoftPC's Win32 console/window handling is a
read-only design reference: its useful observation is that RDP may supply a
character without a physical scan code and that a recovered character must
emit a complete make/break sequence.  Its source and tables are not imported.

## Design Boundary

Core `platform` owns the reusable Win32 host-input capability: it recognizes
Console/window/RDP records, validates UTF-16, resolves host-layout characters
through `VkKeyScanExW` and `MapVirtualKeyExW`, suppresses duplicate text after
a physical key, and publishes copied normalized physical transitions through a
caller-provided sink.  This is a host capability, not a VM-only feature, so it
belongs below product composition and is reusable by VM, Mantle, and later
products.

VM owns only its product binding: it supplies the session input sink and the
selected profile maps each normalized transition to the keyboard's native scan
set.  Core machine remains the sole owner of the keyboard device, scan-set
serial stream, 8042 translation, FIFO, IRQ1, and guest state.  Neither the
Win32 platform capability nor VM writes a KBC byte, guest memory, BIOS buffer,
or session state directly.

One shared Core-platform Win32 helper emits either a native physical key or a
layout-recovered Unicode character as modifiers, the key, and their releases
in one ordered sequence, so RDP packets without a matching key-up cannot leave
modifiers stuck.  Console and Window frontends both consume that helper.  A
normal physical key is delivered once; its later text message is recognized as
its character result and is not delivered again.

Unicode is accepted as UTF-16 at the host boundary, including surrogate-pair
validation.  A PC keyboard cannot physically express every Unicode scalar, and
DOS/Windows 3.x cannot consume arbitrary Unicode by a scan-code alone.  Thus
the implementation must distinguish: (1) a scalar that the active host layout
can translate into a physical guest sequence, which is delivered; (2) a valid
scalar that has no such sequence, which receives an explicit observable
unsupported disposition and is never silently truncated or converted; and
(3) malformed UTF-16, which is rejected without guest mutation.  Supporting
arbitrary semantic Unicode text beyond the guest keyboard/layout is a separate
guest IME or text-service capability, not a firmware/BIOS shortcut in this
task.

## Subtask Plan

1. **S1 - ingress ledger and reference reconciliation.** Audit both current
   Win32 paths and their tests; freeze the finite event matrix and confirm the
   SoftPC observation against Win32 documentation and a local RDP-capable
   probe.  Record exact duplicate, fallback, and unsupported cases.
2. **S2 - one Core-platform Win32 normalizer.** Replace Console- and
   Window-specific scan-code fallbacks with one reusable Core-platform host
   capability.  It handles enhanced keys, layout recovery, modifier ordering,
   duplicate suppression, UTF-16 pairing, and explicit failure without a
   parallel queue or guest-mutation route.
3. **S3 - owner-local regressions.** Add table-driven repository-only tests
   for physical input, missing-scan Unicode input, shifted/layout-derived
   characters, enhanced keys, key-up behavior, duplicate character messages,
   BMP and surrogate-pair dispositions, and no-stuck-modifier failure paths.
4. **S4 - real frontend proof and closure.** Exercise Console and Window
   paths with local and RDP-originated soft-keyboard input where available;
   prove the unchanged VM-to-KBC route, run the complete unit suite and the
   external integration suite, and record every layout-dependent unsupported
   scalar without claiming it was injected.

## Exit Criteria

- Both Win32 frontends consume one Core-platform normalized production path
  into the existing input sink and VM/profile keyboard mapper.
- RDP/soft-keyboard character packets lacking physical scans reach an
  equivalent physical guest sequence whenever the active host layout provides
  one; no duplicate key or stuck modifier is introduced.
- UTF-16 is handled without truncation; unsupported or malformed text has a
  distinct, tested disposition rather than silent loss or BIOS injection.
- No second keyboard state, profile-side KBC model, guest text backdoor, or
  imported SoftPC source/table remains.
