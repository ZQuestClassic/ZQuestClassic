
#ifndef _zc_script_drawing_h
#define _zc_script_drawing_h

#include <vector>
#include <string>


#define MAX_SCRIPT_DRAWING_COMMANDS 1000
#define SCRIPT_DRAWING_COMMAND_VARIABLES 20


//simple class that manages allegro bitmaps for script drawing
class ScriptDrawingBitmapPool
{
public:
	ScriptDrawingBitmapPool()
	{
	}

	void Dispose()
	{
		if(_parent_bmp)
			destroy_bitmap(_parent_bmp), _parent_bmp = 0;
	}

	inline BITMAP* AquireSubBitmap(int w, int h) { return AquireSubBitmap(0, 0, w, h); }
	inline BITMAP* AquireSubBitmap(int x, int y, int w, int h)
	{
		//todo: can currently only partition out one bitmap at a time.
		if(!_parent_bmp)
			_parent_bmp  = create_bitmap_ex(8,512,512);

		BITMAP* bmp = _parent_bmp;
		if( w <= 256 && h <= 256 )
			bmp = create_sub_bitmap(_parent_bmp, x, y, w, h);

		clear_bitmap(bmp);

		return bmp;
	}

	inline void ReleaseSubBitmap(BITMAP* b)
	{
		if( is_sub_bitmap(b) || b != _parent_bmp )
			destroy_bitmap(b);
	}


protected:
	static BITMAP *_parent_bmp;
	//static int _rc = 0;
};


class DrawingContainer
{
public:
	DrawingContainer() : drawstring(), current_string_count(0) {}
	~DrawingContainer()
	{
		this->Dispose();
	}

	void Dispose()
	{
		for( size_t i(0); i < drawstring.size(); ++i )
			delete drawstring[i];

		drawstring.clear();
	}

	void Clear()
	{
		current_string_count = 0;
	}

	std::string* GetString()
	{
		std::string* str;
		if( drawstring.size() > current_string_count )
		{
			str = drawstring[current_string_count];
			str->clear();
		}
		else
		{
			str = new std::string();
			drawstring.push_back(str);
		}

		current_string_count++;

		return str;
	}

protected:
	std::vector<std::string*> drawstring;
	size_t current_string_count;
};




class CScriptDrawingCommandVars
{
public:
	CScriptDrawingCommandVars()
	{
		this->Clear();
	}

	void Clear()
	{
		memset( (void*)this, 0, sizeof(CScriptDrawingCommandVars) );
	}

	void SetString( std::string* str )
	{
		ptr = (void*)str;
	}

	void* GetPtr()
	{
		return ptr;
	}

	int &operator [] ( const int i ) { return data[i]; }
	const int &operator [] ( const int i ) const { return data[i]; }

protected:
	int data[ SCRIPT_DRAWING_COMMAND_VARIABLES ];
	void* ptr; //will be changed later
};



class CScriptDrawingCommands
{
public:
	typedef CScriptDrawingCommandVars value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::vector<value_type> vec_type;
	typedef vec_type ::iterator vec_type_iter;

	// Unlikely people will be using all 1000 commands.
	const static int DefaultCapacity = 196;

	CScriptDrawingCommands() : commands(), count(0) {}
	~CScriptDrawingCommands(){}

	void Dispose()
	{
		bitmap_pool.Dispose();
	}

	void Init()
	{
		// Interesting bit here; legacy quests have no need for script drawing,
		// therefore zc should not allocate memory for them. We can instead
		// do that when zc first queries for a valid command index. -Gleeok

		//commands.resize(DefaultCapacity);
	}

	void Clear()
	{
		if( commands.empty() )
			return;

		//only clear what was used.
		memset( (void*)&commands[0], 0, count * sizeof(CScriptDrawingCommandVars) );
		count = 0;

		draw_container.Clear();
	}

	int Count() const { return count; }
	std::string* GetString() { return draw_container.GetString(); }

	int GetNext()
	{
		const int next_index = count;
		const int capacity = commands.capacity();

		if( ++count > capacity )
		{
			if( capacity < MAX_SCRIPT_DRAWING_COMMANDS )
			{
				if( commands.empty() )
				{
					//first use
					commands.resize(DefaultCapacity);
				}
				else
				{
					//might as well just max it out
					commands.resize(MAX_SCRIPT_DRAWING_COMMANDS);
				}
			}
			else
			{
				return -1;
			}
		}

		return next_index;
	}

	reference operator [] ( const int i ) { return commands[i]; }
	const_reference operator [] ( const int i ) const { return commands[i]; }


	inline BITMAP* AquireSubBitmap(int w, int h)
	{
		return bitmap_pool.AquireSubBitmap(w,h);
	}

	inline void ReleaseSubBitmap(BITMAP* b)
	{
		bitmap_pool.ReleaseSubBitmap(b);
	}

protected:
	vec_type commands;
	int count;

	DrawingContainer draw_container;
	ScriptDrawingBitmapPool bitmap_pool;

private:
	CScriptDrawingCommands(const CScriptDrawingCommands&) {}
	// Warning: no return statement in function returning non-void
	//CScriptDrawingCommands& operator =(const CScriptDrawingCommands&) {}
};


extern CScriptDrawingCommands script_drawing_commands;




#endif

