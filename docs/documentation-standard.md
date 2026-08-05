# Documentation Standard

Documentation exists to make the next correct decision easy, not to preserve a
separate file for every completed action.

## Principal Documents

The daily governance surface has exactly seven subjects:

1. [Design Principles](design-principles.md)
2. [Coding Standard](coding-standard.md)
3. [Documentation Standard](documentation-standard.md)
4. [System Architecture](architecture/overview.md)
5. [Roadmap And Milestones](planning/roadmap.md)
6. [Debt Tracking](debt-tracking.md)
7. [Execution Workflow](execution-workflow.md)

`planning/status.md` and `planning/m5-closure-checklist.md` are operational
companions, not additional design authorities.

## Rules

- One topic has one current authority. Link to it; do not copy it into another
  plan, task note, or history document.
- Current documents state present commitments, boundaries, and acceptance
  conditions. Completed task detail belongs in a compact milestone history or
  Git, never beside current planning.
- A task record is temporary operational scaffolding. On governed completion it
  is summarized or removed; its commit remains the detailed evidence.
- A completed task cannot be summarized or removed until the execution
  workflow's requirement-by-requirement closure audit passes.
- Keep documents short enough to review. Split only when authority or audience
  differs, not to create a record per subtask.
- Update direct links whenever a current authority moves or is consolidated.
- History explains why a decision occurred but cannot override architecture,
  requirements, roadmap, policy, or active status.

See [Historical Summaries](history/README.md) for retrieval of retired detail.
