# Documentation Rules

Use the public [Documentation Governance skill](https://github.com/cshaxu/skills/blob/main/documentation-governance/SKILL.md)
for reusable method. This file is the ntvdm64-specific authority.

`docs/README.md` is the sole entry point. The direct `docs/` files are exactly
`README.md`, `STATUS.md`, `QUEUE.md`, and `TODO.md`. Its direct directories are
exactly `rules/`, `design/`, `history/`, and `etc/`.

`rules/` contains exactly `DOCUMENT.md`, `EXECUTION.md`, `ARCHITECTURE.md`, and
`CODING.md`. `design/` contains exactly `GOAL.md`, `ARCHITECTURE.md`,
`CODING.md`, `UI.md`, and `ROADMAP.md`. `STATUS.md`, `QUEUE.md`, and `TODO.md`
have distinct operational roles; `history/` contains numbered implementation-
task records; and `etc/` contains only owner-approved supporting material
indexed by [etc/README.md](../etc/README.md).

`rules/ARCHITECTURE.md` and [design/ARCHITECTURE.md](../design/ARCHITECTURE.md)
are linked rule-versus-project-design authorities. `rules/CODING.md` and
[design/CODING.md](../design/CODING.md) have the same distinction.

`rules/ARCHITECTURE.md` owns abstract enforceable invariants; only
`design/ARCHITECTURE.md` maps ntvdm64 components, modules, coupling,
composition, and host boundaries. `rules/CODING.md` owns coding discipline;
only `design/CODING.md` maps source directories, entry points, and naming.

## Authority Boundaries

Keep each principal document within its assigned decision level. A link is the
normal way to cross a boundary; a copied summary is not a second authority.

| Location | Contains | Does not contain |
| --- | --- | --- |
| `docs/README.md` | Task reading set and authority map. | Product design, task state, or policy copies. |
| `STATUS.md` | One active packet, current baseline, compact active-task progress, and capped recent task closures. | Milestone plan, reading guide, completed task packet, or long evidence record. |
| `QUEUE.md` | Ordered, unnumbered candidate work. | Task identifiers, baseline, detailed contract, or active plan. |
| `TODO.md` | Open debt or deferred admission with priority and admission path. | Goals, roadmap, completed work, or active plan. |
| `rules/*.md` | Mandatory project constraints in their named discipline. | Concrete component map, source map, or task history. |
| `design/GOAL.md` | Strategic outcomes. | Milestones, architecture, acceptance detail, or current status. |
| `design/ARCHITECTURE.md` | Concrete component ownership, coupling, composition, and host boundary. | Abstract rule copies, source-file map, or task plan. |
| `design/CODING.md` | Source tree, entry points, file names, and source organization. | Dependency policy, UX, or task process. |
| `design/UI.md` | High-level product interaction. | Full CLI contract, acceptance matrix, or delivery plan. |
| `design/ROADMAP.md` | Milestone outcomes and dependency order. | Task allocation, active work, or implementation contract. |
| `history/` | Closed numeric-task facts and evidence. | Current status or forward authority. |
| `etc/` | Indexed supporting detail. | A competing current authority. |

- Root `README.md` is public orientation only. `AGENTS.md` is an instruction
  pointer for automated contributors, and `CONTRIBUTING.md` is a submission
  guide; neither restates a design or rule authority.
- A local README beneath `tests/` or `tools/` may describe that local area, but
  it does not define global architecture, process, or status.
- One topic has one current authority. Link instead of copying.
- [STATUS.md](../STATUS.md) alone defines active work, technical baseline,
  artifact identity, and recent closure summaries.
- While a numeric task package remains open, `STATUS.md` may retain compact
  summaries of its completed subtasks as task progress, including between
  admitted subtasks. It may retain progress for only that one latest open
  numeric task. When the task closes, replace that progress with one task-level
  closure summary and retain detailed evidence in its history or indexed
  supporting record. The recent task-level closure list is capped at eight
  rows.
- `STATUS.md` records the active contract, current phase, concise progress, and
  links to durable evidence. Long command output, requirement matrices,
  implementation narratives, and detailed review records belong in the
  committed delivery, numbered task history, or indexed `etc/` evidence. A
  link does not move task authority: the active packet remains the contract.
- [QUEUE.md](../QUEUE.md) contains only unnumbered, ordered candidate work.
  [TODO.md](../TODO.md) contains only unplanned debt.
- Each numeric implementation task closed under this topology has one
  `history/M<milestone>-T<task>-*.md` record. Earlier consolidated evidence
  remains in indexed legacy material. Standalone `Td` tasks have no history
  record; their compact outcome is retained in Status and Git.
- `etc/history/legacy/` is read-only aggregation retained during migration. It
  is evidence, never a current authority.

The [Documentation Governance Gate](EXECUTION.md#documentation-governance-gate)
owns closure commands and mechanical checks. It validates topology, principal
document schemas, links, packet and identifier structure, supporting indexes,
queue/debt boundaries, artifact identity, and capped Status summaries. It does
not classify free prose as a goal, architecture decision, or roadmap item.

The task-closure audit reviews that semantic ownership against the authority
matrix above. A wording regex may be added only for a previously observed,
stable recurrence shape that a schema cannot represent; it must name the
failure it prevents and remain narrower than a general semantic classifier. A
failure blocks closure until paths, authorities, and retained-detail indexes
agree.
