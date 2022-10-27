#ifndef GLOBALSYMBOLS_H //2.53 Updated to 16th Jan, 2017
#define GLOBALSYMBOLS_H

#include "DataStructs.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;
using namespace ZScript;

class SymbolTable;
namespace ZScript
{
	class Function;
	class Scope;
}

static const int32_t SETTER = 0;
static const int32_t GETTER = 1;
static const int32_t FUNCTION = 2;

struct AccessorTable
{
    string name;
    int32_t rettype;
    int32_t setorget;
    int32_t var;
    int32_t numindex;
	int32_t funcFlags;
	int32_t numParams;
    int32_t params[20];
};

class LibrarySymbols
{
public:
	static LibrarySymbols* getTypeInstance(DataTypeId typeId);

	virtual void addSymbolsToScope(ZScript::Scope& scope);
    virtual ~LibrarySymbols();

protected:
    AccessorTable *table;
	LibrarySymbols() : refVar(0), hasPrefixType(true) {}
    int32_t refVar;
	bool hasPrefixType;

	ZScript::Function* getFunction(string const& name, int32_t numParams) const;

private:
	map<std::pair<string, int32_t>, ZScript::Function*> functions;
	
	// Generates the code for functions which can't be auto generated.
	virtual void generateCode() = 0;
};

class GlobalSymbols : public LibrarySymbols
{
public:
    static GlobalSymbols &getInst()
    {
        return singleton;
    }
private:
    static GlobalSymbols singleton;
    GlobalSymbols();
	void generateCode();
};

class FFCSymbols : public LibrarySymbols
{
public:
    static FFCSymbols &getInst()
    {
        return singleton;
    }
private:
    static FFCSymbols singleton;
    FFCSymbols();
	void generateCode();
};

class HeroSymbols : public LibrarySymbols
{
public:
    static HeroSymbols &getInst()
    {
        return singleton;
    }
private:
    static HeroSymbols singleton;
    HeroSymbols();
	void generateCode();
};

class ScreenSymbols : public LibrarySymbols
{
public:
    static ScreenSymbols &getInst()
    {
        return singleton;
    }
private:
    static ScreenSymbols singleton;
    ScreenSymbols();
	void generateCode();
};

class ItemSymbols : public LibrarySymbols
{
public:
    static ItemSymbols &getInst()
    {
        return singleton;
    }
private:
    static ItemSymbols singleton;
    ItemSymbols();
	void generateCode();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
    static ItemclassSymbols &getInst()
    {
        return singleton;
    }
private:
    static ItemclassSymbols singleton;
    ItemclassSymbols();
	void generateCode();
};

class GameSymbols : public LibrarySymbols
{
public:
    static GameSymbols &getInst()
    {
        return singleton;
    }
private:
    static GameSymbols singleton;
    GameSymbols();
	void generateCode();
};

class NPCSymbols : public LibrarySymbols
{
public:
    static NPCSymbols &getInst()
    {
        return singleton;
    }
private:
    static NPCSymbols singleton;
    NPCSymbols();
	void generateCode();
};

class HeroWeaponSymbols : public LibrarySymbols
{
public:
    static HeroWeaponSymbols &getInst()
    {
        return singleton;
    }
private:
    static HeroWeaponSymbols singleton;
    HeroWeaponSymbols();
	void generateCode();
};

class EnemyWeaponSymbols : public LibrarySymbols
{
public:
    static EnemyWeaponSymbols &getInst()
    {
        return singleton;
    }
private:
    static EnemyWeaponSymbols singleton;
    EnemyWeaponSymbols();
	void generateCode();
};

//New Types

//Graphics
class GraphicsSymbols : public LibrarySymbols
{
public:
    static GraphicsSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static GraphicsSymbols singleton;
    GraphicsSymbols();
    void generateCode();
};

// Bitmap
class BitmapSymbols : public LibrarySymbols
{
public:
    static BitmapSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static BitmapSymbols singleton;
    BitmapSymbols();
    void generateCode();
};

//SpriteData
class SpriteDataSymbols : public LibrarySymbols
{
public:
    static SpriteDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static SpriteDataSymbols singleton;
    SpriteDataSymbols();
    void generateCode();
};

//DMapData
class DMapDataSymbols : public LibrarySymbols
{
public:
    static DMapDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static DMapDataSymbols singleton;
    DMapDataSymbols();
    void generateCode();
};


//MessageData
class MessageDataSymbols : public LibrarySymbols
{
public:
    static MessageDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static MessageDataSymbols singleton;
    MessageDataSymbols();
    void generateCode();
};

//ShopData
class ShopDataSymbols : public LibrarySymbols
{
public:
    static ShopDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static ShopDataSymbols singleton;
    ShopDataSymbols();
    void generateCode();
};

//ComboData->

class CombosPtrSymbols : public LibrarySymbols
{
public:
    static CombosPtrSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static CombosPtrSymbols singleton;
    CombosPtrSymbols();
    void generateCode();
};

//Audio->

class AudioSymbols : public LibrarySymbols
{
public:
    static AudioSymbols &getInst()
    {
        return singleton;
    }
private:
    static AudioSymbols singleton;
    AudioSymbols();
	void generateCode();
};

//Debug->

class DebugSymbols : public LibrarySymbols
{
public:
    static DebugSymbols &getInst()
    {
        return singleton;
    }
private:
    static DebugSymbols singleton;
    DebugSymbols();
	void generateCode();
};

//NPCData->

class NPCDataSymbols : public LibrarySymbols
{
public:
    static NPCDataSymbols &getInst()
    {
        return singleton;
    }
private:
    static NPCDataSymbols singleton;
    NPCDataSymbols();
	void generateCode();
};

//Text->
class TextPtrSymbols : public LibrarySymbols
{
public:
    static TextPtrSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static TextPtrSymbols singleton;
    TextPtrSymbols();
    void generateCode();
};

//Filesystem->
class FileSystemSymbols : public LibrarySymbols
{
public:
    static FileSystemSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static FileSystemSymbols singleton;
    FileSystemSymbols();
    void generateCode();
};

//Input->
class InputSymbols : public LibrarySymbols
{
public:
    static InputSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static InputSymbols singleton;
    InputSymbols();
    void generateCode();
};

//MapData->
class MapDataSymbols : public LibrarySymbols
{
public:
    static MapDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static MapDataSymbols singleton;
    MapDataSymbols();
    void generateCode();
};

//nil
class UntypedSymbols : public LibrarySymbols
{
public:
    static UntypedSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static UntypedSymbols singleton;
    UntypedSymbols();
    void generateCode();
};

//dropdata
class DropsetSymbols : public LibrarySymbols
{
public:
    static DropsetSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static DropsetSymbols singleton;
    DropsetSymbols();
    void generateCode();
};

//ponddata
class PondSymbols : public LibrarySymbols
{
public:
    static PondSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static PondSymbols singleton;
    PondSymbols();
    void generateCode();
};

//warpringdata
class WarpringSymbols : public LibrarySymbols
{
public:
    static WarpringSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static WarpringSymbols singleton;
    WarpringSymbols();
    void generateCode();
};

//doorsetdata
class DoorsetSymbols : public LibrarySymbols
{
public:
    static DoorsetSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static DoorsetSymbols singleton;
    DoorsetSymbols();
    void generateCode();
};

//misccolordata
class MiscColourSymbols : public LibrarySymbols
{
public:
    static MiscColourSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static MiscColourSymbols singleton;
    MiscColourSymbols();
    void generateCode();
};

//rgbdata
class RGBSymbols : public LibrarySymbols
{
public:
    static RGBSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static RGBSymbols singleton;
    RGBSymbols();
    void generateCode();
};

//palettedata
class PaletteSymbols : public LibrarySymbols
{
public:
    static PaletteSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static PaletteSymbols singleton;
    PaletteSymbols();
    void generateCode();
};

//tunesdata
class TunesSymbols : public LibrarySymbols
{
public:
    static TunesSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static TunesSymbols singleton;
    TunesSymbols();
    void generateCode();
};

//palcycledata
class PalCycleSymbols : public LibrarySymbols
{
public:
    static PalCycleSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static PalCycleSymbols singleton;
    PalCycleSymbols();
    void generateCode();
};

//gamedata
class GamedataSymbols : public LibrarySymbols
{
public:
    static GamedataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static GamedataSymbols singleton;
    GamedataSymbols();
    void generateCode();
};

//cheatdata
class CheatsSymbols : public LibrarySymbols
{
public:
    static CheatsSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static CheatsSymbols singleton;
    CheatsSymbols();
    void generateCode();
};

class FileSymbols : public LibrarySymbols
{
public:
    static FileSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static FileSymbols singleton;
    FileSymbols();
    void generateCode();
};

class DirectorySymbols : public LibrarySymbols
{
public:
    static DirectorySymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static DirectorySymbols singleton;
    DirectorySymbols();
    void generateCode();
};

class StackSymbols : public LibrarySymbols
{
public:
    static StackSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static StackSymbols singleton;
    StackSymbols();
    void generateCode();
};

class SubscreenDataSymbols : public LibrarySymbols
{
public:
    static SubscreenDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static SubscreenDataSymbols singleton;
    SubscreenDataSymbols();
    void generateCode();
};

class RNGSymbols : public LibrarySymbols
{
public:
    static RNGSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static RNGSymbols singleton;
    RNGSymbols();
    void generateCode();
};

class PalDataSymbols : public LibrarySymbols
{
public:
    static PalDataSymbols& getInst()
    {
        return singleton;
    }
protected:
private:
    static PalDataSymbols singleton;
    PalDataSymbols();
    void generateCode();
};

class ModuleSymbols : public LibrarySymbols
{
public:
    static ModuleSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static ModuleSymbols singleton;
    ModuleSymbols();
    void generateCode();
};

class BottleTypeSymbols : public LibrarySymbols
{
public:
    static BottleTypeSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static BottleTypeSymbols singleton;
    BottleTypeSymbols();
    void generateCode();
};

class BottleShopSymbols : public LibrarySymbols
{
public:
    static BottleShopSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static BottleShopSymbols singleton;
    BottleShopSymbols();
    void generateCode();
};

class GenericDataSymbols : public LibrarySymbols
{
public:
    static GenericDataSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static GenericDataSymbols singleton;
    GenericDataSymbols();
    void generateCode();
};

#endif


