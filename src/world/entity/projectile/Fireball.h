#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__

// v21: ghast / blaze fireball (new entity type, EntityTypes::IdFireball).
// A Throwable with no gravity that explodes (radius 1, with fire, like
// modern MC) on hitting a block or an entity. The (Level, Mob) ctor
// reuses Throwable's: it positions the ball at the shooter's eye and
// fires it along the look direction; ranged mobs then re-aim it with
// shoot(xd, yd, zd, ...) before the first tick, which overwrites the
// velocity (Throwable::shoot sets it, it does not add).

#include "Throwable.h"
#include "../EntityTypes.h"

class Fireball: public Throwable
{
	typedef Throwable super;
public:
	Fireball(Level* level);
	Fireball(Level* level, Mob* mob);

	float getGravity() { return 0.0f; }
	int getEntityTypeId() const { return EntityTypes::IdFireball; }

protected:
	void onHit(const HitResult& res);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__*/
