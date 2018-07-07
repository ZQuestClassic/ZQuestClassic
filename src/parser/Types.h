#ifndef ZSPARSER_TYPES_H
#define ZSPARSER_TYPES_H

#include <string>

class Scope;

using std::string;

////////////////////////////////////////////////////////////////
// Script Types

enum ScriptType {SCRIPTTYPE_VOID, SCRIPTTYPE_GLOBAL, SCRIPTTYPE_FFC, SCRIPTTYPE_ITEM};

////////////////////////////////////////////////////////////////
// Variable Types

typedef int ZVarTypeId;

// I can't figure out a better way to do this in C++98.
enum ZVarTypeClassId
{
	ZVARTYPE_CLASSID_BASE,
	ZVARTYPE_CLASSID_SIMPLE,
	ZVARTYPE_CLASSID_UNRESOLVED
};

enum ZVarTypeIdSimple
{
    ZVARTYPEID_VOID,
    ZVARTYPEID_FLOAT,
    ZVARTYPEID_BOOL,
    ZVARTYPEID_FFC,
    ZVARTYPEID_ITEM,
    ZVARTYPEID_ITEMCLASS,
    ZVARTYPEID_NPC,
    ZVARTYPEID_LWPN,
    ZVARTYPEID_EWPN,
    ZVARTYPEID_GAME,
    ZVARTYPEID_LINK,
    ZVARTYPEID_SCREEN,
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
    ZVARTYPEID_UNTYPED,
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
	ZVARTYPEID_END
};

class ZVarTypeSimple;

class ZVarType
{
public:
	virtual ZVarType* clone() const = 0;
	virtual string getName() const = 0;
	virtual ZVarType* resolve(Scope& scope) {return this;}
	virtual bool isResolved() const {return true;}
	virtual bool canBeGlobal() const {return false;}
	virtual bool canCastTo(ZVarType const& target) const = 0;
	virtual int classId() const {return ZVARTYPE_CLASSID_BASE;};

	int compare(ZVarType const& other) const;
	bool operator==(ZVarType const& other) const {return compare(other) == 0;}
	bool operator!=(ZVarType const& other) const {return compare(other) != 0;}
	bool operator<(ZVarType const& other) const {return compare(other) < 0;}

	struct PointerLess : public std::less<ZVarType*> {
		bool operator() (ZVarType* const& a, ZVarType* const& b) const
		{
			if (b == NULL) return false;
			if (a == NULL) return true;
			return *a < *b;
		}
	};
protected:
	virtual int selfCompare(ZVarType const& other) const = 0;

// Standard Types.
public:
	static ZVarTypeSimple const VOID;
	static ZVarTypeSimple const FLOAT;
	static ZVarTypeSimple const BOOL;
	static ZVarTypeSimple const FFC;
	static ZVarTypeSimple const ITEM;
	static ZVarTypeSimple const ITEMCLASS;
	static ZVarTypeSimple const NPC;
	static ZVarTypeSimple const LWPN;
	static ZVarTypeSimple const EWPN;
	static ZVarTypeSimple const GAME;
	static ZVarTypeSimple const LINK;
	static ZVarTypeSimple const SCREEN;
	static ZVarTypeSimple const NPCDATA;
	static ZVarTypeSimple const DEBUG;
	static ZVarTypeSimple const AUDIO;
	static ZVarTypeSimple const COMBOS;
	static ZVarTypeSimple const SPRITEDATA;
	static ZVarTypeSimple const GRAPHICS;
	static ZVarTypeSimple const TEXT;
	static ZVarTypeSimple const INPUT;
	static ZVarTypeSimple const MAPDATA;
	static ZVarTypeSimple const DMAPDATA;
	static ZVarTypeSimple const ZMESSAGE;
	static ZVarTypeSimple const SHOPDATA;
	static ZVarTypeSimple const UNTYPED;
	static ZVarTypeSimple const DROPSET;
	static ZVarTypeSimple const PONDS;
	static ZVarTypeSimple const WARPRING;
	static ZVarTypeSimple const DOORSET;
	static ZVarTypeSimple const ZUICOLOURS;
	static ZVarTypeSimple const RGBDATA;
	static ZVarTypeSimple const PALETTE;
	static ZVarTypeSimple const TUNES;
	static ZVarTypeSimple const PALCYCLE;
	static ZVarTypeSimple const GAMEDATA;
	static ZVarTypeSimple const CHEATS;
	static ZVarType const* get(ZVarTypeId id);
};

class ZVarTypeSimple : public ZVarType
{
public:
	ZVarTypeSimple(ZVarTypeIdSimple simpleId, string const& name) : simpleId(simpleId), name(name) {}
	ZVarTypeSimple* clone() const {return new ZVarTypeSimple(*this);}
	string getName() const {return name;}
	bool canBeGlobal() const;
	bool canCastTo(ZVarType const& target) const;
	ZVarTypeIdSimple getId() const {return simpleId;}
	int classId() const {return ZVARTYPE_CLASSID_SIMPLE;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	ZVarTypeIdSimple simpleId;
	string name;
};

class ZVarTypeUnresolved : public ZVarType
{
public:
	ZVarTypeUnresolved(string const& name) : name(name) {}
	ZVarTypeUnresolved* clone() const {return new ZVarTypeUnresolved(*this);}
	string getName() const {return name;}
	ZVarType* resolve(Scope& scope);
	bool isResolved() const {return false;}
	bool canCastTo(ZVarType const& target) const {return false;}
	int classId() const {return ZVARTYPE_CLASSID_UNRESOLVED;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	string name;
};

#endif
