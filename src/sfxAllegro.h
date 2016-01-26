#ifndef _ZC_SFXALLEGRO_H_
#define _ZC_SFXALLEGRO_H_

class AllegroSFX
{
public:
    AllegroSFX(int id);
    ~AllegroSFX();
    void play();
    void play(int x);
    void addLoop();
    void removeLoop();
    bool isPlaying() const;
    
private:
    int id;
    int numLoopers;
};

#endif
