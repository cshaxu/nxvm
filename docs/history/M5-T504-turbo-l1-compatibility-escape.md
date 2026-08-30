# M5 T504 Turbo L1 Compatibility Escape

T504 is the bounded compatibility receiver for the remaining Core-owned L1
waits without a source-qualified deadline. It preserves one guest-time owner:
Core selects and orders known events, while VM selects Standard or Turbo and
observes copied status only.

Its first subtask inventories the entire reachable blocked/no-deadline
universe before any implementation. Later subtasks may add one shared,
bounded Turbo-only progression only when an owner's normal lifecycle remains
preserved. Standard does not use that progression and neither mode receives
guest time from the host.

No task result may promote causal-only timing to L2/L3, invent a controller
deadline, add a parallel scheduler, or retain a device-specific workaround.
