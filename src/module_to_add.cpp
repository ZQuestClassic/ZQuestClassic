//name entry tiles

hardcoded selector tile

// overtile16(framebuf,0,48,ypos+17,(save_num%3)+10,0);               //link
//! reposition this call and allow settig the tile data in the module
//hardcoded quest icons:
//these are called by overtile16 in this function:
static void list_save(int save_num, int ypos)
{
    bool r = refreshpal;
    
    if(save_num<savecnt)
    {
        game->set_maxlife(saves[save_num].get_maxlife());
        game->set_life(saves[save_num].get_maxlife());
        wpnsbuf[iwQuarterHearts].newtile = 4;
        //boogie!
        lifemeter(framebuf,144,ypos+((game->get_maxlife()>16*(game->get_hp_per_heart()))?8:0),0,0);
        textout_ex(framebuf,zfont,saves[save_num].get_name(),72,ypos+16,1,0);
        
        if(saves[save_num].get_quest())
            textprintf_ex(framebuf,zfont,72,ypos+24,1,0,"%3d",saves[save_num].get_deaths());
            
        if(saves[save_num].get_quest()==2)
            //2nd quest hardcoded icon
            overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
            
        if(saves[save_num].get_quest()==3)
        {
            overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
            overtile16(framebuf,41,41,ypos+14,9,0);             //put sword on third quests
        }
        
        if(saves[save_num].get_quest()==4)
        {
            overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
            overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
        }

		if(saves[save_num].get_quest()==5)
		{
			overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
		}

        textprintf_ex(framebuf,zfont,72,ypos+16,1,0,"%s",saves[save_num].get_name());
    }
    
    byte *hold = newtilebuf[0].data;
    byte holdformat=newtilebuf[0].format;
    newtilebuf[0].format=tf4Bit;
    newtilebuf[0].data = saves[save_num].icon;
    overtile16(framebuf,0,48,ypos+17,(save_num%3)+10,0);               //link
    newtilebuf[0].format=holdformat;
    newtilebuf[0].data = hold;
    
    hold = colordata;
    colordata = saves[save_num].pal;
    loadpalset((save_num%3)+10,0); //quest number changes the palette
    colordata = hold;
    
    textout_ex(framebuf,zfont,"-",136,ypos+16,1,0);
    
    refreshpal = r;
}

load_game_icon
load_game_icon_to_buffer

resetItems --called by init_game() in zelda.cpp, if (firstplay)
	
write (game->_hasplayed = true to prevent this.

In init_game() (zeldaq.cpp),   game->Clear() is what clears all of the data between quests.

Allowing serial quests without clearing data will need a completely
new set of functions. 


subscreen counters
defence results?
weapon types (wMax)

guy names (e.g. Abei) : done for the screen menu. 
	They may need to be fixed elsewhere, and their help strings need to be fixed.

Zora screen flag

Help text for combos, guys and so forth.
	
const char *combotype_help_string[cMAX*3] =
{
    "","","",
    "Link is warped via Tile Warp A if he steps on","the bottom half of this combo.","",
    "Link marches down into this combo and is warped","via Tile Warp A if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Liquid can contain Zora enemies and can be","crossed with various weapons and items.","If the matching quest rule is set, Link can drown in it.",
    "When touched, this combo produces an Armos","and changes to the screen's Under Combo.","",
    "When touched, this combo produces one Ghini.","","",
    "Raft paths must begin on a Dock-type combo.","(Use the Raft combo flag to create raft paths.)","",
    "","","",
    "A Bracelet is not needed to push this combo,", "but it can't be pushed until the", "enemies are cleared from the screen.",
    "A Bracelet is needed to push this combo.","The screen's Under Combo will appear beneath","it when it is pushed aside.",
    "A Bracelet is needed to push this combo,","and it can't be pushed until the", "enemies are cleared from the screen.",
    "If the 'Statues Shoot Fire' Screen Data","flag is checked, an invisible fireball","shooting enemy is spawned on this combo.",
    "If the 'Statues Shoot Fire' Screen Data","flag is checked, an invisible fireball","shooting enemy is spawned on this combo.",
    "Link's movement speed is reduced","while he walks on this combo.","Enemies will not be affected.",
    // Conveyors
    "","","",
    "","","",
    "","","",
    "","","",
    // Anyway...
    "Link is warped via Tile Warp A if he swims on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp A if he dives on","this combo. Otherwise, this is identical to Water.","",
    "If this combo is solid, the Ladder and Hookshot","can be used to cross over it.","It only permits the Ladder if it's on Layer 0.",
    "This triggers Screen Secrets when the bottom half","of this combo is stepped on, but it does not set","the screen's 'Secret' Screen State.",
    "This triggers Screen Secrets when the bottom half","of this combo is stepped on, and sets the screen's","'Secret' Screen State, making the secrets permanent.",
    "","","", // Unused
    "When stabbed or slashed with a Sword, this", "combo changes into the screen's Under Combo.","",
    "Identical to Slash, but an item","from Item Drop Set 12 is created","when this combo is slashed.",
    "A Bracelet with a Push Combo Level of 2","is needed to push this combo. Otherwise,","this is identical to Push (Heavy).",
    "A Bracelet with a Push Combo Level of 2","is needed to push this combo. Otherwise,","this is identical to Push (Heavy, Wait).",
    "When hit by a Hammer, this combo changes","into the next combo in the list.","",
    "If this combo is struck by the Hookshot,", "Link is pulled towards the combo.","",
    // Damage Combos
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",
    // Anyway...
    "If the 'Statues Shoot Fire' Screen Data","flag is checked, an invisible fireball","shooting enemy is spawned on this combo.",
    "This flag is obsolete. It behaves identically to","Combo Flag 32, Trap (Horizontal, Line of Sight).","",
    "This flag is obsolete. It behaves identically to","Combo Flag 33, Trap (Vertical, Line of Sight).","",
    "This flag is obsolete. It behaves identically to","Combo Flag 34, Trap (4-Way, Line of Sight).","",
    "This flag is obsolete. It behaves identically to","Combo Flag 35, Trap (Horizontal, Constant).","",
    "This flag is obsolete. It behaves identically to","Combo Flag 36, Trap (Vertical Constant).","",
    "Link is warped via Tile Warp A if he touches","any part of this combo, but his on-screen","position remains the same. Ground enemies can't enter.",
    "If this combo is solid, the Hookshot","can be used to cross over it.","",
    "This combo's tile is drawn between layers","3 and 4 if it is placed on layer 0.","",
    "Flying enemies (Keese, Peahats, Moldorms,","Patras, Fairys, Digdogger, Manhandla, Ghinis,","Gleeok heads) can't fly over or appear on this combo.",
    "Wand magic and enemy magic that hits"," this combo is reflected 180 degrees,","and becomes 'reflected magic'.",
    "Wand magic and enemy magic that hits"," this combo is reflected 90 degrees,","and become 'reflected magic'.",
    "Wand magic and enemy magic that hits"," this combo is reflected 90 degrees,","and become 'reflected magic'.",
    "Wand magic and enemy magic that hits"," this combo is duplicated twice, causing","three shots to be fired in three directions.",
    "Wand magic and enemy magic that hits"," this combo is duplicated thrice, causing","four shots to be fired from each direction.",
    "Wand magic and enemy magic that hits","this combo is destroyed.","",
    "Link marches up into this combo and is warped","via Tile Warp A if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "The combo's tile changes depending on","Link's position relative to the combo.","It uses eight tiles per animation frame.",
    "Identical to Eyeball (8-Way A), but the","angles at which the tile will change are","offset by 22.5 degrees (pi/8 radians).",
    "Tektites cannot jump through or","appear on this combo.","",
    "Identical to Slash->Item, but when it is","slashed, Bush Leaves sprites are drawn and","the 'Tall Grass slashed' sound plays.",
    "Identical to Slash->Item, but when it is","slashed, Flower Clippings sprites are","drawn and the 'Tall Grass slashed' sound plays.",
    "Identical to Slash->Item, but when it is","slashed, Grass Clippings sprites are","drawn and the 'Tall Grass slashed' sound plays.",
    "Ripples sprites are drawn on Link when","he walks on this combo. Also, Quake","Hammer pounds are nullified by this combo.",
    "If the combo is solid and Link pushes it with at","least one Key, it changes to the next combo, the 'Lock Blocks'","Screen State is set, and one key is used up.",
    "Identical to Lock Block, but if any other","Lock Blocks are opened on the same screen,","this changes to the next combo.",
    "If the combo is solid and Link pushes it with the","Boss Key, it changes to the next combo and","the 'Boss Lock Blocks' Screen State is set.",
    "Identical to Lock Block (Boss), but if any other","Boss Lock Blocks are opened on the same","screen, this changes to the next combo.",
    "If this combo is solid, the Ladder","can be used to cross over it.","Only works on layer 0.",
    "When touched, this combo produces a Ghini","and changes to the next combo in the list.","",
    "If Link pushes the bottom of this solid combo,","it changes to the next combo in the list.","Best used with the Armos/Chest->Item combo flag.",
    "Identical to Treasure Chest (Normal), but if any","other Normal Treasure Chests are opened on the","same screen, this changes to the next combo.",
    "Identical to Treasure Chest (Normal), but if Link","doesn't have a Key, it can't be opened.","Otherwise, a key is used up when it opens.",
    "Identical to Treasure Chest (Locked), but if any","other Locked Treasure Chests are opened on the","same screen, this changes to the next combo.",
    "Identical to Treasure Chest (Normal), but if Link","doesn't have the Boss Key, it can't be opened.","",
    "Identical to Treasure Chest (Boss), but if any","other Boss Treasure Chests are opened on the","same screen, this changes to the next combo.",
    "If Link touches this, the Screen States are cleared,","and Link is re-warped back into the screen,","effectively resetting the screen entirely.",
    "Press the 'Start' button when Link is standing on","the bottom of this combo, and the Save menu appears. Best","used with the Save Point->Continue Here Screen Flag.",
    "Identical to Save Point, but the Quit option","is also available in the menu.","",
    "Link marches down into this combo and is warped","via Tile Warp B if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link marches down into this combo and is warped","via Tile Warp C if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link marches down into this combo and is warped","via Tile Warp D if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link is warped via Tile Warp B if he steps on","the bottom half of this combo.","",
    "Link is warped via Tile Warp C if he steps on","the bottom half of this combo.","",
    "Link is warped via Tile Warp D if he steps on","the bottom half of this combo.","",
    "Link is warped via Tile Warp B if he touches","any part of this combo, but his on-screen","position remains the same. Ground enemies can't enter.",
    "Link is warped via Tile Warp C if he touches","any part of this combo, but his on-screen","position remains the same. Ground enemies can't enter.",
    "Link is warped via Tile Warp D if he touches","any part of this combo, but his on-screen","position remains the same. Ground enemies can't enter.",
    "Link marches up into this combo and is warped","via Tile Warp B if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link marches up into this combo and is warped","via Tile Warp C if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link marches up into this combo and is warped","via Tile Warp D if he steps on this. The combo's tile","will be drawn above Link during this animation.",
    "Link is warped via Tile Warp B if he swims on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp C if he swims on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp D if he swims on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp B if he dives on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp C if he dives on","this combo. Otherwise, this is identical to Water.","",
    "Link is warped via Tile Warp D if he dives on","this combo. Otherwise, this is identical to Water.","",
    "Identical to Stairs [A], but the Tile Warp","used (A, B, C, or D) is chosen at random. Use this only","in screens where all four Tile Warps are defined.",
    "Identical to Direct Warp [A], but the Tile Warp","used (A, B, C, or D) is chosen at random. Use this only","in screens where all four Tile Warps are defined.",
    "As soon as this combo appears on the screen,","Side Warp A is triggered. This is best used with","secret combos or combo cycling.",
    "As soon as this combo appears on the screen,","Side Warp B is triggered. This is best used with","secret combos or combo cycling.",
    "As soon as this combo appears on the screen,","Side Warp C is triggered. This is best used with","secret combos or combo cycling.",
    "As soon as this combo appears on the screen,","Side Warp D is triggered. This is best used with","secret combos or combo cycling.",
    "Identical to Auto Side Warp [A], but the Side Warp","used (A, B, C, or D) is chosen at random. Use this only","in screens where all four Side Warps are defined.",
    "Identical to Stairs [A], but Link will be warped","as soon as he touches the edge of this combo.","",
    "Identical to Stairs [B], but Link will be warped","as soon as he touches the edge of this combo.","",
    "Identical to Stairs [C], but Link will be warped","as soon as he touches the edge of this combo.","",
    "Identical to Stairs [D], but Link will be warped","as soon as he touches the edge of this combo.","",
    "Identical to Stairs [Random], but Link will be","warped as soon as he touches the edge of this combo.","",
    "Identical to Step->Secrets (Temporary), but","Screen Secrets are triggered as soon as","Link touches the edge of this combo.",
    "Identical to Step->Secrets (Permanent), but","Screen Secrets are triggered as soon as","Link touches the edge of this combo.",
    "When Link steps on this combo, it will","change into the next combo in the list.","",
    "Identical to Step->Next, but if other instances","of this particular combo are stepped on, this","also changes to the next combo in the list.",
    "When Link steps on this combo, each of","the Step->Next combos on screen will change","to the next combo after them in the list.",
    "When Link steps on a Step->Next (All) type combo,","this will change into the next combo in the list.","",
    "Enemies cannot enter or","appear on this combo.","",
    "Level 1 Link arrows that hit this combo","are destroyed. Enemy arrows are unaffected.","",
    "Level 1 or 2 Link arrows that hit this combo","are destroyed. Enemy arrows are unaffected.","",
    "All Link arrows that hit this combo","are destroyed. Enemy arrows are unaffected.","",
    "Level 1 Link boomerangs bounce off","this combo. Enemy boomerangs are unaffected.","",
    "Level 1 or 2 Link boomerangs bounce off","this combo. Enemy boomerangs are unaffected.","",
    "All Link boomerangs bounce off","this combo. Enemy boomerangs are unaffected.","",
    "Link sword beams or enemy sword beams","that hit this combo are destroyed.","",
    "All weapons that hit this combo are","either destroyed, or bounce off.","",
    "Enemy fireballs and reflected fireballs","that hit this combo are destroyed.","",
    // More damage
    "","","",
    "","","",
    "","","",
    "","","", // Unused
    "A Spinning Tile immediately appears on this combo,","using the combo's tile to determine its sprite.","The combo then changes to the next in the list.",
    "","","", // Unused
    "While this combo is on the screen, all action is frozen,","except for FFC animation and all scripts. Best used","in conjunction with Changer FFCs or scripts.",
    "While this combo is on the screen, FFCs and FFC scripts","will be frozen. Best used in conjunction","with combo cycling, screen secrets or global scripts.",
    "Enemies that don't fly or jump","cannot enter or appear on","this combo.",
    "Identical to Slash, but instead of changing","into the Under Combo when slashed, this","changes to the next combo in the list.",
    "Identical to Slash (Item), but instead of changing","into the Under Combo when slashed, this","changes to the next combo in the list.",
    "Identical to Bush, but instead of changing","into the Under Combo when slashed, this","changes to the next combo in the list.",
    // Continuous variation
    "Slash","","",
    "Slash->Item","","",
    "Bush","","",
    "Flowers","","",
    "Tall Grass","","",
    "Slash->Next","","",
    "Slash->Next (Item)","","",
    "Bush->Next","","",
    "Identical to Eyeball (8-Way A), but only the four","cardinal directions/sets of tiles are used","(up, down, left and right, respectively).",
    "Identical to Tall Grass, but instead of changing","into the Under Combo when slashed, this","changes to the next combo in the list.",
    // Script types
    "","","", //1
    "","","",
    "","","",
    "","","",
    "","","", //5
    "","","",
    "","","",
    "","","",
    "","","",
    "","","", //10
    "","","",
    "","","",
    "","","",
    "","","",
    "","","", //15
    "","","",
    "","","",
    "","","",
    "","","",
    "","","" //20
};

const char *flag_help_string[(mfMAX)*3] =
{
    "","","",
    "Allows Link to push the combo up or down once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Allows Link to push the combo in any direction once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Triggers Screen Secrets when Link plays the Whistle on it.","Is replaced with the 'Whistle' Secret Combo.","Doesn't interfere with Whistle related Screen Flags.",
    "Triggers Screen Secrets when Link touches it with", "fire from any source (Candle, Wand, Din's Fire, etc.)", "Is replaced with the 'Blue Candle' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with one of his Arrows.", "Is replaced with the 'Wooden Arrow' Secret Combo.",
    "Triggers Screen Secrets when the middle", "part of a Bomb explosion touches it.", "Is replaced with the 'Bomb' Secret Combo.",
    "Makes a heart circle appear on screen", "when Link steps on it, and refills his life.", "See also the Heart Circle-related Quest Rules.",
    "Place in paths to define the path Link travels", "when using the Raft. Use with Dock-type combos.", "If a path branches, Link takes the clockwise-most path.",
    "When placed on an Armos-type combo, causes the 'Stairs'"," Secret Combo to appear when the Armos is triggered,","instead of the screen's Under Combo.",
    "When placed on an Armos or treasure chest, causes","the room's Special Item to appear when the combo is activated.","Requires the 'Special Item' Room Type.",
    "Triggers Screen Secrets when the middle", "part of a Super Bomb explosion touches it.", "Is replaced with the 'Super Bomb' Secret Combo.",
    "Place at intersections of Raft flag paths to define", "points where the player may change directions.","Change directions by holding down a directional key.",
    "When Link dives on a flagged water-type combo","he will recieve the screen's Special Item.","Requires the 'Special Item' Room Type.",
    "Combos with this flag will flash white when", "viewed with the Lens of Truth item.","",
    "When Link steps on this flag, the quest will", "end, and the credits will roll.","",
    // 16-31
    "","","",
    "","","",
    "","","",//18
    "","","",
    "","","",
    "","","",//21
    "","","",
    "","","",
    "","","",//24
    "","","",
    "","","",
    "","","",//27
    "","","",
    "","","",
    "","","",//30
    "","","",
    // Anyway...
    "Creates the lowest-numbered enemy with the","'Spawned by 'Horz Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
    "Creates the lowest-numbered enemy with the","'Spawned by 'Vert Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
    "Creates the lowest-numbered enemy with the","'Spawned by '4-Way Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
    "Creates the lowest-numbered enemy with the","'Spawned by 'LR Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
    "Creates the lowest-numbered enemy with the","'Spawned by 'UD Trap' Combo Type/Flag' enemy","data flag on the flagged combo.",
    // Enemy 0-9
    "","","",
    "","","",
    "","","",//2
    "","","",
    "","","",
    "","","",//5
    "","","",
    "","","",
    "","","",//8
    "","","",
    //Anyway...
    "Allows Link to push the combo left or right once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Allows Link to push the combo up once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Allows Link to push the combo down once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Allows Link to push the combo left once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    "Allows Link to push the combo right once,","triggering Screen Secrets (or just the 'Stairs',","secret combo) as well as Block->Shutters.",
    // Push Silent
    "","","",//52
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",//59
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",
    "","","",
    //Anyway...
    "Pushing blocks onto ALL Block Triggers will trigger","Screen Secrets (or just the 'Stairs' secret combo)","as well as Block->Shutters.",
    "Prevents push blocks from being pushed onto the","flagged combo, even if it is not solid.","",
    "Triggers Screen Secrets when Link", "touches it with one of his Boomerangs.", "Is replaced with the 'Wooden Boomerang' Secret Combo.",
    "Triggers Screen Secrets when Link touches", "it with a level 2 or higher Boomerang.", "Is replaced with the 'Magic Boomerang' Secret Combo.",
    "Triggers Screen Secrets when Link touches", "it with a level 3 or higher Boomerang.", "Is replaced with the 'Fire Boomerang' Secret Combo.",
    "Triggers Screen Secrets when Link touches", "it with a level 2 or higher Arrow.", "Is replaced with the 'Silver Arrow' Secret Combo.",
    "Triggers Screen Secrets when Link touches", "it with a level 3 or higher Arrow.", "Is replaced with the 'Golden Arrow' Secret Combo.",
    "Triggers Screen Secrets when Link touches it with", "fire from a level 2 Candle, a Wand, or Din's Fire.", "Is replaced with the 'Red Candle' Secret Combo.",
    "Triggers Screen Secrets when Link touches it with", "fire from a Wand, or Din's Fire.", "Is replaced with the 'Wand Fire' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with Din's Fire. Is replaced", "with the 'Din's Fire' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with Wand magic, be it fire or not.", "Is replaced with the 'Wand Magic' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with reflected Wand magic.", "Is replaced with the 'Reflected Magic' Secret Combo.",
    "Triggers Screen Secrets when Link touches", "it with a Shield-reflected fireball.", "Is replaced with the 'Reflected Fireball' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with one of his Swords.", "Is replaced with the 'Wooden Sword' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 2 or higher Sword.", "Is replaced with the 'White Sword' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 3 or higher Sword.", "Is replaced with the 'Magic Sword' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 4 or higher Sword.", "Is replaced with the 'Master Sword' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with one of his Sword beams.", "Is replaced with the 'Sword Beam' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 2 or higher Sword's beam.", "Is replaced with the 'White Sword Beam' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 3 or higher Sword's beam.", "Is replaced with the 'Magic Sword Beam' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with a level 4 or higher Sword's beam.", "Is replaced with the 'Master Sword Beam' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with one of his Hookshot hooks.", "Is replaced with the 'Hookshot' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with one of his Wands.", "Is replaced with the 'Wand' Secret Combo.",
    "Triggers Screen Secrets when Link", "pounds it with one of his Hammers.", "Is replaced with the 'Hammer' Secret Combo.",
    "Triggers Screen Secrets when Link", "touches it with any weapon or projectile.", "Is replaced with the 'Any Weapon' Secret Combo.",
    "A push block pushed onto this flag","will cycle to the next combo in the list,","and lose the Push flag that was presumably on it.",
    "Makes a heart circle appear on screen", "when Link steps on it, and refills his magic.", "See also the Heart Circle-related Quest Rules.",
    "Makes a heart circle appear on screen when", "Link steps on it, and refills his life and magic.", "See also the Heart Circle-related Quest Rules.",
    "When stacked with a Trigger Combo Flag, it","prevents the triggered Secrets process from","changing all other flagged combos on-screen.",
    "Similar to 'Trigger->Self Only', but the","Secret Tile (16-31) flagged combos will still change.","(The 'Hit All Triggers->16-31' Screen Flag overrides this.)",
    "Enemies cannot enter or appear","on the flagged combo.","",
    "Enemies that don't fly or jump","cannot enter or appear on","the flagged combo.",
    //Script Flags follow.
    "", "", "",
    "", "", "",
    "", "", "",
    "", "", "",
    "", "", "",
    //Raft bounce flag! ^_^
    "When Link is rafting, and hits","this flag, he will be turned around.", ""
};

	
lwNone,
lwSword,
lwBeam,
lwBrang,
lwBomb,
lwSBomb,
lwLitBomb,
lwLitSBomb,
lwArrow,
lwFire,
lwWhistle,
lwBait,
lwWand,
lwMagic,
lwCatching,
lwWind,
lwRefMagic,
lwRefFireball,
lwRefRock, 
lwHammer,
lwHookshot, 
lwHSHandle, 
lwHSChain, 
lwSSparkle,
lwFSparkle, 
lwSmack, 
lwPhantom, 
lwCByrna,
lwRefBeam, 
lwStomp,
lwScript1, 
lwScript2, 
lwScript3, 
lwScript4,
lwScript5, 
lwScript6, 
lwScript7, 
lwScript8,
lwScript9, 
lwScript10, 
lwIce, 
