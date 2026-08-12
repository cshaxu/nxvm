# Contributing

Read the [Documentation Guide](docs/README.md), the active
[Project Status](docs/STATUS.md), and [Execution Rules](docs/rules/EXECUTION.md)
before proposing or changing work. Follow the guide's **Task Reading Set** for
your role and change surface. The design and rule documents linked there are
the implementation authorities; this file does not restate them.

## Change Submission

- Follow the applicable [Architecture Rules](docs/rules/ARCHITECTURE.md) and
  [Coding Rules](docs/rules/CODING.md), and always follow
  [Execution Rules](docs/rules/EXECUTION.md).
- Follow the [source and research procedure](docs/etc/operations/policy/source-policy.md)
  before importing, deriving, or packaging third-party source, firmware, guest
  media, or Microsoft material.
- Treat research and optional local tools as non-product inputs unless an
  approved task explicitly admits them.

## Review Record

Every change records:

- affected ownership boundary and user-visible behavior;
- source provenance, license, and redistributability impact when applicable;
- tests and evidence added or updated; and
- any deferred issue, owner-approved exception, or research boundary.

Report only information applicable to the report type. A completion or blocker
delivery uses four items: completion or blocker, pushed commit, verification
result or evidence location, and known boundary or transfer. Contract
confirmation or objection and S-required progress reports state the applicable
decision or progress node and link to evidence. Link to committed evidence,
task history, or indexed supporting material instead of copying command output,
requirement matrices, or an implementation narrative into the conversation.
This report format is an index; it does not replace the complete review and
evidence record required by the [Execution Rules](docs/rules/EXECUTION.md).

## Commits And Tracking

Use the identifier, commit-subject, artifact, cleanup, and closure rules in
[Execution Rules](docs/rules/EXECUTION.md). Do not allocate a task identifier
or treat a local developer artifact as release evidence outside that policy.
