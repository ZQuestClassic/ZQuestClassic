#ifndef ZC_ENEMY_HIVE_H
#define ZC_ENEMY_HIVE_H

#include "../guys.h"
#include <vector>

class HiveOrbiter;

class Hive: public enemy
{
public:
	static int numOrbiters(guydata& data);

	Hive(zfix x, zfix y, int id, int clk);
	HiveOrbiter* createOrbiter();

private:
	int& mainTimer;
	int timerLimit;
	std::vector<HiveOrbiter*> orbiters;
	int outerRingCount, innerRingCount, patternCounter;

	/* Called by orbiters when they die. Updates the list of living orbiters. */
	void orbiterDied(HiveOrbiter* orbiter);
	int defend(int wpnId, int *power, int edef) override;
	int defendNew(int wpnId, int *power, int edef, byte unblockable) override;
	bool animate(int index) override;
	void draw(BITMAP *dest) override;

	inline bool isBig() const { return dmisc10 == 1; }

	friend class HiveOrbiter;
};

class HiveOrbiter: public enemy
{
public:
	HiveOrbiter(int id, Hive* parent, int pos, bool inner);


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
	bool animate(int index) override;
	void draw(BITMAP *dest) override;

	inline bool isBig() const { return dmisc10 == 1; }

	friend class Hive;
};

#endif
