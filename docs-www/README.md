# docs.zquestclassic.com

This folder contains the source code for docs.zquestclassic.com. However, it is deployed from a different repo: https://github.com/ZQuestClassic/docs.zquestclassic.com/

## Building

1. Ensure the `zscript` target is already built
2. Run `bash docs-www/update.sh` to both populate the `source/zscript` files (which are generated from binding files) and run the Sphinx  generator
3. Find result at `docs-www/build/html`

## Adding a tutorial

1. Make a new file: `docs-www/source/tutorials/short-tutorial-title.rst`
2. Write the title:

```rst
Making a custom enemy
=====================
```

3. Write the contents of the tutorial. Read up on `reStructuredText` (`.rst`) [here](https://www.writethedocs.org/guide/writing/reStructuredText/)
4. Add the tutorial document name (ex: `tutorials/short-tutorial-title`) to the correct `toctree` directive in the Tutorials page index (`docs-www/source/tutorials/index.rst`)

## Deploying

1. Build the docs locally (see above)
2. From the [deployment repo](https://github.com/ZQuestClassic/docs.zquestclassic.com/), run `bash update.sh path/to/zc`
3. Commit it
4. Netlify will deploy it with a couple minutes

> TODO: move all information found in `resources/docs` to this doc site

> TODO: include this doc site in release package, remove old zscript one

> TODO: automate docs website deployment

> TODO: move all the content from resources/docs/tango/**.md here (and stop copying the old tango website docs in `update.sh`) (ditto for ghost)
