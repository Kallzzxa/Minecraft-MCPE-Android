#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__ThrownPotion_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__ThrownPotion_H__

// v24: thrown splash potion (new entity type, EntityTypes::
// IdThrownPotion). A Throwable with a slight arc (gravity 0.05,
// heavier than the 0.03 default so it lobs like a thrown bottle)
// that SPLASHES on hitting a block or an entity: every mob within
// 3 blocks of the impact gets the potion's effect via
// PotionItem::applyEffect (poison / slowness / weakness / harming —
// the v20 effects, durations included). This is a real area splash,
// not a direct-hit-only projectile: a potion that lands at the
// player's feet still poisons them. The thrower is never splashed
// by its own potion. The potion type (aux) is saved/loaded so a
// thrown potion survives a save cycle mid-flight.

#include "Throwable.h"
#include "../EntityTypes.h"

class ThrownPotion: public Throwable
{
	typedef Throwable super;
public:
	ThrownPotion(Level* level);
	ThrownPotion(Level* level, Mob* mob);
	ThrownPotion(Level* level, Mob* mob, int potionAux);

	float getGravity() { return 0.05f; }
	int getEntityTypeId() const { return EntityTypes::IdThrownPotion; }

	int getPotionAux() const { return potionAux; }

	void addAdditonalSaveData(CompoundTag* tag);
	void readAdditionalSaveData(CompoundTag* tag);

protected:
	void onHit(const HitResult& res);

private:
	int potionAux;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__ThrownPotion_H__*/
