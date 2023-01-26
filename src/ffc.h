#ifndef _FFC_H_
#define _FFC_H_

#include "zfix.h"
#include "solidobject.h"

//x = ffx
//y = ffy
//vx = ffxdelta
//vy = ffydelta
//ax = ffxdelta2
//ay = ffydelta2
//flags  = ffflags
//data = ffdata
//delay = ffdelay
//cset = ffcset
//link = fflink
//script = ffscript
//ffwidth, ffheight?

class ffcdata : public solid_object
{
public:
	zfix ax, ay;
	dword flags;
	word delay;
	byte cset, link, txsz = 1, tysz = 1;
	word script;
	int32_t initd[INITIAL_D];
	int32_t inita[INITIAL_A];
	bool initialized, hooked;
	
	ffcdata() = default;
	ffcdata(ffcdata const& other);
	virtual void solid_update(bool push = true) override;
	void changerCopy(ffcdata& other, int32_t i = -1, int32_t j = -1);
	void clear();
	
	void setData(word newdata);
	void incData(int32_t inc);
	word const& getData() const {return data;}
	void draw(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay);
	
	virtual bool setSolid(bool set) override;
	virtual void updateSolid() override;
	void setLoaded(bool set);
	bool getLoaded() const;
	
	//Overload to do damage to Hero on pushing them
	virtual void doContactDamage(int32_t hdir) override;
private:
	word data;
	bool loaded;
};

#endif
