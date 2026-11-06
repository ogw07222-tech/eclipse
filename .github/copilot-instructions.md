# ECLIPSE coding assistant policy (mandatory)

Before any work, read /AGENTS.md in this repository and follow it.

The project owner prohibits **all GitHub Actions runs without explicit prior approval for each run**, including builds, tests, reruns, and workflow dispatches. Never add or enable automatic triggers (push, pull_request, schedule, or related events). The owner runs tests personally. Before committing/pushing/opening/merging a PR, inspect automatic triggers and stop if the operation would run CI. Report builds/tests as not run unless independently verified by the owner.
