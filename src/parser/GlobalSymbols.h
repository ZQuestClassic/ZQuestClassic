#ifndef GLOBALSYMBOLS_H //2.53 Updated to 16th Jan, 2017
#define GLOBALSYMBOLS_H

#include "DataStructs.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class Scope;
class SymbolTable;

static const int SETTER = 0;
static const int GETTER = 1;
static const int FUNCTION = 2;

struct AccessorTable
{
    string name;
    int rettype;
    int setorget;
    int var;
    int numindex;
    int params[20];
};

class LibrarySymbols
{
public:
    virtual void addSymbolsToScope(Scope *scope, SymbolTable *t);
    virtual map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
    virtual int matchFunction(string name);
    virtual ~LibrarySymbols();
protected:
    AccessorTable *table;
    LibrarySymbols() {}
    int firstid;
    int refVar;
    map<string, int> memberids;
    virtual vector<Opcode *> getVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setBoolVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> getIndexedVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setIndexedVariable(LinkTable &lt, int id, int var);
};

class GlobalSymbols : public LibrarySymbols
{
public:
    static GlobalSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static GlobalSymbols singleton;
    GlobalSymbols();
};

class FFCSymbols : public LibrarySymbols
{
public:
    static FFCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static FFCSymbols singleton;
    FFCSymbols();
};

class LinkSymbols : public LibrarySymbols
{
public:
    static LinkSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static LinkSymbols singleton;
    LinkSymbols();
};

class ScreenSymbols : public LibrarySymbols
{
public:
    static ScreenSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static ScreenSymbols singleton;
    ScreenSymbols();
};

class ItemSymbols : public LibrarySymbols
{
public:
    static ItemSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static ItemSymbols singleton;
    ItemSymbols();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
    static ItemclassSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static ItemclassSymbols singleton;
    ItemclassSymbols();
};

class GameSymbols : public LibrarySymbols
{
public:
    static GameSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static GameSymbols singleton;
    GameSymbols();
};

class NPCSymbols : public LibrarySymbols
{
public:
    static NPCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static NPCSymbols singleton;
    NPCSymbols();
};

class LinkWeaponSymbols : public LibrarySymbols
{
public:
    static LinkWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static LinkWeaponSymbols singleton;
    LinkWeaponSymbols();
};

class EnemyWeaponSymbols : public LibrarySymbols
{
public:
    static EnemyWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static EnemyWeaponSymbols singleton;
    EnemyWeaponSymbols();
};

//New Types

//Graphics->

class GfxPtrSymbols : public LibrarySymbols
{
public:
    static GfxPtrSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static GfxPtrSymbols singleton;
    GfxPtrSymbols();
};

//SpriteData
class SpriteDataSymbols : public LibrarySymbols
{
public:
    static SpriteDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static SpriteDataSymbols singleton;
    SpriteDataSymbols();
};

//DMapData
class DMapDataSymbols : public LibrarySymbols
{
public:
    static DMapDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static DMapDataSymbols singleton;
    DMapDataSymbols();
};


//MessageData
class MessageDataSymbols : public LibrarySymbols
{
public:
    static MessageDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static MessageDataSymbols singleton;
    MessageDataSymbols();
};

//ShopData
class ShopDataSymbols : public LibrarySymbols
{
public:
    static ShopDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static ShopDataSymbols singleton;
    ShopDataSymbols();
};

//ComboData->

class CombosPtrSymbols : public LibrarySymbols
{
public:
    static CombosPtrSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static CombosPtrSymbols singleton;
    CombosPtrSymbols();
};

//Audio->

class AudioSymbols : public LibrarySymbols
{
public:
    static AudioSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static AudioSymbols singleton;
    AudioSymbols();
};

//Debug->

class DebugSymbols : public LibrarySymbols
{
public:
    static DebugSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static DebugSymbols singleton;
    DebugSymbols();
};

//NPCData->

class NPCDataSymbols : public LibrarySymbols
{
public:
    static NPCDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static NPCDataSymbols singleton;
    NPCDataSymbols();
};

//Text->
class TextPtrSymbols : public LibrarySymbols
{
public:
    static TextPtrSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static TextPtrSymbols singleton;
    TextPtrSymbols();
};

//Input->
class InputSymbols : public LibrarySymbols
{
public:
    static InputSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static InputSymbols singleton;
    InputSymbols();
};

//MapData->
class MapDataSymbols : public LibrarySymbols
{
public:
    static MapDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static MapDataSymbols singleton;
    MapDataSymbols();
};



//nil
class UntypedSymbols : public LibrarySymbols
{
public:
    static UntypedSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static UntypedSymbols singleton;
    UntypedSymbols();
};

//dropdata
class DropsetSymbols : public LibrarySymbols
{
public:
    static DropsetSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static DropsetSymbols singleton;
    DropsetSymbols();
};

//ponddata
class PondSymbols : public LibrarySymbols
{
public:
    static PondSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static PondSymbols singleton;
    PondSymbols();
};

//warpringdata
class WarpringSymbols : public LibrarySymbols
{
public:
    static WarpringSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static WarpringSymbols singleton;
    WarpringSymbols();
};

//doorsetdata
class DoorsetSymbols : public LibrarySymbols
{
public:
    static DoorsetSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static DoorsetSymbols singleton;
    DoorsetSymbols();
};

//misccolordata
class MiscColourSymbols : public LibrarySymbols
{
public:
    static MiscColourSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static MiscColourSymbols singleton;
    MiscColourSymbols();
};

//rgbdata
class RGBSymbols : public LibrarySymbols
{
public:
    static RGBSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static RGBSymbols singleton;
    RGBSymbols();
};

//palettedata
class PaletteSymbols : public LibrarySymbols
{
public:
    static PaletteSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static PaletteSymbols singleton;
    PaletteSymbols();
};

//tunesdata
class TunesSymbols : public LibrarySymbols
{
public:
    static TunesSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static TunesSymbols singleton;
    TunesSymbols();
};

//palcycledata
class PalCycleSymbols : public LibrarySymbols
{
public:
    static PalCycleSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static PalCycleSymbols singleton;
    PalCycleSymbols();
};

//gamedata
class GamedataSymbols : public LibrarySymbols
{
public:
    static GamedataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static GamedataSymbols singleton;
    GamedataSymbols();
};

//cheatdata
class CheatsSymbols : public LibrarySymbols
{
public:
    static CheatsSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static CheatsSymbols singleton;
    CheatsSymbols();
};


#endif


