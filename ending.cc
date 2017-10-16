
void inc_quest();
void noproc() {}

void put_triforce()
{
  overtile16(framebuf,72,120,113,((frame&8)>>3)+7,0);
  overtile16(framebuf,72,136,113,((frame&8)>>3)+7,0);
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
      textprintf(framebuf,zfont,x+(i<<3),y,CSET(0)+1,"%c",s[i]);
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

void ending()
{
  stop_midi();
  kill_sfx();
  sfx(WAV_ZELDA);
  Quit=0;

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
    link.draw(framebuf);
    advanceframe();
    if(Quit) return;
  }

  rectfill(framebuf,96,150,160,200,0);

  putendmsg("THANKS LINK,YOU'RE",32,96,6,noproc);
  putendmsg("THE HERO OF HYRULE.",32,112,6,noproc);

  puttile16(framebuf,36,120,129,6,0);
  puttile16(framebuf,29,136,129,6,0);

  for(int f=0; f<256; f++)
  {
    if(f==64)
    {
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

  putendmsg("FINALLY,",96,160,8,put_triforce);
  putendmsg("PEACE RETURNS TO HYRULE.",32,176,8,put_triforce);
  putendmsg("THIS ENDS THE STORY.",48,200,8,put_triforce);

  for(int f=0; f<128+32; f++)
  {
    if(f<128)
      put_triforce();
    if(f==128)
      rectfill(framebuf,120,119,160,145,0);
    advanceframe();
    if(Quit) return;
  }

  clear(scrollbuf);
  blit(framebuf,scrollbuf,0,0,0,0,256,224);
  for(int i=0; i<TILEBUF_SIZE/4; i++)
    ((long*)tilebuf)[i]=((long*)data[TIL_NES].dat)[i];
  puttile16(scrollbuf,462,256,0,0,0);
  RAMpal[CSET(2)+1] = NESpal(0x22);
  RAMpal[CSET(3)+1] = NESpal(0x2A);
  loadpalset(9,pSprite(spPILE));
  refreshpal=true;

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
      case 240: textout(scrollbuf,zfont," STAFF ",104,224,CSET(0)+1); break;
      case 272: textout(scrollbuf,zfont,"EXECUTIVE",40,224,CSET(2)+1); break;
      case 280: textout(scrollbuf,zfont,"PRODUCER... H.YAMAUCHI",40,224,CSET(2)+1); break;
      case 320: textout(scrollbuf,zfont,"PRODUCER.... S.MIYAHON",40,224,CSET(3)+1); break;
      case 360: textout(scrollbuf,zfont,"DIRECTOR.... S.MIYAHON",40,224,CSET(1)+1); break;
      case 384: textout(scrollbuf,zfont,"        ...... TEN TEN",40,224,CSET(1)+1); break;
      case 424: textout(scrollbuf,zfont,"DESIGNER...... TEN TEN",40,224,CSET(2)+1); break;
      case 464: textout(scrollbuf,zfont,"PROGRAMMER.. T.NAKAZOO",40,224,CSET(3)+1); break;
      case 488: textout(scrollbuf,zfont,"          ..... YACHAN",40,224,CSET(3)+1); break;
      case 512: textout(scrollbuf,zfont,"          ... MARUMARU",40,224,CSET(3)+1); break;
      case 552: textout(scrollbuf,zfont,"SOUND",40,224,CSET(1)+1); break;
      case 560: textout(scrollbuf,zfont,"COMPOSER...... KONCHAN",40,224,CSET(1)+1); break;
      }

      if(game.quest==1)
      {
        switch(y)
        {
        case 656: textout(scrollbuf,zfont,"ANOTHER QUEST WILL START",32,224,CSET(0)+1); break;
        case 672: textout(scrollbuf,zfont,"FROM HERE.",88,224,CSET(0)+1); break;
        case 696: textout(scrollbuf,zfont,"PRESS THE START BUTTON.",40,224,CSET(0)+1); break;
        case 760: textprintf(scrollbuf,zfont,72,224,CSET(0)+1,"%c1986 NINTENDO",0xA9); break;
        }
      }
      else
      {
        switch(y)
        {
        case 768: textout(scrollbuf,zfont,"YOU ARE GREAT.",72,224,CSET(0)+1); break;
        case 792: textprintf(scrollbuf,zfont,72,224,CSET(1)+1,"%-8s -%3d",game.name,game.deaths); break;
        case 816: textout(scrollbuf,zfont,"YOU HAVE AN AMAZING",48,224,CSET(0)+1); break;
        case 832: textout(scrollbuf,zfont,"WISDOM AND POWER.",64,224,CSET(0)+1); break;
        case 880: textout(scrollbuf,zfont,"END OF",104,224,CSET(0)+1); break;
        case 896: textout(scrollbuf,zfont,"\"THE LEGEND OF ZELDA 1\"",32,224,CSET(0)+1); break;
        case 912: textprintf(scrollbuf,zfont,72,224,CSET(2)+1,"%c1986 NINTENDO",0xA9); break;
        }
      }
    }
    if(f&1)
      blit(scrollbuf,scrollbuf,0,1,0,0,256,232);
    blit(scrollbuf,framebuf,0,0,0,0,256,224);
    advanceframe();
    if(Quit) return;
    rSbtn();
  }

  int h=rand()&1;
  do {
    puttile16(scrollbuf,175,256,0,((frame&8)>>3)+7,0);
    overtile16(scrollbuf,176,256,0,9,0);
    puttile16(scrollbuf,176,256,16,9,0);
    overtile16(scrollbuf,175,256,16,((frame&8)>>3)+7,0);
    blit(scrollbuf,framebuf,256,h?16:0,120,129,8,16);
    blit(scrollbuf,framebuf,264,h?0:16,128,129,8,16);
    advanceframe();
    if(Quit) return;
  } while(!rSbtn());

  inc_quest();
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
  game.maxlife = game.life = 3*8;
  game.maxbombs = 8;
  game.hasplayed = true;

  saves[currgame] = game;
}



