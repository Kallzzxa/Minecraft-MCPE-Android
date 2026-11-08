#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__WitherSkeleton_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__WitherSkeleton_H__

// v32: wither skeleton (MobTypes::WitherSkeleton = 45). The
// fortress's other signature mob (v31 audit deferred it: Skeleton
// is a bow mob, the wither effect did not exist, no skull item).
//
// A Skeleton subclass, but melee, NOT ranged:
//  - aiStep is Monster::aiStep, NOT Skeleton::aiStep — the skeleton
//    sunlight burn is skipped (wither skeletons do not burn in
//    daylight in modern MC, and they live in the Nether anyway).
//  - checkHurtTarget is Monster::checkHurtTarget — a melee hit
//    within 2 blocks. Crucially it does NOT set holdGround (the
//    skeleton's ranged version does, which is why a skeleton stands
//    still and shoots); the old AI in PathfinderMob::updateAi keeps
//    re-pathing toward the target while !holdGround, so the wither
//    skeleton walks up and swings like a zombie. No goal setup is
//    needed — the witch (v24) proves the same old-AI +
//    checkHurtTarget-override pattern for a non-melee mob.
//  - doHurtTarget applies the wither effect (10 s, Mob::tick drains
//    1 HP / 40 ticks and CAN kill — the poison contrast).
//  - getCarriedItem returns a stone sword (modern MC) instead of
//    the skeleton's bow; getUseDuration returns 0 so no bow-draw
//    pose is ever applied to it.
//  - fireImmune (lava/Nether), 20 HP, 5 melee damage (modern MC
//    normal difficulty), black skeleton texture (a darkened
//    mob/skeleton.png, same tint precedent as the v31 magma cube).
// The plain Skeleton is untouched: it still shoots arrows.

#include "Skeleton.h"
#include "../../item/ItemInstance.h"

class Level;
class Entity;

class WitherSkeleton: public Skeleton
{
	typedef Skeleton super;
public:
	WitherSkeleton(Level* level);

	int getMaxHealth();

	void aiStep();

	int getDeathLoot();

	ItemInstance* getCarriedItem();

	virtual int getEntityTypeId() const;
protected:
	const char* getAmbientSound();
	std::string getHurtSound();
	std::string getDeathSound();

	void checkHurtTarget(Entity* target, float d);
	bool doHurtTarget(Entity* target);
	void dropDeathLoot();
	virtual int getUseDuration();
private:
	ItemInstance sword;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__WitherSkeleton_H__*/
