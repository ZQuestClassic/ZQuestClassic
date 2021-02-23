//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  ending.cc
//
//  Ending code for Zelda Classic.
//
//--------------------------------------------------------

void inc_quest();
void noproc() {}

void put_triforce()
{
  putitem(framebuf,120,113,iTriforce);
  putitem(framebuf,136,113,iTriforce);
}

void putendmsg(char *s,int x,int y,int speed,void(proc)())
{
  text_mode(0);
  int i=0;
  int c=strlen(s)*speed;

  for(int f=0; f<c && !Quit; f++)
  {
    if((f%speed)==0)
    {
      if(s[i]!=' ')
        sfx(WAV_MSG);
      textprintf(framebuf,zfont,x+(i<<3),y,WHITE,"%c",s[i]);
      i++;
    }
    proc();
    advanceframe();
  }
}

void brick(int x,int y)
{
  blit(scrollbuf,scrollbuf,256,0,x,y,8,8);
}

void endingpal()
{
  byte pal[16*3] = {
     0, 0, 0,  // clear
    63,63,63,  // white
    31,31,31,  // gray
     0, 0, 0,  // black
    63,14, 0,  // red
    26,34,63,  // blue
    22,54,21   // green
    };
  byte *hold = colordata;
  colordata = pal;
  loadpalset(csBOSS,0);
  colordata = hold;
  refreshpal = true;
}


void ending()
{
  const int white = WHITE;
  const int red   = CSET(csBOSS)+4;
  const int blue  = CSET(csBOSS)+5;
  const int green = CSET(csBOSS)+6;

  stop_midi();
  kill_sfx();
  sfx(WAV_ZELDA);
  Quit=0;

  game.cheat |= (cheat>1)?1:0;

  int x=0,c=0;
  for(int f=0; f<380; f++)
  {
    if(f>=300 && f<380)
    {
      x++;
      switch(++c)
      {
      case 5: c=0;
      case 0:
      case 2:
      case 3: x++; break;
      }
      rectfill(scrollbuf,0,0,x-1,175,0);
      rectfill(scrollbuf,256-x,0,255,175,0);
    }
    guys.animate();
    blit(scrollbuf,framebuf,0,0,0,56,256,168);
    guys.draw(framebuf,true);
    Link.draw(framebuf);
    do_layer2(0, tmpscr, 0, 0);
    do_layer2(1, tmpscr, 0, 0);
    do_layer2(2, tmpscr, 0, 0);
    do_layer2(3, tmpscr, 0, 0);
    do_layer2(-1, tmpscr, 0, 0);
    for(int i=0; i<guys.Count(); i++)
    {
      if(isflier(guys.spr(i)->id)) {
          guys.spr(i)->draw(framebuf);
      }
    }
    do_layer2(4, tmpscr, 0, 0);
    do_layer2(5, tmpscr, 0, 0);
    advanceframe();
    if(Quit) return;
  }

  rectfill(framebuf,0,149,255,223,0);

  char tmpmsg[6][25];

  for (int x=0; x<3; x++) {
    sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
//    sprintf(tmpmsg[x], "%.24s", MsgStrings[QMisc.endstring].s+(24*x));
//    strncpy(tmpmsg[x], MsgStrings[QMisc.endstring].s+(24*x), 24);
  }
  for (int x=0; x<3; x++) {
    sprintf(tmpmsg[x+3], "%.24s", MsgStrings[QMisc.endstring+1].s+(24*x));
//    strncpy(tmpmsg[x+3], MsgStrings[QMisc.endstring+1].s+(24*x), 24);
  }

  if (QMisc.endstring==0) {
    putendmsg("THANKS LINK,YOU'RE",32,96,6,noproc);
    putendmsg("THE HERO OF HYRULE.",32,112,6,noproc);
  } else {
    putendmsg(tmpmsg[0],32,80,6,noproc);
    putendmsg(tmpmsg[1],32,96,6,noproc);
    putendmsg(tmpmsg[2],32,112,6,noproc);
  }
  rectfill(framebuf,120,129,152,150,0);
  blit(framebuf,tmp_bmp,120,113,0,0,32,32);

  overtile16(framebuf,36,120,129,6,0);
  overtile16(framebuf,BSZ?32:29,136,129,6,0);

  for(int f=0; f<256; f++)
  {
    if(f==64)
    {
      blit(framebuf,scrollbuf,0,0,0,0,256,56);
      for(int y=0; y<224; y++)
        for(int x=0; x<256; x++)
          if(!(framebuf->line[y][x]&15))
            framebuf->line[y][x]=16;
    }
    if(f>=64 && f<192)
    {
      static byte flash[4] = {0x12,0x16,0x2A,0x0F};
      RAMpal[16] = NESpal(flash[f&3]);
      refreshpal=true;
    }
    if(f==192)
    {
      blit(scrollbuf,framebuf,0,0,0,0,256,56);
      jukebox(MUSIC_ENDING);
      for(int y=0; y<224; y++)
        for(int x=0; x<256; x++)
          if(framebuf->line[y][x]==16)
            framebuf->line[y][x]=0;
    }
    put_triforce();
    advanceframe();
    if(Quit) return;
  }

  if (QMisc.endstring==0) {
    putendmsg("FINALLY,",96,160,8,put_triforce);
    putendmsg("PEACE RETURNS TO HYRULE.",32,176,8,put_triforce);
    putendmsg("THIS ENDS THE STORY.",48,200,8,put_triforce);
  } else {
    putendmsg(tmpmsg[3],32,160,6,noproc);
    putendmsg(tmpmsg[4],32,176,6,noproc);
    putendmsg(tmpmsg[5],32,200,6,noproc);
  }

  for(int f=0; f<128+32; f++)
  {
    if(f<128)
      put_triforce();
    if(f==128)
      blit(tmp_bmp,framebuf,0,0,120,113,32,32);
    advanceframe();
    if(Quit) return;
  }

  clear(scrollbuf);
  blit(framebuf,scrollbuf,0,0,0,0,256,224);
  endingpal();
  // draw the brick
  puttile16(scrollbuf,3,256,0,csBOSS,0);

  int len=600*2;
  if(game.quest>1)
    len=720*2;

  for(int f=0; f<len; f++)
  {
    if(!(f&15))
    {
      text_mode(0);
      int y=(f>>1)+224;
      if(y>240 && y<584)
      {
        brick(24,224);
        brick(224,224);
      }
      if(y==240 || y==584)
        for(int x=24; x<=224; x+=8)
          brick(x,224);
      switch(y)
      {
      case 240: textout(scrollbuf,zfont," STAFF ",104,224,white); break;
      case 272: textout(scrollbuf,zfont,"EXECUTIVE",40,224,blue); break;
      case 280: textout(scrollbuf,zfont,"PRODUCER... H.YAMAUCHI",40,224,blue); break;
      case 320: textout(scrollbuf,zfont,"PRODUCER.... S.MIYAHON",40,224,green); break;
      case 360: textout(scrollbuf,zfont,"DIRECTOR.... S.MIYAHON",40,224,red); break;
      case 384: textout(scrollbuf,zfont,"        ...... TEN TEN",40,224,red); break;
      case 424: textout(scrollbuf,zfont,"DESIGNER...... TEN TEN",40,224,blue); break;
      case 464: textout(scrollbuf,zfont,"PROGRAMMER.. T.NAKAZOO",40,224,green); break;
      case 488: textout(scrollbuf,zfont,"          ..... YACHAN",40,224,green); break;
      case 512: textout(scrollbuf,zfont,"          ... MARUMARU",40,224,green); break;
      case 552: textout(scrollbuf,zfont,"SOUND",40,224,red); break;
      case 560: textout(scrollbuf,zfont,"COMPOSER...... KONCHAN",40,224,red); break;
      }

      switch(game.quest)
      {
      case 1:
        switch(y)
        {
        case 656: textout(scrollbuf,zfont,"ANOTHER QUEST WILL START",32,224,white); break;
        case 672: textout(scrollbuf,zfont,"FROM HERE.",88,224,white); break;
        case 696: textout(scrollbuf,zfont,"PRESS THE START BUTTON.",40,224,white); break;
        case 760: textprintf(scrollbuf,zfont,72,224,white,"%c1986 NINTENDO",0xA9); break;
        }
        break;

      case 2:
        switch(y)
        {
        case 768: textout(scrollbuf,zfont,"YOU ARE GREAT.",72,224,white); break;
        case 792: textprintf(scrollbuf,zfont,72,224,red,"%-8s -%3d",game.name,game.deaths); break;
        case 816: textout(scrollbuf,zfont,"YOU HAVE AN AMAZING",48,224,white); break;
        case 832: textout(scrollbuf,zfont,"WISDOM AND POWER.",64,224,white); break;
/*
        case 880: textout(scrollbuf,zfont,"END OF",104,224,white); break;
        case 896: textout(scrollbuf,zfont,"\"THE LEGEND OF ZELDA 1\"",32,224,white); break;
*/
        case 912: textprintf(scrollbuf,zfont,72,224,blue,"%c1986 NINTENDO",0xA9); break;
        }
        break;

      case 3:
        switch(y)
        {
        case 768: textout(scrollbuf,zfont,"YOU ARE GREAT.",72,224,white); break;
        case 792: textprintf(scrollbuf,zfont,72,224,red,"%-8s -%3d",game.name,game.deaths); break;
        case 816: textout(scrollbuf,zfont,"YOU HAVE AN AMAZING",48,224,white); break;
        case 832: textout(scrollbuf,zfont,"WISDOM AND POWER.",64,224,white); break;
        case 880: textout(scrollbuf,zfont,"END OF",104,224,white); break;
        case 896: textout(scrollbuf,zfont,"\"THE LEGEND OF ZELDA 1\"",32,224,white); break;
        case 912: textprintf(scrollbuf,zfont,72,224,blue,"%c1986 NINTENDO",0xA9); break;
        }
        break;

      default:
        switch(y)
        {
        case 768: textout_centre(scrollbuf,zfont,"Congratulations!",128,224,white); break;
        case 784: textprintf(scrollbuf,zfont,72,224,red,"%-8s -%3d",game.name,game.deaths); break;
        case 800: if(game.timevalid && !game.cheat)
                    textout_centre(scrollbuf,zfont,time_str_med(game.time),128,224,blue); break;
        case 816: textout_centre(scrollbuf,zfont,"You beat a",128,224,white); break;
        case 832: textout_centre(scrollbuf,zfont,"custom quest.",128,224,white); break;
        case 880: textout_centre(scrollbuf,zfont,"ZELDA CLASSIC",128,224,white); break;
        case 896: textout_centre(scrollbuf,zfont,"1999-2000",128,224,white); break;
        case 912: textout_centre(scrollbuf,zfont,"Armageddon Games",128,224,blue); break;
        }
        break;
      }
    }

    if(f==200) // after subscreen has scrolled away
    {
      init_NES_mode();
      loadpalset(9,pSprite(spPILE));
      endingpal();
    }

    if(f&1)
      blit(scrollbuf,scrollbuf,0,1,0,0,256,232);
    blit(scrollbuf,framebuf,0,0,0,0,256,224);
    advanceframe();
    if(Quit) return;
    rSbtn();
  }

  do {
    if(frame&1)
      overtile16(framebuf,176,120,129,9,0);
    overtile16(framebuf,175,120,129,((frame&8)>>3)+7,0);
    if(!(frame&1))
      overtile16(framebuf,176,120,129,9,0);

    advanceframe();
    if(Quit) return;
  } while(!rSbtn());

  if(game.quest>0 && game.quest<255)
    inc_quest();

  load_quest(&game);
  load_game_icon(&game);
  saves[currgame] = game;
  save_savedgames();
}


void inc_quest()
{
  char name[9];
  strcpy(name,game.name);
  int quest = min(game.quest+1,QUEST_COUNT-1);
  int deaths = game.deaths;

  int *di = (int*)(&game);
  for(unsigned i=0; i<sizeof(gamedata)/sizeof(int); i++)
    *(di++) = 0;

  strcpy(game.name,name);
  game.quest = quest;
  game.deaths = deaths;
  game.maxlife = game.life = 3*HP_PER_HEART;
  game.maxbombs = 8;
  game.hasplayed = true;
}



