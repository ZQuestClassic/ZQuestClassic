
#ifndef _zc_script_drawing_h
#define _zc_script_drawing_h

#include <vector>
#include <string>

#define MAX_SCRIPT_DRAWING_COMMANDS 10000
#define SCRIPT_DRAWING_COMMAND_VARIABLES 20

// For Quad and Triangle. *allegro Bug-Fix* -Gleeok
class SmallBitmapTextureCache
{
public:
    SmallBitmapTextureCache()
    {
        is_init = false;
        
        for(int i(0); i < 4; ++i)
            for(int j(0); j < 4; ++j)
                _bmp[i][j] = 0;
    }
    
    void Init()
    {
        if(is_init)
            return;
            
        const int size[4] = { 16, 32, 64, 128 };
        
        for(int i(0); i < 4; ++i)
        {
            for(int j(0); j < 4; ++j)
                _bmp[i][j] = create_bitmap_ex(8, size[i], size[j]);
        }
        
        is_init = true;
    }
    
    ~SmallBitmapTextureCache()
    {
        Dispose();
    }
    
    void Dispose()
    {
        for(int i(0); i < 4; ++i)
            for(int j(0); j < 4; ++j)
            {
                if(_bmp[i][j])
                    destroy_bitmap(_bmp[i][j]);
                    
                _bmp[i][j] = 0;
            }
            
        is_init = false;
    }
    
    inline BITMAP* GetTexture(int bw, int bh)
    {
        BITMAP* ret = 0;
        
        if(!is_init)
            Init();
            
        if(bw == 1 || bw == 2 || bw == 4 || bw == 8)
        {
            int x = bw >> 1;
            
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
    
    void Dispose()
    {
        if(_parent_bmp)
            destroy_bitmap(_parent_bmp), _parent_bmp = 0;
    }
    
    inline BITMAP* AquireSubBitmap(int w, int h)
    {
        return AquireSubBitmap(0, 0, w, h);
    }
    inline BITMAP* AquireSubBitmap(int x, int y, int w, int h)
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
    //static int _rc = 0;
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
    
    std::vector<long>* GetVector()
    {
        std::vector<long>* v;
        
        if(drawdata.size() > current_drawdata_count)
        {
            v = drawdata[current_drawdata_count];
            v->clear();
        }
        else
        {
            v = new std::vector<long>();
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
    std::vector<std::vector<long>*> drawdata;
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
    
    //void SetArray(std::array* aptr)
    //{
    //    ptr = (void*)aptr;
    //}
    
    void SetVector(std::vector<long>* v)
    {
        ptr = (void*)v;
    }
    
    void* GetPtr()
    {
        return ptr;
    }
    
    int &operator [](const int i)
    {
        return data[i];
    }
    const int &operator [](const int i) const
    {
        return data[i];
    }
    
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
    const static int DefaultCapacity = 256; //176 + some extra
    
    CScriptDrawingCommands() : commands(), count(0) {}
    ~CScriptDrawingCommands() {}
    int GetCount();
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
    
    int Count() const
    {
        return count;
    }
    std::string* GetString()
    {
        return draw_container.GetString();
    }
    std::vector<long>* GetVector()
    {
        return draw_container.GetVector();
    }
    
    int GetNext()
    {
        if(count>=MAX_SCRIPT_DRAWING_COMMANDS)
            return -1;
        
        const int next_index = count;
        const int capacity = commands.capacity();
        
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
    
    reference operator [](const int i)
    {
        return commands[i];
    }
    const_reference operator [](const int i) const
    {
        return commands[i];
    }
    
    
    inline BITMAP* AquireSubBitmap(int w, int h)
    {
        return bitmap_pool.AquireSubBitmap(w,h);
    }
    
    inline void ReleaseSubBitmap(BITMAP* b)
    {
        bitmap_pool.ReleaseSubBitmap(b);
    }
    
    inline BITMAP* GetSmallTextureBitmap(int bw, int bh)
    {
        return small_tex_cache.GetTexture(bw, bh);
    }
    
public: 
	int count;
protected:
    vec_type commands;
    
    
    DrawingContainer draw_container;
    ScriptDrawingBitmapPool bitmap_pool;
    SmallBitmapTextureCache small_tex_cache;
    
private:
    CScriptDrawingCommands(const CScriptDrawingCommands&) {}
    // Warning: no return statement in function returning non-void
    //CScriptDrawingCommands& operator =(const CScriptDrawingCommands&) {}
};


extern CScriptDrawingCommands script_drawing_commands;




#endif

