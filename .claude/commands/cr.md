Perform a focused code review of everything that differs from `master`: committed changes on the current branch, staged changes, and unstaged changes.

Steps:
1. Determine the current branch and confirm it's not `master`/`main` itself.
2. Gather the full diff scope:
   - `git diff master...HEAD` for commits made on this branch since it diverged from master.
   - `git diff --cached` for staged-but-uncommitted changes.
   - `git diff` for unstaged changes in tracked files.
   - `git status --porcelain` to catch untracked new files, and include their full content as "added" in the review.
3. Build a mental model of the complete set of changes (branch commits + staged + unstaged + untracked) as a single unified diff against `master`.

Review rules:
- Comment ONLY on lines/logic that were actually added, removed, or modified in this diff set. Do not review or critique pre-existing code that was merely touched incidentally (e.g. moved, reformatted by a tool) unless the move/reformat itself introduces a bug.
- Do NOT comment on pre-existing issues, tech debt, or style problems in surrounding code that the diff didn't introduce.
- Do NOT speculate about downstream/side effects on other parts of the codebase unless you can point to a concrete, verifiable break (e.g. a caller that will now fail to compile or clearly misbehave). Avoid vague "this might affect X elsewhere" comments.
- Do NOT suggest unrelated refactors, cleanups, or improvements outside the scope of the changed lines.
- Focus on: correctness bugs, logic errors, edge cases introduced by the change, obvious security issues, and clear violations of patterns already established elsewhere in the same file/change.
- If a change looks correct and has no issues, say so briefly rather than inventing nitpicks.

Output format:
- Group findings by file.
- For each finding: cite the file and line(s), state the issue concisely, and explain the concrete failure scenario.
- End with a one-line overall verdict (e.g. "Looks good", "One correctness issue to fix before merging", etc.).
- Keep the review terse — no restating of the whole diff, no praise padding.
