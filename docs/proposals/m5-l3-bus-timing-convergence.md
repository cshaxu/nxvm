# M5 L3 Bus-Timing Convergence

After the selected controllers own explicit service states, converge remaining
PC/AT bus visibility: CPU memory/I/O wait boundaries, DMA bus ownership, and
device-specific service competition.  Build on T346's deterministic due-event
timeline and the adopted controller state machines.  This task does not claim
cycle-exact prefetch, pin, analog, or universal cycle behavior; every timing
cost requires a primary contract or a reproducible hardware/corpus need.

