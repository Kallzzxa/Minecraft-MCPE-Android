#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__

// v34: wither boss (MobTypes::Wither = 46), stage 1.
//
// A Monster (old-AI, like the ghast / ender dragon) that flies by
// steering (dragon's steerTo + ghast's gravity-free travel), chases
// the nearest player in the air and shoots wither-skull projectiles
// (WitherSkull). 300 HP, fire-immune, regenerates 1 HP / 20 ticks.
//
// Spawning: summon-only — never in any spawn list. The ritual is
// detected in trySummonFromSkull (called from SkullTile::setPlacedBy
// when a skull is placed): soul sand in a T (3 in a row + 1 below
// the middle) with a wither skull on top of each of the 3, in either
// the X or the Z orientation. The structure is consumed, an
// explosion goes off, and the wither appears above it.
//
// Boss-bar: Gui::renderBossBar finds the live wither in
// Level::entities (no tracking state on the boss itself).
//
// Like the dragon, tick() calls Mob::tick(), NOT Monster::tick() —
// Monster::tick removes the mob on PEACEFUL, which would delete the
// boss mid-fight. Sounds reuse the skeleton's (no wither audio
// files exist in this tree).

#include "Monster.h"

class Wither: public Monster
{
	typedef Monster super;
public:
	Wither(Level* level);

	int getMaxHealth() { return WITHER_MAX_HEALTH; }
	int getEntityTypeId() const { return MobTypes::Wither; }
	int getMaxSpawnClusterSize() { return 1; }

	// Mob::heal caps at 20 HP; the wither's max is 300 (same
	// override shape as the dragon's 200).
	void heal(int amount);

	void tick();
	void travel(float xa, float ya);

	// The wither flies by steering — it never needs the ground
	// jump PathfinderMob::updateAi sets on horizontal collision
	// (same fix shape as the dragon's perch-hop suppression,
	// but unconditional: the wither is airborne everywhere).
	void updateAi();

	const char* getAmbientSound() { return "mob.skeleton"; }
	std::string getHurtSound() { return "mob.skeletonhurt"; }
	std::string getDeathSound() { return "mob.skeletondeath"; }

	virtual int getExperienceReward(Player* killedBy);

	// Ritual detection. (x, y, z) is a freshly placed skull tile.
	// Returns true when the full T + 3 skulls matched and a wither
	// was summoned (the structure is consumed). Server-side only
	// (level->isClientSide bails out — in MP the client also runs
	// the place path, and the server is the one that summons).
	static bool trySummonFromSkull(Level* level, int x, int y, int z);

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();

	// Dragon-only wide target acquisition, same reason: a flying
	// boss hovers well outside Monster's 16-block range, so the
	// 16-block findAttackTarget would almost never acquire while
	// the wither circles above the player.
	Entity* findAttackTarget();

private:
	static const int WITHER_MAX_HEALTH = 300;
	static const int WITHER_TARGET_RANGE = 48;
	static const int WITHER_HOVER_HEIGHT = 3; // blocks above the player

	// Steer the velocity toward a fixed point (the dragon's orbit
	// steering, reused exactly: ease toward the point, capped
	// 0.35/tick, velocity smoothed 0.2).
	void steerTo(float tx, float ty, float tz);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__*/
