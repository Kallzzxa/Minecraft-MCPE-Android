#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__

// v21: ghast / blaze fireball (new entity type, EntityTypes::IdFireball).
// A Throwable with no gravity that explodes (radius 1, with fire, like
// modern MC) on hitting a block or an entity. The (Level, Mob) ctor
// reuses Throwable's: it positions the ball at the shooter's eye and
// fires it along the look direction; ranged mobs then re-aim it with
// shoot(xd, yd, zd, ...) before the first tick, which overwrites the
// velocity (Throwable::shoot sets it, it does not add).
//
// v41: the ball is pickable and punchable — a player punch reflects it
// (see Fireball::hurt): it flies back along the player's look
// direction and its ownerId is reassigned to the deflector, so the
// v40 owner-only immunity lets the reflected ball hurt the ghast /
// blaze that shot it (modern MC's ghast fight). Only ghast/blaze
// fireballs reflect; a dragon fireball is left alone (the dragon has
// no v40 owner immunity, so reflecting its ball would let it hurt
// the dragon — a change to the End fight this batch does not make).

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

	// v41: pickable so the crosshair can target it (GameRenderer::pick
	// skips non-pickable entities), and a generous pick radius — the
	// ball is small (0.5x0.5) and fast (~1 block/tick), so the
	// default 0.1 radius would make it nearly unhittable.
	bool isPickable();
	float getPickRadius();
	bool hurt(Entity* source, int damage);

protected:
	void onHit(const HitResult& res);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Fireball_H__*/
