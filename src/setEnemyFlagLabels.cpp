
struct EnemyNameInfo
{
    int enemy_family;
    char *flag1;
    char *flag2;
    char *flag3;
    char *flag4;
    char *flag5;
   
    char *flag6;
    char *flag7;
    char *flag8;
    char *flag9;
    char *flag10;
    char *flag11;
    char *flag12;
    char *flag13;
    char *flag14;
    char *flag15;
    char *flag16;
    
};

//Sets the Item Editor Field Names
static EnemyNameInfo enameinf[]=
{	
		//class			//flag1,	//flag2,	//flag3, 	//flag4,	//flag5,	//flag6, 	//flag7, 	//flag8,	//flag9,	//flag10,	//flag11,	//flag12,	//flag13,	//flag14,	//flag15,	//flag16
		
		{ eeGUY, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" }
		{ eeWALK,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" }
	
		{ eeTEK, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeLEV, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eePEAHAT, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeZORA, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeROCK,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeGHINI, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeKEESE, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeTRAP,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeWALLM, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeWIZZ, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeAQUA, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeMOLD,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeDONGO, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeMANHAN,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeGLEEOK, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeDIG, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeGHOMA, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeLANM, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eePATRA, 			(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeGANON,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eePROJECTILE,  		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeSPINTILE, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeNONE,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeFAIRY, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeFIRE, 		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		{ eeOTHER,		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
		//{ scripted		(char *)"Flag1:",(char *)"Flag2:",(char *)"Flag3:",(char *)"Flag4:",(char *)"Flag5:",(char *)"Flag6:",(char *)"Flag7:",(char *)"Flag8:",(char *)"Flag9:",(char *)"Flag10:",(char *)"Flag11:",(char *)"Flag12:",(char *)"Flag13:",(char *)"Flag14:",(char *)"Flag15:",(char *)"Flag16:" },
                
		{ -1,                             NULL,                                     NULL,                                       NULL,                                           NULL,                                   NULL,                                   NULL,                              NULL,                              NULL,                              NULL,                              NULL,                                        NULL,                                   NULL,                                   NULL,                                   NULL,                                   NULL,                                   NULL,                                   NULL,                                        NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                      NULL,                                           NULL,                                                                             NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL,                                           NULL                 }
};

static std::map<int, EnemyNameInfo *> *enamemap = NULL;

std::map<int, ItemNameInfo *> *getEnemyNameMap()
{
    if(enamemap == NULL)
    {
        enamemap = new std::map<int, EnemyNameInfo *>();
        
        for(int i=0;; i++)
        {
            EnemyNameInfo *inf = &enameinf[i];
            
            if(inf->enemy_family == -1)
                break;
                
            (*enamemap)[inf->enemy_family] = inf;
        }
    }
    
    return enamemap;
}


void setEnemyLabels(int iclass)
{
    std::map<int, EnemyNameInfo *> *nmap = getEnemyNameMap();
    std::map<int, EnemyNameInfo *>::iterator it = nmap->find(enemy_family);
    EnemyNameInfo *inf = NULL;
    
    if(it != nmap->end())
        inf = it->second;
        
    if((inf == NULL) || (inf->flag1 == NULL))
    {
        enedata_dlg[254].dp = (void *)"Flags[0]";
        enedata_dlg[254].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[254].dp = inf->flag1;
        enedata_dlg[254].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag2 == NULL))
    {
        enedata_dlg[255].dp = (void *)"Flags[1]";
        enedata_dlg[255].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[255].dp = inf->flag2;
        enedata_dlg[255].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag3 == NULL))
    {
        enedata_dlg[256].dp = (void *)"Flags[2]";
        enedata_dlg[256].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[256].dp = inf->flag3;
        enedata_dlg[256].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag4 == NULL))
    {
        enedata_dlg[257].dp = (void *)"Flags[3]";
        enedata_dlg[257].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[257].dp = inf->flag4;
        enedata_dlg[257].flags &= ~D_DISABLED;
    }
    
    if((inf == NULL) || (inf->flag5 == NULL))
    {
        enedata_dlg[258].dp = (void *)"Flags[4]";
        enedata_dlg[258].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[258].dp = inf->flag5;
        enedata_dlg[258].flags &= ~D_DISABLED;
    }
    
    //flags 6 through 16
    //flag6
    if((inf == NULL) || (inf->flag6 == NULL))
    {
        enedata_dlg[259].dp = (void *)"Flags[5]";
        enedata_dlg[259].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[259].dp = inf->flag6;
        enedata_dlg[259].flags &= ~D_DISABLED;
    }
	//flag7
    if((inf == NULL) || (inf->flag7 == NULL))
    {
        enedata_dlg[260].dp = (void *)"Flags[6]";
        enedata_dlg[260].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[260].dp = inf->flag7;
        enedata_dlg[260].flags &= ~D_DISABLED;
    }
    //flag8
    if((inf == NULL) || (inf->flag8 == NULL))
    {
        enedata_dlg[261].dp = (void *)"Flags[7]";
        enedata_dlg[261].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[261].dp = inf->flag8;
        enedata_dlg[261].flags &= ~D_DISABLED;
    }
    //flag9
    if((inf == NULL) || (inf->flag9 == NULL))
    {
        enedata_dlg[262].dp = (void *)"Flags[8]";
        enedata_dlg[262].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[262].dp = inf->flag9;
        enedata_dlg[262].flags &= ~D_DISABLED;
    }
    //flag10
    if((inf == NULL) || (inf->flag10 == NULL))
    {
        enedata_dlg[263].dp = (void *)"Flags[9]";
        enedata_dlg[263].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[263].dp = inf->flag10;
        enedata_dlg[263].flags &= ~D_DISABLED;
    }
    //flag11
    if((inf == NULL) || (inf->flag11 == NULL))
    {
        enedata_dlg[264].dp = (void *)"Flags[10]";
        enedata_dlg[264].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[264].dp = inf->flag11;
        enedata_dlg[264].flags &= ~D_DISABLED;
    }
    //flag12
    if((inf == NULL) || (inf->flag12 == NULL))
    {
        enedata_dlg[265].dp = (void *)"Flags[11]";
        enedata_dlg[265].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[265].dp = inf->flag12;
        enedata_dlg[265].flags &= ~D_DISABLED;
    }
    //flag13
    if((inf == NULL) || (inf->flag13 == NULL))
    {
        enedata_dlg[266].dp = (void *)"Flags[12]";
        enedata_dlg[266].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[266].dp = inf->flag13;
        enedata_dlg[266].flags &= ~D_DISABLED;
    }
    //flag14
    if((inf == NULL) || (inf->flag14 == NULL))
    {
        enedata_dlg[267].dp = (void *)"Flags[13]";
        enedata_dlg[267].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[267].dp = inf->flag14;
        enedata_dlg[267].flags &= ~D_DISABLED;
    }
    //flag15
    if((inf == NULL) || (inf->flag15 == NULL))
    {
        enedata_dlg[268].dp = (void *)"Flags[14]";
        enedata_dlg[268].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[268].dp = inf->flag15;
        enedata_dlg[268].flags &= ~D_DISABLED;
    }
    //flag16
    if((inf == NULL) || (inf->flag16 == NULL))
    {
        enedata_dlg[269].dp = (void *)"Flags[15]";
        enedata_dlg[269].flags &= ~D_DISABLED;
    }
    else
    {
        enedata_dlg[269].dp = inf->flag16;
        enedata_dlg[269].flags &= ~D_DISABLED;
    }
}