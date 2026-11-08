#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__

// v22: ender dragon (MobTypes::EnderDragon = 41). The End boss.
//
// A Monster (old-AI, like the ghast) that flies a steady orbit
// around the island centre (radius 22, y=82 — above every pillar
// top inside that radius) and shoots fireballs at the player from
// up to 40 blocks away, skeleton-style, with a melee fallback up
// close. 200 HP, 6 melee damage, fire-immune. It never lands or
// perches (no perch AI exists in this tree — the dragon circles
// until it is shot or hit out of the sky).
//
// Spawning: NOT in any spawn list. Minecraft::ensureEndDragon
// spawns exactly one when the player first enters the End, and
// never again once the exit portal exists (the dragon's death
// fills it — see dropDeathLoot). A dragon killed by a player drops
// 120 XP (modern MC gives 12,000, but Mob::dropExperience splits
// into 1-3 XP orbs, so 12,000 would spawn ~6,000 orb entities;
// 120 is the biggest sane reward here) and activates the exit
// portal + dragon egg. Sounds are placeholders (no dragon audio
// files exist in this tree; v19 left the enderman silent rather
// than faking it — the dragon keeps silent ambient, pop for hurt,
// explode for death).

#include "Monster.h"

class EnderDragon: public Monster
{
	typedef Monster super;
public:
	EnderDragon(Level* level);

	int getMaxHealth() { return 200; }
	int getEntityTypeId() const { return MobTypes::EnderDragon; }
	int getMaxSpawnClusterSize() { return 1; }

	void tick();
	void travel(float xa, float ya);

	const char* getAmbientSound() { return NULL; }
	std::string getHurtSound() { return "random.pop"; }
	std::string getDeathSound() { return "random.explode"; }

	virtual int getExperienceReward(Player* killedBy);

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();

private:
	// Orbit: radius 22 around (0, 0), altitude 82. The dragon
	// steers toward the moving orbit point every tick.
	static const int ORBIT_RADIUS = 22;
	static const int ORBIT_HEIGHT = 82;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__*/
