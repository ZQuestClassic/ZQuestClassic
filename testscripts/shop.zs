const int FLAG_SHOP_ITEM_1 = 100;
const int FLAG_SHOP_ITEM_2 = 101;
const int FLAG_SHOP_ITEM_3 = 102;
const int CMB_SHOP_ITEM = 2500;
const int SHOP_PRICE_Y_OFFSET = 18;
const int SHOP_PRICE_X_OFFSET = -4;
const int SFX_BUYITEM = 63;
const int SHOP_STRING_LAYER = 2;
const int SHOP_STRING_SHADOW_X_OFFSET = 1;
const int SHOP_STRING_SHADOW_Y_OFFSET = 1;
const int SHOP_STRING_BG_COLOUR = 0x0F;
const int SHOP_STRING_FG_COLOUR = 0x01;

ffc script Automatic_Z3_Shop
{
	void run(int shop_id)
	{
		item shopitems[3]; 
		shopdata sd = Game->LoadShopData(shop_id);
		for ( int q = 0; q < 176; ++q )
		{
			//FInd the flags and place the items
			if ( Screen->ComboF[q] == FLAG_SHOP_ITEM_1 )
			{
				Screen->ComboD[q] = CMB_SHOP_ITEM;
				shopitems[0] = Screen->CreateItem(sd->Item[0]);
				shopitems[0]->X = ComboX(q);
				shopitems[0]->Y = ComboY(q);
				shopitems[0]->HitXOffset = -32768;
				shopitems[0]->PString = sd->String[0];
				shopitems[0]->PStringFlags |= 0x04;
			}
			if ( Screen->ComboF[q] == FLAG_SHOP_ITEM_2 )
			{
				Screen->ComboD[q] = CMB_SHOP_ITEM;
				shopitems[1] = Screen->CreateItem(sd->Item[1]);
				shopitems[1]->X = ComboX(q);
				shopitems[1]->Y = ComboY(q);
				shopitems[1]->HitXOffset = -32768;
				shopitems[1]->PString = sd->String[1];
				shopitems[1]->PStringFlags |= 0x04;
				
			}
			if ( Screen->ComboF[q] == FLAG_SHOP_ITEM_3 )
			{
				Screen->ComboD[q] = CMB_SHOP_ITEM;
				shopitems[2] = Screen->CreateItem(sd->Item[2]);
				shopitems[2]->X = ComboX(q);
				shopitems[2]->Y = ComboY(q);
				shopitems[2]->HitXOffset = -32768;
				shopitems[2]->PString = sd->String[2];
				shopitems[2]->PStringFlags |= 0x04;
			}
		}
		while(1)
		{
			
			//Draw the prices
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[0]->X+SHOP_PRICE_X_OFFSET+SHOP_STRING_SHADOW_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET+SHOP_STRING_SHADOW_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_BG_COLOUR,
				0, 0, Game->GetMessage(sd->String[0]), OP_OPAQUE);
				
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[0]->X+SHOP_PRICE_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_FG_COLOUR,
				0, 0, Game->GetMessage(sd->String[0]), OP_OPAQUE);
			
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[1]->X+SHOP_PRICE_X_OFFSET+SHOP_STRING_SHADOW_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET+SHOP_STRING_SHADOW_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_BG_COLOUR,
				0, 0, Game->GetMessage(sd->String[1]), OP_OPAQUE);
				
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[1]->X+SHOP_PRICE_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_FG_COLOUR,
				0, 0, Game->GetMessage(sd->String[1]), OP_OPAQUE);
				
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[2]->X+SHOP_PRICE_X_OFFSET+SHOP_STRING_SHADOW_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET+SHOP_STRING_SHADOW_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_BG_COLOUR,
				0, 0, Game->GetMessage(sd->String[2]), OP_OPAQUE);
				
			Screen->DrawString( SHOP_STRING_LAYER, shopitems[2]->X+SHOP_PRICE_X_OFFSET,
				shopitems[0]->Y+SHOP_PRICE_Y_OFFSET, FONT_Z3_SMALL, SHOP_STRING_FG_COLOUR,
				0, 0, Game->GetMessage(sd->String[2]), OP_OPAQUE);
			
			for ( int q = 0; q < 3; ++q )
			{
				if ( PressedBuyButton() )
				{
					if ( Link->Dir == DIR_UP )
					{
						if ( Game->Counter[CR_RUPEES] >= sd->Price[q] )
						{
							if ( Below(shopitems[q]) ) 
							{
								if ( DistXY(shopitems[q], 12) )
								{
									Audio->PlaySound(SFX_BUYITEM);
									Game->DCounter[CR_RUPEES] += sd->Price[q];
									shopitems[q]->Pickup = IP_HOLDUP; 
									shopitems[q]->HitXOffset = 0; 
									shopitems[q]->X = Link->X; 
									shopitems[q]->Y = Link->Y; 
								}
							}
						}
					}
				}
			}
			Waitframe();
		}
	}
}
			