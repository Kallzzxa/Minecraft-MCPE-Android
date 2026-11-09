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
//
// v25: player throws. The (Level, Mob, aux, splashOwner) ctor lets a
// PLAYER-thrown potion splash its own thrower (modern MC: a splash
// potion of healing at your own feet heals you, a splash poison
// hurts you) while the witch keeps the v24 behaviour (never splashed
// by its own potion). The aux is normalised to its base type on
// construction (PotionItem::baseAux strips the v25 splash flag), so
// onHit / the renderer / save-load always see a 0..10 potion type.

#include "Throwable.h"
#include "../EntityTypes.h"

class ThrownPotion: public Throwable
{
	typedef Throwable super;
public:
	ThrownPotion(Level* level);
	ThrownPotion(Level* level, Mob* mob);
	ThrownPotion(Level* level, Mob* mob, int potionAux);
	// v25: splashOwner=true lets the thrower be splashed by its own
	// potion (player throws); false keeps the v24 witch behaviour.
	ThrownPotion(Level* level, Mob* mob, int potionAux, bool splashOwner);

	float getGravity() { return 0.05f; }
	int getEntityTypeId() const { return EntityTypes::IdThrownPotion; }

	int getPotionAux() const { return potionAux; }

	// v48: MP sync. The server packs the base potion aux into the
	// AddEntityPacket data int (see getAuxData) so the client's
	// copy of the potion knows its type — without this the client
	// copy keeps the default-ctor POISON and both the flying bottle
	// (ThrownPotionRenderer reads getPotionAux) and the impact
	// burst tint would always be poison. setPotionAux is only
	// called from the network handler, never from gameplay.
	void setPotionAux(int aux);
	// v48: packet data = ownerId (low 24 bits, as Throwable has
	// always sent) | base aux in bits 24..27. Base aux only (0..10)
	// — the tint and the bottle icon both mask to base anyway, and
	// keeping it to 4 bits leaves bit 31 clear so hasMovementData()
	// (> 0) behaves exactly as before. Bound: ownerId < 2^24
	// (entityIds are a per-run counter; the tree already packs
	// FallingTile tile|data<<16 the same way).
	int getAuxData();

	void addAdditonalSaveData(CompoundTag* tag);
	void readAdditionalSaveData(CompoundTag* tag);

protected:
	void onHit(const HitResult& res);

private:
	int potionAux;
	bool splashOwner;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__ThrownPotion_H__*/
