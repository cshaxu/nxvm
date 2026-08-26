# 8088 Instruction Function And Timing Completeness

## Purpose

Complete the frozen Intel 8088 instruction surface as one CPU T, building on
T485 S1--S20 evidence without treating it as a sequence of isolated opcode
repairs. Core's existing decode and private timing selector remain the only
owners.

## Required sequence

S1 validates the original Intel manual and source provenance. S2 creates List
1 for every Table 2-21 instruction and prefix row, including formula/range and
non-instruction boundaries. S3 maps every row to List 2 current owner/gap/test.
Only then may implementation consume the complete selected List-2 batch,
remove duplicate paths and close with the source-to-test matrix.

## Boundaries

No board cycle, prefetch, physical-time, controller, profile or VM change.
Manual ranges stay explicit unless an approved source-qualified formula or
hardware-observation contract supplies a selection rule.
