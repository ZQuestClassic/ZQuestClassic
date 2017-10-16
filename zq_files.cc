/*
  ZQ_FILES.CC
*/


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

 int ret=load_quest(temppath);
 if(!ret) {
   strcpy(filepath,temppath);
   saved=true;
   }
 else {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,qst_error[ret],NULL,"O&K",NULL,'k',0);
   filepath[0]=0;
   }
 refresh(rALL);
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
 if(!getname("Export DMaps (.dmp)","dmp",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_dmaps(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
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
 if(!getname("Export Palettes (.zpl)","zpl",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_pals(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
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
 if(!getname("Export String Table (.zqs)","zqs",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_msgstrs(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 return D_O_K;
}



int onImport_Combos()
{
 if(!getname("Import Combo Table (.cmb)","cmb",datapath,false))
   return D_O_K;
 saved=false;
 if(!load_combos(temppath)) {
   char buf[80],name[13];
   extract_name(temppath,name,FILENAME8_3);
   sprintf(buf,"Unable to load %s",name);
   alert(buf,NULL,NULL,"O&K",NULL,'k',0);
   }
 refresh(rALL);
 return D_O_K;
}


int onExport_Combos()
{
 if(!getname("Export Combo Table (.cmb)","cmb",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_combos(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 return D_O_K;
}



int onImport_Tiles()
{
 if(!getname("Import Tiles (.til)","til",datapath,false))
   return D_O_K;
 saved=false;
 usetiles=true;
 if(!load_tiles(temppath)) {
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
 if(!getname("Export Tiles (.til)","til",datapath,false))
   return D_O_K;
 char buf[80],name[13];
 extract_name(temppath,name,FILENAME8_3);
 if(save_tiles(temppath))
   sprintf(buf,"Saved %s",name);
 else
   sprintf(buf,"Error saving %s",name);
 alert(buf,NULL,NULL,"O&K",NULL,'k',0);
 return D_O_K;
}

