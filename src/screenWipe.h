#ifndef _ZC_SCREENWIPE_H_
#define _ZC_SCREENWIPE_H_

struct BITMAP;

/// Begin wipe in from black.
void beginScreenWipeIn(int centerX, int centerY);

/// Begin wipe out to black.
void beginScreenWipeOut(int centerX, int centerY);

/// Cancel any active screen wipe.
void abortScreenWipe();

/// Returns true if a screen wipe is in progress.
bool screenWipeIsActive();

/// Updates the active screen wipe.
void updateScreenWipe(bool advance);

#endif
