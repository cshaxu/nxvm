# M5 T13 S5 Core Ownership Design

The T13 S1 dependency audit is now a five-slice execution plan in
`docs/planning/m5-t13-core-ownership-cutover.md`. It orders the low-risk type
dependency first, then product composition, host sleep, execution stop, and
firmware-service callback work. Each substantial callback is isolated behind
the existing whole-PC regression matrix; no runtime source changed in S5.
