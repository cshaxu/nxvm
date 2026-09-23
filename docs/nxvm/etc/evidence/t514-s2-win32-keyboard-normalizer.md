# T514 S2: Win32 Keyboard Normalizer

The Core-platform Win32 capability is the single physical-key normalization
boundary for native, character-only and virtual-key-only host packets.  It
converts host-layout Unicode characters to a complete physical make/break
sequence, resolves a missing scan from a virtual key, and validates UTF-16
pairs without writing guest text, BIOS state, or KBC state directly.

Console uses the capability for zero-scan `KEY_EVENT_RECORD` characters;
Window uses it for zero-scan `WM_CHAR` and `WM_UNICHAR`.  A Window virtual-key
recovery suppresses only its subsequent matching character, so it cannot
duplicate the same guest key while an unrelated character remains deliverable.
Existing physical-key handling and VM F9 command policy remain unchanged.

Focused owner-local coverage verifies layout recovery, complete release,
surrogate rejection, virtual-key matching, and the existing VM request route
for both recovered virtual keys and recovered characters.  A rebuilt stripped
Release `nxvm_0_5_0514.exe` and complete repository-only unit suite pass:
317/317, 2026-09-01.  Real RDP soft-keyboard observation remains the S4
frontend proof.
