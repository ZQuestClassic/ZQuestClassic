#ifndef ZSPARSER_TYPES_H
#define ZSPARSER_TYPES_H

#include <string>

using std::string;

typedef int ZVarTypeId;

namespace ZScript
{
	class Scope;
}

////////////////////////////////////////////////////////////////
// Variable Types

// I can't figure out a better way to do this in C++98.
enum ZVarTypeClassId
{
	ZVARTYPE_CLASSID_BASE,
	ZVARTYPE_CLASSID_SIMPLE,
	ZVARTYPE_CLASSID_UNRESOLVED,
	ZVARTYPE_CLASSID_CONST_FLOAT,
	ZVARTYPE_CLASSID_CLASS,
	ZVARTYPE_CLASSID_ARRAY
};

enum ZVarTypeIdBuiltin
{
	ZVARTYPEID_START = 0,

	ZVARTYPEID_PRIMITIVE_START = 0,
    ZVARTYPEID_UNTYPED = 0,
    ZVARTYPEID_VOID,
    ZVARTYPEID_FLOAT,
    ZVARTYPEID_BOOL,
	ZVARTYPEID_PRIMITIVE_END,

	ZVARTYPEID_CONST_FLOAT = ZVARTYPEID_PRIMITIVE_END,

	ZVARTYPEID_CLASS_START,
    ZVARTYPEID_GAME = ZVARTYPEID_CLASS_START,
    ZVARTYPEID_LINK,
    ZVARTYPEID_SCREEN,
    ZVARTYPEID_FFC,
    ZVARTYPEID_ITEM,
    ZVARTYPEID_ITEMCLASS,
    ZVARTYPEID_NPC,
    ZVARTYPEID_LWPN,
    ZVARTYPEID_EWPN,
    ZVARTYPEID_NPCDATA,
    ZVARTYPEID_DEBUG,
    ZVARTYPEID_AUDIO,
    ZVARTYPEID_COMBOS,
    ZVARTYPEID_SPRITEDATA,
    ZVARTYPEID_GRAPHICS,
    ZVARTYPEID_TEXT,
    ZVARTYPEID_INPUT,
    ZVARTYPEID_MAPDATA,
    ZVARTYPEID_DMAPDATA,
    ZVARTYPEID_ZMESSAGE,
    ZVARTYPEID_SHOPDATA,
    ZVARTYPEID_DROPSET,
    ZVARTYPEID_PONDS,
    ZVARTYPEID_WARPRING,
    ZVARTYPEID_DOORSET,
    ZVARTYPEID_ZUICOLOURS,
    ZVARTYPEID_RGBDATA,
    ZVARTYPEID_PALETTE,
    ZVARTYPEID_TUNES,
    ZVARTYPEID_PALCYCLE,
    ZVARTYPEID_GAMEDATA,
	ZVARTYPEID_CHEATS,
	ZVARTYPEID_CLASS_END,

	ZVARTYPEID_END = ZVARTYPEID_CLASS_END
};

class ZVarTypeSimple;
class ZVarTypeConstFloat;
class ZVarTypeClass;
class ZVarTypeArray;

class ZVarType
{
public:
	virtual ZVarType* clone() const = 0;
	virtual string getName() const = 0;
	virtual ZVarType* resolve(ZScript::Scope& scope) {return this;}
	virtual bool isResolved() const {return true;}
	virtual bool canBeGlobal() const {return false;}
	virtual bool canCastTo(ZVarType const& target) const = 0;
	virtual int typeClassId() const {return ZVARTYPE_CLASSID_BASE;};

	int compare(ZVarType const& other) const;
	bool operator==(ZVarType const& other) const {return compare(other) == 0;}
	bool operator!=(ZVarType const& other) const {return compare(other) != 0;}
	bool operator<(ZVarType const& other) const {return compare(other) < 0;}

	// Comparator for pointers to types.
	struct PointerLess : public std::less<ZVarType*> {
		bool operator() (ZVarType* const& a, ZVarType* const& b) const
		{
			if (b == NULL) return false;
			if (a == NULL) return true;
			return *a < *b;
		}
	};

	// This comes up so often I'm adding in this shortcut.
	bool isArray() const {return typeClassId() == ZVARTYPE_CLASSID_ARRAY;}

	// Get the number of nested arrays at top level.
	int getArrayDepth() const;
	
protected:
	virtual int selfCompare(ZVarType const& other) const = 0;

// Standard Types.
public:
	static ZVarTypeSimple const UNTYPED;
	static ZVarTypeSimple const ZVOID;
	static ZVarTypeSimple const FLOAT;
	static ZVarTypeSimple const BOOL;
	static ZVarTypeConstFloat const CONST_FLOAT;
	static ZVarTypeClass const FFC;
	static ZVarTypeClass const ITEM;
	static ZVarTypeClass const ITEMCLASS;
	static ZVarTypeClass const NPC;
	static ZVarTypeClass const LWPN;
	static ZVarTypeClass const EWPN;
	static ZVarTypeClass const GAME;
	static ZVarTypeClass const LINK;
	static ZVarTypeClass const SCREEN;
	static ZVarTypeClass const NPCDATA;
	static ZVarTypeClass const DEBUG;
	static ZVarTypeClass const AUDIO;
	static ZVarTypeClass const COMBOS;
	static ZVarTypeClass const SPRITEDATA;
	static ZVarTypeClass const GRAPHICS;
	static ZVarTypeClass const TEXT;
	static ZVarTypeClass const INPUT;
	static ZVarTypeClass const MAPDATA;
	static ZVarTypeClass const DMAPDATA;
	static ZVarTypeClass const ZMESSAGE;
	static ZVarTypeClass const SHOPDATA;
	static ZVarTypeClass const DROPSET;
	static ZVarTypeClass const PONDS;
	static ZVarTypeClass const WARPRING;
	static ZVarTypeClass const DOORSET;
	static ZVarTypeClass const ZUICOLOURS;
	static ZVarTypeClass const RGBDATA;
	static ZVarTypeClass const PALETTE;
	static ZVarTypeClass const TUNES;
	static ZVarTypeClass const PALCYCLE;
	static ZVarTypeClass const GAMEDATA;
	static ZVarTypeClass const CHEATS;
	static ZVarType const* get(ZVarTypeId id);
};

class ZVarTypeSimple : public ZVarType
{
public:
	ZVarTypeSimple(int simpleId, string const& name, string const& upName)
			: simpleId(simpleId), name(name), upName(upName) {}
	ZVarTypeSimple* clone() const {return new ZVarTypeSimple(*this);}
	string getName() const {return name;}
	string getUpName() const {return upName;}
	bool canBeGlobal() const;
	bool canCastTo(ZVarType const& target) const;
	int getId() const {return simpleId;}
	int typeClassId() const {return ZVARTYPE_CLASSID_SIMPLE;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	int simpleId;
	string name;
	string upName;
};

class ZVarTypeUnresolved : public ZVarType
{
public:
	ZVarTypeUnresolved(string const& name) : name(name) {}
	ZVarTypeUnresolved* clone() const {return new ZVarTypeUnresolved(*this);}
	string getName() const {return name;}
	ZVarType* resolve(ZScript::Scope& scope);
	bool isResolved() const {return false;}
	bool canCastTo(ZVarType const& target) const {return false;}
	int typeClassId() const {return ZVARTYPE_CLASSID_UNRESOLVED;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	string name;
};

// Temporary while only floats can be constant.
class ZVarTypeConstFloat : public ZVarType
{
public:
	ZVarTypeConstFloat() {}
	ZVarType* clone() const {return new ZVarTypeConstFloat(*this);}
	string getName() const {return "const float";}
	ZVarType* resolve(ZScript::Scope& scope) {return this;}
	bool canBeGlobal() const {return true;}
	bool canCastTo(ZVarType const& target) const;
	int typeClassId() const {return ZVARTYPE_CLASSID_CONST_FLOAT;};
protected:
	int selfCompare(ZVarType const& other) const {return 0;};
};

class ZVarTypeClass : public ZVarType
{
public:
	ZVarTypeClass(int classId) : classId(classId), className("") {}
	ZVarTypeClass(int classId, string const& className) : classId(classId), className(className) {}
	ZVarTypeClass* clone() const {return new ZVarTypeClass(*this);}
	string getName() const;
	string getClassName() const {return className;}
	int getClassId() const {return classId;}
	ZVarType* resolve(ZScript::Scope& scope);
	bool canBeGlobal() const {return true;}
	bool canCastTo(ZVarType const& target) const;
	int typeClassId() const {return ZVARTYPE_CLASSID_CLASS;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	int classId;
	string className;
};

class ZVarTypeArray : public ZVarType
{
public:
	ZVarTypeArray(ZVarType const& elementType) : elementType(elementType) {}
	ZVarTypeArray* clone() const {return new ZVarTypeArray(*this);}

	int typeClassId() const {return ZVARTYPE_CLASSID_ARRAY; }

	string getName() const {return elementType.getName() + "[]";}
	ZVarType* resolve(ZScript::Scope& scope) {return this;}

	bool canBeGlobal() const {return true;}
	bool canCastTo(ZVarType const& target) const;
	ZVarType const& getElementType() const {return elementType;}
	ZVarType const& getBaseType() const;
protected:
	int selfCompare(ZVarType const& other) const;
private:
	ZVarType const& elementType;
};

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Script Types

	// Basically an enum.
	class ScriptType
	{
	public:
		ScriptType()
			: id(ID_NULL), name("null"), thisTypeId(ZVARTYPEID_VOID)
		{}
		
		bool operator==(ScriptType const& other) const {
			return id == other.id;}
		string const& getName() const {return name;}
		ZVarTypeId getThisTypeId() const {return thisTypeId;}
		bool isNull() const {return id == ID_NULL;}

		static ScriptType const GLOBAL;
		static ScriptType const FFC;
		static ScriptType const ITEM;
		
	private:
		enum Id {ID_NULL, ID_GLOBAL, ID_FFC, ID_ITEM};
		
		ScriptType(Id id, string const& name, ZVarTypeId thisTypeId)
			: id(id), name(name), thisTypeId(thisTypeId)
		{}

		int id;
		string name;
		ZVarTypeId thisTypeId;
	};
}

#endif
