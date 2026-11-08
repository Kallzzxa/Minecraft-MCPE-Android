#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Witch_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Witch_H__

// v24: witch (MobTypes::Witch = 43, modern MC id). A ranged monster
// that keeps its distance and throws splash potions at the player,
// old-AI style like the skeleton/ghasts (PathfinderMob chase +
// checkHurtTarget). 26 HP like modern MC, no melee (attackDamage 0),
// does NOT burn in daylight (modern MC). Drops 0-2 sticks + a chance
// of spider eye / sugar / glowstone dust / glass bottle. When badly
// hurt it drinks a healing potion (heal 4, 10 s cooldown) like the
// modern witch. Sounds are "mob.witch", unregistered — silent, same
// as the villager/enderman precedent (v12/v19/v23). Spawns at night
// (Biome enemies, weight 1, always alone) and one lives in each
// swamp hut (SwampHutFeature::ensureWitch).

#include "Monster.h"

class Witch: public Monster
{
	typedef Monster super;
public:
	Witch(Level* level);

	int getMaxHealth() { return 26; }
	int getEntityTypeId() const { return MobTypes::Witch; }
	int getMaxSpawnClusterSize() { return 1; }

	void tick();

	const char* getAmbientSound() { return "mob.witch"; }
	std::string getHurtSound() { return "mob.witch"; }
	std::string getDeathSound() { return "mob.witch"; }

	// Modern MC: 5 XP (Monster's default is 3-5).
	virtual int getExperienceReward(Player* killedBy);

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();

private:
	// Pick the potion to throw: finish a weak target with harming,
	// otherwise poison / slowness / weakness / harming at random,
	// skipping an effect the target already has.
	int choosePotion(Mob* target);

	int drinkCooldown;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Witch_H__*/
