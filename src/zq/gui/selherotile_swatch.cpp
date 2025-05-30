#include "selherotile_swatch.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include <cassert>
#include <utility>
#include "hero_tiles.h"
#include "tiles.h"
#include "zq/zq_tiles.h"
#include <allegro/internal/aintern.h>

extern int32_t ex;
extern int32_t hero_animation_speed;
extern int32_t liftspeed;

namespace GUI
{
	int32_t newg_selherotile_proc(int32_t msg, DIALOG* d, int32_t)
	{
		GUI::SelHeroTileSwatch& swatch = *(GUI::SelHeroTileSwatch*)d->dp3;
		int32_t oldtile = 0;
		int32_t oldflip = 0;

		switch (msg)
		{
		case MSG_START:
		{
			herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);
			break;
		}

		case MSG_CLICK:
		{
			int32_t cs = 6;
			herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);

			switch (swatch.extend)
			{
			case 0:
				if (!isinRect(gui_mouse_x(), gui_mouse_y(), d->x + 2 + 8, d->y + 2 + 4, d->x + 32 + 8 + 2, d->y + (16 + 32) + 2))
					return D_O_K;
				break;

			case 1:
				if (!isinRect(gui_mouse_x(), gui_mouse_y(), d->x + 2 + 8, d->y + 2 + 4, d->x + (32) + 8 + 2, d->y + (4 + 64) + 2))
					return D_O_K;
				break;

			case 2:
				if (!isinRect(gui_mouse_x(), gui_mouse_y(), d->x + 2 + 8, d->y + 4, d->x + (64) + 8 + 2, d->y + (4 + 64) + 2))
					return D_O_K;
				break;
			}

			if ((CHECK_CTRL_CMD)
				? select_tile_2(swatch.tile, swatch.flip, 2, cs, false, swatch.extend, true)
				: select_tile(swatch.tile, swatch.flip, 2, cs, false, swatch.extend, true))
			{
				swatch.extend = ex;
				setherotile(swatch.tile, swatch.flip, swatch.extend, d->d2, d->d1);
				return D_REDRAW;
			}
			break;
		}

		case MSG_VSYNC:
		{
			oldtile = swatch.tile;
			oldflip = swatch.flip;
			swatch.clock++;
			auto lspeed = vbound(liftspeed, 1, 255);

			switch (zinit.heroAnimationStyle)
			{
			case las_original:                                             //2-frame
				switch (d->d2)
				{
				case ls_charge:
				case ls_walk:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 6)
					{
						if (d->d1 == up && d->d2 == ls_walk)
						{
							swatch.flip = 1;                //h flip
						}
						else
						{
							swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;               //tile++
						}
					};

					if (swatch.clock >= 11)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswim:
				case ls_sideswimcharge:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 6)
					{
						if (d->d1 == up && d->d2 == ls_sideswim)
						{
							swatch.flip = 1;				//h flip
						}
						else
						{
							swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;			   //tile++
						}
					};

					if (swatch.clock >= 11)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswimslash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;				  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_sideswimstab:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;				  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_sideswimpound:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 30)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 31)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_jump:
				{
					if (swatch.clock >= 24)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 36)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					}

					break;
				}

				case ls_slash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_slash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_revslash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_revslash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_stab:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_pound:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_pound, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 30)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 31)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_float:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 12)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 23)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_swim:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_swim, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 12)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 23)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_dive:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_dive, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 50)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					if ((swatch.clock / 12) & 1)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_drown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_drown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					if ((swatch.clock / 12) & 1)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}
				case ls_sidedrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sidedrown, d->d1, zinit.heroAnimationStyle);


					if ((swatch.clock / 12) & 1)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					};
					break;
				}

				case ls_lavadrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_lavadrown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					if ((swatch.clock / 12) & 1)
					{
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++
					};

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_falling:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_falling, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock % 70) / 10) * (swatch.extend == 2 ? 2 : 1);
					break;
				}

				case ls_landhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_landhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_cast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_cast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					};

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswimcast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sideswimcast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					};

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_lifting:
				{
					herotile(&swatch.tile, &swatch.flip, ls_lifting, d->d1, zinit.heroAnimationStyle);
					size_t fr = swatch.clock / lspeed;
					if (fr >= swatch.frames)
					{
						fr = 0;
						swatch.clock = -1;
					}
					swatch.tile += fr * (swatch.extend == 2 ? 2 : 1);
					break;
				}
				case ls_liftwalk:
				{
					herotile(&swatch.tile, &swatch.flip, ls_liftwalk, d->d1, zinit.heroAnimationStyle);
					if (swatch.clock >= 6)
					{
						if (d->d1 == up)
						{
							swatch.flip = 1;                //h flip
						}
						else
						{
							swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;               //tile++
						}
					};

					if (swatch.clock >= 11)
					{
						swatch.clock = -1;
					}
					break;
				}

				default:
					break;
				}

				break;

			case las_bszelda:                                             //3-frame BS
				switch (d->d2)
				{
				case ls_charge:
				case ls_walk:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 27)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswim:
				case ls_sideswimcharge:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 27)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswimslash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;				  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_sideswimstab:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;				  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_sideswimpound:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 30)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, ls_sideswim, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 31)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_jump:
				{
					if (swatch.clock >= 24)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 36)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					}

					break;
				}

				case ls_slash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_slash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_revslash:
				{
					if (swatch.clock < 6)
					{
						herotile(&swatch.tile, &swatch.flip, ls_revslash, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_stab:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 13)
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
						swatch.extend == 2 ? swatch.tile += 2 : swatch.tile++;                  //tile++

						if (swatch.clock >= 16)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_pound:
				{
					if (swatch.clock < 12)
					{
						herotile(&swatch.tile, &swatch.flip, ls_pound, d->d1, zinit.heroAnimationStyle);
					}
					else if (swatch.clock < 30)
					{
						herotile(&swatch.tile, &swatch.flip, ls_stab, d->d1, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, ls_walk, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 31)
						{
							swatch.clock = -1;
						}
					};

					break;
				}

				case ls_float:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 55)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_swim:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_swim, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 55)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_dive:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_dive, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 50)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_drown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_drown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sidedrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sidedrown, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					};


					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_lavadrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_lavadrown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_falling:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_falling, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock % 70) / 10) * (swatch.extend == 2 ? 2 : 1);
					break;
				}

				case ls_landhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_landhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_cast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_cast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					}

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswimcast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sideswimcast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					};

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_lifting:
				{
					herotile(&swatch.tile, &swatch.flip, ls_lifting, d->d1, zinit.heroAnimationStyle);
					size_t fr = swatch.clock / lspeed;
					if (fr >= swatch.frames)
					{
						fr = 0;
						swatch.clock = -1;
					}
					swatch.tile += fr * (swatch.extend == 2 ? 2 : 1);
					break;
				}
				case ls_liftwalk:
				{
					herotile(&swatch.tile, &swatch.flip, ls_liftwalk, d->d1, zinit.heroAnimationStyle);
					swatch.tile += anim_3_4(swatch.clock, 7) * (swatch.extend == 2 ? 2 : 1);

					if (swatch.clock >= 27)
					{
						swatch.clock = -1;
					}
					break;
				}

				default:
					break;
				}
				break;

			case las_zelda3slow:                                         //multi-frame Zelda 3 (slow)
			case las_zelda3:                                             //multi-frame Zelda 3
				switch (d->d2)
				{
				case ls_charge:
				case ls_walk:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= (64 * (hero_animation_speed)))
					{
						swatch.tile += (swatch.extend == 2 ? 2 : 1);

						int32_t l = ((swatch.clock / hero_animation_speed) & 15);
						l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
						swatch.tile += (l / 2) * (swatch.extend == 2 ? 2 : 1);

						//swatch.tile+=(((swatch.clock>>2)%8)*(swatch.extend==2?2:1));
						if (swatch.clock >= 255)
						{
							swatch.clock = -1;
						}
					}

					break;
				}
				case ls_sideswim:
				case ls_sideswimcharge:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= (64 * (hero_animation_speed)))
					{
						swatch.tile += (swatch.extend == 2 ? 2 : 1);

						int32_t l = ((swatch.clock / hero_animation_speed) & 15);
						l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
						swatch.tile += (l / 2) * (swatch.extend == 2 ? 2 : 1);

						//swatch.tile+=(((swatch.clock>>2)%8)*(swatch.extend==2?2:1));
						if (swatch.clock >= 255)
						{
							swatch.clock = -1;
						}
					}

					break;

				case ls_sideswimstab:
				{
					if (swatch.clock > 35)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswimstab, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 3) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_sideswimslash:
				{
					if (swatch.clock > 23)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswimslash, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 6) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_sideswimpound:
				{
					if (swatch.clock > 35)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswimpound, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 3) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sideswim, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_jump:
				{
					if (swatch.clock >= 24)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);

						if (swatch.clock >= 36)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					}

					break;
				}

				case ls_slash:
				{
					if (swatch.clock > 23) //24 frames, advances by one every 4 frames, 6 tiles total
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_slash, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 6) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_revslash:
				{
					if (swatch.clock > 23) //24 frames, advances by one every 4 frames, 6 tiles total
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_revslash, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 6) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_stab: //12 frames, advances by one every 4 frames, 3 tiles total.
					if (swatch.clock > 35)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_stab, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 3) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_pound:
				{
					if (swatch.clock > 35)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_pound, d->d1, zinit.heroAnimationStyle);
						swatch.tile += (((swatch.clock >> 2) % 3) * (swatch.extend == 2 ? 2 : 1));

						if (swatch.clock >= 47)
						{
							swatch.clock = -1;
						}
					}
					else
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_walk, d->d1, zinit.heroAnimationStyle);
					}

					break;
				}

				case ls_float:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock / 6) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock >= 23)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_swim:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_swim, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock / 12) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock >= 47)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_dive:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_dive, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= 50)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += ((swatch.clock / 6) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_drown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_drown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += ((swatch.clock / 6) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sidedrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_sidedrown, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock / 6) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_jump, d->d1, zinit.heroAnimationStyle);
						swatch.tile += swatch.extend == 2 ? ((int32_t)swatch.clock / 8) * 2 : ((int32_t)swatch.clock / 8);
					};


					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_lavadrown:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_lavadrown, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock <= 4)
					{
						herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_float, d->d1, zinit.heroAnimationStyle);
					};

					swatch.tile += ((swatch.clock / 6) % 4) << (swatch.extend == 2 ? 1 : 0);

					if (swatch.clock >= 81)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_falling:
				{
					herotile(&swatch.tile, &swatch.flip, &swatch.extend, ls_falling, d->d1, zinit.heroAnimationStyle);
					swatch.tile += ((swatch.clock % 70) / 10) * (swatch.extend == 2 ? 2 : 1);
					break;
				}

				case ls_landhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_landhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_waterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_waterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold1:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold1, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_sidewaterhold2:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					break;
				}

				case ls_cast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_cast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_landhold2, d->d1, zinit.heroAnimationStyle);
					};

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_sideswimcast:
				{
					herotile(&swatch.tile, &swatch.flip, ls_sideswimcast, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock < 96)
					{
						herotile(&swatch.tile, &swatch.flip, ls_sidewaterhold2, d->d1, zinit.heroAnimationStyle);
					};

					if (swatch.clock >= 194)
					{
						swatch.clock = -1;
					}

					break;
				}

				case ls_lifting:
				{
					herotile(&swatch.tile, &swatch.flip, ls_lifting, d->d1, zinit.heroAnimationStyle);
					size_t fr = swatch.clock / lspeed;
					if (fr >= swatch.frames)
					{
						fr = 0;
						swatch.clock = -1;
					}
					swatch.tile += fr * (swatch.extend == 2 ? 2 : 1);
					break;
				}
				case ls_liftwalk:
				{
					herotile(&swatch.tile, &swatch.flip, ls_liftwalk, d->d1, zinit.heroAnimationStyle);

					if (swatch.clock >= (64 * (hero_animation_speed)))
					{
						swatch.tile += (swatch.extend == 2 ? 2 : 1);

						int32_t l = ((swatch.clock / hero_animation_speed) & 15);
						l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
						swatch.tile += (l / 2) * (swatch.extend == 2 ? 2 : 1);

						if (swatch.clock >= 255)
						{
							swatch.clock = -1;
						}
					}
					break;
				}

				default:
					break;
				}
				break;

			default:
				break;
			}

			if ((swatch.tile != oldtile) || (swatch.flip != oldflip))
			{
				d->flags |= D_DIRTY;
			}

			break;
		}

		case MSG_DRAW:
		{
			BITMAP* buf = create_bitmap_ex(8, 1, 1);
			BITMAP* buf2 = buf;
			//herotile(&swatch.tile, &swatch.flip, &swatch.extend, d->d2, d->d1, zinit.heroAnimationStyle);
			int32_t w = 16;
			int32_t h = 16;

			switch (swatch.extend)
			{
			case 0: //16x16
				break;

			case 1: //16x32
				w = 16;
				h = 32;
				break;

			case 2: //32x32
				w = 32;
				h = 32;
				break;

			default:
				break;
			}

			buf = create_bitmap_ex(8, w, h);

			w *= 2;
			h *= 2;

			BITMAP* bigbmp = create_bitmap_ex(8, w + 4, h + 4);

			if (buf && bigbmp)
			{
				clear_to_color(buf, 0);

				switch (swatch.extend)
				{
				case 0:
					overtile16(buf, swatch.tile, 0, 0, 6, swatch.flip);
					stretch_blit(buf, bigbmp, 0, 0, 16, 16, 2, 2, w, h);
					jwin_draw_frame(bigbmp, 0, 0, w + 4, h + 4, FR_DEEP);
					blit(bigbmp, screen, 0, 0, d->x + 4, d->y + 4, w + 4, h + 4);
					break;

				case 1:
					overtile16(buf, swatch.tile - TILES_PER_ROW, 0, 0, 6, swatch.flip);
					overtile16(buf, swatch.tile, 0, 16, 6, swatch.flip);
					stretch_blit(buf, bigbmp, 0, 0, 16, 32, 2, 2, w, h);
					jwin_draw_frame(bigbmp, 0, 0, w + 4, h + 4, FR_DEEP);
					blit(bigbmp, screen, 0, 0, d->x + 4, d->y + 4, w + 4, h + 4);
					break;

				case 2:
					overtile16(buf, swatch.tile - TILES_PER_ROW, 8, 0, 6, swatch.flip); //top middle
					overtile16(buf, swatch.tile - TILES_PER_ROW - (swatch.flip ? -1 : 1), -8, 0, 6, swatch.flip); //top left
					overtile16(buf, swatch.tile - TILES_PER_ROW + (swatch.flip ? -1 : 1), 8 + 16, 0, 6, swatch.flip); //top right
					overtile16(buf, swatch.tile - (swatch.flip ? -1 : 1), -8, 16, 6, swatch.flip); //bottom left
					overtile16(buf, swatch.tile + (swatch.flip ? -1 : 1), 8 + 16, 16, 6, swatch.flip); //bottom right
					overtile16(buf, swatch.tile, 8, 16, 6, swatch.flip);              //bottom middle
					stretch_blit(buf, bigbmp, 0, 0, 32, 32, 2, 2, w, h);
					jwin_draw_frame(bigbmp, 0, 0, w + 4, h + 4, FR_DEEP);
					blit(bigbmp, screen, 0, 0, d->x + 4, d->y + 4, w + 4, h + 4);
					break;

				default:
					break;
				}

				destroy_bitmap(bigbmp);
				destroy_bitmap(buf);
				destroy_bitmap(buf2);
			}
			break;
		}
		}

		return D_O_K;
	}

	SelHeroTileSwatch::SelHeroTileSwatch() :
		clock(0), tile(0), flip(0), extend(0), frames(0),
		heroSprite(0), dir(0), alDialog()
	{
		Size s = 64_px + 4_px;
		setPreferredWidth(s);
		setPreferredHeight(s);
	}

	void SelHeroTileSwatch::click()
	{
		if (alDialog && allowDraw())
		{
			if (alDialog.message(MSG_CLICK, 0) & D_REDRAW)
			{
				pendDraw();
			}
		}
	}

	void SelHeroTileSwatch::setHeroSprite(int32_t value)
	{
		heroSprite = value;
		if (alDialog)
		{
			alDialog->d1 = value;
			pendDraw();
		}
	}

	void SelHeroTileSwatch::setDir(int32_t value)
	{
		dir = value;
		if (alDialog)
		{
			alDialog->d2 = value;
			pendDraw();
		}
	}

	void SelHeroTileSwatch::setFrames(int32_t value)
	{
		frames = value;
		if (alDialog)
		{
			pendDraw();
		}
	}

	int32_t SelHeroTileSwatch::getHeroSprite()
	{
		return alDialog ? alDialog->d1 : heroSprite;
	}

	int32_t SelHeroTileSwatch::getDir()
	{
		return alDialog ? alDialog->d2 : dir;
	}

	int32_t SelHeroTileSwatch::getFrames()
	{
		return frames;
	}

	void SelHeroTileSwatch::applyVisibility(bool visible)
	{
		Widget::applyVisibility(visible);
		if (alDialog) alDialog.applyVisibility(visible);
	}

	void SelHeroTileSwatch::applyDisabled(bool dis)
	{
		Widget::applyDisabled(dis);
		if (alDialog) alDialog.applyDisabled(dis);
	}

	void SelHeroTileSwatch::realize(DialogRunner& runner)
	{
		Widget::realize(runner);
		
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<newg_selherotile_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0,
			getFlags(),
			dir, heroSprite, // d1, d2,
			nullptr, nullptr, this // dp, dp2, dp3
			});
	}

	int32_t SelHeroTileSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
	{
		return -1;
	}
}