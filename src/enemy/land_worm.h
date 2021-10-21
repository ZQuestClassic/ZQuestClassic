#ifndef ZC_ENEMY_LANDWORM_H
#define ZC_ENEMY_LANDWORM_H

#include "guys.h"
#include <deque>
#include <memory>

class LandWormSegment;

class LandWorm : public enemy
{
public:
	LandWorm(zfix x, zfix y, int32_t id);
	/* Creates and returns the next segment. Doesn't keep track of how many
	 * should be created; use numSegments() for that.
	 */
	LandWormSegment* createSegment();

	/* Returns the number of segments to be created. */
	static int32_t numSegments(const guydata& data);

private:
	int32_t segmentsCreated;
	LandWormSegment* lastSegmentCreated;
	std::shared_ptr<std::deque<direction>> moveHistory;

	/* Moves the worm if its fixed spawn point is in an invalid location. */
	void adjustSpawnPoint();

	bool animate(int32_t index) override;
	void draw(BITMAP*) override {} // Invisible

	static constexpr int32_t MAX_SEGMENTS = 254;
};

class LandWormSegment : public enemy
{
public:
	LandWormSegment(zfix x, zfix y, int32_t id, int32_t clk,
		std::shared_ptr<std::deque<direction>> moveHistory);

private:
	LandWormSegment* prevSegment;
	LandWormSegment* nextSegment;
	int32_t moveTimer;
	int32_t moveIndex; // The index in moveHistory this segment reads
	std::shared_ptr<std::deque<direction>> moveHistory;

	/* Called when a segment dies. Shifts its HP (and a few other things)
	 * to the last segment so that it's the one that dies.
	 */
	void moveToEnd();
	inline bool isHead() const { return id&0x1000; }
	inline bool isTail() const { return !nextSegment; }

	/* Similar to constant_walk, but omits irrelevant details and makes
	 * non-head segments follow the same path.
	 */
	void walk();
	int32_t takehit(weapon* w) override;
	bool animate(int32_t index) override;
	void draw(BITMAP* dest) override;

	friend class LandWorm;
};

#endif
