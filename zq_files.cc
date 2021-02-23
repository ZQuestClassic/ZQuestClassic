//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_files.cc
//
//  File support for ZQuest.
//
//--------------------------------------------------------


int onNew()
{
 if(checksave()==0)
   return D_O_K;
 if(!getname("New Quest File (.qst)","qst",filepath,false))
   return D_O_K;
 init_quest();
 saved=false;
 strcpy(filepath,temppath);
 char buf[80],name[13];
 extract_name(filepath,name,FILENAME8_3);
 sprintf(buf,"Initialized %s",name);
 refresh(rALL);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 refresh(rMENU);
 return D_O_K;
}


int onSave()
{
 if(filepath[0]==0)
   return onSaveAs();
 int ret = save_quest(filepath);
 char buf[80],name[13];
 extract_name(filepath,name,FILENAME8_3);
 if(!ret) {
   sprintf(buf,"Saved %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   saved=true;
   }
 else {
   sprintf(buf,"Error saving %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 return D_O_K;
}

int onSaveAs()
{
 if(!getname("Save Quest As (.qst)","qst",filepath,true))
   return D_O_K;
 if(exists(temppath))
   if(alert(temppath,"already exists.","Write over existing file?",
            "&Yes","&No",'y','n')==2)
     return D_O_K;

 int ret = save_quest(temppath);
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(!ret) {
   strcpy(filepath,temppath);
   sprintf(buf,"Saved %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   saved=true;
   }
 else {
   sprintf(buf,"Error saving %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 refresh(rMENU);
 return D_O_K;
}


int onOpen()
{
 if(checksave()==0)
   return D_O_K;
 if(!getname("Load Quest File (.qst)","qst",filepath,true))
   return D_O_K;

 int ret = load_quest(temppath);
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
   alert(buf,qst_error[ret],NULL,"O&K",NULL,'k',0);
   filepath[0]=0;
 }
 setup_combo_animations();
 refresh(rALL);
 return D_O_K;
}

int onOpen2()
{
 if(checksave()==0)
   return D_O_K;
 if(!getname("Load Quest File (.qst)","qst",filepath,true))
   return D_O_K;

 int ret = load_quest2(temppath);
 if(!ret)
 {
   strcpy(filepath,temppath);
   saved = true;
 }
 else
 {
   char buf[80],name[13];
//   extract_name(temppath,name,FILENAME8_3);
   extract_name(filepath,name,FILENAMEALL);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,qst_error[ret],NULL,"O&K",NULL,'k',0);
   filepath[0]=0;
 }
// refresh(rALL);
 return D_O_K;
}

int onUpgrade()
{
 if(checksave()==0)
   return D_O_K;
 if(!getname("Upgrade Quest File (.qst)","qst",filepath,true))
   return D_O_K;

 int ret = load_quest2(temppath);
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
   alert(buf,qst_error[ret],NULL,"O&K",NULL,'k',0);
   filepath[0]=0;
 }

 ret = save_quest(filepath);
 char buf[80],buf2[80],name[13];
 extract_name(filepath,name,FILENAME8_3);
 if(!ret) {
   sprintf(buf,"%s successfully",name);
   sprintf(buf2,"upgraded to %s",VerStr(ZELDA_VERSION));
   alert(buf,buf2,NULL,"O&K",NULL,'k',0);
   saved=true;
   }
 else {
   sprintf(buf,"Error saving %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 return D_O_K;
}


int onImport_Map()
{
 if(Map.getCurrMap()>=MAXMAPS)
   return D_O_K;
 if(!getname("Import Map (.map)","map",datapath,false))
   return D_O_K;
 saved=false;
 int ret=Map.load(temppath);
 if(ret) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,loaderror[ret],NULL,"O&K",NULL,'k',0);
   if(ret>1)
     Map.clearmap();
   }
 refresh(rSCRMAP+rMAP+rMENU);
 return D_O_K;
}

int onExport_Map()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(Map.getCurrMap()>=MAXMAPS)
   return D_O_K;
 if(!getname("Export Map (.map)","map",datapath,false))
   return D_O_K;
 int ret = Map.save(temppath);
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(!ret)
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}



int onImport_DMaps()
{
 if(!getname("Import DMaps (.dmp)","dmp",datapath,false))
   return D_O_K;
 saved=false;
 if(!load_dmaps(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 return D_O_K;
}


int onExport_DMaps()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export DMaps (.dmp)","dmp",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_dmaps(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}



int onImport_Pals()
{
 if(!getname("Import Palettes (.zpl)","zpl",datapath,false))
   return D_O_K;
 saved=false;
 if(!load_pals(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 return D_O_K;
}


int onExport_Pals()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export Palettes (.zpl)","zpl",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_pals(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}



int onImport_Msgs()
{
 if(!getname("Import String Table (.zqs)","zqs",datapath,false))
   return D_O_K;
 saved=false;
 int ret=load_msgstrs(temppath);
 if(ret) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,loaderror[ret],NULL,"O&K",NULL,'k',0);
   if(ret==2)
     init_msgstrs();
   }
 refresh(rMENU);
 return D_O_K;
}


int onExport_Msgs()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export String Table (.zqs)","zqs",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_msgstrs(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}



int onImport_Combos()
{
 if(!getname("Import Combo Table (.cmb)","cmb",datapath,false))
   return D_O_K;
 if(!load_combos(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 else
   saved=false;
 refresh(rALL);
 return D_O_K;
}


int onExport_Combos()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export Combo Table (.cmb)","cmb",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_combos(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}



int onImport_Tiles()
{
 int ret=getnumber("Import Start Page",0);
 bound(ret,0,TILE_PAGES-1);

 if(!getname("Import Tiles (.til)","til",datapath,false))
   return D_O_K;
 saved=false;
 usetiles=true;
 if(!load_tiles(temppath, ret*TILES_PER_PAGE)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 refresh(rALL);
 return D_O_K;
}


int onExport_Tiles()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export Tiles (.til)","til",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_tiles(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}







int onImport_ZGP()
{
 if(!getname("Import Graphics Pack (.zgp)","zgp",datapath,false))
   return D_O_K;
 saved=false;
 usetiles=true;
 if(!load_zgp(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 refresh(rALL);
 return D_O_K;
}


int onExport_ZGP()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export Graphics Pack (.zgp)","zgp",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_zgp(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}


int onImport_UnencodedQuest()
{
 if(!getname("Import Unencoded Quest (.qsu)","qsu",datapath,false))
   return D_O_K;
 saved=false;
 usetiles=true;
 if(!load_qsu(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 refresh(rALL);
 return D_O_K;
}



int onExport_UnencodedQuest()
{
 #ifdef _CRASHONSAVE_
 crash();
 crash2();
 crash3();
 crash4();
 #else
 if(!getname("Export Unencoded Quest (.qsu)","qsu",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_qsu(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 #endif
 return D_O_K;
}

