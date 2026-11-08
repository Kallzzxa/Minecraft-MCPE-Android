#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Enderman_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Enderman_H__

//package net.minecraft.world.entity->monster;

#include "Monster.h"
#include <string>

class Level;

class Enderman: public Monster
{
	typedef Monster super;
public:
    Enderman(Level* level);

	~Enderman();

    /*@Override*/
    int getMaxHealth();

    /*@Override*/
    bool hurt(Entity* source, int dmg);

	virtual int getEntityTypeId() const;

protected:
	const char* getAmbientSound();
    std::string getHurtSound();
    std::string getDeathSound();

    int getDeathLoot();

	// Vanilla-style escape teleport: up to 8 tries for a safe spot
	// within 16 blocks. Returns true if it teleported.
	bool teleportRandomly();
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Enderman_H__*/
