## AI tool usage

Some ZQuest Classic maintainers use AI tools to assist with development.
Individual developers vary in how much they use them, from heavily to not at
all. This doc explains how those tools are and aren't used in this project, and
expectations around AI usage for external contributors.

### **How they're used**

AI tools assist with diagnosing bugs (reproducing them with replays, identifying
root causes, drafting fixes and regression tests), reviewing code changes,
writing tests, drafting commit messages, and automating development chores such
as cherry-picking fixes to release branches. They are also used to generate
code: most often tests, build scripts, bug fixes, and occasionally a first draft
of a larger change, which is then researched, reworked, and tested by a
maintainer until it meets the same bar as hand-written code. Most code is
written by hand.

### **Responsibility**

Every change is reviewed and tested by a maintainer, who is accountable for it
like any other code. Code only lands because a maintainer judges it correct and
maintainable. Correctness is verified the same way as for any change: replays
and other tests that run in CI.

### **What AI is not used for**

No generative art: for example, a future custom tileset bundled with the engine
would not contain any AI-generated art (graphics, audio, etc).

We also won't add features that let AI make quests for you. So no AI
integrations in the editor or features that use AI for generating quest content.
Quest making is a creative act and keeping AI out of that process is a goal of
the project.

No Patreon funds are spent on AI services.

### **External contributions**

If you use AI tools to help produce a pull request, say so in the PR
description. You are responsible for understanding and being able to fix any
code you submit, however it was produced. Any failure to disclose obviously
AI-generated code may result in the PR being closed without much discussion.

If AI tooling is used to assist in your understanding of the code, please share
or summarize the result of that so that a maintainer may weigh in on its
correctness (this helps us catch cases where the AI got it wrong, and helps us
direct the PR back on track).

Essentially: just be honest - your time and effort are much appreciated.

### **Timeline**

The last releases containing no agent-assisted contributions are `2.55.14` and
`3.0.0-prerelease.187+2026-05-14`.
