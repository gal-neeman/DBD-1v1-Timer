Create a pull request to `master` for the current work.

Steps:
1. Run `git branch --show-current` to determine the current branch.
2. If the current branch is `master` (or `main`):
   - Ask me for: an issue number, a type (`bug` or `feature`), and a short general title.
   - Convert the title to kebab-case (lowercase, spaces/punctuation → hyphens).
   - Run `git checkout -b <bug|feature>/<issue-number>/<name-kebab-case>` to create and switch to the new branch.
3. If I was already on a branch other than `master`/`main`, skip the branch creation above and use the current branch.
4. Stage and commit all changes (staged + unstaged, including untracked files relevant to the work):
   - Run `git status` and `git diff` / `git diff --cached` to understand what changed.
   - Run `git add -A` (excluding anything that looks like a secret or credential file — warn me if you see one).
   - Write a concise commit message describing the "why" based on the actual diff, following this repo's existing commit style (see `git log` for examples).
   - Commit the changes.
5. Push the branch to the remote with `git push -u origin <branch-name>`.
6. Create a pull request into `master` using `gh pr create`, with a short title and a body summarizing the changes (Summary + Test plan sections). Return the PR URL when done.

Confirm with me before pushing and before creating the PR if anything about the scope of changes seems ambiguous or unexpected.
