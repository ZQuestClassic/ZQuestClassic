This folder is currently only used for the web port.

Currently experimenting with various configs / sound files for Timidity. Not sure what is best to use, so for now
will provide multiple options. Two different methods for making Timidity cfg files:

## 1) from patch files

See `make-cfg.js`.

## 2) from soundfonts

Put sf2 files in the `soundfonts` folder.

SDL Timidity does not support sf2 directly, so we need to convert them to GUS pat files. Download and build this tool: https://github.com/psi29a/unsf

And then run `node convert-sf.js`.

- `2MGM.sf2` source: https://musical-artifacts.com/artifacts/1425
- `oot.sf2` source: https://hylianmodding.com/?p=2274
