#ifndef ZC_SCRIPT_DRAWING_H_
#define ZC_SCRIPT_DRAWING_H_

#include "core/zdefs.h"
#include <utility>
#include <vector>
#include <string>
#include <set>

#define MAX_SCRIPT_DRAWING_COMMANDS 10000
#define SCRIPT_DRAWING_COMMAND_VARIABLES 20

#define DRAWCMD_MAX_ARG_COUNT  17
#define DRAWCMD_BMP_TARGET     18
#define DRAWCMD_CURRENT_TARGET 19

void do_primitives(BITMAP *bmp, int32_t type, int32_t target_sprite_uid = 0);
void do_primitives(BITMAP *bmp, int32_t type, int32_t x, int32_t y, int32_t target_sprite_uid = 0);

// (bitmap_id, is_user_bitmap)
std::pair<int, bool> resolveScriptingBitmapId(int scripting_bitmap_id);

// For Quad and Triangle. *allegro Bug-Fix* -Gleeok
class SmallBitmapTextureCache
{
public:
    SmallBitmapTextureCache()
    {
        is_init = false;
        
        for(int32_t i(0); i < 4; ++i)
            for(int32_t j(0); j < 4; ++j)
                _bmp[i][j] = 0;
    }
    
    void Init()
    {
        if(is_init)
            return;
            
        int32_t size[4] = { 16, 32, 64, 128 };
        
        for(int32_t i(0); i < 4; ++i)
        {
            for(int32_t j(0); j < 4; ++j)
                _bmp[i][j] = create_bitmap_ex(8, size[i], size[j]);
        }
        
        is_init = true;
    }
    
    ~SmallBitmapTextureCache()
    {
        if (system_driver)
            Dispose();
    }
    
    void Dispose()
    {
        for(int32_t i(0); i < 4; ++i)
            for(int32_t j(0); j < 4; ++j)
            {
                if(_bmp[i][j])
                    destroy_bitmap(_bmp[i][j]);
                    
                _bmp[i][j] = 0;
            }
            
        is_init = false;
    }
    
    inline BITMAP* GetTexture(int32_t bw, int32_t bh)
    {
        BITMAP* ret = 0;
        
        if(!is_init)
            Init();
            
        if(bw == 1 || bw == 2 || bw == 4 || bw == 8)
        {
            int32_t x = bw >> 1;
            
            if(x > 3) x = 3;
            
            if(bh == 1) ret = _bmp[x][0];
            
            if(bh == 2) ret = _bmp[x][1];
            
            if(bh == 4) ret = _bmp[x][2];
            
            if(bh == 8) ret = _bmp[x][3];
        }
        
        if(ret)
            clear_bitmap(ret);
            
        return ret;
    }
    
protected:
    BITMAP* _bmp[4][4];
    bool is_init;
};


//simple class that manages allegro bitmaps for script drawing
class ScriptDrawingBitmapPool
{
public:
    ScriptDrawingBitmapPool()
    {
    }
	
	~ScriptDrawingBitmapPool()
	{
		Dispose();
	}
    
    void Dispose()
    {
        if(_parent_bmp && system_driver)
            destroy_bitmap(_parent_bmp), _parent_bmp = 0;
    }
    
    inline BITMAP* AquireSubBitmap(int32_t w, int32_t h)
    {
        return AquireSubBitmap(0, 0, w, h);
    }
    inline BITMAP* AquireSubBitmap(int32_t x, int32_t y, int32_t w, int32_t h)
    {
        //todo: can currently only partition out one bitmap at a time.
        if(!_parent_bmp)
            _parent_bmp  = create_bitmap_ex(8,512,512);
            
        BITMAP* bmp = _parent_bmp;
        
        if(w <= 512 && h <= 512)
            bmp = create_sub_bitmap(_parent_bmp, x, y, w, h);
            
        clear_bitmap(bmp);
        
        return bmp;
    }
    
    inline void ReleaseSubBitmap(BITMAP* b)
    {
        if(is_sub_bitmap(b) || b != _parent_bmp)
            destroy_bitmap(b);
    }
    
    
protected:
    static BITMAP *_parent_bmp;
    //static int32_t _rc = 0;
};


class DrawingContainer
{
public:
    DrawingContainer() :
        drawstring(), current_string_count(0),
        drawdata(), current_drawdata_count(0)
    {}
    
    ~DrawingContainer()
    {
        this->Dispose();
    }
    
    void Dispose()
    {
        for(size_t i(0); i < drawstring.size(); ++i)
            delete drawstring[i];
            
        drawstring.clear();
        
        
        for(size_t i(0); i < drawdata.size(); ++i)
            delete drawdata[i];
            
        drawdata.clear();
    }
    
    void Clear()
    {
        current_string_count = 0;
        current_drawdata_count = 0;
    }

    // Moves every pooled allocation into 'other', leaving this container empty.
    // Queued commands hold raw pointers into the pool, so ownership must travel
    // together with the commands (see pop_commands/push_commands).
    void give_to(DrawingContainer& other)
    {
        if(this == &other)
            return;
        other.drawstring.insert(other.drawstring.end(), drawstring.begin(), drawstring.end());
        other.drawdata.insert(other.drawdata.end(), drawdata.begin(), drawdata.end());
        // Don't hand these slots out for reuse until the next Clear(), since the
        // commands transferred along with them may still reference them.
        other.current_string_count = other.drawstring.size();
        other.current_drawdata_count = other.drawdata.size();
        drawstring.clear();
        drawdata.clear();
        current_string_count = 0;
        current_drawdata_count = 0;
    }

    std::string* GetString()
    {
        std::string* str;
        
        if(drawstring.size() > current_string_count)
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
    
    std::vector<int32_t>* GetVector()
    {
        std::vector<int32_t>* v;
        
        if(drawdata.size() > current_drawdata_count)
        {
            v = drawdata[current_drawdata_count];
            v->clear();
        }
        else
        {
            v = new std::vector<int32_t>();
            drawdata.push_back(v);
        }
        
        current_drawdata_count++;
        
        return v;
    }
    
protected:
    //for drawstring
    std::vector<std::string*> drawstring;
    size_t current_string_count;
    
    //for other dynamic drawing (quad3d etc..)
    std::vector<std::vector<int32_t>*> drawdata;
    size_t current_drawdata_count;
    
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
        memset((void*)this, 0, sizeof(CScriptDrawingCommandVars));
    }
    
    void SetString(std::string* str)
    {
        ptr = (void*)str;
    }
    
    void SetVector(std::vector<int32_t>* v)
    {
        ptr = (void*)v;
    }
    
    void* GetPtr()
    {
        return ptr;
    }
    
    int32_t &operator [](const int32_t i)
    {
        return data[i];
    }
    const int32_t &operator [](const int32_t i) const
    {
        return data[i];
    }
    
protected:
    int32_t data[ SCRIPT_DRAWING_COMMAND_VARIABLES ];
    void* ptr; //will be changed later

public:
	DrawOrigin draw_origin;
	int32_t draw_origin_target;

	DrawOrigin secondary_draw_origin;
	int32_t secondary_draw_origin_target;
	
	int32_t sprite_draw_target_ref;

	// Script metadata captured at queue time so deferred draw errors
	// log the originating script rather than whatever ran last.
	ScriptType script_type;
	word script_num;
	int32_t script_index;
	int32_t pc;
	bool script_funcrun;
	uint32_t thiskey;
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
    const static int32_t DefaultCapacity = 256; //176 + some extra
    
    CScriptDrawingCommands() : count(0), commands() {}
    ~CScriptDrawingCommands() {}
    int32_t GetCount();
    void Dispose()
    {
        bitmap_pool.Dispose();
        small_tex_cache.Dispose();
    }
    
    void Init()
    {
        // Interesting bit here; legacy quests have no need for script drawing,
        // therefore zc should not allocate memory for them. We can instead
        // do that when zc first queries for a valid command index. -Gleeok
        
        //commands.resize(DefaultCapacity);
        //small_tex_cache.Init();
    }
    
    void Clear();
    
    int32_t Count() const
    {
        return count;
    }
    std::string* GetString()
    {
        return draw_container.GetString();
    }
    std::vector<int32_t>* GetVector()
    {
        return draw_container.GetVector();
    }
    
    int32_t GetNext()
    {
        if(count>=MAX_SCRIPT_DRAWING_COMMANDS)
            return -1;
        
        const int32_t next_index = count;
        const int32_t capacity = commands.capacity();
        
        if(++count > capacity)
        {
            if(commands.empty())
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
        
        return next_index;
    }
	
	void PopLast()
	{
		if(count < 1) return;
		auto ind = --count;
		memset((void*)&commands[ind], 0, sizeof(CScriptDrawingCommandVars));
	}
	
	bool is_dirty(int lyr, int sprite_id = 0);
	void mark_dirty(int lyr, int sprite_id = 0);
    
    reference operator [](const int32_t i)
    {
        return commands[i];
    }
    const_reference operator [](const int32_t i) const
    {
        return commands[i];
    }
    
    
    inline BITMAP* AquireSubBitmap(int32_t w, int32_t h)
    {
        return bitmap_pool.AquireSubBitmap(w,h);
    }
    
    inline void ReleaseSubBitmap(BITMAP* b)
    {
        bitmap_pool.ReleaseSubBitmap(b);
    }
    
    inline BITMAP* GetSmallTextureBitmap(int32_t bw, int32_t bh)
    {
        return small_tex_cache.GetTexture(bw, bh);
    }
	
	CScriptDrawingCommands* pop_commands();
	void push_commands(CScriptDrawingCommands* other, bool del = true);
public: 
	int32_t count;
protected:
    vec_type commands;
	std::set<std::pair<int, int>> dirty_layers;
    
    DrawingContainer draw_container;
    ScriptDrawingBitmapPool bitmap_pool;
    SmallBitmapTextureCache small_tex_cache;
    
private:
    CScriptDrawingCommands(const CScriptDrawingCommands&) : count(0) {}
    // Warning: no return statement in function returning non-void
    //CScriptDrawingCommands& operator =(const CScriptDrawingCommands&) {}
};


extern CScriptDrawingCommands script_drawing_commands;




#endif

