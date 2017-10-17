//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_files.cc
//
//  File support for ZQuest.
//
//--------------------------------------------------------

#include "zq_files.h"
#include "zdefs.h"
#include "zq_misc.h"
#include "zquest.h"
#include "qst.h"
#include "zsys.h"
#include "zq_class.h"
#include "tiles.h"

#include <string.h>
#include <stdio.h>

char qtbuf[31];

void reset_qt(int index)
{
  bound(index,0,MAXQTS-1);
  char *s=QuestTemplates[index].name;
  for(int i=0; i<31; i++)
    *(s++)=0;
  s=QuestTemplates[index].path;
  for(int i=0; i<280; i++)
    *(s++)=0;
}

void init_qts()
{
  for(int i=0; i<MAXQTS; i++)
    reset_qt(i);
  strcpy(QuestTemplates[0].name,"New Default (can't change)");
  qt_count=1;
}

static DIALOG editqt_dlg[] =
{
  /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
  { jwin_win_proc,     32,   68,   257,  102,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Quest Template Editor" },
  { d_dummy_proc,      160,  72,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_button_proc,     50,   140,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Load" },
  { jwin_button_proc,     130,  140,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     210,  140,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_text_proc,       44,   103,   40,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Name:" },
  { jwin_edit_proc,       77,   99,   200,    16,      vc(12),  vc(1),  0,       0,          30,             0,       NULL },
  { jwin_text_proc,       44,   118,  112,  8,    vc(15),  vc(1),  0,       0,          0,             0,       (void *) "Template File:" },
  { jwin_text_proc,       44,   128,  233,  8,    vc(13),  vc(1),  0,       D_DISABLED,          0,             0,       NULL },
  { d_keyboard_proc,   0,    0,    0,    0,    0,    0,    0,       0,       KEY_F1,   0,       (void *) onHelp },
  { NULL }
};

void edit_qt()                                              //this is used to set the quest template for the current quest
{
  char tpath[280];
  char tpath2[280];
  strcpy(temppath, header.templatepath);
  if (temppath[0]==0)
  {
    getcwd(temppath,280);
    fix_filename_case(temppath);
    fix_filename_slashes(temppath);
    put_backslash(temppath);
  }
  bool gotname;
  gotname=getname("Load Quest Template (.zqt)","zqt",temppath,true);
  if (gotname)
  {
    strcpy(tpath, temppath);
    chop_path(tpath);
    getcwd(tpath2,280);
    fix_filename_case(tpath2);
    fix_filename_slashes(tpath2);
    put_backslash(tpath2);
    if (!strcmp(tpath, tpath2))
    {
      strcpy(header.templatepath, get_filename(temppath));
    }
    else
    {
      strcpy(header.templatepath, temppath);
    }
    if (!valid_zqt(temppath))
    {
      jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
      memset(header.templatepath, 0, 280);
    }
  }
}

void edit_qt(int index)
{
  int ret;
  quest_template tqt;
  char tpath[280];
  char tpath2[280];
  tqt=QuestTemplates[index];
  editqt_dlg[0].dp2=lfont;
  do
  {
    editqt_dlg[6].dp=QuestTemplates[index].name;
    editqt_dlg[8].dp=QuestTemplates[index].path;
    strcpy(temppath, QuestTemplates[index].path);
    bool gotname;
    ret=popup_dialog(editqt_dlg,6);
    switch (ret)
    {
      case 2:
        gotname=getname("Load Quest Template (.zqt)","zqt",temppath,true);
        if (gotname)
        {
          strcpy(tpath, temppath);
          chop_path(tpath);
          getcwd(tpath2,280);
          fix_filename_case(tpath2);
          fix_filename_slashes(tpath2);
          put_backslash(tpath2);
          if (!strcmp(tpath, tpath2))
          {
            strcpy(QuestTemplates[index].path, get_filename(temppath));
          }
          else
          {
            strcpy(QuestTemplates[index].path, temppath);
          }
        }
        break;
      case 3:

        if (!valid_zqt(temppath))
        {
          ret=2;
          jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
          break;
        }

        if(index==qt_count)
        {
          ++qt_count;
        }
        break;
      case 4:
        QuestTemplates[index]=tqt;
        break;
    }
  } while (ret==2);
}

char *qtlist(int index, int *list_size)
{
  if(index>=0)
  {
    if(index>=qt_count)
      index=qt_count-1;
    return QuestTemplates[index].name;
  }
  *list_size=qt_count;
  return NULL;
}

static DIALOG qtlist_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc,     8,   40,   303,  149,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Pick Quest Template" },
  { d_dummy_proc,      160,  47,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_list_proc,    13,   66,   271,  87,   jwin_pal[jcBOXFG],  jwin_pal[jcLIGHT],  0,       D_EXIT,     0,             0,       (void *) qtlist },
  { jwin_button_proc,     286,   60+24,  21,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "\x88" },
  { jwin_button_proc,     286,   137-24,  21,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "\x89" },
  { jwin_button_proc,     38,   160,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "Edit" },
  { jwin_button_proc,     118,  160,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     198,  160,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_DEL,       0,       (void *) qtlist_del },
  { NULL }
};

int qtlist_del()
{
  if(qtlist_dlg[2].d1>0 && qtlist_dlg[2].d1<qt_count-1)
    return D_CLOSE;
  return D_O_K;
}

int ListQTs(bool edit)
{
  qtlist_dlg[0].dp2=lfont;
  int index=0;
  quest_template BackupQTs[MAXQTS];
  for (int x=0; x<MAXQTS; x++)
  {
    BackupQTs[x]=QuestTemplates[x];
  }
  int backup_qt_count=qt_count;
  while(index>-1)
  {
    bool hasroom=false;
    if(qt_count<MAXQTS)
    {
      hasroom=true;
      if (edit)
      {
        strcpy(QuestTemplates[qt_count++].name,"<new template>");
      }
    }

    qtlist_dlg[2].x=edit?13:23;
    qtlist_dlg[3].proc=edit?jwin_button_proc:d_dummy_proc;
    qtlist_dlg[4].proc=edit?jwin_button_proc:d_dummy_proc;
    qtlist_dlg[5].proc=edit?jwin_button_proc:d_dummy_proc;
    qtlist_dlg[6].x=edit?118:88;
    qtlist_dlg[7].x=edit?198:168;
    qtlist_dlg[8].proc=edit?d_keyboard_proc:d_dummy_proc;


    int ret=popup_dialog(qtlist_dlg,2);
    index=qtlist_dlg[2].d1;

    int doedit=false;

    switch(ret)
    {
      case 2:
        if (edit)
        {
          doedit=true;
        }
        else
        {
          if (index>0&&!valid_zqt(QuestTemplates[index].path))
          {
            jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
          }
          else
          {
            strcpy(header.templatepath, QuestTemplates[index].path);
            index=-1;
          }
        }
        break;
      case 5:
        doedit=true;
        break;
      case 3:
        if (index>1&&index<qt_count-1)
        {
          swap(QuestTemplates[index],QuestTemplates[index-1]);
          --qtlist_dlg[2].d1;
          index=qtlist_dlg[2].d1;
        }
        break;
      case 4:
        if (index>0&&index<qt_count-2)
        {
          swap(QuestTemplates[index],QuestTemplates[index+1]);
          ++qtlist_dlg[2].d1;
          index=qtlist_dlg[2].d1;
        }
        break;
      case 6:
        if (index>0&&!valid_zqt(QuestTemplates[index].path))
        {
          jwin_alert("ZQuest","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
        }
        else
        {
          if (!edit)
          {
            strcpy(header.templatepath, QuestTemplates[index].path);
          }
          index=-1;
        }
        break;
      case 0:
      case 7:
        if (edit)
        {
          qt_count=backup_qt_count+1;
          for (int x=0; x<MAXQTS; x++)
          {
            QuestTemplates[x]=BackupQTs[x];
          }
        }
        index=-2;
        break;
      case 8:
        char buf[25];
        strncpy(buf,QuestTemplates[index].name,30);
        if(jwin_alert("Confirm Deletion", "Delete this quest template?",buf,"(the file will still exist)","Yes","No",'y',27,lfont)==1)
        {
          for(int i=index; i<MAXQTS-1; i++)
            QuestTemplates[i]=QuestTemplates[i+1];
          reset_qt(MAXQTS-1);
          --qt_count;
        }
        break;
    }
    if(edit&&hasroom)
    {
      //      strcpy(QuestTemplates[--qt_count].name,"              ");
      reset_qt(--qt_count);
      sprintf(QuestTemplates[qt_count].name,"Untitled");
    }

    if(index>0 && doedit)
    {
      edit_qt(index);
    }
  }
  return index;
}

int onQuestTemplates()
{
  ListQTs(true);
  return D_O_K;
}

int NewQuestFile()
{
  memset(filepath,0,255);
  box_start(1, "Initializing Quest", lfont, pfont, false);
  box_out("Please wait."); box_eol();
  box_out("This may take a few moments."); box_eol();

  init_quest();
  saved=true;
  box_end(false);
  refresh(rALL);
  return D_O_K;
}

int onNew()
{
  if(checksave()==0)
    return D_O_K;
  if (ListQTs(false)==-2)
  {
    return D_O_K;
  }
  NewQuestFile();
  return D_O_K;
}

int onSave()
{
  restore_mouse();
  if (disable_saving)
  {
    jwin_alert("ZQuest","Saving is","disabled in this version.",NULL,"O&K",NULL,'k',0,lfont);
    return D_O_K;
  }
  if (OverwriteProtection)
  {
    jwin_alert("ZQuest","Overwriting quests is","disabled in this version.",NULL,"O&K",NULL,'k',0,lfont);
    return D_O_K;
  }
  if(filepath[0]==0)
    return onSaveAs();

  int ret = save_quest(filepath);
  char buf[80],name[13];
  extract_name(filepath,name,FILENAME8_3);
  if(!ret)
  {
    sprintf(buf,"Saved %s",name);
    jwin_alert("ZQuest",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    saved=true;
  }
  else
  {
    sprintf(buf,"Error saving %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }

  if (AutoBackup)
  {
    char backupname[256];
    replace_extension(backupname, filepath, "qsb", 255);

    if(!save_unencoded_quest(backupname,false))
    {
      sprintf(buf,"Backup completed.");
      jwin_alert("ZQuest",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    else
    {

      sprintf(buf,"Error making backup.");
      jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
  }

  return D_O_K;
}

int onSaveAs()
{
  if (disable_saving)
  {
    jwin_alert("ZQuest","Saving is","disabled in this version.",NULL,"O&K",NULL,'k',0,lfont);
    return D_O_K;
  }
  if(!getname("Save Quest As (.qst)","qst",filepath,true))
    return D_O_K;
  if(exists(temppath))
  {
    if (OverwriteProtection)
    {
      jwin_alert("ZQuest","Overwriting quests is","disabled in this version.",NULL,"O&K",NULL,'k',0,lfont);
      return D_O_K;
    }

    if(jwin_alert("Confirm Overwrite",temppath,"already exists.","Write over existing file?","&Yes","&No",'y','n',lfont)==2)
    {
      return D_O_K;
    }
  }

  int ret = save_quest(temppath);
  char buf[1024],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(!ret)
  {
    strcpy(filepath,temppath);
    sprintf(buf,"ZQuest - [%s]", get_filename(filepath));
    set_window_title(buf);
    sprintf(buf,"Saved %s",name);
    jwin_alert("ZQuest",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    saved=true;
  }
  else
  {
    sprintf(buf,"Error saving %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  refresh(rMENU);
  return D_O_K;
}

int onOpen()
{
  restore_mouse();
  if(checksave()==0)
    return D_O_K;
  if(!getname("Load Quest File (.qst)","qst",filepath,true))
    return D_O_K;

  int ret = load_quest(temppath, true, true);
  if(!ret)
  {
    strcpy(filepath,temppath);
    saved = true;
  }
  else
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,qst_error[ret],NULL,"O&K",NULL,'k',0,lfont);
    filepath[0]=0;
  }
  setup_combo_animations();
  refresh(rALL);
  return D_O_K;
}

int onRevert()
{
  if(jwin_alert("Confirm Revert","Are you sure you want to lose","all changes since last save?",NULL,"Yes","No",'y','n',lfont)==2)
  {
    return D_O_K;
  }

  if (filepath[0]!=0)
  {
    int ret = load_quest(filepath, true, true);
    if(!ret)
    {
      saved = true;
    }
    else
    {
      char buf[80],name[13];
      extract_name(filepath,name,FILENAME8_3);
      sprintf(buf,"Unable to load %s",name);
      jwin_alert("Error",buf,qst_error[ret],NULL,"O&K",NULL,'k',0,lfont);
      filepath[0]=0;
    }
    setup_combo_animations();
    refresh(rALL);
  }
  else
  {
    NewQuestFile();
  }
  return D_O_K;

}

static DIALOG import_map_bias_dlg[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
  { jwin_win_proc, 80,   80-8,   160+1,  64+20+1,   vc(14),  vc(1),  0,       D_EXIT,          0,             0,       (void *) "Select Import Bias" },
  { d_dummy_proc,      160,  88-8,     0,       8,   vc(15),  vc(1),  0,       0,          0,             0,       NULL },
  { jwin_button_proc,     90,   120+10,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0,       (void *) "OK" },
  { jwin_button_proc,     170,  120+10,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0,       (void *) "Cancel" },
  { jwin_radio_proc,      130, 104-8,   96+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Total Copy" },
  { jwin_radio_proc,      130, 114-8,  120+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Prefer Original" },
  { jwin_radio_proc,      130, 124-8,  104+1,    8+1,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0,       (void *) "Prefer Import" },
  { NULL }
};

int get_import_map_bias()
{
  import_map_bias_dlg[0].dp2=lfont;
  for (int i=0; i<3; i++)
  {
    import_map_bias_dlg[i+4].flags=0;
  }
  import_map_bias_dlg[ImportMapBias+4].flags=D_SELECTED;
  if(popup_dialog(import_map_bias_dlg,2)==2)
  {
    for (int i=0; i<3; i++)
    {
      if (import_map_bias_dlg[i+4].flags&D_SELECTED)
      {
        ImportMapBias=i;
        break;
      }
    }
    return 0;
  }

  return -1;
}

int onImport_Map()
{
  if(Map.getCurrMap()>=Map.getMapCount())
    return D_O_K;
  if (get_import_map_bias()==-1)
  {
    return D_O_K;
  }
  if(!getname("Import Map (.map)","map",datapath,false))
    return D_O_K;
  saved=false;
  int ret=Map.load(temppath);
  if(ret)
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,loaderror[ret],NULL,"O&K",NULL,'k',0,lfont);
    if(ret>1)
      Map.clearmap(false);
  }
  refresh(rSCRMAP+rMAP+rMENU);
  return D_O_K;
}

int onExport_Map()
{
  if(Map.getCurrMap()>=Map.getMapCount())
    return D_O_K;
  if(!getname("Export Map (.map)","map",datapath,false))
    return D_O_K;
  int ret = Map.save(temppath);
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(!ret)
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_DMaps()
{
  if(!getname("Import DMaps (.dmp)","dmp",datapath,false))
    return D_O_K;
  saved=false;
  if(!load_dmaps(temppath,0))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  return D_O_K;
}

int onExport_DMaps()
{
  if(!getname("Export DMaps (.dmp)","dmp",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_dmaps(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_Pals()
{
  if(!getname("Import Palettes (.zpl)","zpl",datapath,false))
    return D_O_K;
  saved=false;
  if(!load_pals(temppath,0))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  return D_O_K;
}

int onExport_Pals()
{
  if(!getname("Export Palettes (.zpl)","zpl",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_pals(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_Msgs()
{
  if(!getname("Import String Table (.zqs)","zqs",datapath,false))
    return D_O_K;
  saved=false;
  if(!load_msgstrs(temppath,0))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  return D_O_K;
}

int onExport_Msgs()
{
  if(!getname("Export String Table (.zqs)","zqs",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_msgstrs(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_Combos()
{
  int ret=getnumber("Import Start Page",0);
  if (cancelgetnum)
  {
    return D_O_K;
  }
  bound(ret,0,COMBO_PAGES-1);

  if(!getname("Import Combo Table (.cmb)","cmb",datapath,false))
    return D_O_K;
  if(!load_combos(temppath, ret*COMBOS_PER_PAGE))
  {
    // if(!load_combos(temppath)) {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  else
    saved=false;
  refresh(rALL);
  return D_O_K;
}

int onExport_Combos()
{
  if(!getname("Export Combo Table (.cmb)","cmb",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_combos(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_Tiles()
{
  int ret=getnumber("Import Start Page",0);
  if (cancelgetnum)
  {
    return D_O_K;
  }
  bound(ret,0,TILE_PAGES-1);

  if(!getname("Import Tiles (.til)","til",datapath,false))
    return D_O_K;
  saved=false;
  // usetiles=true;
  if(!load_tiles(temppath, ret*TILES_PER_PAGE))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  refresh(rALL);
  return D_O_K;
}

int onExport_Tiles()
{
  if(!getname("Export Tiles (.til)","til",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_tiles(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_ZGP()
{
  if(!getname("Import Graphics Pack (.zgp)","zgp",datapath,false))
    return D_O_K;
  saved=false;
  // usetiles=true;
  if(!load_zgp(temppath))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  refresh(rALL);
  return D_O_K;
}

int onExport_ZGP()
{
  if(!getname("Export Graphics Pack (.zgp)","zgp",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(save_zgp(temppath))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_ZQT()
{
  if(!getname("Import Quest Template (.zqt)","zqt",datapath,false))
    return D_O_K;
  saved=false;
  // usetiles=true;
  if(load_quest(temppath,true, false))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  register_blank_tiles();
  loadlvlpal(Color);
  setup_combo_animations();
  Map.setCurrMap(Map.getCurrMap());                         // for bound checking
  refresh(rALL);
  refresh_pal();
  return D_O_K;
}

int onExport_ZQT()
{
  if(!getname("Export Quest Template (.zqt)","zqt",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(!save_unencoded_quest(temppath, true))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

int onImport_UnencodedQuest()
{
  if(!getname("Import Unencoded Quest (.qsu)","qsu",datapath,false))
    return D_O_K;
  saved=false;
  // usetiles=true;
  if(load_quest(temppath,false, false))
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Unable to load %s",name);
    jwin_alert("Error",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  register_blank_tiles();
  loadlvlpal(Color);
  setup_combo_animations();
  Map.setCurrMap(Map.getCurrMap());                         // for bound checking
  refresh(rALL);
  refresh_pal();
  return D_O_K;
}

int onExport_UnencodedQuest()
{
  if(!getname("Export Unencoded Quest (.qsu)","qsu",datapath,false))
    return D_O_K;
  char buf[80],buf2[80],name[13];
  extract_name(temppath,name,FILENAME8_3);
  if(!save_unencoded_quest(temppath, false))
  {
    sprintf(buf,"ZQuest");
    sprintf(buf2,"Saved %s",name);
  }
  else
  {
    sprintf(buf,"Error");
    sprintf(buf2,"Error saving %s",name);
  }
  jwin_alert(buf,buf2,NULL,NULL,"O&K",NULL,'k',0,lfont);
  return D_O_K;
}

void center_zq_files_dialogs()
{
  jwin_center_dialog(editqt_dlg);
  jwin_center_dialog(import_map_bias_dlg);
  jwin_center_dialog(qtlist_dlg);
}
