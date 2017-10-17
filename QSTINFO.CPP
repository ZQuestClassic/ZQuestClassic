#include "zc_alleg.h"
#include <stdio.h>
#include <string.h>
#include "qst.h"
#include "zdefs.h"
#include "zsys.h"

zquestheader hdr;

bool bad_version(int ver)
{
  if(ver < 0x170)
    return true;

  return false;
}

int readheader(PACKFILE *f, zquestheader *header, char *filename)
{
  int dummy;
  memset(header,0,sizeof(header));

  if(!pfread(header->id_str,sizeof(header->id_str),f))      // first read old header
  {
    return qe_invalid;
  }

  // check header
  if(strcmp(header->id_str,QH_NEWIDSTR))
  {
    if(strcmp(header->id_str,QH_IDSTR))
    {
      return qe_invalid;
    }
  }
  if(!strcmp(header->id_str,QH_IDSTR))                      //pre-1.93 version
  {
    byte padding;
    if(!p_getc(&padding,f))
    {
      return qe_invalid;
    }
    if(!p_igetw(&header->zelda_version,f))
    {
      return qe_invalid;
    }
    // check header
    if(header->zelda_version > ZELDA_VERSION)
    {
      return qe_version;
    }
    if(strcmp(header->id_str,QH_IDSTR))
    {
      return qe_invalid;
    }
    if(bad_version(header->zelda_version))
    {
      return qe_obsolete;
    }
    if(!p_igetw(&header->internal,f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->quest_number,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->rules,2,f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->map_count,f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->old_str_count,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->data_flags,sizeof(header->data_flags),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->rules2,2,f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->options,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->version,sizeof(header->version),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->title,sizeof(header->title),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->author,sizeof(header->author),f))
    {
      return qe_invalid;
    }
    if(!p_getc(&padding,f))
    {
      return qe_invalid;
    }
    if(!p_igetw(&header->pwdkey,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->password,sizeof(header->password),f))
    {
      return qe_invalid;
    }
    if(header->zelda_version < 0x177)                       // lacks new header stuff...
    {
      memset(header->minver,0,20);                          //   char minver[9], byte build
    }                                                       //   byte foo[10]
    else
    {
      if(!pfread(header->minver,sizeof(header->minver),f))
      {
        return qe_invalid;
      }
      if(!p_getc(&header->build,f))
      {
        return qe_invalid;
      }
      if(!p_getc(&header->use_keyfile,f))
      {
        return qe_invalid;
      }
      if(!pfread(header->foo,sizeof(header->foo),f))
      {
        return qe_invalid;
      }
    }                                                       // starting at minver
    if(header->zelda_version < 0x187)                       // lacks newer header stuff...
    {
      memset(header->rules3,0,16);                          //   word rules3..rules10
    }
    else
    {
      if(!pfread(header->rules3,16,f))                      // read new header additions
      {
        return qe_invalid;                                  // starting at rules3
      }
    }
    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<149)))
    {
      set_bit(header->rules3,qr3_BRKNSHLDTILES,(get_bit(header->rules3,qr3_BRKBLSHLDS)));
      set_bit(header->rules3,qr3_BRKBLSHLDS,1);
    }
    if(header->zelda_version < 0x192)                       //  lacks newer header stuff...
    {
      memset(header->midi_flags,0,32);                      //  byte midi_flags[32] (for expansion)
      memset(header->foo2,0,18);                            //  byte foo2[18] (for expansion)
    }
    else
    {
      if(!pfread(header->midi_flags,32,f))                  // read new header additions
      {
        return qe_invalid;                                  // starting at foo2
      }
      if(!pfread(header->foo2,18,f))                        // read new header additions
      {
        return qe_invalid;                                  // starting at foo2
      }
    }
    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<145)))
    {
      memset(header->templatepath,0,280);
    }
    else
    {
      if(!pfread(header->templatepath,280,f))               // read templatepath
      {
        return qe_invalid;
      }
    }
    if ((header->zelda_version < 0x192)||
      ((header->zelda_version == 0x192)&&(header->build<186)))
    {
      header->use_keyfile=0;
    }
  }
  else
  {
    //section id
    if(!p_mgetl(&dummy,f))
    {
      return qe_invalid;
    }

    //section version info
    if(!p_igetw(&dummy,f))
    {
      return qe_invalid;
    }
    if(!p_igetw(&dummy,f))
    {
      return qe_invalid;
    }

    //section size
    if(!p_igetl(&dummy,f))
    {
      return qe_invalid;
    }

    //finally...  section data
    if(!p_igetw(&header->zelda_version,f))
    {
      return qe_invalid;
    }

    //do some quick checking...
    if(header->zelda_version > ZELDA_VERSION)
    {
      return qe_version;
    }
    if(strcmp(header->id_str,QH_NEWIDSTR))
    {
      return qe_invalid;
    }
    if(bad_version(header->zelda_version))
    {
      return qe_obsolete;
    }

    if(!p_getc(&header->build,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->password,sizeof(header->password),f))
    {
      return qe_invalid;
    }
    if(!p_igetw(&header->pwdkey,f))
    {
      return qe_invalid;
    }
    if(!p_igetw(&header->internal,f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->quest_number,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->version,sizeof(header->version),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->minver,sizeof(header->minver),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->title,sizeof(header->title),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->author,sizeof(header->author),f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->use_keyfile,f))
    {
      return qe_invalid;
    }
    if(!pfread(header->data_flags,sizeof(header->data_flags),f))
    {
      return qe_invalid;
    }
    if(!pfread(header->templatepath,sizeof(header->templatepath),f))
    {
      return qe_invalid;
    }
    if(!p_getc(&header->map_count,f))
    {
      return qe_invalid;
    }

    //unneeded data
    /*
    if(!pfread(header->midi_flags,sizeof(header->midi_flags),f)){
      return qe_invalid;
    }
    if(!pfread(header->foo,sizeof(header->foo),f)){
      return qe_invalid;
    }
    if(!pfread(header->foo2,sizeof(header->foo2),f)){
      return qe_invalid;
    }
    if(!p_getc(&header->old_str_count,f)){
    return qe_invalid;
    }
    if(!p_getc(&header->options,f)){
    return qe_invalid;
    }
    */
  }
  return 0;
}

int getquestinfo(char *filename, zquestheader *header)
{
  char tmpbuf[L_tmpnam];
  char *tmpfilename = tmpnam(tmpbuf);

  // oldquest flag is set when an unencrypted qst file is suspected.
  bool oldquest = false;

  int ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_CURRENT);
  if(ret)
  {
    switch(ret)
    {
      case 1: return qe_notfound;
      case 2: return qe_internal;
      // be sure not to delete tmpfilename now...
    }
    if (ret==5)                                             //old encryption?
    {
      ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B105);
    }
    if (ret==5)                                             //old encryption?
    {
      ret = decode_file_007(filename, tmpfilename, ENC_STR, ENC_METHOD_192B104);
    }
    if (ret)
    {
      oldquest = true;
    }
  }

  PACKFILE *f = pack_fopen(oldquest ? filename : tmpfilename, F_READ_PACKED);
  if(!f)
  {
    if(!oldquest)
    {
      delete_file(tmpfilename);
    }
    return qe_invalid;
  }

  //header
  ret=readheader(f, header, oldquest ? filename : tmpfilename);
  checkstatus(ret);

  // check data
  pack_fclose(f);
  if(!oldquest)
    delete_file(tmpfilename);
  return qe_OK;

  invalid:
  pack_fclose(f);
  if(!oldquest)
    delete_file(tmpfilename);
  return qe_invalid;

}

int listqsts(const char *filename, int attrib, void *param)
{
  if (!stricmp(ustrlwr(get_extension(filename)),"qst"))
  {
    printf("%s\n",filename);
    packfile_password(datapwd);
    if (getquestinfo((char *)filename, &hdr)==qe_OK)
    {
      printf("Version: %X.%X build %d\n",hdr.zelda_version>>8,hdr.zelda_version&0xFF,hdr.build);
    }
    packfile_password(NULL);

  }
  return 0;
}

int listdirs(const char *filename, int attrib, void *param)
{
  char nfn[512];
  strcpy(nfn,filename);
  strcat(nfn, "/*");
  if (stricmp(get_filename(filename),".")&&
    stricmp(get_filename(filename),".."))
  {
    for_each_file_ex(nfn, FA_DIREC, 0, listdirs, NULL);
    for_each_file_ex(nfn, 0, FA_DIREC, listqsts, NULL);
  }
  return 0;
}

int main()
{
  allegro_init();
  resolve_password(datapwd);
  for_each_file_ex("*", FA_DIREC, 0, listdirs, NULL);
  return 0;
}
