#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Slime_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Slime_H__

//package net.minecraft.world.entity->monster;

#include "Monster.h"
#include <string>

class Level;

class Slime: public Monster
{
	typedef Monster super;
public:
    Slime(Level* level);

	~Slime();

    /*@Override*/
    int getMaxHealth();

    /*@Override*/
    void aiStep();
    /*@Override*/
    void tick();

	virtual int getEntityTypeId() const;

	// Squash-and-stretch animation value, -0.2 (airborne, stretched)
	// .. 0.25 (on ground, squashed); interpolated by the renderer.
	float getSquish(float a);

protected:
	const char* getAmbientSound();
    std::string getHurtSound();
    std::string getDeathSound();

    int getDeathLoot();

	int jumpDelay;
	float squish;
	float prevSquish;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Slime_H__*/
