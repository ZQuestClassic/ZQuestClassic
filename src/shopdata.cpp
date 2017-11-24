
struct shoptype
{
    char name[32];
    byte item[3];
    byte hasitem[3];
    word price[3];
    //10
};

struct infotype
{
    char name[32];
    word str[3];
    //byte padding;
    word price[3];
    //14
};

struct miscQdata
{
    shoptype shop[256];
    //160 (160=10*16)
    infotype info[256];
}

#define SHOPDATANAME
#define SHOPDATAITEM
#define SHOPDATAHASITEM
#define SHOPDATAPRICE
//Pay for info
#define SHOPDATASTRING 

#define LOADSHOPR
#define LOADSHOPV
//Game->LoadShopData(int shop)
//loads shop ID
#define LOADINFOSHOPR
#define LOADINFOSHOPV
//Game->LoadInfoShopData(int id)
//loads id+256 then if id > 255, we know it is an info shop type.
//The pointer value for shops ia 0-255, for info shops 256-511


//New Datatypes
	case LOADSHOPR:
            FFScript::do_loadshopdata(false);
            break;
        case LOADSHOPV:
            FFScript::do_loadshopdata(true);
            break;
	
	case LOADINFOSHOPR:
            FFScript::do_loadinfoshopdata(false);
            break;
        case LOADINFOSHOPV:
            FFScript::do_loadinfoshopdata(true);
            break;
	
	
void FFScript::do_loadshopdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 255 )
    {
	Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
	ri->shopsref = LONG_MAX;
    }
        
    else ri->shopsref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}

#define NUMSHOPS 256
void FFScript::do_loadinfoshopdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 255 )
    {
	Z_scripterrlog("Invalid Shop ID passed to Game->LoadShopData: %d\n", ID);
	ri->shopsref = LONG_MAX;
    }
        
    else ri->shopsref = ID+NUMSHOPS;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}


misc.shop[index].member
misc.info[index].price

#define SET_SHOPDATA_VAR_INDEX(member, str, indexbound) \
	{ \
			int ref = ri->shopsref; \
			byte isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); \
			int indx = ri->d[0] / 10000; \
			if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				if ( isInfo ) \
				{ \
					misc.info[indx].member = vbound((value/10000), 0, 214747)); \
				} \
				else \
				{ \
					misc.shop[indx].member = vbound((value/10000), 0, 214747)); \
				} \
			} \
	}

	#define SET_SHOPDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
			int ref = ri->shopsref; \
			byte isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); \
			int indx = ri->d[0] / 10000; \
			if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				if ( isInfo ) \
				{ \
					misc.info[indx].member = vbound((value/10000), 0, 214747)); \
				} \
				else \
				{ \
					misc.shop[indx].member = vbound((value/10000), 0, 214747)); \
				} \
			} \
	}
	
	//case SHOPDATANAME
	
#define NUMINFOSHOPS 511
case SHOPDATAITEM: SET_SHOPDATA_VAR_INDEX(item, Item, 2); break;
case SHOPDATAHASITEM: SET_SHOPDATA_VAR_INDEX(hasitem, HasItem, 2); break;
case SHOPDATAPRICE: SET_SHOPDATA_VAR_INDEX(price, Price, 2); break;
//Pay for info
case SHOPDATASTRING:
{
	if ( ri->shopsref < NUMSHOPS || ri->shopsref > NUMINFOSHOPS )
	{
		Z_scripterrlog("Invalid Info Shop ID passed to shopdata->String[]: %d\n", ri->shopsrer); 
		break;
	}
	else SET_SHOPDATA_VAR_INDEX(str, String, 2); break;
}



	void do_setShoName()
	{
	    long arrayptr = get_register(sarg1) / 10000;
	    int ref = ri->shopsref; \
	    byte isInfo = ( ref > NUMSHOPS && ref < LONG_MAX );
	    string shopinfo_str;

		misc.shop[index].member

		
	    ArrayH::getString(arrayptr, shopinfo_str, 32);
	    if ( isInfo )
	    {
		strncpy(misc.info[ref].name, shopinfo_str.c_str(), 31);
		misc.info[ref].name[32]='\0';
	    }
	    else
	    {
		strncpy(misc.shop[ref].name, shopinfo_str.c_str(), 31);
		misc.shop[ref].name[32]='\0';
	    }
	}
			
			
	//getter
	
	
	#define GET_SHOPDATA_VAR_INDEX(member, str, indexbound) \
	{ \
			int ref = ri->shopsref; \
			byte isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); \
			int indx = ri->d[0] / 10000; \
			if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				if ( isInfo ) \
				{ \
					ret = (misc.info[indx].member*10000); \
				} \
				else \
				{ \
					ret = (misc.shop[indx].member * 10000) \
				} \
			} \
	}

	#define GET_SHOPDATA_BYTE_INDEX(member, str, indexbound) \
	{ \
			int ref = ri->shopsref; \
			byte isInfo = ( ref > NUMSHOPS && ref < LONG_MAX ); \
			int indx = ri->d[0] / 10000; \
			if ( indx < 0 || indx > indexbound ) \
			{ \
				Z_scripterrlog("Invalid Array Index passed to shopdata->%s: %d\n", indx, str); \
			} \
			else \
			{ \
				if ( isInfo ) \
				{ \
					ret = (misc.info[indx].member*10000); \
				} \
				else \
				{ \
					ret = (misc.shop[indx].member * 10000) \
				} \
			} \
	}
	case SHOPDATAITEM: GET_SHOPDATA_VAR_INDEX(item, Item, 2); break;
	case SHOPDATAHASITEM: GET_SHOPDATA_VAR_INDEX(hasitem, HasItem, 2); break;
	case SHOPDATAPRICE: GET_SHOPDATA_VAR_INDEX(price, Price, 2); break;
	//Pay for info
	case SHOPDATASTRING:
	{
		if ( ri->shopsref < NUMSHOPS || ri->shopsref > NUMINFOSHOPS )
		{
			Z_scripterrlog("Invalid Info Shop ID passed to shopdata->String[]: %d\n", ri->shopsrer); 
			break;
		}
		else GET_SHOPDATA_VAR_INDEX(str, String, 2); break;
	}