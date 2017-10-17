if (<.97)
{
}

else
{
  if (<=1.92)
  {
    if (==1.92)
  }

  if(buf[0]!=0xFFFF)                                      // below v0.97
  {
    pack_fseek(f,-16);
    printf(conv_str);
    //    for(int i=0; i<MAXSAVES; i++)
    for(int i=0; i<old_max_saves; i++)
    {
      if(!pfread(saves+i,sizeof(oldgamedata)-128,f))
        goto reset;

      byte *p = (byte*)(saves+i);
      for(int j=sizeof(gamedata)-1; j>=256; j--)
        *(p+j) = *(p+j-128);
      for(int j=128; j<256; j++)
        *(p+j) = 0;
    }
    //    for(int i=0; i<MAXSAVES; i++)
    for(int i=0; i<old_max_saves; i++)
    {
      if(saves[i].quest!=1)                                 // delete the demo quest saves
      {
        //        for(int j=i; j<MAXSAVES-1; j++)
        for(int j=i; j<old_max_saves-1; j++)
          saves[j]=saves[j+1];
        //        int *di = (int*)(saves+MAXSAVES-1);
        int *di = (int*)(saves+old_max_saves-1);
        for(unsigned k=0; k<sizeof(oldgamedata)/sizeof(int); k++)
          *(di++) = 0;
      }
    }
  }
  else
  {
    if(buf[1] < 0x0174)                                     // below v1.74, only has 10 maps instead of 16
    {
      printf(conv_str);
      //      for(int i=0; i<MAXSAVES; i++)
      for(int i=0; i<old_max_saves; i++)
      {
        byte *di = (byte*)(saves+i);

        // header and bmaps[]
        if(!pfread(di, 256+32*64, f))
          goto reset;

        // maps[0..9]
        di += 256+32*64;
        if(!pfread(di, 10*128, f))
          goto reset;

        // maps[10..15]
        di += 10*128;
        for(int i=0; i<6*128; i++)
          *(di++) = 0;

        // guys[0..9]
        if(!pfread(di, 10*128, f))
          goto reset;

        // guys[10..15]
        di += 10*128;
        for(int i=0; i<6*128; i++)
          *(di++) = 0;

        // last chunk of data
        if(!pfread(di, 260, f))
          goto reset;
      }
    }
    else if (buf[1] < 0x0190)                               // below v1.90, only has 24 save states. ->255
    {
      int *di = (int*)saves;
      for(unsigned i=0; i<sizeof(gamedata)*MAXSAVES/sizeof(int); i++)
        *(di++) = 0;
      if(!pfread(saves,sizeof(gamedata)*old_max_saves,f))
        goto reset;
    } else
    {
      if(!pfread(saves,sizeof(gamedata)*MAXSAVES,f))
        goto reset;
    }

    if(buf[1] < 0x0102)                                     // below v1.02, trim file paths
    {
      char path[260];
      for(int i=0; i<MAXSAVES; i++)
      {
        strcpy(path,saves[i].qstpath);                      // trim dir off the quest file name
        strcpy(saves[i].qstpath,get_filename(path));
      }
    }

    if(buf[1] < 0x0176)                                     // below v1.76, has lvlitems[16] instead of 32
    {
      for(int i=0; i<MAXSAVES; i++)
      {
        memcpy(&(saves[i].lvlitems), ((byte*)(saves+i))+10, 16);
        memset(((byte*)(saves+i))+10, 0, 16);
      }
    }
  }
