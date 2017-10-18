#ifndef _ZC_SEQUENCE_SEQUENCE_H_
#define _ZC_SEQUENCE_SEQUENCE_H_

// Sequences are special events that suspend gameplay entirely.
class Sequence
{
public:
    Sequence(): finished(false) {}
    virtual ~Sequence() {}
    virtual void activate()=0;
    virtual void update()=0;
    inline bool isFinished() const { return finished; }
    
protected:
    inline void finish() { finished=true; }
    
private:
    bool finished;
};

#endif
