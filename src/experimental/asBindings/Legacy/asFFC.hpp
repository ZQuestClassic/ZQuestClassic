
#pragma once


namespace FFCBindings
{
			case FX:
				ret = tmpscr->ffx[ri->ffcref];
				break;
			case FY:
				ret = tmpscr->ffy[ri->ffcref];
				break;
			case XD:
				ret = tmpscr->ffxdelta[ri->ffcref];
				break;
			case YD:
				ret = tmpscr->ffydelta[ri->ffcref];
				break;
			case XD2:
				ret = tmpscr->ffxdelta2[ri->ffcref];
				break;
			case YD2:
				ret = tmpscr->ffydelta2[ri->ffcref];
				break;

	int32 Data
	{
		//todo
	}

	int32 Script
	{
		//todo
	}

	int32 CSet
	{
		//todo
	}

	int32 Delay
	{
		//todo
	}
			case DATA:
				ret = tmpscr->ffdata[ri->ffcref]*10000;
				break;
			case FFSCRIPT:
				ret = tmpscr->ffscript[ri->ffcref]*10000;
				break;
			case FCSET:
				ret = tmpscr->ffcset[ri->ffcref]*10000;
				break;
			case DELAY:
				ret = tmpscr->ffdelay[ri->ffcref]*10000;
				break;

	float X
	{
		//todo
	}

	float Y
	{
		//todo
	}

	float Vx
	{
		//todo
	}

	float Vy
	{
		//todo
	}

	float Ax
	{
		//todo
	}

	float Ay
	{
		//todo
	}


		case FFFLAGSD:
			ret=((tmpscr->ffflags[ri->ffcref] >> (ri->d[0] / 10000))&1) ? 10000 : 0;
			break;

		case FFCWIDTH:
			ret=((tmpscr->ffwidth[ri->ffcref]&0x3F)+1)*10000;
			break;

		case FFCHEIGHT:
			ret=((tmpscr->ffheight[ri->ffcref]&0x3F)+1)*10000;
			break;

		case FFTWIDTH:
			ret=((tmpscr->ffwidth[ri->ffcref]>>6)+1)*10000;
			break;

		case FFTHEIGHT:
			ret=((tmpscr->ffheight[ri->ffcref]>>6)+1)*10000;
			break;

		case FFLINK:
			ret=(tmpscr->fflink[ri->ffcref])*10000;
			break;

		case FFMISCD:
			{
				int a = ri->d[0] / 10000;

				if(BC::checkMisc(a, "ffc->Misc") != SH::_NoError)
					ret = -10000;
				else
					ret = ffmisc[ri->ffcref][a];
			}
			break;


	bool Flags[]
	{
		//todo
	}

	int32 TileWidth
	{
		//todo
	}

	int32 TileHeight
	{
		//todo
	}

	int32 EffectWidth
	{
		//todo
	}

	int32 EffectHeight
	{
		//todo
	}

	int32 Link
	{
		//todo
	}

	float InitD[]
	{
		//todo
	}

	float Misc[]
	{
		//todo
	}
}

