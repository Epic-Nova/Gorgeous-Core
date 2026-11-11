# Repository instructions for coding agents

These instructions apply to all work in Gorgeous Core. Read this file before planning, editing code or documentation, creating commits, or opening pull requests. See [the development guide](.github/DEVELOPMENT.md) for local setup.

## Jira issue traceability (mandatory for tracked work)

- Before implementation, identify the **actual** relevant issue in the Epic Nova Jira project: https://epicnova.atlassian.net/browse/GT- .
- Never invent an issue key or claim a Jira ticket exists without verifying it. If no issue is provided and none can be confidently matched, ask the requester or clearly flag the missing link; do not silently associate an unrelated issue. Small untracked repository housekeeping can proceed without fabricating a key, but must state the exception.
- For changes addressing an existing issue, include the exact uppercase Jira key in **every relevant commit subject**, preferably at the beginning: `GT-123: Fix validation of object references`. A subject such as `fix: GT-123 validate object references` is also acceptable.
- Include the same key in the feature branch name where practical (`fix/GT-123-reference-validation`) and in the pull-request title (`GT-123: Validate schema references`). For multi-issue work, list every *genuinely affected* issue key in the PR body and include the applicable key(s) in each commit; do not attach unrelated keys.
- When preparing a commit, inspect staged changes and identify the precise Jira issue they implement. Do not amend arbitrary historical commits to add speculative Jira keys.
- In the PR description, link the ticket directly (`https://epicnova.atlassian.net/browse/GT-123`), summarize implemented scope, and provide test evidence and limitations. When possible, indicate which acceptance criteria are covered.
- Do **not** auto-transition a Jira issue to Done merely because a commit or PR exists or merged. Only mark verified acceptance criteria as complete; an open implementation PR supports In Progress, while Done requires complete, validated scope and appropriate authority.
- The Jira key allows the GitHub for Jira integration to correlate commits, branches and pull requests **when that integration is installed and connected**; this file itself does not install the integration or guarantee immediate syncing. Do not add Smart Commit directives such as `#done` or `#transition` unless explicitly requested.
- If an agent has authorized Jira access, it may update issue references and verified progress; otherwise include the issue key and links for a human to review. Never assert a ticket was updated without confirming the operation.

## Engineering and documentation

- Preserve existing Unreal Engine plugin architecture, module boundaries, and supported engine-version behavior; do not introduce circular module dependencies.
- Match the repository's existing C++ and Blueprint conventions. Add regression tests for behavior changes and document what was actually tested.
- Documentation changes must describe the actual target branch and distinguish shipped behavior from unmerged or planned work.
- Do not claim build, gameplay, platform or release verification unless it was performed. Respect user-defined exclusions from automatic Jira status updates.
