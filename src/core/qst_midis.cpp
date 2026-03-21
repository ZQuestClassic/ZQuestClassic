#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern bitstring midi_bitstr;

namespace {

void *read_block(PACKFILE *f, int32_t size, int32_t alloc_size)
{
    void *p;
    
    p = _AL_MALLOC(MAX(size, alloc_size));
    
    if(!p)
    {
        return NULL;
    }
    
    if(!pfread(p,size,f))
    {
        _AL_FREE(p);
        return NULL;
    }
    
    if(pack_ferror(f))
    {
        _AL_FREE(p);
        return NULL;
    }
    
    return p;
}

/* read_midi:
  *  Reads MIDI data from a datafile (this is not the same thing as the
  *  standard midi file format).
  */
MIDI *read_midi(PACKFILE *f)
{
    MIDI *m;
    int32_t c;
    int16_t divisions=0;
    int32_t len=0;
    
    m = (MIDI*)_AL_MALLOC(sizeof(MIDI));
    
    if(!m) return NULL;
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        m->track[c].len = 0;
        m->track[c].data = NULL;
    }
    
    p_mgetw(&divisions,f);
    m->divisions=divisions;
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        p_mgetl(&len,f);
        m->track[c].len=len;
        
        if(m->track[c].len > 0)
        {
            m->track[c].data = (byte*)read_block(f, m->track[c].len, 0);
            
            if(!m->track[c].data)
            {
                destroy_midi(m);
                return NULL;
            }
        }
    }
    
    LOCK_DATA(m, sizeof(MIDI));
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        if(m->track[c].data)
        {
            LOCK_DATA(m->track[c].data, m->track[c].len);
        }
    }
    
    return m;
}

} // end namespace

int32_t readmidis(PACKFILE *f, zquestheader *Header, zctune *tunes /*zcmidi_ *midis*/)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_midis);
	
	byte temp_midi_flags[32];
	
	int32_t dummy;
	word dummy2;
	// zcmidi_ temp_midi;
	word tunes_to_read;
	word section_version=0;
	zctune temp;
	char old_title[37] = {0};
	
	if(Header->zelda_version < 0x193)
	{
		if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<178)))
		{
			tunes_to_read=MAXCUSTOMMIDIS192b177;
		}
		else
		{
			tunes_to_read=MAXCUSTOMMIDIS;
		}
	}
	else
	{
		//section version info
		if(!p_igetw(&section_version,f))
		{
			return qe_invalid;
		}

		if (section_version > V_MIDIS)
			return qe_version;
	
		if (!should_skip)
			FFCore.quest_format[vMIDIs] = section_version;
		
		if(!p_igetw(&dummy2,f))
		{
			return qe_invalid;
		}
		
		//section size
		if(!p_igetl(&dummy,f))
		{
			return qe_invalid;
		}
		
		//finally...  section data
		
		if (section_version < 5)
		{
			if(!pfread(temp_midi_flags,32,f))
				return qe_invalid;
			vector<byte> vb{ temp_midi_flags, temp_midi_flags + 32 };
			midi_bitstr.inner() = vb;
			tunes_to_read = 252;
		}
		else
		{
			if (!p_getbitstr(&midi_bitstr, f))
				return qe_invalid;
			if (!p_igetw(&tunes_to_read, f))
				return qe_invalid;
			if (tunes_to_read > MAXCUSTOMMIDIS)
				return qe_invalid;
		}
	}
	
	if (!should_skip)
		for(uint q = 0; q < MAXCUSTOMMIDIS; ++q)
			tunes[q].reset();
	
	for(uint i = 0; i < tunes_to_read; ++i)
	{
		auto& temp_tune = should_skip ? temp : tunes[i];
		if (should_skip) temp.reset();
        
        if (!midi_bitstr.get(i)) continue;
		
		if (section_version > 4)
		{
			if (!p_getwstr(&temp_tune.song_title, f))
				return qe_invalid;
		}
		else if (section_version == 4)
		{
			if (!p_getstr(old_title,36,f))
				return qe_invalid;
			temp_tune.song_title = old_title;
		}
		else if (section_version < 4)
		{
			if (!p_getstr(old_title,20,f))
				return qe_invalid;
			temp_tune.song_title = old_title;
		}
		
		if (!p_igetl(&temp_tune.start,f))
			return qe_invalid;
		
		if (!p_igetl(&temp_tune.loop_start,f))
			return qe_invalid;
		
		if (!p_igetl(&temp_tune.loop_end,f))
			return qe_invalid;
		
		if (!p_igetw(&temp_tune.loop,f))
			return qe_invalid;
		
		if (!p_igetw(&temp_tune.volume,f))
			return qe_invalid;
		
		if (Header->zelda_version < 0x193)
			if(!p_igetl(&dummy,f))
				return qe_invalid;
		
		if (section_version >= 3)
			if(!pfread(&temp_tune.flags,sizeof(temp_tune.flags),f))
				return qe_invalid;
		
		if (section_version < 2)
		{
			if((temp_tune.data=read_midi(f))==NULL)
				return qe_invalid;
		}
		else
		{
			byte format;
			if(!pfread(&format,sizeof(format),f))
				return qe_invalid;
			
			// MIDI is the only format saved here.
			// Never did more than MIDI for a zctune, and no plans to now.
			if (format != MFORMAT_MIDI)
				return qe_invalid;
			
			temp_tune.data = read_midi(f);
			if (!temp_tune.data)
				return qe_invalid;
		}
	}
	temp.reset();
	return 0;
}
