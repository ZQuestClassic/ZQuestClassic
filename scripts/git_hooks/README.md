To speed up creating the release notes in the future, we use a structured format for commit messages.

Pretty much https://www.conventionalcommits.org/en/v1.0.0/ - except no notion of BREAKING CHANGE (that is for libraries). An example of these commits are:

```
fix(zc): the thing in the player now does not crash
fix: enemies being un-killable
feat(zq): cool new combo wizard for doors
```

The idea is that all commit messages should begin first by specifying a type - `feat:` for new features, `fix:` for bug fixes. We can use anything here, but I've started us with: `feat, fix, docs, build, chore, refactor, test, ci, misc`. When adding a new feature that the end user will care about, the commit type should be `feat`. If it is just a code clean up, it'd be `refactor`.

Secondary to the commit message type is the scope. If a feature commit adds a thing in just the editor, it should start with `feat(zq):`. If it fixes a bug playing a quest, it should be `fix(zc):`. For any new gameplay feature that has an editor change too, it should just be `feat:`. The scope is optional. The scopes I've defined right now are: `zc, zq, zscript, launcher, zconsole, web`. They are largely tied to the application the commit relates to.

The only odd ones here is `zscript` - since a dedicated section of the changelog should be scripting related changes, anything that fixes a bug or adds a feature related to scripting should be given the scope `zscript` instead of the also-applicable `zc` or `zq`. Also, any changes to `src/parser` should be marked as `zscript` (ex: `fix(zscript): fix compile error with class constructors` or whatever).

There can be some ambiguity in what type or scope to us. It helps to consider how you'd want the user learning about the change to read about it in the changelog, and select types/scopes based on that. A specific ambiguous example: If you make a `feat` commit, then realize you broke the build, you could argue either way that the next commit which fixes it should be either `feat:` or `build:` or even `chore:`. My preference would be to mark the followup commit the same type as the previous, so that they get grouped together in the changelog.

In this folder are git commit message hooks to help enforce this. An example of it in action:

`git commit --allow-empty -m 'i changed the thing'`

```
i changed the thing

============== FAILED TO COMMIT ==================
commit message must match expected pattern, using an expected type and optional scope.
examples:
        fix: enemies being un-killable
        fix(zc): the thing in the player now does not crash

valid types (the first word of the commit) are: feat, fix, build, chore, ci, docs, refactor, test, misc
valid scopes (the optional text in parentheses) are: zc, zq, parser, launcher, zscript, zconsole, web


commit message is not valid, please rewrite it
to skip this validation, commit again with the --no-verify flag
for reference, the commit message is above
```

If you just do `git commit`, you'll also sometimes find a suggested commit type/scope given to you (when it can be derived automatically).
