//////////////////////////////
//        Credits.zh        //
//           V1.01          //
//           Emily          //
//////////////////////////////
#option SHORT_CIRCUIT on
#option BINARY_32BIT off
#option HEADER_GUARD on
#include "std.zh"

namespace Credits //start
{
	typedef const int DEFINE;
	typedef const int CONFIG;
	bitmap _bitmap;
	untyped data[CD_SZ];
	enum
	{
		DATA_CURRY,
		DATA_SPACING,
		DATA_HEADER_SPACING,
		DATA_SIDE_BUFFER,
		DATA_FONT,
		DATA_TEXT_COLOR,
		CD_SZ
	};
	CONFIG DEFAULT_HEIGHT = 1024;
	CONFIG BACKGROUND_COLOR = 0x0F;
	CONFIG DEFAULT_TEXT_COLOR = 0x01;
	CONFIG DEFAULT_SIDE_BUFFER = 16;
	CONFIG DEFAULT_SPACING = 2;
	CONFIG DEFAULT_HEADER_SPACING = 8;
	CONFIG DEFAULT_FONT = FONT_ALLEGRO;
	CONFIG EXP_Y_AMNT = 256;
	//start Header System functions
	void init()
	{
		if(_bitmap->isValid())
			_bitmap->Free();
		_bitmap = create(256, DEFAULT_HEIGHT);
		_bitmap->Clear(0);
		data[DATA_CURRY] = 0;
		data[DATA_SPACING] = DEFAULT_SPACING;
		data[DATA_HEADER_SPACING] = DEFAULT_HEADER_SPACING;
		data[DATA_SIDE_BUFFER] = DEFAULT_SIDE_BUFFER;
		data[DATA_FONT] = DEFAULT_FONT;
		data[DATA_TEXT_COLOR] = DEFAULT_TEXT_COLOR;
	}
	void destroy() //Call when you are done rolling the credits, to delete them
	{
		if(_bitmap->isValid())
			_bitmap->Free();
		_bitmap = NULL;
		data[DATA_CURRY] = 0;
		data[DATA_SPACING] = DEFAULT_SPACING;
		data[DATA_HEADER_SPACING] = DEFAULT_HEADER_SPACING;
		data[DATA_SIDE_BUFFER] = DEFAULT_SIDE_BUFFER;
		data[DATA_FONT] = DEFAULT_FONT;
		data[DATA_TEXT_COLOR] = DEFAULT_TEXT_COLOR;
	}
	bitmap getBitmap() //The bitmap can change whenever you add a line; make sure to call this function again if you need it again!
	{
		return _bitmap;
	}
	bitmap create(int w, int h)
	{
		if ( Game->FFRules[qr_OLDCREATEBITMAP_ARGS] )
			return Game->CreateBitmap(h, w);
		else return Game->CreateBitmap(w, h);
	}
	void resize(int height)
	{
		if(_bitmap->Height == height) return;
		bitmap tmp = create(256, height);
		tmp->Clear(0);
		int h = Min(_bitmap->Height, height);
		_bitmap->Blit(0, tmp, 0, 0, 256, h, 0, 0, 256, h, 0, 0, 0, BITDX_NORMAL, 0, false);
		_bitmap->Free();
		_bitmap = tmp;
	}
	void setSpacing(int spc)
	{
		data[DATA_SPACING] = spc;
	}
	void setHeaderSpacing(int spc)
	{
		data[DATA_HEADER_SPACING] = spc;
	}
	void setSideBuffer(int spc)
	{
		data[DATA_SIDE_BUFFER] = spc;
	}
	void setFont(int font)
	{
		data[DATA_FONT] = font;
	}
	void setTextColor(int color)
	{
		data[DATA_TEXT_COLOR] = color;
	}
	void output(char32[] filepath)
	{
		bitmap tmp = create(256, data[DATA_CURRY]);
		_bitmap->Blit(7, tmp, 0, 0, 256, data[DATA_CURRY], 0, 0, 256, data[DATA_CURRY], 0, 0, 0, BITDX_NORMAL, 0, false);
		tmp->Write(7, filepath, true);
		tmp->Free();
	}
	//end
	//start Credits drawing functions
	void addSpace(int px)
	{
		data[DATA_CURRY] += px;
		if(data[DATA_CURRY] > _bitmap->Height)
		{
			resize(_bitmap->Height + EXP_Y_AMNT);
		}
	}
	void addLine(char32[] text)
	{
		addLine(text, data[DATA_FONT], TF_CENTERED, data[DATA_TEXT_COLOR], data[DATA_SPACING]);
	}
	void addLine(char32[] text, int font)
	{
		addLine(text, font, TF_CENTERED, data[DATA_TEXT_COLOR], data[DATA_SPACING]);
	}
	void addLine(char32[] text, int font, int format)
	{
		addLine(text, font, format, data[DATA_TEXT_COLOR], data[DATA_SPACING]);
	}
	void addLine(char32[] text, int font, int format, int font_color)
	{
		addLine(text, font, format, font_color, data[DATA_SPACING]);
	}
	void addHeader(char32[] text)
	{
		addLine(text, data[DATA_FONT], TF_CENTERED, data[DATA_TEXT_COLOR], data[DATA_HEADER_SPACING]);
	}
	void addHeader(char32[] text, int font)
	{
		addLine(text, font, TF_CENTERED, data[DATA_TEXT_COLOR], data[DATA_HEADER_SPACING]);
	}
	void addHeader(char32[] text, int font, int format)
	{
		addLine(text, font, format, data[DATA_TEXT_COLOR], data[DATA_HEADER_SPACING]);
	}
	void addHeader(char32[] text, int font, int format, int font_color)
	{
		addLine(text, font, format, font_color, data[DATA_HEADER_SPACING]);
	}
	void addLine(char32[] text, int font, int format, int font_color, int spc)
	{
		int x;
		switch(format)
		{
			case TF_NORMAL:
				x = data[DATA_SIDE_BUFFER];
				break;
			case TF_RIGHT:
				x = 256 - data[DATA_SIDE_BUFFER];
				break;
			case TF_CENTERED:
			default: //In case you pass a bad format?
				x = 256/2;
		}
		int fontheight = Text->FontHeight(font);
		bitmap b = _bitmap;
		if(data[DATA_CURRY] + spc + fontheight > b->Height)
		{
			resize(b->Height + EXP_Y_AMNT);
			b = _bitmap;
		}
		b->DrawString(0, x, data[DATA_CURRY] + spc, font, font_color, -1, format, text, OP_OPAQUE);
		data[DATA_CURRY] += spc + fontheight;
	}
	//end
	//start Screen drawing functions
	void drawCredits(int y)
	{
		Screen->Rectangle(7, 0, -56, 255, 175, BACKGROUND_COLOR, 1, 0, 0, 0, true, OP_OPAQUE);
		DEFINE HEI = 224 + (y < 0 ? y : 0);
		if(HEI <= 0) return;
		int desty = (y < 0 ? -y : 0) -56;
		if(y < 0) y = 0;
		_bitmap->Blit(7, RT_SCREEN, 0, y, 256, HEI, 0, desty, 256, HEI, 0, 0, 0, BITDX_NORMAL, 0, true);
	}
	
	void rollCredits(int scrollspeed)
	{
		for(int y = -224; y < data[DATA_CURRY]; y += scrollspeed)
		{
			drawCredits(y);
			for(int q = 0; q < CB_MAX; ++q) //Kill all inputs
			{
				Input->Button[q] = false;
				Input->Press[q] = false;
			}
			Waitframe();
		}
	}
	
	void blackDelay(int frames)
	{
		while(frames-- > 0)
		{
			Screen->Rectangle(7, 0, -56, 255, 175, BACKGROUND_COLOR, 1, 0, 0, 0, true, OP_OPAQUE);
			Waitframe();
		}
	}
	//end
} //end

ffc script ExampleCredits //start
{
	void run()
	{
		runTestCredits();
	}
	
	void runTestCredits()
	{
		Credits::init();
		Credits::addHeader("Author");
		Credits::addLine("Emily", FONT_Z3SMALL);
		Credits::addHeader("Graphics");
		Credits::addLine("Emily", FONT_Z3SMALL);
		Credits::addHeader("Scripts");
		Credits::addLine("Emily", FONT_Z3SMALL);
		Credits::output("CreditsBMP.png");
		Credits::blackDelay(30);
		Credits::rollCredits(.5);
		Credits::blackDelay(30);
		Credits::destroy();
		for (int i = 0; i < 10; i++)
			Waitframe();
		Quit();
	}
} //end
