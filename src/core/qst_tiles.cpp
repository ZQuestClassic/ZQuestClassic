#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

int32_t read_tile_entry(PACKFILE *f, tiledata *buf, int32_t i, byte *temp_tile, bool skip)
{
	byte format = tf4Bit;
	memset(temp_tile, 0, tilesize(tf32Bit));
	if(!p_getc(&format,f))
		return qe_invalid;
	if(!format)
	{
		if(!skip)
			reset_tile(buf, i, tf4Bit);
		return 0;
	}
	int size = format == tf4Bit ? 128 : tilesize(format);
	if(!pfread(temp_tile, size, f))
		return qe_invalid;
	if(skip)
		return 0;
	buf[i].format = format;
	if(buf[i].data)
	{
		free(buf[i].data);
		buf[i].data = NULL;
	}
	buf[i].data = (byte *)malloc(tilesize(buf[i].format));
	if(format == tf4Bit)
	{
		byte temp[256];
		byte *si = temp_tile + 128;
		byte *di = temp + 256;
		for(int j=127; j>=0; --j)
		{
			(*(--di)) = (*(--si)) >> 4;
			(*(--di)) = (*si) & 15;
		}
		memcpy(buf[i].data, temp, 256);
	}
	else
	{
		memcpy(buf[i].data, temp_tile, tilesize(buf[i].format));
	}
	return 0;
}

int32_t readtiles(PACKFILE *f, tiledata *buf, zquestheader *Header, word version, word build, word start_tile, int32_t max_tiles, bool from_init)
{
    bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_tiles);

    int32_t tiles_used=0;
	word section_version = 0;
	int32_t section_size= 0;
    byte *temp_tile = new byte[tilesize(tf32Bit)];
	
    //Tile Expansion
    //if ( version >= 0x254 && build >= 41 )
    if (version < 0x254 && build < 41)
    {
	    max_tiles = ZC250MAXTILES;
    }
    
	
    if(Header!=NULL&&(!Header->data_flags[ZQ_TILES]&&!from_init))         //keep for old quests
    {
		if(!init_tiles_for_190(true, Header))
		{
			al_trace("Unable to initialize tiles\n");
		}
        
        delete[] temp_tile;
        temp_tile=NULL;
        return 0;
    }
    else
    {
        if(version > 0x192)
        {
            //section version info
            if(!p_igetw(&section_version,f))
            {
                delete[] temp_tile;
                return qe_invalid;
            }

			if (section_version > V_TILES)
				return qe_version;
            
			FFCore.quest_format[vTiles] = section_version;
			
            if(!read_deprecated_section_cversion(f))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
            
            //section size
            if(!p_igetl(&section_size,f))
            {
                delete[] temp_tile;
                return qe_invalid;
            }
        }
        
		//if ( build < 41 ) 
		//{
		//	tiles_used = ZC250MAXTILES;
		//}
		
        if(version < 0x174)
        {
            tiles_used=TILES_PER_PAGE*4;
        }                                                       //no expanded tile space
        else if(version < 0x191)
        {
            tiles_used=OLDMAXTILES;
        }
        else
        {
            //finally...  section data
            if ( version >= 0x254 && build >= 41 ) //read and write the size of tiles_used properly
			{ 
				if(!p_igetl(&tiles_used,f))
				{
					delete[] temp_tile;
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetw(&tiles_used,f))
				{
					delete[] temp_tile;
					return qe_invalid;
				}
			}
        }
        
        tiles_used=zc_min(tiles_used, max_tiles);
        
		//if ( version < 0x254 || ( version >= 0x254 && build < 41 )) //don't do this, it crashes ZQuest. -Z
		//if ( version < 0x254 && build < 41 )
		if ( version < 0x254 || (version == 0x254 && build < 41) )
		//if ( build < 41 )
		{
			tiles_used=zc_min(tiles_used, ZC250MAXTILES-start_tile);
		}
		else //2.55
		{
			tiles_used = zc_min(tiles_used,NEWMAXTILES-start_tile); 
		}
		
		//if ( section_version > 1 ) tiles_used = NEWMAXTILES;
			
	
        bool modern_format = (version > 0x211) || ((version == 0x211) && (build > 4));
        bool blank_tile_opt = section_version > 2;

        for(int32_t i=0; i<tiles_used; ++i)
        {
            if(modern_format && blank_tile_opt)
            {
                int ret = read_tile_entry(f, buf, start_tile+i, temp_tile, should_skip);
                if(ret) { delete[] temp_tile; return ret; }
                continue;
            }

            // Legacy path: either no format byte in file (v<=0x211), or no blank-tile
            // optimization (section_version<=2, so format=0 means data still follows).
            byte format = tf4Bit;
            memset(temp_tile, 0, tilesize(tf32Bit));

            if(modern_format)
            {
                if(!p_getc(&format, f)) { delete[] temp_tile; return qe_invalid; }
            }

            int size = format == tf4Bit ? 128 : tilesize(format);
            if(!pfread(temp_tile, size, f)) { delete[] temp_tile; return qe_invalid; }

            if(should_skip)
                continue;

            buf[start_tile+i].format = format;
            if(buf[start_tile+i].data)
            {
                free(buf[start_tile+i].data);
                buf[start_tile+i].data = NULL;
            }
            buf[start_tile+i].data = (byte*)malloc(tilesize(buf[start_tile+i].format));
            if(format == tf4Bit)
            {
                byte temp[256];
                byte *si = temp_tile + 128;
                byte *di = temp + 256;
                for(int j = 127; j >= 0; --j)
                {
                    (*(--di)) = (*(--si)) >> 4;
                    (*(--di)) = (*si) & 15;
                }
                memcpy(buf[start_tile+i].data, temp, 256);
            }
            else
            {
                memcpy(buf[start_tile+i].data, temp_tile, tilesize(buf[start_tile+i].format));
            }
        }
    }

	if (should_skip)
		return 0;
    
	if ( section_version < 2 ) //write blank tile data --check s_version with this again instead?
	{
		for ( int32_t q = ZC250MAXTILES; q < NEWMAXTILES; ++q )
		{
			
			//memcpy(buf[q].data,temp_tile,tilesize(buf[q].format));
			reset_tile(buf,q,tf4Bit);
			
			
			/*
			
			byte tempbyte;
			for(int32_t i=0; i<tilesize(tf4Bit); i++)
			{
				tempbyte=buf[ZC250MAXTILES-1].data[i];
				buf[q].data[i] = tempbyte;
			}
			//int32_t temp = tempbyte=buf[130].data[i];
			//buf[q].data = buf[ZC250MAXTILES-1].data;
			*/
			//reset_tile(buf,q,tf4Bit);
		}
		
	}
    
	if ( version < 0x254 || ( version >= 0x254 && build < 41 ))
	{
		for(int32_t i=start_tile+tiles_used; i<max_tiles; ++i)
		{
			reset_tile(buf,i,tf4Bit);
		}
	}
	else
	{
		for(int32_t i=start_tile+tiles_used; i<max_tiles; ++i)
		{
			reset_tile(buf,i,tf4Bit);
		}
	}
	
	if((version < 0x192)|| ((version == 0x192)&&(build<186)))
	{
		if(get_qr(qr_BSZELDA))   //
		{
			byte tempbyte;
			int32_t floattile = sprite_data_buf.get(iwSwim).tile;
			
			for(int32_t i=0; i<tilesize(tf4Bit); i++)  //BSZelda tiles are out of order //does this include swim tiles?
			{
				tempbyte=buf[23].data[i];
				buf[23].data[i]=buf[24].data[i];
				buf[24].data[i]=buf[25].data[i];
				buf[25].data[i]=buf[26].data[i];
				buf[26].data[i]=tempbyte;
			}
			//swim tiles are out of order, too, but nobody cared? -Z 
			for(int32_t i=0; i<tilesize(tf4Bit); i++)
			{
				tempbyte=buf[floattile+11].data[i];
				buf[floattile+11].data[i]=buf[floattile+12].data[i];
				buf[floattile+12].data[i]=tempbyte;
			}
		}
	}
	
	if((version < 0x211)||((version == 0x211)&&(build<7)))   //Goriya tiles are out of order
	{
		if(!get_qr(qr_NEWENEMYTILES))
		{
			byte tempbyte;
			
			for(int32_t i=0; i<tilesize(tf4Bit); i++)
			{
				tempbyte=buf[130].data[i];
				buf[130].data[i]=buf[132].data[i];
				buf[132].data[i]=tempbyte;
				
				tempbyte=buf[131].data[i];
				buf[131].data[i]=buf[133].data[i];
				buf[133].data[i]=tempbyte;
			}
		}
	}
	
	al_trace("Registering blank tiles\n");
	register_blank_tiles();
    
    //memset(temp_tile, 0, tilesize(tf32Bit));
    delete[] temp_tile;
    temp_tile=NULL;
    return 0;
}
