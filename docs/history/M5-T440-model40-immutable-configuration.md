# M5 T440: Model-40 Immutable Configuration Enforcement

T440 closed at S1. Its approved
[candidate proposal](../proposals/m5-model40-immutable-configuration.md)
enforces the selected Compaq DeskPro 386 Model 40 configuration at the VM
profile/session boundary. The task preserves Core's machine-neutral
configuration mechanism and uses no second configuration framework or mutable
profile state.

P1 `3b21d761` places fixed-memory validation at the session boundary, deletes
the duplicate Model-40 configuration literal, and proves BYOB rejection leaves
state unchanged while generic session reconfiguration remains valid. The full
gate retains the independent T344 71-versus-75 fixture-count baseline.
