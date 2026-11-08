#include "WitherSkeleton.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../../util/Mth.h"
#include "Monster.h"

WitherSkeleton::WitherSkeleton(Level* level)
:	super(level),
	sword(Item::sword_stone, 1)
{
	// Skeleton's ctor set ER_SKELETON_RENDERER (HumanoidMobRenderer
	// + SkeletonModel) and the bow; only the texture, the weapon and
	// the Nether traits differ.
	this->textureName = "mob/witherskeleton.png";
	fireImmune = true;
	runSpeed = 0.5f;
	attackDamage = 5;
}

int WitherSkeleton::getMaxHealth() {
	return 20;
}

void WitherSkeleton::aiStep() {
	// Skip Skeleton::aiStep on purpose: its sunlight burn does not
	// apply to wither skeletons. Monster::aiStep is what Skeleton
	// calls after the burn check (updateAttackAnim + old AI).
	Monster::aiStep();
}

int WitherSkeleton::getDeathLoot() {
	return Item::bone->id;
}

ItemInstance* WitherSkeleton::getCarriedItem() {
	return &sword;
}

int WitherSkeleton::getEntityTypeId() const {
	return MobTypes::WitherSkeleton;
}

const char* WitherSkeleton::getAmbientSound() {
	return "mob.skeleton";
}

std::string WitherSkeleton::getHurtSound() {
	return "mob.skeletonhurt";
}

std::string WitherSkeleton::getDeathSound() {
	return "mob.skeletondeath";
}

void WitherSkeleton::checkHurtTarget(Entity* target, float d) {
	// Melee, not the skeleton's bow shot: Monster's version hits
	// within 2 blocks and never sets holdGround, so updateAi keeps
	// walking the mob toward its target.
	Monster::checkHurtTarget(target, d);
}

bool WitherSkeleton::doHurtTarget(Entity* target) {
	bool hit = Monster::doHurtTarget(target);
	if (hit && target != NULL && target->isMob()) {
		// Modern MC: 10 s of wither on normal difficulty.
		((Mob*)target)->witherEffectTicks = 10 * 20;
	}
	return hit;
}

void WitherSkeleton::dropDeathLoot() {
	// Modern MC: 0-2 bones, 0-1 coal, and a 2.5% wither skull.
	int count = random.nextInt(3);
	for (int i = 0; i < count; i++) {
		spawnAtLocation(Item::bone->id, 1);
	}
	if (random.nextInt(3) == 0) {
		spawnAtLocation(Item::coal->id, 1);
	}
	if (random.nextFloat() < 0.025f) {
		spawnAtLocation(Item::witherSkull->id, 1);
	}
}

int WitherSkeleton::getUseDuration() {
	// No bow: never report a bow-draw duration.
	return 0;
}
