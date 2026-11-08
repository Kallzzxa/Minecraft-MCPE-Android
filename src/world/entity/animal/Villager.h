#ifndef NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Villager_H__
#define NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Villager_H__

// v23: villager. Passive Animal (like Pig/Cow): it never despawns on its
// own (Animal::removeWhenFarAway() is false) and wanders with the old
// PathfinderMob stroll AI (updateAi -> findRandomStrollLocation); there
// is no "go home at night" goal in this tree, so villagers just stroll
// around the village. Tapping one opens the trading screen
// (Villager::interact -> Player::openTrading). No death loot, no XP on
// kill (unlike other animals). Sounds are "mob.villager", which has no
// registered file — silent, same as the enderman precedent (v12/v19).

#include "Animal.h"

class Player;

class Villager: public Animal
{
	typedef Animal super;

public:
	Villager(Level* level);

	int getEntityTypeId() const;
	int getMaxHealth();

	bool interact(Player* player);

	const char* getAmbientSound();
	std::string getHurtSound();
	std::string getDeathSound();

	// Villagers give no XP when killed (modern MC).
	virtual int getExperienceReward(Player* killedBy);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Villager_H__*/
