
//triggerflags[0], only 18 bits are available to ZScript
#define ctrigSWORD 		0x01
#define ctrigBEAM 		0x02
#define ctrigBRANG 		0x04
#define ctrigBOMB 		0x08
#define ctrigSBOMB		0x10
#define ctrigLITBOMB		0x20
#define ctrigLITSBOMB		0x40
#define ctrigARROW		0x80
#define ctrigFIRE		0x100
#define ctrigWHISTLE		0x200
#define ctrigBAIT		0x400
#define ctrigWAND		0x800
#define ctrigMAGIC		0x1000
#define ctrigWIND		0x2000
#define ctrigREFMAGIC		0x4000
#define ctrigREFFBALL		0x8000
#define ctrigREFFROCK		0x10000
#define ctrigHAMMER		0x20000
//triggerflags[1], only 18 bits are available to ZScript
#define ctrigHOOKSHOT		0x01
#define ctrigSPARKLE		0x02
#define ctrigBYRNA		0x04
#define ctrigREFBEAM		0x08
#define ctrigSTOMP		0x10
#define ctrigSCRIPT1		0x20
#define ctrigSCRIPT2		0x40
#define ctrigSCRIPT3		0x80
#define ctrigSCRIPT4		0x100
#define ctrigSCRIPT5		0x200
#define ctrigSCRIPT6		0x400
#define ctrigSCRIPT7		0x800
#define ctrigSCRIPT8		0x1000
#define ctrigSCRIPT9		0x2000
#define ctrigSCRIPT10		0x4000
#define ctrigUNUSED_2_16 	0x8000
#define ctrigUNUSED_2_17	0x10000
#define ctrigUNUSED_2_18	0x20000
//triggerflags[2], only 18 bits are available to ZScript
#define ctrigUNUSED_3_01	0x01
#define ctrigUNUSED_3_02	0x02
#define ctrigUNUSED_3_03	0x04
#define ctrigUNUSED_3_04	0x08
#define ctrigUNUSED_3_05	0x10
#define ctrigUNUSED_3_06	0x20
#define ctrigUNUSED_3_07	0x40
#define ctrigUNUSED_3_08	0x80
#define ctrigUNUSED_3_09	0x100
#define ctrigUNUSED_3_10	0x200
#define ctrigUNUSED_3_11	0x400
#define ctrigUNUSED_3_12	0x800
#define ctrigUNUSED_3_13	0x1000
#define ctrigUNUSED_3_14	0x2000
#define ctrigUNUSED_3_15	0x4000
#define ctrigUNUSED_3_16 	0x8000
#define ctrigUNUSED_3_17	0x10000
#define ctrigUNUSED_3_18	0x20000

struct newcombo
{
	word tile; //16 bits
	byte flip; //8 bits
	byte walk; //8 bits
	byte type; //8 bits
	byte csets; //8 bits
	word foo; // 8 bits ;  used in zq_tiles for some reason. May be redundant. -- L.
	byte frames; //8 bits
	byte speed; //8 bits
	word nextcombo; //16 bits
	byte nextcset; //8 bits
	byte flag; //8 bits
	byte skipanim; //8 bits
	word nexttimer; //16 bits
	byte skipanimy; //8 bits
	byte animflags; //8 bits
	byte expansion[6]; //48 bits
	
	long attributes[NUM_COMBO_ATTRIBUTES]; //32 bits; combodata->Attributes[] and Screen->GetComboAttribute(pos, indx) / SetComboAttribute(pos, indx)
	long usrflags; //32 bits ; combodata->Flags and Screen->ComboFlags[pos]
	long triggerflags[3]; //96 bits
	long triggerlevel; //32 bits
		//Only one of these per combo: Otherwise we would have 
		//long triggerlevel[54] (1,728 bits extra per combo in a quest, and in memory) !!
		//Thus, a weapon level affects all triggers for that combo type. 
	//384 bits total per object
};

struct newcombo
{
	word tile; //16 bits
	byte flip; //8 bits
	byte walk; //8 bits
	byte type; //8 bits
	byte csets; //8 bits
	word foo; // 8 bits ;  used in zq_tiles for some reason. May be redundant. -- L.
	byte frames; //8 bits
	byte speed; //8 bits
	word nextcombo; //16 bits
	byte nextcset; //8 bits
	byte flag; //8 bits
	byte skipanim; //8 bits
	word nexttimer; //16 bits
	byte skipanimy; //8 bits
	byte animflags; //8 bits
	byte expansion[6]; //48 bits
	
	long attributes[NUM_COMBO_ATTRIBUTES]; //32 bits; combodata->Attributes[] and Screen->GetComboAttribute(pos, indx) / SetComboAttribute(pos, indx)
	long usrflags; //32 bits ; combodata->Flags and Screen->ComboFlags[pos]
	long triggerflags[3]; //96 bits
	long triggerlevel; //32 bits
		//Only one of these per combo: Otherwise we would have 
		//long triggerlevel[54] (1,728 bits extra per combo in a quest, and in memory) !!
		//Thus, a weapon level affects all triggers for that combo type. 
	//384 bits total per object
};


struct comboclass
{
	char  name[64];                       //  name
	byte  block_enemies;                  //  a (bit flags, 1=ground, 2=jump, 4=fly)
	byte  block_hole;                     //  b
	byte  block_trigger;                  //  c
	byte  block_weapon[32];               //  d (ID of lweapons/eweapons to block)
	short conveyor_x_speed;               //  e
	short conveyor_y_speed;               //  f
	word  create_enemy;                   //  g
	byte  create_enemy_when;              //  h
	long  create_enemy_change;            //  i
	byte  directional_change_type;        //  j
	long  distance_change_tiles;          //  k
	short dive_item;                      //  l
	byte  dock;                           //  m
	byte  fairy;                          //  n
	byte  ff_combo_attr_change;           //  o
	long  foot_decorations_tile;          //  p
	byte  foot_decorations_type;          //  q
	byte  hookshot_grab_point;            //  r
	byte  ladder_pass;                    //  s
	byte  lock_block_type;                //  t
	long  lock_block_change;              //  u
	byte  magic_mirror_type;              //  v
	short modify_hp_amount;               //  w
	byte  modify_hp_delay;                //  x
	byte  modify_hp_type;                 //  y
	short modify_mp_amount;               //  z
	byte  modify_mp_delay;                // aa
	byte  modify_mp_type;                 // ab
	byte  no_push_blocks;                 // ac
	byte  overhead;                       // ad
	byte  place_enemy;                    // ae
	byte  push_direction;                 // af
	byte  push_weight;                    // ag
	byte  push_wait;                      // ah
	byte  pushed;                         // ai
	byte  raft;                           // aj
	byte  reset_room;                     // ak
	byte  save_point_type;                // al
	byte  screen_freeze_type;             // am
	byte  secret_combo;                   // an
	byte  singular;                       // ao
	byte  slow_movement;                  // ap
	byte  statue_type;                    // aq
	byte  step_type;                      // ar
	long  step_change_to;                 // as
	byte  strike_weapons[32];             // at
	long  strike_remnants;                // au
	byte  strike_remnants_type;           // av
	long  strike_change;                  // aw
	short strike_item;                    // ax
	short touch_item;                     // ay
	byte  touch_stairs;                   // az
	byte  trigger_type;                   // ba
	byte  trigger_sensitive;              // bb
	byte  warp_type;                      // bc
	byte  warp_sensitive;                 // bd
	byte  warp_direct;                    // be
	byte  warp_location;                  // bf
	byte  water;                          // bg
	byte  whistle;                        // bh
	byte  win_game;                       // bi
	byte  block_weapon_lvl;               // bj (maximum item level to block)
};



