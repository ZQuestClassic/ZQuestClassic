# Contribute to Zelda Classic

From its inception Zelda Classic has been a collaborative effort. The official developer list has undergone many changes since Phantom Menace and War Lord first set about coding "Zelda DX" in C--, but the continued success of the project depends on the fresh energy and perspectives of new developers.

## How to Submit Patches

1. Create a Github account and fork the Zelda Classic project. While this step is not needed, strictly speaking, it greatly simplifies the process of submitting pull requests.

2. Clone your forked repository onto your local machine, and compile the source. Please see README.md for some build hints; if you need more help, feel free to ask on the forums.

3. Create a new branch dedicated to the changes you want to submit. **Do not skip this step! It is critical.**

4. On the new branch, make *only* the changes you want to submit to us as a patch.

5. Push the new branch to Github and submit a pull request. We will review your request and, if there are no issues with it, incorporate your changes. Please see below for detailed information about pull requests.

6. **Do not make any additional changes** to your branch, except to fix issues raised in response to your pull request. If you want to work on other changes or features, create a new, independent branch for them.

## Guide to Getting Your Request Accepted

All patches to the ZC source will be reviewed by the ZC developers. Please submit only fully-working, tested, useful code. Submit only **one independent, incremental feature at a time.** Do not hoard up hundreds of small changes and submit them in one giant patch; such patches will be rejected outright. Other reasons for rejecting a patch include:
 - it breaks the build (for any of the project targets, on any of the supported platforms) and it is not obvious to us how to fix the problem
 - it breaks old quest compatibility
 - it is clearly buggy/untested.

In addition, the following types of submissions will receive heightened scrutiny. You are strongly encouraged to discuss your intentions, and seek pre-approval from the developers, before starting work on any patch that
 - adds a major new feature (rather than fixing a bug)
 - substantially changes how the project is configured or built
 - changes the quest file format or otherwise risks breaking old quests
 - significantly increases the executable memory footprint or degrades performance
 - requires new external libraries
 - you expect might be controversial for any other reason.

## Where to Get Help

Reach out to the developers [on Discord](https://discord.gg/ddk2dk4guq).
