#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__WitherSkull_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__WitherSkull_H__

// v34: wither-skull projectile (new entity type,
// EntityTypes::IdWitherSkull). A Throwable with no gravity, on the
// Fireball pattern — but it does NOT set fire, and on impact it
// applies Wither I (10 s, the v32 effect system) to every mob caught
// in the blast before a small explosion (radius 1, no fire, like
// modern MC). Fired by the wither boss (Wither::checkHurtTarget).

#include "Throwable.h"
#include "../EntityTypes.h"

class WitherSkull: public Throwable
{
	typedef Throwable super;
public:
	WitherSkull(Level* level);
	WitherSkull(Level* level, Mob* mob);

	float getGravity() { return 0.0f; }
	int getEntityTypeId() const { return EntityTypes::IdWitherSkull; }

protected:
	void onHit(const HitResult& res);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__WitherSkull_H__*/
