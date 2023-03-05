#include "cheatkeys.h"
#include "info.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "base/zsys.h"
#include "gui/use_size.h"
#include "zc/cheats.h"
#include <fmt/format.h>

const char *key_str[] =
{
    "(none)       ",              "a            ",              "b            ",              "c            ",
    "d            ",              "e            ",              "f            ",              "g            ",
    "h            ",              "i            ",              "j            ",              "k            ",
    "l            ",              "m            ",              "n            ",              "o            ",
    "p            ",              "q            ",              "r            ",              "s            ",
    "t            ",              "u            ",              "v            ",              "w            ",
    "x            ",              "y            ",              "z            ",              "0            ",
    "1            ",              "2            ",              "3            ",              "4            ",
    "5            ",              "6            ",              "7            ",              "8            ",
    "9            ",              "num 0        ",              "num 1        ",              "num 2        ",
    "num 3        ",              "num 4        ",              "num 5        ",              "num 6        ",
    "num 7        ",              "num 8        ",              "num 9        ",              "f1           ",
    "f2           ",              "f3           ",              "f4           ",              "f5           ",
    "f6           ",              "f7           ",              "f8           ",              "f9           ",
    "f10          ",              "f11          ",              "f12          ",              "esc          ",
    "~            ",              "-            ",              "=            ",              "backspace    ",
    "tab          ",              "{            ",              "}            ",              "enter        ",
    ":            ",              "quote        ",              "\\           ",              "\\ (2)       ",
    ",            ",              ".            ",              "/            ",              "space        ",
    "insert       ",              "delete       ",              "home         ",              "end          ",
    "page up      ",              "page down    ",              "left         ",              "right        ",
    "up           ",              "down         ",              "num /        ",              "num *        ",
    "num -        ",              "num +        ",              "num delete   ",              "num enter    ",
    "print screen ",              "pause        ",              "abnt c1      ",              "yen          ",
    "kana         ",              "convert      ",              "no convert   ",              "at           ",
    "circumflex   ",              ": (2)        ",              "kanji        ",              "num =        ",
    "back quote   ",              ";            ",              "command      ",              "unknown (0)  ",
    "unknown (1)  ",              "unknown (2)  ",              "unknown (3)  ",              "unknown (4)  ",
    "unknown (5)  ",              "unknown (6)  ",              "unknown (7)  ",              "left shift   ",
    "right shift  ",              "left control ",              "right control",              "alt          ",
    "alt gr       ",              "left win     ",              "right win    ",              "menu         ",
    "scroll lock  ",              "number lock  ",              "caps lock    ",      "MAX"
};

std::string get_keystr(int key)
{
	std::string str(key_str[key]);
	util::trimstr(str);
	return str;
}

CheatKeysDialog::CheatKeysDialog(bool* confirm):
	confirm_ptr(confirm)
{}

void CheatKeysDialog::load_labels()
{
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		if(!bindable_cheat((Cheat)q)) continue;
		for(size_t alt = 0; alt <= 1; ++alt)
		{
			lbls[q][alt]->setText(get_keystr(cheatkeys[q][alt]));
		}
	}
}

std::string cheatName(Cheat c)
{
	switch(c)
	{
		case Cheat::Life:
			return fmt::format("Refill {}", ZI.getCtrName(crLIFE));
		case Cheat::Magic:
			return fmt::format("Refill {}", ZI.getCtrName(crMAGIC));
		case Cheat::Rupies:
			return fmt::format("Refill {}", ZI.getCtrName(crMONEY));
		case Cheat::Bombs:
			return fmt::format("Refill {}", ZI.getCtrName(crBOMBS));
		case Cheat::Arrows:
			return fmt::format("Refill {}", ZI.getCtrName(crARROWS));
		case Cheat::Kill:
			return "Kill Enemies";
		case Cheat::Fast:
			return "Quick Movement";
		case Cheat::Clock:
			return "Invincibility";
		case Cheat::Walls:
			return "Noclip";
		case Cheat::Light:
			return "Toggle Darkness";
		case Cheat::IgnoreSideView:
			return "Ignore Sideview";
		case Cheat::MaxLife:
			return fmt::format("Max {}", ZI.getCtrName(crLIFE));
		case Cheat::MaxMagic:
			return fmt::format("Max {}", ZI.getCtrName(crMAGIC));
		case Cheat::MaxBombs:
			return fmt::format("Max {}", ZI.getCtrName(crBOMBS));
		case Cheat::PlayerData:
			return "Player Data";
		case Cheat::TrigSecrets:
			return "Trigger Secrets (T)";
		case Cheat::TrigSecretsPerm:
			return "Trigger Secrets (P)";
		case Cheat::ShowL0:
		case Cheat::ShowL1:
		case Cheat::ShowL2:
		case Cheat::ShowL3:
		case Cheat::ShowL4:
		case Cheat::ShowL5:
		case Cheat::ShowL6:
			return fmt::format("Toggle Layer {}", c-Cheat::ShowL0);
		case Cheat::ShowFFC:
			return "Toggle FFCs";
		case Cheat::ShowSprites:
			return "Toggle Sprites";
		case Cheat::ShowWalkability:
			return "Toggle Walkability";
		case Cheat::ShowEffects:
			return "Toggle Effect";
		case Cheat::ShowOverhead:
			return "Toggle Overheads";
		case Cheat::ShowPushblock:
			return "Toggle Pushblocks";
		case Cheat::ShowHitbox:
			return "Toggle Hitboxes";
		case Cheat::ShowFFCScripts:
			return "Toggle FFC Scripts";
		case Cheat::ShowInfoOpacity:
			return "Change Debug Opacity";
		default:
			return cheat_to_string(c);
	}
}

static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> CheatKeysDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	std::shared_ptr<GUI::Grid> cheatlist = Rows<9>();
	cheatlist->add(Label(text = "Lvl"));
	cheatlist->add(Label(text = "Name"));
	cheatlist->add(DummyWidget());
	cheatlist->add(Label(text = "Main Key", colSpan = 3));
	cheatlist->add(Label(text = "Alternate Key", colSpan = 3));
	
	std::map<int, std::vector<Cheat>> bindable_cheats;
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		Cheat c = (Cheat)q;
		if(!bindable_cheat(c)) continue;
		bindable_cheats[cheat_lvl(c)].push_back(c);
	}
	for(auto& pair : bindable_cheats)
	{
		for(Cheat c : pair.second)
		{
			cheatlist->add(Label(text = std::to_string(cheat_lvl(c))));
			cheatlist->add(Label(text = fmt::format("{} :",cheatName(c)), hAlign = 1.0));
			cheatlist->add(INFOBTN(cheat_help(c)));
			cheatlist->add(Button(text = "Bind", type = GUI::Button::type::BIND_KB, kb_ptr = &(cheatkeys[c][0]), onClick = message::RELOAD));
			cheatlist->add(Button(text = "Clear", type = GUI::Button::type::BIND_KB_CLEAR, kb_ptr = &(cheatkeys[c][0]), onClick = message::RELOAD));
			cheatlist->add(lbls[c][0] = Label(minwidth = 5_em, textAlign = 1));
			cheatlist->add(Button(text = "Bind", type = GUI::Button::type::BIND_KB, kb_ptr = &(cheatkeys[c][1]), onClick = message::RELOAD));
			cheatlist->add(Button(text = "Clear", type = GUI::Button::type::BIND_KB_CLEAR, kb_ptr = &(cheatkeys[c][1]), onClick = message::RELOAD));
			cheatlist->add(lbls[c][1] = Label(minwidth = 5_em, textAlign = 1));
		}
	}
	std::shared_ptr<GUI::Window> window = Window(
		title = "Cheat Hotkeys",
		onClose = message::CANCEL,
		Column(
			maxwidth = Size::pixels(zq_screen_w),
			ScrollingPane(
				ptr = &scroll_pos1,
				fitParent = true,
				minheight = 300_px,
				cheatlist),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	load_labels();
	return window;
}

bool CheatKeysDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD:
			load_labels();
			break;
		case message::OK:
			if(confirm_ptr)
				*confirm_ptr = true;
			return true;
		case message::CANCEL:
		default:
			if(confirm_ptr)
				*confirm_ptr = false;
			return true;
	}
	return false;
}
