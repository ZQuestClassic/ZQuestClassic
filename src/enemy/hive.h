#ifndef ZC_ENEMY_HIVE_H
#define ZC_ENEMY_HIVE_H

#include "../guys.h"

class ePatra : public enemy
{
public:
	int flycnt,flycnt2, loopcnt, lookat;
	double circle_x, circle_y;
	double temp_x, temp_y;
	bool adjusted;
	//ePatra(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	ePatra(zfix X,zfix Y,int Id,int Clk);                     // : enemy((zfix)128,(zfix)48,Id,Clk)
	virtual bool animate(int index);
	virtual void draw(BITMAP *dest);
	virtual int defend(int wpnId, int *power, int edef);
	virtual int defendNew(int wpnId, int *power, int edef, byte unblockable);
};

// segment class
class esPatra : public enemy
{
public:
	//esPatra(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	esPatra(zfix X,zfix Y,int Id,int Clk,sprite * prnt);                    // : enemy(X,Y,Id,Clk)
	sprite * parent;
	virtual bool animate(int index);
	virtual void draw(BITMAP *dest);
};

class ePatraBS : public enemy
{
public:
	int flycnt,flycnt2, loopcnt, lookat;
	double temp_x, temp_y;
	bool adjusted;
	//ePatraBS(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	ePatraBS(zfix X,zfix Y,int Id,int Clk);                   // : enemy((zfix)128,(zfix)48,Id,Clk)
	virtual bool animate(int index);
	virtual void draw(BITMAP *dest);
	virtual int defend(int wpnId, int *power, int edef);
	virtual int defendNew(int wpnId, int *power, int edef, byte unblockable);
};

// segment class
class esPatraBS : public enemy
{
public:
	//esPatraBS(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	esPatraBS(zfix X,zfix Y,int Id,int Clk,sprite * prnt);                  // : enemy(X,Y,Id,Clk)
	sprite * parent;
	virtual bool animate(int index);
	virtual void draw(BITMAP *dest);
};

#endif
