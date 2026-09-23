# T522 S4 UX Cutover Map

## Retained Product Flow

```text
Core copied display frame
  -> VM session's sole display conversion
  -> UX copied mailbox
  -> native UX Console or Window loop

native UX event/action
  -> VM product binding
  -> existing VM request ingress
  -> Core keyboard/mouse owner
```

The VM product binding owns conversion and action policy. `lib/ux` owns neither
Core's display snapshot nor the VM request transport; its public ABI continues
to expose only copied frames/events and an opaque callback context.

## Source Disposition

| Existing source family | S4 disposition | Replacement owner |
| --- | --- | --- |
| `vm/platform/win32/{w32cdisp,w32adisp,win32con,win32app}` | Delete | `lib/ux/win32` Console/Window loops. |
| `vm/platform/linux/{linux,linuxcon}` display/input loop | Delete | `lib/ux/linux` Console loop; Linux Window remains unsupported. |
| Win32 host-key normalization and shortcut classification in `vm/platform/win32/win32.c` | Delete after policy transfer | `lib/ux/win32/input` normalizes host records; VM binding owns only action outcome and guest ingress. |
| `vm_platform_run_handle` presenter state, host-surface lease and display-router state | Delete or reduce to VM guest-runner ownership only | VM product run context owns guest thread lifecycle; `lib/ux` owns native presentation state and Console lease. |
| `vm/platform/{request_bridge,vm_request_transport,virtual_time}` | Retain | Existing VM request/timing owner; not presentation code. |
| `vm/composition/session/{control,runner,lifecycle,display}` | Retain with one binding call | Existing VM lifecycle and Core display owner. |

## Required Product Binding Operations

1. Capture the existing Core frame at the session's current display-publish
   point and copy it once into the UX mailbox.
2. Translate a copied `ux_event` into the existing copied VM request ingress.
3. Translate a product action into pause, mouse-release, Ctrl+Alt+Del, or
   Alt+Enter request/guest-event semantics without a native type in `lib/ux`.
4. Report VM lifecycle state and title to the UX binding; native close asks the
   existing VM owner to pause rather than destroying guest state.
5. Run guest execution on the VM-owned runner and a native UX loop on its
   product-owned presenter thread. The library owns the latter loop, but not
   thread, session, profile or machine construction.

## Implemented Cutover

The VM session display publisher first publishes one Core copied frame to the
retained Core mailbox. The VM product binding captures that same mailbox entry,
performs the sole Core-frame-to-`ux_frame` copy, and publishes it to the UX
mailbox. One private VM binding converts UX events to the existing request
ingress, reports the VM lifecycle/title, and maps product actions without
exporting a VM/Core type through the public UX ABI. The retained
`vm_platform_run_handle` is only VM execution control and thread ownership; it
no longer stores a renderer, host-surface lease, native input classifier or
display transition state.

The following obsolete source families and their renderer-private tests are
removed in the same change: Win32 `w32cdisp`, `w32adisp`, `win32con`,
`win32app`; Linux `linuxcon`; and VM `host_surface`.  The replacement native
loops are `lib/ux/win32` and `lib/ux/linux`; `vm/platform/{request_bridge,
vm_request_transport,virtual_time}` remain their distinct VM owners.

The focused binding proof covers a normal F-key event, pause callback,
Ctrl+Alt+Del and Alt+Enter injection, the Unicode-text fallback and lifecycle
title. A consumed Win32 Ctrl+Alt product chord first releases those host
modifiers through the shared UX keyboard helper, so neither pause nor injected
guest chords can leave a modifier pressed. The frame proof covers copied text,
palette and cursor values. Full repository unit verification after the cutover
is `304/304` passing. Documentation governance and `git diff --check` pass.
The old presenter/input/display API sweep is empty, and root `lib/ux` headers
contain neither Core/VM identifiers nor native platform types.
The Linux Console retains its former F1--F12, cursor/navigation, Enter,
Backspace and DOS-attribute-color behavior in the shared native loop. Its
platform-neutral key mapping has a repository unit proof, and its native
Console/VM adapter sources pass strict C11 static analysis. This host has no
installed Linux target; the owner accepts these source and unit gates in place
of a native Linux run for S4.
