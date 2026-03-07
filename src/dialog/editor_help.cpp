#include "editor_help.h"
#include "common.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "zinfo.h"

extern int32_t zq_screen_w, zq_screen_h;
static const string discord_url = "https://discord.gg/ddk2dk4guq";
static const string site_url = "https://zquestclassic.com";
static const string site_tutorials_url = "https://docs.zquestclassic.com/tutorials/";

#if !defined(ALLEGRO_MACOSX)
static const string ctrl_cmd = "Ctrl";
#else
static const string ctrl_cmd = "Cmd";
#endif

enum class help_page
{
	homepage,
	tile_combo_swatches,
	corner_swatches,
	terms,
	main_view_help,
};

namespace terms
{
	struct term
	{
		term(std::initializer_list<string> const& term_names, string const& disambiguation, string const& description)
			: names(term_names), disambiguation(disambiguation), description(description)
		{
			// term must have at least one name, and no empty names
			ASSERT(!names.empty());
			for (auto const& name : names)
				ASSERT(!name.empty());
		}
		term(std::initializer_list<string> const& names, string const& description)
			: term(names, "", description)
		{}
		term(string const& name, string const& disambiguation, string const& description)
			: term({name}, disambiguation, description)
		{}
		term(string const& name, string const& description)
			: term({name}, "", description)
		{}
		
		bool operator<(term const& other) const
		{
			return names[0] == other.names[0]
				? disambiguation < other.disambiguation
				: names[0] < other.names[0];
		}
		
		bool filter(string search, bool check_desc) const
		{
			util::lowerstr(search);
			for (auto name : names)
			{
				util::lowerstr(name);
				if (name.find(search) != string::npos)
					return true;
			}
			if (check_desc)
			{
				auto d = description;
				util::lowerstr(d);
				if (d.find(search) != string::npos)
					return true;
			}
			return false;
		}
		string get_header() const
		{
			string ret = fmt::format("'{}'", names[0]);
			for (size_t q = 1; q < names.size(); ++q)
				ret += fmt::format(", '{}'", names[q]);
			if (!disambiguation.empty())
				ret += fmt::format(" [{}]", disambiguation);
			return ret;
		}
		string get_body() const
		{
			return description;
		}
		char get_lead_char() const
		{
			return names[0][0];
		}
	private:
		vector<string> names;
		string disambiguation;
		string description;
	};
	
	static std::set<term> term_set;
	void init_terms()
	{
		term_set = {
			{ "Tile",
				"Raw graphics, in 16x16 pixel format"
				" (although some features use 8x8 quarter-tiles)."
				" Tiles are 4-bit (16-color) by default, allowing them"
				" to be used with different csets for simple palette-swapping."
				" They can also be set to 8-bit (256-color), giving more"
				" color depth at the cost of simple palette-swapping."
				" Tiles can be accessed via 'Quest->Graphics->Tiles'."
			},
			{ "Combo",
				"A 'combination' of settings."
				" They contain a tile + animation data, a 4-corner solidity mask,"
				" a variety of 'types' with different unique effects, Combo Triggers, and more."
				"\nCombos are placed directly on each layer of a screen, as well as"
				" being used by FFCs, 'Bottle Shop' room types, other combos, and more."
				"Combos can be accessed via the panel(s) on the right side of the main editor view,"
				" or via 'Quest->Graphics->Combos'."
			},
			{ "Combo Trigger",
				"A highly-configurable mechanism for causing custom things to happen."
				" Combo Triggers are set up on each Combo via the Combo Editor. They can"
				" be triggered by various conditions, such as a weapon hitting the combo,"
				" the player stepping on the combo, the last enemy on the screen dying, etc."
				" They can cause numerous different effects when triggered, such as changing"
				" the combo, displaying a message, teleporting the player, toggling a state,"
				" spawning an item or enemy, changing the current palette, playing a sound,"
				" changing the music, and more."
				"\n\nTriggers that do simple things are usually fairly easy to set up, "
				" while triggers that do incredibly complex things may require a lot of set up,"
				" such as having multiple different triggers on multiple different combos set up to"
				" interact with each other."
				"\nSome examples of triggers are available in the 'quests/examples' folder."
			},
			{ "Screen",
				"Rectangular (16x11 combo, 256x176 pixel) sections of the environment."
				" Screens group together to form Regions, and are stored together in Maps."
				" Screens with numbers >= 0x80 are 'special', and should not be used for"
				" normal gameplay. Screen 0x80 of each map is used for 'Cave/Item Cellar'"
				" rooms, while 0x81 is used for '3-Stair Warp' rooms and 'Passageway' rooms."
			},
			{ "Region",
				"User-defined rectangular groups of Screens."
				" Regions are defined for each Map in 'Quest->Map Settings'."
				" By default, every screen is in it's own 1x1 Region."
				" The player explores the game one Region at a time,"
				" with the camera freely following them around within the"
				" region's boundaries until they warp or scroll to a new Region."
			},
			{ "Map", "screens/regions",
				fmt::format("A 16x8 section of Screens. Each quest can store up to {} maps.", MAXMAPS)
			},
			{ {"Quest Rule", "QR"},
				"Togglable settings that change a variety of ways the engine functions."
				" These are accessible from 'Quest->Options', where they are"
				" listed by category, and a 'Search QRs' button allows"
				" listing them all with a substring-search (which can optionally search the"
				" help text of the QRs in addition to the names)."
				"\n\nQRs under the 'Compat' section generally re-enable old *buggy* behavior,"
				" and when developing a new quest it is recommended these be disabled. They"
				" may be necessary for old quests to retain their original functionality, however."
				" (Note: When creating a quest via 'File->New', the engine automatically"
				" disables Compat QRs for you)."
			},
			{ "MIDI",
				"Basic music compositions that can be included as music inside the"
				" quest file. Can be loaded via 'Quest->Audio->MIDIs', and used in"
				" Music entries in 'Quest->Audio->Music'."
			},
			{ "Music",
				"Each entry in the 'Quest->Audio->Music' list contains all the data necessary to play"
				" music- they can include an 'Enhanced Music' track (and associated looping / crossfade settings),"
				" in addition to a MIDI (which will be used as a backup if the Enhanced Music fails to load, or is blank)."
				" Only one music track of any kind can be playing at a time (excluding the crossfade feature of Enhanced Music)"
			},
			{ "Enhanced Music",
				"Music files of various extensions can be targetted as 'Enhanced Music', either"
				" in a 'Quest->Audio->Music' entry, or via scripts."
				" Enhanced Music is NOT saved as part of the quest file, instead being loaded"
				" at playtime- so, any enhanced music files you use will need to be provided"
				" to anyone playing your quest."
				"\nZC will try to load enhanced music files from a few specific places:"
				"\n([exe] indicates the folder 'zplayer.exe' is in."
				" [qst] indicates the folder your quest file is in."
				" [qstname] indicates the name (excluding extension) of your quest file.)"
				"\n1. [exe]/"
				"\n2. [qst]/"
				"\n3. [exe]/[qstname]_music/"
				"\n4. [exe]/music/"
				"\n5. [qst]/[qstname]_music/"
				"\n6. [qst]/music/"
			},
			{ {"SFX", "Sound Effects", "Sound FX"},
				"Sound Effects are small sounds that can be played by a variety of things in the engine-"
				" such as items being used, the player walking over a particular combo, an enemy taking"
				" damage, etc. Each individual sound effect can only be playing once at a time (trying"
				" to play a sound that is already playing will restart the sfx instead)- but multiple"
				" different sound effects can play simultaneously. Edit SFX at 'Quest->Audio->SFX Data'."
				"\nSFX can be in '.wav' or '.ogg' format. '.ogg' format sounds will be much smaller,"
				" so it is the recommended format. Loading too many large SFX can increase the "
				" time it takes to load your quest (so, avoid putting full music tracks inside sfx)."
			},
			{ "Header",
				"The header contains basic information about your quest; notably, the title and author"
				" are set here. You can also set a 'Quest Password' which will be required to open the"
				" quest, but this is not secure (there are easy ways to bypass it) so it only acts as"
				" a basic deterrant. Additionally, you can set a 'Quest Version' and 'Min Version',"
				" which when set up correctly can allow you to release updates to your quest that"
				" produce a clear error message if a user tries to load a save file from an older version"
				" (which is useful if your modifications to the quest require a fresh save file)."
				" The header is accessible under 'Quest->Options->Header'."
			},
			{ "Cheat",
				"A quest can define up to 4 levels of 'cheat code'. Based on the level of cheat entered,"
				" the player will gain access to a variety of cheat features, such as refilling their health,"
				" becoming invincible, walking through walls, giving themselves items, etc."
				" When testing your quest via Test Mode, all cheats are accessible without any code entry."
				" Set cheat codes in 'Quest->Options->Cheats'."
			},
			{ "Item",
				fmt::format("Collectable objects, which can either be 'equipment' that you keep (like"
				" Swords, Lanterns, Bows), or transient collectables that do something when collected"
				" (like money, ammo refills, or 'Item Bundles' that grant you multiple other items at once)."
				" A max of {} different items can exist in a quest."
				"\nOne item can be directly placed on each Screen (which can be set to only appear"
				" once all enemies are defeated, once secrets are triggered, or to be carried by one"
				" of the screen's enemies). Items can also be granted via special Room Types, by"
				" opening Chest type combos, spawned by Combo Triggers, given via SCCs, and more."
				" Edit items at 'Quest->Items'.", MAXITEMS)
			},
			{ { "Enemy", "Enemies", "NPC" },
				fmt::format("Foes for the player to fight, including bosses."
				" A max of {} different enemy varieties can exist in a quest. Up to 10 enemies"
				" can be placed on each screen, and additional enemies can be spawned via other means"
				" such as Combo Triggers. Enemies generally attack the player via Contact Damage and"
				" by firing EWeapons. Edit enemies at 'Quest->Enemies'.", MAXNPCS)
			},
			{ { "Weapon", "LWeapon", "EWeapon" },
				"Weapons are the main way of dealing damage. 'LWeapons' are weapons associated with the player,"
				" which damage enemies, and most of the time do not damage the player (bombs, fire, and reflected"
				" weapons can be exceptions). Usually these are spawned by the player, though they can be created"
				" via other means (such as any Weapon reflecting off a Mirror/Prism combo, or a Shooter combo acting"
				" as a turret). 'EWeapons' are weapons associated with enemies, which specifically damage the player."
				" These are generally spawned by enemies firing them, though they can be created via other means,"
				" such as a Shooter combo acting as a turret."
			},
			{ "Guy",
				"Guys are a special NPC type, who specifically are used alongside 'Room Types'."
				" Guys can be, ex, a shopkeeper of a shop type room."
			},
			{ { "String", "Message String" },
				"Strings are sets of formatted text to display to the player. Used with a Room Type + Guy,"
				" a String sets what the Guy will say to the player. Used with Signpost combos, strings"
				" determine the text that will be read. Strings include full formatting; a background,"
				" font options, text shadow options, an optional Portrait, margins, and more."
				" Additionally, strings can contain SCCs, which can be used to cause a wide variety of effects."
				" Strings are edited via the String Editor, at 'Quest->Strings'."
			},
			{ { "String Control Code", "SCC" },
				"SCCs are special codes included in Strings, which can cause a variety of effects"
				" to occur- such as branching a conversation to a different string based on a"
				" condition, prompting the player to select an option, granting the player an item,"
				" taking an item from the player, altering the text formatting mid-string,"
				" playing a sound or music, setting a state of the screen, and more."
			},
			{ "Init Data",
				"A series of settings that apply at the start of new save files. Includes the starting"
				" inventory, physics constants like the strength of Gravity, other misc constants like"
				" the style of darkness (dithering/transparency settings) in dark rooms, enemy flicker"
				" settings, etc. Accessible via 'Quest->Init Data'."
			},
			{ { "Palette (Main)", "Main Palette" },
				"The main colors of the quest. Only 256 colors can ever be displayed on-screen"
				" at once, and these colors default to the Main palette, divided into 16 groups of 16 colors"
				" called 'CSets'. Certain sections of the main palette will be overwritten with other palettes"
				" throughout the game; see 'Palette (Level)' / 'Palette (Sprite)' for more details on this."
				" The Main Palette is edited at 'Quest->Graphics->Palettes->Main'."
			},
			{ { "Palette (Level)", "Level Palette" },
				"Level-specific colors of the quest. Up to 512 level palettes can be created,"
				" and by default the level palette of the current DMap will be loaded at any given time."
				" Combo Triggers can also change the current level palette. The csets 2, 3, 4, and 9 (by"
				" default; QRs can add additional csets) of the level palette will replace the same-numbered"
				" csets of the main palette while the level palette is active."
				" The Level Palettes are edited at 'Quest->Graphics->Palettes->Levels'."
			},
			{ { "Palette (Sprite)", "Sprite Palette" },
				fmt::format("Sprite Palettes are used in specific circumstances; such as swapping CSet 6"
				" based on the current highest-level '{}' item the player owns, or swapping CSet 14 based on"
				" an enemy with a 'Boss Palette' set being on-screen. The Level Palettes are edited at"
				" 'Quest->Graphics->Palettes->Sprites'.", ZI.getItemClassName(itype_ring))
			},
			{ "CSet",
				"16-color sections of the 256-color total palette. As tiles are usually 4-bit / 16-color,"
				" they can be displayed in different csets to have a 'palette-swap' effect (though 8-bit"
				" / 256-color tiles also exist, and ignore csets entirely). Certain CSets will change as"
				" the game progresses, such as when a new level is entered; see 'Palette (Level)'"
				" / 'Palette (Sprite)' for more details."
			},
			{ { "Combo Flag", "Placed Flag", "Inherent Flag" },
				fmt::format("Combo Flags are a type of flag that can be placed on the screen ('Placed Flags'),"
				" or set as inherent to a combo ('Inherent Flags'). These flags can have a variety of effects,"
				" such as responding to Screen Secrets (ex. a '{}' flag will trigger the screen's secrets"
				" when hit by a '{}', and change the combo at that position to the screen's 'Bomb' Secret Combo),"
				" or allowing a combo to be pushed as a pushblock (ex. '{}', '{}', '{}')."
				" Inherent Flags are set in the combo editor, while Placed Flags are placed using the Flag tool"
				" at the bottom-left of the main editor view.",
				ZI.getMapFlagName(mfBOMB), ZI.getWeapName(wBomb),
				ZI.getMapFlagName(mfPUSH4INS), ZI.getMapFlagName(mfPUSHLR), ZI.getMapFlagName(mfPUSHUDINS))
			},
			{ { "Freeform Combo", "FFC" },
				fmt::format("Special types of combos that can be placed off the standard combo grid,"
				" which can move and do a variety of things. These can be edited for the current"
				" screen via 'Screen->Freeform Combos', or via the right-click menu of the main"
				" screen view. A max of {} FFCs can exist per screen, and only FFCs from the"
				" base (layer 0) of each screen exist- though each FFC can have it's own 'Layer' set,"
				" to change their draw order.", MAXFFCS)
			},
			{ { "DMap", "Dungeon Map" },
				"DMaps are sets of data that define an area (such as a floor of a dungeon, an overworld,"
				" or a cave system). Each DMap applies to either an 8x8 or 16x8 section of a single Map."
				" DMaps can affect things such as what music plays, what 'Level' an area is part of,"
				" what Level Palette is loaded for an area, and several other misc effects."
				" DMaps are editable from 'Quest->DMaps'."
			},
			{ "Level",
				"Each 'Level' has several associated values, such as Level States and Level Items,"
				" which generally take effect when the player is on a DMap set to the matching Level number."
				" Multiple DMaps that share the same level number will share these states (useful for multiple"
				" floors of the same dungeon)."
			},
			{ "Level State",
				"A set of 32 on/off states for each Level. Some combo types, as well as Combo Triggers,"
				" can change these states, and react to them. Useful for changes that affect a large area"
				" across multiple screens."
			},
			{ "Global State",
				"A set of 256 states similar to Level States. Some combo types, as well as Combo Triggers,"
				" can change these states, and react to them. Useful for changes that affect a large area"
				" across multiple screens. Unlike Level States, these states are not just on/off; they can also"
				" optionally be *timed*, allowing for timed switches across multiple screens."
			},
			{ "Level Item",
				fmt::format("A set of flags that can be on/off for each level. Most often these"
				" are toggled on by collecting an item, but despite the name, this is not a"
				" requirement- notable exception being the 'Boss Defeated' Level Item, which is"
				" set on when the enemies are killed on a screen marked with the Dungeon Boss flag."
				" Standard level items include '{}' to show dungeon layouts, '{}' to show treasure"
				" locations, '{}' to unlock special doors/chests, etc. Several slots are blank and customizable,"
				" able to be renamed using ZInfo and used for whatever purpose you like."
				, ZI.getLevelItemName(li_map), ZI.getLevelItemName(li_compass), ZI.getLevelItemName(li_boss_key))
			},
			{ "ZInfo",
				"'Quest->ZInfo' is a menu that allows customizing the names (and descriptions) of"
				" various things in the engine, such as item types, weapon types, level items, etc."
				" This is mostly an editor-only effect, though if players access the Level 3 cheat menu"
				" they will see the changes there. The primary purpose of this is to help organize"
				" your quest, as these changes will appear in many dropdowns and dialogs across"
				" the editor."
			},
			{ "Subscreen",
				"Subscreens represent in-game menus; the Passive Subscreen displays at the top of the screen"
				" during gameplay, while the Overlay Subscreen displays over the entire screen at all times."
				" The Active Subscreen and Map Subscreen pause the game when opened with the Start and Map"
				" buttons respectively (though the Map button may bring up a default engine map or do nothing"
				" instead, if no Map Subscreen is assigned for the current dmap)."
				" Generally the Passive + Overlay subscreens show your currently equipped equipment and"
				" meters/counters such as your life/money/ammo; while Active Subscreens show your full"
				" inventory, allowing you to change equipment, and Map Subscreens are intended to show"
				" an area-specific map. Editable via 'Quest->Misc Data->Subscreens'."
			},
			{ "Item Drop Set",
				"Collections of items paired with weighted odds, used to determine random item drops"
				" from enemies, bushes, etc. Editable via 'Quest->Misc Data->Item Drop Sets'."
			},
			{ "Script",
				"Scripts are a powerful and complex tool, allowing code written in the ZScript language"
				" to run during gameplay and do a large variety of things; from custom bosses to"
				" custom items to custom dungeon gimmicks, scripts can do nearly anything if you"
				" have the know-how to write the script for it. Scripts are compiled from the 'ZScript' menu."
			},
			{ "Hotkey",
				"Hotkeys are a great way to speed up your use of the editor. Many dialogs have"
				" dialog-specific hotkeys, which can often be found detailed in a '?' button in the dialog."
				" The main editor view has a wide variety of hotkeys, which can be summarized in"
				" 'Etc->List Hotkeys' and fully customized in 'Etc->Hotkeys'."
			},
			{ "Combo Alias",
				"Aliases are groups of combos set to specific layers. An alias could, for instance,"
				" be a 2x2 collection of Tree Trunk combos on layer 0 and Tree Leaves combos"
				" on layer 4. Painting such an alias on the screen would place all 8 of those"
				" combos in one click (2x2 on layer 0, 2x2 on layer 4)."
			},
			{ "Combo Pool",
				"Combo Pools are weighted groups of combos. When painting a combo pool on the screen,"
				" for each combo position you paint over, a random combo from the pool is selected"
				" according to the specified weights. This is especially useful for random detailing,"
				" such as mossy or cracked walls, or grass / bush variations."
			},
			{ { "Combo (Auto)", "Auto Combos" },
				"Auto Combos are precisely-configured tools, designed to make painting specific things"
				" onto the screen easier. For instance, a 'Flat Mountain' style auto-combo"
				" allows you to simply paint a mountain on the screen, automatically handling edges"
				" of the mountain appropriately (given a classic 3x2 combo mountain setup)."
				" A 'Pancake Mountain' or 'Complex Mountain' would allow more complicated"
				" mountains to be drawn, although require more complicated setups."
				" A 'Basic Relational' auto combo allows simple water / pits / grass patch"
				" setups to be easily painted. Each type has it's own specific use cases."
			},
		};
	}
}

help_page cur_help;
std::map<help_page, int> sub_page;

void call_help_dialog()
{
	terms::init_terms(); // re-init each launch, to handle ZInfo updates and such
	EditorHelpDialog().show();
}

#define TRAVERSE_BTN(str, target_help) \
Button( \
	text = str, \
	fitParent = true, \
	onPressFunc = [=]() \
	{ \
		cur_help = target_help; \
		refresh_dlg(); \
	} \
)

enum class focus
{
	nil, lpage, rpage, search,
};
static focus cur_focus = focus::nil;
static string term_search;
static bool search_description;
std::shared_ptr<GUI::Widget> EditorHelpDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	const Size body_width = 450_px;
	
	string dlg_title;
	size_t page_count = 1;
	auto body = Column(DummyWidget(width = body_width, height = 0_px));
	bool add_search = false;
	std::shared_ptr<GUI::Grid> grid;
	
	auto& pg = sub_page.try_emplace(cur_help, 0).first->second;
	string page_prefix = "";
	switch (cur_help)
	{
		case help_page::homepage:
		{
			dlg_title = "Editor Help";
			page_count = 3;
			string txt;
			switch (pg)
			{
				case 0:
				{
					txt = "Welcome to ZQuest Classic!"
						"\nThis dialog is designed to help with a variety of general topics."
						" Use the < / > buttons to change pages."
						"\nSpecific help can be found in many '?' buttons throughout the editor,"
						" giving information about specific fields, sections, or dialogs."
						" Be sure not to ignore them if you need help, and feel free to let"
						" us know if any can be improved.";
					break;
				}
				case 1:
				{
					txt = "This is still WIP, and many topics are not covered; additional help"
						" can be found in the Discord server, and more in-depth guides can be found"
						" in the Tutorials section of the website."
						"\nIf you have any ideas for things that should be added here,"
						" (or any bug reports/feature requests)"
						" please feel free to bring them to the dev discord.";
					grid = Row(
						Button(text = "Website",
							onPressFunc = [&]()
							{
								util::open_web_link(site_url);
							}
						),
						Button(text = "Discord",
							onPressFunc = [&]()
							{
								util::open_web_link(discord_url);
							}
						),
						Button(text = "Web Tutorials",
							onPressFunc = [&]()
							{
								util::open_web_link(site_tutorials_url);
							}
						)
					);
					break;
				}
				case 2:
				{
					grid = Columns<2>(
						Label(text = "General Help:"),
						Columns<5>(
							TRAVERSE_BTN("Terms / Definitions", help_page::terms),
							TRAVERSE_BTN("Main Editor View", help_page::main_view_help)
						),
						Label(text = "GUI Help:"),
						Columns<5>(
							TRAVERSE_BTN("Tile && Combo Swatches", help_page::tile_combo_swatches),
							TRAVERSE_BTN("Corner Swatches", help_page::corner_swatches)
						)
					);
					break;
				}
			}
			if (!txt.empty())
				body->add(Label(maxwidth = body_width, textAlign = 1, text = txt));
			if (grid)
				body->add(grid);
			break;
		}
		case help_page::tile_combo_swatches:
		{
			dlg_title = "Editor Help: Tile/Combo Swatches";
			page_count = 3;
			string txt;
			grid = Column();
			//!TODO add multi-tile/combo swatches (ex. 2x2)
			switch (pg)
			{
				case 0:
				case 1:
				{
					txt = fmt::format("{0} Swatches are a GUI element that"
						" allow you to select a {0} (and often also a CSet) for some purpose."
						"\nNotable behaviors:"
						"\nBy default, clicking a {0} Swatch will open the {0} Pages"
						" to the page of whatever {0} was selected by the swatch previously."
						"\nBy holding '{1}', clicking a {0} Swatch will instead open the {0} Pages"
						" to whatever page was previously open (even if it was last open from a"
						" different dialog entirely), making it easy to set up many similar tiles in a row.",
						pg ? "Combo" : "Tile", ctrl_cmd);
					
					if (pg) // combo
					{
						txt += "\nBy holding 'Alt', clicking a Combo Swatch will place"
							" the current cursor combo on the swatch (same as placing a combo on a screen)."
							" Additionally, holding 'Shift+Alt' will place only the cset"
							" (rather than combo id + cset with just 'Alt')";
						grid->add(SelComboSwatch(showvals = false));
						grid->add(SelComboSwatch(showvals = true));
					}
					else
					{
						grid->add(SelTileSwatch(showvals = false));
						grid->add(SelTileSwatch(showvals = true));
					}
					break;
				}
				case 2:
				{
					txt = "Some Tile Swatches have a small square overlaid on them."
						"These allow selecting a quarter-tile, rather than a whole tile."
						"\nYou can use them just like a normal Tile Swatch, except that"
						" right-clicking them moves the small square to choose a corner of the tile.";
					grid->add(SelTileSwatch(mini = true, showvals = false));
					grid->add(SelTileSwatch(mini = true, showvals = true));
					break;
				}
			}
			
			body->add(Label(maxwidth = body_width, textAlign = 1, text = txt));
			body->add(grid);
			break;
		}
		case help_page::corner_swatches:
		{
			dlg_title = "Editor Help: Corner Swatches";
			page_count = 1;
			body->add(Label(maxwidth = body_width, textAlign = 1,
				text = fmt::format("Corner Swatches are used to select 4 corners"
				" of something. They basically act as 4 checkboxes, but"
				" specifically shaped as a 2x2 square. Corner Swatches"
				" can come in any color."
				"\nClicking on a corner toggles just that corner, by default."
				"\n{}+Clicking anywhere on the swatch toggles all 4 corners."
				"\nShift+Clicking on a corner will toggle that corner,"
				" then fill the entire square with that color.",
				ctrl_cmd)));
			body->add(Columns<2>(
				CornerSwatch(val = 0x0, color = vc(12)),
				CornerSwatch(val = 0xF, color = vc(11)),
				CornerSwatch(val = 0x3, color = vc(10)),
				CornerSwatch(val = 0xC, color = vc(9))
			));
			break;
		}
		case help_page::terms:
		{
			dlg_title = "Editor Help: Terms";
			page_count = 1;
			const Size max_height = 30_em;
			auto temp_label = Label(maxwidth = body_width - 10_px);
			add_search = true;
			
			grid = Rows<2>(bottomPadding = 10_px, rightPadding = 10_px);
			
			char start = 0, end = 0;
			size_t h = 0;
			bool found = false;
			for (auto const& term : terms::term_set)
			{
				if (!term_search.empty())
					if (!term.filter(term_search, search_description))
						continue;
				auto txt = fmt::format("{}:\n{}", term.get_header(), term.get_body());
				temp_label->setText(txt);
				h += temp_label->getHeight();
				if (h > max_height)
				{
					++page_count;
					h = temp_label->getHeight();
				}
				
				if (page_count == pg+1)
				{
					char c = toupper(term.get_lead_char());
					if (!start)
						start = c;
					end = c;
					found = true;
					grid->add(Label(text = " - ", vAlign = 0.0));
					grid->add(Label(maxwidth = body_width - 10_px, textAlign = 0, hAlign = 0.0, text = txt));
				}
			}
			if (found)
			{
				page_prefix = fmt::format("{} - {}", start, end);
				body->add(grid);
			}
			else if (pg >= page_count)
			{
				// invalid page ... swap to page 0 and reload dialog again
				pg = 0;
				refresh_dlg();
				return DummyWidget();
			}
			else
			{
				body->add(Label(maxwidth = body_width, textAlign = 1,
					text = "[No Matching Terms Found]"));
			}
			break;
		}
		case help_page::main_view_help:
		{
			dlg_title = "Editor Help: Main View Help";
			page_count = 7;
			string txt;
			switch (pg)
			{
				case 0:
				{
					txt = "In the main view of the editor, you are presented with a screen"
						" editing area, a section of combo columns to pick combos from,"
						" and a variety of sections of tools at the bottom."
						"\n\nThe main view can be customized; it has two main display modes,"
						" 'Expanded' and 'Compact', togglable via a button at the top-right."
						" Just next to this button is a pair of '+' and '-', which allow zooming"
						" the screen editing area out to multiple screens at once."
						"\n\nA '<|>' / '>|<' button above the combo columns allows changing the"
						" number of columns displayed. Favorite Commands and Favorite Combos sections"
						" also have some buttons which change their layout; the '?' buttons in those"
						" sections explain in more detail."
						"\n\nAnd finally, in Compact view specifically, there is a 'SWP' button in"
						" the lower-left allowing you to change between seeing all the tools in that"
						" panel at once, or showing just some of them (but they are larger) with"
						" arrow buttons to cycle through them.";
					break;
				}
				case 1:
				{
					txt = fmt::format("To edit screens, you select combos from the combo columns on"
						" the right, and paint them onto the screen editor area to create your"
						" environments. Combinations of 'Shift', '{0}', and 'Alt' being held while"
						" clicking the screen area have different effects, such as only painting"
						" the CSet while holding Shift, floodfilling while holding {0}, and"
						" dropper-tool style selecting the clicked combo when holding Alt."
						" Combinations of these work as expected (Shift+Alt selects the clicked CSet,"
						" Shift+Ctrl floodfills the CSet).", ctrl_cmd);
					break;
				}
				case 2:
				{
					txt = "Right-clicking on the screen area gives a number of options, including settings"
						" that change how the floodfill works, the ability to directly place FFCs,"
						" and more; most of these options should be fairly self-explanatory."
						"\n\nBelow the screen is a set of buttons labeled 0-7, with checkboxes next to them."
						" These are the Layer buttons. Any layer checkbox that is unchecked, indicates that"
						" layer is hidden (invisible) in the editor currently. Whichever layer button"
						" is currently 'pressed in' is the layer that is currently being edited;"
						" clicking a different layer button will change the layer, if that layer exists"
						" to edit (be sure the layer is set up for that screen via 'Screen->Layers' first!)";
					break;
				}
				case 3:
				{
					txt = "At the lower-left is a view of the current map. Right-clicking"
						" this anywhere will enlargen the map, making it easier to view."
						" Left-clicking on the map will move the screen editing window to the"
						" screen you clicked. Which map is being edited can be changed"
						" using hotkeys, which are rebindable (see 'Etc->List Hotkeys' /"
						" 'Etc->Hotkeys'). The total number of maps can be changed in"
						" 'Quest->Options'."
						"\n\nAbove this is a set of buttons labeled with pairs of numbers,"
						" such as '1:00', one of which is 'pressed in'. These are 'Screen"
						" Bookmarks', and clicking on one will make that the active bookmark,"
						" and change the current map+screen being edited to the bookmarked"
						" map+screen (ex. '1:00' = Map 1, Screen 0x00)."
						" Right-clicking on a bookmark (other than the currently active one)"
						" will reset it back to '1:00'.";
					break;
				}
				case 4:
				{
					txt = "At the upper-right is a button displaying the current 'Drawing Mode'"
						" ('Normal' by default). Clicking this button cycles through the available"
						" modes in order, though right-clicking it pops up a list of them all."
						" Each mode will change the combo columns on the right, allowing the use"
						" of Combo Aliases, Combo Pools, and Auto Combos; effectively, special"
						" types of brush to paint combos onto the screen with."
						"\n\nBelow the combo columns is a section of 'Favorite Combos'."
						" This section can hold a large number of Combos, Combo Aliases,"
						" Combo Pools, and/or Auto Combos which can be quickly accessed at any time"
						" (without needing to find the right spot in the combo columns)."
						" It has a '?' button to explain itself in more detail.";
					break;
				}
				case 5:
				{
					txt = "At the lower-middle, to the left of the Favorite Combos,"
						" is a section of Favorite Commands. These are bindable shortcuts"
						" that can be bound to a variety of editor actions, readily available"
						" from the main view at a single click. A '?' button next to them explains"
						" in more detail.";
					break;
				}
				case 6:
				{
					txt = fmt::format("Finally, we come to the lower-left panel. A number of icons"
						" are present here. A symbol of a gem representing an item allows you"
						" to place an item for the current Screen. A symbol of stairs allows"
						" placing the Stairs Secret marker. A green square allows moving the"
						" Screen's Arrival Square (though this will be greyed out / disabled"
						" in modern quests that don't use Arrival Squares). A waving flag"
						" allows you to select and paint Combo Flags on the screen (with modifier"
						" keys similar to those for placing combos; {0}+Click floodfills, while"
						" Shift+Click erases flags. Mousewheel changes the flag being placed)."
						" 4 animated blue squares (lettered A, B, C, D) allow placing the 4"
						" 'Warp Return' squares, where you spawn after warping. And finally,"
						" a black not-quite-rectangle shape allows setting the Screen's Enemies.",
						ctrl_cmd);
					break;
				}
			}
			body->add(Label(maxwidth = body_width, textAlign = 1, text = txt));
			break;
		}
		default:
		{
			body->add(Label(maxwidth = body_width, textAlign = 1,
				text = "ERROR: Unimplemented Help Page"));
			break;
		}
	}
	
	if (add_search && cur_focus == focus::nil)
		cur_focus = focus::search;
	
	auto search_label = Label(text = "Search:", visible = add_search);
	auto search_field = TextField(
		leftPadding = 0_px, colSpan = 2,
		maxLength = 512, forceFitW = true, vPadding = 0_px,
		visible = add_search, focused = cur_focus == focus::search,
		text = term_search, 
		onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
		{
			term_search = str;
			cur_focus = focus::search;
			refresh_dlg();
		}
	);
	auto search_cb = Checkbox(text = "Search Description",
		visible = add_search, topPadding = 0_px, colSpan = 2,
		hAlign = 0.0, checked = search_description,
		onToggleFunc = [&](bool state)
		{
			search_description = state;
			cur_focus = focus::search;
			refresh_dlg();
		}
	);
	
	window = Window(
		title = dlg_title,
		onClose = message::CLOSE,
		Column(
			Row(
				Rows<3>(spacing = 0_px,
					DummyWidget(padding = 0_px, height = 0_px),
					DummyWidget(padding = 0_px, height = 0_px, minwidth = 75_px),
					DummyWidget(padding = 0_px, height = 0_px, minwidth = 75_px),
				
					DummyWidget(),
					Label(colSpan = 2, textAlign = 1,
						text = fmt::format("{}\nPage {} / {}",
						page_prefix, pg + 1, page_count)),
					
					DummyWidget(),
					Button(fitParent = true,
						padding = 1_px,
						type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_LEFT,
						disabled = pg < 1,
						focused = cur_focus == focus::lpage,
						onPressFunc = [&]()
						{
							pg -= 1;
							cur_focus = focus::lpage,
							refresh_dlg();
						}
					),
					Button(fitParent = true,
						padding = 1_px,
						type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_RIGHT,
						disabled = pg + 1 >= page_count,
						focused = cur_focus == focus::rpage,
						onPressFunc = [&]()
						{
							pg += 1;
							cur_focus = focus::rpage;
							refresh_dlg();
						}
					),
					
					DummyWidget(),
					Button(fitParent = true,
						padding = 1_px,
						text = "Back",
						disabled = cur_help == help_page::homepage,
						onPressFunc = [&]()
						{
							cur_help = help_page::homepage;
							refresh_dlg();
						}
					),
					Button(fitParent = true,
						padding = 1_px,
						text = "Close",
						onClick = message::CLOSE
					),
					
					search_label,
					search_field,
					
					DummyWidget(),
					search_cb
				),
				body
			)
		)
	);
	
	cur_focus = focus::nil;
	
	return window;
}

bool EditorHelpDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::CLOSE:
			return true;
	}
	return false;
}
