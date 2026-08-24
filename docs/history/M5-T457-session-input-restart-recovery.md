# M5 T457: Session Input Restart Recovery

T457 owns the current-product F9-stop then `START` keyboard-input regression.
Its one scope is the shared VM session/platform lifecycle: diagnose the
first/second input epochs, repair their owner if necessary, and retain an
end-to-end regression without changing guest keyboard semantics.
