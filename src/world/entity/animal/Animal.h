#ifndef NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Animal_H__
#define NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Animal_H__

//package net.minecraft.world.entity.animal;

#include "../AgableMob.h"
#include "../Creature.h"

class Level;
class Entity;
class CompoundTag;

class Animal:   public AgableMob,
                public Creature
{
	typedef AgableMob super;

public:
    Animal(Level* level);

	//@Override
	bool hurt(Entity* source, int dmg);

    bool canSpawn();

    int getAmbientSoundInterval();

	int getCreatureBaseType() const;

	// v8: passive animals drop 1-3 XP when killed by a player.
	virtual int getExperienceReward(Player* killedBy);

	bool removeWhenFarAway();

protected:
	float getWalkTargetValue(int x, int y, int z);
	Entity* findAttackTarget();
private:
	int inLove;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ANIMAL__Animal_H__*/
