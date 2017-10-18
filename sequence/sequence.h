#ifndef _ZC_SEQUENCE_SEQUENCE_H_
#define _ZC_SEQUENCE_SEQUENCE_H_

enum sequenceID
{
	seq_ganonIntro, seq_getTriforce, seq_getBigTriforce, seq_gameOver,
	seq_whistle, seq_potion
};

void beginSpecialSequence(sequenceID id, int itemID=0);
bool specialSequenceIsActive();
void updateSpecialSequence();
void clearSpecialSequence();


// Sequences are special events that suspend gameplay entirely.
class Sequence
{
public:
    Sequence();
    virtual ~Sequence();
    virtual void activate()=0;
    virtual void update()=0;
    inline bool isFinished() const { return finished; }
    
protected:
    inline void finish() { finished=true; }
    
private:
    bool finished;
};

#endif
