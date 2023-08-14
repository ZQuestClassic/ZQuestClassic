Files in this directory massage the commit history for purposes of generating a better changelog. Each file here will be processed
for overrides.

Override types:
- `subject`: change the first line of a commit message, leaving the body intact
- `reword`: change a commit message, including the first line. Subsequent lines will be used as the commit message, only ending at `=end`

`generic_changes.md` is for one-off commits. For commits belonging to a specific project, make a new file matching `YYYY_MM_DD_some_short_description.md`. The specific file an override is in does not matter for the end result, it is only for organization purposes.

This helpful command will log commits with their full hashes:

```sh
git log --format="%C(auto)pick %H %s%n%b%-"
```
