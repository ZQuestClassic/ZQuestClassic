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
static const int32_t CONSTANT = 3;

struct AccessorTable
{
    string name;
	byte tag;
    int32_t rettype;
    int32_t var;
	int32_t funcFlags;
    vector<int32_t> params;
    vector<int32_t> optparams;
	byte extra_vargs; //how many listd params should be treatd as vargs
	string info;
	
	//
	
	optional<string> alias_name;
	byte alias_tag;
	
	AccessorTable(string const& name, byte tag, int32_t rettype, int32_t var, int32_t flags,
		vector<int32_t>const& params, vector<int32_t> const& opts, byte exvargs = 0,
		string const& info = "");
	AccessorTable(string const& name, byte tag,
		string const& alias, byte alias_tag,
		int32_t flags = 0, string const& info = "");
};

class LibrarySymbols
{
public:
	static LibrarySymbols* getTypeInstance(DataTypeId typeId);

	virtual void addSymbolsToScope(ZScript::Scope& scope);
    virtual ~LibrarySymbols();

protected:
	LibrarySymbols() : table(nullptr), refVar(0),
		hasPrefixType(true)
	{}
    AccessorTable *table;
    int32_t refVar;
	bool hasPrefixType;

	ZScript::Function* getFunction(string const& name, byte tag = 0) const;
	ZScript::Function* getAlias(string const& name, byte tag = 0) const;

private:
	static LibrarySymbols nilsymbols;
	map<std::pair<string, byte>, ZScript::Function*> functions;
	map<std::pair<string, byte>, ZScript::Function*> alias_functions;
	
	// Generates the code for functions which can't be auto generated.
	virtual void generateCode(){};
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

class RegionSymbols : public LibrarySymbols
{
public:
    static RegionSymbols &getInst()
    {
        return singleton;
    }
private:
    static RegionSymbols singleton;
    RegionSymbols();
	void generateCode();
};

class ViewportSymbols : public LibrarySymbols
{
public:
    static ViewportSymbols &getInst()
    {
        return singleton;
    }
private:
    static ViewportSymbols singleton;
    ViewportSymbols();
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

class ZInfoSymbols : public LibrarySymbols
{
public:
    static ZInfoSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static ZInfoSymbols singleton;
    ZInfoSymbols();
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

class PortalSymbols : public LibrarySymbols
{
public:
    static PortalSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static PortalSymbols singleton;
    PortalSymbols();
    void generateCode();
};

class SavedPortalSymbols : public LibrarySymbols
{
public:
    static SavedPortalSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static SavedPortalSymbols singleton;
    SavedPortalSymbols();
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
class SubscreenPageSymbols : public LibrarySymbols
{
public:
    static SubscreenPageSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static SubscreenPageSymbols singleton;
    SubscreenPageSymbols();
    void generateCode();
};
class SubscreenWidgetSymbols : public LibrarySymbols
{
public:
    static SubscreenWidgetSymbols &getInst()
    {
        return singleton;
    }
protected:
private:
    static SubscreenWidgetSymbols singleton;
    SubscreenWidgetSymbols();
    void generateCode();
};

#endif


