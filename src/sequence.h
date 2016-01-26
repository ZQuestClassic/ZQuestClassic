#ifndef _ZC_SEQUENCE_H_
#define _ZC_SEQUENCE_H_

enum sequenceID
{
    seq_ganonIntro, seq_getTriforce, seq_getBigTriforce, seq_gameOver,
    seq_whistle, seq_potion
};

void beginSpecialSequence(sequenceID id, int itemID=0);
bool specialSequenceIsActive();
void updateSpecialSequence();
void clearSpecialSequence();

#endif
