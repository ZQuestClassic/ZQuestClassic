#ifndef SFXBACKEND_H
#define SFXBACKEND_H

#include "../zc_alleg.h"
#include <string>
#include <vector>
#include <map>

/*
 * Data structure for storing a sound effect sample, consisting of the raw
 * sound data as well as some metadata (wether or not the sound effect is built
 * into ZC, or loaded from file by a custom quest; and the name of the sound
 * effect).
 */
struct SFXSample
{
    /*
     * Constructs a new sound effect from a sample. Deep copies the sample.
     * Doesn't set any of the other fields of the data structure.
     */
    SFXSample(const SAMPLE &sample);
    ~SFXSample();

    SAMPLE sample;
    bool iscustom;
    std::string name;

private:
    SFXSample();
    SFXSample(const SFXSample &other);
    SFXSample &operator=(const SFXSample &other);
};

struct SFXVoice
{
    int voice;
    int loopCount;
};

class SFXBackend
{
public:    
    ~SFXBackend();

    /*
    * Read the configuration options from the config file. The options will be
    * read from the config section "XXX-sfx" where XXX is replaced
    * by the prefix string.
    *
    * Current options are:
    * - volume: The master volume at which all sound effects will play, on a
    *           range from 0 to 255.
    * - pan:    How panning of the sound effect is to be handled. Settings
    *           currently allowed are 0 (no panning), 1 (pan using the middle
    *           1/2 of the panning range) or 2 (pan using the middle 3/4 of
    *           the panning range) and 3 (full pan).
    */
    void readConfigurationOptions(const std::string &prefix);

    /*
    * Writes the configuration options to the config file. The options will be
    * written to the config section "XXX-sfx" where XXX is replaced by the
    * prefix string.
    */
    void writeConfigurationOptions(const std::string &prefix);

    /*
    * Replaces the current sound effect in slot sample with the deault sound
    * effect stored in sfxdata, and assigns it the corresponding name in
    * the sfxnames list.
    * Does not (and cannot) check whether sample is a valid index into sfxdata
    * or sfxnames -- this must be guaranteed by the caller.
    * If the sound effect in slot sample is currently playing, it will be
    * stopped as part of the replacement process.
    */
    void loadDefaultSample(int sample, DATAFILE *sfxdata, const char * const *sfxnames);

    /*
    * Clears all sound effects that have been previously loaded and repalces
    * them all with the default set of sound effects, stored in sfxdat, and
    * sets their names to those in sfxnames. Requires the caller to specify
    * how many samples are stored in these data structures in numsamples.
    * Stops all sound effect playback as part of the replacement process.
    */
    void loadDefaultSamples(int numsamples, DATAFILE *sfxdata, const char * const *sfxnames);

    /*
    * Stops all currently-playing sound effects and unloads them all.
    */
    void clearSamples();

    /*
    * Removes the sound effect stored in the specified slot. This will stop
    * playback of the selected sound effect, if it is currently playing.
    * Does nothing if the slot number does not correspond to a legal sound
    * effect slot.
    * For the purposes of numSamples(), this method will not truncate the sound
    * effect list, if called on the last sound effect. The sound effect list
    * will simply contain an empty slot at the end, instead.
    */
    void clearSample(int slot);

    /*
    * Loads a new sound effect into the specified slot, and assigns it the
    * given name. This replaces the previous sound effect in that slot and
    * stops its playback, if it is currently playing.
    * Does nothing if slot is negative.
    * If slot is greater than the current number of allocated sound effect
    * slots, this method will create new empty slots so that all slots
    * 0, 1, ..., slot now exist. Slots created in this way are empty but can
    * be filled with later calls to loadSample, etc.
    */
    void loadSample(int slot, const SAMPLE &sfx, const std::string name);

    /*
    * Returns the sound effect currently stored in the specified slot.
    * Returns NULL if the slot is empty (i.e. no sound effect is stored there)
    * or if the slot index is illegal (negative, or larger than all allocated
    * sound effect slots.)
    */
    const SFXSample *getSample(int slot);

    /*
    * Flags the sound effect in the given slot as being a custom sound effect.
    * This does nothing on its own but this flag is maintained by the backend
    * and returned in the metadata queried by getSample(), etc.
    * Does nothing if the slot number is illegal (negative or corresponding to
    * an empty slot.)
    */
    void setIsCustom(int slot, bool isCustom);

    /*
    * Returns the number of sound effect slots being tracked by the backend.
    * This is at least the number of sound effects that have been loaded, but
    * also counts empty slots in between the allocated sound effects, slots
    * for sound effects that have been previously been removed, etc.
    */
    int numSlots();

    /*
    * Returns the current sound effect volume, from 0 to 255.
    */
    int getVolume();

    /*
    * Sets the master volume for all sound effects. The volume must be in the 
    * range [0,255].
    * Currently-playing sound effects will have their volume adjusted, and all
    * later sound effects will play with the specified volume.
    */
    void setVolume(int volume);

    /*
    * Returns the current pan style, an integer from 0 to 3:
    * 0: no panning
    * 1: 1/2 panning
    * 2: 3/4 panning
    * 3: full panning.
    */
    int getPanStyle();

    /*
    * Sets the pan style for all subsequent sound effects. See getPanStyle()
    * for the list of allowed pan style values. Calling this function will
    * only affect future sound effects: currently-playing sounds (including
    * looping sound effects) continue to use the original value of the pan
    * style.
    */
    void setPanStyle(int panstyle);

    /*
    * Plays the sound effect in the given slot. Pan specifies the horizontal
    * location of the sound effect, from far left (0) to far right (255); the
    * actual effect of this value is determined by the current pan style, as
    * specified using setPanStyle().
    * Does nothing if no sound effect is loaded into the given slot.
    * If the sound effect is currently playing looped, this call restarts the
    * playback, but the sound effect will continue to loop.
    * Otherwise, if the sound is currently playing, playback will be
    * interrupted, then the sound will play once from the start.
    * This call will also reset the pan location of the sound effect, if the
    * sound effect is currently playing.
    *
    * There is some chance that the sound effect does not play, due to
    * hardware limitations on the number of sounds that can be played
    * simultaneously.
    */
    void play(int slot, int pan);

    /*
    * Stops playback of the sound effect in the given slot, if it is currently
    * playing. Does nothing if the sound isn't playing or if the given slot
    * does not correspond to a legal sound effect.
    * This will also (permanently) stop looping sound effects.
    */
    void stop(int slot);

    /*
    * Stops playback of all sound effects.
    */
    void stopAll();

    /*
    * Sets the sound effect in the given slot to play, and to continue
    * looping indefinitely, until the sound effect is stopped or unlooped.
    * This method can be called if the sound effect is already playing. If the
    * sound effect was previously specified to play only once, it will
    * continue looping after playback is finished. If the sound effect was
    * already scheduled to loop, this call will increment the "loop"-ness of
    * the sound effect, increasing the number of times unloop() must be called
    * to stop the sound effect by one.
    *
    * This call will also reset the pan location of the sound effect, if the
    * sound effect is currently playing.
    * 
    * There is some chance that the sound effect does not play, due to
    * hardware limitations on the number of sounds that can be played
    * simultaneously.
    */
    void loop(int slot, int pan);

    /*
    * Schedules a sound effect to stop looping, after playback of the current
    * iteration of the effect has finishes. Use stop() to end playback
    * immediately.
    * Does nothing if the sound effect in the given slot is not currently
    * playing in looped mode.
    *
    * If loop() has been called multiple times, unloop() does nothing until
    * it has been called an equal number of times as loop() was previously
    * called.
    */
    void unloop(int slot);

    /*
    * Returns true if the sound effect in the given slot is currently playing.
    * Returns false otherwise (i.e., if the sound effect is not playing,
    * or no valid sound effect exists in the specified slot.)
    */
    bool isPlaying(int slot);

    /*
    * Pauses playback of the sound in the given slot, until the next call to
    * resume(). Does nothing if the given slot does not contain a sound effect
    * that is currently playing.
    */
    void pause(int slot);

    /*
    * Pauses all sounds currently playing. They can be resumed with 
    * resumeAll().
    */
    void pauseAll();

    /*
    * Resumes playback of the sound effect in the given slot. Does nothing
    * if the sound effect in that slot was not previously paused.
    */
    void resume(int slot);

    /*
    * Resumes all paused sound effects.
    */
    void resumeAll();

    /*
    * Informs the backend that the current time is a good time to do some
    * cleanup. This must be called every now and then since the hardware
    * supports only a limited number of sound effects playing at any given
    * time; this method will find sound effects that have finished playing
    * and unload them from the hardware, freeing room for future sound effects.
    */
    void garbageCollect();
	
	friend class Backend;

private:
    std::vector<SFXSample *> samples_;
    std::vector<SFXVoice> voices_;
    int volume_;
    int pan_style_;

    
    int allocateVoice(int slot);
    int adjustPan(int pan);

	SFXBackend();
};


#endif
