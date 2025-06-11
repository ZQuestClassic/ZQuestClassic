#include "base/qrs.h"
#include "zc/scripting/arrays.h"

// hero arrays.

static ArrayRegistrar LINKMISCD_registrar(LINKMISCD, []{
	static ScriptingArray_ObjectMemberCArray<HeroClass, &HeroClass::miscellaneous> impl;
	impl.setMul10000(false);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar LINKDEFENCE_registrar(LINKDEFENCE, []{
	static ScriptingArray_ObjectMemberCArray<HeroClass, &HeroClass::defence> impl;
	impl.setMul10000(true);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar HEROLIFTFLAGS_registrar(HEROLIFTFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<HeroClass, &HeroClass::liftflags, NUM_LIFTFL> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar HEROMOVEFLAGS_registrar(HEROMOVEFLAGS, []{
	static ScriptingArray_ObjectMemberBitwiseFlags<HeroClass, &HeroClass::moveflags, 11> impl;
	impl.setDefaultValue(0);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar HEROSTEPS_registrar(HEROSTEPS, []{
	static ScriptingArray_GlobalCArray impl(lsteps, comptime_array_size(lsteps));
	impl.setMul10000(true);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar LINKITEMD_registrar(LINKITEMD, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int) { return MAXITEMS; },
		[](int, int index) {
			return game->item[index];
		},
		[](int, int index, bool value) {
			int itemID = index;

			// If the Cane of Byrna is being removed, cancel its effect.
			if (!value && itemID==current_item_id(itype_cbyrna))
				stopCaneOfByrna();
			
			auto& data = get_item_script_engine_data(itemID);
		
			//Stop current script if set false.
			if ( !value && data.doscript )
			{
				data.doscript = 4; //Val of 4 means 'clear stack and quit'
				//itemScriptData[itemID].Clear(); //Don't clear here, causes crash if is current item!
			}
			else if ( value && !data.doscript )
			{
				//Clear the item refInfo and stack for use.
				data.ref.Clear();
				if ( (itemsbuf[itemID].flags&item_passive_script) ) data.doscript = 1;
			}
			else if ( value && data.doscript == 4 ) 
			{
				// Arbitrary event number 49326: Writing the item false, then true, in the same frame. -Z
				if ( (itemsbuf[itemID].flags&item_passive_script) ) data.doscript = 1;
			}
			
			//Sanity check to prevent setting the item if the value would be the same. -Z
			if ( game->item[itemID] != value ) 
			{
				game->set_item(itemID, value);
			}
					
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (cur_screen<128 || dlevel)) 
			{
				ringcolor(false);
				//refreshpal=true;
			}
			if (!value) //setting the item false clears the state of forced ->Equipment writes.
			{
				if (game->forced_bwpn == itemID)
					game->forced_bwpn = -1;
				if (game->forced_awpn == itemID)
					game->forced_awpn = -1;
				if (game->forced_xwpn == itemID)
					game->forced_xwpn = -1;
				if (game->forced_ywpn == itemID)
					game->forced_ywpn = -1;
			}

			return true;
		}
	);
	impl.setMul10000(true);
	impl.boundIndex();
	return &impl;
}());

static ArrayRegistrar LINKHITBY_registrar(LINKHITBY, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int) { return NUM_HIT_TYPES_USED; },
		[](int, int index) -> int {
			switch (index)
			{
				//screen indices of objects
				case 0:
				case 1:
				case 2:
				case 3:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 16:
					return (Hero.gethitHeroUID(index)) * 10000;

				//uids of objects
				case 4:
				case 5:
				case 6:
				case 7:
				case 13:
				case 14:
				case 15:
					return Hero.gethitHeroUID(index); //do not multiply by 10000! UIDs are not *10000!

				default: NOTREACHED();
			}
		},
		[](int, int, int) {
			return false;
		}
	);
	impl.setDefaultValue(0);
	impl.setMul10000(false);
	impl.readOnly();
	return &impl;
}());