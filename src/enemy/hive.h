#ifndef ZC_ENEMY_HIVE_H
#define ZC_ENEMY_HIVE_H

#include "../guys.h"
#include <vector>

class HiveOrbiter;

class Hive: public enemy
{
public:
	static int numOrbiters(guydata& data);

	Hive(zfix x, zfix y, int32_t id, int32_t clk);
	HiveOrbiter* createOrbiter();

private:
	int32_t& mainTimer;
	int32_t timerLimit;
	std::vector<HiveOrbiter*> orbiters;
	int32_t outerRingCount, innerRingCount, patternCounter;

	/* Called by orbiters when they die. Updates the list of living orbiters. */
	void orbiterDied(HiveOrbiter* orbiter);
	int32_t defend(int32_t wpnId, int32_t* power, int32_t edef) override;
	int32_t defendNew(int32_t wpnId, int32_t* power, int32_t edef, byte unblockable) override;
	bool animate(int32_t index) override;
	void draw(BITMAP *dest) override;

	inline bool isBig() const { return dmisc10 == 1; }

	friend class HiveOrbiter;
};

class HiveOrbiter: public enemy
{
public:
	HiveOrbiter(int32_t id, Hive* parent, int32_t pos, bool inner);


private:
	Hive* parent;

	/* True if this is part of the inner ring. */
	bool inner;

	/* This orbiter's position as a fraction of a circle ( [0, 1) ) */
	double relOffset;

	/* This orbiter's position as an angle ( [0, 2*pi) ) */
	double absOffset;

	void positionBigCircle();
	void positionOval();
	bool animate(int32_t index) override;
	void draw(BITMAP *dest) override;

	inline bool isBig() const { return dmisc10 == 1; }

	friend class Hive;
};

#endif
