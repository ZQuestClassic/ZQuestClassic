// std_constants.zh
// Contains required constants for std.zh and ZScript functions.

// Obsolete
//#define PI						3.1416
//#define PI2						6.2832
//#define E						2.7183
//#define NULL						0
//#define MAX_CONSTANT			214747.9999
//
//#define MAX_BITSPERVARIABLE		18
//#define MAX_SCRIPTDRAWINGCOMMANDS	1000

#define HP_PER_HEART				16
#define MP_PER_BLOCK				32

#define MAX_MESSAGES				65535
//#define MAX_MESSAGELENGTH			145 //Pass an array of (at least) this size to Game->GetMessage

#define MAX_DMAPS					512
#define MAX_DMAPNAMELENGTH		21 //Size of arrays to pass to Game->GetDMapX
#define MAX_DMAPTITLELENGTH		21
#define MAX_DMAPINTROLENGTH		73

//Defaults from Init Data
#define GRAVITY					0.16f
#define TERMINAL_VELOCITY		3.20f
#define JUMPING_LAYER_THRESHOLD	16

//Sound effect IDs. To be used for PlaySound().
#define SFX_ARROW       1 // Arrow is fired.
#define SFX_BEAM        2 // Sword beam is fired.
#define SFX_BOMB        3 // Bomb explodes.
#define SFX_BRANG       4 // Boomerang spinning.
#define SFX_CHARGE1     35 // Sword powering up.
#define SFX_CHARGE2     36 // Sword powering up again (Hurricane Spin).
#define SFX_CURSOR      5 // Subscreen cursor.
#define SFX_CLINK       6 // Shield is hit.
#define SFX_CLEARED     7 // Chime when screen item appears.
#define SFX_DINSFIRE    37 // Din's Fire blast.
#define SFX_DODONGO     8 // Dodongo's roar.
#define SFX_SHUTTER     9 // Shutter bang.
#define SFX_EDEAD       10 // Enemy is killed.
#define SFX_EHIT        11 // Enemy is hit.
#define SFX_LOWHP       12 // Low hearts warning beep.
#define SFX_FALL        38 // Enemy falls from ceiling (unused).
#define SFX_FARORESWIND 39 // Farore's Wind spell.
#define SFX_FIRE        13 // Flame roar.
#define SFX_FIREBALL    40 // Enemy fireball.
#define SFX_GANON       14 // Music when Ganon appears/dies.
#define SFX_GASP        15 // Boss is hit.
#define SFX_GRASSCUT    41 // Grass or bush slashed.
#define SFX_HAMMER      16 // Hammer pound.
#define SFX_HAMMERPOST  42 // Pounded a post.
#define SFX_HOOKSHOT    17 // Hookshot chain rattle.
#define SFX_HOVER       43 // Hover boots.
#define SFX_ICE         44 // Unused.
#define SFX_JUMP        45 // Jumping.
#define SFX_LENSOFF     46 // Lens of Truth off.
#define SFX_LENSON      47 // Lens of Truth on.
#define SFX_MSG         18 // Message typing.
#define SFX_NAYRUSLOVE1 48 // Nayru's Love shield hum.
#define SFX_NAYRUSLOVE2 49 // Nayru's Love shield running out.
#define SFX_OUCH        19 // Link is hit.
#define SFX_PICKUP      20 // Chime when item is held above head.
#define SFX_PLACE       21 // Bomb is placed.
#define SFX_PLINK       22 // Heart/Rupee is collected.
#define SFX_PUSHBLOCK   50 // Pushed a block.
#define SFX_REFILL      23 // Hearts being refilled.
#define SFX_ROAR        24 // Aquamentus, Gleeok and Ganon's roar.
#define SFX_ROCK        51 // Octorok rock is fired.
#define SFX_ROCKETDOWN  52 // Spell rocket descends.
#define SFX_ROCKETUP    53 // Spell rocket launched.
#define SFX_SCALE       25 // Chime when fairy appears/is picked up.
#define SFX_SEA         26 // "Ocean SFX" ambience.
#define SFX_SECRET      27 // Chime when secret is discovered.
#define SFX_SPINATTACK  54 // Sword spin.
#define SFX_SPIRAL      28 // Link dies.
#define SFX_SPLASH      55 // Splashing in shallow water.
#define SFX_STAIRS      29 // Link marches down stairs.
#define SFX_SUMMON      56 // Summoner magic.
#define SFX_SWORD       30 // Sword swipe.
#define SFX_TAP1        57 // Sword taps wall.
#define SFX_TAP2        58 // Sword taps bombable wall.
#define SFX_PATRA       31 // Patra and Manhandla's shriek.
#define SFX_WAND        32 // Magic is fired by Wizzrobes or Link.
#define SFX_WHIRLWIND   59 // Whistle whirlwind.
#define SFX_WHISTLE     33 // Whistle is played.
#define SFX_ZELDA       34 // Music when ending sequence begins.

//Item class ids. Use as comparisons with Itemdata->Family
#define IC_AMULET             11
#define IC_ARROW              2
#define IC_ARROWAMMO          40
#define IC_BAIT               5
#define IC_BOMB               27
#define IC_BOMBAMMO           47
#define IC_BOMBBAG            48
#define IC_BOOK               16
#define IC_BOOTS              20
#define IC_BOSSKEY            35
#define IC_BOW                13
#define IC_BOWANDARROW        87
#define IC_BRACELET           18
#define IC_BRANG              1
#define IC_CANDLE             3
#define IC_CBYRNA             38
#define IC_CHARGERING         55
#define IC_CLOCK              29
#define IC_COMPASS            34
#define IC_CROSSSCROLL        52
#define IC_DINSFIRE           24
#define IC_FAIRY              41
#define IC_FARORESWIND        25
#define IC_FLIPPERS           19
#define IC_HAMMER             23
#define IC_HEART              43
#define IC_HEARTCONTAINER     44
#define IC_HEARTPIECE         45
#define IC_HEARTRING          58
#define IC_HOOKSHOT           21
#define IC_HOVERBOOTS         50
#define IC_HURRICANESCROLL    60
#define IC_KEY                30
#define IC_KILLALL            46
#define IC_LADDER             15
#define IC_LENS               22
#define IC_LETTER             6
#define IC_LETTERPOTION       88
#define IC_LEVELKEY           37
#define IC_MAGIC              42
#define IC_MAGICCONTAINER     31
#define IC_MAGICKEY           17
#define IC_MAGICRING          59
#define IC_MAP                33
#define IC_MISC               66
#define IC_NAYRUSLOVE         26
#define IC_PERILRING          65
#define IC_PERILSCROLL        56
#define IC_POTION             7
#define IC_QUAKESCROLL        53
#define IC_QUIVER             36
#define IC_RAFT               14
#define IC_RING               9
#define IC_ROCS               49
#define IC_RUPEE              39
#define IC_SBOMB              28
#define IC_SHIELD             12
#define IC_SPINSCROLL         51
#define IC_SQUAKESCROLL       61
#define IC_STOMPBOOTS         63
#define IC_STONEAGONY         62
#define IC_SWORD              0
#define IC_TRIFORCE           32
#define IC_WALLET             10
#define IC_WAND               8
#define IC_WEALTHMEDAL        57
#define IC_WHIMSICALRING      64
#define IC_WHISPRING          54
#define IC_WHISTLE            4

#define IC_CUSTOM1            67
#define IC_CUSTOM2            68
#define IC_CUSTOM3            69
#define IC_CUSTOM4            70
#define IC_CUSTOM5            71
#define IC_CUSTOM6            72
#define IC_CUSTOM7            73
#define IC_CUSTOM8            74
#define IC_CUSTOM9            75
#define IC_CUSTOM10           76
#define IC_CUSTOM11           77
#define IC_CUSTOM12           78
#define IC_CUSTOM13           79
#define IC_CUSTOM14           80
#define IC_CUSTOM15           81
#define IC_CUSTOM16           82
#define IC_CUSTOM17           83
#define IC_CUSTOM18           84
#define IC_CUSTOM19           85
#define IC_CUSTOM20           86


//Item IDs. Use with CreateItem() and Link->Item[].
//These are DEFAULT values, and may not be correct for custom quests which
//have overridden item action data.
#define I_AMULET1             50
#define I_AMULET2             63
#define I_ARROW1              13
#define I_ARROW2              14
#define I_ARROW3              57
#define I_ARROWAMMO1          70
#define I_ARROWAMMO10         72
#define I_ARROWAMMO30         73
#define I_ARROWAMMO5          71
#define I_BAIT                16
#define I_BOMB                3
#define I_BOMBAMMO1           77
#define I_BOMBAMMO30          80
#define I_BOMBAMMO4           78
#define I_BOMBAMMO8           79
#define I_BOMBBAG1            81
#define I_BOMBBAG2            82
#define I_BOMBBAG3            83
#define I_BOMBBAG4            106
#define I_BOOK                32
#define I_BOOTS               55
#define I_BOSSKEY             67
#define I_BOW1                15
#define I_BOW2                68
#define I_BRACELET1           107
#define I_BRACELET2           19
#define I_BRACELET3           56
#define I_BRANG1              23
#define I_BRANG2              24
#define I_BRANG3              35
#define I_CANDLE1             10
#define I_CANDLE2             11
#define I_CBYRNA              88
#define I_CHARGERING1         101
#define I_CHARGERING2         102
#define I_CLOCK               4
#define I_COMPASS             22
#define I_CROSSSCROLL         95
#define I_DINSFIRE            64
#define I_DUST_PILE           43
#define I_FAIRY               34
#define I_FAIRYSTILL          69
#define I_FARORESWIND         65
#define I_FLIPPERS            51
#define I_HAMMER              54
#define I_HCPIECE             49
#define I_HEART               2
#define I_HEARTCONTAINER      28
#define I_HEARTRING1          112
#define I_HEARTRING2          113
#define I_HEARTRING3          114
#define I_HOOKSHOT1           52
#define I_HOOKSHOT2           89
#define I_HOVERBOOTS          92
#define I_KEY                 9
#define I_KILLALL             62
#define I_LADDER1             27
#define I_LADDER2             108
#define I_LENS                53
#define I_LETTER              12
#define I_LETTERUSED          90
#define I_LEVELKEY            84
#define I_MAGICCONTAINER      58
#define I_MAGICJAR1           59
#define I_MAGICJAR2           60
#define I_MAGICKEY            33
#define I_MAGICRING1          115
#define I_MAGICRING2          116
#define I_MAGICRING3          117
#define I_MAGICRING4          118
#define I_MAP                 21
#define I_MISC1               46
#define I_MISC2               47
#define I_NAYRUSLOVE          66
#define I_PERILRING           121
#define I_PERILSCROLL         103
#define I_POTION1             29
#define I_POTION2             30
#define I_QUAKESCROLL1        96
#define I_QUAKESCROLL2        97
#define I_QUIVER1             74
#define I_QUIVER2             75
#define I_QUIVER3             76
#define I_QUIVER4             105
#define I_RAFT                26
#define I_RING1               17
#define I_RING2               18
#define I_RING3               61
#define I_ROCSFEATHER         91
#define I_RUPEE1              0
#define I_RUPEE10             86
#define I_RUPEE100            87
#define I_RUPEE20             38
#define I_RUPEE200            40
#define I_RUPEE5              1
#define I_RUPEE50             39
#define I_SBOMB               48
#define I_SELECTA             45
#define I_SELECTB             85
#define I_SHIELD1             93
#define I_SHIELD2             8
#define I_SHIELD3             37
#define I_SPINSCROLL1         94
#define I_SPINSCROLL2         98
#define I_STOMPBOOTS          120
#define I_STONEAGONY          119
#define I_SWORD1              5
#define I_SWORD2              6
#define I_SWORD3              7
#define I_SWORD4              36
#define I_TRIFORCE            20
#define I_TRIFORCEBIG         44
#define I_WALLET500           41
#define I_WALLET999           42
#define I_WALLETA             104
#define I_WAND                25
#define I_WEALTHMEDAL         109
#define I_WEALTHMEDAL2        110
#define I_WEALTHMEDAL3        111
#define I_WHIMSICALRING       122
#define I_WHISPRING1          99
#define I_WHISPRING2          100
#define I_WHISTLE             31

//Link actions
#define LA_NONE               0
#define LA_WALKING            1
#define LA_ATTACKING          2
#define LA_FROZEN             3
#define LA_HOLD1LAND          4 // One hand.
#define LA_HOLD2LAND          5 // Both hands.
#define LA_RAFTING            6
#define LA_GOTHURTLAND        7
#define LA_INWIND             8
#define LA_SCROLLING          9
#define LA_WINNING            10
#define LA_SWIMMING           11
#define LA_HOPPING            12 // Leaving water.
#define LA_GOTHURTWATER       13
#define LA_HOLD1WATER         14
#define LA_HOLD2WATER         15
#define LA_CASTING            16 // Casting a spell.
#define LA_CAVEWALKUP         17
#define LA_CAVEWALKDOWN       18
#define LA_DYING              19
#define LA_DROWNING           20
#define LA_CHARGING           22 // Charging the sword.
#define LA_SPINNING           23 // Spin attack.
#define LA_DIVING             24

//NPC Types. Compare against npc->Type.
#define NPCT_GUY			0
#define NPCT_WALK			1
#define NPCT_TEKTITE			3
#define NPCT_LEEVER			4
#define NPCT_PEAHAT			5
#define NPCT_ZORA			6
#define NPCT_ROCK			7
#define NPCT_GHINI			8
#define NPCT_ARMOS			9
#define NPCT_KEESE			10
#define NPCT_TRAP			15
#define NPCT_WALLMASTER		16
#define NPCT_WIZZROBE			21
#define NPCT_AQUAMENTUS		22
#define NPCT_MOLDORM			23
#define NPCT_DODONGO			24
#define NPCT_MANHANDLA		25
#define NPCT_GLEEOK			26
#define NPCT_DIGDOGGER		27
#define NPCT_GOHMA			28
#define NPCT_LANMOLA			29
#define NPCT_PATRA			30
#define NPCT_GANON			31
#define NPCT_PROJECTILE		32
#define NPCT_SPINTILE			37
#define NPCT_NONE			38
#define NPCT_FAIRY			39
#define NPCT_OTHERFLOAT		40
#define NPCT_OTHER			41

//Guy IDs. Use with CreateNPC().
#define NPC_ABEI              1
#define NPC_AMA               2
#define NPC_MERCHANT          3
#define NPC_MOBLIN            4
#define NPC_FIRE              5
#define NPC_FAIRY             6
#define NPC_GORIYA            7
#define NPC_ZELDA             8
#define NPC_ABEI2             9

//Default enemy IDs. Use with CreateNPC().
#define NPC_AQUAMENTUSL       93
#define NPC_AQUAMENTUSR       58
#define NPC_ARMOS             37
#define NPC_BAT               106
#define NPC_BOMBCHU           160
#define NPC_BOULDER           135
#define NPC_BUBBLEITEMP       118	// Permanently jinxes
#define NPC_BUBBLEITEMR       119	// Restores item usage
#define NPC_BUBBLEITEMT       117
#define NPC_BUBBLESWORDP      81  // Permanently jinxes
#define NPC_BUBBLESWORDR      82  // Restores sword usage
#define NPC_BUBBLESWORDT      51
#define NPC_CEILINGMASTER     101 // This enemy isn't implemented
#define NPC_DARKNUT1          49
#define NPC_DARKNUT2          50
#define NPC_DARKNUT3          92	// Splits when killed.
#define NPC_DARKNUT4          172  // This enemy isn't implemented
#define NPC_DARKNUT5          87   // Sword beams.
#define NPC_DIGDOGGER1        66 // Splits into 1
#define NPC_DIGDOGGER3        67 // Splits into 3
#define NPC_DIGKID1           68 // Spawned by NPC_DIGDOGGER1
#define NPC_DIGKID2           69 // Spawned by NPC_DIGDOGGER3
#define NPC_DIGKID3           70 // Spawned by NPC_DIGDOGGER3
#define NPC_DIGKID4           71 // Spawned by NPC_DIGDOGGER3
#define NPC_DODONGO           60 // Uses one tile when walking vertically
#define NPC_DODONGOBS         114 // Uses two tiles when walking vertically
#define NPC_DODONGOFIRE       115 // This enemy isn't implemented
#define NPC_ENEMYFIRE         85  // The Fire enemy in Princess Zelda's screen
#define NPC_FLOORMASTER       102 // This enemy isn't implemented
#define NPC_GANON             78
#define NPC_GEL               42
#define NPC_GELFIRE           161
#define NPC_GELFIRETRIB       163
#define NPC_GELTRIB           88
#define NPC_GHINI1            35
#define NPC_GHINI2            36
#define NPC_GHINIMGC          173 // This enemy isn't implemented
#define NPC_GIBDO             54
#define NPC_GLEEOK1           62
#define NPC_GLEEOK1FIRE       109
#define NPC_GLEEOK2           63
#define NPC_GLEEOK2FIRE       110
#define NPC_GLEEOK3           64
#define NPC_GLEEOK3FIRE       111
#define NPC_GLEEOK4           65
#define NPC_GLEEOK4FIRE       112
#define NPC_GOHMA1            72
#define NPC_GOHMA2            73
#define NPC_GOHMA3            121
#define NPC_GOHMAFIRE         122
#define NPC_GORIYA1           45
#define NPC_GORIYA2           46
#define NPC_GORIYA3           136
#define NPC_GRAPBUGHP         174 // This enemy isn't implemented
#define NPC_GRAPBUGMP         175 // This enemy isn't implemented
#define NPC_ITEMFAIRY         84
#define NPC_KEESE1            38
#define NPC_KEESE2            39
#define NPC_KEESE3            40
#define NPC_KEESETRIB         90
#define NPC_LANMOLA1          74
#define NPC_LANMOLA2          75
#define NPC_LEEVER1           26
#define NPC_LEEVER2           27
#define NPC_LEEVER3           137
#define NPC_LIKELIKE          53
#define NPC_LYNEL1            30
#define NPC_LYNEL2            31
#define NPC_LYNEL3            168
#define NPC_MANHANDLA         61
#define NPC_MANHANDLA2        94
#define NPC_MOBLIN1           28
#define NPC_MOBLIN2           29
#define NPC_MOLDORM           59
#define NPC_OCTOROCK1F        22
#define NPC_OCTOROCK1S        20
#define NPC_OCTOROCK2F        23
#define NPC_OCTOROCK2S        21
#define NPC_OCTOROCKBOMBF     141
#define NPC_OCTOROCKBOMBS     140
#define NPC_OCTOROCKFIREF     139
#define NPC_OCTOROCKFIRES     138
#define NPC_OCTOROCKMGC       86
#define NPC_PATRA1            76
#define NPC_PATRA2            104
#define NPC_PATRA3            105
#define NPC_PATRABS           103
#define NPC_PATRAOVAL         77
#define NPC_PEAHAT            32
#define NPC_PEAHATFIRE        169 // This enemy isn't implemented
#define NPC_POLSVOICE         55
#define NPC_POLSVOICEBS       171 // This enemy isn't implemented
#define NPC_POLSVOICEMGC      170
#define NPC_ROCK              34
#define NPC_ROPE1             44
#define NPC_ROPE2             80
#define NPC_SHOOTFBALL        83
#define NPC_SHOOTFLAME        158
#define NPC_SHOOTFLAME2       159
#define NPC_SHOOTMAGIC        154
#define NPC_SHOOTROCK         155
#define NPC_SHOOTSPEAR        156
#define NPC_SHOOTSWORD        157
#define NPC_SPINTILE          166
#define NPC_SPINTILERND       167 // Random spin tile
#define NPC_STALFOS1          41
#define NPC_STALFOS2          79
#define NPC_STALFOS3          120
#define NPC_TEKTITE1          24
#define NPC_TEKTITE2          25
#define NPC_TEKTITE3          165 // This enemy isn't implemented
#define NPC_TRAP              47
#define NPC_TRAP8WAY          142  // This enemy isn't implemented
#define NPC_TRAPBACKSLASHC    146	// This enemy isn't implemented
#define NPC_TRAPBACKSLASHLOS  147 // This enemy isn't implemented
#define NPC_TRAPCCLOCKWISEC   150 // This enemy isn't implemented
#define NPC_TRAPCCLOCKWISELOS 151 // This enemy isn't implemented
#define NPC_TRAPCLOCKWISEC    148 // This enemy isn't implemented
#define NPC_TRAPCLOCKWISELOS  149 // This enemy isn't implemented
#define NPC_TRAPDIAG          143 // This enemy isn't implemented
#define NPC_TRAPHORIZC        97
#define NPC_TRAPHORIZLOS      95
#define NPC_TRAPSLASHC        144	// This enemy isn't implemented
#define NPC_TRAPSLASHLOS      145
#define NPC_TRAPVERTC         98
#define NPC_TRAPVERTLOS       96
#define NPC_TRIGGER           116
#define NPC_VIRE              52
#define NPC_VIRETRIB          91
#define NPC_WALLMASTER        48
#define NPC_WIZZROBE1         56 // Teleports
#define NPC_WIZZROBE2         57 // Phases through blocks
#define NPC_WIZZROBEBAT       107
#define NPC_WIZZROBEBAT2      108 // This enemy isn't implemented
#define NPC_WIZZROBEFIRE      99
#define NPC_WIZZROBEICE       153 // This enemy isn't implemented
#define NPC_WIZZROBEMIRR      113
#define NPC_WIZZROBESUMM      152
#define NPC_WIZZROBEWIND      100
#define NPC_ZOL               43
#define NPC_ZOLFIRE           162
#define NPC_ZOLFIRETRIB       164
#define NPC_ZOLTRIB           89
#define NPC_ZORA              33

//NPC hit point values. Use with npc->HP to make the enemy die in a special manner.
#define HP_SILENT -1000  // The enemy will die without their Death Sound playing.

//LWeapon types. Use with Screen->CreateLWeapon() and Screen->LoadLWeapon().
#define LW_SWORD              1 // This cannot be created
#define LW_WAND               12 // This cannot be created
#define LW_CANDLE             12 // This cannot be created
#define LW_HAMMER             19 // This cannot be created
#define LW_HOOKSHOT           20 // This cannot be created
#define LW_CANEOFBYRNA        27 // This cannot be created
#define LW_ARROW              8
#define LW_BEAM               2 // Sword beam
#define LW_BRANG              3
#define LW_BOMB               6
#define LW_BOMBBLAST          4 // Bomb explosion
#define LW_SBOMB              7
#define LW_SBOMBBLAST         5 // Super Bomb explosion
#define LW_FIRE               9
#define LW_WHISTLE            10 // Whistle music - kills Digdogger
#define LW_BAIT               11
#define LW_MAGIC              13 // Wand magic
#define LW_WIND               15 // Whistle whirlwind
#define LW_REFMAGIC           16 // Reflected wand magic
#define LW_REFFIREBALL        17 // Reflected fireball
#define LW_REFROCK            18 // Reflected rock
#define LW_REFBEAM            28 // Reflected sword beam
#define LW_SPARKLE            23
#define LW_FIRESPARKLE        24 // Damages enemies
// Dummy weapons, for use with scripts.
#define LW_SCRIPT1            31
#define LW_SCRIPT2            32
#define LW_SCRIPT3            33
#define LW_SCRIPT4            34
#define LW_SCRIPT5            35
#define LW_SCRIPT6            36
#define LW_SCRIPT7            37
#define LW_SCRIPT8            38
#define LW_SCRIPT9            39
#define LW_SCRIPT10           40

//EWeapon types. Use with Screen->CreateEWeapon() and Screen->LoadEWeapon().
#define EW_ARROW              130
#define EW_BRANG              131
#define EW_BEAM               132 // Sword beam
#define EW_ROCK               133
#define EW_MAGIC              134
#define EW_FIREBALL           129
#define EW_FIREBALL2          145 // Arcing boss fireball
#define EW_BOMB               137 // Projectile bomb
#define EW_BOMBBLAST          135 // Bomb explosion
#define EW_SBOMB              138 // Projectile super bomb
#define EW_SBOMBBLAST         136 // Super bomb explosion
#define EW_FIRETRAIL          139
#define EW_FIRE               140
#define EW_WIND               141 // Wind wizzrobe magic
#define EW_FIRE2				142
// Dummy weapons, for use with scripts.
#define EW_SCRIPT1            31
#define EW_SCRIPT2            32
#define EW_SCRIPT3            33
#define EW_SCRIPT4            34
#define EW_SCRIPT5            35
#define EW_SCRIPT6            36
#define EW_SCRIPT7            37
#define EW_SCRIPT8            38
#define EW_SCRIPT9            39
#define EW_SCRIPT10           40

//Weapon DeadState values. Use with Weapon->DeadState.
#define WDS_NOHIT             -10 // This value switches collision detection off. Deprecated by weapon->CollDetection.
#define WDS_ALIVE		-1 // Weapon is currently 'alive'.
#define WDS_DEAD              0  // Use to dispose of most weapons.
#define WDS_BEAMSHARDS        23 // Use with LW_BEAMs to shatter it into shards.
#define WDS_ARROW             4  // Use with LW_ARROWs to make them 'wink out' using tile 54.
#define WDS_BOUNCE            1  // Use with LW_BRANGs or LW_HOOKSHOTs to
// make it 'bounce off' and start returning to Link.

//FFC Flags. To be used as indices into ffc->Flags[].
#define FFCF_OVERLAY          0 // Draw Over (draw between layers 4 and 5)
#define FFCF_TRANS            1 // Translucent
#define FFCF_SOLID            2 // Unused
#define FFCF_CARRYOVER        3 // Carry Over
#define FFCF_STATIONARY       4 // Stationary
#define FFCF_CHANGER          5 // Is A Changer
#define FFCF_PRELOAD          6 // Run Script On Screen Init
#define FFCF_LENSVIS          7 // Only Visible to Lens of Truth
#define FFCF_RESET            8 // Script Resets When Carried Over
#define FFCF_ETHEREAL         9 // The FFC's Combo Type is ignored.
#define FFCF_IGNOREHOLDUP     10 // The FFC is updated while Link is holding up an item

//Directions. Use with Link->Dir, etc.
#define DIR_UP                0 //000b
#define DIR_DOWN              1 //001b
#define DIR_LEFT              2 //010b
#define DIR_RIGHT             3 //011b
#define DIR_LEFTUP            4 //100b
#define DIR_RIGHTUP           5 //101b
#define DIR_LEFTDOWN          6 //110b
#define DIR_RIGHTDOWN         7 //111b

//Combo flags. Use with Screen->ComboF[]
#define CF_NONE               0
#define CF_PUSHUPDOWN         1
#define CF_PUSH4WAY           2
#define CF_WHISTLE            3
#define CF_CANDLE1            4
#define CF_ARROW              5
#define CF_BOMB               6
#define CF_FAIRY              7
#define CF_RAFT               8
#define CF_ARMOSSECRET        9
#define CF_ARMOSITEM          10
#define CF_SBOMB              11
#define CF_RAFTBRANCH         12
#define CF_DIVEITEM           13
#define CF_ZELDA              15
#define CF_SECRETS01          16
#define CF_SECRETS02          17
#define CF_SECRETS03          18
#define CF_SECRETS04          19
#define CF_SECRETS05          20
#define CF_SECRETS06          21
#define CF_SECRETS07          22
#define CF_SECRETS08          23
#define CF_SECRETS09          24
#define CF_SECRETS10          25
#define CF_SECRETS11          26
#define CF_SECRETS12          27
#define CF_SECRETS13          28
#define CF_SECRETS14          29
#define CF_SECRETS15          30
#define CF_SECRETS16          31
#define CF_TRAPH              32
#define CF_TRAPV              33
#define CF_TRAP4WAY           34
#define CF_TRAPLR             35
#define CF_TRAPUD             36
#define CF_ENEMY0             37
#define CF_ENEMY1             38
#define CF_ENEMY2             39
#define CF_ENEMY3             40
#define CF_ENEMY4             41
#define CF_ENEMY5             42
#define CF_ENEMY6             43
#define CF_ENEMY7             44
#define CF_ENEMY8             45
#define CF_ENEMY9             46
#define CF_PUSHLR             47
#define CF_PUSHUP             48
#define CF_PUSHDOWN           49
#define CF_PUSHLEFT           50
#define CF_PUSHRIGHT          51
#define CF_PUSHUPDOWNNS       52
#define CF_PUSHLEFTRIGHTNS    53
#define CF_PUSH4WAYNS         54
#define CF_PUSHUPNS           55
#define CF_PUSHDOWNNS         56
#define CF_PUSHLEFTNS         57
#define CF_PUSHRIGHTNS        58
#define CF_PUSHUPDOWNINS      59
#define CF_PUSHLEFTRIGHTINS   60
#define CF_PUSH4WAYINS        61
#define CF_PUSHUPINS          62
#define CF_PUSHDOWNINS        63
#define CF_PUSHLEFTINS        64
#define CF_PUSHRIGHTINS       65
#define CF_BLOCKTRIGGER       66
#define CF_NOBLOCKS           67
#define CF_BRANG1             68
#define CF_BRANG2             69
#define CF_BRANG3             70
#define CF_ARROW2             71
#define CF_ARROW3             72
#define CF_CANDLE2            73
#define CF_WANDFIRE           74
#define CF_DINSFIRE           75
#define CF_WANDMAGIC          76
#define CF_REFMAGIC           77
#define CF_REFFIREBALL        78
#define CF_SWORD1             79
#define CF_SWORD2             80
#define CF_SWORD3             81
#define CF_SWORD4             82
#define CF_SWORD1BEAM         83
#define CF_SWORD2BEAM         84
#define CF_SWORD3BEAM         85
#define CF_SWORD4BEAM         86
#define CF_HOOKSHOT           87
#define CF_WAND               88
#define CF_HAMMER             89
#define CF_STRIKE             90
#define CF_BLOCKHOLE          91
#define CF_MAGICFAIRY         92
#define CF_ALLFAIRY           93
#define CF_SINGLE             94
#define CF_SINGLE16           95
#define CF_NOENEMY            96
#define CF_NOGROUNDENEMY      97
#define CF_SCRIPT1            98
#define CF_SCRIPT2            99
#define CF_SCRIPT3            100
#define CF_SCRIPT4            101
#define CF_SCRIPT5            102

//Combo types. Use with Screen->ComboT[]
#define CT_ARMOS              4
#define CT_AWARPA             94
#define CT_AWARPB             95
#define CT_AWARPC             96
#define CT_AWARPD             97
#define CT_AWARPR             98
#define CT_BLOCKALL           118
#define CT_BLOCKARROW1        111
#define CT_BLOCKARROW2        112
#define CT_BLOCKARROW3        113
#define CT_BLOCKBRANG1        114
#define CT_BLOCKBRANG2        115
#define CT_BLOCKBRANG3        116
#define CT_BLOCKFIREBALL      119
#define CT_BLOCKMAGIC         50
#define CT_BLOCKSWORDBEAM     117
#define CT_BOSSCHEST          69
#define CT_BOSSCHEST2         70
#define CT_BOSSLOCKBLOCK      61
#define CT_BOSSLOCKBLOCK2     62
#define CT_BSGRAVE            64
#define CT_BUSH               55
#define CT_BUSHC             134
#define CT_BUSHNEXT           131
#define CT_BUSHNEXTC             139
#define CT_CAVE               2
#define CT_CAVE2              51
#define CT_CAVE2B             83
#define CT_CAVE2C             84
#define CT_CAVE2D             85
#define CT_CAVEB              74
#define CT_CAVEC              75
#define CT_CAVED              76
#define CT_CENTERSTATUE       35
#define CT_CHEST              65
#define CT_CHEST2             66
#define CT_CVDOWN             15
#define CT_CVLEFT             16
#define CT_CVRIGHT            17
#define CT_CVUP               14
#define CT_DAMAGE1            31
#define CT_DAMAGE2            32
#define CT_DAMAGE3            33
#define CT_DAMAGE4            34
#define CT_DAMAGE5            120
#define CT_DAMAGE6            121
#define CT_DAMAGE7            122
#define CT_DIVEWARP           19
#define CT_DIVEWARPB          89
#define CT_DIVEWARPC          90
#define CT_DIVEWARPD          91
#define CT_DOCK               6
#define CT_EYEBALL4WAY        140
#define CT_EYEBALLA           52
#define CT_EYEBALLB           53
#define CT_FLOWERS            56
#define CT_FLOWERSC           135
#define CT_GRAVE              5
#define CT_HOOKSHOTONLY       42
#define CT_HSBRIDGE           30
#define CT_HSGRAB             29
#define CT_LADDERHOOKSHOT     20
#define CT_LADDERONLY         63
#define CT_LEFTSTATUE         11
#define CT_LOCKBLOCK          59
#define CT_LOCKBLOCK2         60
#define CT_LOCKEDCHEST        67
#define CT_LOCKEDCHEST2       68
#define CT_MAGICPRISM         48
#define CT_MAGICPRISM4WAY     49
#define CT_MIRROR             45
#define CT_MIRRORBACKSLASH    47
#define CT_MIRRORSLASH        46
#define CT_NOENEMY            110
#define CT_NOFLYZONE          44
#define CT_NOGROUNDENEMY      128
#define CT_NOJUMPZONE         54
#define CT_NONE               0
#define CT_OVERHEAD           43
#define CT_PIT                41
#define CT_PITB               80
#define CT_PITC               81
#define CT_PITD               82
#define CT_PITR               93
#define CT_POUND              28
#define CT_PUSHHEAVY          9
#define CT_PUSHHEAVY2         26
#define CT_PUSHHEAVY2WAIT     27
#define CT_PUSHHEAVYWAIT      10
#define CT_PUSHWAIT           8
#define CT_RESET              71
#define CT_RIGHTSTATUE        12
#define CT_SAVE               72
#define CT_SAVE2              73
#define CT_SCREENFREEZE       126
#define CT_SCREENFREEZEFF     127
#define CT_SCRIPT1		  142
#define CT_SCRIPT2		  143
#define CT_SCRIPT3		  144
#define CT_SCRIPT4		  145
#define CT_SCRIPT5		  146
#define CT_SHALLOWWATER       58
#define CT_SLASH              24
#define CT_SLASHC             132
#define CT_SLASHITEM          25
#define CT_SLASHITEMC         133
#define CT_SLASHNEXT          129
#define CT_SLASHNEXTC             137
#define CT_SLASHNEXTITEM      130
#define CT_SLASHNEXTITEMC             138
#define CT_SPINTILE1          124
#define CT_SPINTILE2          125
#define CT_STAIR              1
#define CT_STAIRB             77
#define CT_STAIRC             78
#define CT_STAIRD             79
#define CT_STAIRR             92
#define CT_STEP               106
#define CT_STEPALL            108
#define CT_STEPCOPY           109
#define CT_STEPSAME           107
#define CT_STRIGFLAG          105
#define CT_STRIGNOFLAG        104
#define CT_SWARPA             99
#define CT_SWARPB             100
#define CT_SWARPC             101
#define CT_SWARPD             102
#define CT_SWARPR             103
#define CT_SWIMWARP           18
#define CT_SWIMWARPB          86
#define CT_SWIMWARPC          87
#define CT_SWIMWARPD          88
#define CT_TALLGRASS          57
#define CT_TALLGRASSC         136
#define CT_TALLGRASSNEXT      141
#define CT_TRAP4WAY           38
#define CT_TRAPH              36
#define CT_TRAPLEFTRIGHT      39
#define CT_TRAPUPDOWN         40
#define CT_TRAPV              37
#define CT_TRIGFLAG           22
#define CT_TRIGNOFLAG         21
#define CT_WALKSLOW           13
#define CT_WATER              3
#define CT_ZELDA              23


//Item pickup flags. OR (|) these together to use with item->Pickup
//Other values are reserved for internal usage, and have no effect
#define IP_HOLDUP				0x002 //Link holds it up when collecting it.
#define IP_ST_ITEM			0x004 //Sets the screen state "Item" when collected.
#define IP_DUMMY				0x008 //A 'dummy' item, such as rupee markers in shops. Can't be collected and ignores gravity.
#define IP_ENEMYCARRIED		0x080 //The item-carrying NPC carries it. If no NPC has an item, the most recently created NPC carries it.
#define IP_TIMEOUT			0x100 //Disappears after 512 frames. Can be collected by melee weapons if the related quest rule is not set.
#define IP_ST_SPECIALITEM		0x800 //Sets the screen state "Special Item" when collected.

//Screen Flag categories. Flags from each category are ORed together, starting with the first flag
//See Screen Data dialog for what flags are contained in each category. Use in Screen->Flags
#define SF_ROOMTYPE			0
#define SF_VIEW				1
#define SF_SECRETS			2
#define SF_WARP				3
#define SF_ITEMS				4
#define SF_COMBOS				5
#define SF_SAVE				6
#define SF_FFC				7
#define SF_WHISTLE			8
#define SF_MISC				9

//Screen Enemy Flag categories, handled in the same way as the SF_ constants.
//Use in Screen->EFlags
#define SEF_SPAWN				0
#define SEF_LIST1				1 //left-hand list
#define SEF_LIST2				2 //right-hand list

//Room types (Screen => Room Type)
//Use with Screen->RoomType
#define RT_DOORREPAIR         5
#define RT_FEEDTHEGORIYA      7
#define RT_GAMBLE             4
#define RT_GANON              15
#define RT_ITEMPOND           17 //doesn't work
#define RT_LEARNSLASH         19
#define RT_LEVEL9ENTRANCE     8
#define RT_MAGICUPGRADE       18
#define RT_MONEYORLIFE        12
#define RT_MOREARROWS         20
#define RT_MOREBOMBS          11
#define RT_NONE               0
#define RT_PAYFORINFO         2
#define RT_POTIONORHEART      6
#define RT_POTIONSHOP         9
#define RT_SECRETMONEY        3
#define RT_SHOP               10
#define RT_SPECIALITEM        1
#define RT_STAIRWARP          14
#define RT_TAKEONEITEM        21
#define RT_TENRUPEES          13
#define RT_ZELDA              16

//DMap Flag constants. Use with Game->DMapFlags.
#define DMF_CAVESNOTCELLARS	0x0001 //Use caves instead of item cellars
#define DMF_3STAIRWARPS		0x0002 //Allow 3-stair warps
#define DMF_WWIND				0x0004 //Allow whistle whirlwinds
#define DMF_GUYCAVES			0x0008 //Special rooms and guys are in caves only
#define DMF_NOCOMPASS			0x0010 //Don't display compass marker in minimap
#define DMF_WAVY 				0x0020 //Underwater wave effect
#define DMF_WWINDRET			0x0040 //Whistle whirlwind returns Link to start
#define DMF_ALWAYSINTROMSG	0x0080 //Always display intro string
#define DMF_VIEWMAP       	0x0100 //View overworld map by pressing 'Map'
#define DMF_DMAPMAP         	0x0200 //...But only show screens marked in minimap
#define DMF_MINIMAPCOLORFIX 	0x0400 //Use minimap foreground colour 2
#define DMF_SCRIPT1 			0x0800
#define DMF_SCRIPT2 			0x1000
#define DMF_SCRIPT3 			0x2000
#define DMF_SCRIPT4 			0x4000
#define DMF_SCRIPT5 			0x8000

//NPC Attribute 1 constants. Use with npc->Attributes.
#define NPCA1_1SHOT			0
#define NPCA1_ENDHALT			1
#define NPCA1_RAPIDFIRE		2
#define NPCA1_1FAST			3
#define NPCA1_1SLANT			4
#define NPCA1_3SHOTS			5
#define NPCA1_4SHOTS			6
#define NPCA1_5SHOTS			7
#define NPCA1_3FAST			8
#define NPCA1_BREATH			9
#define NPCA1_8SHOTS			10
#define NPCA1_SUMMON			11
#define NPCA1_SUMMONLAYER		12

//NPC Attribute 2 constants. Use with npc->Attributes.
#define NPCA2_NORMAL			0
#define NPCA2_SPLITHIT		1
#define NPCA2_SPLIT			2
#define NPCA2_8SHOTS			3
#define NPCA2_EXPLODE			4
#define NPCA2_TRIBBLE			5

//NPC Attribute 7 constants. Use with npc->Attributes.
#define NPCA7_NORMAL			0
#define NPCA7_TEMPJINX		1
#define NPCA7_PERMJINX		2
#define NPCA7_CUREJINX		3
#define NPCA7_LOSEMAGIC		4
#define NPCA7_LOSERUPEES		5
#define NPCA7_DRUNK			6
#define NPCA7_EATITEMS		7
#define NPCA7_EATMAGIC		8
#define NPCA7_EATRUPEES		9

//NPC Attribute 9 constants. Use with npc->Attributes.
#define NPCA9_NORMAL			0
#define NPCA9_ROPE			1
#define NPCA9_VIRE			2
#define NPCA9_POLSVOICE		3

//NPC Defenses. Use as indices to npc->Defense[].
#define NPCD_ARROW			3
#define NPCD_BEAM				10
#define NPCD_BRANG			0
#define NPCD_BOMB				1
#define NPCD_BYRNA			16
#define NPCD_FIRE				4
#define NPCD_HAMMER			8
#define NPCD_HOOKSHOT			7
#define NPCD_MAGIC			6
#define NPCD_REFBEAM			11
#define NPCD_REFMAGIC			12
#define NPCD_REFFIREBALL		13
#define NPCD_REFROCK			14
#define NPCD_SBOMB			2
#define NPCD_SCRIPT			17
#define NPCD_STOMP			15
#define NPCD_SWORD			9
#define NPCD_WAND				5

//NPC Defense types. Use with npc->Defense[].
#define NPCDT_NONE			0
#define NPCDT_HALFDAMAGE		1
#define NPCDT_QUARTERDAMAGE	2
#define NPCDT_STUN			3
#define NPCDT_STUNORBLOCK		4
#define NPCDT_STUNORIGNORE	5
#define NPCDT_BLOCK1			6
#define NPCDT_BLOCK2			7
#define NPCDT_BLOCK4			8
#define NPCDT_BLOCK6			9
#define NPCDT_BLOCK8			10
#define NPCDT_BLOCK			11
#define NPCDT_IGNORE1			12
#define NPCDT_IGNORE			13
#define NPCDT_ONEHITKILL		14

//Draw styles. Use with item->DrawStyle
#define DS_NORMAL             0
#define DS_PHANTOM            1
#define DS_CLOAKED            2
#define DS_LINK               3

//Warp types. Use with Screen->SetSideWarp
#define WT_CAVE				0
#define WT_PASSAGE			1
#define WT_ENTRANCEEXIT		2
#define WT_SCROLLING			3
#define WT_IAWARP				4
#define WT_IWARPBLACKOUT		5
#define WT_IWARPOPENWIPE		6
#define WT_IWARPZAP			7
#define WT_IWARPWAVE		  	8
#define WT_NOWARP				9

// Font types. Use with Screen->DrawCharacter *note -some of these fonts may not represent normal ascii characters,
//and all of these are completely undocumented. Use at your own peril
#define FONT_Z1               0
#define FONT_Z3               1
#define FONT_Z3SMALL          2
#define FONT_DEF              3
#define FONT_L                4
#define FONT_L2               5
#define FONT_P                6
#define FONT_MATRIX           7
#define FONT_ZTIME            8
#define FONT_S                9
#define FONT_S2               10
#define FONT_SP               11
#define FONT_SUBSCREEN1       12
#define FONT_SUBSCREEN2       13
#define FONT_SUBSCREEN3       14
#define FONT_SUBSCREEN4       15
#define FONT_GBLA             16
#define FONT_GORON            17
#define FONT_ZORAN            18
#define FONT_HYLIAN1          19
#define FONT_HYLIAN2          20
#define FONT_HYLIAN3          21
#define FONT_HYLIAN4          22
#define FONT_GBORACLE         23
#define FONT_GBORACLEP        24
#define FONT_DSPHANTOM        25
#define FONT_DSPHANTOMP       26

// PrintString Text Formats. Use with Screen->DrawString.
#define TF_NORMAL             0 // treats the left-most char as (x)
#define TF_CENTERED           1 // prints the string centered on (x)
#define TF_RIGHT              2 // treats the right-most char as (x)

// Texture mapping rendering modes. Use with Screen->"Psuedo 3D" or "*3D" drawing routines. ie Polygon, Quad, or Triangle
#define PT_FLAT               0 // single color shading
#define PT_FLATSHADED         1 // "" with flat shading (*3D only)
#define PT_COLSHADED          2 // "" with color shading (*3D only)
#define PT_TEXTURE            3 // texture mapped polygon using 2d linear interpolation
#define PT_PTEXTURE           4 // "" 
#define PT_MASKTEXTURE        5 // transparent texture mapped polygon using 2d linear interpolation
#define PT_MASKPTEXTURE       6 // "" 
#define PT_LITTEXTURE         7 // "" with lighting (*3D only)
#define PT_LITPTEXTURE        8 // "" (*3D only)
#define PT_MASKLITTEXTURE     9 // "" with transparency
#define PT_MASKLITPTEXTURE    10 // "" 
#define PT_TRANSTEXTURE       11 // translucent texture mapped polygon using 2d linear interpolation
#define PT_TRANSPTEXTURE      12 // "" 
#define PT_MASKTRANSTEXTURE   13 // translucent and transparent texture mapped polygon using 2d linear interpolation
#define PT_MASKTRANSPTEXTURE  14 // "" 

// Opacity options. Use with drawing commands.
#define OP_TRANS              64
#define OP_OPAQUE             128

// Render Targets. Use with Screen->"*RenderTarget" and Screen->"Bitmap" drawing routines.
#define RT_SCREEN             -1 // draws to screen
#define RT_BITMAP0            0 // draws to off-screen bitmap 0
#define RT_BITMAP1            1 // draws to off-screen bitmap 1
#define RT_BITMAP2            2 // draws to off-screen bitmap 2
#define RT_BITMAP3            3 // draws to off-screen bitmap 3
#define RT_BITMAP4            4 // draws to off-screen bitmap 4
#define RT_BITMAP5            5 // draws to off-screen bitmap 5
#define RT_BITMAP6            6 // draws to off-screen bitmap 6

//Sprite extending methods. Use with Link->Extend, item->Extend,
//npc->Extend, eweapon->Extend and lweapon->Extend.
#define EXT_NONE              0 // Not extended
#define EXT_NORMAL            3 // Extended
#define EXT_NOSHADOW          4 // Extended NPC has no shadow.

//Generic counters. Use with itemclass->Counter, Game->Counter[], Game->MCounter[] and Game->DCounter[]
#define CR_LIFE               0
#define CR_RUPEES             1
#define CR_BOMBS              2
#define CR_ARROWS             3
#define CR_MAGIC              4
#define CR_KEYS               5
#define CR_SBOMBS             6
#define CR_SCRIPT1            7
#define CR_SCRIPT2            8
#define CR_SCRIPT3            9
#define CR_SCRIPT4            10
#define CR_SCRIPT5            11
#define CR_SCRIPT6            12
#define CR_SCRIPT7            13
#define CR_SCRIPT8            14
#define CR_SCRIPT9            15
#define CR_SCRIPT10           16
#define CR_SCRIPT11           17
#define CR_SCRIPT12           18
#define CR_SCRIPT13           19
#define CR_SCRIPT14           20
#define CR_SCRIPT15           21
#define CR_SCRIPT16           22
#define CR_SCRIPT17           23
#define CR_SCRIPT18           24
#define CR_SCRIPT19           25
#define CR_SCRIPT20           26
#define CR_SCRIPT21           27
#define CR_SCRIPT22           28
#define CR_SCRIPT23           29
#define CR_SCRIPT24           30
#define CR_SCRIPT25           31


//Generic data indices. Use this with Game->Generic[]
#define GEN_HEARTPIECES       0 // Heart Pieces currently possessed
#define GEN_MAGICDRAINRATE    1 // Link's magic usage equals n / 2.
#define GEN_CANSLASH          2 // Link can slash with the sword.
#define GEN_WARPLEVEL         3 // Warp Ring number that a Whirlwind warp will send Link to.
// It is set every time that Link uses the Whistle.
#define GEN_HCPPERHC          4 // How many Pieces of Heart form a complete Heart Container?
#define GEN_CONTINUEHEARTS    5 // Amount of life that Link continues with
#define GEN_CONTINUEISPERCENT 6 // ...Or a percentage of his total life

//Level item flags. Or (|) these together and use with Game->LItems[]
#define LI_TRIFORCE           0x01 // The Triforce of this Level has been obtained
#define LI_MAP                0x02 // The Dungeon Map of this Level has been obtained
#define LI_COMPASS            0x04 // The Compass of this Level has been obtained
#define LI_BOSS               0x08 // A "Dungeon Boss" enemy has been slain
#define LI_BOSSKEY            0x10 // The Boss Key of this Level has been obtained

//Screen states. Use these with Screen->State[], etc.

#define ST_DOORUP				0 // The locked/bomb door has been permanently unlocked/bombed
#define ST_DOORDOWN			1 // The locked/bomb door has been permanently unlocked/bombed
#define ST_DOORLEFT			2 // The locked/bomb door has been permanently unlocked/bombed
#define ST_DOORRIGHT			3 // The locked/bomb door has been permanently unlocked/bombed
#define ST_SECRET				13 // Screen Secrets have been made permanent
#define ST_ITEM				4 // If set, the item is gone
#define ST_SPECIALITEM		5 // If set, the Cave/Item Cellar/Dive For Item/Armos item is gone
#define ST_ENEMYNORETURN		6 // If set, enemies won't return
#define ST_TEMPNORETURN		7 // If set, enemies won't return. This is automatically unset when you leave the DMap
#define ST_LOCKBLOCK			8 // The lock block on the screen has been triggered
#define ST_BOSSLOCKBLOCK		9 // The boss lock block on the screen has been triggered
#define ST_CHEST				10 // The unlocked chest on this screen has been opened
#define ST_LOCKEDCHEST		11 // The locked chest on this screen has been opened
#define ST_BOSSCHEST			12 // The boss chest on this screen has been opened
#define ST_VISITED			14 // If set, this screen will appear on the Overworld Map

//Door types. Use with Screen->Door[]
#define D_WALL                0
#define D_OPEN                1
#define D_LOCKED              2
#define D_UNLOCKED            3 // A locked door that has been opened
#define D_SHUTTER             4 // Defeat enemies to unlock this door
#define D_BOMB                6 // A door that changes when bombed
#define D_BOMBED              7 // A door that has been bombed
#define D_WALKTHRU            8 // Push against the wall to pass through
#define D_BOSSLOCKED          10
#define D_BOSSUNLOCKED        11 // A boss door that has been opened
#define D_OPENSHUTTER         12 // A shutter that has been opened
#define D_1WAYSHUTTER         14 // A shutter that locks behind you and never opens

//Enemy projectile weapon selections. Use with npc->Weapon.
//Note: Don't use these with Screen->CreateEWeapon! Use the EW_ constants instead.
#define WPN_NONE              0
#define WPN_ENEMYFLAME        140
#define WPN_ENEMYWIND         141
#define WPN_ENEMYFIREBALL     129
#define WPN_ENEMYARROW        130
#define WPN_ENEMYBRANG        131
#define WPN_ENEMYSWORD        132
#define WPN_ENEMYROCK         133
#define WPN_ENEMYMAGIC        134
#define WPN_ENEMYBOMB         135
#define WPN_ENEMYSBOMB        136
#define WPN_ENEMYLITBOMB      137
#define WPN_ENEMYLITSBOMB     138
#define WPN_ENEMYFIRETRAIL    139
#define WPN_ENEMYFLAME2       142
#define WPN_ENEMYFLAME2TRAIL  143 // Not implemented
#define WPN_ENEMYICE          144 // Not implemented
#define WPN_ENEMYFIREBALL2    145 // A fireball that doesn't fly toward Link

//Default item drop sets. Use with npc->ItemSet
#define IS_NONE               0
#define IS_DEFAULT            1
#define IS_BOMBS              2
#define IS_MONEY              3
#define IS_LIFE               4
#define IS_BOMB100            5 // Used by Dodongos
#define IS_SBOMB100           6 // Used by Dodongos
#define IS_MAGIC              7
#define IS_MAGICBOMBS         8
#define IS_MAGICMONEY         9
#define IS_MAGICLIFE          10
#define IS_MAGIC2             11
#define IS_COMBOS				12 // Used by Tall Grass / Bush / Slash combos.

//Boss palettes. Use with npc->BossPal
#define BPAL_AQUA             0
#define BPAL_GLEEOK           1
#define BPAL_DODONGO          2
#define BPAL_GANON            3
#define BPAL_BROWN            4
#define BPAL_PILE             5  // Used for the Dust Pile.
#define BPAL_BLUE             6
#define BPAL_RED              7
#define BPAL_GOLD             8
#define BPAL_ICON1            9  // Used for Quest Icon 1.
#define BPAL_ICON2            10 // Used for Quest Icon 2.
#define BPAL_ICON3            11 // Used for Quest Icon 3.
#define BPAL_ICON4            12 // Used for Quest Icon 4.
#define BPAL_GLEEOKF          13
#define BPAL_FROZEN           14 // Unused

//Sprite lists. Use with Screen->ClearSprites()
#define SL_GUYS               0 // Enemies
#define SL_ITEMS              1 // Items
#define SL_EWPNS              2 // Enemy weapons
#define SL_LWPNS              3 // Link's weapons
#define SL_DECORATIONS        4 // Grass clippings, Hover boots glow, etc.
#define SL_PARTICLES          5 // Farore's Wind particles

//Mouse buttons
#define MB_LEFTCLICK			0x1 //AND (&) these with Link->InputMouseB to get the current button state of the mouse
#define MB_RIGHTCLICK			0x2
#define MB_MIDDLECLICK		0x4 //Keep in in mind that not all mice/touchpads have a middle mouse button

