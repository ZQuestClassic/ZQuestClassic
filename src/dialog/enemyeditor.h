#ifndef ZC_DIALOG_ENEMYEDITOR_H_
#define ZC_DIALOG_ENEMYEDITOR_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/drop_down_list.h>
#include <gui/switcher.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <zq/gui/seltile_swatch.h>
#include <functional>
#include <string_view>
#include <map>
#include <vector>

bool call_enemy_editor(int32_t index);

class EnemyEditorDialog : public GUI::Dialog<EnemyEditorDialog>
{
public:
	enum class message {
		REFR_INFO, ENEMYTYPE, SETALLDEFENSE, OK, CANCEL, CLEAR, DEFAULT, WARNINGS
	};


	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	EnemyEditorDialog(guydata const& ref, int32_t index);
	EnemyEditorDialog(int32_t index);

	void refreshScript();
	void loadEnemyType();
	void loadAttributes();
	void updateWarnings();
	void apply_enemy();

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Button> warnbtn;
	std::shared_ptr<GUI::SelTileSwatch> tswatch[3];
	//Attributes
	std::shared_ptr<GUI::Label> l_attributes[32];
	std::shared_ptr<GUI::Switcher> sw_attributes[32];
	std::shared_ptr<GUI::TextField> tf_attributes[32];
	std::shared_ptr<GUI::DropDownList> ddl_attributes[32];
	std::shared_ptr<GUI::Button> ib_attributes[32];
	std::shared_ptr<GUI::Button> pbtn_attributes[32];
	vector<CheckListInfo> const* plist_attributes[32];
	//Defenses
	std::shared_ptr<GUI::DropDownList> ddl_defenses[41];
	//Behavior Flags
	std::shared_ptr<GUI::Checkbox> l_bflags[16];
	std::shared_ptr<GUI::Button> ib_bflags[16];
	
	string h_initd[8], h_attribute[32], h_bflag[16];
	string l_initd[8], l_attribute[32], l_bflag[16];

	vector<string> warnings;

	int32_t index;
	guydata local_guyref;
	string enemy_name;
	int32_t spawn_type,death_type;

	GUI::ListData list_enemies, list_families, list_animations, list_guyscripts, list_ewpnscripts,
		list_sfx, list_counters_nn, list_dropsets, list_sprites, list_eweaptype, list_defenses,
		list_lwpn_defenses, list_scriptwpn_defenses, list_deftypes;

	std::shared_ptr<GUI::Widget> NumberField(auto* data, int32_t _min, int32_t _max, int _length = 5, bool _disabled = false);

	std::shared_ptr<GUI::Widget> NameField(string const& str);

	std::shared_ptr<GUI::Widget> DropDownField(auto* field, GUI::ListData const& ls, bool _disabled = false);

	bool NoDefenses();

	std::shared_ptr<GUI::Widget> DefenseField(const std::vector<int>& indices, bool _dobutton=false);

	std::shared_ptr<GUI::Widget> SizeFlag(int32_t index);

	std::shared_ptr<GUI::Widget> GuyFlag(guy_flags flag, string const& str, bool _disabled = false);

	std::shared_ptr<GUI::Widget> EditorFlag(int32_t index);

	std::shared_ptr<GUI::Widget> MoveFlag(move_flags index, string const& str);

	std::shared_ptr<GUI::Widget> WeaponFlag(weapon_flags index, string const& str);
	
	std::shared_ptr<GUI::Widget> WeaponMoveFlag(move_flags index, string const& str);

	std::shared_ptr<GUI::Widget> ScriptField(int index);

	friend bool call_enemy_editor(int32_t index);
};
#endif