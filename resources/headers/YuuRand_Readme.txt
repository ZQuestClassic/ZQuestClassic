YuuRand Ghost Tools

Find and replace all instances of Ghost_Checkhit wih Ghost_CheckHit_YR
and all manual calls of UpdateEWeapon with UpdateEWeapon_YR and
all manual calls of UpdateEWeapons with UpdateEWeapons_YR.

In the global active main loop, call UpdateGhostZH1_YR instead of UpdateGhostZH1.

The original versions of the following are preserved in ghost_yuurand.zh,
	but I added versions that accept array pointers, where your G[] 
	array pointer and Player-II indices are passed y reference. 

//__UpdateEWMHomingP2(eweapon wpn,(wpn, G, G_PLAYER_TWO_X, G_PLAYER_TWO_Y)
//__UpdateEWMHomingReaimP2(eweapon wpn,(wpn, G, G_PLAYER_TWO_X, G_PLAYER_TWO_Y)
//__DoEWeaponDeathAimAtP2(wp, G, G_PLAYER_TWO_X, G_PLAYER_TWO_Y

Also, //Ghost_CheckHit(this, ghost, G,  G_PLAYER_TWO_X, G_PLAYER_TWO_Y)
