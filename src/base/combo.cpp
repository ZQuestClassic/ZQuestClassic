#include "combo.h"
#include "general.h"
#include "mapscr.h"
#include "zinfo.h"
#include "new_subscr.h"
#include "qst.h"

extern const char* icounter_str2[-(sscMIN + 1)];
extern char *item_string[MAXITEMS];
extern char* guy_string[eMAXGUYS];
extern char* sfx_string[WAV_COUNT];
extern item_drop_object item_drop_sets[MAXITEMDROPSETS];
extern std::map<int32_t, script_slot_data > genericmap;


std::vector<newcombo> combobuf;

static string get_sign(combo_trigger const& trig, size_t flag_le, size_t flag_ge)
{
	if (trig.trigger_flags.all({ flag_ge,flag_le }))
		return "==";
	else if (trig.trigger_flags.get(flag_ge))
		return ">=";
	else if (trig.trigger_flags.get(flag_le))
		return "<=";
	return "";
}

std::string combo_trigger::summarize(newcombo const& cmb) const
{
#define TRIM_TRAILING_CHARS(val) val = val.substr(0, val.find_last_not_of(", \n")+1)
	std::ostringstream output, wpn_causes, wpn_effects, causes, conditions, effects, fail_effects;
	string indent = "   ";

	// lweapon causes
	{
		vector<std::string> lwpns;
		if (trigger_flags.get(TRIGFLAG_SWORD))
			lwpns.emplace_back("Sword");
		if (trigger_flags.get(TRIGFLAG_SWORDBEAM))
			lwpns.emplace_back("Sword Beam");
		if (trigger_flags.get(TRIGFLAG_BRANG))
			lwpns.emplace_back("Boomerang");
		if (trigger_flags.get(TRIGFLAG_BOMB))
			lwpns.emplace_back("Bomb Boom");
		if (trigger_flags.get(TRIGFLAG_SBOMB))
			lwpns.emplace_back("Super Bomb Boom");
		if (trigger_flags.get(TRIGFLAG_LITBOMB))
			lwpns.emplace_back("Placed Bomb");
		if (trigger_flags.get(TRIGFLAG_LITSBOMB))
			lwpns.emplace_back("Placed Super Bomb");
		if (trigger_flags.get(TRIGFLAG_ARROW))
			lwpns.emplace_back("Arrow");
		if (trigger_flags.get(TRIGFLAG_FIRE))
			lwpns.emplace_back("Fire");
		if (trigger_flags.get(TRIGFLAG_WHISTLE))
			lwpns.emplace_back("Whistle");
		if (trigger_flags.get(TRIGFLAG_BAIT))
			lwpns.emplace_back("Bait");
		if (trigger_flags.get(TRIGFLAG_WAND))
			lwpns.emplace_back("Wand");
		if (trigger_flags.get(TRIGFLAG_MAGIC))
			lwpns.emplace_back("Magic");
		if (trigger_flags.get(TRIGFLAG_WIND))
			lwpns.emplace_back("Wind");
		if (trigger_flags.get(TRIGFLAG_REFMAGIC))
			lwpns.emplace_back("Refl. Magic");
		if (trigger_flags.get(TRIGFLAG_REFFIREBALL))
			lwpns.emplace_back("Refl. Fireball");
		if (trigger_flags.get(TRIGFLAG_REFROCK))
			lwpns.emplace_back("Refl. Rock");
		if (trigger_flags.get(TRIGFLAG_LWREFARROW))
			lwpns.emplace_back("Refl. Arrow");
		if (trigger_flags.get(TRIGFLAG_LWREFFIRE))
			lwpns.emplace_back("Refl. Fire");
		if (trigger_flags.get(TRIGFLAG_LWREFFIRE2))
			lwpns.emplace_back("Refl. Fire 2");
		if (trigger_flags.get(TRIGFLAG_HAMMER))
			lwpns.emplace_back("Hammer");
		if (trigger_flags.get(TRIGFLAG_HOOKSHOT))
			lwpns.emplace_back("Hookshot");
		if (trigger_flags.get(TRIGFLAG_SPARKLE))
			lwpns.emplace_back("Sparkle");
		if (trigger_flags.get(TRIGFLAG_BYRNA))
			lwpns.emplace_back("Byrna");
		if (trigger_flags.get(TRIGFLAG_REFBEAM))
			lwpns.emplace_back("Refl. Beam");
		if (trigger_flags.get(TRIGFLAG_STOMP))
			lwpns.emplace_back("Stomp");
		if (trigger_flags.get(TRIGFLAG_THROWN))
			lwpns.emplace_back("Thrown");
		if (trigger_flags.get(TRIGFLAG_QUAKESTUN))
			lwpns.emplace_back("Quake Hammer");
		if (trigger_flags.get(TRIGFLAG_SQUAKESTUN))
			lwpns.emplace_back("S. Quake Hammer");
		if (trigger_flags.get(TRIGFLAG_SCRIPT01))
			lwpns.emplace_back("Custom Weapon 1");
		if (trigger_flags.get(TRIGFLAG_SCRIPT02))
			lwpns.emplace_back("Custom Weapon 2");
		if (trigger_flags.get(TRIGFLAG_SCRIPT03))
			lwpns.emplace_back("Custom Weapon 3");
		if (trigger_flags.get(TRIGFLAG_SCRIPT04))
			lwpns.emplace_back("Custom Weapon 4");
		if (trigger_flags.get(TRIGFLAG_SCRIPT05))
			lwpns.emplace_back("Custom Weapon 5");
		if (trigger_flags.get(TRIGFLAG_SCRIPT06))
			lwpns.emplace_back("Custom Weapon 6");
		if (trigger_flags.get(TRIGFLAG_SCRIPT07))
			lwpns.emplace_back("Custom Weapon 7");
		if (trigger_flags.get(TRIGFLAG_SCRIPT08))
			lwpns.emplace_back("Custom Weapon 8");
		if (trigger_flags.get(TRIGFLAG_SCRIPT09))
			lwpns.emplace_back("Custom Weapon 9");
		if (trigger_flags.get(TRIGFLAG_SCRIPT10))
			lwpns.emplace_back("Custom Weapon 10");

		if (!lwpns.empty())
		{
			wpn_causes << indent;
			bool invert = trigger_flags.get(TRIGFLAG_INVERTMINMAX);
			int lv = zc_max(1, triggerlevel);
			if (invert)
				wpn_causes << fmt::format("LWeapons (level <= {}): ", lv);
			else if (lv > 1)
				wpn_causes << fmt::format("LWeapons (level >= {}): ", lv);
			else wpn_causes << "LWeapons: ";

			for (auto const& str : lwpns)
				wpn_causes << str << ", ";
			wpn_causes.seekp(int(wpn_causes.tellp()) - 2);
			wpn_causes << "\n";
		}
	}

	// eweapon causes
	{
		vector<std::string> ewpns;
		if (trigger_flags.get(TRIGFLAG_EWFIREBALL))
			ewpns.emplace_back("Fireball");
		if (trigger_flags.get(TRIGFLAG_EWARROW))
			ewpns.emplace_back("Arrow");
		if (trigger_flags.get(TRIGFLAG_EWBRANG))
			ewpns.emplace_back("Boomerang");
		if (trigger_flags.get(TRIGFLAG_EWSWORD))
			ewpns.emplace_back("Sword");
		if (trigger_flags.get(TRIGFLAG_EWROCK))
			ewpns.emplace_back("Rock");
		if (trigger_flags.get(TRIGFLAG_EWMAGIC))
			ewpns.emplace_back("Magic");
		if (trigger_flags.get(TRIGFLAG_EWBBLAST))
			ewpns.emplace_back("Bomb Blast");
		if (trigger_flags.get(TRIGFLAG_EWSBBLAST))
			ewpns.emplace_back("SBomb Blast");
		if (trigger_flags.get(TRIGFLAG_EWLITBOMB))
			ewpns.emplace_back("Lit Bomb");
		if (trigger_flags.get(TRIGFLAG_EWLITSBOMB))
			ewpns.emplace_back("Lit SBomb");
		if (trigger_flags.get(TRIGFLAG_EWFIRETRAIL))
			ewpns.emplace_back("Fire Trail");
		if (trigger_flags.get(TRIGFLAG_EWFLAME))
			ewpns.emplace_back("Flame");
		if (trigger_flags.get(TRIGFLAG_EWWIND))
			ewpns.emplace_back("Wind");
		if (trigger_flags.get(TRIGFLAG_EWFLAME2))
			ewpns.emplace_back("Flame 2");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT01))
			ewpns.emplace_back("Custom Weapon 1");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT02))
			ewpns.emplace_back("Custom Weapon 2");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT03))
			ewpns.emplace_back("Custom Weapon 3");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT04))
			ewpns.emplace_back("Custom Weapon 4");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT05))
			ewpns.emplace_back("Custom Weapon 5");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT06))
			ewpns.emplace_back("Custom Weapon 6");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT07))
			ewpns.emplace_back("Custom Weapon 7");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT08))
			ewpns.emplace_back("Custom Weapon 8");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT09))
			ewpns.emplace_back("Custom Weapon 9");
		if (trigger_flags.get(TRIGFLAG_EWSCRIPT10))
			ewpns.emplace_back("Custom Weapon 10");

		if (!ewpns.empty())
		{
			wpn_causes << indent << "EWeapons: ";

			for (auto const& str : ewpns)
				wpn_causes << str << ", ";
			wpn_causes.seekp(int(wpn_causes.tellp()) - 2);
			wpn_causes << "\n";
		}
	}

	// other weapon causes
	{
		vector<std::string> wpns;
		if (trigger_flags.get(TRIGFLAG_ANYFIRE))
			wpns.emplace_back("Any Fire");
		if (trigger_flags.get(TRIGFLAG_STRONGFIRE))
			wpns.emplace_back("Strong Fire");
		if (trigger_flags.get(TRIGFLAG_MAGICFIRE))
			wpns.emplace_back("Magic Fire");
		if (trigger_flags.get(TRIGFLAG_DIVINEFIRE))
			wpns.emplace_back("Divine Fire");
		if (!wpns.empty())
		{
			wpn_causes << indent << "Any weapons matching: ";

			for (auto const& str : wpns)
				wpn_causes << "'" << str << "', ";
			wpn_causes.seekp(int(wpn_causes.tellp()) - 2);
			wpn_causes << "\n";
		}
	}

	if (!wpn_causes.str().empty())
	{
		if (trigger_flags.get(TRIGFLAG_ONLY_GROUND_WPN))
			wpn_causes << indent << "...only grounded weapons count.\n";
	}

	// weapon effects
	{
		vector<std::string> fx;
		if (trigger_flags.get(TRIGFLAG_KILLWPN))
			fx.emplace_back("Kill Triggering Weapon");
		string ignite = trigger_flags.get(TRIGFLAG_UNIGNITE_WEAPONS) ? "Unignite" : "Ignite";
		if (trigger_flags.get(TRIGFLAG_IGNITE_ANYFIRE))
			fx.emplace_back(fmt::format("{} Any Fire", ignite));
		if (trigger_flags.get(TRIGFLAG_IGNITE_STRONGFIRE))
			fx.emplace_back(fmt::format("{} Strong Fire", ignite));
		if (trigger_flags.get(TRIGFLAG_IGNITE_MAGICFIRE))
			fx.emplace_back(fmt::format("{} Magic Fire", ignite));
		if (trigger_flags.get(TRIGFLAG_IGNITE_DIVINEFIRE))
			fx.emplace_back(fmt::format("{} Divine Fire", ignite));
		if (!fx.empty())
		{
			wpn_effects << indent;
			for (auto const& str : fx)
				wpn_effects << str << ", ";
			wpn_effects.seekp(int(wpn_effects.tellp()) - 2);
			wpn_effects << "\n";
		}
	}

	bool did_btn_cause = false;
	// Other Causes
	{
		bool cause_ex1 = exstate > -1 && !trigger_flags.get(TRIGFLAG_UNSETEXSTATE);
		bool cause_ex2 = exdoor_dir > -1 && !trigger_flags.get(TRIGFLAG_UNSETEXDOOR);
		if (cause_ex1)
			causes << indent << fmt::format("ExState {}", exstate);
		if (cause_ex2)
			causes << (cause_ex1 ? ", " : indent) << fmt::format("ExDoor {} {}", dirstr_proper[exdoor_dir], exdoor_ind);
		if (cause_ex1 || cause_ex2)
			causes << " (only causes 'Combo Change'/'CSet Change', prevents other causes)\n";

		if (trigger_flags.get(TRIGFLAG_CMBTYPECAUSES))
		{
			switch (cmb.type)
			{
				case cWATER:
					causes << indent << "Drowning Here\n";
					break;
				case cPITFALL:
					causes << indent << "Falling Here\n";
					break;
				case cCUTSCENEEFFECT:
					switch (cmb.attribytes[0])
					{
						case CUTEFF_PLAYER_WALK:
							causes << indent << "Auto-walk finishes\n";
							break;
						default:
							causes << indent << "[ComboType Causes-> is checked but not doing anything]\n";
					}
					break;
			}
		}

		vector<string> btn_dirs;
		if (trigger_flags.get(TRIGFLAG_BTN_TOP))
			btn_dirs.emplace_back("Top");
		if (trigger_flags.get(TRIGFLAG_BTN_BOTTOM))
			btn_dirs.emplace_back("Bottom");
		if (trigger_flags.get(TRIGFLAG_BTN_LEFT))
			btn_dirs.emplace_back("Left");
		if (trigger_flags.get(TRIGFLAG_BTN_RIGHT))
			btn_dirs.emplace_back("Right");
		if (triggerbtn && !btn_dirs.empty())
		{
			vector<string> btns;
			if (triggerbtn & INT_BTN_A)
				btns.emplace_back("A");
			if (triggerbtn & INT_BTN_B)
				btns.emplace_back("B");
			if (triggerbtn & INT_BTN_L)
				btns.emplace_back("L");
			if (triggerbtn & INT_BTN_R)
				btns.emplace_back("R");
			if (triggerbtn & INT_BTN_EX1)
				btns.emplace_back("Ex1");
			if (triggerbtn & INT_BTN_EX2)
				btns.emplace_back("Ex2");
			if (triggerbtn & INT_BTN_EX3)
				btns.emplace_back("Ex3");
			if (triggerbtn & INT_BTN_EX4)
				btns.emplace_back("Ex4");

			causes << indent;
			if (btns.size() > 1)
			{
				causes << "Press any of ";
				for (auto const& str : btns)
					causes << str << ", ";
				causes.seekp(int(causes.tellp()) - 2);
			}
			else causes << "Press " << btns.front();

			if (btn_dirs.size() == 4)
				causes << " from any side\n";
			else if (btn_dirs.size() == 1)
				causes << " from the " << btn_dirs.front() << "\n";
			else
			{
				causes << " from any of the ";
				for (auto const& str : btn_dirs)
					causes << str << ", ";
				causes.seekp(int(causes.tellp()) - 2);
				causes << " sides\n";
			}
			did_btn_cause = true;
		}

		vector<string> misc_cause;

		if (trigcopycat && !trigger_flags.get(TRIGFLAG_NO_COPYCAT_CAUSE))
			misc_cause.emplace_back(fmt::format("Copycat {}", trigcopycat));

		if (trigger_flags.get(TRIGFLAG_TGROUP_LESS))
			misc_cause.emplace_back(fmt::format("Trigger Group {} < {}", trig_group, trig_group_val));

		if (trigger_flags.get(TRIGFLAG_TGROUP_GREATER))
			misc_cause.emplace_back(fmt::format("Trigger Group {} > {}", trig_group, trig_group_val));

		if (trigger_flags.get(TRIGFLAG_TRIGLEVELSTATE))
			misc_cause.emplace_back(fmt::format("LevelState {} toggles", trig_lstate));

		if (trigger_flags.get(TRIGFLAG_TRIGGLOBALSTATE))
			misc_cause.emplace_back(fmt::format("GlobalState {} toggles", trig_gstate));

		string light_beam = triglbeam ? fmt::format("Beam {}", triglbeam) : "Any Beam";
		if (trigtimer)
			misc_cause.emplace_back(fmt::format("Timed ({} frames)", trigtimer));
		if (trigger_flags.get(TRIGFLAG_STEP))
			misc_cause.emplace_back("Step->");
		if (trigger_flags.get(TRIGFLAG_STEPSENS))
			misc_cause.emplace_back("Step-> (Sensitive)");
		if (trigger_flags.get(TRIGFLAG_LIGHTON))
			misc_cause.emplace_back(fmt::format("Light On-> ({})", light_beam));
		if (trigger_flags.get(TRIGFLAG_LIGHTOFF))
			misc_cause.emplace_back(fmt::format("Light Off-> ({})", light_beam));
		if (trigger_flags.get(TRIGFLAG_LENSON))
			misc_cause.emplace_back("Lens On->");
		if (trigger_flags.get(TRIGFLAG_LENSOFF))
			misc_cause.emplace_back("Lens Off->");
		if (trigger_flags.get(TRIGFLAG_PUSH))
			misc_cause.emplace_back(fmt::format("Push-> ({} frames)", trig_pushtime));
		if (trigger_flags.get(TRIGFLAG_PUSHEDTRIG))
			misc_cause.emplace_back("Pushed->");
		if (trigger_flags.get(TRIGFLAG_DIVETRIG))
			misc_cause.emplace_back("Dive->");
		if (trigger_flags.get(TRIGFLAG_DIVESENSTRIG))
			misc_cause.emplace_back("Dive-> (Sensitive)");
		if (trigger_flags.get(TRIGFLAG_SCREENLOAD))
			misc_cause.emplace_back("On screen load");
		if (trigger_flags.get(TRIGFLAG_AUTOMATIC))
			misc_cause.emplace_back("Always");
		if (trigger_flags.get(TRIGFLAG_ENEMIESKILLED))
			misc_cause.emplace_back("Enemies->");
		if (trigger_flags.get(TRIGFLAG_SHUTTER))
			misc_cause.emplace_back("Shutter->");
		if (trigger_flags.get(TRIGFLAG_PLAYERLANDHERE))
			misc_cause.emplace_back("Land Here->");
		if (trigger_flags.get(TRIGFLAG_PLAYERLANDANYWHERE))
			misc_cause.emplace_back("Land Anywhere->");
		if (trigger_flags.get(TRIGFLAG_SECRETSTR))
			misc_cause.emplace_back("Secrets->");

		if (!misc_cause.empty())
		{
			causes << indent;
			for (auto const& str : misc_cause)
				causes << str << ", ";
			causes.seekp(int(causes.tellp()) - 2);
			causes << "\n";
		}
	}
	
	string dmlevel = trigdmlevel < 0 ? "current level" : ("level " + trigdmlevel);
	std::ostringstream litems;
	string litem_str;
	if (trig_levelitems)
	{
		for (int q = 0; q < li_max; ++q)
			if (trig_levelitems & (1 << q))
				litems << ZI.getLevelItemName(q) << ", ";
		litem_str = litems.str();
		TRIM_TRAILING_CHARS(litem_str);
	}
	bool ctr_discounted = trigger_flags.get(TRIGFLAG_COUNTERDISCOUNT) && trigctramnt > 0;
	bool ctr_perc = trigger_flags.get(TRIGFLAG_COUNTER_PERCENT);
	bool ctr_consume = trigger_flags.get(TRIGFLAG_COUNTEREAT);
	bool ctr_consume_no_trig = trigger_flags.get(TRIGFLAG_CTRNONLYTRIG);
	bool ctr_gradual = trigger_flags.get(TRIGFLAG_COUNTER_GRADUAL);
	string ctr_str;
	if (trigctramnt)
	{
		string ctr_name;
		if (trigctr >= 0)
			ctr_name = ZI.getCtrName(trigctr);
		else if (trigctr >= sscMIN)
			ctr_name = icounter_str2[trigctr + -(sscMIN + 1)];
		ctr_str = fmt::format("{}{} {}", trigctramnt, ctr_perc ? "%" : "", ctr_name);
		if (ctr_discounted)
			ctr_str += fmt::format(" (discounted by {})", ZI.getItemClassName(itype_wealthmedal));
	}
	// Conditionals
	{
		if (trigcooldown)
			conditions << indent << trigcooldown << " frame cooldown\n";
		if (triggeritem)
		{
			string pref;
			if (trigger_flags.get(TRIGFLAG_INVERTITEM))
				pref = "Doesn't have";
			else pref = "Has";
			conditions << indent << fmt::format("{} Item #{}\n", pref, triggeritem);
		}
		if (trigprox)
		{
			string negator;
			if (trigger_flags.get(TRIGFLAG_INVERTPROX))
				negator = " not";
			conditions << indent << fmt::format("Player is{} within {} pixels radius\n", negator, trigprox);
		}
		if (req_player_dir)
		{
			vector<string> dirs;
			for (int q = 0; q < 4; ++q)
				if (req_player_dir & (1 << q))
					dirs.emplace_back(dirstr_proper[q]);
			if (dirs.size() == 4)
				; // any dir, not really a condition?
			else if (dirs.size() == 1)
				conditions << indent << fmt::format("Player is facing {}\n", dirs.front());
			else
			{
				conditions << indent << "Player is facing any of ";
				for (auto const& str : dirs)
					conditions << str << ", ";
				conditions.seekp(int(conditions.tellp()) - 2);
				conditions << "\n";
			}
		}
		if (trigger_flags.get(TRIGFLAG_PLAYER_STANDING))
			conditions << indent << "Player is standing\n";
		if (trigger_flags.get(TRIGFLAG_PLAYER_NOTSTANDING))
			conditions << indent << "Player is not standing\n";

		// Player position
		{
			string req_sign;

			req_sign = get_sign(*this, TRIGFLAG_REQ_X_LE, TRIGFLAG_REQ_X_GE);
			if (!req_sign.empty())
			{
				conditions << indent << fmt::format("Player X {} {}", req_sign, req_player_x);
				if (trigger_flags.get(TRIGFLAG_REQ_X_REL))
					conditions << " (Relative to combo)";
				conditions << "\n";
			}

			req_sign = get_sign(*this, TRIGFLAG_REQ_Y_LE, TRIGFLAG_REQ_Y_GE);
			if (!req_sign.empty())
			{
				conditions << indent << fmt::format("Player Y {} {}", req_sign, req_player_y);
				if (trigger_flags.get(TRIGFLAG_REQ_Y_REL))
					conditions << " (Relative to combo)";
				conditions << "\n";
			}

			if (trigger_flags.get(TRIGFLAG_INVERT_PLAYER_Z))
				conditions << indent << fmt::format("Player Z < {}\n", req_player_z);
			else if (req_player_z)
				conditions << indent << fmt::format("Player Z >= {}\n", req_player_z);

			req_sign = get_sign(*this, TRIGFLAG_REQ_JUMP_LE, TRIGFLAG_REQ_JUMP_GE);
			if (!req_sign.empty())
				conditions << indent << fmt::format("Player Jump {} {}\n", req_sign, req_player_jump);
		}

		if (trigger_flags.get(TRIGFLAG_COND_DARK))
			conditions << indent << "Room is dark\n";
		if (trigger_flags.get(TRIGFLAG_COND_NODARK))
			conditions << indent << "Room is not dark\n";

		if (trig_levelitems && trigger_flags.get(TRIGFLAG_LITEM_COND))
			conditions << indent << fmt::format("Has all level items in list ({}) for {}\n", litem_str, dmlevel);
		if (trig_levelitems && trigger_flags.get(TRIGFLAG_LITEM_REVCOND))
			conditions << indent << fmt::format("Doesn't have all level items in list ({}) for {}\n", litem_str, dmlevel);

		std::ostringstream states;
		if (req_level_state)
		{
			int count = 0;
			for (int q = 0; q < 32; ++q)
				if (req_level_state & (1 << q))
				{
					++count;
					states << q << ", ";
				}
			auto state_str = states.str();
			TRIM_TRAILING_CHARS(state_str);
			string s = count != 1 ? "s" : "";

			conditions << indent << fmt::format("Level state{} {} enabled for {}\n", s, state_str, dmlevel);
		}
		if (unreq_level_state)
		{
			states.str(""); // reset str
			int count = 0;
			for (int q = 0; q < 32; ++q)
				if (unreq_level_state & (1 << q))
				{
					++count;
					states << q << ", ";
				}
			auto state_str = states.str();
			TRIM_TRAILING_CHARS(state_str);
			string s = count != 1 ? "s" : "";

			conditions << indent << fmt::format("Level state{} {} disabled for {}\n", s, state_str, dmlevel);
		}
		if (!req_global_state.empty())
		{
			states.str(""); // reset str
			int count = 0;
			for (int q = 0; q < 256; ++q)
				if (req_global_state.get(q))
				{
					++count;
					states << q << ", ";
				}
			auto state_str = states.str();
			TRIM_TRAILING_CHARS(state_str);
			string s = count != 1 ? "s" : "";

			conditions << indent << fmt::format("Global state{} {} enabled\n", s, state_str);
		}
		if (!unreq_global_state.empty())
		{
			states.str(""); // reset str
			int count = 0;
			for (int q = 0; q < 256; ++q)
				if (unreq_global_state.get(q))
				{
					++count;
					states << q << ", ";
				}
			auto state_str = states.str();
			TRIM_TRAILING_CHARS(state_str);
			string s = count != 1 ? "s" : "";

			conditions << indent << fmt::format("Global state{} {} disabled\n", s, state_str);
		}

		if (trigctramnt)
		{
			if (trigger_flags.get(TRIGFLAG_COUNTERGE))
				conditions << indent << ">= " << ctr_str << "\n";
			if (trigger_flags.get(TRIGFLAG_COUNTERLT))
				conditions << indent << "< " << ctr_str << "\n";
		}
	}

	// Other Effects
	{
		bool itm_ex = trigger_flags.get(TRIGFLAG_EXSTITEM);
		bool specitem = trigger_flags.get(TRIGFLAG_SPCITEM);
		bool enm_ex = trigger_flags.get(TRIGFLAG_EXSTENEMY);
		if (trigger_flags.get(TRIGFLAG_CMBTYPEFX))
		{
			switch (cmb.type)
			{
				case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
				case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
				case cTRIGGERGENERIC:
				case cSTEPSFX:
				default:
					effects << indent << "Apply the engine combo-type based effects";
					break;
				case cCUTSCENEEFFECT:
					switch (cmb.attribytes[0])
					{
						case CUTEFF_PLAYER_WALK:
							effects << indent << "Auto-walk the player\n";
							break;
						default:
							effects << indent << "[->ComboType Effects is checked but not doing anything]\n";
					}
					break;
				case cCUTSCENETRIG:
					effects << indent << ((cmb.usrflags & cflag1) ? "Exit" : "Enter") << " cutscene mode.\n";
					break;
				case cCSWITCH:
					effects << indent << "Toggle the switch\n";
					break;
				case cCSWITCHBLOCK:
					effects << indent << "Toggle the switch block\n";
					break;
				case cSIGNPOST:
					effects << indent << "Play the sign's string\n";
					break;
				case cSLASH: case cSLASHITEM: case cBUSH: case cFLOWERS: case cTALLGRASS:
				case cTALLGRASSNEXT:case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
				case cSLASHTOUCHY: case cSLASHITEMTOUCHY: case cBUSHTOUCHY: case cFLOWERSTOUCHY:
				case cTALLGRASSTOUCHY: case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY:
				case cBUSHNEXTTOUCHY:
					effects << indent << "'Slash' the combo\n";
					break;
				case cSTEP: case cSTEPSAME: case cSTEPALL:
					effects << indent << "'Step on' the combo\n";
					break;
				case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
				case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
				case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
				case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
				case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
				case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
					effects << indent << "Warp the player\n";
					break;
				case cCHEST:
					effects << indent << "Open the chest\n";
					break;
				case cLOCKEDCHEST: case cBOSSCHEST:
					effects << indent << "Attempt to open the chest\n";
					break;
				case cLOCKBLOCK: case cBOSSLOCKBLOCK:
					effects << indent << "Attempt to open the block\n";
					break;
				case cARMOS:
					effects << indent << "Awaken the statue\n";
					break;
				case cBSGRAVE: case cGRAVE:
					effects << indent << "Spawn the grave's enemy\n";
					break;
				case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
				case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
					effects << indent << "Damage the player\n";
					break;
				case cSWITCHHOOK:
					effects << indent << "Switch with the player\n";
					break;
				case cSHOOTER:
					effects << indent << "Fire a shot\n";
					break;
				case cSAVE: case cSAVE2:
					effects << indent << "Activate the save combo\n";
					break;
			}
		}

		if (trigchange)
			effects << indent << "Change combo by " << trigchange << "\n";
		if (trigcschange)
			effects << indent << "Change cset by " << trigcschange << "\n";
		if (trigsfx)
			effects << indent << fmt::format("Play SFX #{} ({})\n", trigsfx, sfx_string[trigsfx]);
		if (trig_genscr)
			effects << indent << fmt::format("Run generic script {} ({}) in frozen mode\n", trig_genscr, genericmap[trig_genscr].scriptname);
		if (trig_msgstr)
			effects << indent << "Play message string " << trig_msgstr << "\n";
		if (trigger_flags.get(TRIGFLAG_REVERT_GRAVITY))
			effects << indent << "Reset screen's gravity + terminal velocity\n";
		else if (trigger_flags.get(TRIGFLAG_SET_GRAVITY))
			effects << indent << fmt::format("Set screen's gravity = {} px/frame^2 and terminal velocity = {} px/frame\n",
				trig_gravity, trig_terminal_v);

		vector<string> misc_effects;

		if (trigger_flags.get(TRIGFLAG_SECRETS))
			misc_effects.emplace_back("Triggers Secrets");
		if (trigger_flags.get(TRIGFLAG_TOGGLEDARK))
			misc_effects.emplace_back("Toggle Darkness");
		if (trigger_flags.get(TRIGFLAG_RESETANIM))
			misc_effects.emplace_back("Reset combo's animation");
		if (trigger_flags.get(TRIGFLAG_CANCEL_TRIGGER))
			misc_effects.emplace_back("Cancel further triggers");
		if (trigger_flags.get(TRIGFLAG_KILLENEMIES))
			misc_effects.emplace_back("Kill all enemies");
		if (trigger_flags.get(TRIGFLAG_CLEARENEMIES))
			misc_effects.emplace_back("Clear all enemies");
		if (trigger_flags.get(TRIGFLAG_CLEARLWEAPONS))
			misc_effects.emplace_back("Clear all lweapons");
		if (trigger_flags.get(TRIGFLAG_CLEAREWEAPONS))
			misc_effects.emplace_back("Clear all eweapons");
		if (trigcopycat)
			misc_effects.emplace_back(fmt::format("Activate Copycat {}", trigcopycat));

		if (!misc_effects.empty())
		{
			effects << indent;
			for (auto const& str : misc_effects)
				effects << str << ", ";
			effects.seekp(int(effects.tellp()) - 2);
			effects << "\n";
		}

#define STATUS_EFFECT(var, name) \
if (var > -2) \
{ \
	if (var == -1) \
		effects << indent << "Inflict " name " perm\n"; \
	else if (var == 0) \
		effects << indent << "Cure " name "\n"; \
	else \
		effects << indent << fmt::format("Inflict " name "{} frames\n", var); \
}
		STATUS_EFFECT(trig_swjinxtime, "Sword Jinx")
			STATUS_EFFECT(trig_itmjinxtime, "Item Jinx")
			STATUS_EFFECT(trig_shieldjinxtime, "Shield Jinx")
			STATUS_EFFECT(trig_stuntime, "Stun")
			STATUS_EFFECT(trig_bunnytime, "Bunny")

#undef STATUS_EFFECT

			if (trigger_flags.get(TRIGFLAG_SETPLAYER_X_ABS))
				effects << indent << fmt::format("Set player X to {}\n", dest_player_x);
			else if (trigger_flags.get(TRIGFLAG_SETPLAYER_X_REL_CMB))
				effects << indent << fmt::format("Set player X to {} (relative to combo)\n", dest_player_x);
			else if (dest_player_x)
				effects << indent << fmt::format("Add {} to player X\n", dest_player_x);

		if (trigger_flags.get(TRIGFLAG_SETPLAYER_Y_ABS))
			effects << indent << fmt::format("Set player Y to {}\n", dest_player_y);
		else if (trigger_flags.get(TRIGFLAG_SETPLAYER_Y_REL_CMB))
			effects << indent << fmt::format("Set player Y to {} (relative to combo)\n", dest_player_y);
		else if (dest_player_y)
			effects << indent << fmt::format("Add {} to player Y\n", dest_player_y);

		if (trigger_flags.get(TRIGFLAG_SETPLAYER_Z_ABS))
			effects << indent << fmt::format("Set player Z to {}\n", dest_player_z);
		else if (dest_player_z)
			effects << indent << fmt::format("Add {} to player Z\n", dest_player_z);

		if (dest_player_dir > -1)
			effects << indent << fmt::format("Force player to face {}\n", dirstr_proper[dest_player_dir]);

		if (player_bounce)
			effects << indent << fmt::format("Set player Jump to {}\n", player_bounce);

		if (force_ice_combo > -1)
			effects << indent << fmt::format("Set player forced ice physics to use Combo #{}\n", force_ice_combo);
		if (trigger_flags.all({ TRIGFLAG_FORCE_ICE_VX, TRIGFLAG_FORCE_ICE_VY }))
			effects << indent << fmt::format("Force player's ice velocity to {},{}\n", force_ice_vx, force_ice_vy);
		else if (trigger_flags.get(TRIGFLAG_FORCE_ICE_VX))
			effects << indent << fmt::format("Force player's ice X velocity to {}\n", force_ice_vx);
		else if (trigger_flags.get(TRIGFLAG_FORCE_ICE_VY))
			effects << indent << fmt::format("Force player's ice Y velocity to {}\n", force_ice_vy);

		if (spawnenemy)
		{
			effects << indent << fmt::format("Spawn enemy #{} ({})\n", spawnenemy, guy_string[spawnenemy]);
			if (exstate > -1 && enm_ex)
				effects << indent << fmt::format("Sets ExState {} when spawned enemy is killed\n", exstate);
		}
		if (spawnitem)
		{
			effects << indent;
			if (spawnitem > 0)
				effects << fmt::format("Spawn item #{} ({})", spawnitem, item_string[spawnitem]);
			else effects << fmt::format("Spawn item from dropset #{} ({})", -spawnitem, item_drop_sets[-spawnitem].name);
			if (specitem)
				effects << " (using room's 'Special Item' state)";
			if (spawnip)
			{
				vector<string> ips;
				if (spawnip & ipHOLDUP)
					ips.emplace_back("Hold Up");
				if (spawnip & ipTIMER)
					ips.emplace_back("Disappears");
				if (spawnip & ipSECRETS)
					ips.emplace_back("Triggers Secrets");
				if (spawnip & ipCANGRAB)
					ips.emplace_back("Always Grabbable");
				effects << " (pickup flag(s): ";
				for (auto const& str : misc_effects)
					effects << str << ", ";
				effects.seekp(int(effects.tellp()) - 2);
				effects << ")";
			}
			if (trigger_flags.get(TRIGFLAG_AUTOGRABITEM))
				effects << " (instantly collected)";
			effects << "\n";
			if (exstate > -1 && itm_ex)
				effects << indent << fmt::format("Sets ExState {} when spawned item is collected\n", exstate);
		}

		if (exstate > -1)
		{
			if (trigger_flags.get(TRIGFLAG_UNSETEXSTATE))
				effects << indent << fmt::format("Unsets ExState {}\n", exstate);
			else if (!itm_ex && !enm_ex)
				effects << indent << fmt::format("Sets ExState {}\n", exstate);
		}
		if (exdoor_dir > -1)
		{
			if (trigger_flags.get(TRIGFLAG_UNSETEXDOOR))
				effects << indent << fmt::format("Unsets ExDoor {} {}", dirstr_proper[exdoor_dir], exdoor_ind);
			else
				effects << indent << fmt::format("Sets ExDoor {} {}", dirstr_proper[exdoor_dir], exdoor_ind);
		}

		if (trigger_flags.get(TRIGFLAG_LEVELSTATE))
			effects << indent << fmt::format("Toggle LevelState {}\n", trig_lstate);
		if (trigger_flags.get(TRIGFLAG_GLOBALSTATE))
		{
			if (trig_statetime > 0)
				effects << indent << fmt::format("Set GlobalState {} Timer = {}\n", trig_gstate, trig_statetime);
			else effects << indent << fmt::format("Toggle GlobalState {}\n", trig_gstate);
		}

		if (triggeritem && trigger_flags.get(TRIGFLAG_CONSUMEITEM))
			effects << indent << fmt::format("Consume item {} '{}'\n", triggeritem, item_string[triggeritem]);
			if (trigctramnt && ctr_consume)
			{
				effects << indent << "Consume " << ctr_str;
				if (ctr_gradual)
					effects << " (gradually)";
				effects << "\n";
			}

		bool clear_tint = trigger_flags.get(TRIGFLAG_TINT_CLEAR);
		if (trigtint[0] || trigtint[1] || trigtint[2])
		{
			effects << indent;
			if (clear_tint)
				effects << fmt::format("Set palette tint to {}r {}g {}b\n", trigtint[0], trigtint[1], trigtint[2]);
			else effects << fmt::format("Add {}r {}g {}b to palette tint\n", trigtint[0], trigtint[1], trigtint[2]);
		}
		else if (clear_tint)
			effects << indent << "Clear palette tint\n";
		if (triglvlpalette != -1)
		{
			string lvlstr = std::to_string(triglvlpalette);
			if (triglvlpalette == -2)
				lvlstr = "current screen's editor palette";
			effects << indent << "Load level palette: " << lvlstr << "\n";
		}
		if (trigbosspalette > -1)
			effects << indent << "Load boss palette: " << trigbosspalette << "\n";
		if (trigquaketime > -1)
			effects << indent << "Set quake effect timer to " << trigquaketime << "\n";
		if (trigwavytime > -1)
			effects << indent << "Set wavy effect timer to " << trigwavytime << "\n";

		if (trig_levelitems)
		{
			if (trigger_flags.get(TRIGFLAG_LITEM_SET))
				effects << indent << fmt::format("Give level items ({}) for {}\n", litem_str, dmlevel);
			if (trigger_flags.get(TRIGFLAG_LITEM_UNSET))
				effects << indent << fmt::format("Take level items ({}) for {}\n", litem_str, dmlevel);
		}

		if (did_btn_cause && prompt_cid)
		{
			effects << indent << fmt::format("Show Button Prompt (when in range), combo {}, cset {}, at {},{} relative to the player\n",
				prompt_cid, prompt_cs, prompt_x, prompt_y);
		}
	}

	string condition_str = conditions.str();
	TRIM_TRAILING_CHARS(condition_str);
	if (!condition_str.empty())
	{
		// Failure Effects
		if (did_btn_cause && fail_prompt_cid)
		{
			fail_effects << indent << fmt::format("Show Button Prompt (when in range), combo {}, cset {}, at {},{} relative to the player\n",
				fail_prompt_cid, fail_prompt_cs, prompt_x, prompt_y);
		}
		if (fail_msgstr)
			fail_effects << indent << "Play message string " << fail_msgstr << "\n";
		if (trigctramnt && ctr_consume && ctr_consume_no_trig)
		{
			fail_effects << indent << "Consume counter: " << ctr_str;
			if (ctr_gradual)
				fail_effects << " (gradually)";
			fail_effects << "\n";
		}
	}
	
	bool separate_weapon = trigger_flags.get(TRIGFLAG_SEPARATEWEAPON);
	auto wpn_cause_str = wpn_causes.str();
	auto wpn_effect_str = wpn_effects.str();
	TRIM_TRAILING_CHARS(wpn_cause_str);
	TRIM_TRAILING_CHARS(wpn_effect_str);
	if (wpn_cause_str.empty() && wpn_effect_str.empty())
		separate_weapon = false; // nothing to separate
	
	if (causes.str().empty() && effects.str().empty())
		separate_weapon = false; // nothing to separate
	
	string cause_str, effect_str;
	if (!separate_weapon && !wpn_cause_str.empty())
		cause_str = fmt::format("{}\n{}", wpn_cause_str, causes.str());
	else cause_str = causes.str();
	
	if (!separate_weapon && !wpn_effect_str.empty())
		effect_str = fmt::format("{}\n{}", wpn_effect_str, effects.str());
	else effect_str = effects.str();
	
	TRIM_TRAILING_CHARS(cause_str);
	TRIM_TRAILING_CHARS(effect_str);
	
	string fail_str = fail_effects.str();
	TRIM_TRAILING_CHARS(fail_str);
	
	output << fmt::format("Label: {}\n", label);
	if (trigger_flags.get(TRIGFLAG_TGROUP_CONTRIB))
		output << fmt::format("Contributes to Trigger Group {}\n", trig_group);
	if (separate_weapon)
	{
		output << "Weapon causes and effects are separate.\n";
		output << "Weapon Causes:\n" << wpn_cause_str << "\n";
		output << "Weapon Effects:\n" << wpn_effect_str << "\n";
	}
	if (!cause_str.empty())
		output << (separate_weapon ? "Non-Weapon Causes:\n" : "Caused by:\n") << cause_str << "\n";
	if (!condition_str.empty())
		output << (separate_weapon ? "Non-Weapon Conditions:\n" : "Conditions:\n") << condition_str << "\n";
	if (!effect_str.empty())
		output << (separate_weapon ? "Non-Weapon Effects:\n" : "Effects:\n") << effect_str << "\n";
	if (!fail_str.empty())
		output << "Effects on Condition Fail:\n" << fail_str << "\n";
	auto ret_str = output.str();
	TRIM_TRAILING_CHARS(ret_str);
	return ret_str;
#undef TRIM_TRAILING_CHARS
}

bool combo_trigger::is_blank() const
{
	return *this == combo_trigger();
}

void combo_trigger::clear()
{
	*this = combo_trigger();
}

void newcombo::set_tile(int32_t newtile)
{
	o_tile = newtile;
	tile = newtile;
}

void newcombo::clear()
{
	*this = newcombo();
}
	
bool newcombo::is_blank(bool ignoreEff) const
{
	if(tile) return false;
	if(flip) return false;
	if(walk&0xF) return false;
	if(!ignoreEff && (walk&0xF0)!=0xF0) return false;
	if(type) return false;
	if(csets) return false;
	if(frames) return false;
	if(speed) return false;
	if(nextcombo) return false;
	if(nextcset) return false;
	if(flag) return false;
	if(skipanim) return false;
	if(nexttimer) return false;
	if(skipanimy) return false;
	if(animflags) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
		if(attributes[q]) return false;
	if(usrflags) return false;
	if(genflags) return false;
	if(!label.empty()) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
		if(attribytes[q]) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
		if(attrishorts[q]) return false;
	if(script) return false;
	for(auto q = 0; q < 8; ++q)
		if(initd[q]) return false;
	if(o_tile) return false;
	if(cur_frame) return false;
	if(aclk) return false;
	
	if(liftcmb) return false;
	if(liftundercmb) return false;
	if(liftcs) return false;
	if(liftundercs) return false;
	if(liftdmg) return false;
	if(liftlvl) return false;
	if(liftitm) return false;
	if(liftflags) return false;
	if(liftgfx) return false;
	if(liftsprite) return false;
	if(liftsfx) return false;
	if(liftbreaksprite != -1) return false;
	if(liftbreaksfx) return false;
	if(lifthei != 8) return false;
	if(lifttime != 16) return false;
	if(lift_parent_item) return false;
	
	if(speed_mult != 1) return false;
	if(speed_div != 1) return false;
	if(speed_add) return false;
	
	if(sfx_appear) return false;
	if(sfx_disappear) return false;
	if(sfx_loop) return false;
	if(sfx_walking) return false;
	if(sfx_standing) return false;
	if(sfx_tap) return false;
	if(sfx_landing) return false;
	
	if(spr_appear) return false;
	if(spr_disappear) return false;
	if(spr_walking) return false;
	if(spr_standing) return false;
	if(spr_falling) return false;
	if(spr_drowning) return false;
	if(spr_lava_drowning) return false;
	if(sfx_falling) return false;
	if(sfx_drowning) return false;
	if(sfx_lava_drowning) return false;
	
	if(z_height) return false;
	if(z_step_height) return false;
	if(dive_under_level) return false;
	
	if(!triggers.empty()) return false;
	if(!misc_weap_data.is_blank()) return false;
	if(!lift_weap_data.is_blank()) return false;
	return true;
}

bool newcombo::can_cycle() const
{
	if(nextcombo != 0)
		return true;
	if(animflags & AF_CYCLEUNDERCOMBO)
		return true;
	return false;
}

int newcombo::each_tile(std::function<bool(int32_t)> proc) const
{
	int tile = o_tile;
	int frame = 0;
	do
	{
		if(proc(tile))
			return frame;
		if(++frame >= frames)
			break;
		tile += ((1+skipanim)*frame);
		if(int rowoffset = TILEROW(tile)-TILEROW(o_tile))
			tile += skipanimy * rowoffset * TILES_PER_ROW;
	}
	while(true);
	return -1;
}

void newcombo::advpaste(newcombo const& other, bitstring const& flags)
{
	if (&other == this) return;
	if(flags.get(CMB_ADVP_TILE))
	{
		tile = other.tile;
		o_tile = other.o_tile;
		flip = other.flip;
	}
	if(flags.get(CMB_ADVP_CSET2))
		csets = other.csets;
	if(flags.get(CMB_ADVP_SOLIDITY))
		walk = (walk&0xF0) | (other.walk&0x0F);
	if(flags.get(CMB_ADVP_ANIM))
	{
		frames = other.frames;
		speed = other.speed;
		nextcombo = other.nextcombo;
		nextcset = other.nextcset;
		skipanim = other.skipanim;
		nexttimer = other.nexttimer;
		skipanimy = other.skipanimy;
		animflags = other.animflags;
	}
	if(flags.get(CMB_ADVP_TYPE))
		type = other.type;
	if(flags.get(CMB_ADVP_INHFLAG))
		flag = other.flag;
	if(flags.get(CMB_ADVP_ATTRIBUTE))
	{
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
			attribytes[q] = other.attribytes[q];
		for(int32_t q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
			attrishorts[q] = other.attrishorts[q];
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
			attributes[q] = other.attributes[q];
	}
	if(flags.get(CMB_ADVP_FLAGS))
		usrflags = other.usrflags;
	if(flags.get(CMB_ADVP_GENFLAGS))
		genflags = other.genflags;
	if(flags.get(CMB_ADVP_LABEL))
		label = other.label;
	if(flags.get(CMB_ADVP_SCRIPT))
	{
		script = other.script;
		for(int32_t q = 0; q < 8; ++q)
			initd[q] = other.initd[q];
	}
	if(flags.get(CMB_ADVP_EFFECT))
		walk = (walk&0x0F) | (other.walk&0xF0);
	if(flags.get(CMB_ADVP_TRIGGERS))
		triggers = other.triggers;
	if(flags.get(CMB_ADVP_LIFTING))
	{
		liftcmb = other.liftcmb;
		liftundercmb = other.liftundercmb;
		liftcs = other.liftcs;
		liftundercs = other.liftundercs;
		liftdmg = other.liftdmg;
		liftlvl = other.liftlvl;
		liftitm = other.liftitm;
		liftflags = other.liftflags;
		liftgfx = other.liftgfx;
		liftsprite = other.liftsprite;
		liftsfx = other.liftsfx;
		liftbreaksprite = other.liftbreaksprite;
		liftbreaksfx = other.liftbreaksfx;
		lifthei = other.lifthei;
		lifttime = other.lifttime;
		lift_parent_item = other.lift_parent_item;
		lift_weap_data = other.lift_weap_data;
	}
	if(flags.get(CMB_ADVP_GEN_MOVESPEED))
	{
		speed_mult = other.speed_mult;
		speed_div = other.speed_div;
		speed_add = other.speed_add;
	}
	if(flags.get(CMB_ADVP_GEN_SFX))
	{
		sfx_appear = other.sfx_appear;
		sfx_disappear = other.sfx_disappear;
		sfx_loop = other.sfx_loop;
		sfx_walking = other.sfx_walking;
		sfx_standing = other.sfx_standing;
		sfx_tap = other.sfx_tap;
		sfx_landing = other.sfx_landing;
	}
	if(flags.get(CMB_ADVP_GEN_SPRITES))
	{
		spr_appear = other.spr_appear;
		spr_disappear = other.spr_disappear;
		spr_walking = other.spr_walking;
		spr_standing = other.spr_standing;
		spr_falling = other.spr_falling;
		spr_drowning = other.spr_drowning;
		spr_lava_drowning = other.spr_lava_drowning;
		sfx_falling = other.sfx_falling;
		sfx_drowning = other.sfx_drowning;
		sfx_lava_drowning = other.sfx_lava_drowning;
	}
	if(flags.get(CMB_ADVP_GEN_ZHEIGHT))
	{
		z_height = other.z_height;
		z_step_height = other.z_step_height;
		dive_under_level = other.dive_under_level;
		CPYFLAG(genflags, cflag3|cflag4, other.genflags); // z-height related flags
	}
	if(flags.get(CMB_ADVP_MISC_WEAP_DATA))
		misc_weap_data = other.misc_weap_data;
}

bool is_push_flag(int flag, optional<int> dir)
{
	switch(flag)
	{
		case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
			return !dir || *dir <= down;
		case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
			return !dir || *dir >= left;
		case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
			return !dir || *dir==up;
		case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
			return !dir || *dir==down;
		case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
			return !dir || *dir==left;
		case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
			return !dir || *dir==right;
		case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
			return true;
	}
	return false;
}
