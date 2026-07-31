---
name: cherry-pick-2-55
description: Instructions for how to cherry-pick changes from main to the releases/2.55 branch
---

You MUST be on the releases/2.55 branch for this. Per CLAUDE.md, use the
`../ZQuestClassic-2.55` worktree rather than switching the main checkout, and run
all git/build/test commands from there (the worktree gets its own build folder).

Here is how to help cherry-pick changes from main to the releases/2.55 branch.

We first need a list of candidate commits, saved to a file (for example
/Users/connorclark/Downloads/2.55.15.txt). The user will need to create this list
manually to start, and prune what isn't relevant before cherry-picking begins.

It should look like this: the commit sha followed by the oneline message:

```
624a236d28c6 misc(zq)!: improve infotext for string editor 'Layer'
a139c83ab090 fix(zc): non-triggering weapons still trigger secret flags
5dffacbc8471 fix(zc): pushblock lens hints not drawing over block sprite layer
b8a56512ae6f refactor(zc): greatly speed up dithercircfill
a6d3ae970117 fix(zc): Heart Container / Magic Container cheats using outdated values
```

You can use `manage_commits.py path/to/file.txt` to normalize this list of commits (dropping ones already cherry-picked), and print ready-to-run `git cherry-pick -x` commands for the ones that apply without conflict. The list is ordered most recent first, so start at the end of the list.

Some guidelines:

- Cherry-pick starting from the oldest candidates.
- use `git cherry-pick -x`, and resolve the conflict for me. Do not EVER change the commit message.
- Write a file .tmp/CHERRY_PICK_PROGRESS.md and record notes for every commit you assess. Be sure to append to the file, not overwrite it.
- Validate each cherry-pick'd commit via `cmake --build build --config Release -t all` and `python tests/run_replay_tests.py --filter playground`
- If a commit modifies anything in `tests`, you should probably take only the .zs changes and drop playground.qst or .zplay changes. Will need to regenerate those for 2.55.
- I'm pretty certain most candidate commits are relevant to cherry-pick, but I may be wrong about some. If the merge conflict suggests many things are missing, skip it and make a note about why
- Never update the replay version in replay.cpp - when a cherry-pick'd commit uses a new replay check, you must add a new function in replay_compat.cpp and use that instead.
- The main branch may have other changes that make cherry-picks not apply cleanly. If possible, make as minimal a change as possible to adapt the commit. If too much is missing, just skip it and move on.
- The qst file format reading code is split across many files on main, but for the 2.55 branch it still is in one file qst.cpp (for example, main has src/core/qst_rules.cpp but in this branch src/qst.cpp has the rules section, and all other sections)
- When a cherry-pick'd commit increases V_COMPATRULE, instead of changing it in 2.55 (we cannot) check the ZC version w/ tempheader.compareVer. See the bottom of readrules in qst.cpp
