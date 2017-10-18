#include "sound.h"
#include "zc_alleg.h"
#include "zc_sys.h"
#include "zelda.h"
#include "zeldadat.h"

/*****************************/
/**** Custom Sound System ****/
/*****************************/

static bool midi_paused=false;
static int sfx_voice[WAV_COUNT];

static inline int mixvol(int v1,int v2)
{
    return (zc_min(v1,255)*zc_min(v2,255)) >> 8;
}

// Run an NSF, or a MIDI if the NSF is missing somehow.
bool try_zcmusic(char *filename, int track, int midi)
{
    ZCMUSIC *newzcmusic = NULL;
    
    // Try the ZC directory first
    {
        char exepath[2048];
        char musicpath[2048];
        get_executable_name(exepath, 2048);
        replace_filename(musicpath, exepath, filename, 2048);
        newzcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
    }
    
    // Not in ZC directory, try the quest directory
    if(newzcmusic==NULL)
    {
        char musicpath[2048];
        replace_filename(musicpath, qstpath, filename, 2048);
        newzcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
    }
    
    // Found it
    if(newzcmusic!=NULL)
    {
        zcmusic_stop(zcmusic);
        zcmusic_unload_file(zcmusic);
        stop_midi();
        
        zcmusic=newzcmusic;
        zcmusic_play(zcmusic, emusic_volume);
        
        if(track>0)
            zcmusic_change_track(zcmusic,track);
            
        return true;
    }
    
    // Not found, play MIDI - unless this was called by a script (yay, magic numbers)
    else if(midi>-1000)
        jukebox(midi);
        
    return false;
}

void jukebox(int index,int loop)
{
    music_stop();
    
    if(index<0)         index=MAXMIDIS-1;
    
    if(index>=MAXMIDIS) index=0;
    
    music_stop();
    
    // Allegro's DIGMID driver (the one normally used on on Linux) gets
    // stuck notes when a song stops. This fixes it.
    if(strcmp(midi_driver->name, "DIGMID")==0)
        set_volume(0, 0);
        
    set_volume(-1, mixvol(tunes[index].volume,midi_volume>>1));
    play_midi((MIDI*)tunes[index].data,loop);
    
    if(tunes[index].start>0)
        midi_seek(tunes[index].start);
        
    midi_loop_end = tunes[index].loop_end;
    midi_loop_start = tunes[index].loop_start;
    
    currmidi=index;
    master_volume(digi_volume,midi_volume);
    midi_paused=false;
}

void jukebox(int index)
{
    if(index<0)         index=MAXMIDIS-1;
    
    if(index>=MAXMIDIS) index=0;
    
    // do nothing if it's already playing
    if(index==currmidi && midi_pos>=0)
    {
        midi_paused=false;
        return;
    }
    
    jukebox(index,tunes[index].loop);
}

void play_DmapMusic()
{
    static char tfile[2048];
    static int ttrack=0;
    bool domidi=false;
    
    // Seems like this ought to call try_zcmusic()...
    
    if(DMaps[currdmap].tmusic[0]!=0)
    {
        if(zcmusic==NULL ||
           strcmp(zcmusic->filename,DMaps[currdmap].tmusic)!=0 ||
           (zcmusic->type==ZCMF_GME && zcmusic->track != DMaps[currdmap].tmusictrack))
        {
            if(zcmusic != NULL)
            {
                zcmusic_stop(zcmusic);
                zcmusic_unload_file(zcmusic);
                zcmusic = NULL;
            }
            
            // Try the ZC directory first
            {
                char exepath[2048];
                char musicpath[2048];
                get_executable_name(exepath, 2048);
                replace_filename(musicpath, exepath, DMaps[currdmap].tmusic, 2048);
                zcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
            }
            
            // Not in ZC directory, try the quest directory
            if(zcmusic==NULL)
            {
                char musicpath[2048];
                replace_filename(musicpath, qstpath, DMaps[currdmap].tmusic, 2048);
                zcmusic=(ZCMUSIC*)zcmusic_load_file(musicpath);
            }
            
            if(zcmusic!=NULL)
            {
                stop_midi();
                strcpy(tfile,DMaps[currdmap].tmusic);
                zcmusic_play(zcmusic, emusic_volume);
                int temptracks=0;
                temptracks=zcmusic_get_tracks(zcmusic);
                temptracks=(temptracks<2)?1:temptracks;
                ttrack = vbound(DMaps[currdmap].tmusictrack,0,temptracks-1);
                zcmusic_change_track(zcmusic,ttrack);
            }
            else
            {
                tfile[0] = 0;
                domidi=true;
            }
        }
    }
    else
    {
        domidi=true;
    }
    
    if(domidi)
    {
        int m=DMaps[currdmap].midi;
        
        switch(m)
        {
        case 1:
            jukebox(ZC_MIDI_OVERWORLD);
            break;
            
        case 2:
            jukebox(ZC_MIDI_DUNGEON);
            break;
            
        case 3:
            jukebox(ZC_MIDI_LEVEL9);
            break;
            
        default:
            if(m>=4 && m<4+MAXCUSTOMMIDIS)
                jukebox(m-4+ZC_MIDI_COUNT);
            else
                music_stop();
        }
    }
}

void playLevelMusic()
{
    int m=tmpscr->screen_midi;
    
    switch(m)
    {
    case -2:
        music_stop();
        break;
        
    case -1:
        play_DmapMusic();
        break;
        
    case 1:
        jukebox(ZC_MIDI_OVERWORLD);
        break;
        
    case 2:
        jukebox(ZC_MIDI_DUNGEON);
        break;
        
    case 3:
        jukebox(ZC_MIDI_LEVEL9);
        break;
        
    default:
        if(m>=4 && m<4+MAXCUSTOMMIDIS)
            jukebox(m-4+ZC_MIDI_COUNT);
        else
            music_stop();
    }
}

bool get_midi_paused()
{
    return midi_paused;
}

void set_midi_paused(bool paused)
{
    midi_paused=paused;
}

void music_pause()
{
    //al_pause_duh(tmplayer);
    zcmusic_pause(zcmusic, ZCM_PAUSE);
    midi_pause();
    set_midi_paused(true);
}

void music_resume()
{
    //al_resume_duh(tmplayer);
    zcmusic_pause(zcmusic, ZCM_RESUME);
    midi_resume();
    set_midi_paused(false);
}

void music_stop()
{
    //al_stop_duh(tmplayer);
    //unload_duh(tmusic);
    //tmusic=NULL;
    //tmplayer=NULL;
    zcmusic_stop(zcmusic);
    zcmusic_unload_file(zcmusic);
    stop_midi();
    set_midi_paused(false);
    currmidi=0;
}

void master_volume(int dv,int mv)
{
    if(dv>=0) digi_volume=zc_max(zc_min(dv,255),0);
    
    if(mv>=0) midi_volume=zc_max(zc_min(mv,255),0);
    
    int i = zc_min(zc_max(currmidi,0),MAXMIDIS-1);
    set_volume(digi_volume,mixvol(tunes[i].volume,midi_volume));
}

void set_all_voice_volume(int sfx_volume)
{
    for(int i=0; i<WAV_COUNT; ++i)
    {
        //allegro assertion fails when passing in -1 as voice -DD
        if(sfx_voice[i] > 0)
            voice_set_volume(sfx_voice[i], sfx_volume);
    }
}

/*****************/
/*****  SFX  *****/
/*****************/


// array of voices, one for each sfx sample in the data file
// 0+ = voice #
// -1 = voice not allocated
void Z_init_sound()
{
    for(int i=0; i<WAV_COUNT; i++)
        sfx_voice[i]=-1;
        
    for(int i=0; i<ZC_MIDI_COUNT; i++)
        tunes[i].data = (MIDI*)mididata[i].dat;
        
    for(int j=0; j<MAXCUSTOMMIDIS; j++)
        tunes[ZC_MIDI_COUNT+j].data=NULL;
        
    master_volume(digi_volume,midi_volume);
}

// clean up finished samples
void sfx_cleanup()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1 && voice_get_position(sfx_voice[i])<0)
        {
            deallocate_voice(sfx_voice[i]);
            sfx_voice[i]=-1;
        }
}

// allocates a voice for the sample "wav_index" (index into zelda.dat)
// if a voice is already allocated (and/or playing), then it just returns true
// Returns true:  voice is allocated
//         false: unsuccessful
bool sfx_init(int index)
{
    // check index
    if(index<=0 || index>=WAV_COUNT)
        return false;
        
    if(sfx_voice[index]==-1)
    {
        if(sfxdat)
        {
            if(index<Z35)
            {
                sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[index].dat);
            }
            else
            {
                sfx_voice[index]=allocate_voice((SAMPLE*)sfxdata[Z35].dat);
            }
        }
        else
        {
            sfx_voice[index]=allocate_voice(&customsfxdata[index]);
        }
        
        voice_set_volume(sfx_voice[index], sfx_volume);
    }
    
    return sfx_voice[index] != -1;
}

// plays an sfx sample
void sfx(int index,int pan,bool loop, bool restart)
{
    if(!sfx_init(index))
        return;
        
    voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
    voice_set_pan(sfx_voice[index],pan);
    
    int pos = voice_get_position(sfx_voice[index]);
    
    if(restart) voice_set_position(sfx_voice[index],0);
    
    if(pos<=0)
        voice_start(sfx_voice[index]);
}

// true if sfx is allocated
bool sfx_allocated(int index)
{
    return (index>0 && index<WAV_COUNT && sfx_voice[index]!=-1);
}

// start it (in loop mode) if it's not already playing,
// otherwise adjust it to play in loop mode -DD
void cont_sfx(int index)
{
    if(!sfx_init(index))
    {
        return;
    }
    
    if(voice_get_position(sfx_voice[index])<=0)
    {
        voice_set_position(sfx_voice[index],0);
        voice_set_playmode(sfx_voice[index],PLAYMODE_LOOP);
        voice_start(sfx_voice[index]);
    }
    else
    {
        adjust_sfx(index, 128, true);
    }
}

// adjust parameters while playing
void adjust_sfx(int index,int pan,bool loop)
{
    if(index<=0 || index>=WAV_COUNT || sfx_voice[index]==-1)
        return;
        
    voice_set_playmode(sfx_voice[index],loop?PLAYMODE_LOOP:PLAYMODE_PLAY);
    voice_set_pan(sfx_voice[index],pan);
}

// pauses a voice
void pause_sfx(int index)
{
    if(index>0 && index<WAV_COUNT && sfx_voice[index]!=-1)
        voice_stop(sfx_voice[index]);
}

// resumes a voice
void resume_sfx(int index)
{
    if(index>0 && index<WAV_COUNT && sfx_voice[index]!=-1)
        voice_start(sfx_voice[index]);
}

// pauses all active voices
void pause_all_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
            voice_stop(sfx_voice[i]);
}

// resumes all paused voices
void resume_all_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
            voice_start(sfx_voice[i]);
}

// stops an sfx and deallocates the voice
void stop_sfx(int index)
{
    if(index<=0 || index>=WAV_COUNT)
        return;
        
    if(sfx_voice[index]!=-1)
    {
        deallocate_voice(sfx_voice[index]);
        sfx_voice[index]=-1;
    }
}

// Stops SFX played by Link's item of the given family
void stop_item_sfx(int family)
{
    int id=current_item_id(family);
    
    if(id<0)
        return;
        
    stop_sfx(itemsbuf[id].usesound);
}

void kill_sfx()
{
    for(int i=0; i<WAV_COUNT; i++)
        if(sfx_voice[i]!=-1)
        {
            deallocate_voice(sfx_voice[i]);
            sfx_voice[i]=-1;
        }
}

int pan(int x)
{
    switch(pan_style)
    {
    case 0:
        return 128;
        
    case 1:
        return vbound((x>>1)+68,0,255);
        
    case 2:
        return vbound(((x*3)>>2)+36,0,255);
    }
    
    return vbound(x,0,255);
}
