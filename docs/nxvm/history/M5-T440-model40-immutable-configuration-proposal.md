# M5 Model-40 Immutable Configuration Enforcement

## Purpose

Enforce the Model-40's selected fixed configuration at the VM profile boundary,
including its fixed 1 MiB memory policy, so generic session controls cannot
silently reconfigure the machine after materialization.

## Required Scope

Inventory all Model-40 memory/configuration mutation routes. Reject or make
inapplicable every incompatible runtime reconfiguration before it reaches Core,
while retaining legal generic session behavior for profiles that declare the
choice. Prove that failed requests leave Model-40 state unchanged.

## Dependencies And Completion

Independent of the preceding reset repair but ordered after it for one-active-S
execution. Completion requires Model-40 and non-Model-40 focused regressions,
the full configuration-route sweep, and current gates.

## Non-goals And Stop Conditions

Do not change Model-40 hardware composition, expose mutable profile state,
encode a Core machine-name branch, or create a new configuration framework.
Stop if the policy cannot be expressed by the existing profile/session seam.
