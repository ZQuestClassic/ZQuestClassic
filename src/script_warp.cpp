//enum { warpFlagKILLSCRIPTDRAWS, warpFlagKILLSOUNDS, warpFlagKILLMUSIC };
//enum { warpEffectNONE, warpEffectZap, warpEffectWave, warpEffectInstant, warpEffectMozaic, warpEffectOpen }; 
//valid warpTypes: tile, side, exit, cancel, instant
bool ffscript::warp_link(int warpType, int dmapID, int scrID, int warpDestX,
	int warpDestY, int warpEffect, int warpSound, int warpFlags, int linkFacesDir)
{
	byte t = 0;
	t=(currscr<128)?0:1;
	bool overlay=false;
	if ( (unsigned)dmapID >= MAXDMAPS ) return false;
	if ( (unsigned)scrID > MAXSCREENS ) return false;
	if ( warpType == wtNOWARP ) { Z_eventlog("Used a Cancel Warped to DMap %d: %s, screen %d", currdmap, DMaps[currdmap].name,currscr); return false; }
	int mapid = (DMaps[dmapID].map+1);
        
	mapscr *m = &TheMaps[mapID * MAPSCRS + scrID]; 
	int wx = 0, wy = 0;
	if ( warpDestX < 0 )
	{
		warpDestX *= -1;
		--warpDestX;
		
		wx = m->WarpReturnX[( vbound(warpDestX,0,3) )];
	}
	else 
	{
		wx = warpDestX;
	}
	if ( warpDestY < 0 )
	{
		warpDestY *= -1;
		--warpDestY;
		
		wy = m->WarpReturnY[( vbound(warpDestY,0,3) )];
	}
	else 
	{
		wy = warpDestY;
	}
	//warp coordinates are wx, wy, not x, y! -Z
	if ( warpFlags&warpFlagKILLSCRIPTDRAWS ) script_drawing_commands.Clear();
	int wrindex = 0;
	//we also need to check if dmaps are sideview here! -Z
	//Likewise, we need to add that check to the normal Link:;dowarp(0
	bool wasSideview = isSideViewGravity(); //((tmpscr[t].flags7 & fSIDEVIEW)!=0 || DMaps[currdmap].sideview) && !ignoreSideview;
	sfx(warpSound);
	switch(warpType)
	{
		case wtIWARP:
		case wtIWARPBLK:
		case wtIWARPOPEN:
		case wtIWARPZAP:
		case wtIWARPWAVE: 
		{
			bool wasswimming = (action == swimming);
			byte olddiveclk = diveclk;
			ALLOFF();
			if ( warpFlags&warpFlagKILLMUSIC ) music_stop();
			if ( warpFlags&warpFlagKILLSOUNDS ) kill_sfx();
			if(wasswimming)
			{
				action=swimming; FFCore.setLinkAction(swimming);
				diveclk = olddiveclk;
			}
            
			switch(warpEffect)
			{
				case warpEffectZap: zapout(); break;
				case warpEffectWave: wavyout(!cavewarp); break;
				case warpEffectInstant: 
				{
				    bool b2 = COOLSCROLL&&cavewarp;
				    blackscr(30,b2?false:true);
				    break;
				}
				case warpEffectMozaic: 
				{
					
					break;
				}
				case warpEffectOpen:
				{
					
					break;
				}
				case warpEffectNONE:
				default: break;
			}
			int c = DMaps[currdmap].color;
			currdmap = dmapID;
			dlevel = DMaps[currdmap].level;
			currmap = DMaps[currdmap].map;
			init_dmap();
			update_subscreens(dmapID);
			
			ringcolor(false);
			
			if(DMaps[currdmap].color != c)
			    loadlvlpal(DMaps[currdmap].color);
			    
			homescr = currscr = scrID + DMaps[currdmap].xoff;
			
			lightingInstant(); // Also sets naturaldark
			
			loadscr(0,currdmap,currscr,-1,overlay);
			
			x = wx;
			y = wy;
			
			if(x==0)  
			{
				dir=right;
			}
			if(x==240) 
			{
				dir=left;
			}
			x
			if(y==0)   
			{
				dir=down;
			}
			
			if(y==160) 
			{
				dir=up;
			}
			
			markBmap(dir^1);
			
			if(iswater(MAPCOMBO(x,y+8)) && _walkflag(x,y+8,0) && current_item(iwarpType_flippers))
			{
			    hopclk=0xFF;
			    attackclk = charging = spins = 0;
			    action=swimming; FFCore.setLinkAction(swimming);
			}
			else
			{
			    action=none; FFCore.setLinkAction(none);
			}
			    
			//preloaded freeform combos
			ffscript_engine(true);
			
			putscr(scrollbuf,0,0,tmpscr);
			putscrdoors(scrollbuf,0,0,tmpscr);
			
			switch(warpEffect)
			{
				case warpEffectZap:  zapin(); break;
				case warpEffectWave; wavyin(); break;
				case warpEffectMozaic: 
				{
					
					break;
				}
				case warpEffectOpen:
				{
					openscreen();
					break;
				}
				case warpEffectNONE:
				default: break;
			}
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			if ( warpFlags&warpFlagKILLMUSIC ) playLevelMusic();
			currcset=DMaps[currdmap].color;
			dointro();
			setEntryPoints(x,y);
			
			break;
		}
		
		
		case wtEXIT:
		{
			lighting(false,false,pal_litRESETONLY);//Reset permLit, and do nothing else; lighting was not otherwise called on a wtEXIT warp.
			ALLOFF();
			if ( warpFlags&warpFlagKILLMUSIC ) music_stop();
			if ( warpFlags&warpFlagKILLSOUNDS ) kill_sfx();
			blackscr(30,false);
			currdmap = dmapID;
			dlevel=DMaps[currdmap].level;
			currmap=DMaps[currdmap].map;
			init_dmap();
			update_subscreens(dmapID);
			loadfullpal();
			ringcolor(false);
			loadlvlpal(DMaps[currdmap].color);
			//lastentrance_dmap = currdmap;
			homescr = currscr = scrID + DMaps[currdmap].xoff;
			loadscr(0,currdmap,currscr,-1,overlay);
			
			if(tmpscr->flags&fDARK)
			{
			    if(get_bit(quest_rules,qr_FADE))
			    {
				interpolatedfade();
			    }
			    else
			    {
				loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
			    }
			    
			    darkroom=naturaldark=true;
			}
			else
			{
			    darkroom=naturaldark=false;
			}
			
			
			if(((wx>0||wy>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
			{
			    if(dlevel)
			    {
				lastentrance = currscr;
			    }
			    else
			    {
				lastentrance = DMaps[currdmap].cont + DMaps[currdmap].xoff;
			    }
			    
			    lastentrance_dmap = dmapID;
			}
			
			//Move Link's coordinates
			x=wx;
			y=wy;
			//set his dir
			dir=down; //could be = linkFacesDir
			
			if(x==0)  
			{				
				dir = right;
			} 
			
			if(x==240) 
			{
				dir = left;
			}
			
			if(y==0)   
			{
				dir = down;
			}
			
			if(y==160)
			{	
				dir = up;
			}
			
			if(dlevel)
			{
			    // reset enemy kill counts
			    for(int i=0; i<128; i++)
			    {
				game->guys[(currmap*MAPSCRSNORMAL)+i] = 0;
				game->maps[(currmap*MAPSCRSNORMAL)+i] &= ~mTMPNORET;
			    }
			}
			
			markBmap(dir^1);
			//preloaded freeform combos
			ffscript_engine(true);
			reset_hookshot();
			
			if(isdungeon())
			{
			    openscreen();
			    if(get_bit(extra_rules, er_SHORTDGNWALK)==0)
				stepforward(diagonalMovement?11:12, false);
			    else
				// Didn't walk as far pre-1.93, and some quests depend on that
				stepforward(8, false);
			}
			else
			{
			    openscreen();
			}
			
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			playLevelMusic();
			currcset=DMaps[currdmap].color;
			dointro();
			setEntryPoints(x,y);
			
			for(int i=0; i<6; i++)
			    visited[i]=-1;
			    
			break;
			
		}
		case wtSCROLL:                                          // scrolling warp
		{
			int c = DMaps[currdmap].color;
			currmap = DMaps[dmapID].map;
			update_subscreens(dmapID);
			
			dlevel = DMaps[dmapID].level;
			    //check if Link has the map for the new location before updating the subscreen. ? -Z
			    //This works only in one direction, if Link had a map, to not having one.
			    //If Link does not have a map, and warps somewhere where he does, then the map still briefly shows. 
			update_subscreens(dmapID);
			    
			if ( has_item(iwarpType_map, dlevel) ) 
			{
				//Blank the map during an intra-dmap scrolling warp. 
				dlevel = -1; //a hack for the minimap. This works!! -Z
			}
			    
			// fix the scrolling direction, if it was a tile or instant warp
			sdir = vbound(dir,0,3);
			
			
			scrollscr(sdir, scrID+DMaps[dmapID].xoff, dmapID);
			dlevel = DMaps[dmapID].level; //Fix dlevel and draw the map (end hack). -Z
			
			reset_hookshot();
			
			if(!intradmap)
			{
			    currdmap = dmapID;
			    dlevel = DMaps[currdmap].level;
			    homescr = currscr = scrID + DMaps[dmapID].xoff;
			    init_dmap();
			    
			    
			    if(((wx>0||wy>0)||(get_bit(quest_rules,qr_WARPSIGNOREARRIVALPOINT)))&&(!get_bit(quest_rules,qr_NOSCROLLCONTINUE))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
			    {
				if(dlevel)
				{
				    lastentrance = currscr;
				}
				else
				{
				    lastentrance = DMaps[currdmap].cont + DMaps[currdmap].xoff;
				}
				
				lastentrance_dmap = dmapID;
			    }
			}
			
			if(DMaps[currdmap].color != c)
			{
			    lighting(false, true);
			}
			
			playLevelMusic();
			currcset=DMaps[currdmap].color;
			dointro();
			break;
		}
	}
	// Stop Link from drowning!
	if(action==drowning)
	{
		drownclk=0;
		action=none; FFCore.setLinkAction(none);
	}
	    
	// But keep him swimming if he ought to be!
	if(action!=rafting && iswater(MAPCOMBO(x,y+8)) && (_walkflag(x,y+8,0) || get_bit(quest_rules,qr_DROWN))
		    && (current_item(iwarpType_flippers)) && (action!=inwind))
	{
		hopclk=0xFF;
		action=swimming; FFCore.setLinkAction(swimming);
	}
	    
	nescrID_clk=frame;
	activated_timed_warp=false;
	eat_buttons();
	    
	if(warpType!=wtIWARP) { attackclk=0; }
		
	didstuff=0;
	map_bkgsfx(true);
	loadside=dir^1;
	whistleclk=-1;
	    
	if(z>0 && isSideview())
	{
		y-=z;
		z=0;
	}
	else if(!isSideview())
	{
		fall=0;
	}
	    
	// If warping between top-down and sideview screens,
	// fix enemies that are carried over by Full Screen Warp
	const bool tmpscr_is_sideview = isSideview();
	    
	if(!wasSideview && tmpscr_is_sideview)
	{
		for(int i=0; i<guys.Count(); i++)
		{
		    if(guys.spr(i)->z > 0)
		    {
			guys.spr(i)->y -= guys.spr(i)->z;
			guys.spr(i)->z = 0;
		    }
		    
		    if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
			guys.spr(i)->yofs += 2;
		}
	}
	else if(wasSideview && !tmpscr_is_sideview)
	{
		for(int i=0; i<guys.Count(); i++)
		{
		    if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
			guys.spr(i)->yofs -= 2;
		}
	}
	if ( warpType == wtEXIT )
	{
		game->set_continue_scrn(DMaps[currdmap].cont + DMaps[currdmap].xoff);
		game->set_continue_dmap(currdmap);
		lastentrance_dmap = currdmap;
		lastentrance = game->get_continue_scrn();
	}
	if(tmpscr->flags4&fAUTOSAVE)
	{
		save_game(true,0);
	}
	    
	if(tmpscr->flags6&fCONTINUEHERE)
	{
		lastentrance_dmap = currdmap;
		lastentrance = homescr;
	}
	    
	update_subscreens();
	verifyBothWeapons();
	Z_eventlog("Warped to DMap %d: %s, screen %d, via %s.\n", currdmap, DMaps[currdmap].name,currscr,
                        warpType==wtEXIT ? "Entrance/Exit" :
                        warpType==wtSCROLL ? "Scrolling Warp" :
                        warpType==wtNOWARP ? "Cancel Warp" :
                        "Insta-Warp");
                        
	eventlog_mapflags();
	return true;
	
	
	
}