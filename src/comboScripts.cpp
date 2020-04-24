//Combo Scripts v0.4
//27th Septemver, 2019
refInfo comboScriptData[176*7];
word combo_doscript[176] = {0}; //one bit per layer
byte combo_waitdraw[176] = {0}; //one bit per layer
byte combo_initialised[176] = {0}; //one bit per layer
int comboscript_combo_ids[176*7] = {0};
long combo_stack[176*7][MAX_SCRIPT_REGISTERS];

#define COMBOSCRIPT_RUNTYPE_DISABLED 0
#define COMBOSCRIPT_RUNTYPE_RUNNING 1
#define COMBOSCRIPT_RUNTYPE_CLEARING 2

//Add to refInfo class: comboposref, used for layer and positional data for special combo script functions

void init_combo_doscript()
{
	for ( int q = 0; q < 1232; q++ ) combo_doscript[q] = 1;
}
void clear_combo_refinfo()
{
	for ( int q = 0; q < 1232; q++ )
		comboScriptData[q].Clear();
}

void clear_combo_refinfo(int pos)
{
	comboScriptData[pos].Clear();
}

void clear_combo_stacks()
{
	for ( int q = 0; q < 1232; q++ )
		memset(combo_stack[q], 0, sizeof(combo_stack[q]));
}
void clear_combo_stack(int q)
{
	memset(combo_stack[q], 0, sizeof(combo_stack[q]));
}

void clear_combo_initialised()
{
	memset(combo_initialised, 0, sizeof(combo_initialised));
}

//c+176*q : This tells the interpreter which combo pos and which layer it is running.
//used for X(), Y(), Layer() and Pos()

//RunScript
case SCRIPT_COMBO:
{
	ri = &(comboScriptData[i]);

	curscript = comboscripts[script];
	stack = &(combo_stack[i]);
	int pos = ((i%176));
	int id = comboscript_combo_ids[i]; 

	if(!(combo_initialised[pos]&l))
	{
		memset(ri->d, 0, 8 * sizeof(long));
		for ( int q = 0; q < COMBOINITDMAX; q++ )
			ri->d[q] = combosbuf[m->data[id]].initD[q];
	}

	ri->combosref = id; //'this' pointer
	ri->comboposref = i; //used for X(), Y(), Layer(), and so forth.
}

//Quit
case SCRIPT_COMBO:
{
	int l = 0; //get the layer
	for int q = 176; q < 1232; q+= 176 )
	{
		if ( i < q )
		{
			break;
		}
		++l;
	}
	int pos = ((i%176));
	combo_doscript[pos] &=  ~(1<<l);
	combo_initialised[pos] &=  ~(1<<l);
	
	FFScript::deallocateAllArrays(type, i); //need to add combo arrays

	break;
}

//Clear stacks and refinfo in LOADSCR
void ClearComboScripts()
{
	for ( int c = 0; c < 176; c++ )
	{
		combo_doscript[c] = 0;
		combo_waitdraw[c] = 0;
		combo_initialised[c] = 0;
		for ( int l = 0; l < 7; l++)
		{
			if ( get_bit(quest_rules, qr_COMBOSCRIPTS_LAYER_0+l) )
			{
				comboscript_combo_ids[c+(176*l)] = 0;
				comboScriptData[c+(176*l)].Clear();
				for ( int r = 0; q < MAX_SCRIPT_REGISTERS; ++r )
				{
					combo_stack[c+(176*l)][r] = 0; //clear the stacks
				}
			}
		}
	}
}

//call in zelda.cpp
void ComboScriptEngine()
{
	
	for ( int q = 0; q < 7 ; q++ )
	{
		if (!get_bit(quest_rules, qrCOMBO_SCRIPTS_RUN_ON_LAYER_0+q)) continue;
		for ( int c = 0; c < 176; c++ )
		{
			if ( !q )
			{
				//layer 0
				//not initialised, set it up to run
				//if ( !(combo_initialised[c] & 1 ))
				//{
				//	combo_doscript[c] |= 1;
				//	combo_initialised[c] |= 1;
				//}
				if ( combosbuf[tmpscr->data[c]].script )
				{
					if ( combo_doscript[c] & 1 )
					{
						//combo_doscript[c] |= 1;
						comboscript_combo_ids[c+(176*q)] = tmpscr->data[c];
						ZScriptVersion::RunScript(SCRIPT_COMBO, combosbuf[tmpscr->data[c].script, c+(176*q));
					}
				}
			}
			else //higher layers
			{
				int ls = tmpscr->layerscreen[q];
				int lm = tmpscr->layermap[q];
				mapscr *m = &TheMaps[(zc_max((lm)-1,0) * MAPSCRS + ls)];
				if ( combosbuf[m->data[c]].script )
				{
					if ( combo_doscript[c] & (1<<q) )
					{
						combo_doscript[c] |= (1<<q);
						comboscript_combo_ids[c+(176*q)] = m->data[c];
						ZScriptVersion::RunScript(SCRIPT_COMBO, combosbuf[m->data[c]].script, c=176*q);
					}
				}
			}
		}
	}
}
				

//tba
void deallocateComboArrays();

int getComboDataLayer(int c, int scripttype)
{
	if ( scripttype != SCRIPT_COMBO )
	{
		Z_scripterrorlog("combodata->Layer() only runs from combo scripts, not from script type &s\n", scripttypenames[scripttype]);
		return -1;
	}
	else
	{
		int l = 0;
		for int q = 176; q < 1232; q+= 176 )
		{
			if ( c < q )
			{
				return l;
			}
			++l;
		}
	}
}

int getCombodataPos(int c, int scripttype)
{
	if ( scripttype != SCRIPT_COMBO )
	{
		Z_scripterrorlog("combodata->YPos() only runs from combo scripts, not from script type &s\n", scripttypenames[scripttype]);
		return -1;
	}
	else return ((c%176));
}

int getCombodataX(int c, int scripttype)
{
	if ( scripttype != SCRIPT_COMBO )
	{
		Z_scripterrorlog("combodata->X() only runs from combo scripts, not from script type &s\n", scripttypenames[scripttype]);
		return -1;
	}
	else
	{
		int pos = getCombodataPos(c);
		return COMBOX(pos);
	}
}

int getCombodataY(int c, int scripttype)
{
	if ( scripttype != SCRIPT_COMBO )
	{
		Z_scripterrorlog("combodata->Y() only runs from combo scripts, not from script type &s\n", scripttypenames[scripttype]);
		return -1;
	}
	else
	{
		int pos = getCombodataPos(c);
		return COMBOY(pos);
	}
}


```cpp

//Code for combodata script this->X() and this->Y interpreter calls:
//i is the script UID, for the combo grid pos running it
//'script' is the script number
case COMBOXR:
{
	if ( type == SCRIPT_COMBO )
	{
		ret = (( ((i)%16*16) ) * 10000); //comboscriptstack[i]
		//i is the current script number
	}
	else
	{
		Z_scripterrorlog("Combo->X() can only be called by combodata scripts, but you tried to use it from script type %s, script token %s\n", scripttypenames[type], ffcmap[script].scriptname.c_str() );
		ret = -10000;
	}
	break;
}

case COMBOYR:
{
	if ( type == SCRIPT_COMBO )
	{
		ret = (( ((i)&0xF0) ) * 10000); //comboscriptstack[i]
	}
	else
	{
		Z_scripterrorlog("Combo->X() can only be called by combodata scripts, but you tried to use it from script type %s, script token %s\n", scripttypenames[type], ffcmap[script].scriptname.c_str() );
		ret = -10000;
	}
	break;
}


