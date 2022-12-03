//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  aglogo.cc
//
//  Armageddon games logo with fire field generator.
//  Adapted from code written by Midnight and included in
//  SPHINX C-- examples.
//  (Tweaked by Jeremy Craner.)
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "zeldadat.h"
#include "render.h"

extern DATAFILE* datafile;

extern int32_t joystick_index;
int32_t logovolume = 0;

extern FONT* dsphantompfont;

static void SetCols(RGB* pal)
{

	byte firepal[256*3];
	word DI,BX=0;
	
	firepal[0]=0;
	firepal[1]=0;
	firepal[2]=0;
	
	do
	{
		word AX = BX * 64 / 85;
		byte DL = AX;
		
		DI = BX+BX+BX;
		firepal[DI+3] = DL;
		firepal[DI+3+1] = 0;
		
		if(BX<75)
			firepal[DI+3+2] = BX<8 ? (BX<<1)+12 : 20;
		else firepal[DI+3+2] = BX-75;
		
		firepal[DI+3+85+85+85] = 63;
		firepal[DI+3+85+85+85+1] = DL;
		firepal[DI+3+85+85+85+2] = 0;
		
		firepal[DI+3+85+85+85+85+85+85] = 63;
		firepal[DI+3+85+85+85+85+85+85+1] = 63;
		firepal[DI+3+85+85+85+85+85+85+2] = DL;
		
		++BX;
	}
	while(BX < 85);
	
	firepal[255*3]=0;
	firepal[255*3+1]=0;
	firepal[255*3+2]=0;
	DI=0;
	
	for(int32_t i=0; i<256; i++)
	{
		pal[i].r = firepal[DI++];
		pal[i].g = firepal[DI++];
		pal[i].b = firepal[DI++];
	}
}

static void AddFire(BITMAP* firebuf,word N)
{
	while(N)
	{
		int32_t cx = (zc_oldrand()%10+1)<<1;
		int32_t x = zc_oldrand()%314;
		byte color = (zc_oldrand()&127) + 128;
		
		while(cx)
		{
			firebuf->line[200][x]=color;
			firebuf->line[202][x]=color;
			++x;
			--cx;
		}
		
		--N;
	}
}

static void CopyAvg(BITMAP* fire)
{
	for(int32_t y=4; y<204; y+=2)
	{
		for(int32_t x=2; x<336; x+=2)
		{
			byte* si=&(fire->line[y][x-2]);
			int32_t AX = (*si + *(si+2) + *(si+4) + fire->line[y+2][x]) >> 2;
			AX = (AX + fire->line[y-2][x]) >> 1;
			
			if(AX>128)
				AX-=2;
			else if(AX>12)
				AX-=4;
			else if(AX>8)
				AX-=2;
			else if(AX>0)
				--AX;
			
			AX=(AX<<8)+AX;
			((word*)fire->line[y-4])[x>>1]=AX;
			((word*)fire->line[y-3])[x>>1]=AX;
		}
	}
}

int32_t aglogo_new_nofire(BITMAP *frame, BITMAP *firebuf, int32_t resx, int32_t resy)
{
    // frame should be 320x200, firebuf at least 340x206
    PALETTE pal;
    SetCols(pal);
    PALETTE workpal;
    set_palette(black_palette);
    clear_bitmap(frame);
    clear_bitmap(firebuf);
    clear_bitmap(screen);
    
    //for(int32_t f=0; f<128; f++)
    //{
    //    AddFire(firebuf,(f>>3)+1);
    //    CopyAvg(firebuf);
    //}
    
    int32_t fadecnt=0;
    bool blackout=false;
    logovolume = zc_get_config("zeldadx","logo_volume",255);
    play_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat,logovolume,128,1000,true);
    blit((BITMAP*)datafile[RLE_AGTEXT].dat,frame,0,0,0,0,256,224);
    textout_ex(frame, dsphantompfont, "Celebrating Twenty Years", 79-32-1, 170-1, 3, -1);
    textout_ex(frame, dsphantompfont, "Celebrating Twenty Years", 79-32, 170, 200, -1);

    rti_screen.visible = true;
    
    do
    {
        //AddFire(firebuf,17);
        //CopyAvg(firebuf);

        //blit(firebuf,frame,8,0,0,0,320,198);
        //draw_rle_sprite(frame,(RLE_SPRITE*)data[RLE_AGTEXT].dat,24,90);
	//stretch_blit((BITMAP*)data[RLE_AGTEXT].dat,screen,0,0,255,223,0,0,SCREEN_W, SCREEN_H);//255, 223);
	
	//blit((BITMAP*)data[RLE_AGTEXT].dat,frame, 0,0,0,0,256,224);
	//    textout_ex(screen, dsphantompfont, "Celebrating Twenty Years", 79, 170, 2, -1);
	    
        vsync();
        
		stretch_blit(frame,screen, 0,0,255,223, 0,0,screen->w, screen->h);
            
        poll_joystick();
        
        if((keypressed()||joy[joystick_index].button[0].b||joy[joystick_index].button[1].b)&&fadecnt>=32)
            blackout=true;
            
        if(!blackout)
        {
            if(fadecnt<64) //I get some problems here when fade_interpolate takes invalid parameters.
            {
                if(!(++fadecnt < 0))
                    fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
                    
                set_palette_range(workpal,0,255,false);
            }
        }
        else
        {
            if(!(--fadecnt < 0))
                fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
                
            set_palette_range(workpal,0,255,false);
        }
    }
    while(fadecnt>0);

    rti_screen.visible = false;
    
    stop_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat);
    clear_keybuf();
    return 0;
}

COLOR_MAP aglogo_trans_table;

int32_t aglogo(BITMAP *frame, BITMAP *firebuf, int32_t resx, int32_t resy)
{
    // frame should be 320x200, firebuf at least 340x206
    PALETTE pal;
    SetCols(pal);
    PALETTE workpal;
    set_palette(black_palette);
    clear_bitmap(frame);
    clear_bitmap(firebuf);
    clear_bitmap(screen);
    
    for(int32_t f=0; f<128; f++)
    {
        AddFire(firebuf,(f>>3)+1);
        CopyAvg(firebuf);
    }
    
    int32_t fadecnt=0;
    bool blackout=false;
    logovolume = zc_get_config("zeldadx","logo_volume",255);
    play_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat,logovolume,128,1000,true);
    blit((BITMAP*)datafile[RLE_AGTEXT].dat,frame,0,0,0,0,256,224);
    textout_ex(frame, dsphantompfont, "Celebrating Twenty Years", 79-32-1, 170-1, 3, -1);
    textout_ex(frame, dsphantompfont, "Celebrating Twenty Years", 79-32, 170, 200, -1);
    
    BITMAP* interm = create_bitmap_ex(8, 256, 224);
    BITMAP* overla = create_bitmap_ex(8, 256, 224);
    create_trans_table(&aglogo_trans_table, pal, 128, 128, 128, NULL);
    clear_bitmap(interm);
    clear_bitmap(overla);
    blit((BITMAP*)datafile[RLE_AGTEXT].dat,overla, 0,0,0,0, 256,224);

    rti_screen.visible = true;
    do
    {
        AddFire(firebuf,17);
        CopyAvg(firebuf);

	stretch_blit(firebuf,interm, 0,0,255,223, -10,50+64+36,400, 120-36);
	; 
        color_map = &aglogo_trans_table;
	draw_trans_sprite(interm, overla, 0, 0);
	textout_ex(interm, dsphantompfont, "Celebrating Twenty Years", 79-32-1, 170-1, 3, -1);
	textout_ex(interm, dsphantompfont, "Celebrating Twenty Years", 79-32, 170, 200, -1);
	
	stretch_blit(interm,screen, 0,0,255,223, 0,0,screen->w, screen->h);
        vsync();
	// stretch_blit(frame,screen, 0,0,255,223, 0,0,SCREEN_W, SCREEN_H);
        
	//else
        //    blit(frame,screen, 0,0,(resx-320)>>1, (resy-198)>>1, 320,198);
	
	
        
        poll_joystick();
        
        if((keypressed()||joy[joystick_index].button[0].b||joy[joystick_index].button[1].b)&&fadecnt>=32)
            blackout=true;
            
        if(!blackout)
        {
            if(fadecnt<64) //I get some problems here when fade_interpolate takes invalid parameters.
            {
                if(!(++fadecnt < 0))
                    fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
                    
                set_palette_range(workpal,0,255,false);
            }
        }
        else
        {
            if(!(--fadecnt < 0))
                fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
                
            set_palette_range(workpal,0,255,false);
        }

        update_hw_screen();
    }
    while(fadecnt>0);

    rti_screen.visible = false;
    
    stop_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat);
    clear_keybuf();
    return 0;
}


int32_t aglogo_old(BITMAP *frame, BITMAP *firebuf, int32_t resx, int32_t resy)
{
	// frame should be 320x200, firebuf at least 340x206
	PALETTE pal;
	SetCols(pal);
	PALETTE workpal;
	set_palette(black_palette);
	clear_bitmap(frame);
	clear_bitmap(firebuf);
	clear_bitmap(screen);
	
	for(int32_t f=0; f<128; f++)
	{
		AddFire(firebuf,(f>>3)+1);
		CopyAvg(firebuf);
	}
	
	int32_t fadecnt=0;
	bool blackout=false;
	logovolume = zc_get_config("zeldadx","logo_volume",255);
	play_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat,logovolume,128,1000,true);
	
	do
	{
		AddFire(firebuf,17);
		CopyAvg(firebuf);
		blit(firebuf,frame,8,0,0,0,320,198);
		draw_rle_sprite(frame,(RLE_SPRITE*)datafile[RLE_AGTEXT].dat,24,90);
		textout_ex(frame, dsphantompfont, "Celebrating Twenty Years", 79, 170, 2, -1);
		
		vsync();
		
		blit(frame,screen, 0,0,(resx-320)>>1, (resy-198)>>1, 320,198);
		
		poll_joystick();
		
		if((keypressed()||joy[joystick_index].button[0].b||joy[joystick_index].button[1].b)&&fadecnt>=32)
			blackout=true;
		
		if(!blackout)
		{
			if(fadecnt<64) //I get some problems here when fade_interpolate takes invalid parameters.
			{
				if(!(++fadecnt < 0))
					fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
				
				set_palette_range(workpal,0,255,false);
			}
		}
		else
		{
			if(!(--fadecnt < 0))
				fade_interpolate(black_palette,pal,workpal,fadecnt,0,255);
			
			set_palette_range(workpal,0,255,false);
		}
	}
	while(fadecnt>0);
	
	stop_sample((SAMPLE*)datafile[WAV_00_AGFIRE].dat);
	clear_keybuf();
	return 0;
}

