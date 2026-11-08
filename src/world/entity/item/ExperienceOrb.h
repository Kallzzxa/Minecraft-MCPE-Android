#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__ExperienceOrb_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__ExperienceOrb_H__

//package net.minecraft.world.entity.item;
//
// v8: experience orb. Follows the ItemEntity pattern (small falling
// entity, picked up on touch), but instead of going into the inventory
// it grants XP to the player. No XP system existed in this tree before
// v8 (only a vestigial WaterAnimal::getExperienceReward).

#include "../../entity/Entity.h"
#include "../../../SharedConstants.h"

class Level;
class Player;

class ExperienceOrb: public Entity
{
	typedef Entity super;

	static const int LIFETIME;
public:
	ExperienceOrb(Level* level);
	ExperienceOrb(Level* level, float x, float y, float z, int value);
	~ExperienceOrb();

	void tick();
	void playerTouch(Player* player);
	int getEntityTypeId() const;
	int getValue() const { return value; }

protected:
	void addAdditonalSaveData(CompoundTag* entityTag);
	void readAdditionalSaveData(CompoundTag* tag);

public:
	int value;
	int age;
private:
	int lifeTime;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__ExperienceOrb_H__*/
