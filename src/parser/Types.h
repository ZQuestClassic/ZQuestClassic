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
	ZVARTYPE_CLASSID_UNRESOLVED,
	ZVARTYPE_CLASSID_CONST_FLOAT
};

enum ZVarTypeIdSimple
{
    ZVARTYPEID_VOID, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL,
    ZVARTYPEID_FFC, ZVARTYPEID_ITEM, ZVARTYPEID_ITEMCLASS, ZVARTYPEID_NPC, ZVARTYPEID_LWPN, ZVARTYPEID_EWPN,
    ZVARTYPEID_GAME, ZVARTYPEID_LINK, ZVARTYPEID_SCREEN,
	ZVARTYPEID_END
};

class ZVarTypeSimple;
class ZVarTypeConstFloat;

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
	static ZVarTypeConstFloat const CONST_FLOAT;
	static ZVarType const* get(ZVarTypeId id);
};

class ZVarTypeSimple : public ZVarType
{
public:
	ZVarTypeSimple(ZVarTypeIdSimple simpleId, string const& name, string const& upName)
			: simpleId(simpleId), name(name), upName(upName) {}
	ZVarTypeSimple* clone() const {return new ZVarTypeSimple(*this);}
	string getName() const {return name;}
	string getUpName() const {return upName;}
	bool canBeGlobal() const;
	bool canCastTo(ZVarType const& target) const;
	ZVarTypeIdSimple getId() const {return simpleId;}
	int classId() const {return ZVARTYPE_CLASSID_SIMPLE;}
protected:
	int selfCompare(ZVarType const& other) const;
private:
	ZVarTypeIdSimple simpleId;
	string name;
	string upName;
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

// Temporary while only floats can be constant.
class ZVarTypeConstFloat : public ZVarType
{
public:
	ZVarTypeConstFloat() {}
	ZVarType* clone() const {return new ZVarTypeConstFloat(*this);}
	string getName() const {return "const float";}
	ZVarType* resolve(Scope& scope) {return this;}
	bool canBeGlobal() const {return true;}
	bool canCastTo(ZVarType const& target) const;
	int classId() const {return ZVARTYPE_CLASSID_CONST_FLOAT;};
protected:
	int selfCompare(ZVarType const& other) const {return 0;};
};

#endif
